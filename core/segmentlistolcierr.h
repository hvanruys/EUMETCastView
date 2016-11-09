#ifndef SEGMENTLISTOLCIERR_H
#define SEGMENTLISTOLCIERR_H

#include <QObject>
#include "segmentlist.h"

class SatelliteList;

class SegmentListOLCIerr : public SegmentList
{
    Q_OBJECT

public:
    SegmentListOLCIerr(SatelliteList *satl = 0, QObject *parent = 0);
private:
    SatelliteList *satlist;

};


#endif // SEGMENTLISTOLCIERR_H

