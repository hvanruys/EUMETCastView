#include "sathorizon.h"

SatHorizon::SatHorizon(QOpenGLShaderProgram *prog)
{
    program = prog;
    initializeOpenGLFunctions();

    program->bind();

    vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    posBuf.create();
    posBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    posBuf.bind();
    posBuf.allocate(sizeof(QVector3D) * SEGMENTS);

    vertexPosition = program->attributeLocation("VertexPosition");
    program->enableAttributeArray(vertexPosition);
    program->setAttributeBuffer(vertexPosition, GL_FLOAT, 0, 3);

}

void SatHorizon::createCircleBuffer(float r, int num_segments)
{

    float theta = 2 * PI / float(num_segments);
    float c = cosf(theta);//precalculate the sine and cosine
    float s = sinf(theta);
    float t;

    float x = r;//we start at angle = 0
    float y = 0;

    for(int i = 0; i < num_segments; i++)
    {
        vertexData[i] = QVector3D(x, y, 0.0f);

        //apply the rotation matrix
        t = x;
        x = c * x - s * y;
        y = s * t + c * y;
    }
}

void SatHorizon::render(QMatrix4x4 projection, float distance, QQuaternion quat, QVector3D posnorm, float alt, QColor rendercolor)
{

    float radius = sqrt( alt * alt - 1 ) / alt;

    float theta = acos(QVector3D::dotProduct(QVector3D(0,0,1), posnorm));
    QVector3D vecnorm = QVector3D::crossProduct(QVector3D(0,0,1), posnorm);
    vecnorm.normalize();

    createCircleBuffer(radius, SEGMENTS);

    QMatrix4x4 modelview;
    modelview.translate(0.0, 0.0, distance);
    modelview.rotate(quat);

    modelview.translate(posnorm * (1/alt) * (alt > 1.5 ? 1.0015 : 1.0001));
    modelview.rotate(theta * 180.0f/ PI, vecnorm );

    posBuf.bind();
    posBuf.write(0, vertexData, SEGMENTS * sizeof(QVector3D));
    posBuf.release();

    program->bind();

    program->setUniformValue("MVP", projection * modelview);
    QMatrix3x3 norm = modelview.normalMatrix();
    program->setUniformValue("NormalMatrix", norm);

    program->setUniformValue("outcolor", QVector4D(rendercolor.redF(), rendercolor.greenF(), rendercolor.blueF(), 1.0f));
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);
    glDrawArrays(GL_LINE_LOOP, 0, SEGMENTS);

}

SatHorizon::~SatHorizon()
{
    vao.destroy();
    posBuf.destroy();
 }

