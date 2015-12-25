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
    void SmoothProjectionBrightnessTemp();
    float getMinBrightnessTemp() { return minBrightnessTemp; }
    float getMaxBrightnessTemp() { return maxBrightnessTemp; }

    void ComposeGVProjection(int inputchannel);

private:
    void CalculateLUT();
    void CalculateProjectionLUT();
    bool PixelOK(int pix);
    void printData(SegmentVIIRSM *segm, int linesfrom, int viewsfrom);

    void BilinearInterpolationFloat(SegmentVIIRSM *segm);
    void BilinearBetweenSegmentsFloat(SegmentVIIRSM *segmfirst, SegmentVIIRSM *segmnext);
    bool bhm_line_float(int x1, int y1, int x2, int y2, float bt1, float bt2, float *canvas, int *canvas1, int dimx);
    void MapInterpolationFloat(float *canvas, int *canvas1, quint16 dimx, quint16 dimy);
    void MapCanvasFloat(float *canvas, int *canvas1, qint32 anchorX, qint32 anchorY, quint16 dimx, quint16 dimy);

    SatelliteList *satlist;
    int lut[256];
    int earthviews;
    float stat_max_dnb;
    float stat_min_dnb;
    float minBrightnessTemp;
    float maxBrightnessTemp;
    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> inverselist;

protected:

    QFutureWatcher<void> *watcherviirs;



protected slots:
    void finishedviirs();
    void progressreadvalue(int progress);

};

#endif // SEGMENTLISTVIIRS_H
