#ifndef FORMEPHEM_H
#define FORMEPHEM_H

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QUdpSocket>

#include "options.h"
#include "satellite.h"
#include "avhrrsatellite.h"
#include "downloadmanager.h"

namespace Ui {
    class FormEphem;
}

class FormEphem : public QWidget
{
    Q_OBJECT

public:
    explicit FormEphem(QWidget *parent = 0, SatelliteList *satlist = 0, AVHRRSatellite *seglist = 0);
    void showAvailSat();
    void showActiveSatellites(void);

    ~FormEphem();


private:
    Ui::FormEphem *ui;
    //QTimer *timer;
    AVHRRSatellite *segs;
    void updateSatelliteEphem(void);
    void showSegmentDirectoryList(void);
    void AddRootDirectoryWidgetItem(QString segname, Qt::CheckState checkstate);
    void NewSegmentOverviewItem();

    int metopcount;
    int noaacount;
    int gaccount;
    int hrpcount;

    DownloadManager downloadmanager;
    QUdpSocket *udpSocket;



private slots:
    void timerDone();
    void on_btnAdd_clicked();
    void on_btnDel_clicked();
    void on_btnUpdateTLE_clicked();
    void on_tletreewidget_itemChanged(QTreeWidgetItem *item, int column);
    void itemSelectedtreewidget( QTreeWidgetItem* );
    void itemSelectedsegmentdirectory( QTreeWidgetItem *item);
    void tlefilesread(QString str);
    void showprogress(QString str);
    //void writeDebugInfo(QTreeWidgetItem *it, int nbr );
    void setSegmentsShownValue();
    void setRealMinutesShownValue();
    void setNbrOfHours();
    void getSegmentsForCalendar();
    void setProgressBar(int progress);
    void resetProgressBar(int maxprogress,const QString &mytext);
    void on_btnAddsegmentdir_clicked();
    void on_btnDelsegmentdir_clicked();
    void showSelectedSegmentList(void);
    void processPendingDatagrams();

    void on_btnReload_clicked();

    void on_calendar_selectionChanged();

    void on_rdbDownloadFromDatahub_clicked(bool checked);

public slots:
    void showSegmentsAdded();

signals:
    void signalSegmenttype( const QString &text );
    void signalDirectoriesRead();
    void signalDatagram(QByteArray);

protected:
    SatelliteList *sats;



};

#endif // FORMEPHEM_H
