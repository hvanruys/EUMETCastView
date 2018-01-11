#ifndef PROJEXTENTSGL_H
#define PROJEXTENTSGL_H

#include "qsgp4globals.h"
#include "globals.h"
#include "sathorizon.h"
#include "cylequidist.h"
#include "octahedron.h"


#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>


class ProjExtentsGL  : protected QOpenGLFunctions
{
public:
    ProjExtentsGL(QOpenGLShaderProgram *prog );
    void renderLCC(QMatrix4x4 projection, float dist, QQuaternion quat);
    void renderLCCtest(QMatrix4x4 projection, float dist, QQuaternion quat);
    void renderGVP(QMatrix4x4 projection, float dist, QQuaternion quat);

    ~ProjExtentsGL();

private:
    void RenderContour(QMatrix4x4 projection, QMatrix4x4 modelview);
    void CalculateGreatCircleArc(QVector<GLfloat> *positions, float lat_first, float lon_first, float lat_last, float lon_last);
    void CalculateSmallCircleArc(QVector<GLfloat> *positions, float lon_first, float lon_last, float lat);

    GLuint vertexPosition;
    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer positionsBuf;
    int nbrOfVertices;
    SatHorizon *horizon;
    Octahedron *octa;

};

#endif // PROJEXTENTSGL_H
