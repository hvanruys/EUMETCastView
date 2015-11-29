#include "segmentlistgac.h"
#include "segmentimage.h"
#include "options.h"

extern Options opts;
extern SegmentImage *imageptrs;

SegmentListGAC::SegmentListGAC(QObject *parent) :
    SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    seglisttype = eSegmentType::SEG_GAC;

    qDebug() << QString("in constructor SegmentListGAC");

}


