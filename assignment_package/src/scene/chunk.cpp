#include "chunk.h"
#include <iostream>
#include "cave.h"

Chunk::Chunk(OpenGLContext *context, int x, int z)
    : Drawable(context), m_xCorner(x), m_zCorner(z),
      m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}},
      isVBOready(false)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

glm::ivec2 Chunk::getCorners()
{
    return glm::ivec2(m_xCorner, m_zCorner);
}

BlockType Chunk::getWorldBlock(int x, int y, int z)
{
    x = m_xCorner + x;
    z = m_zCorner + z;
    auto HB = noise.computeHeight(x, z);
    int H = HB.first;
    int biome = HB.second;
    float snow_noise = noise.m_mountainHeightMap.noise2D({x, z});
    float caveNoiseVal = cavePerlinNoise3D(glm::vec3(x/25.f, y/16.f, z/25.f))/2 + 0.5; // output range [-1, 1] mapped to [0, 1]
    float caveMask = cavePerlinNoise3D(glm::vec3(z/100.f, x/100.f, y/100.f))/2 + 0.5; // similar to previous but rotate
    return noise.getBlockType(y, H, biome, snow_noise, caveNoiseVal, caveMask);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z, bool isModified = false) {
    // Boundary constraints on y direction
    if(y > 255)
    {
        return EMPTY;
    }
    if(y < 0)
    {
        return UNCERTAIN;
    }
    if(x > 15 || x < 0 || z > 15 || z < 0)
    {
        if(!isModified)
            return getWorldBlock(x, y, z);
        else
        {
            if(x < 0 && m_neighbors[XNEG] != nullptr)
            {
                return m_neighbors[XNEG]->getBlockAt(15, y, z);
            }
            if(x > 15 && m_neighbors[XPOS] != nullptr)
            {
                return m_neighbors[XPOS]->getBlockAt(0, y, z);
            }
            if(z < 0 && m_neighbors[ZNEG] != nullptr)
            {
                return m_neighbors[ZNEG]->getBlockAt(x, y, 15);
            }
            if(z > 15 && m_neighbors[ZPOS] != nullptr)
            {
                return m_neighbors[ZPOS]->getBlockAt(x, y, 0);
            }
            return getWorldBlock(x, y, z);
        }
    }
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}


// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

bool isOpaque(BlockType block)
{
    if(transparent_blocks.find(block) != transparent_blocks.end())
        return false;
    return true;
}

bool isAnimatable(BlockType block)
{
    if(animatable_blocks.find(block) != animatable_blocks.end())
        return true;
    return false;
}

glm::vec4 getColor(BlockType block)
{
    glm::vec4 color(0, 0, 0, 1);
    switch(block)
    {
        case GRASS:
            color = glm::vec4(0.494f, 0.784f, 0.313f, 1.f);
            break;
        case DIRT:
            color = glm::vec4(0.607f, 0.462f, 0.325f, 1.f);
            break;
        case STONE:
            color = glm::vec4(0.368f, 0.368f, 0.368f, 1.f);
            break;
        case ICE:
            color = glm::vec4(0.647f, 0.949f, 0.949f, 1.f);
            break;
        case WATER:
            color = glm::vec4(0.f, 0.f, 0.75f, 1.f);
            break;
        case SNOW:
            color = glm::vec4(1.f, 1.f, 1.f, 1.f);
            break;
        case SAND:
            color = glm::vec4(0.981f, 0.93f, 0.501f, 1.f);
        default:
            break;
    }
    return color;
}

GLenum Chunk::drawMode() {
    return GL_TRIANGLES;
}

void Chunk::generateVBOdata(bool isModified)
{
    isVBOready = false;
    // OPAQUE PASS

        indicesOpq.clear();
        interleavedOpq.clear();
        int running_index = 0;
        for(int x = 0; x < 16; x++)
        {
            for(int y = 0; y < 256; y++)
            {
                for(int z = 0; z < 16; z++)
                {
                    BlockType curr = getBlockAt(x, y, z);
                    if(isOpaque(curr))
                    {
                        glm::vec4 color = getColor(curr);
                        for(auto &block : adjacentFaces)
                        {
                            glm::ivec3 curr_neighbor = glm::ivec3(x, y, z) + glm::ivec3(block.directionVec);
                            if(!isOpaque(getBlockAt(curr_neighbor.x, curr_neighbor.y, curr_neighbor.z, isModified)))
                            {
                                for(VertexData vertex : block.vertices)
                                {
                                    // Position
                                    interleavedOpq.push_back(glm::vec4(glm::vec3(x, y, z) + glm::vec3(vertex.pos), 1));
                                    // Color
                                    interleavedOpq.push_back(color);
                                    // Normal
                                    interleavedOpq.push_back(glm::vec4(block.directionVec, 0));
                                    // UV coordinates, Animatable flag
                                    glm::vec2 uv_coord = vertex.uv + blockFaceUVs[curr][block.direction];
                                    float animatable = isAnimatable(getBlockAt(x, y, z));
                                    interleavedOpq.push_back(glm::vec4(uv_coord, animatable, 0.f)); // last el zero bc of new dual-tranparent alpha setting
                                }
                                // Quadrangulated indicesOpq
                                indicesOpq.push_back(running_index);
                                indicesOpq.push_back(running_index + 1);
                                indicesOpq.push_back(running_index + 2);
                                indicesOpq.push_back(running_index);
                                indicesOpq.push_back(running_index + 2);
                                indicesOpq.push_back(running_index + 3);
                                running_index += 4;
                            }
                        }
                    }
                }
            }
        }

    // TRANSPARENT PASS
        indicesTra.clear();
        interleavedTra.clear();
        running_index = 0;
        for(int x = 0; x < 16; x++)
        {
            for(int y = 0; y < 256; y++)
            {
                for(int z = 0; z < 16; z++)
                {
                    BlockType curr = getBlockAt(x, y, z);
                    if(!isOpaque(curr) && curr != EMPTY)
                    {
                        glm::vec4 color = getColor(curr);
                        for(auto &block : adjacentFaces)
                        {
                            glm::ivec3 curr_neighbor = glm::ivec3(x, y, z) + glm::ivec3(block.directionVec);
                            BlockType curr_neighbor_type = getBlockAt(curr_neighbor.x, curr_neighbor.y, curr_neighbor.z, isModified);

                            if(!isOpaque(curr_neighbor_type) && curr_neighbor_type != curr)
                            //if(curr_neighbor_type == EMPTY)
                            {
                                // INHERENT ISSUES WITH THIS (goal: showing ICE and WATER together when they touch)
                                // 11/02 Lecture @ 1:15:00 onward discusses why the correct drawing order is only completed when looking
                                // from a specific angle... Should probably just never draw face between adjacent transparent blocks
                                // if the occluding fragments clogging the z-buffer are in fact fully clear, can use discard() in frag shader
                                // visualize this by replacing sand with ice around water
                                bool curr_has_priority = transparent_block_order[curr] > transparent_block_order[curr_neighbor_type];
                                if(curr_has_priority)
                                {
                                    if (curr_neighbor_type == EMPTY)
                                    {
                                        // Draw transparent block normally
                                        for(VertexData vertex : block.vertices)
                                        {
                                            // Position
                                            interleavedTra.push_back(glm::vec4(glm::vec3(x, y, z) + glm::vec3(vertex.pos), 1));
                                            // Color
                                            interleavedTra.push_back(color);
                                            // Normal
                                            interleavedTra.push_back(glm::vec4(block.directionVec, 0));
                                            // UV coordinates, Animatable flag
                                            glm::vec2 uv_coord = vertex.uv + blockFaceUVs[curr][block.direction];
                                            float animatable = isAnimatable(getBlockAt(x, y, z));

                                            // Adding another float flag so that transparent blockfaces with priority between
                                            // 2 transparent blocks are told to draw from texture with alpha set to 1
                                            interleavedTra.push_back(glm::vec4(uv_coord, animatable, 0.f));
                                        }
                                        // Quadrangulated indicesTra
                                        indicesTra.push_back(running_index);
                                        indicesTra.push_back(running_index + 1);
                                        indicesTra.push_back(running_index + 2);
                                        indicesTra.push_back(running_index);
                                        indicesTra.push_back(running_index + 2);
                                        indicesTra.push_back(running_index + 3);
                                        running_index += 4;
                                    }
                                    else
                                    {
//                                        std::cout << "IN special Case" << std::endl;
                                        // Force dominant transparent blockface at face where two different transparent types
                                        // meet to use an alpha value of 1 so it gets drawn through the weaker transparent block
                                        // Goal is to show Ice and Water well together!
                                        for(VertexData vertex : block.vertices)
                                        {
                                            // Position
                                            interleavedOpq.push_back(glm::vec4(glm::vec3(x, y, z) + glm::vec3(vertex.pos), 1));
                                            // Color
                                            interleavedOpq.push_back(color);
                                            // Normal
                                            interleavedOpq.push_back(glm::vec4(block.directionVec, 0));
                                            // UV coordinates, Animatable flag
                                            glm::vec2 uv_coord = vertex.uv + blockFaceUVs[curr][block.direction];
                                            float animatable = isAnimatable(getBlockAt(x, y, z));

                                            // TODO: consider adding another float flag to pass to shader s.t.
                                            // transparent blockface w/ priority between 2 transparent blocks
                                            // are told to draw from texture with alpha set to 1

                                            interleavedOpq.push_back(glm::vec4(uv_coord, animatable, 1.f));
                                        }
                                        // Quadrangulated indicesTra
                                        int idx = indicesOpq.back()+1;
                                        indicesOpq.push_back(idx);
                                        indicesOpq.push_back(idx + 1);
                                        indicesOpq.push_back(idx + 2);
                                        indicesOpq.push_back(idx);
                                        indicesOpq.push_back(idx + 2);
                                        indicesOpq.push_back(idx + 3);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
}

void Chunk::loadVBOdata()
{
    // OPAQUE PASS
    opaquePass = true;
        m_countOpq = indicesOpq.size();
        // Create a VBO on our GPU and store its handle in bufIdx
        generateIdx();
        // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
        // and that it will be treated as an element array buffer (since it will contain triangle indices)
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpq);
        // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
        // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesOpq.size() * sizeof(GLuint), indicesOpq.data(), GL_STATIC_DRAW);
        generateInterleavedList();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInterleavedOpq);
        mp_context->glBufferData(GL_ARRAY_BUFFER, interleavedOpq.size() * sizeof(glm::vec4), interleavedOpq.data(), GL_STATIC_DRAW);
        //isOpqVBOready = true;

        // TRANSPARENT PASS
        opaquePass = false;
        m_countTra = indicesTra.size();
        // Create a VBO on our GPU and store its handle in bufIdx
        generateIdx();
        // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
        // and that it will be treated as an element array buffer (since it will contain triangle indicesTra)
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTra);
        // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
        // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesTra.size() * sizeof(GLuint), indicesTra.data(), GL_STATIC_DRAW);
        generateInterleavedList();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInterleavedTra);
        mp_context->glBufferData(GL_ARRAY_BUFFER, interleavedTra.size() * sizeof(glm::vec4), interleavedTra.data(), GL_STATIC_DRAW);

        isVBOready = true;
}
void Chunk::recreateVBOdata()
{
    destroyVBOdata();
    generateVBOdata(true);
    loadVBOdata();

}

void Chunk::createVBOdata()
{
    generateVBOdata();
    loadVBOdata();
}

Chunk::~Chunk()
{}
