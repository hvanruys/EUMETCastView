#ifndef SEGMENTLISTVIIRSM_H
#define SEGMENTLISTVIIRSM_H

#include <QObject>
#include "segmentviirsm.h"
#include "segmentlist.h"

class SatelliteList;
class SegmentListVIIRSM  : public SegmentList
{
        Q_OBJECT

public:
    SegmentListVIIRSM(SatelliteList *satl = 0, QObject *parent = 0);
    bool ComposeVIIRSImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    bool ComposeVIIRSImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);

    void ShowImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    void SmoothVIIRSImage(bool combine);
    void ComposeGVProjection(int inputchannel);

private:
    void CalculateLUT();
    void CalculateProjectionLUT();
    bool PixelOK(int pix);
    void printData(SegmentVIIRSM *segm, int linesfrom, int viewsfrom);

    SatelliteList *satlist;
    int lut[256];
    int earthviews;
    float stat_max_dnb;
    float stat_min_dnb;

protected:

    QFutureWatcher<void> *watcherviirs;

protected slots:
    void finishedviirs();
    void progressreadvalue(int progress);

signals:
    void progressCounter(int);

};

#endif // SEGMENTLISTVIIRS_H
