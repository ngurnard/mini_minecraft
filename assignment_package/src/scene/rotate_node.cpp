#include "rotate_node.h"

RotateNode::RotateNode() : Node(), angle(0), axis(glm::vec3(1, 0, 0))
{}

RotateNode::RotateNode(float angle, glm::vec3 axis) : Node(), angle(angle), axis(axis)
{}

glm::mat4 RotateNode::computeTransformation()
{
    return glm::rotate(glm::mat4(), glm::radians(angle), axis);
}
RotateNode::~RotateNode()
{}
