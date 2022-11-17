#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"

glm::vec3 caveRand3( glm::vec3 p );
float caveSurflet3D(glm::vec3 P, glm::vec3 gridPoint);
float cavePerlinNoise3D(glm::vec3 uvw);


