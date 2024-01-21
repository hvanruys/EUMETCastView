#include "generalverticalperspective.h"
#include "globals.h"
#include "pixgeoconversion.h"
#include <QtConcurrent/QtConcurrent>
#include <QPainter>
#include <QDebug>

extern gshhsData *gshhsdata;

GeneralVerticalPerspective::GeneralVerticalPerspective(XMLVideoReader *reader, RSSVideo *video, QImage *imGeostationary, QObject *parent ) : QObject(parent)
{
    image_width = 0;
    image_height = 0;
    this->reader = reader;
    this->video = video;
    this->imageGeostationary = imGeostationary;
    Initialize(reader->gvplongitude, reader->gvplatitude, reader->gvpheight, reader->gvpscale,
               reader->gvpfalseeasting, reader->gvpfalsenorthing, reader->videowidth, reader->videoheight);
}

GeneralVerticalPerspective::~GeneralVerticalPerspective()
{
    delete imageProjection;
}

double GeneralVerticalPerspective::Initialize(double lonmapdeg, double latmapdeg, double heightmapkm, double scaling,
                                              double falseeasting, double falsenorthing, int imagewidth, int imageheight)
{

//    if (imagewidth != video->ptrimageProjection->width() || imageheight != video->ptrimageProjection->height())
//    {
//        delete video->ptrimageProjection;
        imageProjection = new QImage(imagewidth, imageheight, QImage::Format_ARGB32);
        imageProjection->fill(qRgba(0, 0, 0, 250));
//    }

//    video->ptrimageProjectionRed.reset(new quint16[imagewidth * imageheight]);
//    video->ptrimageProjectionGreen.reset(new quint16[imagewidth * imageheight]);
//    video->ptrimageProjectionBlue.reset(new quint16[imagewidth * imageheight]);
//    video->ptrimageProjectionAlpha.reset(new quint16[imagewidth * imageheight]);

//    for(int i = 0; i < imagewidth * imageheight; i++)
//    {
//        video->ptrimageProjectionRed[i] = 0;
//        video->ptrimageProjectionGreen[i] = 0;
//        video->ptrimageProjectionBlue[i] = 0;
//        video->ptrimageProjectionAlpha[i] = 0;
//    }


    image_width = imagewidth;
    image_height = imageheight;
    lon_center = lonmapdeg*PIE/180.0;
    lat_center = latmapdeg*PIE/180.0;
    R = 6370997.0;
    p = 1.0 + heightmapkm*1000/R;

    int mapwh;
    if (imagewidth > imageheight)
    {
        mapwh = imageheight;
        mapdeltax = (imagewidth - imageheight)/2;
        mapdeltay = 0;
    }
    else if(imageheight > imagewidth)
    {

        mapwh = imagewidth;
        mapdeltay = (imageheight - imagewidth)/2;
        mapdeltax = 0;
    }
    else
    {
        mapwh = imageheight;
        mapdeltax = 0;
        mapdeltay = 0;
    }

    map_width = mapwh;
    map_height = mapwh;
    false_easting =  falseeasting * 1000.0;
    false_northing = falsenorthing * 1000.0;
    map_radius = R*sqrt((p-1)/(p+1));
    scale = scaling;

//    qDebug() << QString("-------> Map radius = %1 scale = %2").arg(map_radius).arg(scale);


#ifdef WIN32 //&& __GNUC__
        sin_lat_o = sin(lat_center);
        cos_lat_o = cos(lat_center);
#else
        sincos(lat_center, &sin_lat_o,&cos_lat_o);
#endif

    return map_radius;
}



void GeneralVerticalPerspective::CreateMapFromGeoStationary(QPainter *fb_painter, int leca, int lsla, int lwca, int lnla,
                                                            int ueca, int usla, int uwca, int unla)
{

    QRgb *scanl;
    QRgb rgbval;
    //QPainter fb_painter(video->ptrimageProjection);

    fb_painter->setPen( Qt::black );
    fb_painter->setBrush( Qt::NoBrush );

    pixgeoConversion pixconv;

    double lon_rad;
    double lat_rad;
    int col, piccol;
    int row, picrow;
    int hrvmap = 0;

    int piccnt = 0;
    double sub_lon;

    if(reader->daykindofimage == "HRV" || reader->daykindofimage == "HRV Color")
        hrvmap = 1;

    int LECAdiff = 0;
    int LSLAdiff = 0;
    int LWCAdiff = 0;
    int LNLAdiff = 0;

    int UECAdiff = 0;
    int USLAdiff = 0;
    int UWCAdiff = 0;
    int UNLAdiff = 0;

    if(reader->satname == "MET_11" || reader->satname == "MET_10" || reader->satname == "MET_9" || reader->satname == "MET_8")
    {
        LECAdiff = 11136 - leca;
        LSLAdiff = 11136 - lsla;
        LWCAdiff = 11136 - lwca;
        LNLAdiff = 11136 - lnla;

        UECAdiff = 11136 - ueca;
        USLAdiff = 11136 - usla;
        UWCAdiff = 11136 - uwca;
        UNLAdiff = 11136 - unla;
    }


    sub_lon = reader->satlon;

    long coff = hrvmap ? reader->coffhrv : reader->coff;
    long loff = hrvmap ? reader->loffhrv : reader->loff;
    double cfac = hrvmap ? reader->cfachrv : reader->cfac;
    double lfac = hrvmap ? reader->lfachrv : reader->lfac;



    double scale_x = 0.000056;
    double scale_y = -0.000056;
    double offset_x = -0.151844;
    double offset_y = 0.151844;
    int sat = 1;
    double lat_deg;
    double lon_deg;
    int ret;
    double fgf_x, fgf_y;


    for (int j = 0; j < imageProjection->height(); j++)
    {
        for (int i = 0; i < imageProjection->width(); i++)
        {
            if (this->map_inverse(i, j, lon_rad, lat_rad))
            {
                if(reader->satname == "MET_11" || reader->satname == "MET_10" || reader->satname == "MET_9" || reader->satname == "MET_8")
                {
                    if(pixconv.geocoord2pixcoord(sub_lon, lat_rad*180.0/PIE, lon_rad*180.0/PIE, coff, loff, cfac, lfac, &col, &row) == 0)
                    {
                        if( reader->bhrv == false)
                        {
                            if(row < imageGeostationary->height())
                            {
                                scanl = (QRgb*)imageGeostationary->scanLine(row);
                                rgbval = scanl[col];
                                fb_painter->setPen(rgbval);
                                fb_painter->drawPoint(i,j);
                            }
                        }
                        else
                        {
                            row+=5; //5;
                            col+=3; //3;
                            picrow = row;

                            if(reader->brss)
                            {
                                // for RSS images UWCA and UECA are zero
                                if( row < 9*464)
                                {
                                    scanl = (QRgb*)imageGeostationary->scanLine(picrow);

                                    if( col > LWCAdiff && col < LECAdiff)
                                        piccol = col - LWCAdiff;
                                    else
                                        piccol = 0;

                                    if(piccol > 0 && piccol < 5568)
                                    {
                                        rgbval = scanl[piccol];
                                        fb_painter->setPen(rgbval);
                                        fb_painter->drawPoint(i,j);
                                    }
                                }

                            }
                            else
                            {
                                if( picrow < 6*464)
                                {
                                    scanl = (QRgb*)imageGeostationary->scanLine(picrow);

//                                    if (picrow > usla ) //LOWER
//                                    {
//                                        if( col > LWCA && col < LECA)
//                                            piccol = col - LWCA;
//                                        else
//                                            piccol = 0;
//                                    }
//                                    else //UPPER
//                                    {
                                        if( col > UWCAdiff && col < UECAdiff)
                                            piccol = col - UWCAdiff;
                                        else
                                            piccol = 0;
//                                    }

                                    if(piccol > 0 && piccol < 5568)
                                    {
                                        rgbval = scanl[piccol];
                                        fb_painter->setPen(rgbval);
                                        fb_painter->drawPoint(i,j);
                                    }
                                }
                            }
                        }

                    }
                }
//                else if( reader->satname == "GOES_16" || reader->satname == "GOES_17")
//                {
//                    lon_deg = lon_rad * 180.0 / PI;
//                    lat_deg = lat_rad * 180.0 / PI;

//                    pixconv.earth_to_fgf_(&sat, &lon_deg, &lat_deg, &scale_x, &offset_x, &scale_y, &offset_y, &sub_lon, &fgf_x, &fgf_y);
//                    if(fgf_x >= 0 && fgf_x < opts.geosatellites.at(geoindex).imagewidth && fgf_y >= 0 && fgf_y < opts.geosatellites.at(geoindex).imageheight)
//                    {
//                        col = (int)fgf_x;
//                        row = (int)fgf_y;
//                        ret = 0;
//                    }
//                    else
//                        ret = 1;

//                    if(ret == 0)
//                    {
//                        picrow = row;
//                        if(picrow < video->ptrimageGeostationary->height())
//                        {
//                            scanl = (QRgb*)video->ptrimageGeostationary->scanLine(picrow);
//                            rgbval = scanl[col];
//                            fb_painter.setPen(rgbval);
//                            fb_painter.drawPoint(i,j);
//                        }
//                    }
//                }
//                else
//                {
//                    if (this->map_inverse(i, j, lon_rad, lat_rad))
//                    {
//                        if(pixconv.geocoord2pixcoord(sub_lon, lat_rad*180.0/PI, lon_rad*180.0/PI, coff, loff, cfac, lfac, &col, &row) == 0)
//                        {
//                            picrow = row;
//                            if(picrow < imageGeostationary->height())
//                            {
//                                scanl = (QRgb*)imageGeostationary->scanLine(picrow);
//                                rgbval = scanl[col];
//                                fb_painter->setPen(rgbval);
//                                fb_painter->drawPoint(i,j);
//                            }
//                        }
//                    }
//                }
            }
        }
    }


}



bool GeneralVerticalPerspective::map_forward(double lon_rad, double lat_rad, double &map_x, double &map_y)
{

    double x, y;
    bool ret = this->genpersfor(lon_rad, lat_rad, &x, &y);
    if(ret)
    {
        map_y = mapdeltay + (map_radius * scale - y) / (map_radius * scale / (map_height/2));
        map_x = mapdeltax + (x + map_radius * scale) / (map_radius * scale / (map_width/2));
        if(map_x < 0 || map_x >= image_width || map_y < 0 || map_y >= image_height)
            ret = false;

    }

    return ret;
}

bool GeneralVerticalPerspective::map_forward_neg_coord(double lon_rad, double lat_rad, double &map_x, double &map_y)
{

    double x, y;
    bool ret = this->genpersfor(lon_rad, lat_rad, &x, &y);
    if(ret)
    {
        map_y = mapdeltay + (map_radius * scale - y) / (map_radius * scale / (map_height/2));
        map_x = mapdeltax + (x + map_radius * scale) / (map_radius * scale / (map_width/2));
    }

    return ret;
}

bool GeneralVerticalPerspective::genpersfor(double lon, double lat, double *x, double *y)
{
    double dlon;
    double sinlon;
    double sinphi,cosphi;
    double coslon;
    double g;
    double ksp;

    // Forward equations
    //-----------------
    dlon = adjust_lon_rad(lon - lon_center);


//#ifdef WIN32 && __GNUC__
//        sinphi = sin(lat);
//        cosphi = cos(lat);
//#else
//        sincos(lat,&sinphi,&cosphi);
//#endif
        sinphi = sin(lat);
        cosphi = cos(lat);

        coslon = cos(dlon);
        sinlon = sin(dlon);

    g = sin_lat_o * sinphi + cos_lat_o * cosphi * coslon;
    if (g < (1.0/ p))
    {
        //qDebug() << QString("Point cannot be projected lon = %1 lat = %2").arg(lon*180/PI).arg(lat*180/PI);
        return(false);
    }
    ksp = (p - 1.0)/(p - g);
    *x = false_easting + R * ksp * cosphi * sinlon;
    *y = false_northing + R * ksp * (cos_lat_o * sinphi - sin_lat_o * cosphi * coslon);

    return(true);
}

bool GeneralVerticalPerspective::map_inverse(double map_x, double map_y, double &lon_rad, double &lat_rad)
{
    double x, y;



    //map_y = mapdeltay + (map_radius * scale - y) / (map_radius * scale / (map_height/2));
    //map_x = mapdeltax + (x + map_radius * scale) / (map_radius * scale / (map_width/2));

    y = map_radius * scale * (- 2*map_y + 2*mapdeltay + map_height) / map_height;
    x = map_radius * scale * (2*map_x - 2*mapdeltax - map_width) / map_width;

    //qDebug() << QString("x=%1 y=%2 map_radius=%3 map_height=%4 map_width=%5").arg(x).arg(y).arg(map_radius).arg(map_height).arg(map_width);

    bool ret = this->genpersinv(x, y, &lon_rad, &lat_rad);
    return ret;
}

bool GeneralVerticalPerspective::genpersinv(double x, double y, double *lon, double *lat)
{
    double rh;
    double r;
    double con;
    double com;
    double z,sinz,cosz;


    // Inverse equations
    // -----------------
    x -= false_easting;
    y -= false_northing;
    rh = sqrt(x * x + y * y);
    r  = rh / R;
    con = p - 1.0;
    com = p + 1.0;
    if (r > sqrt(con/com))
    {
        //qDebug() << "r > sqrt(con/com)";
        //p_error("Input data error","gvnsp-for");
        return(false);
    }

    sinz = (p - sqrt(1.0 - (r * r * com) / con)) / (con / r + r/con);
    z = asinz(sinz);
//#ifdef WIN32 && __GNUC__
//    sinz = sin(z);
//    cosz = cos(z);
//#else
//    sincos(z,&sinz,&cosz);
//#endif

    sinz = sin(z);
    cosz = cos(z);

    *lon = lon_center;
    if (fabs(rh) <= EPSLN)
    {
        *lat = lat_center;
        return(true);
    }
    *lat = asinz(cosz * sin_lat_o + ( y * sinz * cos_lat_o)/rh);
    con = fabs(lat_center) - PIO2;
    if (fabs(con) <= EPSLN)
    {
        if (lat_center >= 0.0)
        {
            *lon = adjust_lon_rad(lon_center + atan2(x, -y));
            return(true);
        }
        else
        {
            *lon = adjust_lon_rad(lon_center - atan2(-x, y));
            return(true);
        }
    }
    con = cosz - sin_lat_o * sin(*lat);
    if ((fabs(con) < EPSLN) && (fabs(x) < EPSLN))
        return(true);
    *lon  = adjust_lon_rad(lon_center + atan2((x * sinz * cos_lat_o), (con * rh)));

    return(true);
}

// Function to eliminate roundoff errors in asin
double GeneralVerticalPerspective::asinz(double con)
{
    if (fabs(con) > 1.0)
    {
        if (con > 1.0)
            con = 1.0;
        else
            con = -1.0;
    }
    return(asin(con));
}

