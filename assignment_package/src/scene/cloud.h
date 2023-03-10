#pragma once

#include "drawable.h"
#include "scene/noise.h"
#include <glm_includes.h>

class Cloud: public Drawable
{
private:
    int m_height;        // Cloud altitude
    glm::ivec4 m_bounds; // [xmin, zmin, xmax, zmax]

public:
    Cloud(OpenGLContext* context);
    virtual ~Cloud() override;
    void createVBOdata() override;
    GLenum drawMode() override;

    void setHeight(int h);
    void setBounds(int xmin, int xmax, int zmin, int zmax);
};

