#ifndef NOISE_H
#define NOISE_H
#include "scene/deltariver.h"
#pragma once
#include "customfbm.h" // Custom Noise Function Class
#include "block.h"
#include <vector>
class Noise
{
public:
    Noise();
    customFBM m_mountainHeightMap;
    customFBM m_grasslandHeightMap;
    customFBM m_biomeMaskMap;
    DeltaRiver delta = DeltaRiver(glm::vec2(0.f, 0.f), glm::vec2(-0.5f, 1.0f));
    int num_delta_iterations = 5, start_radius = 6;
    std::vector<std::pair<int, int>> deltaRiverCoords;
    bool m_permit_caves;

    void createHeightMaps();
    void clearOutofRangeRiverCoords();
    void carveDeltaRivers();
    void populateDeltaRivers();
    void mountainHeightPostProcess(float&);
    void grasslandHeightPostProcess(float&);
    void biomeMaskPostProcess(float&);

    std::pair<int, int> computeHeight(int x, int z); // returns H, biome
    void printHeight(int x, int z);

    // yes caves
    BlockType getBlockType(int height, int max_height, int biome, float snow_noise, float caveNoiseVal, float caveMask, bool isDeltaRiver);
    // no caves
    BlockType getBlockType(int height, int max_height, int biome, float snow_noise, bool isDeltaRiver);

};

#endif // NOISE_H
