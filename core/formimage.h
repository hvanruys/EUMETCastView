#ifndef FORMIMAGE_H
#define FORMIMAGE_H

#include <QWidget>
#include <QHBoxLayout>

#include "satellite.h"
#include "avhrrsatellite.h"
#include "generalverticalperspective.h"
#include "formtoolbox.h"
#include "forminfrascales.h"

class FormToolbox;
class FormInfraScales;
class ImageLabel;
class AspectRatioPixmapLabel;

class FormImage : public QWidget
{
    Q_OBJECT

public:
    explicit FormImage(QWidget *parent = 0, SatelliteList *satlist=0, AVHRRSatellite *seglist=0);
    QLabel *returnimageLabelptr();
    void ComposeImage();
    bool ShowVIIRSMImage();
    bool ShowVIIRSDNBImage();
    bool ShowOLCIefrImage(int histogrammethod, bool normalized);
    bool ShowOLCIerrImage(int histogrammethod, bool normalized);
    void setHistogramMethod(int histogrammethod, bool normalized);
    QSize getPictureSize() const;
    void recalculateCLAHE(QVector<QString> spectrumvector, QVector<bool> inversevector);
    void recalculateCLAHEAvhrr(QVector<QString> spectrumvector, QVector<bool> inversevector);
    void recalculateCLAHEOLCI(QVector<QString> spectrumvector, QVector<bool> inversevector);
    void CLAHEprojection();
    void OverlayGeostationary(QPainter *paint, SegmentListGeostationary *sl);
    void OverlayProjection(QPainter *paint,  SegmentListGeostationary *sl);
    void OverlayOLCI(QPainter *paint);
    void ToInfraColorProjection();
    void FromInfraColorProjection();

    bool getOverlayMeteosat() { return overlaymeteosat; }
    bool getOverlayProjection() { return overlayprojection; }
    bool getOverlayOLCI() { return overlayolci; }
    bool toggleOverlayMeteosat();
    bool toggleOverlayProjection();
    bool toggleOverlayOLCI();
    bool toggleOverlayGridOnOLCI();
    void SetFormToolbox(FormToolbox *ptr) { formtoolbox = ptr; }
    void SetDockWidgetInfraScales(FormInfraScales *ptr) { dockinfrascales = ptr; }
    void showInfraScales() { changeinfraprojection = true; }

    void displayImage(eImageType channel);
    void test();
    void setKindOfImage(QString koi) { kindofimage = koi; }
    QString getKindOfImage() { return kindofimage; }

    void setSegmentType(eSegmentType st) { segmenttype = st; }
    eSegmentType getSegmentType() { return segmenttype; }

    void setZoomValue(int z);
    int getZoomValue();
    void makeZoom(double f);
    inline void zoomIn(){makeZoom(getZoomValue() + zoomIncrement);}
    inline void zoomOut(){makeZoom(getZoomValue() - zoomIncrement);}
    inline void normalSize(){makeZoom(100);}

    void adjustImage();
    void adjustPicSize(bool setwidth);
    bool ShowHistogramImage(int histogrammethod, bool normalized);

    void UpdateProjection();
    bool SaveAsPNG48bits(bool mapto65535);

    int metopcount;
    int noaacount;
    int gaccount;
    int hrpcount;

    int metopAhrptcount;
    int metopBhrptcount;
    int noaa19hrptcount;
    int M01hrptcount;
    int M02hrptcount;

    int viirsmcount;
    int viirsdnbcount;
    int olciefrcount;
    int olcierrcount;


    eImageType channelshown; // which button channel
    QString txtInfo;
    bool refreshoverlay;
    ImageLabel *imageLabel;


    ~FormImage();

private:

    void displayAVHRRImageInfo();
    void displayVIIRSImageInfo();
    void displayOLCIImageInfo();
    void displayGeoImageInfo();
    void displayGeoImageInformation(QString satname);

    SatelliteList *sats;
    AVHRRSatellite *segs;
    FormToolbox *formtoolbox;
    FormInfraScales *dockinfrascales;

    //double metop_gamma_ch[5], noaa_gamma_ch[5], gac_gamma_ch[5], hrp_gamma_ch[5];
    //double metop_gammaEqual_ch[5], noaa_gammaEqual_ch[5], gac_gammaEqual_ch[5], hrp_gammaEqual_ch[5];
    bool metop_inverse_ch[5], noaa_inverse_ch[5], gac_inverse_ch[5], hrp_inverse_ch[5];
    //bool metop_inverseEqual_ch[5], noaa_inverseEqual_ch[5], gac_inverseEqual_ch[5], hrp_inverseEqual_ch[5];

    //AspectRatioPixmapLabel *imageLabel;

    QVBoxLayout *mainLayout;

    QPoint mousepoint;
    double scaleFactor;

    bool overlaymeteosat;
    bool overlayprojection;
    bool overlayolci;
    bool changeinfraprojection;

    QString kindofimage;
    eSegmentType segmenttype;

    int zoomValueavhrr;
    int zoomValuemeteosat;
    int zoomValueprojection;
    int zoomValueviirs;
    int zoomValueolci;
    int zoomIncrement;
    int maxZoomValue;
    int minZoomValue;

    void fillptrimage(quint16 *pix);
    void fillptrimageHRV(quint16 *pixHRV);
    void formwheelZoom(int d);

signals:
    void moveImage(QPoint, QPoint);
    void picSizeChanged();
    void pixmapChanged();
    void wheelZoom(int);
    void render3dgeo(SegmentListGeostationary::eGeoSatellite);
    void allsegmentsreceivedbuttons(bool);

public slots:
    void slotMakeImage();
    void slotShowVIIRSMImage();
    void slotShowOLCIefrImage(int histogrammethod, bool normalized);
    void slotShowOLCIerrImage(int histogrammethod, bool normalized);
    void slotShowHistogramImage(int histogrammethod, bool normalized);

    void setPixmapToLabel(bool settoolboxbuttons);
    void setPixmapToLabelDNB(bool settoolboxbuttons);
    void slotUpdateMeteosat();
    // void slotUpdateHimawari();
    void slotRefreshOverlay();
    void slotRepaintProjectionImage();

protected:
    void paintEvent(QPaintEvent *);
    void wheelEvent(QWheelEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

};

class ImageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ImageLabel(QWidget *parent=0, AVHRRSatellite *seglist=0);
    void setFormImagePtr(FormImage *ptr) { formimage = ptr; }
    void resize(const QSize&);

public slots:
    void setPixmap ( const QPixmap & );
protected:
    void mouseMoveEvent(QMouseEvent *);
private:
    QSize originalpixmapsize;
    QSize scaledpixmapsize;
    FormImage *formimage;
    AVHRRSatellite *segs;

signals:
    void coordinateChanged(QString str);

};

//class AspectRatioPixmapLabel : public QLabel
//{
//    Q_OBJECT
//public:
//    explicit AspectRatioPixmapLabel(QWidget *parent = 0);
//    virtual int heightForWidth( int width ) const;
//    virtual QSize sizeHint() const;
//    QPixmap scaledPixmap() const;
//public slots:
//    void setPixmap ( const QPixmap & );
//    void resizeEvent(QResizeEvent *);
//private:
//    QPixmap pix;
//};

//class AspectRatioPixmapLabel : public QLabel
//{
//    Q_OBJECT
//public:
//    explicit AspectRatioPixmapLabel(const QPixmap &pixmap, QWidget *parent = 0);
//    virtual int heightForWidth(int width) const;
//    virtual bool hasHeightForWidth() { return true; }
//    virtual QSize sizeHint() const { return pixmap()->size(); }
//    virtual QSize minimumSizeHint() const { return QSize(0, 0); }


//};


#endif // FORMIMAGE_H
