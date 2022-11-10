#pragma once
#include "glm_includes.h"
#include <iostream>

class customFBM
{
private:

    float noise2D(glm::vec2) const;
    float interpNoise2D(float, float) const;

    int octaves;
    float freq;
    float amp;
    float persistance;
    glm::vec2 range;

public:
    customFBM();
    customFBM(int oct, float f, float A, float p);
    customFBM(int oct, float f, float A, float p, glm::vec2 range);

    void mapOutput2Range(float&) const;
    float computeFBM(int x, int z) const;
};
