#ifndef FORMIMAGE_H
#define FORMIMAGE_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPrinter>
#include <QtConcurrent/QtConcurrent>

#include "avhrrsatellite.h"
#include "formtoolbox.h"

class FormToolbox;

class FormImage : public QGraphicsView
{
    Q_OBJECT


public:
    explicit FormImage(QWidget *parent = 0, AVHRRSatellite *seglist=0);

    void MakeImage();

    void resetView();
    void fitWindow();
    void originalSize();
    void rotateView(const int nVal);
    void printView();
    bool saveViewToDisk(QString &strError);
    inline bool isModified() { return m_rotateAngle!=0; }
    inline int getRotateAngle(){ return m_rotateAngle; }
    QString getImageFormat(QString strFileName);
    QPixmap *getPixmap() { return &m_pixmap; }
    QGraphicsScene *getScene() { return m_scene; }
    QGraphicsPixmapItem *getPixmapItem() { return m_pixmapItem; }
    void setFormImagePtr(FormImage *ptr) { formimage = ptr; }
    //QSize getPixmapSize() { return scaledpixmapsize; }
    void SetFormToolbox(FormToolbox *ptr) { formtoolbox = ptr; }
    void setKindOfImage(QString koi) { kindofimage = koi; }
    void setChannelShown(eImageType channel) { channelshown = channel; }
    QString getKindOfImage() { return kindofimage; }
    void displayImage(eImageType channel, bool resize);
    void setupGeoOverlay(int geoindex);
    void setupGshhs(int geoindex, int k);
    void setWindowTitle();
    void setSegmentType(eSegmentType st) { segmenttype = st; }
    eSegmentType getSegmentType() { return segmenttype; }
    void drawOverlays(QPainter *painter);
    void savePNGImage(QString fileName);
    void zoomIn();
    void zoomOut();

    bool toggleOverlayMeteosat();
    bool toggleOverlayProjection();
    bool toggleOverlayOLCI();
    bool toggleOverlayGridOnOLCI();

    bool ShowVIIRSMImage();
    bool ShowVIIRSDNBImage();
    bool ShowOLCIefrImage(int histogrammethod, bool normalized);
    bool ShowOLCIerrImage(int histogrammethod, bool normalized);
    bool ShowSLSTRImage(int histogrammethod);
    bool ShowMERSIImage(int histogrammethod, bool normalized);

    void CLAHERGBRecipe(float cliplimit);

    void recalculateCLAHE(QVector<QString> spectrumvector, QVector<bool> inversevector);

    void setViewInitialized(bool init) { qDebug() << "setViewInitialized = " << init; this->m_ViewInitialized = init; }

    static void concurrentSetRed(SegmentListGeostationary *sm, const int &line, const int &value);

    eImageType channelshown; // which button channel

    QString txtInfo;

private:

    void displayGeoImageInfo();
    void displayGeoImageInformation(QString satname);

    void OverlayGeostationary(QPainter *paint, SegmentListGeostationary *sl);
    void OverlayGeostationaryHRV(QPainter *paint, SegmentListGeostationary *sl, int geoindex);
    void OverlayGeostationaryHRV1(QPainter *paint, SegmentListGeostationary *sl, int geoindex);
    void OverlayGeostationaryH8(QPainter *paint, SegmentListGeostationary *sl);
    void OverlayProjection(QPainter *paint);
    void OverlayOLCI(QPainter *paint);
    void DrawLongLat(QPainter *paint, SegmentListGeostationary *sl, int coff, int loff,
                     double cfac, double lfac, bool hrvimage);


    void displayAVHRRImageInfo();
    void displayVIIRSImageInfo(eSegmentType type);
    void displaySentinelImageInfo(eSegmentType type);
    void displayMERSIImageInfo(eSegmentType type);

    mutable QImage *m_image;
    //QImage *m_image;
    QPixmap m_pixmap;
    QGraphicsPixmapItem *m_pixmapItem;
    QGraphicsScene *m_scene;
    int m_rotateAngle;
    bool m_ViewInitialized;
    QString m_fileName;
    FormImage *formimage;
    FormToolbox *formtoolbox;

    QString kindofimage;
    eSegmentType segmenttype;
    AVHRRSatellite *segs;

    bool overlaymeteosat;
    bool overlayprojection;
    bool overlayolci;


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
    int viirsmcountnoaa20;
    int viirsdnbcountnoaa20;
    int olciefrcount;
    int olcierrcount;
    int slstrcount;
    int mersicount;

    QVector<QVector2D> geooverlay[3];



#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

public slots:
    void slotMakeImage();
    void slotcomposefinished(QString kindofimage, int index);
    void setPixmapToScene(bool settoolboxbuttons);
    void slotUpdateGeosat();
    void slotSetRedValue(int);



protected:
    virtual void wheelEvent(QWheelEvent * event);
    virtual void resizeEvent(QResizeEvent * event);
    virtual void drawForeground(QPainter *painter, const QRectF &rect);

signals:
    void render3dgeo(int geoindex);
    void allsegmentsreceivedbuttons(bool);
    void setmapcylbuttons(bool stat);
    void signalMainWindowTitleChanged(QString);

};


#endif // FORMIMAGE_H
