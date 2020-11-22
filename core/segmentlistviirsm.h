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
    SegmentListVIIRSM(SatelliteList *satl = 0, eSegmentType type = eSegmentType::SEG_NONE, QObject *parent = 0);
    bool ComposeVIIRSImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);

    void ShowImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    void SmoothVIIRSImage(bool combine);
    void SmoothVIIRSImage12bits();
    void SmoothProjectionBrightnessTemp();
    float getMinBrightnessTemp() { return minBrightnessTemp; }
    float getMaxBrightnessTemp() { return maxBrightnessTemp; }
    float getMinBrightnessTempProjection() { return minBrightnessTempProjection; }
    float getMaxBrightnessTempProjection() { return maxBrightnessTempProjection; }

    void ComposeGVProjection(int inputchannel);
    void ComposeLCCProjection(int inputchannel);
    void ComposeSGProjection(int inputchannel);
    void ComposeOMProjection(int inputchannel);

    void GetCentralCoords(double *startcentrallon, double *startcentrallat, double *endcentrallon, double *endcentrallat);
    void GetCornerCoords(double *cornerlon1, double *cornerlat1, double *cornerlon2, double *cornerlat2, double *cornerlon3, double *cornerlat3, double *cornerlon4, double *cornerlat4);

    static void doComposeVIIRSMImageInThread(SegmentListVIIRSM *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);

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
    void initBrightnessTemp();

    bool ComposeVIIRSImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);


    SatelliteList *satlist;
    int lut[256];
    int earthviews;
    float stat_max_dnb;
    float stat_min_dnb;
    float minBrightnessTemp;
    float maxBrightnessTemp;
    float minBrightnessTempProjection;
    float maxBrightnessTempProjection;


protected:

    QFutureWatcher<void> watcherviirs;



protected slots:
    void finishedviirs();
    void progressreadvalue(int progress);

};

#endif // SEGMENTLISTVIIRS_H
