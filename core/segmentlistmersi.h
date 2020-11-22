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
    void ComposeOMProjection(int inputchannel);
    void GetCentralCoords(double *startlon, double *startlat, double *endlon, double *endlat);
    void GetCornerCoords(double *cornerlon1, double *cornerlat1, double *cornerlon2, double *cornerlat2, double *cornerlon3, double *cornerlat3, double *cornerlon4, double *cornerlat4);

protected:
    QFutureWatcher<void> *watchermersi;

protected slots:
    void finishedmersi();
    void progressreadvalue(int progress);

private:
    int getIndexFromColor(int colorindex);

    bool invertarrayindex[3];
    int colorarrayindex[3];
    int bandindex;

    SatelliteList *satlist;
    bool normalized;
    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;


};

#endif // SEGMENTLISTMERSI_H
