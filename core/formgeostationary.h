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

//typedef struct {
//    int listindex;
//    int spectral_channel_nbr;
//    QString directory;
//    QString productid1;
//    QString productid2;
//    QString timing;
//    double day_of_year;
//    float min;
//    float max;
//    float *data;
//    seviri_units units;
//    double slope;
//    double offset;
//} bandstorage;

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
    int wildcmp(const char *wild, const char *string);
    void setTreeWidget(QTreeWidget *widget, bool state);
    void SetFormImage(FormImage *p_formimage) { formimage = p_formimage; }
    void SelectGeoWidgetItem(int geoindex, QTreeWidgetItem *item, int column );
    SegmentListGeostationary *setActiveSegmentList(int geoindex);
    SegmentListGeostationary *getActiveSegmentList();
    int getTabWidgetGeoIndex();
    //void ComposeGeoRGBRecipe(bandstorage &bs);

    ~FormGeostationary();

private:
    QStringList getGeostationarySegments(int geoindex, const QString imagetype, const QString filepath, QVector<QString> spectrumvector, QString filepattern);
    void PopulateTreeGeo(int geoindex);
    void CreateGeoImageXRIT(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod);
    void CreateGeoImageHDF(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector);
    void CreateGeoImagenetCDF(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor);

    Ui::FormGeostationary *ui;
    AVHRRSatellite *segs;
    SatelliteList *sats;
    FormToolbox *formtoolbox;
    FormImage *formimage;
    QList<QTreeWidget *> geotreewidgetlist;


public slots:
    void PopulateTree();
    void slotCreateGeoImage(QString type, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor);
    void slotCreateRGBrecipe(int recipe);

private slots:
    void ontreeWidgetitemClicked(QTreeWidgetItem *item, int column);
    void on_tabGeostationary_tabBarClicked(int index);

signals:
    void geostationarysegmentschosen(int geoindex, QStringList ll);
    void setbuttonlabels(int geoindex, bool state);
    void enabletoolboxbuttons(bool);

};

#endif // FORMGEOSTATIONARY_H
