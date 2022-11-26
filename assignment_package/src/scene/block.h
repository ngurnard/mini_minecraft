#pragma once
#include <QString>
#include <unordered_map>

enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, LAVA, ICE, SNOW, SAND, UNCERTAIN, BEDROCK
};

const static std::unordered_map<int, QString> type_enum_to_string = {
    { 0, "EMPTY" },
    { 1, "GRASS" },
    { 2, "DIRT" },
    { 3, "STONE" },
    { 4, "WATER" },
    { 5, "LAVA" },
    { 6, "ICE" },
    { 7, "SNOW" },
    { 8, "SAND" },
    { 9, "UNCERTAIN" },
    { 10, "BEDROCK" },
};
