#ifndef DATAHUBACCESSMANAGER_H
#define DATAHUBACCESSMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QtXml>
#include "productlist.h"


enum eDatahub {
    HUBESA = 0,
    HUBEUMETSAT
};

class DatahubAccessManager : public QObject
{
    Q_OBJECT
public:
    DatahubAccessManager();
    void DownloadXML(int nbrofpages, eDatahub hub);
    void DownloadXML(QDate selectdate, eDatahub hub, QString type);
    void DownloadProduct(QList<ProductList> prodlist, int index, eDatahub hub, int whichdownload);
    void CancelDownload();
    int getWhichDownload() { return whichdownload; }
    int getDownloadIndex() { return downloadindex; }
    bool isProductDownloadBusy() { return isProductBusy; }
    QString getProductname() { return completebasename; }
    QString getBandOrQuicklook() { return band_or_quicklook; }
    ~DatahubAccessManager();

private slots:
    void slotFinishedXML();
    void slotReadDataXML();
    void slotFinishedProduct();
    void slotReadDataProduct();
    void slotdownloadproductProgress(qint64 bytesReceived, qint64 bytesTotal);


private:
    void endTransmission();
    bool appendToOutDocument();
    void FinishedXML();
    QString extractFootprint(QString footprint);
    QString getresourcepath(QString strselectdate, int page, QString type);
    //void TestForDirectory(QString dirpath);
    QDir TestForDirectory();

    QString typetodownload;

    bool isAborted;
    bool isProductBusy;
    QByteArray* m_pBuffer;
    int nbrofpages;
    int nbrofpagescounter;
    QString selectdate;
    QString completebasename;
    QString uuid;
    QString band_or_quicklook;
    QDomDocument docout;
    QNetworkReply *reply;
    QNetworkRequest request;
    eDatahub hub;
    int whichdownload;
    int downloadindex;


signals:
    void productProgress(qint64 bytesReceived, qint64 bytesTotal, int whichdownload);
    void productFinished(int whichdownload, int downloadindex, QString absoluteproductpath, QString absolutepath, QString filename);
    void XMLFinished(QString selectdate);
    void XMLProgress(int pages);
};

#endif
