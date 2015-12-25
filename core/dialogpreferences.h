#ifndef DIALOGPREFERENCES_H
#define DIALOGPREFERENCES_H

#include <QDialog>
#include <QListWidgetItem>
#include <QColorDialog>

#include "options.h"

namespace Ui {
    class DialogPreferences;
}

class DialogPreferences : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPreferences(QWidget *parent = 0);
    void setupStationsTable();
    void setupTLESourceTable();
    void setupPOILCCTable();
    void setupPOIGVPTable();
    void setupPOISGTable();
    void setupVIIRSMConfigTable();

    ~DialogPreferences();

public slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);

private slots:

    void dialogaccept();
    void dialogreject();
    void on_listWidget_itemChanged(QListWidgetItem *item);
    void addStationRow();
    void deleteStationRow();
    void addTLESourceRow();
    void deleteTLESourceRow();
    void addPOILCCRow();
    void deletePOILCCRow();
    void addPOIGVPRow();
    void deletePOIGVPRow();
    void addPOISGRow();
    void deletePOISGRow();
    void addVIIRSMConfigRow();
    void deleteVIIRSMConfigRow();

    void on_btnLocalDirRemote_clicked();

    void on_btnGshhsOverlay1_clicked();
    void on_btnGshhsOverlay2_clicked();
    void on_btnGshhsOverlay3_clicked();

    void on_btnGshhsGlobe1_clicked();
    void on_btnGshhsGlobe2_clicked();
    void on_btnGshhsGlobe3_clicked();

    void on_btnSkyboxUp_clicked();
    void on_btnSkyboxDown_clicked();
    void on_btnSkyboxLeft_clicked();
    void on_btnSkyboxRight_clicked();
    void on_btnSkyboxFront_clicked();
    void on_btnSkyboxBack_clicked();

    void on_btnImageOverlayColor1_clicked();
    void on_btnImageOverlayColor2_clicked();
    void on_btnImageOverlayColor3_clicked();

    void on_btnGlobeOverlayColor1_clicked();
    void on_btnGlobeOverlayColor2_clicked();
    void on_btnGlobeOverlayColor3_clicked();

    void on_btnBackImage2D_clicked();
    void on_btnBackImage3D_clicked();

    void on_btnMapLCCExtentColor_clicked();
    void on_btnMapGVPExtentColor_clicked();

    void on_btnProjectionOverlayColor1_clicked();
    void on_btnProjectionOverlayColor2_clicked();
    void on_btnProjectionOverlayColor3_clicked();
    void on_btnProjectionOverlayLonLatColor_clicked();

    void on_btnSatHorizonColor_clicked();

    void on_btnSatTrackColor_clicked();

    void on_btnSatSegmentColor_clicked();

    void on_btnSatSegmentColorSel_clicked();

    void on_btnGlobeLonLatColor_clicked();


    void on_btnEquirectangularDirectory_clicked();

private:
    Ui::DialogPreferences *ui;
    QAbstractTableModel *myStationModel;
    QAbstractTableModel *myTLESourceModel;
    QAbstractTableModel *myPOILCCModel;
    QAbstractTableModel *myPOIGVPModel;
    QAbstractTableModel *myPOISGModel;
    QAbstractTableModel *myVIIRSMConfigModel;
    QColorDialog *colordialog;
    bool POItablechanged;

};

class StationModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    StationModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex & index) const ;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

private:

signals:
    void editCompleted();
};

class TLESourceModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TLESourceModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex & index) const ;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

private:

signals:
    void editCompleted();
};

class POILCCModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    POILCCModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex & index) const ;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

private:

signals:
    void editCompleted();
};

class POIGVPModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    POIGVPModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex & index) const ;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

private:

signals:
    void editCompleted();
};

class POISGModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    POISGModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex & index) const ;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

private:

signals:
    void editCompleted();
};

class VIIRSMConfigModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    VIIRSMConfigModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex & index) const ;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());

private:

signals:
    void editCompleted();
};
#endif // DIALOGPREFERENCES_H
