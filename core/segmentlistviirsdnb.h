#ifndef SEGMENTLISTVIIRSDNB_H
#define SEGMENTLISTVIIRSDNB_H

#include <QObject>
#include "segmentviirsdnb.h"
#include "segmentlist.h"

class SatelliteList;
class SegmentListVIIRSDNB  : public SegmentList
{
        Q_OBJECT

public:
    SegmentListVIIRSDNB(SatelliteList *satl = 0, QObject *parent = 0);
    //bool ComposeVIIRSImageConcurrent(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    //bool ComposeVIIRSImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    bool ComposeVIIRSImageInThread();
    bool ComposeVIIRSImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    void ComposeGVProjection(int inputchannel);

    //bool ShowImage(QList<bool> bandlist, QList<int> colorlist);
    void ShowImageSerial();
    void SmoothVIIRSImage(bool combine);
    //static void doReadSegmentInMemoryVIIRS(Segment *t);
    //static void doComposeSegmentImageVIIRS(Segment *t);
    //static void doComposeProjection(Segment *t);
    void sliderCentreBandChanged(int val);
    void spbWindowValueChanged(int spbwindowval, int slcentreband);
    float getMoonIllumination() { return moonillumination; }

private:
    void CalculateLUT();
    bool PixelOK(int pix);
    void printData(SegmentVIIRSDNB *segm, int linesfrom, int viewsfrom);

    SatelliteList *satlist;
    int lut[256];
    int earthviews;
    float stat_max_dnb;
    float stat_min_dnb;
    float moonillumination;

protected:
    QFutureWatcher<void> *watcherviirs;

protected slots:
    void finishedviirs();
    void progressreadvalue(int progress);
};

#endif // SEGMENTLISTVIIRS_H
