#include "segmentlistolcierr.h"
#include "options.h"

extern Options opts;

SegmentListOLCIerr::SegmentListOLCIerr(SatelliteList *satl, QObject *parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListOLCIerr");

    satlist = satl;
    seglisttype = eSegmentType::SEG_OLCIERR;

    //earthviews = 3200;

}
