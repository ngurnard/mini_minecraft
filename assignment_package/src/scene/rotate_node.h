#ifndef ROTATE_NODE_H
#define ROTATE_NODE_H
#include "node.h"

class RotateNode : public Node
{
public:
    float angle;
    glm::vec3 axis;
    RotateNode();
    RotateNode(float angle, glm::vec3 axis);
    virtual ~RotateNode();
    // Methods
    glm::mat4 computeTransformation() override;
};
#endif // ROTATE_NODE_H
