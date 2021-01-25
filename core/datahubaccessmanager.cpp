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
    qDebug()<<"SSL version use for build: "<<QSslSocket::sslLibraryBuildVersionString();
    qDebug()<<"SSL version use for run-time: "<<QSslSocket::sslLibraryVersionNumber();
    qDebug()<<QCoreApplication::libraryPaths();

    Q_ASSERT(nbrofpages > 0);

    m_pBuffer->clear();
    this->nbrofpages = nbrofpages;
    this->nbrofpagescounter = 1;
    this->selectdate = "";
    this->hub = hub;

    QDomElement root = docout.createElement(selectdate);
    docout.appendChild(root);

    if(this->hub == HUBESA)
        strurl = QString("https://scihub.copernicus.eu/dhus/search?q=*&start=%1&rows=100").arg(0);
    else
        strurl = QString("https://coda.eumetsat.int/search?q=*&start=%1&rows=100").arg(0);

    qDebug() << strurl;

    url = QUrl(strurl);
    // HTTP Basic authentication header value: base64(username:password)
    QString concatenated;
    if(this->hub == HUBESA)
        concatenated = opts.datahubuser + ":" + opts.datahubpassword;
    else
        concatenated = opts.datahubuser + ":" + opts.datahubpassword;

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

void DatahubAccessManager::DownloadXML(QDate selectdate, eDatahub hub, QString type)
{
    QString strurl;
    QUrl url;

    typetodownload = type;
    qDebug() << "start DownloadXML";
    qDebug()<<"SSL version use for build: "<<QSslSocket::sslLibraryBuildVersionString();
    qDebug()<<"SSL version use for run-time: "<<QSslSocket::sslLibraryVersionNumber();
    qDebug()<<QCoreApplication::libraryPaths();

    if(opts.xmllogging)
    {
        QFile xmlloggingfile(QCoreApplication::applicationDirPath() + "/xmllogging.txt");
        if (xmlloggingfile.open(QFile::WriteOnly | QFile::Truncate)) {
            QTextStream out(&xmlloggingfile);
            out << "Start logging : " << QDate::currentDate().toString();
            out << "SSL version use for build: "<<QSslSocket::sslLibraryBuildVersionString();
            out << "SSL version use for run-time: "<<QSslSocket::sslLibraryVersionNumber();

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

    QString resourcepath = getresourcepath(this->selectdate, nbrofpagescounter - 1, type);
    if (resourcepath.isEmpty())
        return;

    if(this->hub == HUBESA)
        strurl = "https://scihub.copernicus.eu/dhus/search?q=" + resourcepath;
    else
        strurl = "https://coda.eumetsat.int/search?q=" + resourcepath;

    qDebug() << strurl;

    url = QUrl(strurl);
    // HTTP Basic authentication header value: base64(username:password)
    //    QString concatenated;
    //    if(this->hub == HUBESA)
    //        concatenated = opts.esauser + ":" + opts.esapassword;
    //    else
    //        concatenated = opts.eumetsatuser + ":" + opts.eumetsatpassword;

    //    QByteArray data = concatenated.toLocal8Bit().toBase64();
    //    QString headerData = "Basic " + data;

    QString concatenated;
    if(this->hub == HUBESA)
        concatenated = opts.datahubuser + ":" + opts.datahubpassword;
    else
        concatenated = opts.datahubuser + ":" + opts.datahubpassword;

    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    reply = networkaccessmanager.get(request);


    // qDebug() << "Errorstring = " << reply->errorString() << " errorcode = " << reply->error();

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
            strurl = "https://scihub.copernicus.eu/dhus/search?q=" + getresourcepath(this->selectdate, nbrofpagescounter - 1, typetodownload);
        else
            strurl = "https://coda.eumetsat.int/search?q=" + getresourcepath(this->selectdate, nbrofpagescounter - 1, typetodownload);

        QUrl url = QUrl(strurl);
        qDebug() << strurl;

        request.setUrl(url);

        // HTTP Basic authentication header value: base64(username:password)
        QString concatenated;
        if(this->hub == HUBESA)
            concatenated = opts.datahubuser + ":" + opts.datahubpassword;
        else
            concatenated = opts.datahubuser + ":" + opts.datahubpassword;
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
    QString uuid, filename, contentlength, footprint;

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

    for(int i = 0; i < entries.count(); i++)
    {

        QDomNode entrynode = entries.at(i);
        QDomNodeList childnodes = entrynode.childNodes();

        for(int j = 0; j < childnodes.count(); j++)
        {
            QDomNode childnode = childnodes.at(j);
            //convert to an element
            if(childnode.isElement())
            {
                QDomElement child = childnode.toElement();
                if(child.tagName() == "str")
                {
                    if(child.attribute("name") == "uuid")
                        uuid = child.text();
                    else if(child.attribute("name") == "filename")
                        filename = child.text();
                    else if(child.attribute("name") == "size")
                        contentlength = child.text();
                    else if(child.attribute("name") == "footprint")
                        footprint = child.text();
                }
            }
        }


        QDomElement segment = docout.createElement("Segment");
        segment.setAttribute("Name", filename);
        segment.setAttribute("size", contentlength);
        segment.setAttribute("uuid", uuid);
        segment.setAttribute("footprint", footprint);
        docout.firstChildElement().appendChild(segment);

    }

    m_pBuffer->clear();

    if(entries.count() < 100)
        return false;

    return true;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief DatahubAccessManager::download the products
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QString DatahubAccessManager::getresourcepath(QString strselectdate, int page, QString type)
{
    //S3A_OL_1_EFR____20170131
    int start = page * 100;
    QString resourcepath = "";
    QString resourcepatholciefr = QString("filename:S3?_OL_1_EFR____%1*&start=%2&rows=100&orderby=beginposition asc").arg(strselectdate).arg(start);
    QString resourcepatholcierr = QString("filename:S3?_OL_1_ERR____%1*&start=%2&rows=100&orderby=beginposition asc").arg(strselectdate).arg(start);
    QString resourcepathslstr = QString("filename:S3?_SL_1_RBT____%1*&start=%2&rows=100&orderby=beginposition asc").arg(strselectdate).arg(start);


    if(type == "EFR")
        resourcepath = resourcepatholciefr;
    else if(type == "ERR")
        resourcepath = resourcepatholcierr;
    else if(type == "SLSTR")
        resourcepath = resourcepathslstr;

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

void DatahubAccessManager::DownloadProduct(QList<ProductList> prodlist, int index, eDatahub hub, int whichdownload)
{

    isAborted = false;
    isProductBusy = true;
    this->whichdownload = whichdownload;
    this->downloadindex = index;

    this->completebasename = prodlist.at(index).completebasename;
    this->uuid = prodlist.at(index).uuid;
    this->band_or_quicklook = prodlist.at(index).band_or_quicklook;
    QString strurl;


    if(hub == HUBESA)
    {
        if(band_or_quicklook == "quicklook")
            strurl = QString("https://scihub.copernicus.eu/dhus/odata/v1/Products('%1')/Products('Quicklook')/$value").arg(uuid);
        else if(band_or_quicklook == "complete")
            strurl = QString("https://scihub.copernicus.eu/dhus/odata/v1/Products('%1')/$value").arg(prodlist.at(index).uuid);
        else
            strurl = QString("https://scihub.copernicus.eu/dhus/odata/v1/Products('%1')/Nodes('%2')/Nodes('%3')/$value")
                    .arg(uuid).arg(completebasename).arg(band_or_quicklook);
    }
    else
    {
        if(band_or_quicklook == "quicklook")
            strurl = QString("https://coda.eumetsat.int/odata/v1/Products('%1')/Products('Quicklook')/$value").arg(uuid);
        else if(band_or_quicklook == "complete")
            strurl = QString("https://coda.eumetsat.int/odata/v1/Products('%1')/$value").arg(prodlist.at(index).uuid);
        else
            strurl = QString("https://coda.eumetsat.int/odata/v1/Products('%1')/Nodes('%2')/Nodes('%3')/$value")
                    .arg(uuid).arg(completebasename).arg(band_or_quicklook);
    }

    qDebug() << strurl;

    QUrl url(strurl);

    // HTTP Basic authentication header value: base64(username:password)
    QString concatenated;
    if(hub == HUBESA)
        concatenated = opts.datahubuser + ":" + opts.datahubpassword;
    else
        concatenated = opts.datahubuser + ":" + opts.datahubpassword;
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

    disconnect(reply,SIGNAL(readyRead()),this,SLOT(slotReadDataProduct()));
    disconnect(reply,SIGNAL(finished()), this,SLOT(slotFinishedProduct()));
    reply->deleteLater();
    reply = NULL;

    if(!isAborted)
    {

        QString aboluteproductpath;
        QString absolutepath;
        QString filename;
        QDir dir = TestForDirectory();
        absolutepath = dir.absolutePath();
        if(band_or_quicklook == "quicklook")
        {
            filename = completebasename + ".jpg";
            aboluteproductpath = dir.absolutePath() + "/" + completebasename + ".jpg";
        }
        else if(band_or_quicklook == "complete")
        {
            filename = completebasename + ".zip";
            aboluteproductpath = dir.absolutePath() + "/" + completebasename + ".zip";
        }
        else
        {
            filename = completebasename;
            aboluteproductpath = dir.absolutePath() + "/" + band_or_quicklook;
        }

        //        if(quicklook)
        //        {
        //            QImage img = QImage::fromData(*m_pBuffer,"JPG");
        //            img.save(opts.productdirectory + "/quicklook/" + filename + ".jpg");
        //        }

        QFile file(aboluteproductpath);

        if(file.open(QIODevice::WriteOnly))
        {
            file.write(*m_pBuffer);
            file.close();

            qDebug() << "File has been saved to " << aboluteproductpath;
        }
        else
        {
            qDebug() << "Error saving file!";
        }
        emit productFinished(whichdownload, downloadindex, aboluteproductpath, absolutepath, filename);

    }

    m_pBuffer->clear();
    isProductBusy = false;

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

//void DatahubAccessManager::TestForDirectory(QString dirpath)
//{
//    QDir thedir(dirpath);
//    if(!thedir.exists())
//    {
//        thedir.mkdir(dirpath);
//    }

//}

QDir DatahubAccessManager::TestForDirectory()
{
    // S3A_OL_1_EFR____20201205T102330_20201205T102630_20201205T121305_0179_066_008_2340_LN1_O_NR_002.SEN3
    // S3A_OL_1_ERR____20201210T094858_20201210T103307_20201210T115918_2649_066_079______LN1_O_NR_002.SEN3
    // 01234567890123456789012345678901234567890123456789012345678901234567890123456789

    bool dirok;
    QDir dir(opts.productdirectory);
    QString returndirstr;

    QString fileyear = completebasename.mid(16, 4);
    QString filemonth = completebasename.mid(20, 2);
    QString fileday = completebasename.mid(22,2);

    QDir dirdateyear(dir.absolutePath() + "/" + fileyear);
    QDir dirdatemonth(dir.absolutePath() + "/" + fileyear + "/" + filemonth);
    QDir dirdateday(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday);
    QDir dirdateproduct(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday + "/" + completebasename);

    dirok = false;
    if (!dirdateyear.exists())
    {
        returndirstr = dir.absolutePath() + "/" + fileyear;
        dirok = dir.mkdir(returndirstr);
        qDebug() << returndirstr;
    }
    if (!dirdatemonth.exists())
    {
        returndirstr = dir.absolutePath() + "/" + fileyear + "/" + filemonth;
        dirok = dir.mkdir(returndirstr);
        qDebug() << returndirstr;
    }
    if (!dirdateday.exists())
    {
        returndirstr = dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday;
        dirok = dir.mkdir(returndirstr);
        qDebug() << returndirstr;
    }

    if (band_or_quicklook == "quicklook")
    {
        if (!dirdateproduct.exists())
        {
            returndirstr = dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday + "/" + completebasename;
            dirok = dir.mkdir(returndirstr);
            qDebug() << returndirstr;
        }
        returndirstr = dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday + "/" + completebasename + "/quicklook";
        dirok = dir.mkdir(returndirstr);
        qDebug() << returndirstr;
    }
    else if(band_or_quicklook == "complete")
    {
        returndirstr = dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday;
        qDebug() << returndirstr;
    }
    else
    {
        if (!dirdateproduct.exists())
        {
            returndirstr = dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday + "/" + completebasename;
            dirok = dir.mkdir(returndirstr);
            qDebug() << returndirstr;
        }

        returndirstr = dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday + "/" + completebasename;
    }

    return QDir(returndirstr);

}
