#ifndef FORMGEOSTATIONARY_H
#define FORMGEOSTATIONARY_H

#include <QWidget>
#include <QTreeWidget>
#include "satellite.h"
#include "avhrrsatellite.h"
#include "msgfileaccess.h"
#include "msgdataaccess.h"
#include "formtoolbox.h"
#include "formimage.h"

class FormImage;

namespace Ui {
class FormGeostationary;
}

class FormGeostationary : public QWidget
{
    Q_OBJECT
    
public:

    explicit FormGeostationary(QWidget *parent = 0, SatelliteList *satlist = 0, AVHRRSatellite *seglist = 0);
    QStringList globit(const QString filepath, const QString filepattern);
    void SetFormToolBox(FormToolbox *p_formtoolbox) { formtoolbox = p_formtoolbox; }
    // QStringList globitwin32(const QString filepath, const QString filepattern);
    int wildcmp(const char *wild, const char *string);
    //int GetChannelIndex();
    void setTreeWidget(QTreeWidget *widget, bool state);
    void SetFormImage(FormImage *p_formimage) { formimage = p_formimage; }
    void SelectGeoWidgetItem(SegmentListGeostationary::eGeoTreeWidget geosat, QTreeWidgetItem *item, int column );

    ~FormGeostationary();

private:
    QStringList getGeostationarySegments(SegmentListGeostationary::eGeoSatellite whichgeo, const QString imagetype, const QString filepath, QVector<QString> spectrumvector, QString filepattern);
    QStringList getGeostationarySegmentsFengYun(SegmentListGeostationary::eGeoSatellite whichgeo, const QString imagetype, const QString filepath, QVector<QString> spectrumvector, QString filepattern);
    void PopulateTreeGeo(SegmentListGeostationary::eGeoSatellite whichgeo, QMap<QString, QMap<QString, QMap<int, QFileInfo> > > map, QTreeWidget *widget);
    void CreateGeoImageXRIT(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector);
    void CreateGeoImageHDF(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector);
    void CreateGeoImagenetCDF(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector);

    Ui::FormGeostationary *ui;
    AVHRRSatellite *segs;
    SatelliteList *sats;
    FormToolbox *formtoolbox;
    FormImage *formimage;


public slots:
    void PopulateTree();
    void CreateGeoImage(QString type, QVector<QString> spectrumvector, QVector<bool> inversevector);


private slots:
    void on_SegmenttreeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetRSS_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetMet8_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetGOES13dc3_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetGOES15dc3_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetGOES13dc4_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetGOES15dc4_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetGOES16_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetFY2E_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetFY2G_itemClicked(QTreeWidgetItem *item, int column);

    void on_SegmenttreeWidgetH8_itemClicked(QTreeWidgetItem *item, int column);

signals:
    void geostationarysegmentschosen(SegmentListGeostationary::eGeoSatellite geo, QStringList ll);
    void enabletoolboxbuttons(bool);
};

#endif // FORMGEOSTATIONARY_H
