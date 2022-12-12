#ifndef SCALE_NODE_H
#define SCALE_NODE_H

#include "node.h"
class ScaleNode : public Node
{
public:
    glm::vec3 scale;
    ScaleNode();
    ScaleNode(float x, float y, float z);
    ScaleNode(float x, float y, float z, Cube* cube, BlockTypeNPC NPCbodyPart);
    virtual ~ScaleNode();
    // Methods
    glm::mat4 computeTransformation() override;
};

#endif // SCALE_NODE_H
