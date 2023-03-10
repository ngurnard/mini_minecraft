#pragma once
#include "scene/noise.h"
#include "openglcontext.h"
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include <set>
#include <unordered_set>
#include "drawable.h"
using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

static unordered_map<BlockType, int, EnumHash> transparent_block_order {
    //Set priority for how to draw transparent blocks (if they are in-contact)

    {EMPTY, 0}, {WATER, 1}, {ICE, 2}, {LAVA, 3}
};

const static unordered_set<BlockType, EnumHash> transparent_blocks{
    EMPTY, WATER, ICE, LAVA
};

const static unordered_set<BlockType, EnumHash> animatable_blocks{
    WATER, LAVA
};

glm::vec4 getColor(BlockType type);

// Strucutres based on class lectures
struct VertexData
{
    glm::vec4 pos;
    // Relative UV coords to be offset based on BlockType
    glm::vec2 uv;
    /*
    Absolute uv info is obtained from blockFaceUVs
    Surface normal is based on directionVec in BlockFace
    cosine power and animFlag are dependent on BlockType
    */
    VertexData(glm::vec4 pos, glm::vec2 uv) : pos(pos), uv(uv)
    {}
};

struct BlockFace
{
    Direction direction;
    glm::vec3 directionVec;
    array<VertexData, 4> vertices;
    BlockFace(Direction direction, glm::vec3 directionVec, const VertexData &v1, const VertexData &v2, const VertexData &v3, const VertexData &v4)
        : direction(direction), directionVec(directionVec), vertices{v1, v2, v3, v4}
    {}
};

#define BLK_UVX * 0.03125f * 2
#define BLK_UVY * 0.03125f * 2
#define BLK_UV  0.03125f * 2

// Iterate over this in Chunk::create() to check each block adjacent to block [x][y][z] get the relevant vertex info
const static array<BlockFace, 6> adjacentFaces
{
    // +X
    BlockFace(XPOS,
              glm::vec3(1, 0, 0),
              VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(0, BLK_UV))
              ),
    // -X
    BlockFace(XNEG,
              glm::vec3(-1, 0, 0),
              VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(0, BLK_UV))
              ),
    // +Y
    BlockFace(YPOS,
              glm::vec3(0, 1, 0),
              VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(0, BLK_UV))
              ),
    // -Y
    BlockFace(YNEG,
              glm::vec3(0, -1, 0),
              VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(0, BLK_UV))
              ),
    // +Z
    BlockFace(ZPOS,
              glm::vec3(0, 0, 1),
              VertexData(glm::vec4(0, 0, 1, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(1, 0, 1, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(1, 1, 1, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(0, 1, 1, 1), glm::vec2(0, BLK_UV))
              ),
    // -Z
    BlockFace(ZNEG,
              glm::vec3(0, 0, -1),
              VertexData(glm::vec4(1, 0, 0, 1), glm::vec2(0, 0)),
              VertexData(glm::vec4(0, 0, 0, 1), glm::vec2(BLK_UV, 0)),
              VertexData(glm::vec4(0, 1, 0, 1), glm::vec2(BLK_UV, BLK_UV)),
              VertexData(glm::vec4(1, 1, 0, 1), glm::vec2(0, BLK_UV))
              )
};

static std::unordered_map<BlockType, std::unordered_map<Direction, glm::vec2, EnumHash>, EnumHash> blockFaceUVs {
    {GRASS, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                               {XNEG, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                               {YPOS, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                               {YNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {ZPOS, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                               {ZNEG, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)}}},
    {DIRT, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                              {XNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                              {YPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                              {YNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)}}},
    {STONE, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(1.f BLK_UVX, 15.f  BLK_UVY)},
                                                               {XNEG, glm::vec2(1.f BLK_UVX, 15.f  BLK_UVY)},
                                                               {YPOS, glm::vec2(1.f BLK_UVX, 15.f  BLK_UVY)},
                                                               {YNEG, glm::vec2(1.f BLK_UVX, 15.f  BLK_UVY)},
                                                               {ZPOS, glm::vec2(1.f BLK_UVX, 15.f  BLK_UVY)},
                                                               {ZNEG, glm::vec2(1.f BLK_UVX, 15.f  BLK_UVY)}}},
    {WATER, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(15.f BLK_UVX, 3.f  BLK_UVY)},
                                                              {XNEG, glm::vec2(15.f BLK_UVX, 3.f  BLK_UVY)},
                                                              {YPOS, glm::vec2(15.f BLK_UVX, 3.f  BLK_UVY)},
                                                              {YNEG, glm::vec2(15.f BLK_UVX, 3.f  BLK_UVY)},
                                                              {ZPOS, glm::vec2(15.f BLK_UVX, 3.f  BLK_UVY)},
                                                              {ZNEG, glm::vec2(15.f BLK_UVX, 3.f  BLK_UVY)}}},
    {LAVA, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(14.5f BLK_UVX, 0.5f  BLK_UVY)},
                                                              {XNEG, glm::vec2(14.5f BLK_UVX, 0.5f  BLK_UVY)},
                                                              {YPOS, glm::vec2(14.5f BLK_UVX, 0.5f  BLK_UVY)},
                                                              {YNEG, glm::vec2(14.5f BLK_UVX, 0.5f  BLK_UVY)},
                                                              {ZPOS, glm::vec2(14.5f BLK_UVX, 0.5f  BLK_UVY)},
                                                              {ZNEG, glm::vec2(14.5f BLK_UVX, 0.5f  BLK_UVY)}}},
    {ICE, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(3.f BLK_UVX, 11.f  BLK_UVY)},
                                                              {XNEG, glm::vec2(3.f BLK_UVX, 11.f  BLK_UVY)},
                                                              {YPOS, glm::vec2(3.f BLK_UVX, 11.f  BLK_UVY)},
                                                              {YNEG, glm::vec2(3.f BLK_UVX, 11.f  BLK_UVY)},
                                                              {ZPOS, glm::vec2(3.f BLK_UVX, 11.f  BLK_UVY)},
                                                              {ZNEG, glm::vec2(3.f BLK_UVX, 11.f  BLK_UVY)}}},
    {SAND, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(2.f BLK_UVX, 14.f  BLK_UVY)},
                                                              {XNEG, glm::vec2(2.f BLK_UVX, 14.f  BLK_UVY)},
                                                              {YPOS, glm::vec2(2.f BLK_UVX, 14.f  BLK_UVY)},
                                                              {YNEG, glm::vec2(2.f BLK_UVX, 14.f  BLK_UVY)},
                                                              {ZPOS, glm::vec2(2.f BLK_UVX, 14.f  BLK_UVY)},
                                                              {ZNEG, glm::vec2(2.f BLK_UVX, 14.f  BLK_UVY)}}},
    {SNOW, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(2.f BLK_UVX, 11.f  BLK_UVY)},
                                                              {XNEG, glm::vec2(2.f BLK_UVX, 11.f  BLK_UVY)},
                                                              {YPOS, glm::vec2(2.f BLK_UVX, 11.f  BLK_UVY)},
                                                              {YNEG, glm::vec2(2.f BLK_UVX, 11.f  BLK_UVY)},
                                                              {ZPOS, glm::vec2(2.f BLK_UVX, 11.f  BLK_UVY)},
                                                              {ZNEG, glm::vec2(2.f BLK_UVX, 11.f  BLK_UVY)}}},
    {BEDROCK, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(1.f BLK_UVX, 14.f  BLK_UVY)},
                                                              {XNEG, glm::vec2(1.f BLK_UVX, 14.f  BLK_UVY)},
                                                              {YPOS, glm::vec2(1.f BLK_UVX, 14.f  BLK_UVY)},
                                                              {YNEG, glm::vec2(1.f BLK_UVX, 14.f  BLK_UVY)},
                                                              {ZPOS, glm::vec2(1.f BLK_UVX, 14.f  BLK_UVY)},
                                                              {ZNEG, glm::vec2(1.f BLK_UVX, 14.f  BLK_UVY)}}},
    {WOOD, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(6.f BLK_UVX, 10.f BLK_UVY)},
                                                              {XNEG, glm::vec2(6.f BLK_UVX, 10.f BLK_UVY)},
                                                              {YPOS, glm::vec2(6.f BLK_UVX, 10.f BLK_UVY)},
                                                              {YNEG, glm::vec2(6.f BLK_UVX, 10.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(6.f BLK_UVX, 10.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(6.f BLK_UVX, 10.f BLK_UVY)}}},

    {LEAF, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)},
                                                              {XNEG, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)},
                                                              {YPOS, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)},
                                                              {YNEG, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(4.f BLK_UVX, 12.f BLK_UVY)}}}

};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk : public Drawable{
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    int m_xCorner, m_zCorner;
    Noise* noise;

    vector<GLuint> indicesOpq;
    vector<GLuint> indicesTra;
    vector<glm::vec4> interleavedOpq;
    vector<glm::vec4> interleavedTra;

public:
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;
    bool isVBOready;
    bool isBlocksSet;
    Chunk(OpenGLContext* context, int m_xCorner, int m_zCorner, Noise* N);
    glm::ivec2 getCorners();
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z);
    BlockType getBlockAt(int x, int y, int z);
    BlockType getWorldBlock(int x, int y, int z);
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    void recreateVBOdata();
    void virtual createVBOdata() override;
    void generateVBOdata();
    void loadVBOdata();
    GLenum drawMode() override;
    ~Chunk();
};
