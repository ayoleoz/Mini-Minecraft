#ifndef SHADOWFRAMEBUFFER_H
#define SHADOWFRAMEBUFFER_H
#include "openglcontext.h"
#include "glm_includes.h"

class ShadowFrameBuffer
{
private:
    OpenGLContext *mp_context;
    GLuint m_frameBuffer;
    GLuint m_outputTexture;

    unsigned int m_width, m_height, m_devicePixelRatio;
    bool m_created;

    unsigned int m_textureSlot;

public:
    ShadowFrameBuffer(OpenGLContext *context, unsigned int width, unsigned int height, unsigned int devicePixelRatio);
    // Make sure to call resize from MyGL::resizeGL to keep your frame buffer up to date with
    // your screen dimensions
    void resize(unsigned int width, unsigned int height, unsigned int devicePixelRatio);
    // Initialize all GPU-side data required
    void create();
    // Deallocate all GPU-side data
    void destroy();
    void bindFrameBuffer();
    // Associate our output texture with the indicated texture slot
    void bindToTextureSlot(unsigned int slot);
    unsigned int getTextureSlot() const;
};

#endif // SHADOWFRAMEBUFFER_H
