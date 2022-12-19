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

    void on_btnClear_clicked();

    void on_btnffmpeg_clicked();

    void on_lwffmpeg_itemSelectionChanged();

    void on_leffmpegoptions_textEdited(const QString &arg1);

    void on_btnAdd_clicked();

    void on_btnDelete_clicked();

    void on_btnUp_clicked();

    void on_btnDown_clicked();

    void on_btnDefault_clicked();


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
    QListWidgetItem *item;

};

#endif // FORMMOVIE_H
