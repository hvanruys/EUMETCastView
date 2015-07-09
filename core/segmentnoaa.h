#ifndef SEGMENTNOAA_H
#define SEGMENTNOAA_H

#include <QFile>
#include <QImage>

#include "satellite.h"
#include "segment.h"

class SegmentNoaa : public Segment
{
    Q_OBJECT

public:
    explicit SegmentNoaa(QFile *filesegment = 0, SatelliteList *satl = 0, QObject *parent = 0);
    Segment *ReadSegmentInMemory();
    int ReadNbrOfLines();
    void ComposeProjection(int inputchannel, eProjections proj);
    void ComposeSegmentLCCProjection(int inputchannel);
    void ComposeSegmentGVProjection(int inputchannel);
    void ComposeSegmentSGProjection(int inputchannel);
    void RenderSegmentlineInProjection(int channel, int nbrLine, int heightintotalimage , QEci eciref, double ang_vel, eProjections proj);
    void RenderSegmentlineInProjectionCirc(QRgb *row_col, double lat_first, double lon_first, double lat_last, double lon_last, double altitude, eProjections proj);
    void RenderSegmentlineInProjectionAlternative(int channel, int nbrLine, int heightintotalimage, QEci eciref, double ang_vel, eProjections proj);

    //int sensing_start_year;



private:



};

#endif // SEGMENTNOAA_H
