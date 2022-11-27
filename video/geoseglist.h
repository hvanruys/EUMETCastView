#ifndef GEOSEGLIST_H
#define GEOSEGLIST_H

#include <QObject>
#include <QFileInfo>
#include <QMap>
#include <QList>

class GeoSegList : public QObject
{
    Q_OBJECT
public:
    explicit GeoSegList(QObject *parent = nullptr);

    QList<QMap<QString, QMap<QString, QMap< int, QFileInfo > > > > segmentlistmapgeo;


signals:

};

#endif // GEOSEGLIST_H
