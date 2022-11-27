#include <QFile>
#include <QDebug>
#include <QDir>
#include <QImage>
#include "xmlvideoreader.h"

XMLVideoReader::XMLVideoReader(QString xmlfile, QString argsingleimage, QObject *parent) : QObject(parent)
{
    this->spectrum.resize(6);
    this->inverse.resize(6);
    this->argsingleimage = argsingleimage;
    parseXML(xmlfile);
}

void XMLVideoReader::parseXML(QString xmlfile)
{
    QFile file(xmlfile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Couldn't open xml file " << file.errorString();
        return;
    }

    if (!this->read(&file))
        qDebug() << "Parse error in file " << xmlfile; //this->errorString();

}

bool XMLVideoReader::read(QIODevice *device)
{
    reader.setDevice(device);

    if (reader.readNextStartElement()) {
        if (reader.name() == "root")
            readVideoParameters();
        else
            reader.raiseError(QObject::tr("Not an xml file"));
    }
    return !reader.error();
}

void XMLVideoReader::readVideoParameters()
{
    bool ok = true;
    while(reader.readNextStartElement()){
        if(reader.name() == "threadcount")
            this->threadcount = reader.readElementText().toInt();
        else if(reader.name() == "pathlist")
            readPaths();
        else if(reader.name() == "gshhs")
            readGshhs();
        else if(reader.name() == "resolution")
            readResolution();
        else if(reader.name() == "pattern")
            this->filepattern = reader.readElementText();
        else if(reader.name() == "singleimage")
        {
            if(this->argsingleimage.length() == 0)
                this->singleimage = reader.readElementText();
            else
                this->singleimage = argsingleimage;
        }
        else if(reader.name() == "satname")
            this->satname = reader.readElementText();
        else if(reader.name() == "gamma")
            this->gamma = reader.readElementText().toFloat();
        else if(reader.name() == "rss")
            this->brss = reader.readElementText().toInt();
        else if(reader.name() == "dayspectrum")
            readDaySpectrum();
        else if(reader.name() == "nightspectrum")
            readNightSpectrum();
        else if(reader.name() == "overlay")
            readOverlayParameters();
        else if(reader.name() == "projectiontype")
            this->projectiontype = reader.readElementText();
        else if(reader.name() == "gvpprojectionparameters")
            readGVPParameters();
        else if(reader.name() == "videooutputname")
            this->videooutputname = reader.readElementText();

        else
            reader.skipCurrentElement();
    }

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

    if(spectrum.at(3).length() > 0 && spectrum.at(4).length() == 0 && spectrum.at(5).length() == 0)
        this->nightkindofimage = "VIS_IR";
    else if(spectrum.at(3).length() > 0 && spectrum.at(4).length() > 0 && spectrum.at(5).length() > 0)
        this->nightkindofimage = "VIS_IR Color";
    else
        this->nightkindofimage = "";
    qDebug() << "nightkindofimage = " << this->nightkindofimage;

    reader.clear();


}

void XMLVideoReader::readPaths()
{
    while(reader.readNextStartElement()){
        if(reader.name() == "path")
        {
            QString str = reader.readElementText();
            rsspath.append(str);
        }
        else
            reader.skipCurrentElement();
    }
}

void XMLVideoReader::readGshhs()
{
    while(reader.readNextStartElement()){
        if(reader.name() == "gshhsoverlayfile1")
        {
            QString str = reader.readElementText();
            gshhsoverlayfiles.append(str);
        }
        else if(reader.name() == "gshhsoverlayfile2")
        {
            QString str = reader.readElementText();
            gshhsoverlayfiles.append(str);
        }
        else if(reader.name() == "gshhsoverlayfile3")
        {
            QString str = reader.readElementText();
            gshhsoverlayfiles.append(str);
        }
        else if(reader.name() == "gshhsglobe1On")
        {
            bgshhsglobeOn[0] = reader.readElementText().toInt();
        }
        else if(reader.name() == "gshhsglobe2On")
        {
            bgshhsglobeOn[1] = reader.readElementText().toInt();
        }
        else if(reader.name() == "gshhsglobe3On")
        {
            bgshhsglobeOn[2] = reader.readElementText().toInt();
        }
        else
            reader.skipCurrentElement();
    }
}

void XMLVideoReader::readResolution()
{
    while(reader.readNextStartElement()){
        if(reader.name() == "height")
            this->videoheight = reader.readElementText().toInt();
        else if(reader.name() == "width")
            this->videowidth = reader.readElementText().toInt();
        else
            reader.skipCurrentElement();

    }
}

void XMLVideoReader::readDaySpectrum()
{
    while(reader.readNextStartElement()){
        if(reader.name() == "dayred")
            this->spectrum[0] = reader.readElementText();
        else if(reader.name() == "daygreen")
            this->spectrum[1] = reader.readElementText();
        else if(reader.name() == "dayblue")
            this->spectrum[2] = reader.readElementText();
        else if(reader.name() == "dayredinverse")
            this->inverse[0] = reader.readElementText().toInt();
        else if(reader.name() == "daygreeninverse")
            this->inverse[1] = reader.readElementText().toInt();
        else if(reader.name() == "dayblueinverse")
            this->inverse[2] = reader.readElementText().toInt();
        else if(reader.name() == "dayhrv")
            this->bhrv = reader.readElementText().toInt();
        else
            reader.skipCurrentElement();
    }
}

void XMLVideoReader::readNightSpectrum()
{
    while(reader.readNextStartElement()){
        if(reader.name() == "nightred")
            this->spectrum[3] = reader.readElementText();
        else if(reader.name() == "nightgreen")
            this->spectrum[4] = reader.readElementText();
        else if(reader.name() == "nightblue")
            this->spectrum[5] = reader.readElementText();
        else if(reader.name() == "nightredinverse")
            this->inverse[3] = reader.readElementText().toInt();
        else if(reader.name() == "nightgreeninverse")
            this->inverse[4] = reader.readElementText().toInt();
        else if(reader.name() == "nightblueinverse")
            this->inverse[5] = reader.readElementText().toInt();
        else
            reader.skipCurrentElement();
    }
}

void XMLVideoReader::readOverlayParameters()
{
    while(reader.readNextStartElement()){
        if(reader.name() == "coff")
            this->coff = reader.readElementText().toULongLong();
        else if(reader.name() == "loff")
            this->loff = reader.readElementText().toULongLong();
        else if(reader.name() == "cfac")
            this->cfac = reader.readElementText().toDouble();
        else if(reader.name() == "lfac")
            this->lfac = reader.readElementText().toDouble();
        else if(reader.name() == "coffhrv")
            this->coffhrv = reader.readElementText().toLongLong();
        else if(reader.name() == "loffhrv")
            this->loffhrv = reader.readElementText().toLongLong();
        else if(reader.name() == "cfachrv")
            this->cfachrv = reader.readElementText().toDouble();
        else if(reader.name() == "lfachrv")
            this->lfachrv = reader.readElementText().toDouble();
        else if(reader.name() == "satlon")
            this->satlon = reader.readElementText().toDouble();
        else if(reader.name() == "homelon")
            this->homelon = reader.readElementText().toDouble();
        else if(reader.name() == "homelat")
            this->homelat = reader.readElementText().toDouble();
        else if(reader.name() == "projectionoverlaycolor1")
            this->projectionoverlaycolor1 = reader.readElementText();
        else if(reader.name() == "projectionoverlaycolor2")
            this->projectionoverlaycolor2 = reader.readElementText();
        else if(reader.name() == "projectionoverlaycolor3")
            this->projectionoverlaycolor3 = reader.readElementText();
        else if(reader.name() == "projectionoverlaygridcolor")
            this->projectionoverlaygridcolor = reader.readElementText();
        else if(reader.name() == "overlayborder")
            this->boverlayborder = reader.readElementText().toInt();
        else if(reader.name() == "overlaydate")
            this->boverlaydate = reader.readElementText().toInt();
        else if(reader.name() == "overlaydatefontsize")
            this->overlaydatefontsize = reader.readElementText().toInt();
        else
            reader.skipCurrentElement();
    }
}

void XMLVideoReader::readGVPParameters()
{
    while(reader.readNextStartElement()){
        if(reader.name() == "latitude")
            this->gvplatitude = reader.readElementText().toDouble();
        else if(reader.name() == "longitude")
            this->gvplongitude = reader.readElementText().toDouble();
        else if(reader.name() == "scale")
            this->gvpscale = reader.readElementText().toDouble();
        else if(reader.name() == "height")
            this->gvpheight = reader.readElementText().toLong();
        else if(reader.name() == "gridonprojection")
            this->gvpgridonproj = reader.readElementText().toInt();
        else if(reader.name() == "falseeasting")
            this->gvpfalseeasting = reader.readElementText().toDouble();
        else if(reader.name() == "falsenorthing")
            this->gvpfalsenorthing = reader.readElementText().toDouble();
        else
            reader.skipCurrentElement();
    }
}
