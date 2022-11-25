#include "blockwireframe.h"

BlockWireframe::~BlockWireframe()
{}

void BlockWireframe::createVBOdata()
{
    int x = blockpos.x;
    int y = blockpos.y;
    int z = blockpos.z;

    GLuint idx[24] = {0, 1, 1, 2, 2, 3, 3, 0,
                      0, 4, 1, 5, 2, 6, 3, 7,
                      4, 5, 5, 6, 6, 7, 7, 4};

    glm::vec4 pos[8] = {glm::vec4(    x,  y,      z,  1),
                        glm::vec4(x + 1,  y,      z,  1),
                        glm::vec4(x + 1,  y,  z + 1,  1),
                        glm::vec4(    x,  y,  z + 1,  1),

                        glm::vec4(    x,  y + 1,      z,  1),
                        glm::vec4(x + 1,  y + 1,      z,  1),
                        glm::vec4(x + 1,  y + 1,  z + 1,  1),
                        glm::vec4(    x,  y + 1,  z + 1,  1)};

    glm::vec4 col[8] = {glm::vec4(1,1,1,1), glm::vec4(1,1,1,1), glm::vec4(1,1,1,1), glm::vec4(1,1,1,1),
                        glm::vec4(1,1,1,1), glm::vec4(1,1,1,1), glm::vec4(1,1,1,1), glm::vec4(1,1,1,1)};

    m_countOpq = 24;

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpq);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLuint), idx, GL_STATIC_DRAW);
    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec4), pos, GL_STATIC_DRAW);
    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec4), col, GL_STATIC_DRAW);
}

void BlockWireframe::update(glm::ivec3 newpos)
{
    setpos(newpos);
    destroyVBOdata();
    createVBOdata();
}

glm::ivec3 BlockWireframe::getpos()
{
    return blockpos;
}

void BlockWireframe::setpos(glm::ivec3 newpos)
{
    this->blockpos = newpos;
}

GLenum BlockWireframe::drawMode()
{
    return GL_LINES;
}
