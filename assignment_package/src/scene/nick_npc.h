#ifndef NICK_NPC_H
#define NICK_NPC_H
#include "scene/player.h"
#include "translate_node.h"
#include "rotate_node.h"
#include "scale_node.h"
#include "entity.h"

class NickNPC : public Entity
{
public:
    const Player &m_player;
    const Terrain &m_terrain;
    OpenGLContext* context;
    Cube m_geomCube;
    // Root for translation
    uPtr<Node> root;
    // To rotate body parts
    Node* leftHandRotateRaw;
    Node* rightHandRotateRaw;
    Node* rightHandRotate2Raw;
    Node* leftLegRotateRaw;
    Node* rightLegRotateRaw;
    Node* headRotateRaw;

    // To draw/undraw body parts
    Node* torsoScaleRaw;
    Node* leftHandScaleRaw;
    Node* rightHandScaleRaw;
    Node* leftLegScaleRaw;
    Node* rightLegScaleRaw;
    Node* headScaleRaw;

    NickNPC(glm::vec3 m_position, const Terrain &m_terrain, const Player &m_player, OpenGLContext* context);
    virtual ~NickNPC() override;
    void tick(float dT, InputBundle &input) override;
    void constructSceneGraph();
};

#endif // NICK_NPC_H
