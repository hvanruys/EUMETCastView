#ifndef SEGMENTLISTHRPT_H
#define SEGMENTLISTHRPT_H

#include <QObject>
#include "segmentlist.h"

class SatelliteList;

class SegmentListHRPT : public SegmentList
{
    Q_OBJECT

public:
    SegmentListHRPT(eSegmentType type = SEG_HRPT_M01, SatelliteList *satl = 0, QObject *parent = 0);

private:
    SatelliteList *satlist;

};

#endif // SEGMENTLISTHRPT_H
