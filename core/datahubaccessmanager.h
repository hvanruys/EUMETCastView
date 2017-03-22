#ifndef DATAHUBACCESSMANAGER_H
#define DATAHUBACCESSMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QtXml>


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
    void DownloadProduct(QString uuid, QString filename, eDatahub hub);
    void CancelDownloadProduct();
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

    bool isAborted;
    QByteArray* m_pBuffer;
    int nbrofpages;
    int nbrofpagescounter;
    QString selectdate;
    QString filename;
    QDomDocument docout;
    QNetworkReply *reply;
    QNetworkRequest request;
    eDatahub hub;


signals:
    void productProgress(qint64 bytesReceived, qint64 bytesTotal);
    void productFinished();
    void XMLFinished(QString selectdate);
    void XMLProgress(int pages);

};

#endif
