#ifndef JSONVIDEOREADER_H
#define JSONVIDEOREADER_H

#include <qobject.h>
#include <QJsonDocument>
#include <QJsonObject>

class JsonVideoReader : public QObject
{
    Q_OBJECT
public:
    explicit JsonVideoReader(QString jsonfile, QString timestamp, QObject *parent = nullptr);

    void dumpJsonFile();

    int geoindex;
    bool brss;
    bool bhrv;
    double gamma;
    QString selectiondate;
    QString daykindofimage;
    QString shortname;
    QString singleimage;

    QList<QString> gshhsoverlayfileslist;
    QList<QString> projectionoverlaycolorlist;
    QList<QString> segmentspathlist;
    QList<bool> gshhsoverlayOnlist;
    int videowidth;
    int videoheight;
    QString ffmpegparameters;


    QString projectiontype;
    QString videooutputname;

    double cfac;
    double lfac;
    qlonglong coff;
    qlonglong loff;
    double cfachrv;
    double lfachrv;
    qlonglong coffhrv;
    qlonglong loffhrv;
    double homelon;
    double homelat;
    bool boverlayborder;
    bool boverlaydate;
    int overlaydatefontsize;
    double satlon;


    //GVP
    double gvplongitude;
    double gvplatitude;
    double gvpscale;
    long gvpheight;
    bool gvpgridonproj;
    double gvpfalseeasting;
    double gvpfalsenorthing;

    QList<QString> spectrum;
    QList<bool> inverse;

    int maxprocesscount;
    QString timestamp;

    // QString filepattern;
    // QString satname;
    // QVector<QString> strdates;
     // QString nightkindofimage;
    // int threadcount;

    QJsonDocument jsondoc;

private:

    void parseFileData(const QJsonObject &fileObj);
    void parseChannelData(const QJsonObject &channelObj);
    void parseTimestampData(const QJsonObject &timestampObj);

    void getTimestampData(const QJsonObject &timestampObj);
    void getTimestampDataMTG(const QJsonObject &timestampObj);
    void getChannelData(const QJsonObject &channelObj, int color);

    void readVideoParameters();
    void readVideoPaths();
    void readVideoPathsMTG();

signals:
};

#endif // JSONVIDEOREADER_H
