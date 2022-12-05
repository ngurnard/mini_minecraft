#include "nick_npc.h"

NickNPC::NickNPC(glm::vec3 m_position, const Terrain &m_terrain, const Player &m_player, OpenGLContext* context) :
    Entity(m_position), m_player(m_player), m_terrain(m_terrain), context(context), m_geomCube(context),
    root(nullptr), leftHandRotateRaw(nullptr), rightHandRotateRaw(nullptr), rightHandRotate2Raw(nullptr),
    leftLegRotateRaw(nullptr), rightLegRotateRaw(nullptr), headRotateRaw(nullptr),
    torsoScaleRaw(nullptr), leftHandScaleRaw(nullptr), rightHandScaleRaw(nullptr), leftLegScaleRaw(nullptr),
    rightLegScaleRaw(nullptr), headScaleRaw(nullptr)
{}

void NickNPC::tick(float dT, InputBundle &input)
{

}
void NickNPC::constructSceneGraph()
{
    // Construct Torso
    root = mkU<TranslateNode>(m_position.x, m_position.y + 1.25f, m_position.z);
    uPtr<Node> torsoRotate = mkU<RotateNode> (0.f, glm::vec3(0.f, 1.f, 0.f));
    Node& torsoRotateRef = root->addChild(std::move(torsoRotate));
    uPtr<Node> torsoScale = mkU<ScaleNode> (0.5f, 0.75f, 0.25f, &m_geomCube, true, TORSO);
    torsoScaleRaw = torsoScale.get();
    Node& torsoScaleRef = torsoRotateRef.addChild(std::move(torsoScale));

    // Construct head
    uPtr<Node> headTranslate = mkU<TranslateNode> (0.f, 0.625f, 0.f);
    Node& headTranslateRef = torsoRotateRef.addChild(std::move(headTranslate));
    uPtr<Node> headRotate = mkU<RotateNode> ();
    headRotateRaw = headRotate.get();
    Node& headRotateRef = headTranslateRef.addChild(std::move(headRotate));
    uPtr<Node> headScale = mkU<ScaleNode> (0.5f, 0.5f, 0.5f, &m_geomCube, true, HEAD);
    headScaleRaw = headScale.get();
    Node& headScaleRef = headTranslateRef.addChild(std::move(headScale));

    // Construct Left Hand
    uPtr<Node> leftHandTranslate = mkU<TranslateNode> (-0.375f, 0.f, 0.f);
    Node& leftHandTranslateRef = torsoRotateRef.addChild(std::move(leftHandTranslate));
    uPtr<Node> leftHandRotate = mkU<RotateNode> ();
    leftHandRotateRaw = leftHandRotate.get();
    Node& leftHandRotateRef = leftHandTranslateRef.addChild(std::move(leftHandRotate));
    uPtr<Node> leftHandTranslate2 = mkU<TranslateNode> (0.f, -0.25f, 0.f);
    Node& leftHandTranslate2Ref = leftHandRotateRef.addChild(std::move(leftHandTranslate2));
    uPtr<Node> leftHandScale = mkU<ScaleNode> (0.25f, 0.75f, 0.25f, &m_geomCube, true, LEFT_HAND);
    leftHandScaleRaw = leftHandScale.get();
    Node& leftHandScaleRef = leftHandTranslateRef.addChild(std::move(leftHandScale));

    // Construct Right Hand
    uPtr<Node> rightHandTranslate = mkU<TranslateNode> (0.375f, 0.f, 0.f);
    Node& rightHandTranslateRef = torsoRotateRef.addChild(std::move(rightHandTranslate));
    uPtr<Node> rightHandRotate = mkU<RotateNode> (0.f, glm::vec3(0.f, 1.f, 0.f));
    rightHandRotateRaw = rightHandRotate.get();
    Node& rightHandRotateRef = rightHandTranslateRef.addChild(std::move(rightHandRotate));
    uPtr<Node> rightHandRotate2 = mkU<RotateNode> (90.f, glm::vec3(1.f, 0.f, 0.f));
    rightHandRotate2Raw = rightHandRotate2.get();
    Node& rightHandRotate2Ref = rightHandRotateRef.addChild(std::move(rightHandRotate2));
    uPtr<Node> rightHandTranslate2 = mkU<TranslateNode> (0.f, -0.25f, 0.f);
    Node& rightHandTranslate2Ref = rightHandRotate2Ref.addChild(std::move(rightHandTranslate2));
    uPtr<Node> rightHandScale = mkU<ScaleNode> (0.25f, 0.75f, 0.25f, &m_geomCube, true, RIGHT_HAND);
    rightHandScaleRaw = rightHandScale.get();
    Node& rightHandScaleRef = rightHandTranslateRef.addChild(std::move(rightHandScale));


//     Construct Left Leg
    uPtr<Node> leftLegTranslate = mkU<TranslateNode> (-0.125f, -0.75f, 0.f);
    Node& leftLegTranslateRef = torsoRotateRef.addChild(std::move(leftLegTranslate));
    uPtr<Node> leftLegRotate = mkU<RotateNode> ();
    leftLegRotateRaw = leftLegRotate.get();
    Node& leftLegRotateRef = leftLegTranslateRef.addChild(std::move(leftLegRotate));
    uPtr<Node> leftLegTranslate2 = mkU<TranslateNode> (0.f, -0.5f, 0.f);
    Node& leftLegTranslate2Ref = leftLegRotateRef.addChild(std::move(leftLegTranslate2));
    uPtr<Node> leftLegScale = mkU<ScaleNode> (0.25f, 0.75f, 0.25f, &m_geomCube, true, LEFT_LEG);
    leftLegScaleRaw = leftLegScale.get();
    Node& leftLegScaleRef = leftLegTranslateRef.addChild(std::move(leftLegScale));


    // Construct Right Leg
    uPtr<Node> rightLegTranslate = mkU<TranslateNode> (0.125f, -0.75f, 0.f);
    Node& rightLegTranslateRef = torsoRotateRef.addChild(std::move(rightLegTranslate));
    uPtr<Node> rightLegRotate = mkU<RotateNode> ();
    rightLegRotateRaw = rightLegRotate.get();
    Node& rightLegRotateRef = rightLegTranslateRef.addChild(std::move(rightLegRotate));
    uPtr<Node> rightLegTranslate2 = mkU<TranslateNode> (0.f, -0.5f, 0.f);
    Node& rightLegTranslate2Ref = rightLegRotateRef.addChild(std::move(rightLegTranslate2));
    uPtr<Node> rightLegScale = mkU<ScaleNode> (0.25f, 0.75f, 0.25f, &m_geomCube, true, RIGHT_LEG);
    rightLegScaleRaw = rightLegScale.get();
    Node& rightLegScaleRef = rightLegTranslateRef.addChild(std::move(rightLegScale));
}
NickNPC::~NickNPC()
{}
