#ifndef VIDEOMAKER_H
#define VIDEOMAKER_H

#include <QObject>
#include <QUdpSocket>
#include <qimage.h>
#include "jsonvideoreader.h"
#include "gshhsdata.h"

class VideoMaker : public QObject
{
    Q_OBJECT
public:
    explicit VideoMaker(QString jsonfile, QString timestamp, QObject *parent = nullptr);
    void OverlayProjectionGVP();
    void compileImage(QString date, int imagenbr);
    void compileImageMTG(QString date, int imagenbr);

    QStringList getJsonFileList() { return reader->segmentspathlist; }
    //QStringList getGeostationarySegments(QString kindofimage, QList<QString> spectrumvector);
    void checkAvailableSegments(QStringList *segs, QString date);
    void replenishSegmentsRss(QStringList *segs, QString datestr);
    void replenishSegmentsFull(QStringList *segs, QString datestr);
    bool isSegmentAvailable(QString segmentstr, QStringList *segs, QTime time);
    void getFilenameParameters(QString filename, QString &filespectrum, QString &filedate, int &filesequence);
    void getSegmentSamples(QString filepath, quint16 *ptr, int filesequence, QString typelist);
    void ComposeVISIR(quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue, quint16 *ptrNightRed, QImage &imvisir, QString date, int imagenbr);
    void ComposeHRV1(quint16 *ptrHRV, quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue, quint16 *ptrNightRed, QImage &imhrv, QString date,
                     int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla, int imagenbr);
    void CalculateMinMax(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue, quint16 stat_min[], quint16 stat_max[], long active_pixels[]);
    void CalculateLUTGeo(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue, quint16 stat_min[], quint16 stat_max[],
                         long active_pixels[], quint16 lut_ch[3][1024], int minRadianceIndex[], int maxRadianceIndex[]);
    void sendMessages(QString txt);

    JsonVideoReader *reader;
    QUdpSocket *udpSocket;

signals:
private:
    void SetupContrastStretch(quint16 x1, quint16 y1, quint16 x2, quint16 y2);
    quint16 ContrastStretch(quint16 val);

    gshhsData *gshhs;
    QImage overlayimageProjection;
    int CLAHE (unsigned short* pImage, unsigned int uiXRes, unsigned int uiYRes,
            unsigned short Min, unsigned short Max, unsigned int uiNrX, unsigned int uiNrY,
            unsigned int uiNrBins, float fCliplimit);
    void ClipHistogram (unsigned long* pulHistogram, unsigned int uiNrGreylevels, unsigned long ulClipLimit);
    void MakeHistogram (unsigned short* pImage, unsigned int uiXRes, unsigned int uiSizeX, unsigned int uiSizeY,
                        unsigned long* pulHistogram, unsigned int uiNrGreylevels, unsigned short* pLookupTable);
    void MapHistogram (unsigned long* pulHistogram, unsigned short Min, unsigned short Max,
                      unsigned int uiNrGreylevels, unsigned long ulNrOfPixels);
    void MakeLut (unsigned short * pLUT, unsigned short Min, unsigned short Max, unsigned int uiNrBins);
    void Interpolate (unsigned short *pImage, int uiXRes, unsigned long * pulMapLU,
                                 unsigned long * pulMapRU, unsigned long * pulMapLB,  unsigned long * pulMapRB,
                     unsigned int uiXSize, unsigned int uiYSize, unsigned short *pLUT);
    void OverlayGeostationary(QImage *im, bool hrvimage, int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla);
    void OverlayDate(QImage *im, QString date);
    void OverlayGeostationaryHRV(QPainter *paint, int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla);

    void CalculateMinMaxMTG(int colorindex, int index);
    int serialMinMaxMTG(const int &index);

    int serialLUTGeoMTG(const int &index);
    void CalculateLUTGeoMTG(int colorindex, int index);

    void InitializeImageGeostationary( int imagewidth, int imageheight);
    // void CalculateImageMTGConcurrentNight(int index);

    void CalculateImageMTG(int findex);

    void TestCalculateImageMTG(int findex);

    void CalculateImageMTGNight(int findex);

    void getTimeFromIndex(int index, QString *strtime);


    double A1, B1, A2, B2, A3, B3;
    double d_x1, d_x2, d_x3, d_x4, d_y1, d_y2, d_y3, d_y4;

    quint16 mtg_total_number_of_rows[4];
    quint16 mtg_total_number_of_columns[4];

    int mtg_start_position_row[4][40];
    int mtg_end_position_row[4][40];
    int mtg_start_position_column[4][40];
    int mtg_end_position_column[4][40];
    int mtg_total_rows_per_segment[4][40];

    int mtg_nbr_of_rows[4][40];
    int mtg_nbr_of_columns[4][40];

    quint16 mtg_stat_min[4][40];
    quint16 mtg_stat_max[4][40];
    long mtg_active_pixels[4][40];

    quint16 mtg_histogram[4][40][4096];

    quint16 *ptrMTG[4][40];

    int histogrammethod;
    quint16 fillvalue[4];
    quint16 stat_min[5];
    quint16 stat_max[5];

    long active_pixels[5];

    int minRadianceIndex[5];
    int maxRadianceIndex[5];
    quint16 lut_mtg[4][4096];
    QImage *ptrimageGeostationary;
//    QScopedArrayPointer<quint16> ptrimageGeoNight;
    QImage *ptrimageGeoNight;
    bool alphazero;

    double COFF;
    double LOFF;
    double CFAC;
    double LFAC;

    int total_rows[4];

    int tot_rows[4];
    int tot_rest_rows[4];
    QList<int> vec;


};

#endif // VIDEOMAKER_H
