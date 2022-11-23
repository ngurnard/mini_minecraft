#ifndef NOISE_H
#define NOISE_H
#pragma once
#include "customfbm.h" // Custom Noise Function Class
#include "block.h"
class Noise
{
public:
    Noise();
    customFBM m_mountainHeightMap;
    customFBM m_grasslandHeightMap;
    customFBM m_biomeMaskMap;

    bool m_permit_caves;

    void createHeightMaps();
    void mountainHeightPostProcess(float&);
    void grasslandHeightPostProcess(float&);
    void biomeMaskPostProcess(float&);

    std::pair<int, int> computeHeight(int x, int z); // returns H, biome
    void printHeight(int x, int z);

    // yes caves
    BlockType getBlockType(int height, int max_height, int biome, float snow_noise, float caveNoiseVal, float caveMask);
    // no caves
    BlockType getBlockType(int height, int max_height, int biome, float snow_noise);

};

#endif // NOISE_H
