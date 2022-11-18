#include "cave.h"

glm::vec2 caveRand2( glm::vec2 p ) {
    return glm::fract( glm::sin( glm::vec2(glm::dot(p, glm::vec2(120.1, 304.7)),
                            glm::dot(p, glm::vec2(248.5,183.3))) ) * 42389.5453f);
}

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
    // Get the random vector for the grid point (normalized)
    glm::vec3 gradient = glm::normalize(2.f * caveRand3(gridPoint) - glm::vec3(1.f));
//    gradient.y *= caveRand2(glm::vec2(P.x, P.y)).x / caveRand2(glm::vec2(P.y, P.z)).y; // make it so caves sprawl horizontally more
//    gradient.y *= 20; // make caves sprawl horizontally more
//    gradient.y *= glm::floor(5 * caveRand3(P).x);
    // Get the vector from the grid point to P (non-normalized)
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
    return surfletSum; // output range [-1, 1]
}

float noise3D(glm::vec3 p)
{
    return glm::fract(sin(glm::dot(p, glm::vec3(139.2, 269.1, 189.6))) * 45858.5453);
}

float interpNoise3D(float x, float y, float z)
{
    int intX = int(floor(x));
    float fX = glm::fract(x);
    fX = fX * fX * (3 - 2 * fX);

    int intY = int(floor(y));
    float fY = glm::fract(y);
    fY = fY * fY * (3 - 2 * fY);

    int intZ = int(floor(z));
    float fZ = glm::fract(z);
    fZ = fZ * fZ * (3 - 2 * fZ);

    float v1 = noise3D(glm::vec3(intX, intY, intZ));
    float v2 = noise3D(glm::vec3(intX+1, intY, intZ));
    float v3 = noise3D(glm::vec3(intX, intY, intZ+1));
    float v4 = noise3D(glm::vec3(intX+1, intY, intZ+1));

    float v5 = noise3D(glm::vec3(intX, intY+1, intZ));
    float v6 = noise3D(glm::vec3(intX+1, intY+1, intZ));
    float v7 = noise3D(glm::vec3(intX, intY+1, intZ+1));
    float v8 = noise3D(glm::vec3(intX+1, intY+1, intZ+1));

    float i1 = glm::mix(v1, v2, fX);
    float i2 = glm::mix(v3, v4, fX);
    float i3 = glm::mix(i1, i2, fZ);

    float i4 = glm::mix(v5, v6, fX);
    float i5 = glm::mix(v7, v8, fX);
    float i6 = glm::mix(i4, i5, fZ);

    return glm::mix(i3, i6, fY);
}

float computeFBM3D(const int x, const int y, const int z)
{
    float noiseSum = 0;
    float iter_amp = 0.25;
    float iter_freq = 16;

    for (int i = 1; i <= 7; i++) {
        noiseSum += 0.5 * interpNoise3D(float(x)*iter_freq, float(y)*iter_freq, float(z)*iter_freq) * iter_amp;
        iter_amp *= 0.5;
        iter_freq *= 2.f;

    }
    return noiseSum;
}
