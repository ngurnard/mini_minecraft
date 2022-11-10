#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context) //, m_geomCube(context)
{
    createHeightMaps();
}

Terrain::~Terrain() {
//    m_geomCube.destroyVBOdata();
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    // EVAN: fill w/ surface of terrain
    for(int i = x; i < x + 16; ++i) {
        for(int j = z; j < z + 16; ++j) {
            auto HB = computeHeight(i, j);
            float H = HB.first;
            float biome = HB.second;
            if (biome == 0) {
                // grassland
                if (H <= 138) {
                    setBlockAt(i, H, j, WATER);
                } else {
                    setBlockAt(i, H, j, GRASS);
                }
            } else {
                // mountains
                if (H >= 200) {
                    setBlockAt(i, H, j, SNOW);
                } else {
                    setBlockAt(i, H, j, STONE);
                }
            }
        }
    }
    cPtr->destroyVBOdata();
    cPtr->createVBOdata();
    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            if(hasChunkAt(x, z))
            {
                const uPtr<Chunk> &chunk = getChunkAt(x, z);
                shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, 0, z)));
                shaderProgram->drawInterleaved(*chunk);
            }
        }
    }
}

void Terrain::createHeightMaps()
{
    // Creates the FBM-based Height Maps for Mountains and Grassland
    // as well as a FBM-based Mask which will interpolate between these regions

    glm::vec2 range(129, 255); // y Height range where [0,128] should be stone, the rest is biome-specific

    int mtn_octaves = 4; float mtn_freq = 0.1f;
    float mtn_amp = 0.5; float mtn_persistance = 0.5;

    m_mountainHeightMap = customFBM(mtn_octaves, mtn_freq, mtn_amp, mtn_persistance, range);

    int grass_octaves = 8; float grass_freq = 0.06f;
    float grass_amp = 0.5; float grass_persistance = 0.5;

    m_grasslandHeightMap = customFBM(grass_octaves, grass_freq, grass_amp, grass_persistance, range);

    int mask_octaves = 8; float mask_freq = 0.025f;
    float mask_amp = 0.5; float mask_persistance = 0.5;
    glm::vec2 mask_range(0,1);

    m_biomeMaskMap = customFBM(mask_octaves, mask_freq, mask_amp, mask_persistance, mask_range);
}

void Terrain::mountainHeightPostProcess(float& val)
{
    // reduces peak distribution and confines values to >= |range|/2 + range[0]
    // val = 1 - 0.5 * pow(val, 0.3); // original
    val = 0.9 - 0.85 * pow(val, 0.55);
}

void Terrain::grasslandHeightPostProcess(float& val)
{
    // flatten and lower terrain relative to mountains
    // val = 0.25 * (1 - pow(val, 0.5)); // original
    val = 0.05 * pow(val, 0.65);
}

void Terrain::biomeMaskPostProcess(float& val)
{
    // smoothstep to increase contrast
    val = glm::smoothstep(0.85f, 0.15f, val); // original
}

std::pair<int, int> Terrain::computeHeight(int x, int z)
{
    // Computes individual biome heights and blends w/ biomeMask
    float mtnH = m_mountainHeightMap.computeFBM(x, z);
    mountainHeightPostProcess(mtnH);
    m_mountainHeightMap.mapOutput2Range(mtnH);

    float grassH = m_grasslandHeightMap.computeFBM(x, z);
    grasslandHeightPostProcess(grassH);
    m_grasslandHeightMap.mapOutput2Range(grassH);

    float mask = m_biomeMaskMap.computeFBM(x, z);
    biomeMaskPostProcess(mask);
    int biome = 0;
    if (mask > 0.5) {
        biome = 1;
    }
    // mask should be [0,1] bounded...

    return {floor(glm::mix(grassH, mtnH, mask)), biome};
}

void Terrain::printHeight(int x, int z)
{
    auto blockInfo = computeHeight(x, z);
    int H = blockInfo.first;
    int biome = blockInfo.second;
    std::cout << "Height Map @ [" << x << ", " << z << "] = " << H << " --BIOME " << biome << std::endl;
}

void Terrain::CreateTestScene()
{
    std::cout << "TestScene called" << std::endl;
    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(0, 0));

    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x) {
        for(int z = 0; z < 64; ++z) {
            if((x + z) % 2 == 0) {
                setBlockAt(x, 128, z, STONE);
            }
            else {
                setBlockAt(x, 128, z, DIRT);
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x) {
        setBlockAt(x, 129, 0, GRASS);
        setBlockAt(x, 130, 0, GRASS);
        setBlockAt(x, 129, 63, GRASS);
        setBlockAt(0, 130, x, GRASS);
    }
    // Add a central column
    for(int y = 129; y < 140; ++y) {
        setBlockAt(32, y, 32, GRASS);
    }
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            const uPtr<Chunk> &chunk = getChunkAt(x, z);
            chunk->destroyVBOdata();
            chunk->createVBOdata();
        }
    }
}

void Terrain::CreateTestTerrainScene()
{
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            instantiateChunkAt(x, z);
        }
    }

    m_generatedTerrain.insert(toKey(0, 0));

    for(int x = 0; x < 64; ++x) {
        for(int z = 0; z < 64; ++z) {
            auto HB = computeHeight(x, z);
            float H = HB.first;
            float biome = HB.second;
            if (biome == 0) {
                // grassland
                if (H <= 138) {
                    setBlockAt(x, H, z, WATER);
                } else {
                    setBlockAt(x, H, z, GRASS);
                }
            } else {
                // mountains
                if (H >= 200) {
                    setBlockAt(x, H, z, SNOW);
                } else {
                    setBlockAt(x, H, z, STONE);
                }
            }
        }
    }
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            const uPtr<Chunk> &chunk = getChunkAt(x, z);
            chunk->destroyVBOdata();
            chunk->createVBOdata();
        }
    }
}

void Terrain::updateTerrain(const glm::vec3 &player_pos)
{
    int player_pos_x = 16 * static_cast<int>(glm::floor(player_pos.x / 16.f));
    int player_pos_z = 16 * static_cast<int>(glm::floor(player_pos.z / 16.f));
    vector<glm::ivec2> directions = {
        glm::ivec2(0, 1),
        glm::ivec2(1, 0),
        glm::ivec2(0, -1),
        glm::ivec2(-1, 0),
        glm::ivec2(1, 1),
        glm::ivec2(-1, 1),
        glm::ivec2(-1, -1),
        glm::ivec2(1, -1)
    };
    for(int i = 0; i < directions.size(); i++)
    {
        glm::ivec2 new_pos = glm::ivec2(player_pos_x, player_pos_z) + 16 * directions[i];
        if(!hasChunkAt(new_pos.x, new_pos.y))
        {
            instantiateChunkAt(new_pos.x, new_pos.y);
        }
    }
}
