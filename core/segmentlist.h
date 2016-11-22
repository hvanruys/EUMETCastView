#ifndef SEGMENTLIST_H
#define SEGMENTLIST_H

#include <QObject>
#include <QPainter>
#include <QDebug>
#include <QPen>
#include <QFutureWatcher>
#include "globals.h"

class Segment;

class SegmentList : public QObject
{
    Q_OBJECT

public:

    explicit SegmentList(QObject *parent = 0);
    void CalculateSunPosition(double first_julian, double last_julian, QVector3D *sunPosition);
    void SetNbrOfVisibleSegments(int nbr);
    int GetNbrOfVisibleSegments();
    void SetIndexFirstVisible(int cnt) { indexfirstvisible = cnt; }
    void SetIndexLastVisible(int cnt) { indexlastvisible = cnt; }
    void SetTotalSegmentsInDirectory(long nbr) { TotalSegmentsInDirectory = nbr; }
    int GetTotalSegmentsInDirectory() { return TotalSegmentsInDirectory; }
    void SetDirectoryName(QString its) { directoryname = its; }
    QString GetDirectoryName() { return directoryname; }
    QList<Segment *> *GetSegmentlistptr(void) { return &segmentlist; }
    QList<Segment *> *GetSegsSelectedptr(void) { return &segsselected; }
    eSegmentType GetSegmentType() { return seglisttype; }
    void ClearSegments();
    int NbrOfSegments();
    int NbrOfSegmentsSelected();
    int NbrOfSegmentsShown();
    int NbrOfSegmentLinesSelected();

    void GetFirstLastVisible( double *first_julian,  double *last_julian);
    void GetFirstLastVisible( QDateTime *first_date,  QDateTime *last_date);
    void GetFirstLastVisibleFilename( QString *first_filename,  QString *last_filename);
    void RenderEarthLocationsGL();
    void ShowSegment(int value);
    bool TestForSegmentGL(int x, int realy, float distance, const QMatrix4x4 &m, bool showallsegments, QString &segmentname);
    void ShowWinvec(QPainter *painter, float distance, const QMatrix4x4 modelview);

    bool ComposeImage(double gamma[]);
    void ComposeImage1();
    bool TestForSegment(double *deg_lon, double *deg_lat, bool leftbuttondown, bool showallsegments);
    void RenderSegments(QPainter *painter, QColor col, bool renderall);
    int NbrOfEartviewsPerScanline();
    void ComposeGVProjection(int inputchannel);
    void ComposeLCCProjection(int inputchannel);
    void ComposeSGProjection(int inputchannel);
    void SmoothProjectionImageBilinear();
    void SmoothProjectionImageBicubic();

    static void doReadSegmentInMemory(Segment *t);
    static void doComposeSegmentImage(Segment *t);
    static void doComposeGVProjection(Segment *t);

protected:
    void BilinearInterpolation(Segment *segm, bool combine);
    void BilinearBetweenSegments(Segment *segmfirst, Segment *segmnext, bool combine);
    bool bhm_line(int x1, int y1, int x2, int y2, QRgb rgb1, QRgb rgb2, QRgb *canvas, int dimx);
    void MapInterpolation(QRgb *canvas, quint16 dimx, quint16 dimy);
    void MapInterpolation1(QRgb *canvas, quint16 dimx, quint16 dimy);
    void MapCanvas(QRgb *canvas, qint32 anchorX, qint32 anchorY, quint16 dimx, quint16 dimy, bool combine);

    double cubicInterpolate (double p[4], double x);
    double bicubicInterpolate (double p[4][4], double x, double y);

    qint32 Min(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);
    qint32 Max(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);

    int nbrofvisiblesegments;
    int indexfirstvisible;
    int indexlastvisible;
    int earth_views_per_scanline;

    QString segmenttype;
    eSegmentType seglisttype;

    QString directoryname;
    long TotalSegmentsInDirectory;
    long stat_max_ch[5];
    long stat_min_ch[5];

//    long stat_3_0_max_ch;
//    long stat_3_1_max_ch;
//    long stat_3_0_min_ch;
//    long stat_3_1_min_ch;


    quint16 lut_ch[5][256];
    int progressresultready; // for progresscounter
    int projectioninputchannel;
    bool channel_3_select;

signals:
    void segmentlistfinished(bool settoolboxbuttons);
    void segmentprojectionfinished(bool settoolboxbuttons);
    void progressCounter(int);

public slots:

protected:
    QList<Segment *> segmentlist;
    QList<Segment *> segsselected;
    QFutureWatcher<void> *watcherread;
    QFutureWatcher<void> *watchercompose;
    //QFutureWatcher<void> *watchercomposeprojection;

    double factor_ch[5];
    float scale;

protected slots:
    void readfinished();
    void composefinished();
    void progressvaluechanged(int segmentnbr);
    void resultcomposeisready(int segmentnbr);
//    void composeprojectionfinished();
    void composeprojectionreadyat(int segmentnbr);


};

#endif // SEGMENTLIST_H
