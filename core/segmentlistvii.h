#ifndef SEGMENTLISTVII_H
#define SEGMENTLISTVII_H

#include <QObject>
#include "segmentlist.h"
#include "segmentvii.h"

#include "FreeImage.h"

class SatelliteList;

class SegmentListVII : public SegmentList
{
    Q_OBJECT

public:
    SegmentListVII(eSegmentType type = SEG_METOPSGA1, QObject *parent = 0);


private:


protected:

protected slots:

};

#endif // SEGMENTLISTVII_H
