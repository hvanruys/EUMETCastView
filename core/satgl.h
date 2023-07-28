#ifndef SATGL_H
#define SATGL_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "satellite.h"
#include "avhrrsatellite.h"
#include "octahedron.h"
#include "sathorizon.h"

class SatGL  : protected QOpenGLFunctions
{
public:
    SatGL(QOpenGLShaderProgram *prog, AVHRRSatellite *seglist=0 );
    void render(QMatrix4x4 projection, float dist, QQuaternion quat);

    ~SatGL();

private:
    void RenderTrail(Satellite *sat, QMatrix4x4 projection, float distance, QQuaternion quat, bool trackon);
    void showSatHorizon(Satellite *sat, QMatrix4x4 projection, float distance, QQuaternion quat);
    void drawCircle(float cx, float cy, float r, int num_segments);

    AVHRRSatellite *segs;
    GLuint vertexPosition;
    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer positionsBuf;
    QOpenGLVertexArrayObject vaotrail;
    QOpenGLBuffer positionsTrail;
    Octahedron *octa;
    SatHorizon *horizon;
    int nbrActiveSats;
    int tdiff;


};

#endif // SATGL_H
