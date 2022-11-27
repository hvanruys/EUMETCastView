#include <QFile>
#include <QDebug>
#include <QDir>
#include <QImage>
#include "xmlvideo.h"

XMLVideo::XMLVideo(QString xmlfile, QObject *parent) : QObject(parent)
{
    this->spectrum.resize(6);
    this->inverse.resize(6);

    parseXML(xmlfile);
}

void XMLVideo::parseXML(QString xmlfile)
{
    /* We'll parse the example.xml */
    QFile* file = new QFile(xmlfile);
    /* If we can't open it, let's show an error message. */
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Couldn't open xml file";
        return;
    }
    /* QXmlStreamReader takes any QIODevice. */
    QXmlStreamReader xml(file);
    /* We'll parse the XML until we reach end of it.*/
    while(!xml.atEnd() && !xml.hasError())
    {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::Invalid) {
            qDebug() << "Invalid token";
            break;
        }

        /* If token is just StartDocument, we'll go to next.*/
        if(token == QXmlStreamReader::StartDocument) {
            continue;
        }
        /* If token is StartElement, we'll see if we can read it.*/
        if(token == QXmlStreamReader::StartElement)
        {
            qDebug() << "startelement " << xml.name().toString() << "  namespaceuri = " << xml.namespaceUri().string();
            if(xml.name() == "root") {
                continue;
            }
            if(xml.name() == "dayspectrum") {
                continue;
            }
            if(xml.name() == "nightspectrum") {
                continue;
            }
            if(xml.name() == "resolution") {
                continue;
            }
            if(xml.name() == "gshhs") {
                continue;
            }
            if(xml.name() == "overlay") {
                continue;
            }
            if(xml.name() == "paths") {
                continue;
            }
            if(xml.name() == "rss") {
                xml.readNext();
                this->rss = xml.text().toInt();
            }
            if(xml.name() == "overlayborder") {
                xml.readNext();
                this->overlayborder = xml.text().toInt();
            }
            if(xml.name() == "overlaydate") {
                xml.readNext();
                this->overlaydate = xml.text().toInt();
            }
            if(xml.name() == "path") {
                xml.readNext();
                this->rsspath.append(xml.text().toString());
            }
            if(xml.name() == "pattern") {
                xml.readNext();
                this->filepattern = xml.text().toString();
            }
            if(xml.name() == "date") {
                QXmlStreamAttributes attributes = xml.attributes();
                if(attributes.hasAttribute("id")) {
                    //qDebug() << "attribute = " << attributes.value("id").toString();
                }

                xml.readNext();
                //qDebug() << "date = " << xml.text().toString();
                this->strdates.append(xml.text().toString());
            }
            if(xml.name() == "dayred") {
                xml.readNext();
                spectrum[0] = xml.text().toString();
            }
            if(xml.name() == "daygreen") {
                xml.readNext();
                spectrum[1] = xml.text().toString();
            }
            if(xml.name() == "dayblue") {
                xml.readNext();
                spectrum[2] = xml.text().toString();
            }
            if(xml.name() == "dayredinverse") {
                xml.readNext();
                inverse[0] = xml.text().toInt();
            }
            if(xml.name() == "daygreeninverse") {
                xml.readNext();
                inverse[1] = xml.text().toInt();
            }
            if(xml.name() == "dayblueinverse") {
                xml.readNext();
                inverse[2] = xml.text().toInt();
            }
            if(xml.name() == "nightred") {
                xml.readNext();
                spectrum[3] = xml.text().toString();
            }
            if(xml.name() == "nightgreen") {
                xml.readNext();
                spectrum[4] = xml.text().toString();
            }
            if(xml.name() == "nightblue") {
                xml.readNext();
                spectrum[5] = xml.text().toString();
            }
            if(xml.name() == "nightredinverse") {
                xml.readNext();
                inverse[3] = xml.text().toInt();
            }
            if(xml.name() == "nightgreeninverse") {
                xml.readNext();
                inverse[4] = xml.text().toInt();
            }
            if(xml.name() == "nightblueinverse") {
                xml.readNext();
                inverse[5] = xml.text().toInt();
            }
            if(xml.name() == "height") {
                xml.readNext();
                videoheight = xml.text().toInt();
            }
            if(xml.name() == "width") {
                xml.readNext();
                videowidth = xml.text().toInt();
            }
            if(xml.name() == "dayhrv") {
                xml.readNext();
                hrv = xml.text().toInt();
            }
            if(xml.name() == "gshhsoverlayfile") {
                xml.readNext();
                this->gshhsoverlayfiles.append(xml.text().toString());
            }
            if(xml.name() == "coff") {
                xml.readNext();
                coff = xml.text().toLongLong();
            }
            if(xml.name() == "loff") {
                xml.readNext();
                loff = xml.text().toLongLong();
            }
            if(xml.name() == "cfac") {
                xml.readNext();
                cfac = xml.text().toDouble();
            }
            if(xml.name() == "lfac") {
                xml.readNext();
                lfac = xml.text().toDouble();
            }
            if(xml.name() == "coffhrv") {
                xml.readNext();
                coffhrv = xml.text().toLongLong();
            }
            if(xml.name() == "loffhrv") {
                xml.readNext();
                loffhrv = xml.text().toLongLong();
            }
            if(xml.name() == "cfachrv") {
                xml.readNext();
                cfachrv = xml.text().toDouble();
            }
            if(xml.name() == "lfachrv") {
                xml.readNext();
                lfachrv = xml.text().toDouble();
            }
            if(xml.name() == "satlon") {
                xml.readNext();
                satlon = xml.text().toDouble();
            }
            if(xml.name() == "homelon") {
                xml.readNext();
                homelon = xml.text().toDouble();
            }
            if(xml.name() == "homelat") {
                xml.readNext();
                homelat = xml.text().toDouble();
            }
            if(xml.name() == "gamma") {
                xml.readNext();
                gamma = xml.text().toFloat();
            }
        }
    }

    if(xml.hasError()) {
        qDebug() << xml.errorString();
    }

    if(hrv && spectrum.at(0).length() == 0 && spectrum.at(1).length() == 0 && spectrum.at(2).length() == 0)
        this->daykindofimage = "HRV";
    else if(hrv && spectrum.at(0).length() > 0 && spectrum.at(1).length() > 0 && spectrum.at(2).length() > 0)
        this->daykindofimage = "HRV Color";
    else if(!hrv && spectrum.at(0).length() > 0 && spectrum.at(1).length() == 0 && spectrum.at(2).length() == 0)
        this->daykindofimage = "VIS_IR";
    else if(!hrv && spectrum.at(0).length() > 0 && spectrum.at(1).length() > 0 && spectrum.at(2).length() > 0)
        this->daykindofimage = "VIS_IR Color";
    else
        this->daykindofimage = "";
    qDebug() << "daykindofimage = " << this->daykindofimage;
    if(spectrum.at(3).length() > 0 && spectrum.at(4).length() == 0 && spectrum.at(5).length() == 0)
        this->nightkindofimage = "VIS_IR";
    else if(spectrum.at(3).length() > 0 && spectrum.at(4).length() > 0 && spectrum.at(5).length() > 0)
        this->nightkindofimage = "VIS_IR Color";
    else
        this->nightkindofimage = "NONE";
    qDebug() << "nightkindofimage = " << this->nightkindofimage;

    xml.clear();
}

