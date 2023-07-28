#ifndef SEGMENTLISTHRPT_H
#define SEGMENTLISTHRPT_H

#include <QObject>
#include "segmentlist.h"

class SatelliteList;

class SegmentListHRPT : public SegmentList
{
    Q_OBJECT

public:
    SegmentListHRPT(eSegmentType type = SEG_HRPT_M01, QObject *parent = 0);

private:

};

#endif // SEGMENTLISTHRPT_H
