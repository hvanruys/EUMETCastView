#ifndef SEGMENTLISTMERSI_H
#define SEGMENTLISTMERSI_H


#include <QObject>
#include "segmentlist.h"
#include "segmentmersi.h"

#include "FreeImage.h"

class SatelliteList;

class SegmentListMERSI : public SegmentList
{
    Q_OBJECT

public:
    explicit SegmentListMERSI(SatelliteList *satl = 0, QObject *parent = 0);
    bool ComposeMERSIImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles);
    bool ComposeMERSIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles);
//    void ShowImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    void SmoothMERSIImage(bool combine);

    void ComposeGVProjection(int inputchannel);
    void ComposeLCCProjection(int inputchannel);
    void ComposeSGProjection(int inputchannel);


protected:
    QFutureWatcher<void> *watchermersi;

protected slots:
    void finishedmersi();
    void progressreadvalue(int progress);

private:
    bool invertarrayindex[3];
    int colorarrayindex[3];

    SatelliteList *satlist;
    bool normalized;
    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    int getIndexFromColor(int colorindex);



};

#endif // SEGMENTLISTMERSI_H
