#pragma once
#include "glm_includes.h"
#include <iostream>

class customFBM
{
private:


    float interpNoise2D(float, float) const;
    float interpNoise3D(float x, float y, float z) const;

    int octaves;
    float freq;
    float amp;
    float persistance;
    glm::vec2 range;

public:
    customFBM();
    customFBM(int oct, float f, float A, float p);
    customFBM(int oct, float f, float A, float p, glm::vec2 range);

    float noise2D(glm::vec2) const;
    void mapOutput2Range(float&) const;
    float computeFBM(int x, int z) const;

    float noise3D(glm::vec3 p) const;
    float computeFBM(int x, int y, int z) const;
};
