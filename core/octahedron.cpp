#include "octahedron.h"
#include "options.h"

extern Options opts;

Octahedron::Octahedron(QOpenGLShaderProgram *prog)
{
    program = prog;
    initializeOpenGLFunctions();

    QVector3D *vertexData = new QVector3D[24];
    vertexData[0] = QVector3D(1.0f, 0.0f, 0.0f);
    vertexData[1] = QVector3D(0.0f, 1.0f, 0.0f);
    vertexData[2] = QVector3D(0.0f, 0.0f, 1.0f);

    vertexData[3] = QVector3D(-1.0f, 0.0f, 0.0f);
    vertexData[4] = QVector3D(0.0f, 0.0f, 1.0f);
    vertexData[5] = QVector3D(0.0f, 1.0f, 0.0f);

    vertexData[6] = QVector3D(-1.0f, 0.0f, 0.0f);
    vertexData[7] = QVector3D(0.0f, 1.0f, 0.0f);
    vertexData[8] = QVector3D(0.0f, 0.0f, -1.0f);

    vertexData[9] = QVector3D(0.0f, 0.0f, -1.0f);
    vertexData[10] = QVector3D(0.0f, 1.0f, 0.0f);
    vertexData[11] = QVector3D(1.0f, 0.0f, 0.0f);

    vertexData[12] = QVector3D(1.0f, 0.0f, 0.0f);
    vertexData[13] = QVector3D(0.0f, 0.0f, 1.0f);
    vertexData[14] = QVector3D(0.0f, -1.0f, 0.0f);

    vertexData[15] = QVector3D(-1.0f, 0.0f, 0.0f);
    vertexData[16] = QVector3D(0.0f, -1.0f, 0.0f);
    vertexData[17] = QVector3D(0.0f, 0.0f, 1.0f);

    vertexData[18] = QVector3D(0.0f, 0.0f, -1.0f);
    vertexData[19] = QVector3D(0.0f, -1.0f, 0.0f);
    vertexData[20] = QVector3D(-1.0f, 0.0f, 0.0f);

    vertexData[21] = QVector3D(0.0f, 0.0f, -1.0f);
    vertexData[22] = QVector3D(1.0f, 0.0f, 0.0f);
    vertexData[23] = QVector3D(0.0f, -1.0f, 0.0f);

    program->bind();

    vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    posBuf.create();
    posBuf.setUsagePattern(QOpenGLBuffer::StaticDraw);
    posBuf.bind();
    posBuf.allocate(vertexData, sizeof(QVector3D) * 24);

    vertexPosition = program->attributeLocation("VertexPosition");
    program->enableAttributeArray(vertexPosition);
    program->setAttributeBuffer(vertexPosition, GL_FLOAT, 0, 3);

    delete [] vertexData;
}

void Octahedron::render(QMatrix4x4 projection, QMatrix4x4 modelview, QColor rendercolor)
{
    program->bind();

    program->setUniformValue("MVP", projection * modelview);
    QMatrix3x3 norm = modelview.normalMatrix();
    program->setUniformValue("NormalMatrix", norm);

    program->setUniformValue("outcolor", QVector4D(rendercolor.redF(), rendercolor.greenF(), rendercolor.blueF(), 1.0f));
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);
    glDrawArrays(GL_TRIANGLES, 0, 24);
}

Octahedron::~Octahedron()
{
    vao.destroy();
    posBuf.destroy();

}

