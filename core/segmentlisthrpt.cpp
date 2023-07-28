#include "segmentlisthrpt.h"
#include "options.h"

extern Options opts;

SegmentListHRPT::SegmentListHRPT(eSegmentType type, QObject *parent) :
  SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListHRPT");

    seglisttype = type;
}

