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
    explicit SegmentNoaa(QFile *filesegment = 0, QObject *parent = 0);
    ~SegmentNoaa();

    Segment *ReadSegmentInMemory();
    int ReadNbrOfLines();
    void ComposeProjection(int inputchannel, eProjections proj);
    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);
    void RenderSegmentlineInProjection(int channel, int nbrLine, int heightintotalimage , QEci eciref, double ang_vel, eProjections proj);
    void RenderSegmentlineInProjectionCirc(QRgb *row_col, int nbrLine, double lat_first, double lon_first, double lat_last, double lon_last, double altitude, eProjections proj);
    void RenderSegmentlineInProjectionAlternative(int channel, int nbrLine, int heightintotalimage, QEci eciref, double ang_vel, eProjections proj);

    //int sensing_start_year;
private:

};

#endif // SEGMENTNOAA_H
