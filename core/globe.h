#ifndef GLOBE_H
#define GLOBE_H


#define OPENGL32
//#define OPENGL40
//#define OPENGL43

#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>

// Minimum OpenGL version = 3.2
// for glGenVertexArrays glBindVertexArray

#ifdef OPENGL32
#include <QOpenGLFunctions_3_2_Core>
#endif
#ifdef OPENGL40
#include <QOpenGLFunctions_4_0_Core>
#endif
#ifdef OPENGL43
#include <QOpenGLFunctions_4_3_Core>
#endif

#include "trackball.h"
#include "globals.h"
#include "satellite.h"
#include "avhrrsatellite.h"
#include "geometryengine.h"
#include "skybox.h"
#include "soc.h"
#include "satgl.h"
#include "segmentgl.h"
#include "octahedron.h"
#include "projextentsgl.h"
#include "texturewriter.h"

#ifdef OPENGL32
class Globe  : public QOpenGLWidget, protected QOpenGLFunctions_3_2_Core
#endif
#ifdef OPENGL40
class Globe  : public QOpenGLWidget, protected QOpenGLFunctions_4_0_Core
#endif
#ifdef OPENGL43
class Globe  : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
#endif
{

    Q_OBJECT

public:
    Globe(QWidget *parent = NULL, SatelliteList *satlist=0, AVHRRSatellite *seglist=0 );
    void Render3DGeoSegment(eGeoSatellite sat);
    void Render3DGeoSegmentFBO(eGeoSatellite sat);
    void drawSatelliteNames(QPainter *painter, QMatrix4x4 modelview);
    void drawStationNames(QPainter *painter, QMatrix4x4 modelview);
    void printTexture();
    void testfbo();
    void dumpOpenGLdiagnostics();

    ~Globe();

    QSize sizeHint() const;
    int xRotation() const { return xRot; }
    int yRotation() const { return yRot; }
    int zRotation() const { return zRot; }

    void keyPressEvent(QKeyEvent *event);


public slots:
    void Render3DGeo(eGeoSatellite sat);
private slots:
    void slotRender3DGeoFinished();

protected:
    void initializeGL();
    //void paintEvent(QPaintEvent *event);
    void paintGL();
    void resizeGL(int width, int height);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void showEvent(QShowEvent *event);
    void wheelEvent(QWheelEvent * event);

    void initShaders();
    void initTextures();
    void initFrameBuffer();


private:
    void CreateSphere(floatVector c,double r,int n);
    QPointF pixelPosToViewPos(const QPointF& p);
    void showSunPosition( geodetic_t *sungeo );
    void drawInstructions(QPainter *painter);
    QVector2D glhProjectf(QVector3D obj, float *modelview, float *projection, int width, int height);

    void renderStations(QMatrix4x4 projection, QMatrix4x4 modelview, QColor color);
    //void renderActiveSatnames();
    void drawSegmentNames(QPainter *painter, QMatrix4x4 modelview, eSegmentType seg, QList<Segment *> *segptr);

    void mouseDownAction(int x, int y);
    //void displayVector (QVector3D vec);
    //void RenderAllScanAreaGL();
    void TestForSegmentGL( int x, int realy, float distance, const QMatrix4x4 &m);
    void Render3DGeoSegmentLine(int heightinimage, eGeoSatellite geo);
    void Render3DGeoSegmentLineFBO(int heightinimage, eGeoSatellite geo);


    void toggleBorder();
    void togglePhong();
    void toggleSatellites();
    void toggleSegmentNames();

    bool CreateAndLlinkShader(QOpenGLShaderProgram *program, QString vertexshader, QString fragmentshader);
    //void NewFBO();
    bool checkFramebufferStatus();


    QColor qtGreen;
    QColor qtPurple;
    QColor background;
    GLuint object;
    GLfloat xRot, yRot, zRot;

    QPoint lastPos;
    float scaleAll;
    int lineWidth;
    float distance;
    QMatrix4x4 m;
    QMatrix4x4 mt;

    TrackBall trackBall;
    QList<QList<QVector3D>*> veclist;

    int delay;
    SatelliteList *sats;
    AVHRRSatellite *segs;
    bool lighting;

    QOpenGLTexture *textureearth;

    QOpenGLShaderProgram programearthnobump;
    QOpenGLShaderProgram programgshhs;
    QOpenGLShaderProgram programskybox;
    QOpenGLShaderProgram programdraw;
    QOpenGLShaderProgram programsatgl;
    QOpenGLShaderProgram programtexturewriter;

    GeometryEngine *geometries;
    GeometryEngine *sun;
    SkyBox *skybox;
    //Soc *soc;
    SatGL *satgl;
    SegmentGL *segmentgl;
    Octahedron *octa;
    ProjExtentsGL *projextends;
    TextureWriter *texturewriter;

    QMatrix4x4 projection;
    int m_frames;
    QTime m_time;

    bool bBorders;
    bool bSatellites;
    bool bSegmentNames;
    QFutureWatcher<void> watcher;
    QFuture<void> futureRender3DGeo;

    QString segmentnameselected;


signals:
    void mapClicked();
    void renderingglobefinished(bool);

};


#endif // GLOBE_H
