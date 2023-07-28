#ifndef SEGMENTLISTNOAA_H
#define SEGMENTLISTNOAA_H

#include <QObject>
#include <QDir>
#include "segmentlist.h"

class SatelliteList;

class SegmentListNoaa : public SegmentList
{
    Q_OBJECT

public:
    explicit SegmentListNoaa(QObject *parent = 0);
private:

};

#endif // SEGMENTLISTNOAA_H
