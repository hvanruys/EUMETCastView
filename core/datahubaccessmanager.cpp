#include "datahubaccessmanager.h"
#include "options.h"
#include <QFile>
#include <QDebug>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDir>
#include <QMessageBox>
#include <QTextDocument>

extern QNetworkAccessManager networkaccessmanager;
extern Options opts;

//https://scihub.copernicus.eu/s3/odata/v1/Products?$skip=0&$top=100&$filter=substringof(%27S3A_OL_1_EFR____20170131T%27,Name) or substringof(%27S3A_OL_1_ERR____20170131T%27,Name) or substringof(%27S3A_SL_1_RBT____20170131T%27,Name)

DatahubAccessManager::DatahubAccessManager()
{
    m_pBuffer = new QByteArray();
    isAborted = false;
    isProductBusy = false;
    reply = NULL;
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
// S3A_OL_1_EFR____20170212T100905_20170212T101205_20170212T120355_0179_014_179_2159_SVL_O_NR_002

void DatahubAccessManager::DownloadXML(QDate selectdate, eDatahub hub)
{
    QString strurl;
    QUrl url;

    qDebug() << "start DownloadXML";

    if(opts.xmllogging)
    {
        QFile xmlloggingfile(QCoreApplication::applicationDirPath() + "/xmllogging.txt");
        if (xmlloggingfile.open(QFile::WriteOnly | QFile::Truncate)) {
            QTextStream out(&xmlloggingfile);
            out << "Start logging : " << QDate::currentDate().toString();
            xmlloggingfile.close();
        }
    }

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
    //https://coda.eumetsat.int/odata/v1/Products('6b0ab72a-858f-41a0-862d-549dfdd15b59')/Products('Quicklook')/$value

    QDomElement root = docout.createElement("Segments");
    docout.appendChild(root);

    QString resourcepath = getresourcepath(this->selectdate, nbrofpagescounter - 1);
    if (resourcepath.isEmpty())
        return;

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


    qDebug() << "Errorstring = " << reply->errorString() << " errorcode = " << reply->error();

    if(reply->error() != QNetworkReply::NoError)
    {
        QMessageBox msgBox;
        msgBox.setText(QString("The network error code = %1").arg(reply->error()));
        msgBox.exec();
    }
    else
    {
        connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadDataXML()));
        connect(reply, SIGNAL(finished()), this, SLOT(slotFinishedXML()));
    }
}

void DatahubAccessManager::slotFinishedXML()
{

    qDebug() << "AccessManager::slotFinished() buffer = " << m_pBuffer->count();

    disconnect(reply,SIGNAL(readyRead()),this,SLOT(slotReadDataXML()));
    disconnect(reply,SIGNAL(finished()), this,SLOT(slotFinishedXML()));


    if(appendToOutDocument() == false)
    {
        isAborted = false;
        qDebug() << "All pages received !";
        reply->deleteLater();
        reply = NULL;
        endTransmission();
        emit XMLProgress(0);
    }
    else
    {
        emit XMLProgress(nbrofpagescounter);
        nbrofpagescounter++;

        QString strurl;
        if(this->hub == HUBESA)
            strurl = "https://scihub.copernicus.eu/s3/odata/v1/" + getresourcepath(this->selectdate, nbrofpagescounter - 1);
        else
            strurl = "https://coda.eumetsat.int/odata/v1/" + getresourcepath(this->selectdate, nbrofpagescounter - 1);

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

    if(opts.xmllogging)
    {
        QFile xmlloggingfile(QCoreApplication::applicationDirPath() + "/xmllogging.txt");
        if (xmlloggingfile.open(QFile::WriteOnly | QFile::Append)) {
            QTextStream out(&xmlloggingfile);
            out << *m_pBuffer;
            xmlloggingfile.close();
        }
    }

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
        //qDebug() << "m_pBuffer = " << *m_pBuffer;

        QString title = QString("Error Message");
        QTextDocument textdoc(*m_pBuffer);
        QString strdoc = textdoc.toPlainText();
        if( strdoc.mid(0, 15) == "<!DOCTYPE html>")
            strdoc.remove(0, 15);
        QMessageBox msgBox;
        msgBox.setText(strdoc);
        msgBox.exec();
        return false;
    }

    //get the xml root element
    QDomElement xmlroot = document.firstChildElement();


    QDomNodeList entries = xmlroot.elementsByTagName("entry");
    if(entries.count() == 0)
    {
        m_pBuffer->clear();
        return false;
    }

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

    if(entries.count() < 100)
        return false;

    return true;

}

QString DatahubAccessManager::getresourcepath(QString strselectdate, int page)
{
    //QString resourcepath = QString("Products?$skip=0&$top=100&$filter=substringof('S3A_OL_1_EFR____%1',Name) or substringof('S3A_OL_1_ERR____%1',Name) or substringof('S3A_SL_1_RBT____%1',Name)").arg(strselectdate);
    QString resourcepath = QString("Products?$skip=%1&$top=100&$filter=").arg(page * 100);
    QString resourcepatholciefr = QString("substringof('S3A_OL_1_EFR____%1',Name)").arg(strselectdate);
    QString resourcepatholcierr = QString("substringof('S3A_OL_1_ERR____%1',Name)").arg(strselectdate);
    QString resourcepathslstr = QString("substringof('S3A_SL_1_RBT____%1',Name)").arg(strselectdate);

    int countopts = 0;
    if (opts.downloadxmlolciefr) countopts++;
    if (opts.downloadxmlolcierr) countopts++;
    if (opts.downloadxmlslstr) countopts++;

    if(countopts == 0)
        return "";
    if(countopts == 1)
    {
        if(opts.downloadxmlolciefr)
            resourcepath += resourcepatholciefr;
        else if(opts.downloadxmlolcierr)
            resourcepath += resourcepatholcierr;
        else if(opts.downloadxmlslstr)
            resourcepath += resourcepathslstr;
    }
    else if(countopts == 2)
    {
        if(opts.downloadxmlolciefr && opts.downloadxmlolcierr)
            resourcepath += resourcepatholciefr + " or " + resourcepatholcierr;
        else if(opts.downloadxmlolciefr && opts.downloadxmlslstr)
            resourcepath += resourcepatholciefr + " or " + resourcepathslstr;
        else if(opts.downloadxmlolcierr && opts.downloadxmlslstr)
            resourcepath += resourcepatholcierr + " or " + resourcepathslstr;
    }
    else if(countopts == 3)
        resourcepath += resourcepatholciefr + " or " + resourcepatholcierr + " or " + resourcepathslstr;
    return resourcepath;

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

void DatahubAccessManager::DownloadProduct(QList<ProductList> prodlist, int index, eDatahub hub, int whichdownload, bool quicklook)
{

    isAborted = false;
    isProductBusy = true;
    this->whichdownload = whichdownload;
    this->downloadindex = index;
    this->quicklook = quicklook;

    this->filename = prodlist.at(index).productname;
    QString strurl;

    if(hub == HUBESA)
    {
        if(quicklook)
            strurl = QString("https://scihub.copernicus.eu/s3/odata/v1/Products('%1')/Products('Quicklook')/$value").arg(prodlist.at(index).uuid);
        else
            strurl = QString("https://scihub.copernicus.eu/s3/odata/v1/Products('%1')/$value").arg(prodlist.at(index).uuid);
    }
    else
    {
        if(quicklook)
            strurl = QString("https://coda.eumetsat.int/odata/v1/Products('%1')/Products('Quicklook')/$value").arg(prodlist.at(index).uuid);
        else
            strurl = QString("https://coda.eumetsat.int/odata/v1/Products('%1')/$value").arg(prodlist.at(index).uuid);
    }
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


void DatahubAccessManager::CancelDownload()
{
    isAborted = true;
    isProductBusy = false;

    if(reply == NULL)
        return;

    if(reply->isFinished())
    {
        return;
    }

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
    reply = NULL;

    if(!isAborted)
    {

        QString dirpath;
        if(opts.productdirectory.isEmpty())
            dirpath = QCoreApplication::applicationDirPath() + "/" + filename + (quicklook ? ".jpg" : ".zip");
        else
            dirpath = opts.productdirectory + "/" + filename + (quicklook ? ".jpg" : ".zip");

        if(quicklook)
        {
            QImage img = QImage::fromData(*m_pBuffer,"JPG");
            img.save(opts.productdirectory + "/" + "myimage.jpg");
        }

        QFile file(dirpath);

        if(file.open(QIODevice::WriteOnly))
        {
            file.write(*m_pBuffer);
            file.close();
            qDebug() << "File has been saved to " << dirpath;
        }
        else
        {
            qDebug() << "Error saving file!";
        }
    }

    m_pBuffer->clear();
    isProductBusy = false;
    emit productFinished(whichdownload, downloadindex, quicklook);

}

void DatahubAccessManager::slotdownloadproductProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit productProgress(bytesReceived, bytesTotal, whichdownload);
}

DatahubAccessManager::~DatahubAccessManager()
{
    qDebug() << "destroy DatahubAccessManager";
    delete m_pBuffer;
}

