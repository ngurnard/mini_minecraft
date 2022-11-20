#include "terrain.h"
#include <stdexcept>
#include <iostream>
#include "cave.h"

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_tryExpansionTimer(0), m_generatedTerrain(), mp_context(context)
{}

Terrain::~Terrain() {
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

// uncomment for caves
void Terrain::setChunkBlocks(Chunk* chunk, int x, int z) {
    int waterH = 138;       // Height of water level
    for(int i = x; i < x + 16; ++i) {
        for(int j = z; j < z + 16; ++j) {

            // Get height and biome as pair from terrain
            auto HB = noise.computeHeight(i, j);
            int H = HB.first;
            int biome = HB.second;

            glm::vec2 chunkOrigin = glm::vec2(floor(i / 16.f) * 16, floor(j / 16.f) * 16);
            int coord_x = int(i - chunkOrigin.x), coord_z = int(j - chunkOrigin.y);
            float snow_noise = noise.m_mountainHeightMap.noise2D({x, z});
            int upper_bound = H;
            if(biome == 0) {
                upper_bound = std::max(H, waterH);
            }
            for(int y = 0; y <= upper_bound; y++) {
                // Carve out the caves
                float caveNoiseVal = cavePerlinNoise3D(glm::vec3(i/25.f, y/16.f, j/25.f))/2 + 0.5; // output range [-1, 1] mapped to [0, 1]
                float caveMask = cavePerlinNoise3D(glm::vec3(j/100.f, i/100.f, y/100.f))/2 + 0.5; // similar to previous but rotate
                chunk->setBlockAt(coord_x, y, coord_z, noise.getBlockType(y, H, biome, snow_noise, caveNoiseVal, caveMask));                        
            }
        }
    }
}

// uncomment for no caves
//void Terrain::setChunkBlocks(Chunk* chunk, int x, int z) {
//    int waterH = 138;       // Height of water level
//    for(int i = x; i < x + 16; ++i) {
//        for(int j = z; j < z + 16; ++j) {

//            // Get height and biome as pair from terrain
//            auto HB = noise.computeHeight(i, j);
//            int H = HB.first;
//            int biome = HB.second;

//            glm::vec2 chunkOrigin = glm::vec2(floor(i / 16.f) * 16, floor(j / 16.f) * 16);
//            int coord_x = int(i - chunkOrigin.x), coord_z = int(j - chunkOrigin.y);
//            float snow_noise = noise.m_mountainHeightMap.noise2D({x, z});
//            int upper_bound = H;

//            if(biome == 0)
//                upper_bound = std::max(H, waterH);

//            for(int y = 0; y <= upper_bound; y++)
//                chunk->setBlockAt(coord_x, y, coord_z, noise.getBlockType(y, H, biome, snow_noise));
//        }
//    }
//}

void Terrain::multithreadedWork(glm::vec3 playerPos, glm::vec3 playerPosPrev, float dT)
{
    m_tryExpansionTimer += dT;
    if(m_tryExpansionTimer < 0.5f)
        return;
    tryExpansion(playerPos, playerPosPrev);
    checkThreadResults();
    m_tryExpansionTimer = 0.f;
}

std::unordered_set<int64_t> Terrain::terrainZonesBorderingZone(glm::ivec2 zone_position, int num_zones)
{
    std::unordered_set<int64_t> terrainZonesBorderingPosition;
    int half_length = 64.f * glm::floor(num_zones/2.f);
    for(int i = zone_position.x - half_length; i <= zone_position.x + half_length; i += 64)
    {
        for(int j = zone_position.y - half_length; j <= zone_position.y + half_length; j += 64)
        {
            terrainZonesBorderingPosition.insert(toKey(i, j));
        }
    }
    return terrainZonesBorderingPosition;
}

bool Terrain::terrainZoneExists(int64_t key)
{
    return m_generatedTerrain.find(key) != m_generatedTerrain.end();
}

bool Terrain::terrainZoneExists(int x, int z)
{
    return m_generatedTerrain.find(toKey(x, z)) != m_generatedTerrain.end();
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
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
    return cPtr;
}

void Terrain::VBOWorker(Chunk* chunk)
{
    chunk->generateVBOdata();
    m_chunksThatHaveVBODataLock.lock();
    m_chunksThatHaveVBOData.insert(chunk);
    m_chunksThatHaveVBODataLock.unlock();

}

void Terrain::blockTypeWorker(Chunk* chunk)
{
    int coord_x = chunk->getCorners().x, coord_z = chunk->getCorners().y;
    setChunkBlocks(chunk, coord_x, coord_z);
    m_chunksThatHaveBlockDataLock.lock();
    m_chunksThatHaveBlockData.insert(chunk);
    m_chunksThatHaveBlockDataLock.unlock();
}


void Terrain::tryExpansion(glm::vec3 playerPos, glm::vec3 playerPosPrev)
{
    // Find the player's position relative to their current terrain generation zone
    glm::ivec2 currZone = glm::ivec2(64.f *glm::floor(playerPos.x / 64.f), 64.f * glm::floor(playerPos.z / 64.f));
    glm::ivec2 prevZone = glm::ivec2(64.f * glm::floor(playerPosPrev.x / 64.f), 64.f * glm::floor(playerPosPrev.z / 64.f));
    // Determine which terrain zones border our current and previous position
    // This will include out ungenerated terrain zones
    std::unordered_set<int64_t> terrainZonesBorderingCurrPos = terrainZonesBorderingZone(currZone, 5);
    std::unordered_set<int64_t> terrainZonesBorderingPrevPos = terrainZonesBorderingZone(prevZone, 5);
    // Check which terrain zones need to be destroyed by determining which terrain zones were previously in our radius
    // and are now not within radius
    for(auto id : terrainZonesBorderingPrevPos)
    {
        if(terrainZonesBorderingCurrPos.find(id) == terrainZonesBorderingCurrPos.end())
        {
            glm::ivec2 coord = toCoords(id);
            for(int x = coord.x; x < coord.x + 64; x += 16)
            {
                for(int z = coord.y; z < coord.y + 64; z += 16)
                {
                    auto &chunk = getChunkAt(x, z);
                    chunk->isVBOready = false;
                    chunk->destroyVBOdata();
                }
            }
        }
    }
    // Determine if any terrain zones around our current position need VBO data
    // Send these to VBOWorkers
    // Do not send zones to workers if they do not exist in our global map. Instead, we send these to BlockTypeWorker
    for(auto id : terrainZonesBorderingCurrPos)
    {
        glm::ivec2 coord = toCoords(id);
        // If it exists already and is not in prev set, send it to VBOWorker
        // If it's in the prev set, then it's already been sent to a VBOWorker at some point, and may even already have VBOs
        if(terrainZoneExists(id))
        {
            if(terrainZonesBorderingPrevPos.find(id) == terrainZonesBorderingPrevPos.end())
            {
                for(int x = coord.x; x < coord.x + 64; x += 16)
                {
                    for(int z = coord.y; z < coord.y + 64; z += 16)
                    {
                        auto &chunk = getChunkAt(x, z);
                        std::thread t(&Terrain::VBOWorker, this, chunk.get());
                        t.detach();
                    }
                }
            }
        }
        else
        {
            // If it doesn't exist yet, send it to a BlockTypeWorker. This also adds it to the set of generated terrain
            // zones so that we don't try to repeatedly generate it
            m_generatedTerrain.insert(id);
            for(int x = coord.x; x < coord.x + 64; x += 16)
            {
                for(int z = coord.y; z < coord.y + 64; z += 16)
                {
                    instantiateChunkAt(x, z);
                    auto &chunk = getChunkAt(x, z);
                    std::thread t(&Terrain::blockTypeWorker, this, chunk.get());
                    t.detach();
                }
            }

        }
    }
}

void Terrain::checkThreadResults()
{
    // Send chunks that have been processed by BlockTypeWorkers to VBOWorkers for VBO data
    m_chunksThatHaveBlockDataLock.lock();
    for(auto &chunk: m_chunksThatHaveBlockData)
    {
        std::thread t(&Terrain::VBOWorker, this, chunk);
        t.detach();
    }
    m_chunksThatHaveBlockData.clear();
    m_chunksThatHaveBlockDataLock.unlock();
    m_chunksThatHaveVBODataLock.lock();
    for(auto &chunk: m_chunksThatHaveVBOData)
    {
        chunk->loadVBOdata();
    }
    m_chunksThatHaveVBOData.clear();
    m_chunksThatHaveVBODataLock.unlock();
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
                if(chunk->isVBOready)
                {
                    shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, 0, z)));
                    shaderProgram->drawInterleaved(*chunk);
                }
            }
        }
    }
}


void Terrain::CreateTestScene()
{

    int y_offset = 139;
    int x_offset = -32; // must be divisible by 16
    int z_offset = -32; // must be divisible by 16

    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = x_offset; x < x_offset+64; x += 16) {
        for(int z = z_offset; z < z_offset+64; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(x_offset, z_offset));

    // Create the basic terrain floor
    for(int x = x_offset; x < x_offset+64; ++x) {
        for(int z = z_offset; z < z_offset+64; ++z) {
            if((x + z) % 2 == 0) {
                setBlockAt(x, y_offset, z, STONE);
            }
            else {
                setBlockAt(x, y_offset, z, DIRT);
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = x_offset; x < x_offset+64; ++x) {
        setBlockAt(x, y_offset+1, z_offset, GRASS);
        setBlockAt(x, y_offset+2, z_offset, GRASS);
        setBlockAt(x, y_offset+1, z_offset+63, GRASS);
        setBlockAt(x_offset, y_offset+2, x, GRASS);
    }
    // Add a central column
    for(int y = y_offset+1; y < y_offset+11; ++y) {
        setBlockAt(x_offset+32, y, z_offset+32, GRASS);
    }
    for(int x = x_offset; x < x_offset+64; x += 16) {
        for(int z = z_offset; z < z_offset+64; z += 16) {
            const uPtr<Chunk> &chunk = getChunkAt(x, z);
            chunk->destroyVBOdata();
            chunk->createVBOdata();
        }
    }
}

void Terrain::CreateTestTerrainScene()
{
    // instantiates chunks of the world immediately around
    // player spawn point

    int range = 64; //normally 32
    for(int x = -range; x < range; x += 16) {
        for(int z = -range; z < range; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    m_generatedTerrain.insert(toKey(0, 0));
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
        if(!hasChunkAt(new_pos[0], new_pos[1]))
        {
            instantiateChunkAt(new_pos[0], new_pos[1]);
        }
    }
}
