#ifndef SEGMENTLISTOLCI_H
#define SEGMENTLISTOLCI_H

#include <QObject>
#include "segmentlist.h"

class SatelliteList;

class SegmentListOLCI : public SegmentList
{
    Q_OBJECT

public:
    SegmentListOLCI(eSegmentType type = SEG_OLCIEFR, SatelliteList *satl = 0, QObject *parent = 0);
    bool ComposeOLCIImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool untarfiles);
    bool ComposeOLCIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool untarfiles);
    void ComposeGVProjection(int inputchannel);
    void SmoothOLCIImage(bool combine);
    void ShowWinvec(QPainter *painter, float distance, const QMatrix4x4 modelview);
    bool TestForSegmentGLerr(int x, int realy, float distance, const QMatrix4x4 &m, bool showallsegments, QString &segmentname);

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

#endif // SEGMENTLISTOLCI_H
