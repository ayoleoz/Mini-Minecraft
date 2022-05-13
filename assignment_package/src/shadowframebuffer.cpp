#include "shadowframebuffer.h"
#include <iostream>

ShadowFrameBuffer::ShadowFrameBuffer(OpenGLContext *context,
                                     unsigned int width, unsigned int height, unsigned int devicePixelRatio)
                : mp_context(context), m_frameBuffer(-1),
                  m_outputTexture(-1),
                  m_width(width), m_height(height), m_devicePixelRatio(devicePixelRatio), m_created(false)
            {}

void ShadowFrameBuffer::resize(unsigned int width, unsigned int height, unsigned int devicePixelRatio) {
    m_width = width;
    m_height = height;
    m_devicePixelRatio = devicePixelRatio;
}

void ShadowFrameBuffer::create() {
    // Initialize the frame buffers and render textures
    mp_context->glGenFramebuffers(1, &m_frameBuffer);
    mp_context->glGenTextures(1, &m_outputTexture);

    mp_context->glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Bind our texture so that all functions that deal with textures will interact with this one
    mp_context->glBindTexture(GL_TEXTURE_2D, m_outputTexture);
    // Give an empty image to OpenGL ( the last "0" )
    mp_context->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    // Set the render settings for the texture we've just created.
    // Essentially zero filtering on the "texture" so it appears exactly as rendered
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // Clamp the colors at the edge of our texture
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set m_renderedTexture as the color output of our frame buffer
    mp_context->glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_outputTexture, 0);

    // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0,
    // which we previously set to m_renderedTexture
    GLenum drawBuffers[1] = {GL_NONE};
    mp_context->glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers

    m_created = true;
    if(mp_context->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        m_created = false;
        std::cout << "Frame buffer did not initialize correctly..." << std::endl;
        mp_context->printGLErrorLog();
    }
}

void ShadowFrameBuffer::destroy() {
    if(m_created) {
        m_created = false;
        mp_context->glDeleteFramebuffers(1, &m_frameBuffer);
        mp_context->glDeleteTextures(1, &m_outputTexture);
    }
}

void ShadowFrameBuffer::bindFrameBuffer() {
    mp_context->glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
}

void ShadowFrameBuffer::bindToTextureSlot(unsigned int slot) {
    m_textureSlot = slot;
    mp_context->glActiveTexture(GL_TEXTURE0 + slot);
    mp_context->glBindTexture(GL_TEXTURE_2D, m_outputTexture);
}

unsigned int ShadowFrameBuffer::getTextureSlot() const {
    return m_textureSlot;
}
