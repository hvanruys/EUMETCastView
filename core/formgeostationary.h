#ifndef FORMGEOSTATIONARY_H
#define FORMGEOSTATIONARY_H

#include <QWidget>
#include <QTreeWidget>
#include "satellite.h"
#include "avhrrsatellite.h"

namespace Ui {
class FormGeostationary;
}

class FormGeostationary : public QWidget
{
    Q_OBJECT
    
public:

    explicit FormGeostationary(QWidget *parent = 0,SatelliteList *satlist = 0, AVHRRSatellite *seglist = 0);
    QStringList globit(const QString filepath, const QString filepattern);

    // QStringList globitwin32(const QString filepath, const QString filepattern);
    int wildcmp(const char *wild, const char *string);
    //int GetChannelIndex();
    void setTreeWidget(QTreeWidget *widget, bool state);

    ~FormGeostationary();
    
private:
    QStringList getGeostationarySegments(const QString imagetype, const QString filepath, QVector<QString> spectrumvector, QString filepattern);
    void PopulateTreeGeo(SegmentListGeostationary::eGeoSatellite whichgeo, QMap<QString, QMap<QString, QMap<int, QFileInfo> > > map, QTreeWidget *widget);

    Ui::FormGeostationary *ui;
    AVHRRSatellite *segs;
    SatelliteList *sats;

public slots:
    void PopulateTree();
    void CreateGeoImage(SegmentListGeostationary::eGeoSatellite whichgeo, QString type, QVector<QString> spectrumvector, QVector<bool> inversevector);


private slots:
    void on_SegmenttreeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetRSS_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetMet7_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetGOES13dc3_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetGOES15dc3_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetMTSATdc3_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetGOES13dc4_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetGOES15dc4_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetMTSATdc4_itemClicked(QTreeWidgetItem *item, int column);

signals:
    void geostationarysegmentschosen(SegmentListGeostationary::eGeoSatellite geo, QStringList ll);
    void enabletoolboxbuttons(bool);
};

#endif // FORMGEOSTATIONARY_H
