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

enum MapReturn
{
    MAPOK = 0,
    MAPOUT,
    MAP
};

class GeneralVerticalPerspective;
class LambertConformalConic;
class StereoGraphic;

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

    QImage *ptrimagecomp_ch[5];
    QImage *ptrimagecomp_col;
    QImage *ptrexpand_col;
    QImage *ptrimageViirsM;
    QImage *ptrimageViirsDNB;
    QImage *ptrimageOLCI;

    QImage *ptrimageGeostationary;
    QImage *ptrimageProjection;
    QImage *ptrimageProjectionCopy; //for VIIRS M background with VIIRS DNB

    QScopedArrayPointer<float> ptrProjectionBrightnessTemp;
    QScopedArrayPointer<quint8> ptrProjectionInfra; // for Infra

    QPixmap *pmOriginal;
    QPixmap *pmOut;
    //QOpenGLFramebufferObject *fbo;
    GLuint fboId;

    GeneralVerticalPerspective *gvp;
    LambertConformalConic *lcc;
    StereoGraphic *sg;

    quint16 lut_ch[5][1024];
    int stat_max_ch[5];
    int stat_min_ch[5];
    float stat_max_dnb;
    float stat_min_dnb;
    float minBrightnessTemp;
    float maxBrightnessTemp;
    long active_pixels;

    quint16 *ptrRed[10];
    quint16 *ptrGreen[10];
    quint16 *ptrBlue[10];

    quint16 *ptrHRV[24];
    void CalcSatAngles();
    double Sigmadist[2048];
    double fraction[2048];
    double SigmadistGAC[409];
    double fractionGAC[409];


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
