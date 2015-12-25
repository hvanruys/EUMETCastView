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


            initializeProjectionCoord();

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
    double x, y;
    lon_deg = lon_array[map_x];
    lat_deg = lat_array[map_y];
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

void Equirectangular::SmoothProjectionImageBilinear()
{
    qint32 x11;
    qint32 y11;
    QRgb rgb11;

    qint32 x12;
    qint32 y12;
    QRgb rgb12;

    qint32 x21;
    qint32 y21;
    QRgb rgb21;

    qint32 x22;
    qint32 y22;
    QRgb rgb22;

    qint32 xc11;
    qint32 yc11;

    qint32 xc12;
    qint32 yc12;

    qint32 xc21;
    qint32 yc21;

    qint32 xc22;
    qint32 yc22;

    qint32 minx;
    qint32 miny;
    qint32 maxx;
    qint32 maxy;

    qint32 anchorX;
    qint32 anchorY;

    int dimx, dimy;

    long counter = 0;
    long counterb = 0;

    QRgb *canvas;

    for (int line = 0; line < imageheight-1; line++)
    {
        for (int pixelx = 0; pixelx < imagewidth-1; pixelx++)
        {
            x11 = this->getProjectionX(line, pixelx);
            y11 = this->getProjectionY(line, pixelx);

            x12 = this->getProjectionX(line, pixelx+1);
            y12 = this->getProjectionY(line, pixelx+1);

            x21 = this->getProjectionX(line+1, pixelx);
            y21 = this->getProjectionY(line+1, pixelx);

            x22 = this->getProjectionX(line+1, pixelx+1);
            y22 = this->getProjectionY(line+1, pixelx+1);

            if(x11 < 65528 && x12 < 65528 && x21 < 65528 && x22 < 65528
                    && y11 < 65528 && y12 < 65528 && y21 < 65528 && y22 < 65528)
            {
                minx = imageptrs->Min(x11, x12, x21, x22);
                miny = imageptrs->Min(y11, y12, y21, y22);
                maxx = imageptrs->Max(x11, x12, x21, x22);
                maxy = imageptrs->Max(y11, y12, y21, y22);

                anchorX = minx;
                anchorY = miny;
                dimx = maxx + 1 - minx;
                dimy = maxy + 1 - miny;
                if( dimx == 1 && dimy == 1 )
                {
                    counter++;
                }
                else
                {
                    rgb11 = this->getProjectionValue(line, pixelx);
                    rgb12 = this->getProjectionValue(line, pixelx+1);
                    rgb21 = this->getProjectionValue(line+1, pixelx);
                    rgb22 = this->getProjectionValue(line+1, pixelx+1);


                    xc11 = x11 - minx;
                    xc12 = x12 - minx;
                    xc21 = x21 - minx;
                    xc22 = x22 - minx;
                    yc11 = y11 - miny;
                    yc12 = y12 - miny;
                    yc21 = y21 - miny;
                    yc22 = y22 - miny;


                    canvas = new QRgb[dimx * dimy];
                    for(int i = 0 ; i < dimx * dimy ; i++)
                        canvas[i] = qRgba(0,0,0,0);

                    canvas[yc11 * dimx + xc11] = rgb11;
                    canvas[yc12 * dimx + xc12] = rgb12;
                    canvas[yc21 * dimx + xc21] = rgb21;
                    canvas[yc22 * dimx + xc22] = rgb22;


                    imageptrs->bhm_line(xc11, yc11, xc12, yc12, rgb11, rgb12, canvas, dimx);
                    imageptrs->bhm_line(xc12, yc12, xc22, yc22, rgb12, rgb22, canvas, dimx);
                    imageptrs->bhm_line(xc22, yc22, xc21, yc21, rgb22, rgb21, canvas, dimx);
                    imageptrs->bhm_line(xc21, yc21, xc11, yc11, rgb21, rgb11, canvas, dimx);


                    imageptrs->MapInterpolation(canvas, dimx, dimy);
                    imageptrs->MapCanvas(canvas, anchorX, anchorY, dimx, dimy, false);


                    delete [] canvas;
                    counterb++;
                }
            }
        }
    }

    qDebug() << QString("====> end Equirectangular::SmoothProjectionImageBilinear() counter = %1 countern = %2").arg(counter).arg(counterb);

}

