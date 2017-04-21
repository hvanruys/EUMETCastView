#ifndef SEGMENTLISTOLCI_H
#define SEGMENTLISTOLCI_H

#include <QObject>
#include "segmentlist.h"
#include "segmentolci.h"

#include "FreeImage.h"

class SatelliteList;

class SegmentListOLCI : public SegmentList
{
    Q_OBJECT

public:
    SegmentListOLCI(eSegmentType type = SEG_OLCIEFR, SatelliteList *satl = 0, QObject *parent = 0);
    bool ComposeOLCIImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles);
    bool ComposeOLCIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles);

    void ComposeGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSGProjection(int inputchannel, int histogrammethod, bool normalized);

    void SmoothOLCIImage(bool combine);
    void SmoothOLCIImage12bits();
    void ShowWinvec(QPainter *painter, float distance, const QMatrix4x4 modelview);
//    bool TestForSegmentGLextended(int x, int realy, float distance, const QMatrix4x4 &m, bool showallsegments, QString &segmentname);
    void setHistogramMethod(int histo, bool normal) { histogrammethod = histo; normalized = normal;}
    bool ChangeHistogramMethod();

    void ComposeSegments();
    void Compose48bitPNG(QString fileName, bool mapto65535);
    void Compose48bitPNGSegment(SegmentOLCI *segm, FIBITMAP *bitmap, int heightinsegment, bool mapto65535);
    //void Compose48bitProjectionPNG(QString fileName, bool mapto65535);


    void RecalculateCLAHEOLCI();
    long NbrOfSaturatedPixels();
    bool searchLatLon(int mapx, int mapy, float &lon, float &lat);


private:

    void CalculateLUT();
    void CalculateLUTAlt();
    void CalculateLUTFull();
    void CalculateProjectionLUT();
    SatelliteList *satlist;

//    QList<bool> bandlist;
//    QList<int> colorlist;
//    QList<bool> inverselist;

    int histogrammethod;
    bool normalized;

protected:
    QFutureWatcher<void> *watcherolci;

protected slots:
    void finishedolci();
    void progressreadvalue(int progress);

};

#endif // SEGMENTLISTOLCI_H
