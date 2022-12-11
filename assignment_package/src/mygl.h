#pragma once

#include "openglcontext.h"
#include "surfaceshader.h"
#include "scene/worldaxes.h"
#include "scene/camera.h"
#include "scene/terrain.h"
#include "scene/player.h"
#include "scene/quad.h"
#include "scene/hud.h"
#include "scene/blockwireframe.h"

#include "texture.h"
#include "postprocessshader.h"
#include "framebuffer.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <smartpointerhelp.h>
#include "scene/npc.h"

class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    WorldAxes m_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    HUD m_hud; // contains crosshair
    BlockWireframe m_viewedBlock; // white frame around current block in view range

    SurfaceShader m_progSkyTerrain;
    SurfaceShader m_progClouds;
    SurfaceShader m_progLambert;// A shader program that uses lambertian reflection
    SurfaceShader m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    SurfaceShader m_progInstanced;// A shader program that is designed to be compatible with instanced rendering
    FrameBuffer m_frameBuffer;
    PostProcessShader m_noOp;
    PostProcessShader m_postLava;
    PostProcessShader m_postWater;
    PostProcessShader m_HUD;



    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Terrain m_terrain; // All of the Chunks that currently comprise the world.
//    Player m_player; // The entity controlled by the user. Contains a camera to display what it sees as well.
    uPtr<Player> mp_player;
    InputBundle m_inputs; // A collection of variables to be updated in keyPressEvent, mouseMoveEvent, mousePressEvent, etc.

    QTimer m_timer; // Timer linked to tick(). Fires approximately 60 times per second.
    int m_time; // Evan mocking up a timer like in HW4
    NPC m_nick;
    NPC m_evan;
    NPC m_benny;

    void moveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

    void sendPlayerDataToGUI() const;
//    void sendInventoryDataToGUI() const;
    void sendInventoryDataToGUI();

    qint64 prevTime; // the time in the last tick

    //Evan's texture stuff
    //std::vector<std::shared_ptr<Texture>> m_allTextures;
    void createTexAtlas();

    // The screen-space quadrangle used to draw
    // the scene with the post-process shaders.
    Quad m_geomQuad;

    // inventory stuff
    bool showInventory;

public:
    uPtr<Texture> mp_textureAtlas;
    uPtr<Texture> mp_textureNick;
    uPtr<Texture> mp_textureEvan;
    uPtr<Texture> mp_textureBenny;
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    // Called once when MyGL is initialized.
    // Once this is called, all OpenGL function
    // invocations are valid (before this, they
    // will cause segfaults)
    void initializeGL() override;
    // Called whenever MyGL is resized.
    void resizeGL(int w, int h) override;
    // Called whenever MyGL::update() is called.
    // In the base code, update() is called from tick().
    void paintGL() override;

    // refactored to restore default FB to draw to screen
    // and bind m_frameBuffer to texSlot
    void resetFB(int texSlot);

    // Called from paintGL().
    // Calls Terrain::draw().
    void renderTerrain();

    // A helper function that iterates through
    // each of the render passes required by the
    // currently bound post-process shader and
    // invokes them.
    void performPostprocessRenderPass();

    void drawBlockWireframe();
    void traverse(const uPtr<Node> &node, glm::mat4 transform_mat, int texture_slot);

protected:
    // Automatically invoked when the user
    // presses a key on the keyboard
    void keyPressEvent(QKeyEvent *e);
    // release event so keys presses don't overwrite each other
    void keyReleaseEvent(QKeyEvent *e); // a key release event so that the player is able to make diaganol movements.
    // Automatically invoked when the user
    // moves the mouse
    void mouseMoveEvent(QMouseEvent *e);
    // Automatically invoked when the user
    // presses a mouse button
    void mousePressEvent(QMouseEvent *e);

private slots:
    void tick(); // Slot that gets called ~60 times per second by m_timer firing.

signals:
    void sig_sendPlayerPos(QString) const;
    void sig_sendPlayerVel(QString) const;
    void sig_sendPlayerAcc(QString) const;
    void sig_sendPlayerLook(QString) const;
    void sig_sendPlayerChunk(QString) const;
    void sig_sendPlayerTerrainZone(QString) const;
    void sig_sendLiquidBool(QString) const;
    void sig_sendGroundBool(QString) const;
    void sig_sendCamBlock(QString) const;
    void sig_sendPlayer(Player*);
    void sig_sendCurrentHoldingBlock(QString) const;

    // inventory
    void sig_showInventory(bool showInventory);
    void sig_sendGrassCount(int count) const;
    void sig_sendDirtCount(int count) const;
    void sig_sendStoneCount(int count) const;
    void sig_sendWaterCount(int count) const;
    void sig_sendLavaCount(int count) const;
    void sig_sendIceCount(int count) const;
    void sig_sendSnowCount(int count) const;
    void sig_sendSandCount(int count) const;
};
