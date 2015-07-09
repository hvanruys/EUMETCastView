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
    explicit SegmentListNoaa(SatelliteList *satl = 0, QObject *parent = 0);
    void GetFirstLastVisibleSegmentData(QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast);
private:

    SatelliteList *satlist;


};

#endif // SEGMENTLISTNOAA_H
