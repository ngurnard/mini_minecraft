#pragma once

#include "drawable.h"
#include <glm_includes.h>

class HUD : public Drawable
{
public:
    HUD(OpenGLContext* context) : Drawable(context){}
    virtual ~HUD() override;
    void createVBOdata() override;
    GLenum drawMode() override;
};
