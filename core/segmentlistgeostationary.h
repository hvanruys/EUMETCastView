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
        MET_10 =0,
        MET_9,
        MET_8,
        MET_7,
        FY2E,
        FY2G,
        GOES_13,
        GOES_15,
        GOES_16,
        H8,
        NOGEO
    };

    enum eGeoTreeWidget {
        TREEWIDGET_MET_10 =0,
        TREEWIDGET_MET_9,
        TREEWIDGET_MET_8,
        TREEWIDGET_MET_7,
        TREEWIDGET_FY2E,
        TREEWIDGET_FY2G,
        TREEWIDGET_GOES_13DC3,
        TREEWIDGET_GOES_13DC4,
        TREEWIDGET_GOES_15DC3,
        TREEWIDGET_GOES_15DC4,
        TREEWIDGET_GOES_16,
        TREEWIDGET_H8
    };

    explicit SegmentListGeostationary(QObject *parent = 0);
    bool ComposeImageXRIT(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector);
    //bool ComposeImageHDFSerial(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector);
    bool ComposeImageHDFInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector);
    bool ComposeImagenetCDFInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector);

    void displayMinMax();
    void CalculateMinMax(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max);
    void CalculateMinMaxHimawari(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max);
    QString getKindofImage() { return kindofimage; }
    QString getImagePath() { return imagepath; }
    void setImagePath( QString ip) { imagepath = ip; }
    void setKindofImage( QString ip) { kindofimage = ip; }

    void ComposeSegmentImageXRIT(QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector );
    void ComposeSegmentImageXRITHimawari( QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector );

    void ComposeSegmentImageHDF(QFileInfo fileinfo, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector );
    void ComposeSegmentImageHDFInThread(QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector );
    void ComposeSegmentImagenetCDFInThread(QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector );
    void SetupContrastStretch(quint16 x1, quint16 y1, quint16 x2, quint16 y2); //, quint16 x3, quint16 y3, quint16 x4, quint16 y4);
    quint16 ContrastStretch(quint16 val);
    void InsertPresent( QVector<QString> spectrumvector, QString filespectrum, int filesequence);
    bool allHRVColorSegmentsReceived();
    bool allSegmentsReceived();
    bool bActiveSegmentList;
    bool bisRSS;
    eGeoSatellite getGeoSatellite() { return m_GeoSatellite; }
    void setGeoSatellite(eGeoSatellite ws) { m_GeoSatellite = ws; }
    void recalcHimawari();

    QFutureWatcher<void> watcherRed[10];
    QFutureWatcher<void> watcherGreen[10];
    QFutureWatcher<void> watcherBlue[10];
    QFutureWatcher<void> watcherHRV[24];
    QFutureWatcher<void> watcherMono[10];

    bool issegmentcomposedRed[10];
    bool issegmentcomposedGreen[10];
    bool issegmentcomposedBlue[10];
    bool issegmentcomposedHRV[24];
    bool issegmentcomposedMono[10];

    bool isPresentRed[10];
    bool isPresentGreen[10];
    bool isPresentBlue[10];
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

    quint16 maxvalueRed[10];
    quint16 minvalueRed[10];
    quint16 maxvalueGreen[10];
    quint16 minvalueGreen[10];
    quint16 maxvalueBlue[10];
    quint16 minvalueBlue[10];

    QString kindofimage;
    QString imagepath;

    double A1, B1, A2, B2, A3, B3;
    double d_x1, d_x2, d_x3, d_x4, d_y1, d_y2, d_y3, d_y4;

    eGeoSatellite m_GeoSatellite;
    int number_of_columns;
    int number_of_lines;


signals:

    void progressCounter(int val);
    void imagefinished();
    
public slots:

private slots:

protected:


    
};

#endif // SEGMENTLISTMETEOSAT_H
