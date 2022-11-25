#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>
#include <QDir>



MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this), m_hud(this), m_viewedBlock(this),
      m_progSky(this), m_progLambert(this), m_progFlat(this), m_progInstanced(this),
      m_frameBuffer(this, width(), height(), 1.0f),
      m_noOp(this), m_postLava(this), m_postWater(this), m_HUD(this),
      m_terrain(this), m_player(glm::vec3(0.f, 150.f, 0.f), m_terrain),
      m_time(0.f),
      prevTime(QDateTime::currentMSecsSinceEpoch()), m_geomQuad(this),
      mp_textureAtlas(nullptr)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
     m_geomQuad.destroyVBOdata();
    // Deallocate all GPU-side data
    m_frameBuffer.destroy();
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Alpha Blending functionality
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // Set the color with which the screen is filled at the start of each render call.
//    glClearColor(0.37f, 0.74f, 1.0f, 1);
    glClearColor(0.47f, 0.64f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();
    m_hud.createVBOdata();

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    // Generate and bind Texture Atlas
    // stored as uPtr mp_textureAtlas
    createTexAtlas();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");
    m_progSky.create(":/glsl/sky.vert.glsl", ":/glsl/sky.frag.glsl");

    // Create and set up the frame buffer
    m_frameBuffer.create();

    // Create and set up the post process shaders
    m_geomQuad.create(); // create the quadrangle over the whole screen
    m_noOp.create(":/glsl/passthrough.vert.glsl", ":/glsl/noOp.frag.glsl");
    m_postLava.create(":/glsl/passthrough.vert.glsl", ":/glsl/postLava.frag.glsl");
    m_postWater.create(":/glsl/passthrough.vert.glsl", ":/glsl/postWater.frag.glsl");
    m_HUD.create(":/glsl/hudPassthrough.vert.glsl", ":/glsl/hud.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);
    //    m_terrain.CreateTestScene();
    //    m_terrain.CreateTestTerrainScene();

    m_terrain.allowTransparent(true);   // whether to draw transparent blocks
    m_terrain.allowCaves(false);        // whether to draw caves (improves performance considerably)
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)
    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_progSky.setViewProjMatrix(glm::inverse(viewproj));

    // Resize the frame buffer
    m_frameBuffer.resize(w, h, 1.f);
    m_frameBuffer.destroy();
    m_frameBuffer.create();

    // Resize the postprocess shaders
    m_noOp.setDimensions(glm::ivec2(w * devicePixelRatio(), h * devicePixelRatio()));
    m_postLava.setDimensions(glm::ivec2(w * devicePixelRatio(), h * devicePixelRatio()));
    m_postWater.setDimensions(glm::ivec2(w * devicePixelRatio(), h * devicePixelRatio()));
    m_HUD.setDimensions(glm::ivec2(w * devicePixelRatio(), h * devicePixelRatio()));
    m_progSky.setDimensions(glm::ivec2(w * devicePixelRatio(), h * devicePixelRatio()));

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    glm::vec3 playerPosPrev = m_player.mcr_position;
    qint64 currTime = QDateTime::currentMSecsSinceEpoch(); // time at this ticl
    float dT = (currTime - prevTime) / 1000.f; // convert from miliseconds to seconds. also typecast to float for computePhysics
    m_player.tick(dT, m_inputs); // tick the player
    // Uncomment this line to test terrain expansion
//    m_terrain.updateTerrain(m_player.mcr_position);

    // Check if the terrain should expand. This both checks to see if player is near the border of
    // existing terrain and checks the status of any BlockType workers that are generating Chunks.
    m_terrain.multithreadedWork(m_player.mcr_position, playerPosPrev, dT);
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline

    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
    prevTime = currTime; // update the previous time
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
    emit sig_sendLiquidBool(QString::number(m_player.playerInLiquid));
    emit sig_sendGroundBool(QString::number(m_player.playerOnGround));
    emit sig_sendCamBlock(QString(m_player.camBlock));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {

    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Send ViewProj to Shaders
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();
    m_progFlat.setViewProjMatrix(viewproj);
    m_progLambert.setViewProjMatrix(viewproj);
    m_progInstanced.setViewProjMatrix(viewproj);
    m_progSky.setViewProjMatrix(glm::inverse(viewproj));

    // Send camera position to shaders for the sky and blinn-phong
    m_progSky.setEye(m_player.mcr_camera.mcr_position);
    m_progFlat.setEye(m_player.mcr_camera.mcr_position);
    m_progLambert.setEye(m_player.mcr_camera.mcr_position);
    m_progInstanced.setEye(m_player.mcr_camera.mcr_position);

    // Set Timers in Shaders
    m_progLambert.setTime(m_time);
    m_postLava.setTime(m_time);
    m_postWater.setTime(m_time);
    m_HUD.setTime(m_time);
    m_progSky.setTime(m_time);

    m_frameBuffer.bindFrameBuffer();
    glViewport(0,0,this->width(), this->height());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_textureAtlas->bind(0); //must bind with every call to draw

    // Draw Sky, then Terrain
    m_progSky.draw(m_geomQuad, 0);
    renderTerrain();

    // Post process render pass ///
    performPostprocessRenderPass();

    // Draw the world Axes without depth checking
    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(viewproj);
    m_progFlat.draw(m_worldAxes, 0);
    glEnable(GL_DEPTH_TEST);

    m_time++;
}


void MyGL::renderTerrain() {
//    m_terrain.draw(0, 64, 0, 64, &m_progLambert);
    int x = 16 * static_cast<int>(glm::floor(m_player.mcr_position.x / 16.f));
    int z = 16 * static_cast<int>(glm::floor(m_player.mcr_position.z / 16.f));

    int rend_dist = 256;

    // Check if the terrain should expand. This both checks to see if player is near the border of
    // existing terrain and checks the status of any BlockType workers that are generating Chunks.
    m_terrain.draw(x - rend_dist, x + rend_dist, z - rend_dist, z + rend_dist, &m_progLambert);
}

void MyGL::createTexAtlas()
{
    // Create a texture to hold the 256x256px texture atlas.
    // Loads in the image minecraft_textures_all.png and then
    // places it in texture slot 0 to be accessed by frag shader

    mp_textureAtlas = mkU<Texture>(this);
    mp_textureAtlas->create(":/textures/minecraft_textures_all.png");
    mp_textureAtlas->load(0);
}

void MyGL::performPostprocessRenderPass()
{
    // Render the frame buffer as a texture on a screen-size quad

    // Tell OpenGL to render to the viewport's frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Texture already bound (thanks Evan I hope you find this :) - Nick)

    // Need logic of which postprocessor to draw since we no longer select a postprocess shader like hw04 ///
    m_frameBuffer.bindToTextureSlot(2);
//    m_postLava.draw(m_geomQuad, 2);

    BlockType viewedBlock = this->m_player.headSpaceSight();
//    BlockType viewedBlock = LAVA;
    if (viewedBlock == LAVA) {
//        std::cout << " In lava postprocessrenderpass" << std::endl;
        m_postLava.draw(m_geomQuad, 2);
    } else if (viewedBlock == WATER) {
        m_postWater.draw(m_geomQuad, 2);
    } else {
        m_noOp.draw(m_geomQuad, 2); // no post process
    }
    // if in selection range of a block, draw wireframe around it
    drawBlockWireframe();
    m_HUD.draw(m_hud, 2);
}

void MyGL::drawBlockWireframe()
{
    glm::ivec3 blockpos = m_player.getViewedBlockCoord(m_terrain);
    BlockType head = m_player.headSpaceSight();
    if (blockpos.x != NULL && head != WATER && head != LAVA)
    {
        m_viewedBlock.update(blockpos);
        m_progFlat.draw(m_viewedBlock, 2);
    }
}

void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_inputs.rightPressed = true;
    } else if (e->key() == Qt::Key_Left) {
        m_inputs.leftPressed = true;
    } else if (e->key() == Qt::Key_Up) {
        m_inputs.upPressed = true;
    } else if (e->key() == Qt::Key_Down) {
        m_inputs.downPressed = true;
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_Q) {
        if (m_inputs.flightMode)
        {
            m_inputs.qPressed = true;
        }
    } else if (e->key() == Qt::Key_E) {
        if (m_inputs.flightMode)
        {
            m_inputs.ePressed = true;
        }
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    } else if (e->key() == Qt::Key_F) {
        m_inputs.fPressed = true;
        m_inputs.flightMode = !m_inputs.flightMode;
        if (m_inputs.flightMode) {
            m_player.toggle_flying.play();
        }
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_inputs.rightPressed = false;
    } else if (e->key() == Qt::Key_Left) {
        m_inputs.leftPressed = false;
    } else if (e->key() == Qt::Key_Up) {
        m_inputs.upPressed = false;
    } else if (e->key() == Qt::Key_Down) {
        m_inputs.downPressed = false;
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
    } // dont do anything for F this time
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    float dpi = 0.03; // NICK: sensitivity of moving the mouse around the screen
//    float dpi = 0.0005; // BENEDICT: sensitivity of moving the mouse around the screen

    // NOTE: position() returns the position of the point in this event,
    // relative to the widget or item that received the event.
    float dx = -(e->position().x() - this->width() / 2); // mouse position relative to center - center of screen. Negative because testing was backwards
    float dy = -(e->position().y() - this->height() / 2); // mouse position relative to center - center of screen. Negative because testing was backwards
    this->m_player.rotateOnUpGlobal(dx * dpi);
    this->m_player.rotateOnRightLocal(dy * dpi);
    moveMouseToCenter(); // recenter the mouse to properly compute the next dx and dy
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    // somehow call grid marching with the camera as the origin? and remove the block if they click
    if (e->button() == Qt::LeftButton) { // if the player clicks the left mouse button, remove a block
        BlockType removedBlock = this->m_player.removeBlock(this->m_terrain);
    } else if (e->button() == Qt::RightButton) { // if the player clicks the right mouse, place a block
        BlockType blockToPlace = LAVA;//GRASS;
        this->m_player.placeBlock(this->m_terrain, blockToPlace);
    }
}
