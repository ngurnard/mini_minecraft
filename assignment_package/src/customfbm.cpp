#include "customfbm.h"

customFBM::customFBM()
    : octaves(6), freq(16), amp(0.5), persistance(0.5)
{}

customFBM::customFBM(int oct, float f, float A, float p)
    : octaves(oct), freq(f), amp(A), persistance(p), range({129, 256})
{}

customFBM::customFBM(int oct, float f, float A, float p, glm::vec2 range)
    : octaves(oct), freq(f), amp(A), persistance(p), range(range)
{}

// --------------------------------------
// --------------------------------------

float customFBM::noise2D(glm::vec2 p) const
{
    return glm::fract(sin(glm::dot(p, glm::vec2(139.2, 269.1))) * 45858.5453);
}

float customFBM::interpNoise2D(float x, float z) const
{
    int intX = int(floor(x));
    float fX = glm::fract(x);
    fX = fX * fX * (3 - 2 * fX);

    int intZ = int(floor(z));
    float fZ = glm::fract(z);
    fZ = fZ * fZ * (3 - 2 * fZ);

    float v1 = noise2D(glm::vec2(intX,   intZ));
    float v2 = noise2D(glm::vec2(intX+1, intZ));
    float v3 = noise2D(glm::vec2(intX,   intZ+1));
    float v4 = noise2D(glm::vec2(intX+1, intZ+1));

    float i1 = glm::mix(v1, v2, fX);
    float i2 = glm::mix(v3, v4, fX);
    return glm::mix(i1, i2, fZ);
}

void customFBM::mapOutput2Range(float& val) const
{
    // Expects input value in range [0,1]
    // TODO: delete this check if it never raises an issue
    if (val > 1 || val < 0) {
        std::cout << "FBM is computing value outside of expected range [0,1]" << std::endl;
    }
    val = range[0] + val * (range[1] - range[0]);
}

float customFBM::computeFBM(const int x, const int z) const
{
    float noiseSum = 0;
    float iter_amp = amp;
    float iter_freq = freq;

    for (int i = 1; i <= octaves; i++) {
        noiseSum += interpNoise2D(float(x)*iter_freq, float(z)*iter_freq) * iter_amp;
        iter_amp *= persistance;
        iter_freq *= 2.f;

    }

    //mapOutput2Range(noiseSum);
    return noiseSum;
}

