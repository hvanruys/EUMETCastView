#ifndef SEGMENTLISTMETEOSAT_H
#define SEGMENTLISTMETEOSAT_H

#include <QObject>
#include <QFutureWatcher>
#include <QFileInfo>

class SegmentListGeostationary : public QObject
{
    Q_OBJECT

public:

    enum eGeoSatellite {
        MET_10 = 0,
        MET_9,
        MET_7,
        ELECTRO_N1,
        GOES_13,
        GOES_15,
        MTSAT
    };

    explicit SegmentListGeostationary(QObject *parent = 0);
    bool ComposeImage(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector);
    void ComposeWithGamma();

    void displayMinMax();
    void CalculateMinMax();
    int returnNbrOfSegments();
    QString getKindofImage() { return kindofimage; }
    QString getImagePath() { return imagepath; }
    void setImagePath( QString ip) { imagepath = ip; }
    void setKindofImage( QString ip) { kindofimage = ip; }

    void ComposeSegmentImage(QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector );
    void SetupContrastStretch(quint16 x1, quint16 y1, quint16 x2, quint16 y2, quint16 x3, quint16 y3, quint16 x4, quint16 y4);
    quint8 ContrastStretch(quint16 val);
    void InsertPresent( QVector<QString> spectrumvector, QString filespectrum, int filesequence);
    bool allHRVColorSegmentsReceived();
    bool allSegmentsReceived();
    bool bActiveSegmentList;
    bool bisRSS;
    eGeoSatellite getGeoSatellite() { return m_GeoSatellite; }
    void setGeoSatellite(eGeoSatellite ws) { m_GeoSatellite = ws; }


    QFutureWatcher<void> watcherRed[8];
    QFutureWatcher<void> watcherGreen[8];
    QFutureWatcher<void> watcherBlue[8];
    QFutureWatcher<void> watcherHRV[24];
    QFutureWatcher<void> watcherMono[10];

    bool issegmentcomposedRed[8];
    bool issegmentcomposedGreen[8];
    bool issegmentcomposedBlue[8];
    bool issegmentcomposedHRV[24];
    bool issegmentcomposedMono[10];

    bool isPresentRed[8];
    bool isPresentGreen[8];
    bool isPresentBlue[8];
    bool isPresentHRV[24];
    bool isPresentMono[10];


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


private:

    void ComposeColorHRV();

    quint16 maxvalueRed[8];
    quint16 minvalueRed[8];
    quint16 maxvalueGreen[8];
    quint16 minvalueGreen[8];
    quint16 maxvalueBlue[8];
    quint16 minvalueBlue[8];

    QString kindofimage;
    QString imagepath;

    double A1, B1, A2, B2, A3, B3;
    double d_x1, d_x2, d_x3, d_x4, d_y1, d_y2, d_y3, d_y4;

    eGeoSatellite m_GeoSatellite;
    int number_of_columns;
    int number_of_lines;


signals:

    void progressCounter(int val);
    
public slots:

private slots:

protected:


    
};

#endif // SEGMENTLISTMETEOSAT_H
