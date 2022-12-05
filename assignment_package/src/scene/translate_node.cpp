#include "translate_node.h"

TranslateNode::TranslateNode() : Node(), translation(glm::vec3(0.f))
{}

TranslateNode::TranslateNode(float x, float y, float z) : Node(), translation(glm::vec3(x, y, z))
{}

glm::mat4 TranslateNode::computeTransformation()
{
    return glm::translate(glm::mat4(), translation);
}
TranslateNode::~TranslateNode()
{}
