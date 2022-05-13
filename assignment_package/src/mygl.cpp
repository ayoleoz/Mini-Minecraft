#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#define RENDERING_RADIUS 96
#define SUN_VELOCITY 1 / 20000.f

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomQuad(this),
      m_frameBuffer(this, width(), height(), devicePixelRatio()),m_shadowFrameBuffer(this, width(), height(), devicePixelRatio()),
      m_shadowShader(this),m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_progInstanced(this),
      m_postNoOp(this), m_postBlueTint(this), m_postRedTint(this), m_progSky(this),
      openInventory(false), numGrass(10), numDirt(10), numStone(10), numBedrock(10), numWater(10),
      numLava(10), numSnow(10), currBlockType(GRASS),
      m_terrain(this), m_player(glm::vec3(48.f, 150.f, 48.f), m_terrain),m_texture(this), m_time(0),
      prevFrame(QDateTime::currentMSecsSinceEpoch()), currFrame(QDateTime::currentMSecsSinceEpoch())
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

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();
    // Create and set up the diffuse shader
    m_shadowShader.create(":/glsl/shadow.vert.glsl", ":/glsl/shadow.frag.glsl");
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    initializeTexture();
    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    m_postNoOp.create(":/glsl/passthrough.vert.glsl", ":/glsl/noOp.frag.glsl");
    m_postNoOp.setupMemberVars();
    m_postBlueTint.create(":/glsl/passthrough.vert.glsl", ":/glsl/post_blue_tinge.frag.glsl");
    m_postRedTint.create(":/glsl/passthrough.vert.glsl", ":/glsl/post_red_tinge.frag.glsl");

    m_progSky.create(":/glsl/sky.vert.glsl", ":/glsl/sky.frag.glsl");
    m_geomQuad.create();
    m_frameBuffer = FrameBuffer(this, width(), height(), devicePixelRatio());
    m_frameBuffer.create();
    m_shadowFrameBuffer = ShadowFrameBuffer(this, width(), height(), devicePixelRatio());
    m_shadowFrameBuffer.create();
    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);

    m_frameBuffer.resize(w, h, this->devicePixelRatio());
    m_postNoOp.setDimensions(glm::ivec3(w * this->devicePixelRatio(), h * this->devicePixelRatio(), this->devicePixelRatio()));
    m_postBlueTint.setDimensions(glm::ivec3(w * this->devicePixelRatio(), h * this->devicePixelRatio(), this->devicePixelRatio()));
    m_postRedTint.setDimensions(glm::ivec3(w * this->devicePixelRatio(), h * this->devicePixelRatio(), this->devicePixelRatio()));

    m_progSky.setViewProjMatrix(glm::inverse(viewproj));
    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    float dT = (QDateTime::currentMSecsSinceEpoch() - currFrame) / 1000.f;
    m_player.tick(dT, m_inputs);
    currFrame = QDateTime::currentMSecsSinceEpoch();
    m_terrain.tryExpansion(m_player.mcr_position, m_player.mcr_prevPos);
    m_terrain.checkThreadResults();
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
    sendInventoryDataToGUI(); // Update inventory info
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
}

void MyGL::sendInventoryDataToGUI() const {
    emit sig_sendNumGrass(numGrass);
    emit sig_sendNumDirt(numDirt);
    emit sig_sendNumStone(numStone);
    emit sig_sendNumBedrock(numBedrock);
    emit sig_sendNumLava(numLava);
    emit sig_sendNumWater(numWater);
    emit sig_sendNumSnow(numSnow);
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    m_frameBuffer.bindFrameBuffer();
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());

    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progLambert.setCamPos(m_player.mcr_camera.mcr_position);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progInstanced.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progSky.setViewProjMatrix(glm::inverse(m_player.mcr_camera.getViewProj()));
    m_progSky.setEye(m_player.mcr_camera.mcr_position);
    m_progSky.setTime(m_time);
    m_progSky.setDim(glm::vec2(this->width(), this->height()));
    m_progLambert.setEye(m_player.mcr_camera.mcr_position);
    m_progLambert.setTime(m_time);
    //Calculate light dir
    glm::vec3 sunDir = glm::normalize(glm::vec3(cos(m_time * SUN_VELOCITY), sin(m_time * SUN_VELOCITY), 0.f));
    m_progLambert.setLightDir(glm::normalize(glm::vec4(sunDir,0)));
    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-128,128,-128,128,-512,512);
    glm::vec3 cam_basepos = glm::vec3(m_player.mcr_camera.mcr_position.x,128.f,m_player.mcr_camera.mcr_position.z);
    glm::mat4 depthViewMatrix = glm::lookAt(cam_basepos + sunDir, cam_basepos, glm::vec3(0,1,0));
    glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix;
    m_shadowShader.setViewProjMatrix(depthMVP);
    glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
    );
    glm::mat4 depthBiasMVP = biasMatrix*depthMVP;
    m_progLambert.setShadowViewProjMatrix(depthBiasMVP);
    m_postBlueTint.setTime(m_time);
    m_postRedTint.setTime(m_time);
    renderTerrain();

    performPostprocessRenderPass();

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progFlat.draw(m_worldAxes);
    glEnable(GL_DEPTH_TEST);

    m_time ++;
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
    m_shadowFrameBuffer.bindFrameBuffer();
    glViewport(0, 0, 2048,2048);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_shadowShader.drawShadow(m_terrain,int(m_player.mcr_position.x) - 16 - RENDERING_RADIUS, int(m_player.mcr_position.x) + 16 + RENDERING_RADIUS,
                              int(m_player.mcr_position.z) - 16 - RENDERING_RADIUS, int(m_player.mcr_position.z) + 16 + RENDERING_RADIUS);
    m_frameBuffer.bindFrameBuffer();
    glViewport(0, 0, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_progSky.draw(m_geomQuad);
    m_shadowFrameBuffer.bindToTextureSlot(1);
    m_texture.bind(0);
    m_terrain.draw(int(m_player.mcr_position.x) - RENDERING_RADIUS, int(m_player.mcr_position.x) + RENDERING_RADIUS,
                       int(m_player.mcr_position.z) - RENDERING_RADIUS, int(m_player.mcr_position.z) + RENDERING_RADIUS, &m_progLambert);

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

    m_frameBuffer.bindToTextureSlot(2);
    //m_postNoOp.draw(m_geomQuad, 2);

    if (m_player.checkInWaterLow())
    {
        //TODO: blue-tint post-process
        m_postBlueTint.draw(m_geomQuad, 2);
    }
    else if (m_player.checkInLavaLow())
    {
        //TODO: red-tint post-process
        m_postRedTint.draw(m_geomQuad, 2);
    }
    else
    {
        //TODO: noOp post-process
        m_postNoOp.draw(m_geomQuad, 2);
    }

}

void MyGL::initializeTexture()
{
    m_texture.create(":/textures/minecraft_textures_all.png");
    //m_texture.create(":/textures/minecraft_textures_all_grey_grass.png");
    m_texture.load(0);
}

void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    // m_player.mcr_prevPos = m_player.mcr_position;
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
    }
    if (e->key() == Qt::Key_Right) {
        this->m_inputs.rightPressed = true;
    } else if (e->key() == Qt::Key_Left) {
        this->m_inputs.leftPressed = true;
    } else if (e->key() == Qt::Key_Up) {
        this->m_inputs.upPressed = true;
    } else if (e->key() == Qt::Key_Down) {
        this->m_inputs.downPressed = true;
    } else if (e->key() == Qt::Key_W) {
        this->m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        this->m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        this->m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        this->m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_I) {
        openInventory = !openInventory;
        emit sig_inventoryWindow(openInventory);
    }
    if (m_inputs.flightMode) {
        if (e->key() == Qt::Key_Q) {
            m_inputs.qPressed = true;
        } else if (e->key() == Qt::Key_E) {
            m_inputs.ePressed = true;
        }
    } else {
        if (e->key() == Qt::Key_Space) {
            this->currFrame = QDateTime::currentMSecsSinceEpoch();
            m_inputs.spacePressed = true;
        }
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_W) {
        this->m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        this->m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_A) {
        this->m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_D) {
        this->m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_E) {
        this->m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_Q) {
        this->m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_Right) {
        this->m_inputs.rightPressed = false;
    } else if (e->key() == Qt::Key_Left) {
        this->m_inputs.leftPressed = false;
    } else if (e->key() == Qt::Key_Down) {
        this->m_inputs.downPressed = false;
    } else if (e->key() == Qt::Key_Up) {
        this->m_inputs.upPressed = false;
    } else if (e->key() == Qt::Key_Space) {
        this->m_inputs.spacePressed = false;
    } else if (e->key() == Qt::Key_F) {
        if (this->m_inputs.flightMode == false) {
            this->m_inputs.flightMode = true;
        } else {
            this->m_inputs.flightMode = false;
        }
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    const float SENSITIVITY = 50.0;

    float dx = width() * 0.5 - e->pos().x();
    if (dx != 0) {
        m_player.rotateOnUpGlobal(dx/width() * SENSITIVITY);
    }

    float dy = height() * 0.5 - e->pos().y() - 0.5;

    if (dy != 0) {
        m_player.rotateOnRightLocal(dy/height() * SENSITIVITY);
    }

     moveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        BlockType removed = this->m_player.removeBlock(&m_terrain);
                if (removed == GRASS) {
                    numGrass++;
                } else if (removed == DIRT) {
                    numDirt++;
                } else if (removed == STONE) {
                    numStone++;
                } else if (removed == BEDROCK) {
                    numBedrock++;
                } else if (removed == LAVA) {
                    numLava++;
                } else if (removed == WATER) {
                    numWater++;
                } else if (removed == SNOW) {
                    numSnow++;
                }
    }
    if (e->button() == Qt::RightButton) {
        if (currBlockType == GRASS && numGrass > 0 && this->m_player.placeBlock(&m_terrain, GRASS) != EMPTY) {
                    numGrass--;
                } else if (currBlockType == DIRT && numDirt > 0 && this->m_player.placeBlock(&m_terrain, DIRT) != EMPTY) {
                    numDirt--;
                } else if (currBlockType == STONE && numStone > 0 && this->m_player.placeBlock(&m_terrain, STONE) != EMPTY) {
                    numStone--;
                } else if (currBlockType == BEDROCK && numBedrock > 0 && this->m_player.placeBlock(&m_terrain, BEDROCK) != EMPTY) {
                    numBedrock--;
                } else if (currBlockType == LAVA && numLava > 0 && this->m_player.placeBlock(&m_terrain, LAVA) != EMPTY) {
                    numLava--;
                } else if (currBlockType == WATER && numWater > 0 && this->m_player.placeBlock(&m_terrain, WATER) != EMPTY) {
                    numWater--;
                } else if (currBlockType == SNOW && numSnow > 0 && this->m_player.placeBlock(&m_terrain, SNOW) != EMPTY) {
                    numSnow--;
                }
    }
}


