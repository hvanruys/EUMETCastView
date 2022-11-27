#ifndef XMLVIDEOREADER_H
#define XMLVIDEOREADER_H

#include <QObject>
#include <QXmlStreamReader>

class XMLVideoReader : public QObject
{
    Q_OBJECT
public:
    explicit XMLVideoReader(QString xmlfile, QString argsingleimage, QObject *parent = nullptr);
    void parseXML(QString xmlfile);
    bool read(QIODevice *device);
//    QString errorString() const;


    void readVideoParameters();
    void readPaths();
    void readGshhs();
    void readResolution();
    void readDaySpectrum();
    void readNightSpectrum();
    void readOverlayParameters();
    void readGVPParameters();

    QString argsingleimage;
    QVector<QString> rsspath;
    QVector<QString> gshhsoverlayfiles;
    int videowidth;
    int videoheight;
    QXmlStreamReader reader;

    QString filepattern;
    QString satname;
    QVector<QString> strdates;
    QVector<QString> spectrum;
    QVector<bool> inverse;
    bool bhrv;
    QString daykindofimage;
    QString nightkindofimage;
    qlonglong coff;
    qlonglong loff;
    double cfac;
    double lfac;
    qlonglong coffhrv;
    qlonglong loffhrv;
    double cfachrv;
    double lfachrv;
    double satlon;
    double homelon;
    double homelat;
    bool boverlayborder;
    bool boverlaydate;
    float gamma;
    bool bgshhsglobeOn[3];
    QString projectionoverlaycolor1;
    QString projectionoverlaycolor2;
    QString projectionoverlaycolor3;
    QString projectionoverlaygridcolor;
    int overlaydatefontsize;
    bool brss;
    int threadcount;

    QString singleimage;
    QString projectiontype;
    QString videooutputname;
    //GVP
    double gvplongitude;
    double gvplatitude;
    double gvpscale;
    long gvpheight;
    bool gvpgridonproj;
    double gvpfalseeasting;
    double gvpfalsenorthing;



signals:

};

#endif // XMLVIDEOREADER_H
