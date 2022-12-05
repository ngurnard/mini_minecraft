#include "scale_node.h"

ScaleNode::ScaleNode() : Node(), scale(glm::vec3(1.f))
{}

ScaleNode::ScaleNode(float x, float y, float z) : Node(), scale(glm::vec3(x, y, z))
{}
ScaleNode::ScaleNode(float x, float y, float z, Cube* cube, bool toDraw, BlockTypeNPC NPCbodyPart)
    : Node(cube, toDraw, NPCbodyPart), scale(glm::vec3(x, y, z))
{}

glm::mat4 ScaleNode::computeTransformation()
{
    return glm::scale(glm::mat4(), scale);
}
ScaleNode::~ScaleNode()
{}
