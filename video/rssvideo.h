#ifndef XMLEXAMPLE_H
#define XMLEXAMPLE_H
#include <QPainter>
#include <QUdpSocket>

#include "MSG_data.h"
#include "msgfileaccess.h"
#include "msgdataaccess.h"
#include "gshhsdata.h"
#include "xmlvideoreader.h"
#include "generalverticalperspective.h"

#ifdef _WIN32
#include <hdf5.h>
#else
#include <hdf5.h>
#endif
#include <netcdf.h>

class GeneralVerticalPerspective;

class RSSVideo : public QObject
{
        Q_OBJECT

public:
    explicit RSSVideo(QString xmlfile = 0, QString argsingleimage = 0, QObject *parent = 0 );
    ~RSSVideo();
    void compileImage(QString date, QString path, int imagenbr);
    void compileImagesInBetween(QStringList datelist, QStringList pathlist);
    void compileImageMTG(QString date, QString path, int imagenbr);

    static void doCompileImage(RSSVideo *rv, QString date, QString path, int i);
    QVector<QString> getDateVectorFromDir();
    void getDatePathVectorFromDir(QStringList *datelist, QStringList *pathlist);
    void OverlayGeostationary(QImage *im, bool hrvimage, int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla);
    void OverlayGeostationaryHRV(QPainter *paint, int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla);
    void OverlayProjectionGVP();
    void OverlayDate(QImage *im, QString date);
    void sendMessages(QString txt);

    XMLVideoReader *reader;
    QUdpSocket *udpSocket;


private:

    QStringList getGeostationarySegments(const QString imagetype, QString path, QVector<QString> spectrumvector, QString filepattern);
    void getFilenameParameters(QString filename, QString &filespectrum, QString &filedate, int &filesequence);
    //void ComposeSegmentImageXRIT(QString filepath, quint16 *ptrRed, quint16 *ptrGreen, quint16 *ptrBlue, quint16 *ptrHRV);
    void ComposeVISIR(quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue, quint16 *ptrNightRed, quint16 *ptrNightGreen, quint16 *ptrNightBlue, QImage &imvisir, QString date, int imagenbr);
    void ComposeHRV(quint16 *ptrHRV, quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue, quint16 *ptrNightRed, quint16 *ptrNightGreen, quint16 *ptrNightBlue, QImage &imhrv, QString date,
                    int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla, int imagenbr);
    void ComposeHRV1(quint16 *ptrHRV, quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue, quint16 *ptrNightRed, quint16 *ptrNightGreen, quint16 *ptrNightBlue, QImage &imhrv, QString date,
                    int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla, int imagenbr);
    void ComposeHRVFull(quint16 *ptrHRV, quint16 *ptrDayRed, quint16 *ptrDayGreen, quint16 *ptrDayBlue, quint16 *ptrNightRed, quint16 *ptrNightGreen, quint16 *ptrNightBlue, QImage &imhrv, QString date,
                        int leca, int lsla, int lwca, int lnla, int ueca, int usla, int uwca, int unla);
    void CalculateMinMax(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue, quint16 stat_min[], quint16 stat_max[], long active_pixels[]);
    void CalculateLUTGeo(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue, quint16 stat_min[], quint16 stat_max[],
                         long active_pixels[], quint16 lut_ch[3][1024], int minRadianceIndex[], int maxRadianceIndex[]);
    void getSegmentSamples(QString filepath, quint16 *ptr, int filesequence, QString type);

    int CLAHE (unsigned short *pImage, unsigned int uiXRes, unsigned int uiYRes,
         unsigned short Min, unsigned short Max, unsigned int uiNrX, unsigned int uiNrY,
              unsigned int uiNrBins, float fCliplimit);

    void ClipHistogram (unsigned long* pulHistogram, unsigned int
                 uiNrGreylevels, unsigned long ulClipLimit);
    void MakeHistogram (unsigned short* pImage, unsigned int uiXRes,
            unsigned int uiSizeX, unsigned int uiSizeY,
            unsigned long* pulHistogram,
            unsigned int uiNrGreylevels, unsigned short* pLookupTable);
    void MapHistogram (unsigned long* pulHistogram, unsigned short Min, unsigned short Max,
               unsigned int uiNrGreylevels, unsigned long ulNrOfPixels);
    void MakeLut (unsigned short * pLUT, unsigned short Min, unsigned short Max, unsigned int uiNrBins);
    void Interpolate (unsigned short * pImage, int uiXRes, unsigned long * pulMapLU,
         unsigned long * pulMapRU, unsigned long * pulMapLB,  unsigned long * pulMapRB,
         unsigned int uiXSize, unsigned int uiYSize, unsigned short * pLUT);

    quint16 ContrastStretch(quint16 val);
    void SetupContrastStretch(quint16 x1, quint16 y1, quint16 x2, quint16 y2);
    void checkAvailableSegments(QStringList *segs, QString date);
    void replenishSegmentsRss(QStringList *segs, QString date);
    void replenishSegmentsFull(QStringList *segs, QString date);
    bool isSegmentAvailable(QString segmentstr, QStringList *segs, QTime time);

    double A1, B1, A2, B2, A3, B3;
    double d_x1, d_x2, d_x3, d_x4, d_y1, d_y2, d_y3, d_y4;

    gshhsData *gshhs;
    QImage overlayimageProjection;





};

#endif // XMLEXAMPLE_H
