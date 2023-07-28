#ifndef SEGMENTLISTVIIRSDNB_H
#define SEGMENTLISTVIIRSDNB_H

#include <QObject>
#include "segmentviirsdnb.h"
#include "segmentlist.h"
#include "qcustomplot.h"

class SatelliteList;
class SegmentListVIIRSDNB  : public SegmentList
{
        Q_OBJECT

public:
    SegmentListVIIRSDNB(eSegmentType type = eSegmentType::SEG_NONE, QObject *parent = 0);
    bool ComposeVIIRSImageInThread();
    bool ComposeVIIRSImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    void ShowImageSerial();
    void SmoothVIIRSImage(bool combine);
    void sliderCentreBandChanged(int val);
    void spbWindowValueChanged(int spbwindowval, int slcentreband);
    float getMoonIllumination() { return moonillumination; }
    void GetCentralCoords(double *startcentrallon, double *startcentrallat, double *endcentrallon, double *endcentrallat);

    static void doComposeVIIRSDNBImageInThread(SegmentListVIIRSDNB *t);
    QScopedArrayPointer<long> graphvalues;
    QVector<double> xDNBcurve;
    QVector<double> yDNBcurve;

private:
    void CalculateLUT();
    bool PixelOK(int pix);
    void printData(SegmentVIIRSDNB *segm, int linesfrom, int viewsfrom);
    void fitDNBCurve();

    int lut[256];
    int earthviews;
    float stat_max_dnb;
    float stat_min_dnb;
    float moonillumination;

protected:
    QFutureWatcher<void> watcherviirs;

protected slots:
    void finishedviirs();
    void progressreadvalue(int progress);

signals:
    void displayDNBGraph();
};

#endif // SEGMENTLISTVIIRS_H
