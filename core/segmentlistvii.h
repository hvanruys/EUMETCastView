#ifndef SEGMENTLISTVII_H
#define SEGMENTLISTVII_H

#include <QObject>
#include "segmentlist.h"
#include "segmentvii.h"

#include "FreeImage.h"

class SatelliteList;

class SegmentListVII : public SegmentList
{
    Q_OBJECT

public:
    SegmentListVII(eSegmentType type = SEG_METOPSGA1, QObject *parent = 0);
    bool ComposeVIIImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    bool ComposeVIIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, int histogrammethod, bool normalized);
    static void doComposeVIIImageInThread(SegmentListVII *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);

    void ComposeGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSGProjection(int inputchannel, int histogrammethod, bool normalized);

    void SmoothVIIImage(bool combine);
    void SmoothVIIImage12bits();
    void setHistogramMethod(int histo, bool normal) { histogrammethod = histo; normalized = normal; }
    bool ChangeHistogramMethod();

    void ComposeSegments();
    void Compose48bitPNG(QString fileName, bool mapto65535);
    void Compose48bitPNGSegment(SegmentVII *segm, FIBITMAP *bitmap, int heightinsegment, bool mapto65535);

    void RecalculateCLAHEVII();
    long NbrOfSaturatedPixels();
    bool searchLatLon(int mapx, int mapy, float &lon, float &lat);

private:
    void CalculateLUT();
    void CalculateLUTAlt();
    void CalculateLUTFull();
    void CalculateProjectionLUT();

    QFutureWatcher<void> *watchervii;

    int histogrammethod;
    bool normalized;

protected slots:
    void finishedvii();
    void progressreadvalue(int progress);

};

#endif // SEGMENTLISTVII_H
