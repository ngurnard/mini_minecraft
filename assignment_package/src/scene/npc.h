#ifndef NPC_H
#define NPC_H
#include "scene/player.h"
#include "translate_node.h"
#include "rotate_node.h"
#include "scale_node.h"
#include "entity.h"
enum NPCDirection : unsigned char
{
    LEFT, RIGHT, FORWARD, BACKWARD, JUMP, STAY
};
const static std::unordered_map<int, std::string> dir_enum_to_string = {
    { 0, "LEFT" },
    { 1, "RIGHT" },
    { 2, "FORWARD" },
    { 3, "BACKWARD" },
    { 4, "JUMP" },
    { 5, "STAY" },
};
class NPC : public Entity
{
public:
    const Player &m_player;
    const Terrain &m_terrain;
    OpenGLContext* context;
    glm::vec3 m_velocity, m_acceleration;
    Cube m_geomCube;
    // Root for translation
    uPtr<Node> root;
    // To rotate body parts
    Node* leftHandRotateRaw;
    Node* rightHandRotateRaw;
    Node* leftLegRotateRaw;
    Node* rightLegRotateRaw;
    Node* headRotateRaw;
    Node* torsoRotateRaw;

    // To draw/undraw body parts
    Node* torsoScaleRaw;
    Node* leftHandScaleRaw;
    Node* rightHandScaleRaw;
    Node* leftLegScaleRaw;
    Node* rightLegScaleRaw;
    Node* headScaleRaw;
    float m_tryMotionTimer;
    bool is_npc_moving;
    bool doBFS;
    glm::vec3 m_prev_position;
    NPCDirection prev_dir;
    std::deque<NPCDirection> pathToPlayer;
    NPC(glm::vec3 m_position, const Terrain &m_terrain, const Player &m_player, OpenGLContext* context);
    virtual ~NPC() override;
    void tick(float dT, InputBundle &input) override;
    void constructSceneGraph();
    void moveNPC(float dT);
    bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, float *out_dist, glm::ivec3 *out_blockHit, float *interfaceAxis = 0); // last argument is optional
    std::array<bool, 3> checkCollision(glm::vec3 &rayDirection);
    void computePhysics(float dT);
    bool checkOnGround();
    bool checkOnGround(int x, int y, int z);
    void moveAlongVector(glm::vec3 dir) override;
    bool checkInLiquid();
    bool checkIsLiquid(float x, float y, float z);
    void processInputs(InputBundle &inputs);
    void moveInGivenDirection(NPCDirection direction);
    bool checkPlayerOnGround();
    void rotatePlayer(NPCDirection curr_dir);
};

#endif // NPC_H
