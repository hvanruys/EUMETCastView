#include "segmentlistvii.h"
#include "segmentimage.h"
#include "options.h"
#include <QtConcurrent>

extern Options opts;
extern SegmentImage *imageptrs;
extern bool ptrimagebusy;
extern SatelliteList satellitelist;


SegmentListVII::SegmentListVII(eSegmentType type, QObject *parent) :
    SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListVII");

    seglisttype = type;
    histogrammethod = 0; // 0 none , 1 equalize
}

