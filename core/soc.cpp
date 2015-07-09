#include "soc.h"

Soc::Soc(QOpenGLShaderProgram *prog)
{
    // experiment : not using QOpenGLFunctions
    //
    glf = QOpenGLContext::currentContext()->functions();
    program = prog;
    initializeSoc();

}

void Soc::initializeSoc()
{

    vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    QVector<GLfloat> possoc;
    possoc.append(0.0f);
    possoc.append(0.0f);
    possoc.append(0.0f);

    possoc.append(0.0f);
    possoc.append(1.5f);
    possoc.append(0.0f);

    possoc.append(0.0f);
    possoc.append(0.0f);
    possoc.append(0.0f);

    possoc.append(1.5f);
    possoc.append(0.0f);
    possoc.append(0.0f);

    possoc.append(0.0f);
    possoc.append(0.0f);
    possoc.append(0.0f);

    possoc.append(0.0f);
    possoc.append(0.0f);
    possoc.append(1.5f);


    socBuf.create();
    socBuf.setUsagePattern(QOpenGLBuffer::StaticDraw);
    socBuf.bind();
    socBuf.allocate(possoc.data(), possoc.size() * sizeof(GLfloat));

    // Bind shader pipeline for use
    program->bind();
    GLuint vertexPosition = program->attributeLocation("VertexPosition");
    program->enableAttributeArray(vertexPosition);
    program->setAttributeBuffer(vertexPosition, GL_FLOAT, 0, 3);

}

Soc::~Soc()
{
    vao.destroy();
    socBuf.destroy();

}

void Soc::render(QMatrix4x4 projection, QMatrix4x4 modelview)
{

    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    program->bind();
    program->setUniformValue("MVP", projection * modelview);
    program->setUniformValue("outcolor", QVector4D(1.0f, 1.0f, 0.0f, 1.0f));
    QMatrix3x3 norm = modelview.normalMatrix();
    program->setUniformValue("NormalMatrix", norm);

//    glf->glDrawArrays(GL_LINE_STRIP, 0, 6);
    glf->glDrawArrays(GL_LINES, 0, 6);

}
