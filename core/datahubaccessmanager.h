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
    void DownloadXML(QDate selectdate, eDatahub hub);
    void DownloadProduct(QList<ProductList> prodlist, int index, eDatahub hub, int whichdownload, bool quicklook);
    void CancelDownload();
    int getWhichDownload() { return whichdownload; }
    int getDownloadIndex() { return downloadindex; }
    bool isProductDownloadBusy() { return isProductBusy; }
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
    QString getresourcepath(QString strselectdate, int page);

    bool isAborted;
    bool isProductBusy;
    QByteArray* m_pBuffer;
    int nbrofpages;
    int nbrofpagescounter;
    QString selectdate;
    QString filename;
    QDomDocument docout;
    QNetworkReply *reply;
    QNetworkRequest request;
    eDatahub hub;
    int whichdownload;
    int downloadindex;
    bool quicklook;


signals:
    void productProgress(qint64 bytesReceived, qint64 bytesTotal, int whichdownload);
    void productFinished(int whichdownload, int downloadindex, bool quicklook);
    void XMLFinished(QString selectdate);
    void XMLProgress(int pages);
};

#endif
