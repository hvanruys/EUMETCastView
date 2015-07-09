#include "texturewriter.h"
#include "segmentimage.h"

extern SegmentImage *imageptrs;

TextureWriter::TextureWriter(QOpenGLShaderProgram *prog)
{

    program = prog;

    initializeOpenGLFunctions();


    program->bind();

    vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    positionsBuf.create();
    positionsBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    positionsBuf.bind();
    vertexPosition = program->attributeLocation("VertexPosition");
    program->enableAttributeArray(vertexPosition);
    program->setAttributeBuffer(vertexPosition, GL_FLOAT, 0, 2);

    texcoordBuf.create();
    texcoordBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    texcoordBuf.bind();
    vertexTexCoord = program->attributeLocation("VertexTexCoord");
    program->enableAttributeArray(vertexTexCoord);
    program->setAttributeBuffer(vertexTexCoord, GL_FLOAT, 0, 1);

    glGenTextures(1, &texturelineId);


}

void TextureWriter::setupBuffers(QVector<GLfloat> positions, QVector<GLfloat> texpositions, QVector<unsigned char> rainbow)
{

    glBindFramebuffer(GL_FRAMEBUFFER, imageptrs->fboId);

    nbrVerticesinBuffer = positions.size()/2;
    positionsBuf.bind();
    positionsBuf.allocate(positions.data(), positions.size() * sizeof(GLfloat));
    positionsBuf.release();

    texcoordBuf.bind();
    texcoordBuf.allocate(texpositions.data(), texpositions.size() * sizeof(GLfloat));
    texcoordBuf.release();

    //glGenTextures(1, &texturelineId);
    glBindTexture(GL_TEXTURE_1D, texturelineId);

    glTexImage1D(GL_TEXTURE_1D, 0, 4, rainbow.size()/3, 0, GL_RGB, GL_UNSIGNED_BYTE, rainbow.data());
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);



}

void TextureWriter::render()
{

    glBindFramebuffer(GL_FRAMEBUFFER, imageptrs->fboId);
    //set the viewport to be the size of the texture
    glViewport(0,0, imageptrs->pmOut->width(), imageptrs->pmOut->height());

    glBindTexture(GL_TEXTURE_1D, texturelineId);
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    program->bind();

    glDrawArrays(GL_LINE_STRIP, 0, nbrVerticesinBuffer);
    //glDrawArrays(GL_TRIANGLES, 0, nbrVertices);
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

TextureWriter::~TextureWriter()
{
    vao.destroy();
    positionsBuf.destroy();
    texcoordBuf.destroy();
}

