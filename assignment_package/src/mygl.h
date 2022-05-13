#ifndef MYGL_H
#define MYGL_H

#include "openglcontext.h"
#include "shaderprogram.h"
#include "shadowshader.h"
#include "postprocessshader.h"
#include "scene/worldaxes.h"
#include "scene/camera.h"
#include "scene/terrain.h"
#include "scene/player.h"
#include "scene/quad.h"
#include "framebuffer.h"
#include "shadowframebuffer.h"
#include "texteure.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <smartpointerhelp.h>
#include <qdatetime.h>

class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    // The screen-space quadrangle used to draw
    // the scene with the post-process shaders.
    Quad m_geomQuad;

    FrameBuffer m_frameBuffer;
    ShadowFrameBuffer m_shadowFrameBuffer;
    ShadowShader m_shadowShader;

    WorldAxes m_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram m_progInstanced;// A shader program that is designed to be compatible with instanced rendering
    ShaderProgram m_progSky; // A shader program that draws the sky box(day and night cycle)
    PostProcessShader m_postNoOp;
    PostProcessShader m_postBlueTint;
    PostProcessShader m_postRedTint;

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Terrain m_terrain; // All of the Chunks that currently comprise the world.
    Player m_player; // The entity controlled by the user. Contains a camera to display what it sees as well.
    InputBundle m_inputs; // A collection of variables to be updated in keyPressEvent, mouseMoveEvent, mousePressEvent, etc.

    QTimer m_timer; // Timer linked to tick(). Fires approximately 60 times per second.
    Texture m_texture;
    int m_time; //A variable that increases every time paintGL is called

    void moveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

    void sendPlayerDataToGUI() const;
    void sendInventoryDataToGUI() const;

    qint64 prevFrame;
    qint64 currFrame;

public:
    bool openInventory;
        int numGrass;
        int numDirt;
        int numStone;
        int numWater;
        int numLava;
        int numBedrock;
        int numSnow;
        // initial block type holding
        BlockType currBlockType;

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

    void performPostprocessRenderPass();
    void performSkyRender();
    void initializeTexture();
protected:
    // Automatically invoked when the user
    // presses a key on the keyboard
    void keyPressEvent(QKeyEvent *e) override;
    // Automatically invoked when the user
    // releases a key on the keyboard
    void keyReleaseEvent(QKeyEvent *e) override;
    // Automatically invoked when the user
    // moves the mouse
    void mouseMoveEvent(QMouseEvent *e) override;
    // Automatically invoked when the user
    // presses a mouse button
    void mousePressEvent(QMouseEvent *e) override;

private slots:
    void tick(); // Slot that gets called ~60 times per second by m_timer firing.

signals:
    void sig_sendPlayerPos(QString) const;
    void sig_sendPlayerVel(QString) const;
    void sig_sendPlayerAcc(QString) const;
    void sig_sendPlayerLook(QString) const;
    void sig_sendPlayerChunk(QString) const;
    void sig_sendPlayerTerrainZone(QString) const;
    void sig_inventoryWindow(bool);
        void sig_sendNumGrass(int) const;
        void sig_sendNumDirt(int) const;
        void sig_sendNumStone(int) const;
        void sig_sendNumBedrock(int) const;
        void sig_sendNumLava(int) const;
        void sig_sendNumWater(int) const;
        void sig_sendNumSnow(int) const;
};


#endif // MYGL_H
