#include "sgp4sdp4.h"
#include "segmentlistnoaa.h"
#include "segmentnoaa.h"
#include "segmentimage.h"
#include "options.h"

#include <QDir>
#include <QDebug>
#include <QPainter>
#include <math.h>

extern Options opts;
extern SegmentImage *imageptrs;

SegmentListNoaa::SegmentListNoaa(QObject *parent) :
    SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    seglisttype = eSegmentType::SEG_NOAA19;

    qDebug() << QString("in constructor SegmentListNoaa");
}

