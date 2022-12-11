#pragma once

#include "drawable.h"
#include "scene/block.h"
#include <glm_includes.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <vector>
#include <unordered_map>

#define BLK_UVX * 0.125
#define BLK_UVY * 0.125
#define BLK_UV  0.125
static const int CUB_IDX_COUNT = 36;
static const int CUB_VERT_COUNT = 24;

 //ZPOS, XPOS, XNEG, ZNEG, YPOS, YNEG
static std::unordered_map<BlockTypeNPC, std::vector<glm::vec2>> bodyPartUVs{
    {TORSO, {
            glm::vec2(4.f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(2.f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(3.5f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(2.5f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(2.5f BLK_UVX, 5.5f BLK_UVY),
            glm::vec2(3.5f BLK_UVX, 5.5f BLK_UVY),
            }},
    {HEAD, {
            glm::vec2(3.f BLK_UVX, 6.f BLK_UVY),
            glm::vec2(0.f BLK_UVX, 6.f BLK_UVY),
            glm::vec2(2.f BLK_UVX, 6.f BLK_UVY),
            glm::vec2(1.f BLK_UVX, 6.f BLK_UVY),
            glm::vec2(1.f BLK_UVX, 7.f BLK_UVY),
            glm::vec2(2.f BLK_UVX, 7.f BLK_UVY),
            }},
    {LEFT_HAND, {
            glm::vec2(6.5f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(5.f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(6.f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(5.5f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(5.5f BLK_UVX, 5.5f BLK_UVY),
            glm::vec2(6.f BLK_UVX, 5.5f BLK_UVY),
            }},
    {RIGHT_HAND, {
            glm::vec2(5.5f BLK_UVX, 0.f BLK_UVY),
            glm::vec2(4.f BLK_UVX, 0.f BLK_UVY),
            glm::vec2(5.f BLK_UVX, 0.f BLK_UVY),
            glm::vec2(4.5f BLK_UVX, 0.f BLK_UVY),
            glm::vec2(4.5f BLK_UVX, 1.5f BLK_UVY),
            glm::vec2(5.f BLK_UVX, 1.5f BLK_UVY),
            }},
    {LEFT_LEG, {
            glm::vec2(1.5f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(0.f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(1.f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(0.5f BLK_UVX, 4.f BLK_UVY),
            glm::vec2(0.5f BLK_UVX, 5.5f BLK_UVY),
            glm::vec2(1.f BLK_UVX, 5.5f BLK_UVY),
            }},
    {RIGHT_LEG, {
            glm::vec2(3.5f BLK_UVX, 0.f BLK_UVY),
            glm::vec2(2.f BLK_UVX, 0.f BLK_UVY),
            glm::vec2(3.f BLK_UVX, 0.f BLK_UVY),
            glm::vec2(2.5f BLK_UVX, 0.f BLK_UVY),
            glm::vec2(2.5f BLK_UVX, 1.5f BLK_UVY),
            glm::vec2(3.f BLK_UVX, 1.5f BLK_UVY),
            }},
};
static std::unordered_map<BlockTypeNPC, std::vector<glm::vec2>> bodyPartUVOffsets{
    {TORSO, {
            glm::vec2(1.f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(1.f, 1.5f),
            glm::vec2(1.f, 0.5f),
            glm::vec2(1.f, 0.5f),
            }},
    {HEAD, {
            glm::vec2(1.f, 1.f),
            glm::vec2(1.f, 1.f),
            glm::vec2(1.f, 1.f),
            glm::vec2(1.f, 1.f),
            glm::vec2(1.f, 1.f),
            glm::vec2(1.f, 1.f),
            }},
    {LEFT_HAND, {
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 0.5f),
            glm::vec2(0.5f, 0.5f),
            }},
    {RIGHT_HAND, {
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 0.5f),
            glm::vec2(0.5f, 0.5f),
            }},
    {LEFT_LEG, {
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 0.5f),
            glm::vec2(0.5f, 0.5f),
            }},
    {RIGHT_LEG, {
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 1.5f),
            glm::vec2(0.5f, 0.5f),
            glm::vec2(0.5f, 0.5f),
            }},
};
static std::vector<glm::vec2> faceCorners{
    glm::vec2(BLK_UV, BLK_UV),
    glm::vec2(BLK_UV, 0),
    glm::vec2(0, 0),
    glm::vec2(0, BLK_UV),
};

class Cube : public Drawable
{
public:
    Cube(OpenGLContext* context) : Drawable(context){}
    BlockTypeNPC NPCbodyPart;
    virtual ~Cube(){}
    void createVBOdata() override;
    void createCubeUVs(glm::vec2 (&cub_vert_uv)[CUB_VERT_COUNT]);
};
