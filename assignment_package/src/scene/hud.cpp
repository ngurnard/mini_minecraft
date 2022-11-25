#include "hud.h"

HUD::~HUD()
{}

void HUD::createVBOdata()
{
    float crossHalfWidth = 30.f; // width of crosshair

    GLuint idx[4] = {0, 1, 2, 3};
    glm::vec4 pos[4] = {glm::vec4(-crossHalfWidth, 0.f, 0.9f, 1), glm::vec4(crossHalfWidth, 0.f, 0.9f, 1),
                        glm::vec4(0.f, -crossHalfWidth, 0.9f, 1), glm::vec4(0.f, crossHalfWidth, 0.9f, 1)};

    glm::vec4 col[4] = {glm::vec4(1,1,1,1), glm::vec4(1,1,1,1),
                        glm::vec4(1,1,1,1), glm::vec4(1,1,1,1)};

    glm::vec2 UV[4] {glm::vec2(-crossHalfWidth, 0.f),
                          glm::vec2(crossHalfWidth, 0.f),
                          glm::vec2(0.f, -crossHalfWidth),
                          glm::vec2(0.f, crossHalfWidth)};
    m_countOpq = 4;

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpq);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), idx, GL_STATIC_DRAW);
    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), pos, GL_STATIC_DRAW);
    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), col, GL_STATIC_DRAW);
    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), UV, GL_STATIC_DRAW);
}

GLenum HUD::drawMode()
{
    return GL_LINES;
}
