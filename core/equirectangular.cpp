#include "equirectangular.h"

#include <QDebug>
#include "globals.h"
#include "options.h"

extern Options opts;
extern SegmentImage *imageptrs;
extern gshhsData *gshhsdata;

Equirectangular::Equirectangular(QObject *parent, AVHRRSatellite *seglist) :
    QObject(parent)
{
    segs = seglist;
}

Equirectangular::~Equirectangular()
{

}

void Equirectangular::Initialize(int imwidth, int imheight)
{
    imagewidth = imwidth;
    imageheight = imheight;

    lon_array.reset(new float[imagewidth]);
    float lon_del = 360.0/(float)imagewidth;
    float lat_del = 180.0/(float)imageheight;

    for (int i = 0; i < imagewidth; i++)
        lon_array[i] = (i + 0.5) * lon_del - 180.0;

    lat_array.reset(new float[imageheight]);
    for (int i = 0; i < imageheight; i++)
        lat_array[i] = 90.0 - (i + 0.5) * lat_del;



    //initializeProjectionCoord();

}

void Equirectangular::map_forward(float lon_deg, float lat_deg, int &map_x, int &map_y)
{
    for (int i = 0; i < imagewidth-1; i++)
    {
        if(lon_deg >= lon_array[i] && lon_deg < lon_array[i+1])
        {
            map_x = i;
            break;
        }
    }

    for (int i = 0; i < imageheight-1; i++)
    {
        if(lat_deg <= lat_array[i] && lat_deg > lat_array[i+1])
        {
            map_y = i;
            break;
        }
    }

}

void Equirectangular::map_inverse(int map_x, int map_y, float &lon_deg, float &lat_deg)
{
    lon_deg = lon_array[map_x];
    lat_deg = lat_array[map_y];
}

void Equirectangular::map_inverse_rad(int map_x, int map_y, float &lon_rad, float &lat_rad)
{
    lon_rad = lon_array[map_x] * RADS_PER_DEG;
    lat_rad = lat_array[map_y] * RADS_PER_DEG;
}

void Equirectangular::initializeProjectionCoord()
{
    projectionCoordX.reset(new int[imagewidth * imageheight]);
    projectionCoordY.reset(new int[imagewidth * imageheight]);
    projectionCoordValue.reset(new QRgb[imagewidth * imageheight]);

    for( int i = 0; i < imageheight; i++)
    {
        for( int j = 0; j < imagewidth ; j++ )
        {
            projectionCoordX[i * imagewidth + j] = 65535;
            projectionCoordY[i * imagewidth + j] = 65535;
            projectionCoordValue[i * imagewidth + j] = qRgba(0, 0, 0, 0);
        }
    }

}

qint32 Equirectangular::getProjectionX(int line, int pixelx)
{
    return projectionCoordX[line * imagewidth + pixelx];
}

qint32 Equirectangular::getProjectionY(int line, int pixelx)
{
    return projectionCoordY[line * imagewidth + pixelx];
}

QRgb Equirectangular::getProjectionValue(int line, int pixelx)
{
    return projectionCoordValue[line * imagewidth + pixelx];
}

void Equirectangular::setProjectionX(int line, int pixelx, int projX)
{
    projectionCoordX[line * imagewidth + pixelx] = projX;
}

void Equirectangular::setProjectionY(int line, int pixelx, int projY)
{
    projectionCoordY[line * imagewidth + pixelx] = projY;
}

void Equirectangular::setProjectionValue(int line, int pixelx, QRgb val)
{
    projectionCoordValue[line * imagewidth + pixelx] = val;
}

