#ifndef SEGMENTLISTMETEOSAT_H
#define SEGMENTLISTMETEOSAT_H

#include <QObject>
#include <QFutureWatcher>
#include <QFileInfo>
#include <QVector2D>
#include "globals.h"
#include "msgfileaccess.h"
#include "msgdataaccess.h"
#include "segmentimage.h"

typedef struct {
    int listindex;
    int spectral_channel_nbr;
    QString directory;
    QString productid1;
    QString productid2;
    QString timing;
    double day_of_year;
    float min;
    float max;
    float *data;
    int units;
    double slope;
    double offset;
} bandstorage;


class SegmentListGeostationary : public QObject
{
    Q_OBJECT

public:

    explicit SegmentListGeostationary(QObject *parent = 0, int geoindex = 0);
    //bool ComposeImageXRIT(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod);
    //bool ComposeImageHDFSerial(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector);
    bool ComposeImageHDFInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector);
    bool ComposeImagenetCDFInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor);
    bool ComposeImagenetCDFMTGInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor);
    bool ComposeImageXRITMSGInThread(QStringList strlistvis_ir, QStringList strlisthvr, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod);

    void displayMinMax();
    void CalculateMinMax(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max);
    void CalculateMinMax(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue);
    void normalizeMinMax(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max, int &fillvalue, int maxvalue);

    QString getKindofImage() { return kindofimage; }
    QString getImagePath() { return imagepath; }
    QVector<QString> getSpectrumVector() { return this->spectrumvector; }
    void setSpectrumVector(QVector<QString> spec) { this->spectrumvector = spec; }
    void setImagePath( QString ip) { imagepath = ip; }
    void setKindofImage( QString ip) { kindofimage = ip; }

    void ComposeSegmentImageHDF(QFileInfo fileinfo, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector );
    void ComposeSegmentImageHDFInThread(QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector );
    void ComposeSegmentImagenetCDFInThread();
    void ComposeSegmentImagenetCDFMTGInThread();
    void ComposeSegmentImagenetCDFMTGInThread1();
    //void ComposeSegmentImagenetCDFMTGInThreadConcurrent();
    void ComposeSegmentImageXRITMSGInThreadConcurrent();
    void SetupContrastStretch(quint16 x1, quint16 y1, quint16 x2, quint16 y2);
    quint16 ContrastStretch(quint16 val);
    bool bActiveSegmentList;
    bool bisRSS;
    eGeoSatellite getGeoSatellite();
    int getGeoSatelliteIndex() { return geoindex; }
    void setGeoSatellite(eGeoSatellite ws) { m_GeoSatellite = ws; }
    void setGeoSatellite(int geoindex);
    void setFileDateString(QString str) { filedatestring = str; }
    void CalculateLUTGeo(int colorindex);
    void CalculateLUTGeo(int colorindex, quint16 *ptr, quint16 fillvalue);
    void CalculateLUTGeoMTG(int colorindex);
    void CalculateLUTGeoMTG256(int colorindex);

    void CalculateMinMaxMTG(int colorindex, int index);
    void CalculateLUTGeoMTGConcurrent(int colorindex, int index);
    void CalculateImageMTGConcurrent(int index);
    void CalculateImageMTGConcurrentAlt(int index);
    void CalculateImageMTGConcurrentNight(int index);


    void ComposeGeoRGBRecipe(int recipe, QString tex);
    void ComposeGeoRGBRecipeInThread(int recipe);
    static void doComposeGeostationaryHDFInThread(SegmentListGeostationary *sm, QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector);
    static void doComposeGeostationarynetCDFInThread(SegmentListGeostationary *sm);
    static void doComposeGeostationarynetCDFMTGInThread(SegmentListGeostationary *sm);
    static void doComposeGeostationaryXRITMSGInThread(SegmentListGeostationary *sm);
    static void doComposeGeoRGBRecipe(SegmentListGeostationary *sm, int recipe);
    void CalculateGeoRadiances(bandstorage &bs);
    void setThreadParametersnetCDF(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor);
    void setThreadParametersXRIT(QStringList strlistvis_ir, QStringList strlisthvr, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod);
    void ComposeDayMicrophysicsRGB(bandstorage &bs, double julian_day);
    void ComposeSnowRGB(bandstorage &bs, double julian_day);
    void ComposeIR_039sunreflected(bandstorage &bs, double julian_day);


    double COFF;
    double LOFF;
    double CFAC;
    double LFAC;

//    double COFFHRV;
//    double LOFFHRV;
//    double CFACHRV;
//    double LFACHRV;

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

private:

    void ComposeHRV();
    void ComposeHRV_Alt();
    void ComposeVISIR();
    void ComposeVISIR_Alt();
    void ComposeVISIRHimawari();
    void computeGeoImageVISIR(quint16 *pixelsRed, quint16 *pixelsGreen, quint16 *pixelsBlue, quint16 *pixelsNight);
    void computeGeoImageHRV(quint16 *pixelsRed, quint16 *pixelsGreen, quint16 *pixelsBlue, quint16 *pixelsNight, quint16 *pixelsHRV);
    void computeGeoImageHimawari(quint16 *pixelsRed, quint16 *pixelsGreen, quint16 *pixelsBlue, quint16 *pixelsNight);

    void getFilenameParameters(QFileInfo fileinfo, QString *filespectrum, QString *filedate, int *filesequence, int *channelindex);
    void getFilenameParameters(QString file, QString *filespectrum, QString *filedate, int *filesequence, int *channelindex);
    void Printbands();
    void PrintResults();
    void PrintResults(float *ptr, QString title);
    void GetRadBT(int unit, int channel, bandstorage &bs, float *container);
    QString getSeviribandfromChannel(int channel);

    void CalcImage();
    static int concurrentMinMaxMTG(SegmentListGeostationary *sm, const int &index);
    static int concurrentLUTGeoMTG(SegmentListGeostationary *sm, const int &index);
    static int concurrentImageMTG(SegmentListGeostationary *sm, const int &index);
    static int concurrentImageMTGNight(SegmentListGeostationary *sm, const int &index);

    //static bool concurrentComposeImageXRIT(SegmentListGeostationary *sm, const QStringList files, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod);
    static void concurrentReadFilelist(SegmentListGeostationary *sm, QString llFile);
    static void concurrentReadFilelistHimawari(SegmentListGeostationary *sm, QString llFile);
    static void concurrentSetRed(SegmentListGeostationary *sm, const int &line, const int &value);

    void equalizeHistogram(quint16* pdata, int width, int height, int max_val);
    void equalizeHistogram(quint16* pdata, int width, int height, int colorindex, quint16 fillvalue, int max_val);


    void CalculateLonLat();
    QImage *CalculateBitMap(bool HRV);

    quint16 stat_min[4];
    quint16 stat_max[4];

    long active_pixels[4];

    QString kindofimage;
    QString imagepath;

    double A1, B1, A2, B2, A3, B3;
    double d_x1, d_x2, d_x3, d_x4, d_y1, d_y2, d_y3, d_y4;

    eGeoSatellite m_GeoSatellite;
    int geoindex;
    int number_of_columns;
    int number_of_lines;

    QStringList segmentfilelist;
    QStringList segmentfilelisthrv;
    QVector<QString> spectrumvector;
    QVector<bool> inversevector;
    int histogrammethod;
    bool pseudocolor;
    QString filedatestring;

    QList<bandstorage> bands;
    float* result[3];
    double *time;
    float *lat;
    float *lon;

    double lon0;

    float *sza;		/* image of solar zenith angle (degrees: 0.0 -- 180.0) */
    float *saa;		/* image of solar azimuth angle  (degrees: 0.0 -- 360.0) */
    float *vza;		/* image of viewing zenith angle (degrees: 0.0 -- 180.0) */
    float *vaa;		/* image of viewing azimuth angle  (degrees: 0.0 -- 360.0) */
    QString tex;
    int satid;

//    QVector<int> nbr_lines_MTG;


signals:

    void progressCounter(int val);
    void signalcomposefinished(QString kindofimage, int index);
    
public slots:
//    void setRedValue(int red);

private slots:

protected:


    
};

#endif // SEGMENTLISTMETEOSAT_H
