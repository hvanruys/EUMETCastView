#ifndef SEGMENTOLCIERR_H
#define SEGMENTOLCIERR_H

#include <QObject>
#include "segment.h"


class SegmentOLCIerr : public Segment
{
    Q_OBJECT

public:
    explicit SegmentOLCIerr(QFile *filesegment = 0, SatelliteList *satl = 0, QObject *parent = 0);
    void CalculateDetailCornerPoints();
    void setupVector(double statevec);
    ~SegmentOLCIerr();




};

#endif // SEGMENTOLCIERR_H
