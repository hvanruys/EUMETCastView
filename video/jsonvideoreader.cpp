#include "jsonvideoreader.h"
#include <QFile>
#include <QDebug>
#include <QJsonParseError>


JsonVideoReader::JsonVideoReader(QString jsonfile, QString timestamp, QObject *parent) : QObject(parent)
{
    // Open the JSON file
    QFile file(jsonfile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open " << jsonfile << ":" << file.errorString();
    }

    // Read file contents
    QByteArray jsonData = file.readAll();
    file.close();

    // Parse JSON
    QJsonParseError parseError;
    jsondoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
    }

    if (!jsondoc.isObject()) {
        qWarning() << "Root is not a JSON object";
    }
    this->timestamp = timestamp;

    readVideoParameters();

    qDebug() << this->spectrum << "daykindofimage = " << this->daykindofimage;

    if(this->shortname == "MET_12")
    {
        readVideoPathsMTG();
    }
    else
        readVideoPaths();

    // for(int i = 0; i < segmentspathlist.count(); i++)
    // {
    //     qDebug() << i << " " << segmentspathlist.at(i);
    // }


}

void JsonVideoReader::dumpJsonFile()
{
    QJsonObject root = jsondoc.object();

    // Parse files section
    if (root.contains("files") && root["files"].isObject()) {
        QJsonObject filesObj = root["files"].toObject();
        qDebug() << "=== Files ===";

        for (const QString &timestampKey : filesObj.keys()) {
            qDebug() << "Timestamp:" << timestampKey;
            QJsonObject timestampObj = filesObj[timestampKey].toObject();
            parseTimestampData(timestampObj);
            qDebug() << "";
        }
    }
}

void JsonVideoReader::parseFileData(const QJsonObject &fileObj) {
    qDebug() << "      Absolute Path:" << fileObj["absoluteFilePath"].toString();
    qDebug() << "      Size:" << fileObj["size"].toInt() << "bytes";
}

void JsonVideoReader::parseChannelData(const QJsonObject &channelObj) {
    for (const QString &segmentKey : channelObj.keys()) {
        qDebug() << "    Segment:" << segmentKey;
        QJsonObject segmentObj = channelObj[segmentKey].toObject();
        parseFileData(segmentObj);
    }
}

void JsonVideoReader::parseTimestampData(const QJsonObject &timestampObj) {
    for (const QString &channelKey : timestampObj.keys()) {
        qDebug() << "  Channel:" << channelKey;
        QJsonObject channelObj = timestampObj[channelKey].toObject();
        parseChannelData(channelObj);
    }
}

void JsonVideoReader::readVideoPaths()
{
    QJsonObject root = jsondoc.object();

    // Parse files section
    if (root.contains("files") && root["files"].isObject()) {
        QJsonObject filesObj = root["files"].toObject();
         for (const QString &timestampKey : filesObj.keys()) {
            if(timestampKey == this->timestamp)
            {
                QJsonObject timestampObj = filesObj[timestampKey].toObject();
                getTimestampData(timestampObj);
            }
        }
    }
    return;
}

void JsonVideoReader::readVideoPathsMTG()
{
    QJsonObject root = jsondoc.object();

    qDebug() << this->timestamp;

    // Parse files section
    if (root.contains("files") && root["files"].isObject()) {
        QJsonObject filesObj = root["files"].toObject();
        for (const QString &timestampKey : filesObj.keys()) {
            if(timestampKey == this->timestamp)
            {
                QJsonObject timestampObj = filesObj[timestampKey].toObject();
                getTimestampDataMTG(timestampObj);
            }
        }
    }

    qDebug() << "segmentspathlist count = " << this->segmentspathlist.count();
    return;
}

void JsonVideoReader::getTimestampData(const QJsonObject &timestampObj) {
    for (const QString &channelKey : timestampObj.keys())
    {
        QJsonObject channelObj = timestampObj[channelKey].toObject();
        if( channelKey == this->spectrum.at(0) )
        {
            getChannelData(channelObj, 0);

        }
        else if( channelKey == this->spectrum.at(1) )
        {
            getChannelData(channelObj, 1);

        }
        else if( channelKey == this->spectrum.at(2) )
        {
            getChannelData(channelObj, 2);

        }
        else if( channelKey == this->spectrum.at(3) )
        {
            getChannelData(channelObj, 3);
        }
    }
}

void JsonVideoReader::getTimestampDataMTG(const QJsonObject &timestampObj) {
    for (const QString &seqfileKey : timestampObj.keys())
    {
        QJsonObject seqfileObj = timestampObj[seqfileKey].toObject();
        segmentspathlist << seqfileObj["absoluteFilePath"].toString();
    }
}

void JsonVideoReader::getChannelData(const QJsonObject &channelObj, int color) {
    for (const QString &segmentKey : channelObj.keys()) {
        QJsonObject segmentObj = channelObj[segmentKey].toObject();
        segmentspathlist << segmentObj["absoluteFilePath"].toString();
    }
}

// void JsonVideoReader::getSeqfileDataMTG(const QJsonObject &seqfileObj) {
//     for (const QString &segmentKey : seqfileObj.keys()) {
//         //QJsonObject segmentObj = seqfileObj[segmentKey].toObject();
//         segmentspathlist << seqfileObj["absoluteFilePath"].toString();
//     }
// }


void JsonVideoReader::readVideoParameters()
{
    QJsonObject root = jsondoc.object();

    this->geoindex = root["geoindex"].toInt();
    this->brss = root["rss"].toBool();
    this->gamma = root["gamma"].toDouble();
    this->shortname = root["shortname"].toString();
    this->ffmpegparameters = root["ffmpegparameters"].toString();
    this->projectiontype = root["projectiontype"].toString();
    this->videooutputname = root["videooutputname"].toString();
    this->maxprocesscount = root["maxprocesscount"].toInt();
    this->selectiondate = root["selectiondate"].toString();

    QJsonObject object;
    object = QJsonObject();
    object = root["resolution"].toObject();
    this->videoheight = object["height"].toInteger();
    this->videowidth = object["width"].toInteger();
    qDebug() << "videoheight = " << this->videoheight << " videowidth = " << videowidth;



    object = QJsonObject();
    object = root["gshhs"].toObject();
    gshhsoverlayfileslist << object["gshhsoverlayfile1"].toString();
    gshhsoverlayfileslist << object["gshhsoverlayfile2"].toString();
    gshhsoverlayfileslist << object["gshhsoverlayfile3"].toString();
    gshhsoverlayOnlist << object["gshhsoverlayOn1"].toBool();
    gshhsoverlayOnlist << object["gshhsoverlayOn2"].toBool();
    gshhsoverlayOnlist << object["gshhsoverlayOn3"].toBool();
    projectionoverlaycolorlist << object["projectionoverlaycolor1"].toString();
    projectionoverlaycolorlist << object["projectionoverlaycolor2"].toString();
    projectionoverlaycolorlist << object["projectionoverlaycolor3"].toString();
    projectionoverlaycolorlist << object["projectionoverlaygridcolor"].toString();

    object = QJsonObject();
    object = root["overlay"].toObject();
    cfac = object["cfac"].toDouble();
    cfachrv = object["cfachrv"].toDouble();
    coff = object["coff"].toInteger();
    coffhrv = object["coffhrv"].toInteger();
    homelat = object["homelat"].toDouble();
    homelon = object["homelon"].toDouble();
    lfac = object["lfac"].toDouble();
    lfachrv = object["lfachrv"].toDouble();
    loff = object["loff"].toInteger();
    loffhrv = object["loffhrv"].toInteger();
    boverlayborder = object["overlayborder"].toBool();
    boverlaydate = object["overlaydate"].toBool();
    overlaydatefontsize = object["overlaydatefontsize"].toInt();
    satlon = object["satlon"].toDouble();

    object = QJsonObject();
    object = root["gvpprojectionparameters"].toObject();
    gvplongitude = object["longitude"].toDouble();
    gvplatitude = object["latitude"].toDouble();
    gvpscale = object["scale"].toDouble();
    gvpheight = object["height"].toInt();

    qDebug() << "GVP height = " << gvpheight;

    gvpgridonproj = object["gridonprojection"].toBool();
    gvpfalseeasting = object["falseeasting"].toDouble();;
    gvpfalsenorthing = object["falsenorthing"].toDouble();


    object = QJsonObject();
    object = root["spectrum"].toObject();
    spectrum << object["dayred"].toString();
    spectrum << object["daygreen"].toString();
    spectrum << object["dayblue"].toString();
    spectrum << object["nightred"].toString();

    inverse << object["dayredinverse"].toBool();
    inverse << object["daygreeninverse"].toBool();
    inverse << object["dayblueinverse"].toBool();
    inverse << object["nightredinverse"].toBool();

    bhrv = object["bhrv"].toBool();

    if(bhrv && spectrum.at(0).length() == 0 && spectrum.at(1).length() == 0 && spectrum.at(2).length() == 0)
        this->daykindofimage = "HRV";
    else if(bhrv && spectrum.at(0).length() > 0 && spectrum.at(1).length() > 0 && spectrum.at(2).length() > 0)
        this->daykindofimage = "HRV Color";
    else if(!bhrv && spectrum.at(0).length() > 0 && spectrum.at(1).length() == 0 && spectrum.at(2).length() == 0)
        this->daykindofimage = "VIS_IR";
    else if(!bhrv && spectrum.at(0).length() > 0 && spectrum.at(1).length() > 0 && spectrum.at(2).length() > 0)
        this->daykindofimage = "VIS_IR Color";
    else
        this->daykindofimage = "";
    qDebug() << "daykindofimage = " << this->daykindofimage;

}
