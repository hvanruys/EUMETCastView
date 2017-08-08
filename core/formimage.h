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
    bool ShowSLSTRImage(int histogrammethod);
    void setHistogramMethod(int histogrammethod, bool normalized);
    void setHistogramMethodSLSTR(int histogrammethod);
    QSize getPictureSize() const;
    void recalculateCLAHE(QVector<QString> spectrumvector, QVector<bool> inversevector);
    void recalculateCLAHEAvhrr(QVector<QString> spectrumvector, QVector<bool> inversevector);
    void recalculateCLAHEOLCI(QVector<QString> spectrumvector, QVector<bool> inversevector);
    void CLAHEprojection();
    void OverlayGeostationary(QPainter *paint, SegmentListGeostationary *sl);
    void OverlayGeostationaryHRV(QPainter *paint, SegmentListGeostationary *sl, int geoindex);
    void OverlayProjection(QPainter *paint);
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
    bool ShowHistogramImageOLCI(int histogrammethod, bool normalized);
    bool ShowHistogramImageSLSTR(int histogrammethod);

    void UpdateProjection();
    bool SaveAsPNG48bits(bool mapto65535);
    void setupGeoOverlay(int geoindex);

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
    int slstrcount;

    QVector<QVector2D> geooverlay;


    eImageType channelshown; // which button channel
    QString txtInfo;
    bool refreshoverlay;
    ImageLabel *imageLabel;


    ~FormImage();

private:

    void displayAVHRRImageInfo();
    void displayVIIRSImageInfo(eSegmentType type);
    void displaySentinelImageInfo(eSegmentType type);
    void displayGeoImageInfo();
    void displayGeoImageInformation(QString satname);
    void EnhanceDarkSpace(int geoindex);
    void calchimawari(QRgb rgb, int &minred, int &maxred, int &mingreen, int &maxgreen, int &minblue, int &maxblue);
    QRgb ContrastStretch(QRgb val);
    void SetupContrastStretch(quint16 x1r, quint16 y1r, quint16 x2r, quint16 y2r, quint16 x1g, quint16 y1g, quint16 x2g, quint16 y2g, quint16 x1b, quint16 y1b, quint16 x2b, quint16 y2b);

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
    int zoomValueslstr;
    int zoomIncrement;
    int maxZoomValue;
    int minZoomValue;

    void fillptrimage(quint16 *pix);
    void fillptrimageHRV(quint16 *pixHRV);
    void formwheelZoom(int d);

    int currentgeooverlay;
    double A1red, B1red;
    double A1green, B1green;
    double A1blue, B1blue;

signals:
    void moveImage(QPoint, QPoint);
    void picSizeChanged();
    void pixmapChanged();
    void wheelZoom(int);
    void render3dgeo(eGeoSatellite geo);
    void allsegmentsreceivedbuttons(bool);

public slots:
    void slotMakeImage();

    void setPixmapToLabel(bool settoolboxbuttons);
    void setPixmapToLabelDNB(bool settoolboxbuttons);
    void slotUpdateGeosat();
    void slotcomposefinished(QString kindofimage, int channelindex, int filesequence);
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

#endif // FORMIMAGE_H
