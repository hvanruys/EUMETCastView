#include "segmentlisthrpt.h"
#include "options.h"

extern Options opts;

SegmentListHRPT::SegmentListHRPT(eSegmentType type, SatelliteList *satl, QObject *parent) :
  SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListHRPT");

    satlist = satl;
    seglisttype = type;
}

