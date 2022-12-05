#include "node.h"

Node::Node() : cube(nullptr), toDraw(), NPCbodyPart()
{}
Node::Node(Cube* cube, bool toDraw, BlockTypeNPC NPCbodyPart)
    : cube(cube), toDraw(toDraw), NPCbodyPart(NPCbodyPart)
{}
Node& Node::addChild(uPtr<Node> node)
{
    Node &ref = *node;
    this->children.push_back(std::move(node));
    //Also recall that you can get a raw C-style pointer to the address
    //owned by a smart pointer via the get() function.
    return ref;
}
Node::~Node()
{}
