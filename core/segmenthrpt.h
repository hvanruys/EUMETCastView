#ifndef SEGMENTHRPT_H
#define SEGMENTHRPT_H

#include <QObject>
#include <QFile>
#include <QImage>

#include "segment.h"

class SegmentHRPT : public Segment
{
    Q_OBJECT

public:
    explicit SegmentHRPT(eSegmentType type, QFile *filesegment = 0, SatelliteList *satl = 0, QObject *parent = 0);
    void ComposeProjection(int inputchannel, eProjections proj);
    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);
    void RenderSegmentlineInProjectionAlternative(int channel, int nbrLine, int heightintotalimage, QEci eciref, double ang_vel, double pitch, double roll, double yaw, eProjections proj);

    void DecodeHRPTline(QByteArray badata, int heightinsegment);
    void DecodeHRPTDate(QByteArray badata, QSgp4Date &date);

    int ReadNbrOfLines();
    Segment *ReadSegmentInMemory();

    ~SegmentHRPT();

private:
    bool CheckFramesync(QByteArray badata);
    int year;

};

#endif // SEGMENTHRPT_H
