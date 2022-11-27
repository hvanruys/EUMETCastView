#ifndef XMLVIDEO_H
#define XMLVIDEO_H

#include <QXmlStreamReader>
#include <QObject>

class XMLVideo : public QObject
{
    Q_OBJECT
public:
    explicit XMLVideo(QString xmlfile, QObject *parent = nullptr);
    void parseXML(QString xmlfile);
    QVector<QString> rsspath;
    QString filepattern;
    QVector<QString> strdates;
    QVector<QString> spectrum;
    QVector<bool> inverse;
    QVector<QString> gshhsoverlayfiles;
    int videoheight;
    int videowidth;
    bool hrv;
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
    bool overlayborder;
    bool overlaydate;
    float gamma;
    bool rss;

signals:

public slots:
};

#endif // XMLVIDEO_H
