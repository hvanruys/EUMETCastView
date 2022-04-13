#ifndef SEGMENTIMAGE_H
#define SEGMENTIMAGE_H

#include <QImage>

#include <QFutureWatcher>
#include <QOpenGLFramebufferObject>

#include "sgp4sdp4.h"
#include "options.h"
#include "generalverticalperspective.h"
#include "lambertconformalconic.h"
#include "stereographic.h"
#include "obliquemercator.h"

enum  seviriunits {
     SEVIRI_UNIT_CNT,
     SEVIRI_UNIT_RAD,
     SEVIRI_UNIT_REF,
     SEVIRI_UNIT_BRF,
     SEVIRI_UNIT_BT,
     SEVIRI_UNIT_REFL39,

     N_SEVIRI_UNITS
};

struct Bandstorage {
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
    seviriunits units;
    double slope;
    double offset;
};

struct RGBRecipeColor {
    QStringList channels;
    QList<int> spectral_channel_nbr; // 1 - 12
    QList<bool> subtract;
    QList<bool> inverse;
    QList<bool> reflective;
    seviriunits units;
    float rangefrom;
    float rangeto;
    QString dimension;
    float gamma;

};

struct RGBRecipe {
  QString Name;
  bool needsza;
  QVector<RGBRecipeColor> Colorvector;
};

enum MapReturn
{
    MAPOK = 0,
    MAPOUT,
    MAP
};

class GeneralVerticalPerspective;
class LambertConformalConic;
class StereoGraphic;
class ObliqueMercator;

class SegmentImage
{
public:
    SegmentImage();

    void DeleteImagePtrs();
    void ReverseImage();
    QImage *ReverseImageChannel(QImage *ptr);
    void ExpandImage(int channelshown);
    void RotateImage();
    void ResetPtrImage();
    void InitializeAVHRRImages( int imagewidth, int imageheight ); //, long stat_min_ch[], long stat_max_ch[] );
    void InitializeImageGeostationary( int imagewidth, int imageheight ); //, long stat_min_ch[], long stat_max_ch[] );
    int CLAHE (unsigned short *pImage, unsigned int uiXRes, unsigned int uiYRes,
         unsigned short Min, unsigned short Max, unsigned int uiNrX, unsigned int uiNrY,
              unsigned int uiNrBins, float fCliplimit);
    void SmoothProjectionImage();
    void showHistogram(QImage *ptr);
    qint32 Min(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);
    qint32 Max(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22);
    bool bhm_line(int x1, int y1, int x2, int y2, QRgb rgb1, QRgb rgb2, QRgb *canvas, int dimx);
    void MapInterpolation(QRgb *canvas, quint16 dimx, quint16 dimy);
    void MapCanvas(QRgb *canvas, qint32 anchorX, qint32 anchorY, quint16 dimx, quint16 dimy, bool combine);
    void SetupRGBrecipes();
    int GetSpectralChannelNbr(QString channel);


    QImage *ptrimagecomp_ch[5];
    QImage *ptrimagecomp_col;
    QImage *ptrexpand_col;
    QImage *ptrimageViirsM;
    QImage *ptrimageViirsDNB;
    QImage *ptrimageOLCI;
    QImage *ptrimageSLSTR;
    QImage *ptrimageMERSI;
    eSegmentType olcitype;


    QImage *ptrimageGeostationary;
    QImage *ptrimageProjection;
    QImage *ptrimageProjectionCopy; //for VIIRS M background with VIIRS DNB

    QScopedArrayPointer<float> ptrProjectionBrightnessTemp;
    QScopedArrayPointer<quint8> ptrProjectionInfra; // for Infra
    QScopedArrayPointer<quint16> ptrimageProjectionRed;
    QScopedArrayPointer<quint16> ptrimageProjectionGreen;
    QScopedArrayPointer<quint16> ptrimageProjectionBlue;
    QScopedArrayPointer<quint16> ptrimageProjectionAlpha;

    QScopedArrayPointer<quint8> ptrimageRGBRecipeRed;
    QScopedArrayPointer<quint8> ptrimageRGBRecipeGreen;
    QScopedArrayPointer<quint8> ptrimageRGBRecipeBlue;

    QPixmap *pmOriginal;
    QPixmap *pmOut;
    //QOpenGLFramebufferObject *fbo;
    GLuint fboId;

    GeneralVerticalPerspective *gvp;
    LambertConformalConic *lcc;
    StereoGraphic *sg;
    ObliqueMercator *om;

    quint16 lut_ch[5][1024];
    quint16 lut_norm_ch[5][1024];
    quint16 lut_proj_ch[3][1024];
    quint16 lut_sentinel[256];

    int stat_max_ch[5];
    int stat_min_ch[5];
    int stat_max_proj_ch[5]; // used in HistEqual of projection
    int stat_min_proj_ch[5];
    long stat_max_norm_ch[3];
    long stat_min_norm_ch[3];
    long stat_max;
    long stat_min;

    float stat_max_dnb;
    float stat_min_dnb;
    float minBrightnessTemp;
    float maxBrightnessTemp;
    long active_pixels;
    long active_pixels_proj;

    int minRadianceIndex[5];
    int maxRadianceIndex[5];
    int minRadianceIndexNormalized[3];
    int maxRadianceIndexNormalized[3];

    quint16 *ptrRed[10];
    quint16 *ptrGreen[10];
    quint16 *ptrBlue[10];

    quint16 *ptrHRV[24];
    qint8 *ptrDQF[3];
    void CalcSatAngles();
    double Sigmadist[2048];
    double fraction[2048];
    double SigmadistGAC[409];
    double fractionGAC[409];

    int fillvalue[3];  //used for GOES_16



    QScopedArrayPointer<double> time;		/* image of Julian Day Number */
    QScopedArrayPointer<float> lat;		/* image of latitude */
    QScopedArrayPointer<float> lon;		/* image of longitude */
    QScopedArrayPointer<float> sza;		/* image of solar zenith angle (degrees: 0.0 -- 180.0) */
    QScopedArrayPointer<float> saa;		/* image of solar azimuth angle  (degrees: 0.0 -- 360.0) */
    QScopedArrayPointer<float> vza;		/* image of viewing zenith angle (degrees: 0.0 -- 180.0) */
    QScopedArrayPointer<float> vaa;		/* image of viewing azimuth angle  (degrees: 0.0 -- 360.0) */
    QScopedArrayPointer<float> *data;		/* array of pointers to images of length n_bands */
    QScopedArrayPointer<float> data2;		/* array of image data of length n_bands * n_lines * n_columns */

    QList<RGBRecipe> rgbrecipes;


private:
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
    QImage *RotateImageChannel(QImage *ptr);
    void boundaryFill4 (int x, int y);


};

#endif // SEGMENTIMAGE_H
