#pragma once
#include "glm_includes.h"
#include <iostream>

glm::vec2 caveRand2( glm::vec2 p );
glm::vec3 caveRand3( glm::vec3 p );
float caveSurflet3D(glm::vec3 P, glm::vec3 gridPoint);
float cavePerlinNoise3D(glm::vec3 uvw);
float interpNoise3D(float x, float y, float z);
float computeFBM3D(const int x, const int y, const int z);

