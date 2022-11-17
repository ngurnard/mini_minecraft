#include "cave.h"

glm::vec3 caveRand3( glm::vec3 p ) {
    return glm::fract( glm::sin( glm::vec3(glm::dot(p, glm::vec3(127.1, 311.7, 58.24)),
                                           glm::dot(p, glm::vec3(269.5,183.3, 657.3)),
                                           glm::dot(p, glm::vec3(420.69, 69.420, 469.20))) ) * 43758.5453f);
}

float caveSurflet3D(glm::vec3 P, glm::vec3 gridPoint) {
    // Compute falloff function by converting linear distance to a polynomial
    float distX = glm::abs(P.x - gridPoint.x);
    float distY = glm::abs(P.y - gridPoint.y);
    float distZ = glm::abs(P.z - gridPoint.z);
    float tX = 1 - 6 * glm::pow(distX, 5.f) + 15 * glm::pow(distX, 4.f) - 10 * glm::pow(distX, 3.f);
    float tY = 1 - 6 * glm::pow(distY, 5.f) + 15 * glm::pow(distY, 4.f) - 10 * glm::pow(distY, 3.f);
    float tZ = 1 - 6 * glm::pow(distZ, 5.f) + 15 * glm::pow(distZ, 4.f) - 10 * glm::pow(distZ, 3.f);
    // Get the random vector for the grid point
    glm::vec3 gradient = 2.f * caveRand3(gridPoint) - glm::vec3(1.f);
    // Get the vector from the grid point to P
    glm::vec3 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY * tZ;
}

float cavePerlinNoise3D(glm::vec3 uvw) {
    float surfletSum = 0.f;
    // Iterate over the eight integer corners surrounding uv
    for(int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            for(int dz = 0; dz <= 1; ++dz) {
                surfletSum += caveSurflet3D(uvw, glm::floor(uvw) + glm::vec3(dx, dy, dz));
            }
        }
    }
    return surfletSum;
}
