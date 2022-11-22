#pragma once

#include "openglcontext.h"
#include "surfaceshader.h"
#include "scene/worldaxes.h"
#include "scene/camera.h"
#include "scene/terrain.h"
#include "scene/player.h"
#include "scene/quad.h"

#include "texture.h"
#include "postprocessshader.h"
#include "framebuffer.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <smartpointerhelp.h>


class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    WorldAxes m_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    SurfaceShader m_progLambert;// A shader program that uses lambertian reflection
    SurfaceShader m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    SurfaceShader m_progInstanced;// A shader program that is designed to be compatible with instanced rendering
    FrameBuffer m_frameBuffer;
    PostProcessShader m_noOp;
    PostProcessShader m_postLava;
    PostProcessShader m_postWater;


    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Terrain m_terrain; // All of the Chunks that currently comprise the world.
    Player m_player; // The entity controlled by the user. Contains a camera to display what it sees as well.
    InputBundle m_inputs; // A collection of variables to be updated in keyPressEvent, mouseMoveEvent, mousePressEvent, etc.

    QTimer m_timer; // Timer linked to tick(). Fires approximately 60 times per second.
    int m_time; // Evan mocking up a timer like in HW4

    void moveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

    void sendPlayerDataToGUI() const;

    qint64 prevTime; // the time in the last tick

    //Evan's texture stuff
    //std::vector<std::shared_ptr<Texture>> m_allTextures;
    void createTexAtlas();

    // The screen-space quadrangle used to draw
    // the scene with the post-process shaders.
    Quad m_geomQuad;

public:
    uPtr<Texture> mp_textureAtlas;

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

    // Called from paintGL().
    // Calls Terrain::draw().
    void renderTerrain();

    // A helper function that iterates through
    // each of the render passes required by the
    // currently bound post-process shader and
    // invokes them.
    void performPostprocessRenderPass();

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
};
