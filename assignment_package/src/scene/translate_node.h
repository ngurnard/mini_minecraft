#ifndef TRANSLATE_NODE_H
#define TRANSLATE_NODE_H
#include "node.h"
class TranslateNode : public Node
{
public:
    glm::vec3 translation;
    TranslateNode();
    TranslateNode(float x, float y, float z);
    virtual ~TranslateNode();
    // Methods
    glm::mat4 computeTransformation() override;
};

#endif // TRANSLATE_NODE_H
