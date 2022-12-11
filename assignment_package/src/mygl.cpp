#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>
#include <QDir>
#include <utility>
#include "inventory.h"


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this), m_hud(this), m_viewedBlock(this),
      m_progSkyTerrain(this), m_progClouds(this),
      m_progLambert(this), m_progFlat(this), m_progInstanced(this),
      m_frameBuffer(this, width(), height(), 1.0f),
      m_noOp(this), m_postLava(this), m_postWater(this), m_HUD(this),
      m_terrain(this),
      mp_player(mkU<Player>(glm::vec3(2.f, 150.f, -9.f), m_terrain)),
      m_time(0.f),
      m_benny(glm::vec3(2.f, 136.001f, 5.f), m_terrain, *mp_player.get(), this),
      m_evan(glm::vec3(12.f, 141.001f, -8.f), m_terrain, *mp_player.get(), this),
      m_nick(glm::vec3(-11.f, 135.001f, -7.f), m_terrain, *mp_player.get(), this),
      prevTime(QDateTime::currentMSecsSinceEpoch()),
      m_geomQuad(this), showInventory(false),
      mp_textureAtlas(nullptr),
      mp_textureNick(nullptr),
      mp_textureEvan(nullptr),
      mp_textureBenny(nullptr)
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
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
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

    m_nick.m_geomCube.destroyVBOdata();
    m_nick.m_geomCube.createVBOdata();
    m_nick.constructSceneGraph();
    m_benny.m_geomCube.destroyVBOdata();
    m_benny.m_geomCube.createVBOdata();
    m_benny.constructSceneGraph();
    m_evan.m_geomCube.destroyVBOdata();
    m_evan.m_geomCube.createVBOdata();
    m_evan.constructSceneGraph();
    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    // Generate and bind Texture Atlas
    // stored as uPtr mp_textureAtlas
    createTexAtlas();

    // Generate the unique player Ptr to select blocks
//    mup_player = mkU<Player>(mp_player.get());
    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");
    m_progSkyTerrain.create(":/glsl/SkyTerrainUber.vert.glsl", ":/glsl/SkyTerrainUber.frag.glsl");
    m_progClouds.create(":/glsl/cloud.vert.glsl", ":/glsl/cloud.frag.glsl");

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

    m_terrain.allowTransparent(true);   // whether tosss draw transparent blocks
    m_terrain.allowCaves(false);        // whether to draw caves (improves performance considerably)
    m_terrain.allowRivers(true);       // whether to draw LRivers
    m_terrain.allowClouds(true);        // whether to draw clouds
    m_terrain.createCloud();            // creates 16x16 cloud chunk in m_terrain which is drawn above chunks
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    mp_player->setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = mp_player->mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)
    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_progSkyTerrain.setViewProjMatrix(viewproj);
    m_progSkyTerrain.setViewProjInvMatrix(glm::inverse(viewproj));
    m_progClouds.setViewProjMatrix(viewproj);

    // Resize the frame buffer
    m_frameBuffer.resize(w, h, 1.f);
    m_frameBuffer.destroy();
    m_frameBuffer.create();

    // Resize the postprocess shaders
    glm::ivec2 dims(w, h);
    m_noOp.setDimensions(dims);
    m_postLava.setDimensions(dims);
    m_postWater.setDimensions(dims);
    m_HUD.setDimensions(dims);
    m_progSkyTerrain.setDimensions(dims);
    m_progClouds.setDimensions(dims);
    m_progLambert.setDimensions(dims);

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    glm::vec3 playerPosPrev = mp_player->mcr_position;
    qint64 currTime = QDateTime::currentMSecsSinceEpoch(); // time at this ticl
    float dT = (currTime - prevTime) / 1000.f; // convert from miliseconds to seconds. also typecast to float for computePhysics
    mp_player->tick(dT, m_inputs); // tick the player
    if(m_time > 400)
    {
        m_nick.tick(dT, m_inputs);
        m_evan.tick(dT, m_inputs);
        m_benny.tick(dT, m_inputs);

    }
    // Uncomment this line to test terrain expansion
//    m_terrain.updateTerrain(m_player.mcr_position);

    // Check if the terrain should expand. This both checks to see if player is near the border of
    // existing terrain and checks the status of any BlockType workers that are generating Chunks.
    m_terrain.multithreadedWork(mp_player->mcr_position, playerPosPrev, dT);
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline

    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
    sendInventoryDataToGUI();
    prevTime = currTime; // update the previous time
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(mp_player->posAsQString());
    emit sig_sendPlayerVel(mp_player->velAsQString());
    emit sig_sendPlayerAcc(mp_player->accAsQString());
    emit sig_sendPlayerLook(mp_player->lookAsQString());
    glm::vec2 pPos(mp_player->mcr_position.x, mp_player->mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
    emit sig_sendLiquidBool(QString::number(mp_player->playerInLiquid));
    emit sig_sendGroundBool(QString::number(mp_player->playerOnGround));
    emit sig_sendCamBlock(QString(mp_player->camBlock));
    emit sig_sendCurrentHoldingBlock(QString(type_enum_to_string.at(mp_player->holdingBlock)));
}

void MyGL::sendInventoryDataToGUI() {
    emit sig_sendGrassCount(mp_player->grassCount);
    emit sig_sendDirtCount(mp_player->dirtCount);
    emit sig_sendStoneCount(mp_player->stoneCount);
    emit sig_sendWaterCount(mp_player->waterCount);
    emit sig_sendLavaCount(mp_player->lavaCount);
    emit sig_sendIceCount(mp_player->iceCount);
    emit sig_sendSnowCount(mp_player->snowCount);
    emit sig_sendSandCount(mp_player->sandCount);

    // For selecting the holding block
    emit sig_sendPlayer(mp_player.get());
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {

    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Send ViewProj to Shaders
    glm::mat4 viewproj = mp_player->mcr_camera.getViewProj();
    m_progFlat.setViewProjMatrix(viewproj);
    m_progLambert.setViewProjMatrix(viewproj);
    m_progInstanced.setViewProjMatrix(viewproj);
    m_progSkyTerrain.setViewProjMatrix(viewproj);
    m_progSkyTerrain.setViewProjInvMatrix(glm::inverse(viewproj));
    m_progClouds.setViewProjMatrix(viewproj);

    // Send camera position to shaders for the sky and blinn-phong
    m_progClouds.setEye(mp_player->mcr_camera.mcr_position);
    m_progSkyTerrain.setEye(mp_player->mcr_camera.mcr_position);
    m_progFlat.setEye(mp_player->mcr_camera.mcr_position);
    m_progLambert.setEye(mp_player->mcr_camera.mcr_position);
    m_progInstanced.setEye(mp_player->mcr_camera.mcr_position);

    // Set Timers in Shaders
    m_progLambert.setTime(m_time);
    m_postLava.setTime(m_time);
    m_postWater.setTime(m_time);
    m_HUD.setTime(m_time);
    m_progSkyTerrain.setTime(m_time);
    m_progClouds.setTime(m_time);

    // bind frame buffer so that it is drawn to, not the screen
    m_frameBuffer.bindFrameBuffer();
    glViewport(0,0,this->width(), this->height());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // USING NEW SKY/TERRAIN UBER SHADER
    // couples previous sky and lamber shaders into a more
    // efficient single shader which draws sky if u_QuadDraw
    // is set to TRUE. drawing terrian first will now prevent
    // redundant Sky drawing beneath terrain. This uber shader
    // also allows the terrain part to access sky color info and such,
    // for more flexible and interesting options for environment looks

    // Draw Sky
    m_progSkyTerrain.setQuadDraw(true);
    m_progSkyTerrain.draw(m_geomQuad, 0);
    // Draw Terrain
    mp_textureAtlas->bind(0);
    m_progSkyTerrain.setQuadDraw(false);
    renderTerrain();
    mp_textureNick->bind(1);
    mp_textureEvan->bind(2);
    mp_textureBenny->bind(3);

    m_progSkyTerrain.setQuadDraw(false);
    if(m_nick.root)
    {
        m_progSkyTerrain.setModelMatrix(glm::mat4(1.f));
        traverse(m_nick.root, glm::mat4(1.f), 1);
    }
    if(m_evan.root)
    {
        m_progSkyTerrain.setModelMatrix(glm::mat4(1.f));
        traverse(m_evan.root, glm::mat4(1.f), 2);
    }
    if(m_benny.root)
    {
        m_progSkyTerrain.setModelMatrix(glm::mat4(1.f));
        traverse(m_benny.root, glm::mat4(1.f), 3);
    }

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

void MyGL::resetFB(int texSlot) {
    // refactored to restore default FB to draw to screen
    // and bind m_frameBuffer to texSlot

    // Tell OpenGL to render to the viewport's frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // actually bind to appropriate slot
    m_frameBuffer.bindToTextureSlot(texSlot);
}


void MyGL::renderTerrain() {
//    m_terrain.draw(0, 64, 0, 64, &m_progLambert);
    int x = 16 * static_cast<int>(glm::floor(mp_player->mcr_position.x / 16.f));
    int z = 16 * static_cast<int>(glm::floor(mp_player->mcr_position.z / 16.f));

    int rend_dist = 256;
    // Check if the terrain should expand. This both checks to see if player is near the border of
    // existing terrain and checks the status of any BlockType workers that are generating Chunks.
//    m_terrain.draw(x - rend_dist, x + rend_dist, z - rend_dist, z + rend_dist, &m_progLambert);
    m_terrain.draw(x - rend_dist, x + rend_dist, z - rend_dist, z + rend_dist, &m_progSkyTerrain);

    m_terrain.drawClouds(x - rend_dist, x + rend_dist, z - rend_dist, z + rend_dist, &m_progClouds);
}

void MyGL::createTexAtlas()
{
    // Create a texture to hold the 256x256px texture atlas.
    // Loads in the image minecraft_textures_all.png and then
    // places it in texture slot 0 to be accessed by frag shader

    mp_textureAtlas = mkU<Texture>(this);
    mp_textureAtlas->create(":/textures/minecraft_textures_all.png");
    mp_textureAtlas->load(0);

    mp_textureNick = mkU<Texture>(this);
    mp_textureNick->create(":/textures/nick_texture_2.png");
    mp_textureNick->load(1);

    mp_textureEvan = mkU<Texture>(this);
    mp_textureEvan->create(":/textures/evan_texture_2.png");
    mp_textureEvan->load(2);

    mp_textureBenny = mkU<Texture>(this);
    mp_textureBenny->create(":/textures/benny_texture.png");
    mp_textureBenny->load(3);
}

void MyGL::performPostprocessRenderPass()
{
    int fbSlot = 2;
    // Render the frame buffer as a texture on a screen-size quad
    resetFB(fbSlot);

    BlockType viewedBlock = this->mp_player->headSpaceSight();
    if (viewedBlock == LAVA) {
        m_postLava.draw(m_geomQuad, fbSlot);
    } else if (viewedBlock == WATER) {
        m_postWater.draw(m_geomQuad, fbSlot);
    } else {
        m_noOp.draw(m_geomQuad, fbSlot); // no post process
    }
    // if in selection range of a block, draw wireframe around it
    drawBlockWireframe();
    m_HUD.draw(m_hud, fbSlot);
}

void MyGL::drawBlockWireframe()
{
    glm::ivec3 blockpos = mp_player->getViewedBlockCoord(m_terrain);
    BlockType head = mp_player->headSpaceSight();
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
            mp_player->toggle_flying.play();
        }
    } else if (e->key() == Qt::Key_I) {
        showInventory = !showInventory; // switch it from whatever it was before
        emit sig_showInventory(showInventory);
    }
    else if (e->key() == Qt::Key_B) {
        m_benny.doBFS = true;
        m_benny.m_prev_position = m_benny.m_position;
        }
    else if(e->key() == Qt::Key_N)
    {
        m_nick.doBFS = true;
        m_nick.m_prev_position = m_nick.m_position;
    }
    else if(e->key() == Qt::Key_G)
    {
        m_evan.doBFS = true;
        m_evan.m_prev_position = m_evan.m_position;
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
    } else if (e->key() == Qt::Key_I) {
        showInventory = !showInventory; // switch it from whatever it was before
    } // dont do anything for F this time
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
   float dpi = 0.03; // NICK: sensitivity of moving the mouse around the screen
//     float dpi = 0.0008; // BENEDICT: sensitivity of moving the mouse around the screen

    // NOTE: position() returns the position of the point in this event,
    // relative to the widget or item that received the event.
    float dx = -(e->position().x() - this->width() / 2); // mouse position relative to center - center of screen. Negative because testing was backwards
    float dy = -(e->position().y() - this->height() / 2); // mouse position relative to center - center of screen. Negative because testing was backwards
    this->mp_player->rotateOnUpGlobal(dx * dpi);
    this->mp_player->rotateOnRightLocal(dy * dpi);
    moveMouseToCenter(); // recenter the mouse to properly compute the next dx and dy
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    // somehow call grid marching with the camera as the origin? and remove the block if they click
    if (e->button() == Qt::LeftButton) { // if the player clicks the left mouse button, remove a block
        BlockType removedBlock = this->mp_player->removeBlock(this->m_terrain);
    } else if (e->button() == Qt::RightButton) { // if the player clicks the right mouse, place a block
        BlockType blockToPlace = mp_player->holdingBlock;//LAVA;//GRASS;
        this->mp_player->placeBlock(this->m_terrain, blockToPlace);
    }
}

// Implement a function that takes in a pointer to a Node and a mat3,
// and which traverses your scene graph and draws any existing Polygon2Ds
// at each Node using the sequence of transformation matrices that have been
// concatenated thus far, following the example in the lecture slides.
// You should invoke this function from MyGL::paintGL.

void MyGL::traverse(const uPtr<Node> &node, glm::mat4 transform_mat, int texture_slot)
{
    transform_mat = transform_mat * node->computeTransformation();
    // Use a foreach loop to the effect of for(const uPtr& n : node->children).
    //This enables you to read the children without transferring ownership to
    //traverse because you're not creating a second unique_ptr, just a reference to one.
    for(const uPtr<Node> &n : node->children)
    {
        traverse(n, transform_mat, texture_slot);
    }
    if(node->cube != nullptr && node->toDraw)
    {
        node->cube->NPCbodyPart = node->NPCbodyPart;
        node->cube->destroyVBOdata();
        node->cube->createVBOdata();
        m_progSkyTerrain.setModelMatrix(transform_mat);
        m_progSkyTerrain.drawNPC(*(node->cube), texture_slot);
    }
}
