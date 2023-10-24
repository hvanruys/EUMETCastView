#ifndef FORMGEOSTATIONARY_H
#define FORMGEOSTATIONARY_H

#include <QWidget>
#include <QTreeWidget>
#include "formmapcyl.h"
#include "satellite.h"
#include "avhrrsatellite.h"
//#include "msgfileaccess.h"
//#include "msgdataaccess.h"
#include "formimage.h"
//#include "formtoolbox.h"

class FormImage;
class FormToolbox;

namespace Ui {
class FormGeostationary;
}

class FormGeostationary : public QWidget
{
    Q_OBJECT
    
public:

    explicit FormGeostationary(QWidget *parent = 0, AVHRRSatellite *seglist = 0);
    QStringList globit(const QString filepath, const QString filepattern);
    void SetFormToolBox(FormToolbox *p_formtoolbox) { formtoolbox = p_formtoolbox; }
    int wildcmp(const char *wild, const char *string);
    void setTreeWidget(QTreeWidget *widget, bool state);
    void SetFormImage(FormImage *p_formimage) { formimage = p_formimage; }
    void SelectGeoWidgetItem(int geoindex, QTreeWidgetItem *item, int column );
    SegmentListGeostationary *setActiveSegmentList(int geoindex);
    SegmentListGeostationary *getActiveSegmentList();
    int getTabWidgetGeoIndex();
    void getTimeFromFilenbr(int filenbr, QString *strtime);
    ~FormGeostationary();

private:
    QStringList getGeostationarySegments(int geoindex, const QString imagetype, const QString filepath, QVector<QString> spectrumvector, QString filepattern);
    QStringList getGeostationarySegmentsMTG(int geoindex, const QString imagetype, const QString filepath, int filenbr);
    void PopulateTreeGeo(int geoindex);
    void PopulateTreeGeoMTGI1(int geoindex);

    void CreateGeoImageXRIT(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod);
    void CreateGeoImageHDF(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector);
    void CreateGeoImagenetCDF(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor);
    void CreateGeoImagenetCDFMTG(SegmentListGeostationary *sl, QString type, QString tex, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor);
    void CreateGeoImages(QString type, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor, QString tex, int geoindex);
    void CreateGeoImageMTG(QString type, QVector<QString> spectrumvector, QVector<bool> inversevector,int histogrammethod, bool pseudocolor, QString tex, int geoindex);
    Ui::FormGeostationary *ui;
    AVHRRSatellite *segs;
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
