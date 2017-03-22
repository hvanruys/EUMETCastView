#ifndef DATAHUBSEGMENTLIST_H
#define DATAHUBSEGMENTLIST_H

#include <QObject>
#include "segmentlist.h"
#include "segmentdatahub.h"

class SegmentDatahub;

class SegmentListDatahub : public SegmentList
{
    Q_OBJECT
public:
    explicit SegmentListDatahub(QObject *parent = 0);
//    QList<SegmentDatahub *> *GetSegmentlistptr(void) { return &segmentlist; }
//    QList<SegmentDatahub *> *GetSegsSelectedptr(void) { return &segsselected; }


signals:

public slots:

protected:
//    QList<SegmentDatahub *> segmentlist;
//    QList<SegmentDatahub *> segsselected;
};

#endif // DATAHUBSEGMENTLIST_H
