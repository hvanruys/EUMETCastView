#ifndef SEGMENTGL_H
#define SEGMENTGL_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "satellite.h"
#include "avhrrsatellite.h"


class SegmentGL  : protected QOpenGLFunctions
{
public:
    SegmentGL(QOpenGLShaderProgram *prog, AVHRRSatellite *seglist=0 );
    void render(QMatrix4x4 projection, float dist, QQuaternion quat, int width, int height);

    ~SegmentGL();

private:
    void RenderContour(Segment *seg, QMatrix4x4 projection, QMatrix4x4 modelview, int width, int height);
    void RenderContourDetail(Segment *seg, QMatrix4x4 projection, QMatrix4x4 modelview, int width, int height);
    void RenderContourXML(Segment *seg, QMatrix4x4 projection, QMatrix4x4 modelview, int width, int height);

    void CalculateSegmentContour(QVector<GLfloat> *positions, float lat_first, float lon_first, float lat_last, float lon_last, int howdetailed);
    void CalculateSegmentContour(QVector<GLfloat> *positions, QGeodetic first, QGeodetic last, int howdetailed);

    QVector2D glhProjectf(QVector3D obj, float *modelview, float *projection, int width, int height);

    AVHRRSatellite *segs;
    GLuint vertexPosition;
    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer positionsBuf;
    QOpenGLVertexArrayObject vaotrail;
    QOpenGLBuffer positionsTrail;
    int nbrOfVertices;
    int howdetailed;

};

#endif // SEGMENTGL_H
