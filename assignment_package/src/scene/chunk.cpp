#include "chunk.h"
#include <iostream>

Chunk::Chunk(OpenGLContext *context) : Drawable(context), m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) {
    // Boundary constraints on y direction
    if(y > 255)
    {
        return EMPTY;
    }
    if(y < 0)
    {
        return UNCERTAIN;
    }
    // Boundary constraints on x direction
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
    if(x > 15 || x < 0 || z > 15 || z < 0)
    {
        return UNCERTAIN;
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

void Chunk::createVBOdata()
{
    vector<GLuint> indices;
    vector<glm::vec4> interleavedList;
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
                        if(!isOpaque(getBlockAt(curr_neighbor.x, curr_neighbor.y, curr_neighbor.z)))
                        {
                            for(VertexData vertex : block.vertices)
                            {
                                // Position
                                interleavedList.push_back(glm::vec4(glm::vec3(x, y, z) + glm::vec3(vertex.pos), 1));
                                // Color
                                interleavedList.push_back(color);
                                // Normal
                                interleavedList.push_back(glm::vec4(block.directionVec, 0));
                                // UV coordinates, Animatable flag
                                glm::vec2 uv_coord = vertex.uv + blockFaceUVs[curr][block.direction];
                                float animatable = isAnimatable(getBlockAt(x, y, z));
                                interleavedList.push_back(glm::vec4(uv_coord, animatable, 1));
                            }
                            // Quadrangulated indices
                            indices.push_back(running_index);
                            indices.push_back(running_index + 1);
                            indices.push_back(running_index + 2);
                            indices.push_back(running_index);
                            indices.push_back(running_index + 2);
                            indices.push_back(running_index + 3);
                            running_index += 4;
                        }
                    }
                }
            }
        }
    }
    m_count = indices.size();
    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    generateInterleavedList();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInterleavedList);
    mp_context->glBufferData(GL_ARRAY_BUFFER, interleavedList.size() * sizeof(glm::vec4), interleavedList.data(), GL_STATIC_DRAW);
}

Chunk::~Chunk()
{}
