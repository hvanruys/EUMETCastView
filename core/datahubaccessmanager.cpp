#include "datahubaccessmanager.h"
#include "options.h"
#include <QFile>
#include <QDebug>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDir>


extern QNetworkAccessManager networkaccessmanager;
extern Options opts;

//https://scihub.copernicus.eu/s3/odata/v1/Products?$skip=0&$top=100&$filter=substringof(%27S3A_OL_1_EFR____20170131T%27,Name) or substringof(%27S3A_OL_1_ERR____20170131T%27,Name) or substringof(%27S3A_SL_1_RBT____20170131T%27,Name)

DatahubAccessManager::DatahubAccessManager()
{
    m_pBuffer = new QByteArray();
}

void DatahubAccessManager::DownloadXML(int nbrofpages, eDatahub hub)
{
    QString strurl;
    QUrl url;

    qDebug() << "start DownloadXML";
    Q_ASSERT(nbrofpages > 0);

    m_pBuffer->clear();
    this->nbrofpages = nbrofpages;
    this->nbrofpagescounter = 1;
    this->selectdate = "";
    this->hub = hub;

    QDomElement root = docout.createElement(selectdate);
    docout.appendChild(root);

    if(this->hub == HUBESA)
        strurl = QString("https://scihub.copernicus.eu/s3/search?q=*&start=%1&rows=100").arg(0);
    else
        strurl = QString("https://coda.eumetsat.int/search?q=*&start=%1&rows=100").arg(0);

    qDebug() << strurl;

    url = QUrl(strurl);
    // HTTP Basic authentication header value: base64(username:password)
    QString concatenated;
    if(this->hub == HUBESA)
        concatenated = opts.esauser + ":" + opts.esapassword;
    else
        concatenated = opts.eumetsatuser + ":" + opts.eumetsatpassword;

    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    reply = networkaccessmanager.get(request);

    connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadDataXML()));
    connect(reply, SIGNAL(finished()), this,SLOT(slotFinishedXML()));

}

// https://scihub.copernicus.eu/s3/odata/v1/Products?$select=Id&$filter=substringof('S3A_OL_1_EFR____20170309T',Name)
// https://coda.eumetsat.int/odata/v1/Products?$select=Id&$filter=substringof('S3A_OL_1_EFR____20170309T',Name)
//S3A_OL_1_EFR____20170212T100905_20170212T101205_20170212T120355_0179_014_179_2159_SVL_O_NR_002

void DatahubAccessManager::DownloadXML(QDate selectdate, eDatahub hub)
{
    QString strurl;
    QUrl url;

    qDebug() << "start DownloadXML";
    Q_ASSERT(nbrofpages > 0);

    QString strselectdate = selectdate.toString("yyyyMMdd");

    m_pBuffer->clear();
    this->nbrofpagescounter = 1;
    this->selectdate = strselectdate;
    this->hub = hub;

    //QUrl url = QUrl("https://coda.eumetsat.int/search?q=instrumentshortname:SLSTR");
    //QUrl url = QUrl("https://coda.eumetsat.int/odata/v1/Products('52e48b83-c717-484a-a33a-f4ebc941dd84')/$value");
    //QUrl url = QUrl("https://scihub.copernicus.eu/s3/odata/v1/Products");
    //QUrl url = QUrl("https://coda.eumetsat.int/odata/v1/Products?$select=Id&$filter=substringof(%2720170131T185414%27,Name)");
//https://scihub.copernicus.eu/s3/odata/v1/Products?$filter=substringof(%27S3A_OL_1_EFR____20170131T%27,Name)&$skip=400&$top=100

    QDomElement root = docout.createElement("Segments");
    docout.appendChild(root);

    QString resourcepath = QString("Products?$skip=0&$top=100&$filter=substringof('S3A_OL_1_EFR____%1',Name) or substringof('S3A_OL_1_ERR____%1',Name) or substringof('S3A_SL_1_RBT____%1',Name)").arg(strselectdate);
    if(this->hub == HUBESA)
        strurl = "https://scihub.copernicus.eu/s3/odata/v1/" + resourcepath;
    else
        strurl = "https://coda.eumetsat.int/odata/v1/" + resourcepath;

    qDebug() << strurl;

    url = QUrl(strurl);
    // HTTP Basic authentication header value: base64(username:password)
    QString concatenated;
    if(this->hub == HUBESA)
        concatenated = opts.esauser + ":" + opts.esapassword;
    else
        concatenated = opts.eumetsatuser + ":" + opts.eumetsatpassword;

    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    reply = networkaccessmanager.get(request);

    connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadDataXML()));
    connect(reply, SIGNAL(finished()), this,SLOT(slotFinishedXML()));

}

void DatahubAccessManager::slotFinishedXML()
{

    qDebug() << "AccessManager::slotFinished() buffer = " << m_pBuffer->count();

    disconnect(reply,SIGNAL(readyRead()),this,SLOT(slotReadDataXML()));
    disconnect(reply,SIGNAL(finished()), this,SLOT(slotFinishedXML()));

     //if(nbrofpages == nbrofpagescounter)
    if(appendToOutDocument() == false)
    {
        qDebug() << "All pages received !";
        reply->deleteLater();
        endTransmission();
        emit XMLProgress(0);
    }
    else
    {
        emit XMLProgress(nbrofpagescounter);
        nbrofpagescounter++;

        QString strurl;
        QString resourcepath = QString("Products?$skip=%2&$top=100&$filter=substringof('S3A_OL_1_EFR____%1',Name) or substringof('S3A_OL_1_ERR____%1',Name) or substringof('S3A_SL_1_RBT____%1',Name)").arg(this->selectdate).arg((nbrofpagescounter-1) * 100);
        if(this->hub == HUBESA)
            strurl = "https://scihub.copernicus.eu/s3/odata/v1/" + resourcepath;
        else
            strurl = "https://coda.eumetsat.int/odata/v1/" + resourcepath;

        QUrl url = QUrl(strurl);
        qDebug() << strurl;

        request.setUrl(url);

        // HTTP Basic authentication header value: base64(username:password)
        QString concatenated;
        if(this->hub == HUBESA)
            concatenated = opts.esauser + ":" + opts.esapassword;
        else
            concatenated = opts.eumetsatuser + ":" + opts.eumetsatpassword;
        QByteArray data = concatenated.toLocal8Bit().toBase64();
        QString headerData = "Basic " + data;
        request.setRawHeader("Authorization", headerData.toLocal8Bit());
        reply = networkaccessmanager.get(request);
        connect(reply,SIGNAL(readyRead()),this,SLOT(slotReadDataXML()));
        connect(reply,SIGNAL(finished()), this,SLOT(slotFinishedXML()));
    }
}

void DatahubAccessManager::slotReadDataXML()
{
    *m_pBuffer += reply->readAll();
}

void DatahubAccessManager::endTransmission()
{
    //Write to file
     QFile file(QCoreApplication::applicationDirPath() + "/Segments.xml");
     if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
     {
         qDebug() << "Failed to open file for writting";
         return;
     }
     else
     {
         QTextStream stream(&file);
         stream << docout.toString();
         file.close();
         docout.clear();
         qDebug() << "Finished";
     }
     m_pBuffer->clear();
     emit XMLFinished(this->selectdate);
}

bool DatahubAccessManager::appendToOutDocument()
{

    QString errortext;
    int errorline, errorcol;

    QDomDocument document;
    if(document.setContent(*m_pBuffer, false, &errortext, &errorline, &errorcol ) == false)
    {
        qDebug() << "!!!!!!!!!!!!! document wrong ! " << errortext << " linenbr = " << errorline ;
        return false;
    }
    //get the xml root element
    QDomElement xmlroot = document.firstChildElement();


    QDomNodeList entries = xmlroot.elementsByTagName("entry");
    if(entries.count() == 0)
        return false;

    qDebug() << "nbr of entries " << entries.count();
    //qDebug() << QString::fromUtf8((char *)m_pBuffer->data());

    for(int i = 0; i < entries.count(); i++)
    {
        QDomElement entryelement = entries.at(i).toElement();

        QDomNodeList properties = entryelement.elementsByTagName("m:properties");
        //qDebug() << "nbr of properties " << properties.count();
        QDomElement propertyelement = properties.at(0).toElement();
        QDomNodeList namelist = propertyelement.elementsByTagName("d:Name");
        //qDebug() << "nbr of name " << namelist.count();
        QDomElement nameelement = namelist.at(0).toElement();
        QDomNodeList idlist = propertyelement.elementsByTagName("d:Id");
        QDomElement idelement = idlist.at(0).toElement();
        QDomNodeList contentlist = propertyelement.elementsByTagName("d:ContentLength");
        QDomElement contentelement = contentlist.at(0).toElement();
        //qDebug() << i << " " << nameelement.text() << " " << idelement.text() << " " << contentelement.text();

        QDomElement segment = docout.createElement("Segment");
        segment.setAttribute("Name", nameelement.text());
        segment.setAttribute("size", contentelement.text());
        segment.setAttribute("uuid", idelement.text());
        docout.firstChildElement().appendChild(segment);

    }


    m_pBuffer->clear();
    return true;

}

QString DatahubAccessManager::extractFootprint(QString footprint)
{
    footprint.replace("&lt", "<");
    footprint.replace("&gt", ">");
    footprint.replace("&quot", "'");
    QDomDocument document;
    document.setContent(footprint, true);
    //get the xml root element
    QDomElement xmlroot = document.firstChildElement();
    QDomNodeList coords = xmlroot.elementsByTagName("coordinates");
    if(coords.count() > 0)
    {
        QDomElement coord = coords.at(0).toElement();
        return(coord.text());
    }
    else
        return("");
}

void DatahubAccessManager::DownloadProduct(QString uuid, QString filename, eDatahub hub)
{

    isAborted = false;
    this->filename = filename;
    QString strurl;

    if(hub == HUBESA)
        strurl = QString("https://scihub.copernicus.eu/s3/odata/v1/Products('%1')/$value").arg(uuid);
    else
        strurl = QString("https://coda.eumetsat.int/odata/v1/Products('%1')/$value").arg(uuid);

    qDebug() << strurl;

    QUrl url(strurl);

    // HTTP Basic authentication header value: base64(username:password)
    QString concatenated;
    if(hub == HUBESA)
        concatenated = opts.esauser + ":" + opts.esapassword;
    else
        concatenated = opts.eumetsatuser + ":" + opts.eumetsatpassword;
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    reply = networkaccessmanager.get(request);

    connect(reply,SIGNAL(readyRead()),this,SLOT(slotReadDataProduct()));
    connect(reply,SIGNAL(finished()), this,SLOT(slotFinishedProduct()));
    connect(reply,SIGNAL(downloadProgress(qint64,qint64)), this,SLOT(slotdownloadproductProgress(qint64,qint64)));
}

void DatahubAccessManager::CancelDownloadProduct()
{
    isAborted = true;
    reply->abort();
}

void DatahubAccessManager::slotReadDataProduct()
{
    *m_pBuffer += reply->readAll();
}

void DatahubAccessManager::slotFinishedProduct()
{
    qDebug() << "AccessManager::slotFinishedProduct() buffer = " << m_pBuffer->count();

    disconnect(reply,SIGNAL(readyRead()),this,SLOT(slotReadDataXML()));
    disconnect(reply,SIGNAL(finished()), this,SLOT(slotFinishedXML()));
    reply->deleteLater();

    if(!isAborted)
    {
        QFile file(QCoreApplication::applicationDirPath() + "/" + filename + ".zip");

        if(file.open(QIODevice::WriteOnly))
        {
            file.write(*m_pBuffer);
            file.close();
            qDebug() << "File has been saved!";
        }
        else
        {
            qDebug() << "Error saving file!";
        }
    }

    m_pBuffer->clear();
    emit productFinished();

}

void DatahubAccessManager::slotdownloadproductProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit productProgress(bytesReceived, bytesTotal);
}

DatahubAccessManager::~DatahubAccessManager()
{
    qDebug() << "destroy DatahubAccessManager";
    delete m_pBuffer;
}

