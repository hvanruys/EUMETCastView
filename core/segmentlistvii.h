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
    bool ComposeVIIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    static void doComposeVIIImageInThread(SegmentListVII *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);

private:
    QFutureWatcher<void> *watchervii;

    int histogrammethod;
    bool normalized;

protected:
    int num_scans;
    int num_chan;
    int num_chan_solar;
    int num_chan_thermal;
    int num_pixels;
    int num_pixels_alt;
    int num_lines;
    int zone_size_act;
    int zone_size_alt;
    int num_tie_points_act;
    int num_tie_points_alt;

protected slots:
    void finishedvii();

};

#endif // SEGMENTLISTVII_H
