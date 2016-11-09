#ifndef SEGMENTLISTOLCIEFR_H
#define SEGMENTLISTOLCIEFR_H

#include <QObject>
#include "segmentlist.h"

class SatelliteList;

class SegmentListOLCIefr : public SegmentList
{
    Q_OBJECT

public:
    SegmentListOLCIefr(SatelliteList *satl = 0, QObject *parent = 0);
    bool ComposeOLCIImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool untarfiles);
    bool ComposeOLCIefrImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool untarfiles);
    void ComposeGVProjection(int inputchannel);
    void SmoothOLCIImage(bool combine);

private:

    void CalculateLUT();

    SatelliteList *satlist;

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> inverselist;


protected:

    QFutureWatcher<void> *watcherolci;

protected slots:
    void finishedolci();
    void progressreadvalue(int progress);

};

#endif // SEGMENTLISTOLCIEFR_H
