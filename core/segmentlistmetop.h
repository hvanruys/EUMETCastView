#ifndef SEGMENTLISTMETOP_H
#define SEGMENTLISTMETOP_H


#include <QObject>

#include "segmentlist.h"

class SegmentListMetop : public SegmentList
{
    Q_OBJECT

public:
    explicit SegmentListMetop(QObject *parent = 0);
    bool GetGeoLocation(double lon_rad, double lat_rad, int *x, int *y);
    void GetCentralCoords(double *startcentrallon, double *startcentrallat, double *endcentrallon, double *endcentrallat);

};

#endif // SEGMENTLISTMETOP_H
