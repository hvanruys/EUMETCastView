#include "segmentlisthrp.h"
#include "segmentimage.h"
#include "options.h"

extern Options opts;
extern SegmentImage *imageptrs;

SegmentListHRP::SegmentListHRP(QObject *parent) :
    SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    seglisttype = eSegmentType::SEG_HRP;

    qDebug() << QString("in constructor SegmentListHRP");

}

