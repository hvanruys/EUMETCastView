#ifndef SEGMENTLISTVIIRS_H
#define SEGMENTLISTVIIRS_H

#include <QObject>
#include "segmentviirs.h"
#include "segmentlist.h"

class SatelliteList;
class SegmentListVIIRS  : public SegmentList
{
        Q_OBJECT

public:
    SegmentListVIIRS(SatelliteList *satl = 0, QObject *parent = 0);
    void GetFirstLastVisibleSegmentData(QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast);
    //bool ComposeVIIRSImageConcurrent(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    //bool ComposeVIIRSImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    bool ComposeVIIRSImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    bool ComposeVIIRSImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);

    //bool ShowImage(QList<bool> bandlist, QList<int> colorlist);
    void ShowImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    void SmoothVIIRSImage();
    static void doReadSegmentInMemoryVIIRS(Segment *t);
    static void doComposeSegmentImageVIIRS(Segment *t);
    static void doComposeProjection(Segment *t);

private:
    void CalculateLUT();
    bool PixelOK(int pix);
    void InterpolateVIIRS(int index1, int index2, int pixelx);

    SatelliteList *satlist;
    int lut[256];
    int earthviews;

protected:

    QFutureWatcher<void> *watcherreadviirs;
    QFutureWatcher<void> *watchercomposeviirs;

protected slots:
    void readfinishedviirs();
    void composefinishedviirs();
    void progressreadvalue(int progress);
    void viirsFinished();
signals:
    void progressCounter(int);


};

#endif // SEGMENTLISTVIIRS_H
