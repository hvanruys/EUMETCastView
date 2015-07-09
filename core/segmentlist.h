#ifndef SEGMENTLIST_H
#define SEGMENTLIST_H

#include <QObject>
#include <QPainter>
#include <QDebug>
#include <QPen>
#include <QFutureWatcher>

class Segment;

class SegmentList : public QObject
{
    Q_OBJECT

public:

    enum eSegments {
        NONE = 0,
        METOP,
        NOAA,
        HRP,
        GAC,
        VIIRS
    };

    explicit SegmentList(QObject *parent = 0);
    void CalculateSunPosition(double first_julian, double last_julian, QVector3D *sunPosition);
    void drawCircle(float cx, float cy, float r, int num_segments);
    void SetNbrOfVisibleSegments(int nbr);
    int GetNbrOfVisibleSegments();
    void SetIndexFirstVisible(int cnt) { indexfirstvisible = cnt; }
    void SetIndexLastVisible(int cnt) { indexlastvisible = cnt; }
    QString GetSatelliteName() { return segmenttype; }
    void SetTotalSegmentsInDirectory(long nbr) { TotalSegmentsInDirectory = nbr; }
    int GetTotalSegmentsInDirectory() { return TotalSegmentsInDirectory; }
    void SetDirectoryName(QString its) { directoryname = its; }
    QString GetDirectoryName() { return directoryname; }
    QList<Segment *> *GetSegmentlistptr(void) { return &segmentlist; }
    QList<Segment *> *GetSegsSelectedptr(void) { return &segsselected; }
    void ClearSegments();
    int NbrOfSegments();
    int NbrOfSegmentsSelected();
    int NbrOfSegmentsShown();
    int NbrOfSegmentLinesSelected();
    // int NbrOfSegmentsReady();
    bool imageMemory();

    void GetFirstLastVisible( double *first_julian,  double *last_julian);
    void GetFirstLastVisible( QDateTime *first_date,  QDateTime *last_date);
    void GetFirstLastVisibleFilename( QString *first_filename,  QString *last_filename);
    //virtual void GetFirstLastVisibleSegmentData(QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast);
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

protected:
    int nbrofvisiblesegments;
    int indexfirstvisible;
    int indexlastvisible;

    QString segmenttype;
    QString directoryname;
    long TotalSegmentsInDirectory;
    long stat_max_ch[5];
    long stat_min_ch[5];

    quint16 lut_ch[5][256];
    int progressresultready; // for progresscounter


signals:
    void segmentlistfinished();
    void segmentprojectionfinished();

public slots:

protected:
    QList<Segment *> segmentlist;
    QList<Segment *> segsselected;
    QFutureWatcher<Segment*> *watcherread;
    QFutureWatcher<Segment*> *watchercompose;
    //QFutureWatcher<Segment*> *watchercomposegvprojection;

    double factor_ch[5];
    float scale;

protected slots:
    void readfinished();
    void composefinished();
    void resultisready(int segmentnbr);
    void resultcomposeisready(int segmentnbr);
//    void composegvpfinished();
//    void composegvpreadyat(int segmentnbr);

signals:
    void progressCounter(int);

};

#endif // SEGMENTLIST_H
