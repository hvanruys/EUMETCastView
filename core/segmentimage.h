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

    QImage *ptrimagecomp_ch[5];
    QImage *ptrimagecomp_col;
    QImage *ptrexpand_col;
    QImage *ptrimageViirs;

    QImage *ptrimageGeostationary;
    QImage *ptrimageProjection;

    QPixmap *pmOriginal;
    QPixmap *pmOut;
    //QOpenGLFramebufferObject *fbo;
    GLuint fboId;

    GeneralVerticalPerspective *gvp;
    LambertConformalConic *lcc;
    StereoGraphic *sg;

    unsigned long segment_stats_ch[5][1024];
    quint16 lut_ch[5][1024];
    int stat_max_ch[5];
    int stat_min_ch[5];

    quint16 *ptrRed[10];
    quint16 *ptrGreen[10];
    quint16 *ptrBlue[10];

    quint16 *ptrHRV[24];
    void CalcSatAngles();
    double Sigmadist[2048];
    double fraction[2048];


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
