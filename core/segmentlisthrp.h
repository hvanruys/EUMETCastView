#ifndef AVHRRSEGMENTLISTHRP_H
#define AVHRRSEGMENTLISTHRP_H

#include <QObject>
#include <QDir>

#include "segmentlist.h"


class SegmentListHRP : public SegmentList
{
    Q_OBJECT

public:
    explicit SegmentListHRP(QObject *parent = 0);
    void GetFirstLastVisibleSegmentData(QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast);
private:



};

#endif // AVHRRSEGMENTLIST_H
