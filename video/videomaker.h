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


    // All of these start at zero. A VideoMaker is a megabyte and a half of
    // object standing on the stack of main(), and the MTG path reads a number
    // of them before anything writes them : the chunks that are not in the
    // selection keep a start position of 0, total_rows is summed with +=, and
    // InitializeImageGeostationary deletes ptrimageGeostationary before it
    // assigns it. Linux happened to hand out a stack of zeroes, so they read as
    // the values meant; Windows did not, and the image then took its height,
    // its rows and a pointer to delete from whatever stood there.
    double A1 = 0.0, B1 = 0.0, A2 = 0.0, B2 = 0.0, A3 = 0.0, B3 = 0.0;
    double d_x1 = 0.0, d_x2 = 0.0, d_x3 = 0.0, d_x4 = 0.0, d_y1 = 0.0, d_y2 = 0.0, d_y3 = 0.0, d_y4 = 0.0;

    quint16 mtg_total_number_of_rows[4] = {};
    quint16 mtg_total_number_of_columns[4] = {};

    int mtg_start_position_row[4][40] = {};
    int mtg_end_position_row[4][40] = {};
    int mtg_start_position_column[4][40] = {};
    int mtg_end_position_column[4][40] = {};
    int mtg_total_rows_per_segment[4][40] = {};

    int mtg_nbr_of_rows[4][40] = {};
    int mtg_nbr_of_columns[4][40] = {};

    quint16 mtg_stat_min[4][40] = {};
    quint16 mtg_stat_max[4][40] = {};
    long mtg_active_pixels[4][40] = {};

    quint16 mtg_histogram[4][40][4096] = {};

    quint16 *ptrMTG[4][40] = {};

    int histogrammethod = 0;   // CMB_HISTO_NONE_95, the stretch the video is made with
    quint16 fillvalue[4] = {};
    quint16 stat_min[5] = {};
    quint16 stat_max[5] = {};

    long active_pixels[5] = {};

    int minRadianceIndex[5] = {};
    int maxRadianceIndex[5] = {};
    quint16 lut_mtg[4][4096] = {};
    QImage *ptrimageGeostationary = nullptr;
//    QScopedArrayPointer<quint16> ptrimageGeoNight;
    QImage *ptrimageGeoNight = nullptr;
    bool alphazero = false;

    double COFF = 0.0;
    double LOFF = 0.0;
    double CFAC = 0.0;
    double LFAC = 0.0;

    int total_rows[4] = {};

    int tot_rows[4] = {};
    int tot_rest_rows[4] = {};
    QList<int> vec;


};

#endif // VIDEOMAKER_H
