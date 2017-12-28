#ifndef SEGMENTLISTMETEOSAT_H
#define SEGMENTLISTMETEOSAT_H

#include <QObject>
#include <QFutureWatcher>
#include <QFileInfo>
#include <QVector2D>
#include "globals.h"

class SegmentListGeostationary : public QObject
{
    Q_OBJECT

public:

    explicit SegmentListGeostationary(QObject *parent = 0, int geoindex = 0);
    bool ComposeImageXRIT(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod);
    //bool ComposeImageHDFSerial(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector);
    bool ComposeImageHDFInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector);
    bool ComposeImagenetCDFInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod);

    void displayMinMax();
    void CalculateMinMax(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max);
    void CalculateMinMaxGOES16(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue);
    void CalculateMinMaxHimawari(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max);
    void normalizeMinMaxGOES16(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max, int &fillvalue, int maxvalue);

    QString getKindofImage() { return kindofimage; }
    QString getImagePath() { return imagepath; }
    void setImagePath( QString ip) { imagepath = ip; }
    void setKindofImage( QString ip) { kindofimage = ip; }

    void ComposeSegmentImageXRIT(QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector );
    void ComposeSegmentImageXRITHimawari( QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector );

    void ComposeSegmentImageHDF(QFileInfo fileinfo, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector );
    void ComposeSegmentImageHDFInThread(QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector );
    void ComposeSegmentImagenetCDFInThread(QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod );
    void SetupContrastStretch(quint16 x1, quint16 y1, quint16 x2, quint16 y2); //, quint16 x3, quint16 y3, quint16 x4, quint16 y4);
    quint16 ContrastStretch(quint16 val);
    void InsertPresent( QVector<QString> spectrumvector, QString filespectrum, int filesequence);
    bool allHRVColorSegmentsReceived();
    bool allSegmentsReceived();
    bool bActiveSegmentList;
    bool bisRSS;
    eGeoSatellite getGeoSatellite();
    int getGeoSatelliteIndex() { return geoindex; }
    void setGeoSatellite(eGeoSatellite ws) { m_GeoSatellite = ws; }
    void setGeoSatellite(int geoindex, QString strgeo);
    void CalculateLUTGeo(int colorindex);

    bool issegmentcomposedRed[10];
    bool issegmentcomposedGreen[10];
    bool issegmentcomposedBlue[10];
    bool issegmentcomposedHRV[24];

    bool isPresentRed[10];
    bool isPresentGreen[10];
    bool isPresentBlue[10];
    bool isPresentHRV[24];

    void ResetSegments();
    double COFF;
    double LOFF;
    double CFAC;
    double LFAC;

    int LowerEastColumnActual;
    int LowerSouthLineActual;
    int LowerWestColumnActual;
    int LowerNorthLineActual;
    int UpperEastColumnActual;
    int UpperSouthLineActual;
    int UpperWestColumnActual;
    int UpperNorthLineActual;

    int SouthLineActual;
    int WestColumnActual;

    int areatype;
    double geosatlon;
    QString geosatname;
    QString str_GeoSatellite;
    int histogrammethod;

private:

    void ComposeColorHRV();
    void getFilenameParameters(QFileInfo fileinfo, QString &filespectrum, QString &filedate, int &filesequence);

//    quint16 maxvalueRed[10];
//    quint16 minvalueRed[10];
//    quint16 maxvalueGreen[10];
//    quint16 minvalueGreen[10];
//    quint16 maxvalueBlue[10];
//    quint16 minvalueBlue[10];

    quint16 stat_min[3];
    quint16 stat_max[3];

    long active_pixels[3];

    QString kindofimage;
    QString imagepath;

    double A1, B1, A2, B2, A3, B3;
    double d_x1, d_x2, d_x3, d_x4, d_y1, d_y2, d_y3, d_y4;

    eGeoSatellite m_GeoSatellite;
    int geoindex;
    int number_of_columns;
    int number_of_lines;


signals:

    void progressCounter(int val);
    void signalcomposefinished(QString kindofimage, int channelindex, int filesequence);
    
public slots:

private slots:

protected:


    
};

#endif // SEGMENTLISTMETEOSAT_H
