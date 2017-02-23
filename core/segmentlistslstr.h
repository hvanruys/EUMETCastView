#ifndef SEGMENTLISTSLSTR_H
#define SEGMENTLISTSLSTR_H

#include <QObject>
#include "segment.h"
#include "segmentlist.h"
#include "satellite.h"

class SegmentListSLSTR : public SegmentList
{
    Q_OBJECT

public:
    SegmentListSLSTR(SatelliteList *satl = 0, QObject *parent = 0);
    bool ComposeSLSTRImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles, eSLSTRImageView view);
    bool ComposeSLSTRImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles);
    void setHistogramMethod(int histo) { histogrammethod = histo;}
    bool ChangeHistogramMethod();


private:

    void CalculateLUTFull();
    void ComposeSegments();


    SatelliteList *satlist;

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> inverselist;

    int histogrammethod;

protected:
    QFutureWatcher<void> *watcherslstr;

protected slots:
    void finishedslstr();
    void progressreadvalue(int progress);



};

#endif // SEGMENTLISTSLSTR_H
