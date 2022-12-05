#pragma once
#include <QString>
#include <unordered_map>

enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, LAVA, ICE, SNOW, SAND, UNCERTAIN, BEDROCK, WOOD, LEAF
};

enum BlockTypeNPC : unsigned char
{
    TORSO, HEAD, LEFT_HAND, RIGHT_HAND, LEFT_LEG, RIGHT_LEG
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
    { 11, "WOOD" },
    { 12, "LEAF" }
};
const static std::unordered_map<int, std::string> NPCtype_enum_to_string = {
    { 0, "TORSO" },
    { 1, "HEAD" },
    { 2, "LEFT_HAND" },
    { 3, "RIGHT_HAND" },
    { 4, "LEFT_LEG" },
    { 5, "RIGHT_LEG" },
};
