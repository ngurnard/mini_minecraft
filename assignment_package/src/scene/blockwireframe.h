#pragma once

#include "drawable.h"
#include <glm_includes.h>

class BlockWireframe : public Drawable
{
private:
    glm::ivec3 blockpos;
public:
    BlockWireframe(OpenGLContext* context) : Drawable(context){}
    virtual ~BlockWireframe() override;
    void createVBOdata() override;
    GLenum drawMode() override;

    void update(glm::ivec3 newpos);
    glm::ivec3 getpos();
    void setpos(glm::ivec3 newpos);
};
