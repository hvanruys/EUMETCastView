#ifndef FORMMOVIE_H
#define FORMMOVIE_H

#include <QWidget>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include "processmanager.h"
#include "segmentimage.h"
#include "options.h"
#include "formtoolbox.h"
#include "avhrrsatellite.h"

struct VideoGeoSatellites
{
    QString shortname;
    double longitude;
    bool rss;
};

struct VideoMinMaxLat
{
    double maxlat;
    double minlat;
};

namespace Ui {
class FormMovie;
}

class FormMovie : public QWidget
{
    Q_OBJECT

public:
    explicit FormMovie(QWidget *parent = 0, AVHRRSatellite *seglist = 0);
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
    // void on_btnCreateXML_clicked();

    void on_btnOverlayColor1_clicked();

    void on_btnOverlayColor2_clicked();

    void on_btnOverlayColor3_clicked();

    void on_btnOverlayGridColor_clicked();

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

    void PopulateSelectionList(QDate seldate);

    void on_btnJson_clicked();

    void on_rdbMeteosat_12_clicked();

    void on_rdbMeteosat_11_clicked();

    void on_rdbMeteosat_10_clicked();

    void on_rdbMeteosat_9_clicked();

    void deleteManager();

private:
    void setupSpectrumMeteosat();
    void setupSpectrumMTG();
    bool saveFormToOptions();
    void saveOverlayColorsToOptions();
    void saveSpectrumToOptions();
    void writeTolistwidget(QString txt);
    void listWidgets();
    void CreateVideoJson(QString shortname);
    bool convertToJson(const QMap<int, QMap<int, QFileInfo>>& segmentlistmap, const QString& outputFilePath);
    QJsonObject getJasonObjectFromMap(const QMap<int, QMap<int, QFileInfo>>& segmentlistmap);
    QJsonObject getJasonObjectFromMap(const QMap<QString, QMap<QString, QMap< int, QFileInfo > > >& segmentlistmap);
    QMap<int, QMap<int, QFileInfo>> filterByKeys( const QMap<int, QMap<int, QFileInfo>>& input, const QSet<int>& allowedKeys);
    QSet<int> getFilteredSet();
    QDate selectiondate;
    QJsonObject getJsonFileList();
    Ui::FormMovie *ui;
    FormToolbox *formtoolbox;
    QUdpSocket *udpSocket;
    QListWidgetItem *item;
    AVHRRSatellite *segs;
    ProcessManager *processmanager;
    QString shortname;
    int geoindex;
    QList<VideoMinMaxLat> minmaxlist;


};

#endif // FORMMOVIE_H
