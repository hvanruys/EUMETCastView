#ifndef FORMMOVIE_H
#define FORMMOVIE_H

#include <QWidget>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include "segmentimage.h"
#include "options.h"
#include "formtoolbox.h"

struct VideoGeoSatellites
{
    QString shortname;
    double longitude;
    bool rss;
};

namespace Ui {
class FormMovie;
}

class FormMovie : public QWidget
{
    Q_OBJECT

public:
    explicit FormMovie(QWidget *parent = nullptr);
    void SetFormToolbox(FormToolbox *ptr) { formtoolbox = ptr; }
    void getProjectionData();
    void setGVPlat(double latitude);
    void setGVPlon(double longitude);
    void setGVPscale(double scale);
    void setGVPheight(int height);
    void setGVPFalseEasting(double easting);
    void setGVPFalseNorthing(double northing);
    void setGVPDisplayGrid(bool grid);
    void setGVPMapHeight(int height);
    void setGVPMapWidth(int width);


    ~FormMovie();

private slots:
    void on_btnCreateXML_clicked();

    void on_btnOverlayColor1_clicked();

    void on_btnOverlayColor2_clicked();

    void on_btnOverlayColor3_clicked();

    void on_btnOverlayGridColor_clicked();

    void on_btnAddPath_clicked();

    void readPendingDatagrams();

private:
    void setupSpectrum();
    void setupSatname();
    bool saveFormToOptions();
    void saveOverlayColorsToOptions();
    void saveSpectrumToOptions();
    void writeTolistwidget(QString txt);

    Ui::FormMovie *ui;
    FormToolbox *formtoolbox;
    QUdpSocket *udpSocket;

};

#endif // FORMMOVIE_H
