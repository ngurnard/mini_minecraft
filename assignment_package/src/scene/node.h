#ifndef NODE_H
#define NODE_H
#include "scene/block.h"
#include "smartpointerhelp.h"
#include "scene/cube.h"
class Node
{
public:
    // A set of unique_ptrs to the node's children. We recommend using std::vectors to store the pointers, since they act as dynamically
    // resizeable arrays.
    std::vector<uPtr<Node>> children;
    Cube* cube;
    BlockTypeNPC NPCbodyPart;
    Node();
    Node(Cube* cube, BlockTypeNPC NPCbodyPart);
    virtual ~Node();
    // Methods
    // A purely virtual function that computes and returns a 3x3 homogeneous matrix representing the transformation in the node.
    virtual glm::mat4 computeTransformation() = 0;
    // A function that adds a given unique_ptr as a child to this node. You'll have to make use of std::move to make this work.
    // Additionally, to make scene graph construction easier for you, this function should return a Node& that refers directly to the Node
    // that is pointed to by the unique_ptr passed into the function. This will allow you to modify that heap-based Node from within your
    // scene graph construction function without worrying about std::move-ing unique pointers around .
    Node& addChild(uPtr<Node> node);
};

#endif // NODE_H
