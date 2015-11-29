#ifndef FORMIMAGE_H
#define FORMIMAGE_H

#include <QWidget>
#include <QHBoxLayout>

#include "satellite.h"
#include "avhrrsatellite.h"
#include "generalverticalperspective.h"
#include "formtoolbox.h"

class FormToolbox;

class FormImage : public QWidget
{
    Q_OBJECT

public:
    explicit FormImage(QWidget *parent = 0, SatelliteList *satlist=0, AVHRRSatellite *seglist=0);
    QLabel *returnimageLabelptr();
    void ComposeImage();
    bool ShowVIIRSMImage();
    bool ShowVIIRSDNBImage();
    QSize getPictureSize() const;
    void recalculateCLAHE(QVector<QString> spectrumvector, QVector<bool> inversevector);
    void recalculateCLAHEAvhrr(QVector<QString> spectrumvector, QVector<bool> inversevector);
    void CLAHEprojection();
    void OverlayGeostationary(QPainter *paint, SegmentListGeostationary *sl);
    void OverlayProjection(QPainter *paint,  SegmentListGeostationary *sl);
    bool getOverlayMeteosat() { return overlaymeteosat; }
    bool getOverlayProjection() { return overlayprojection; }
    bool toggleOverlayMeteosat();
    bool toggleOverlayProjection();
    void SetFormToolbox(FormToolbox *ptr) { formtoolbox = ptr; }

    void displayImage(int channel);
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

    int metopcount;
    int noaacount;
    int gaccount;
    int hrpcount;
    int viirsmcount;
    int viirsdnbcount;


    int channelshown; // which button channel
    QString txtInfo;
    bool refreshoverlay;


    ~FormImage();

private:

    void displayAVHRRImageInfo();
    void displayVIIRSImageInfo();
    void displayGeoImageInfo();
    void displayGeoImageInformation(QString satname);


    SatelliteList *sats;
    AVHRRSatellite *segs;
    FormToolbox *formtoolbox;

    double metop_gamma_ch[5], noaa_gamma_ch[5], gac_gamma_ch[5], hrp_gamma_ch[5];
    double metop_gammaEqual_ch[5], noaa_gammaEqual_ch[5], gac_gammaEqual_ch[5], hrp_gammaEqual_ch[5];
    bool metop_inverse_ch[5], noaa_inverse_ch[5], gac_inverse_ch[5], hrp_inverse_ch[5];
    bool metop_inverseEqual_ch[5], noaa_inverseEqual_ch[5], gac_inverseEqual_ch[5], hrp_inverseEqual_ch[5];

    QLabel *imageLabel;
    QHBoxLayout *mainLayout;

    QPoint mousepoint;
    double scaleFactor;

    QPixmap *overlay_pix;
    bool overlaymeteosat;
    bool overlayprojection;

    QString kindofimage;
    eSegmentType segmenttype;

    int zoomValueavhrr;
    int zoomValuemeteosat;
    int zoomValueprojection;
    int zoomValueviirs;
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
    void slotShowVIIRSImage();
    void setPixmapToLabel(bool settoolboxbuttons);
    void setPixmapToLabelDNB(bool settoolboxbuttons);
    void slotUpdateMeteosat();
    void slotUpdateHimawari();
    void slotUpdateProjection();
    void slotRefreshOverlay();

protected:
    void paintEvent(QPaintEvent *);
    void wheelEvent(QWheelEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

};

#endif // FORMIMAGE_H
