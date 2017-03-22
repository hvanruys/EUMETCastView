#ifndef SEGMENTDATAHUB_H
#define SEGMENTDATAHUB_H

#include <QObject>
#include "segment.h"
#include "satellite.h"
#include "globals.h"
#include "options.h"

class SegmentDatahub : public Segment
{
    Q_OBJECT
public:
    explicit SegmentDatahub(eSegmentType type = SEG_NONE, QString name = 0, SatelliteList *satl = 0, QObject *parent = 0);
    void setName(QString name) { this->name = name; }
    void setUUID(QString uuid) { this->uuid = uuid; }
    QString getName() { return name; }
    QString getUUID() { return uuid; }

signals:

public slots:
private:
    QString name;
    QString uuid;

protected:
    SatelliteList *satlist;



};

#endif // DATAHUBSEGMENT_H
