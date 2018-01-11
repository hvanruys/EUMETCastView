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
    
signals:
    
public slots:
    
};

#endif // SEGMENTLISTGAC_H
