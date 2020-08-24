#ifndef SEGMENTMERSI_H
#define SEGMENTMERSI_H
#include <QApplication>
#include "segment.h"
#include <QFile>
#include <QImage>

class SegmentMERSI : public Segment
{
    Q_OBJECT

public:

    explicit SegmentMERSI(QFileInfo fileinfo, SatelliteList *satl = 0, QObject *parent = 0);
    int getEarthViewsPerScanline() { return this->earth_views_per_scanline; }
    void ComposeSegmentImage(int colorarrayindex[], bool invertarrayindex[], int histogrammethod, bool normalized, int totallines);
    Segment *ReadSegmentInMemory(bool composecolor, int colorarrayindex[]);
    void RenderSegmentlineInTextureMERSI(int nbrLine, QRgb *row );
    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeProjection(eProjections proj, int histogrammethod, bool normalized);

    void initializeMemory();

    ~SegmentMERSI();


private:
    QString strgeofile;

    void ReadMERSI_1KM(hid_t h5_file_id);
    void ReadGeoFile(hid_t h5_geofile_id);
    void MapPixel(int lines, int views, double map_x, double map_y, bool iscolor);

    int colorarrayindex[3];
    bool invertarrayindex[3];



signals:


public slots:

};

#endif // SEGMENTMERSI_H
