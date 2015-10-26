#ifndef SEGMENTLISTVIIRS_H
#define SEGMENTLISTVIIRS_H

#include <QObject>
#include "segmentviirsm.h"
#include "segmentlist.h"

class SatelliteList;
class SegmentListVIIRS  : public SegmentList
{
        Q_OBJECT

public:
    SegmentListVIIRS(SatelliteList *satl = 0, QObject *parent = 0, eSegmentType type = eSegmentType::SEG_VIIRSM);
    void GetFirstLastVisibleSegmentData(QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast);
    //bool ComposeVIIRSImageConcurrent(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    //bool ComposeVIIRSImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    bool ComposeVIIRSMImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    bool ComposeVIIRSDNBImageInThread();
    bool ComposeVIIRSImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);

    //bool ShowImage(QList<bool> bandlist, QList<int> colorlist);
    void ShowImageSerialM(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    void ShowImageSerialDNB();
    void SmoothVIIRSImage();
    //static void doReadSegmentInMemoryVIIRS(Segment *t);
    //static void doComposeSegmentImageVIIRS(Segment *t);
    //static void doComposeProjection(Segment *t);
    void sliderCentreBandChanged(int val);
    void spbWindowValueChanged(int spbwindowval, int slcentreband);


private:
    void CalculateLUT();
    void CalculateLUTDNB();
    bool PixelOK(int pix);
    void printData(SegmentVIIRSM *segm, int linesfrom, int viewsfrom);

    SatelliteList *satlist;
    int lut[256];
    int earthviews;
    float stat_max_dnb;
    float stat_min_dnb;


protected:

    QFutureWatcher<void> *watcherreadviirs;
    QFutureWatcher<void> *watchercomposeviirs;

protected slots:
    void readfinishedviirs();
    void readfinishedviirsdnb();
    void composefinishedviirs();
    void progressreadvalue(int progress);

signals:
    void progressCounter(int);


};

#endif // SEGMENTLISTVIIRS_H
