#pragma once
#include "scene/staticcloud.h"
#include "scene/cloud.h"
#include "scene/deltariver.h"
#include "scene/noise.h"
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "chunk.h"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include "surfaceshader.h"
#include <thread>
#include <mutex>
//using namespace std;

// Helper functions to convert (x, z) to and from hash map key
int64_t toKey(int x, int z);
glm::ivec2 toCoords(int64_t k);

// The container class for all of the Chunks in the game.
// Ultimately, while Terrain will always store all Chunks,
// not all Chunks will be drawn at any given time as the world
// expands.
class Terrain {
private:
    // Stores every Chunk according to the location of its lower-left corner
    // in world space.
    // We combine the X and Z coordinates of the Chunk's corner into one 64-bit int
    // so that we can use them as a key for the map, as objects like std::pairs or
    // glm::ivec2s are not hashable by default, so they cannot be used as keys.
    std::unordered_map<int64_t, uPtr<Chunk>> m_chunks;
    std::unordered_set<Chunk*> m_chunksThatHaveBlockData;
    std::mutex m_chunksThatHaveBlockDataLock;
    std::unordered_set<Chunk*> m_chunksThatHaveVBOData;
    std::mutex m_chunksThatHaveVBODataLock;

    bool m_permit_transparent_terrain;
    bool m_permit_caves;
    bool m_permit_lrivers;
    bool m_permit_clouds;

    float m_tryExpansionTimer;
    // We will designate every 64 x 64 area of the world's x-z plane
    // as one "terrain generation zone". Every time the player moves
    // near a portion of the world that has not yet been generated
    // (i.e. its lower-left coordinates are not in this set), a new
    // 4 x 4 collection of Chunks is created to represent that area
    // of the world.
    // The world that exists when the base code is run consists of exactly
    // one 64 x 64 area with its lower-left corner at (0, 0).
    // When milestone 1 has been implemented, the Player can move around the
    // world to add more "terrain generation zone" IDs to this set.
    // While only the 3 x 3 collection of terrain generation zones
    // surrounding the Player should be rendered, the Chunks
    // in the Terrain will never be deleted until the program is terminated.
    std::unordered_set<int64_t> m_generatedTerrain;

    Noise noise;

    Cloud m_cloud;
    // TODO: DELETE ALL REFERENCES TO m_geomCube AS YOU WILL NOT USE
    // IT IN YOUR FINAL PROGRAM!
    // The instance of a unit cube we can use to render any cube.
    // Presently, Terrain::draw renders one instance of this cube
    // for every non-EMPTY block within its Chunks. This is horribly
    // inefficient, and will cause your game to run very slowly until
    // milestone 1's Chunk VBO setup is completed.
//    Cube m_geomCube;

    OpenGLContext* mp_context;

    float m_time;
    int num_delta_iterations = 6;
    DeltaRiver delta = DeltaRiver(glm::vec2(-60.f, 0.f), glm::vec2(0.5f, 1.0f));
public:
    Terrain(OpenGLContext *context);
    ~Terrain();

    void allowTransparent(bool);
    void allowCaves(bool);
    void allowRivers(bool);
    void allowClouds(bool);

    void printHeight(int x, int z);

    // Instantiates a new Chunk and stores it in
    // our chunk map at the given coordinates.
    // Returns a pointer to the created Chunk.
    Chunk* instantiateChunkAt(int x, int z);

    void setChunkBlocks(Chunk* chunk, int x, int z);
    // Do these world-space coordinates lie within
    // a Chunk that exists?
    bool hasChunkAt(int x, int z) const;
    // Assuming a Chunk exists at these coords,
    // return a mutable reference to it
    uPtr<Chunk>& getChunkAt(int x, int z);
    // Assuming a Chunk exists at these coords,
    // return a const reference to it
    const uPtr<Chunk>& getChunkAt(int x, int z) const;
    // Given a world-space coordinate (which may have negative
    // values) return the block stored at that point in space.
    BlockType getBlockAt(int x, int y, int z) const;
    BlockType getBlockAt(glm::vec3 p) const;
    // Given a world-space coordinate (which may have negative
    // values) set the block at that point in space to the
    // given type.
    void setBlockAt(int x, int y, int z, BlockType t);

    // Draws every Chunk that falls within the bounding box
    // described by the min and max coords, using the provided
    // ShaderProgram    
    void drawTransparentOrOpaque(int minX, int maxX, int minZ, int maxZ, SurfaceShader *shader, bool opaque);
    void draw(int minX, int maxX, int minZ, int maxZ, SurfaceShader *shader);

    // Cloud stuff
    void drawClouds(int minX, int maxX, int minZ, int maxZ, SurfaceShader *shader);
    void createCloud();

    // Initializes the Chunks that store the 64 x 256 x 64 block scene you
    // see when the base code is run.
    void CreateTestScene();
    void CreateTestTerrainScene();
    /*
    A function in the Terrain class that checks whether a new Chunk should be
    added to the Terrain based on the Player's proximity to the edge of a Chunk
    without a neighbor in a particular direction.
    */
    void updateTerrain(const glm::vec3 &player_pos);
    void multithreadedWork(glm::vec3 playerPos, glm::vec3 playerPosPrev, float dT);

    void destroyOutOfRangeTerrainZoneVBOs(std::unordered_set<int64_t>& terrainZonesBorderingCurrPos,
                                          std::unordered_set<int64_t>& terrainZonesBorderingPrevPos);

    void tryExpansion(glm::vec3 playerPos, glm::vec3 playerPosPrev);
    void checkThreadResults();
    std::unordered_set<int64_t> terrainZonesBorderingZone(glm::ivec2 zone_position, int num_zones);
    bool terrainZoneExists(int64_t key);
    bool terrainZoneExists(int x, int z);
    void VBOWorker(Chunk* chunk);
    void blockTypeWorker(vector<Chunk*> new_chunks);
    void updateNeighbors(int x, int z);
};
