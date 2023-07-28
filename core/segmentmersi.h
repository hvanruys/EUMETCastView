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

    explicit SegmentMERSI(QFileInfo fileinfo, QObject *parent = 0);
    void ComposeSegmentImage(int bandindex, int colorarrayindex[], bool invertarrayindex[], int histogrammethod, bool normalized, int totallines);
    Segment *ReadSegmentInMemory(int bandindex, int colorarrayindex[]);
    void RenderSegmentlineInTextureMERSI(int nbrLine, QRgb *row );
    void ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSegmentOMProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeProjection(eProjections proj, int histogrammethod, bool normalized);
    void GetCentralCoords(double *startlon, double *startlat, double *endlon, double *endlat, int *startindex, int *endindex);
    void GetStartCornerCoords(double *cornerlon1, double *cornerlat1, double *cornerlon2, double *cornerlat2,
                              int *Xstartindex1, int *Xstartindex2, int *Ystartindex12);
    void GetEndCornerCoords(double *cornerlon3, double *cornerlat3, double *cornerlon4, double *cornerlat4,
                                            int *Xstartindex3, int *Xstartindex4, int *Ystartindex34);

    void initializeMemory();

    ~SegmentMERSI();


private:
    QString strgeofile;

    void ReadMERSI_1KM(hid_t h5_file_id);
    void ReadGeoFile(hid_t h5_geofile_id);
    void MapPixel(int lines, int views, double map_x, double map_y, bool iscolor);

    int colorarrayindex[3];
    bool invertarrayindex[3];
    int bandindex;



signals:


public slots:

};

#endif // SEGMENTMERSI_H
