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
    image_width = 0;
    image_height = 0;

    Initialize(opts.mapgvplon, opts.mapgvplat, opts.mapgvpheight, opts.mapgvpscale, opts.mapgvpeasting, opts.mapgvpnorthing, opts.mapwidth, opts.mapheight);
}

GeneralVerticalPerspective::~GeneralVerticalPerspective()
{
}

double GeneralVerticalPerspective::Initialize(double lonmapdeg, double latmapdeg, double heightmapkm, double scaling,
                                              double easting, double northing, int imagewidth, int imageheight)
{

    if (imagewidth != imageptrs->ptrimageProjection->width() || imageheight != imageptrs->ptrimageProjection->height())
    {
        delete imageptrs->ptrimageProjection;
        imageptrs->ptrimageProjection = new QImage(imagewidth, imageheight, QImage::Format_ARGB32);
        imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    }

    imageptrs->ptrimageProjectionRed.reset(new quint16[imagewidth * imageheight]);
    imageptrs->ptrimageProjectionGreen.reset(new quint16[imagewidth * imageheight]);
    imageptrs->ptrimageProjectionBlue.reset(new quint16[imagewidth * imageheight]);
    imageptrs->ptrimageProjectionAlpha.reset(new quint16[imagewidth * imageheight]);

    for(int i = 0; i < imagewidth * imageheight; i++)
    {
        imageptrs->ptrimageProjectionRed[i] = 0;
        imageptrs->ptrimageProjectionGreen[i] = 0;
        imageptrs->ptrimageProjectionBlue[i] = 0;
        imageptrs->ptrimageProjectionAlpha[i] = 0;
    }


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
    false_easting = easting * 1000.0;
    false_northing = northing * 1000.0;
    map_radius = R*sqrt((p-1)/(p+1));
    scale = scaling;

//    qDebug() << QString("-------> Map radius = %1 scale = %2").arg(map_radius).arg(scale);


#ifdef WIN32 //&& __GNUC__
        sin_lat_o = sin(lat_center);
        cos_lat_o = cos(lat_center);
#elif __APPLE__
        __sincos(lat_center, &sin_lat_o,&cos_lat_o);
#else
        sincos(lat_center, &sin_lat_o,&cos_lat_o);
#endif

    return map_radius;
}


void GeneralVerticalPerspective::CreateMapFromAVHRR(int inputchannel, eSegmentType type)
{

    if (type == SEG_NOAA19)
        segs->seglnoaa->ComposeGVProjection(inputchannel);
    else if( type == SEG_METOP)
        segs->seglmetop->ComposeGVProjection(inputchannel);
    else if( type == SEG_GAC)
        segs->seglgac->ComposeGVProjection(inputchannel);
    else if( type == SEG_HRP)
        segs->seglhrp->ComposeGVProjection(inputchannel);
    else if( type == SEG_HRPT_METOPA)
        segs->seglmetopAhrpt->ComposeGVProjection(inputchannel);
    else if( type == SEG_HRPT_METOPB)
        segs->seglmetopBhrpt->ComposeGVProjection(inputchannel);
    else if( type == SEG_HRPT_NOAA19)
        segs->seglnoaa19hrpt->ComposeGVProjection(inputchannel);
    else if( type == SEG_HRPT_M01)
        segs->seglM01hrpt->ComposeGVProjection(inputchannel);
    else if( type == SEG_HRPT_M02)
        segs->seglM02hrpt->ComposeGVProjection(inputchannel);

    if(opts.smoothprojectiontype == 1)
        imageptrs->SmoothProjectionImage();
    else if(opts.smoothprojectiontype == 2)
    {
        if (type == SEG_NOAA19)
            segs->seglnoaa->SmoothProjectionImageBilinear();
        else if( type == SEG_METOP)
            segs->seglmetop->SmoothProjectionImageBilinear();
        else if( type == SEG_GAC)
            segs->seglgac->SmoothProjectionImageBilinear();
        else if( type == SEG_HRP)
            segs->seglhrp->SmoothProjectionImageBilinear();
        else if( type == SEG_HRPT_METOPA)
            segs->seglmetopAhrpt->SmoothProjectionImageBilinear();
        else if( type == SEG_HRPT_METOPB)
            segs->seglmetopBhrpt->SmoothProjectionImageBilinear();
        else if( type == SEG_HRPT_NOAA19)
            segs->seglnoaa19hrpt->SmoothProjectionImageBilinear();
        else if( type == SEG_HRPT_M01)
            segs->seglM01hrpt->SmoothProjectionImageBilinear();
        else if( type == SEG_HRPT_M02)
            segs->seglM02hrpt->SmoothProjectionImageBilinear();
    }

}

void GeneralVerticalPerspective::CreateMapFromVIIRS(eSegmentType type, bool combine)
{
    if (type == SEG_VIIRSM)
        segs->seglviirsm->ComposeGVProjection(0);
    else if( type == SEG_VIIRSDNB)
        segs->seglviirsdnb->ComposeGVProjection(0);
    else if( type == SEG_VIIRSMNOAA20)
        segs->seglviirsmnoaa20->ComposeGVProjection(0);
    else if( type == SEG_VIIRSDNBNOAA20)
        segs->seglviirsdnbnoaa20->ComposeGVProjection(0);

    if(opts.smoothprojectiontype == 1)
        imageptrs->SmoothProjectionImage();
    else if(opts.smoothprojectiontype == 2)
    {
        if (type == SEG_VIIRSM)
        {
            segs->seglviirsm->SmoothVIIRSImage(combine);
            segs->seglviirsm->SmoothProjectionBrightnessTemp();
        }
        else if( type == SEG_VIIRSDNB)
            segs->seglviirsdnb->SmoothVIIRSImage(combine);
        else if (type == SEG_VIIRSMNOAA20)
        {
            segs->seglviirsmnoaa20->SmoothVIIRSImage(combine);
            segs->seglviirsmnoaa20->SmoothProjectionBrightnessTemp();
        }
        else if( type == SEG_VIIRSDNBNOAA20)
            segs->seglviirsdnbnoaa20->SmoothVIIRSImage(combine);
    }

}

void GeneralVerticalPerspective::CreateMapFromOLCI(eSegmentType type, bool combine, int histogrammethod, bool normalized)
{
    if (type == SEG_OLCIEFR)
        segs->seglolciefr->ComposeGVProjection(0, histogrammethod, normalized);
    else if( type == SEG_OLCIERR)
        segs->seglolcierr->ComposeGVProjection(0, histogrammethod, normalized);

    if(opts.smoothprojectiontype == 1)
        imageptrs->SmoothProjectionImage();
    else if(opts.smoothprojectiontype == 2)
    {
        if( type == SEG_OLCIEFR)
            segs->seglolciefr->SmoothOLCIImage(combine);
        else if( type == SEG_OLCIERR)
            segs->seglolcierr->SmoothOLCIImage(combine);
    }
}

void GeneralVerticalPerspective::CreateMapFromMERSI(eSegmentType type, bool combine)
{
    segs->seglmersi->ComposeGVProjection(0);

    if(opts.smoothprojectiontype == 1)
        imageptrs->SmoothProjectionImage();
    else if(opts.smoothprojectiontype == 2)
    {
        segs->seglmersi->SmoothMERSIImage(combine);
    }

}

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

    qDebug() << QString("Start GeneralVerticalPerspective::CreateMapFromGeoStationary");

    SegmentListGeostationary *sl;
    double sub_lon;
    sl = segs->getActiveSegmentList();
    if(sl == NULL)
        return;

    sub_lon = sl->geosatlon;
    int geoindex = sl->getGeoSatelliteIndex();

    if(sl->getKindofImage() == "HRV" || sl->getKindofImage() == "HRV Color")
        hrvmap = 1;

    int LECA = 0;
    int LSLA = 0;
    int LWCA = 0;
    int LNLA = 0;

    int UECA = 0;
    int USLA = 0;
    int UWCA = 0;
    int UNLA = 0;

    if(sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8)
    {
        LECA = 11136 - sl->LowerEastColumnActual;
        LSLA = 11136 - sl->LowerSouthLineActual;
        LWCA = 11136 - sl->LowerWestColumnActual;
        LNLA = 11136 - sl->LowerNorthLineActual;

        UECA = 11136 - sl->UpperEastColumnActual;
        USLA = 11136 - sl->UpperSouthLineActual;
        UWCA = 11136 - sl->UpperWestColumnActual;
        UNLA = 11136 - sl->UpperNorthLineActual;
    }

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


    sub_lon = opts.geosatellites.at(geoindex).longitude;

    double scale_x = 0.000056;
    double scale_y = -0.000056;
    double offset_x = -0.151844;
    double offset_y = 0.151844;
    int sat = 1;
    double lat_deg;
    double lon_deg;
    int ret;
    double fgf_x, fgf_y;


    for (int j = 0; j < imageptrs->ptrimageProjection->height(); j++)
    {
        for (int i = 0; i < imageptrs->ptrimageProjection->width(); i++)
        {
            if (this->map_inverse(i, j, lon_rad, lat_rad))
            {
                if(sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8)
                {
                    if(pixconv.geocoord2pixcoord(sub_lon, lat_rad*180.0/PIE, lon_rad*180.0/PIE, sl->COFF, sl->LOFF, sl->CFAC, sl->LFAC, &col, &row) == 0)
                    {
                        if( hrvmap == 0)
                        {
                            if(row < imageptrs->ptrimageGeostationary->height())
                            {
                                scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(row);
                                rgbval = scanl[col];
                                fb_painter.setPen(rgbval);
                                fb_painter.drawPoint(i,j);
                            }
                        }
                        else
                        {
                            row+=5; //5;
                            col+=3; //3;
                            picrow = row;

                            if( sl->bisRSS)
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
                            else  //if(sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8)
                            {
                                if( picrow < (sl->areatype == 0 ? 5*464 : 11136))
                                {
                                    scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(picrow);

                                    if (picrow > USLA ) //LOWER
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
                else if(sl->getGeoSatellite() == eGeoSatellite::GOES_16 || sl->getGeoSatellite() == eGeoSatellite::GOES_17 || sl->getGeoSatellite() == eGeoSatellite::GOES_18)
                {
                    lon_deg = lon_rad * 180.0 / PIE;
                    lat_deg = lat_rad * 180.0 / PIE;

                    pixconv.earth_to_fgf_(&sat, &lon_deg, &lat_deg, &scale_x, &offset_x, &scale_y, &offset_y, &sub_lon, &fgf_x, &fgf_y);
                    if(fgf_x >= 0 && fgf_x < opts.geosatellites.at(geoindex).imagewidth && fgf_y >= 0 && fgf_y < opts.geosatellites.at(geoindex).imageheight)
                    {
                        col = (int)fgf_x;
                        row = (int)fgf_y;
                        ret = 0;
                    }
                    else
                        ret = 1;

                    if(ret == 0)
                    {
                        picrow = row;
                        if(picrow < imageptrs->ptrimageGeostationary->height())
                        {
                            scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(picrow);
                            rgbval = scanl[col];
                            fb_painter.setPen(rgbval);
                            fb_painter.drawPoint(i,j);
                        }
                    }
                }
                else
                {
                    if (this->map_inverse(i, j, lon_rad, lat_rad))
                    {
                        if(pixconv.geocoord2pixcoord(sub_lon, lat_rad*180.0/PIE, lon_rad*180.0/PIE, sl->COFF, sl->LOFF, sl->CFAC, sl->LFAC, &col, &row) == 0)
                        {
                            picrow = row;
                            if(picrow < imageptrs->ptrimageGeostationary->height())
                            {
                                scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(picrow);
                                rgbval = scanl[col];
                                fb_painter.setPen(rgbval);
                                fb_painter.drawPoint(i,j);
                            }
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

