#ifndef AVHRRSEGMENTHRP_H
#define AVHRRSEGMENTHRP_H
#include <QFile>
#include <QImage>

#include "segment.h"


class SegmentHRP : public Segment
{
    Q_OBJECT

public:
    explicit SegmentHRP(QFile *filesegment = 0, QObject *parent = 0);
    ~SegmentHRP();

    void ComposeProjection(int inputchannel, eProjections proj);
    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);
    void RenderSegmentlineInProjectionAlternative(int channel, int nbrLine, int heightintotalimage, QEci eciref, double ang_vel, double pitch, double roll, double yaw, eProjections proj);
    //void RenderSegmentlineInLCCAlternative1(int channel, int heightintotalimage, cEci eciref, double ang_vel);
    void RenderSegmentlineInProjection(int channel, int heightintotalimage, QEci eciref, double ang_vel,eProjections proj);
    void RenderSegmentlineInProjectionCirc(QRgb *row_col, double lat_first, double lon_first, double lat_last, double lon_last, double altitude, eProjections proj);
    //void RenderSegmentlineInLCCcEci( int channel, int heightintotalimage, cEci eciref, double ang_vel);
    bool inspectMPHRrecord(QByteArray mphr_record);


   void RenderScanArea( QPainter *);
    void DrawAreaLine( QPainter *, double, double);
    void AddToSatlist();
    void GetStats();
    int sensing_start_year;

    Segment *ReadSegmentInMemory();
    int ReadNbrOfLines();


private:

    char saveheader;
    quint32 get_next_header(QByteArray ba);

    quint32 state_vector_year;
    quint32 state_vector_month;
    quint32 state_vector_day;
    quint32 state_vector_hour;
    quint32 state_vector_minute;
    quint32 state_vector_second;


    double wcirkel;


    int x_start, y_start;
    int x_end, y_end;
    double epoch;

};

#endif
