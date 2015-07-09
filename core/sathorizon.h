#ifndef SATHORIZON_H
#define SATHORIZON_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QColor>

#include "qsgp4globals.h"
#define SEGMENTS 128

class SatHorizon : protected QOpenGLFunctions
{
public:
    SatHorizon(QOpenGLShaderProgram *prog);
    void render(QMatrix4x4 projection, float distance, QQuaternion quat, QVector3D posnorm, float alt, QColor rendercolor);
    ~SatHorizon();
private:
    void drawCircle(float cx, float cy, float r, int num_segments);
    void createCircleBuffer(float r, int num_segments);

    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer posBuf;
    int vertexPosition;
    QVector3D vertexData[SEGMENTS];
};

#endif // SATHORIZON_H
