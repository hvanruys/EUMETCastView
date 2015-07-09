#ifndef SEGMENTLISTGAC_H
#define SEGMENTLISTGAC_H

#include <QObject>
#include <QDir>
#include "segmentlist.h"

class SegmentListGAC : public SegmentList
{
    Q_OBJECT

public:
    explicit SegmentListGAC(QObject *parent = 0);
    void GetFirstLastVisibleSegmentData(QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast);
    
signals:
    
public slots:
    
};

#endif // SEGMENTLISTGAC_H
