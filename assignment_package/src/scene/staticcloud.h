#pragma once

#include "customfbm.h"
#include "drawable.h"
#include "scene/noise.h"
#include <glm_includes.h>

class StaticCloud: public Drawable
{
private:
    int m_height;        // Cloud altitude
    int m_threshold;     // Cutoff value for cloud boundary
    glm::ivec4 m_bounds; // [xmin, zmin, xmax, zmax]
    glm::ivec2 m_offset; // integer offset for cloud comp
    Noise* noise;

public:
    StaticCloud(OpenGLContext* context, Noise* N);
    virtual ~StaticCloud() override;
    void createVBOdata() override;
    GLenum drawMode() override;

    void setHeight(int h);
    void setThreshold(int t);
    void setBounds(int xmin, int xmax, int zmin, int zmax);
    void incrementOffset(glm::ivec2 increment);
};

