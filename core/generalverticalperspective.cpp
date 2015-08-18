#include "generalverticalperspective.h"
#include "globals.h"
#include "options.h"
#include "pixgeoconversion.h"
#include <QtConcurrent/QtConcurrent>


#include <QDebug>

extern Options opts;
extern SegmentImage *imageptrs;
extern gshhsData *gshhsdata;

GeneralVerticalPerspective::GeneralVerticalPerspective(QObject *parent, AVHRRSatellite *seglist) :
    QObject(parent)
{
    segs = seglist;
    qDebug() << QString("constructor GeneralVerticalPerspective");
    Initialize(opts.mapgvplon, opts.mapgvplat, opts.mapgvpheight, opts.mapgvpscale, opts.mapwidth, opts.mapheight);
}

GeneralVerticalPerspective::~GeneralVerticalPerspective()
{
}

double GeneralVerticalPerspective::Initialize(double lonmapdeg, double latmapdeg, double heightmapkm, double scaling, int imagewidth, int imageheight)
{

    if (imagewidth != imageptrs->ptrimageProjection->width() || imageheight != imageptrs->ptrimageProjection->height())
    {
        delete imageptrs->ptrimageProjection;
        imageptrs->ptrimageProjection = new QImage(imagewidth, imageheight, QImage::Format_ARGB32);
        imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    }

    image_width = imagewidth;
    image_height = imageheight;
    lon_center = lonmapdeg*PI/180.0;
    lat_center = latmapdeg*PI/180.0;
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
    false_easting =  0.0;
    false_northing = 0.0;
    map_radius = R*sqrt((p-1)/(p+1));
    scale = scaling;

#ifdef WIN32 && __GNUC__
    sin_lat_o = sin(lat_center);
    cos_lat_o = cos(lat_center);
#else
    sincos(lat_center, &sin_lat_o, &cos_lat_o);
#endif
   return map_radius;
}


void GeneralVerticalPerspective::CreateMapFromAVHRR(int inputchannel, eSegmentType type)
{

    if (type == SEG_NOAA)
        segs->seglnoaa->ComposeGVProjection(inputchannel);
    else if( type == SEG_METOP)
        segs->seglmetop->ComposeGVProjection(inputchannel);
    else if( type == SEG_GAC)
        segs->seglgac->ComposeGVProjection(inputchannel);
    else if( type == SEG_HRP)
        segs->seglhrp->ComposeGVProjection(inputchannel);

}

void GeneralVerticalPerspective::CreateMapFromVIIRS()
{
    segs->seglviirs->ComposeGVProjection(0);
}


//    int col, row;
//    double lon_rad, lat_rad;

//    qDebug() << "=====> start SegmentVIIRS::ComposeProjectionAlt";
//    for (int j = 0; j < imageptrs->ptrimageProjection->height(); j++)
//    {
//        for (int i = 0; i < imageptrs->ptrimageProjection->width(); i++)
//        {
//            if (imageptrs->gvp->map_inverse(i, j, lon_rad, lat_rad))
//            {
//                if(segs->seglviirs->lookupLonLat(lon_rad, lat_rad, col, row))
//                {

//                }
//            }
//        }
//    }
//    qDebug() << "=====> end SegmentVIIRS::ComposeProjectionAlt";

//}


void GeneralVerticalPerspective::CreateMapFromGeoStationary()
{
    QApplication::setOverrideCursor( Qt::WaitCursor ); // this might take time

    QRgb *scanl;
    QRgb rgbval;
    QPainter fb_painter(imageptrs->ptrimageProjection);

    fb_painter.setPen( Qt::black );
    fb_painter.setBrush( Qt::NoBrush );

    pixgeoConversion pixconv;

    double lon_rad;
    double lat_rad;
    int col, piccol;
    int row, picrow;
    int hrvmap = 0;

    int piccnt = 0;

    //imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    qDebug() << QString("Start GeneralVerticalPerspective::CreateMapFromGeoStationary");

    SegmentListGeostationary *sl;
    double sub_lon;
    sl = segs->getActiveSegmentList();
    sub_lon = sl->geosatlon;

    if(sl->getKindofImage() == "HRV" || sl->getKindofImage() == "HRV Color")
        hrvmap = 1;

    int LECA = 11136 - sl->LowerEastColumnActual;
    int LSLA = 11136 - sl->LowerSouthLineActual;
    int LWCA = 11136 - sl->LowerWestColumnActual;
    int LNLA = 11136 - sl->LowerNorthLineActual;

    int UECA = 11136 - sl->UpperEastColumnActual;
    int USLA = 11136 - sl->UpperSouthLineActual;
    int UWCA = 11136 - sl->UpperWestColumnActual;
    int UNLA = 11136 - sl->UpperNorthLineActual;

    qDebug() << QString("sl->areatype = %1   hrvmap = %2").arg(sl->areatype).arg(hrvmap);
    qDebug() << QString("COFF = %1").arg(sl->COFF);
    qDebug() << QString("LOFF = %1").arg(sl->LOFF);
    qDebug() << QString("CFAC = %1").arg(sl->CFAC);
    qDebug() << QString("LFAC = %1").arg(sl->LFAC);
    qDebug() << QString("LECA = %1").arg(LECA);
    qDebug() << QString("LSLA = %1").arg(LSLA);
    qDebug() << QString("LWCA = %1").arg(LWCA);
    qDebug() << QString("LNLA = %1").arg(LNLA);
    qDebug() << QString("UECA = %1").arg(UECA);
    qDebug() << QString("USLA = %1").arg(USLA);
    qDebug() << QString("UWCA = %1").arg(UWCA);
    qDebug() << QString("UNLA = %1").arg(UNLA);

    qDebug() << QString("ptrimage projection height = %1 width = %2").arg(imageptrs->ptrimageProjection->height()).arg(imageptrs->ptrimageProjection->width());
    qDebug() << QString("ptrimage meteosat height = %1 width = %2").arg(imageptrs->ptrimageGeostationary->height()).arg(imageptrs->ptrimageGeostationary->width());

    for (int j = 0; j < imageptrs->ptrimageProjection->height(); j++)
    {
        for (int i = 0; i < imageptrs->ptrimageProjection->width(); i++)
        {
            if (this->map_inverse(i, j, lon_rad, lat_rad))
            {
                if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9)
                {
                    if(pixconv.geocoord2pixcoord(sub_lon, lat_rad*180.0/PI, lon_rad*180.0/PI, sl->COFF, sl->LOFF, sl->CFAC, sl->LFAC, &col, &row) == 0)
                    {
                        picrow = row;
                        if( hrvmap == 0)
                        {
                            if(picrow < imageptrs->ptrimageGeostationary->height())
                            {
                                scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(picrow);
                                rgbval = scanl[col];
                                fb_painter.setPen(rgbval);
                                fb_painter.drawPoint(i,j);
                            }
                        }
                        else
                        {
                            if(sl->getGeoSatellite() == SegmentListGeostationary::MET_9)
                            {
                                if( row < 5*464)
                                {
                                    scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(picrow);

                                    if( col > LWCA && col < LECA)
                                        piccol = col - LWCA;
                                    else
                                        piccol = 0;

                                    if(piccol > 0 && piccol < 5568)
                                    {
                                        rgbval = scanl[piccol];
                                        fb_painter.setPen(rgbval);
                                        fb_painter.drawPoint(i,j);
                                    }
                                }

                            }
                            else if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10)
                            {
                                if( row < (sl->areatype == 0 ? 5*464 : 11136))
                                {
                                    scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(picrow);

                                    if (row > USLA ) //LOWER
                                    {
                                        if( col > LWCA && col < LECA)
                                            piccol = col - LWCA;
                                        else
                                            piccol = 0;
                                    }
                                    else //UPPER
                                    {
                                        if( col > UWCA && col < UECA)
                                            piccol = col - UWCA;
                                        else
                                            piccol = 0;
                                    }

                                    if(piccol > 0 && piccol < 5568)
                                    {
                                        rgbval = scanl[piccol];
                                        fb_painter.setPen(rgbval);
                                        fb_painter.drawPoint(i,j);
                                    }
                                }
                            }
                        }

                    }
                }
                else
                {
                    if(pixconv.geocoord2pixcoord(sub_lon, lat_rad*180.0/PI, lon_rad*180.0/PI, sl->COFF, sl->LOFF, sl->CFAC, sl->LFAC, &col, &row) == 0)
                    {
                        picrow = row;
                        if(picrow < imageptrs->ptrimageGeostationary->height())
                        {
                            scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(picrow);
                            rgbval = scanl[col];
                            fb_painter.setPen(rgbval);
                            fb_painter.drawPoint(i,j);
                            if(picrow == 1000)
                                piccnt++;
                        }
                    }
                }
            }
        }
    }

    qDebug() << QString("ptrimage projection einde piccnt = %1").arg(piccnt);
    fb_painter.end();

    QApplication::restoreOverrideCursor();
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

bool GeneralVerticalPerspective::genpersfor(double lon, double lat, double *x, double *y)
{
    double dlon;
    double sinphi,cosphi;
    double coslon;
    double g;
    double ksp;

    // Forward equations
    //-----------------
    dlon = adjust_lon_rad(lon - lon_center);

#ifdef WIN32 && __GNUC__
    sinphi = sin(lat);
    cosphi = cos(lat);
#else
    sincos(lat,&sinphi,&cosphi);
#endif

    coslon = cos(dlon);
    g = sin_lat_o * sinphi + cos_lat_o * cosphi * coslon;
    if (g < (1.0/ p))
    {
        //qDebug() << QString("Point cannot be projected lon = %1 lat = %2").arg(lon*180/PI).arg(lat*180/PI);
        return(false);
    }
    ksp = (p - 1.0)/(p - g);
    *x = false_easting + R * ksp * cosphi * sin(dlon);
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
#ifdef WIN32 && __GNUC__
    sinz = sin(z);
    cosz = cos(z);
#else
    sincos(z,&sinz,&cosz);
#endif

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

