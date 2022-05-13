#include "postprocessshader.h"
#include <QDateTime>
#include <iostream>
PostProcessShader::PostProcessShader(OpenGLContext *context)
    : ShaderProgram(context),
      attrPos(-1), attrUV(-1),
      unifDimensions(-1)
{}

PostProcessShader::~PostProcessShader()
{}

void PostProcessShader::setupMemberVars()
{
    ShaderProgram::setupMemberVars();
    attrPos = context->glGetAttribLocation(prog, "vs_Pos");
    attrUV  = context->glGetAttribLocation(prog, "vs_UV");

    unifSampler2D = context->glGetUniformLocation(prog, "u_RenderedTexture");
    unifDimensions = context->glGetUniformLocation(prog, "u_Dimensions");
    unifTime = context->glGetUniformLocation(prog, "u_Time");
}

void PostProcessShader::draw(Drawable& d, int textureSlot = 0)
{
    useMe();

    // Set our "renderedTexture" sampler to user Texture Unit 0
    //std::cout<< textureSlot << std::endl;
    context->glUniform1i(unifSampler2D, textureSlot);

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    if (attrPos != -1 && d.bindPos()) {
        context->glEnableVertexAttribArray(attrPos);
        context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 0, NULL);
    }

    // TODO: change d.bindUV() to d.bindCol()
    //d.bindUV();
    if (attrUV != -1 && d.bindUV()) {
        context->glEnableVertexAttribArray(attrUV);
        context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false, 0, NULL);
    }

    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdx();
    context->glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0);

    if (attrPos != -1) context->glDisableVertexAttribArray(attrPos);
    if (attrUV != -1) context->glDisableVertexAttribArray(attrUV);

    context->printGLErrorLog();
}


void PostProcessShader::setDimensions(glm::ivec3 dims)
{
    useMe();

    if(unifDimensions != -1)
    {
        context->glUniform3i(unifDimensions, dims.x, dims.y, dims.z);
    }
}
