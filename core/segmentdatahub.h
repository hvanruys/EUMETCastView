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
    void setSize(QString size) { this->size = size; }
    void setFootprint(QString footprint);
    QString getName() { return name; }
    QString getUUID() { return uuid; }
    QString getSize() { return size; }
    QString getFootprintstring() {return footprint; }
    bool filedownloaded;
    bool quicklookdownloaded;

signals:

public slots:
private:
    void addtovector(QVector<QGeodetic> *vec, QString latlonstr);

    QString name;
    QString uuid;
    QString size;
    QString footprint;


protected:
    SatelliteList *satlist;

};

#endif // DATAHUBSEGMENT_H
