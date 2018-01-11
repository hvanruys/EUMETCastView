#ifndef GSHHSDATA_H
#define GSHHSDATA_H

#include "gshhs.h"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>


struct LonLatPair {
        int lonmicro;
        int latmicro;
};

// vector-file
struct VxpFeature {
        int	 nVerts;
        QVector3D *pVerts;
        LonLatPair *pLonLat;
};

struct Vxp {
        int         nFeatures;
        VxpFeature  *pFeatures;
};

class gshhsData : protected QOpenGLFunctions
{
public:
    gshhsData();
    void initializegshhsData(QOpenGLShaderProgram *prog);
    ~gshhsData();

    void render(QMatrix4x4 projection, QMatrix4x4 modelview, int bBorders);

    Vxp	*vxp_data[3];
    Vxp *vxp_data_overlay[3];

private:

    void Initialize(QString data1, QString data2, QString data3, QString dataoverlay1, QString dataoverlay2, QString dataoverlay3);
    void load_gshhs(char *pFileName, int nTotFeatures, Vxp *vxp);
    int check_gshhs(char *pFileName);
    void LonLat2Point(float lat, float lon, QVector3D *pos, float radius);
    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao1;
    QOpenGLVertexArrayObject vao2;
    QOpenGLVertexArrayObject vao3;

    QOpenGLBuffer positionsBuf1;
    QOpenGLBuffer positionsBuf2;
    QOpenGLBuffer positionsBuf3;
    GLuint vertexPosition;
    QVector<GLuint> featurevertsindex[3];

};

#endif // GSHHSDATA_H
