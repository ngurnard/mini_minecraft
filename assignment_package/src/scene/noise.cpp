#include "noise.h"

Noise::Noise()
{
    createHeightMaps();
    populateDeltaRivers();
}
void Noise::populateDeltaRivers()
{
    delta.lsystemParse(num_delta_iterations);
    carveDeltaRivers();
}
void Noise::clearOutofRangeRiverCoords()
{
        deltaRiverCoords.clear();
}
void Noise::carveDeltaRivers()
{
    for(int i = 0; i < delta.axiom.size(); i++)
    {
        char literal = delta.axiom[i];
        glm::vec2 start_position = delta.current_state.position;
        (delta.*(delta.char_to_rulefunction[literal]))();
        glm::vec2 end_position = delta.current_state.position;
        if(literal == 'F')
        {
            int minZ = std::min(start_position[1], end_position[1]), maxZ = std::max(start_position[1], end_position[1]);
            if(start_position[1] == end_position[1])
                continue;
            int radius = start_radius - delta.current_state.iteration;
            for(int z = minZ; z <= maxZ; z++)
            {
                int intercept;
                if(start_position[0] == end_position[0])
                    intercept = floor(start_position[0]);
                else
                    intercept = floor(start_position[0] +
                                ((z - start_position[1]) * (start_position[0] - end_position[0]))/(start_position[1] - end_position[1]));
                for(int x = intercept - radius; x <= intercept + radius; x++)
                {
                    deltaRiverCoords.push_back(std::make_pair(x, z));
                }
            }
        }
    }
}

void Noise::createHeightMaps()
{
    // Creates the FBM-based Height Maps for Mountains and Grassland
    // as well as an FBM-based Mask which will interpolate between these regions

    // y Height range where [0,128] should be stone, the rest is biome-specific
    glm::vec2 range(129, 255);

    // Generic FBM parameters
    int mtn_octaves = 4; float mtn_freq = 0.05f;
    float mtn_amp = 0.5; float mtn_persistance = 0.5;

    m_mountainHeightMap = customFBM(mtn_octaves, mtn_freq, mtn_amp, mtn_persistance, range);

    int grass_octaves = 8; float grass_freq = 0.03f;
    float grass_amp = 0.5; float grass_persistance = 0.5;

    m_grasslandHeightMap = customFBM(grass_octaves, grass_freq, grass_amp, grass_persistance, range);

    int mask_octaves = 8; float mask_freq = 0.009;
    float mask_amp = 0.5; float mask_persistance = 0.6; //very sensitive!!
    glm::vec2 mask_range(0,1);

    m_biomeMaskMap = customFBM(mask_octaves, mask_freq, mask_amp, mask_persistance, mask_range);
}

void Noise::mountainHeightPostProcess(float& val)
{
    // Changes peak distribution to Gaussian
    val = 0.95 * glm::exp(-pow(val, 2.f) / 0.65f);
}

void Noise::grasslandHeightPostProcess(float& val)
{
    // flatten and lower terrain relative to mountains
    val = 0.03 + 0.135 * (pow(val+.2, 2) - 0.5 * pow(val, 3));
}

void Noise::biomeMaskPostProcess(float& val)
{
    // smoothstep to increase contrast
    val = pow(val, 2.25);
    val = glm::smoothstep(0.15f, 0.95f, val); // original
}

std::pair<int, int> Noise::computeHeight(int x, int z)
{
    // Computes individual biome heights and blends w/ biomeMask
    float mtnH = m_mountainHeightMap.computeFBM(x, z);
    mountainHeightPostProcess(mtnH);
    m_mountainHeightMap.mapOutput2Range(mtnH);

    float grassH = m_grasslandHeightMap.computeFBM(x, z);
    grasslandHeightPostProcess(grassH);
    m_grasslandHeightMap.mapOutput2Range(grassH);

    float mask = m_biomeMaskMap.computeFBM(x, z);
    biomeMaskPostProcess(mask);
    int biome = 0; // grassland
    if (mask > 0.4) {
        biome = 1; // mtn
    }
    // mask should be [0,1] bounded...

    return {floor(glm::mix(grassH, mtnH, mask)), biome};
}

void Noise::printHeight(int x, int z)
{
    auto blockInfo = computeHeight(x, z);
    int H = blockInfo.first;
    int biome = blockInfo.second;
    std::cout << "Height Map @ [" << x << ", " << z << "] = " << H << " --BIOME " << biome << std::endl;
}

BlockType Noise::getBlockType(int height, int max_height, int biome, float snow_noise, float caveNoiseVal, float caveMask, bool isDeltaRiver)
{
    // Includes caves
    int biomeBaseH = 129;   // Height below which there is only stone
    int snowH = 200;        // Height where snow is possible
    if(isDeltaRiver && biome == 0 && max_height < 160)
    {
        if(height >= 130 && height <= 135)
            return WATER;
        else if(height > 135)
            return EMPTY;
    }

    if(height == 0)
        return BEDROCK;
    bool caveMaskCondition = caveMask < 0.4 && height < max_height - 15 + 15 * snow_noise && caveNoiseVal < 0.4 && height > 0;
    if (caveMaskCondition)  // make caves mostly under surface, but some noise to sometimes break surface
    {
        if (height < 25)
            return LAVA;
        else
            return EMPTY;
    }
    else
    {
        if(height  < biomeBaseH - 1)
            return STONE;
        if(biome == 0)
        {
            if(height < max_height)
                return DIRT;
            else if(height == max_height)
                return GRASS;
        }
        else if(biome == 1)
        {
            if(max_height >= snowH)
            {
                if(height < max_height)
                    return STONE;
                else if(height == max_height)
                {
                    if(max_height < snowH + 10)
                    {

                        if (float(snowH + 10 - max_height) / 10.f < pow(snow_noise, 0.5)) {
                            return SNOW;
                        } else {
                            return STONE;
                        }

                    }
                    else
                        return SNOW;
                }
            }
            else if(height <= max_height)
                return STONE;
        }
    }
    return EMPTY;
}

BlockType Noise::getBlockType(int height, int max_height, int biome, float snow_noise, bool isDeltaRiver)
{
    // Excludes caves
    int biomeBaseH = 129;   // Height below which there is only stone
    int snowH = 200;        // Height where snow is possible
    if(isDeltaRiver && biome == 0 && max_height < 160)
    {
        if(height >= 130 && height <= 135)
            return WATER;
        else if(height > 135)
            return EMPTY;
    }
    if(height == 0) {return BEDROCK;}
    if(height  < biomeBaseH - 1) {return STONE;}

    if(biome == 0)
    {
        if(height < max_height)
            return DIRT;
        else if(height == max_height)
            return GRASS;
    }
    else if(biome == 1)
    {
        if(max_height >= snowH)
        {
            if(height < max_height)
                return STONE;
            else if(height == max_height)
            {
                if(max_height < snowH + 10)
                {

                    if (float(snowH + 10 - max_height) / 10.f < pow(snow_noise, 0.5)) {
                        return SNOW;
                    } else {
                        return STONE;
                    }

                }
                else
                    return SNOW;
            }
        }
        else if(height <= max_height)
            return STONE;
    }
    return EMPTY;
}
