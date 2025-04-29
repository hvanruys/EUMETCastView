#ifndef EQUIRECTANGULAR_H
#define EQUIRECTANGULAR_H

#include "segmentimage.h"
#include "gshhsdata.h"
#include "avhrrsatellite.h"

class Equirectangular : public QObject
{
    Q_OBJECT

public:
    explicit Equirectangular(QObject *parent = 0, AVHRRSatellite *seglist = 0);
    ~Equirectangular();
    void Initialize(int imwidth, int imheight);
    void map_forward(float lon_deg, float lat_deg, int &map_x, int &map_y);
    void map_inverse(int map_x, int map_y, float &lon_deg, float &lat_deg);
    void map_inverse_rad(int map_x, int map_y, float &lon_rad, float &lat_rad);

    void initializeProjectionCoord();
    qint32 getProjectionX(int line, int pixelx);
    qint32 getProjectionY(int line, int pixelx);
    QRgb getProjectionValue(int line, int pixelx);
    void setProjectionX(int line, int pixelx, int projX);
    void setProjectionY(int line, int pixelx, int projY);
    void setProjectionValue(int line, int pixelx, QRgb val);

private:

    AVHRRSatellite *segs;
    QScopedArrayPointer<float> lon_array;
    QScopedArrayPointer<float> lat_array;
    int imagewidth;
    int imageheight;
    QScopedArrayPointer<int> projectionCoordX;
    QScopedArrayPointer<int> projectionCoordY;
    QScopedArrayPointer<QRgb> projectionCoordValue;


};

#endif // EQUIRECTANGULAR_H
