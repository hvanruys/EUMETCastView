#include "stereographic.h"
#include "globals.h"
#include "options.h"
#include "pixgeoconversion.h"
#include "segmentimage.h"


#include <QDebug>

extern Options opts;
extern SegmentImage *imageptrs;
extern gshhsData *gshhsdata;

StereoGraphic::StereoGraphic(QObject *parent, AVHRRSatellite *seglist) :
  QObject(parent)
{
    segs = seglist;
    qDebug() << QString("constructor StereoGraphic");
    r_major = R_MAJOR_A_WGS84;
    false_northing = 0;
    false_easting = 0;
    scale = opts.mapsgscale;
    double map_x,map_y;

    map_radius = 0;
//    Initialize(opts.mapsglon, opts.mapsglat, opts.mapsgscale, opts.mapwidth, opts.mapheight);
    Initialize(0.0, 0.0, 1.0, opts.mapwidth, opts.mapheight, 0, 0);
    qDebug() << "mapwidth = " << opts.mapwidth << " mapheight = " << opts.mapheight;
    if(forward(0, opts.mapsgradius*PI/180.0, map_x, map_y))
        map_radius = fabs(map_y);
    else
        qDebug() << "wrong map_radius";
    qDebug() << "StereoGraphic::StereoGraphic =======> " << map_radius;
    map_radius =1000000;
}


void StereoGraphic::Initialize(double center_lon, double center_lat, double inscale, int imagewidth, int imageheight, int easting, int northing)
{

    lon_center = center_lon*PI/180.0;
    lat_origin = center_lat*PI/180.0;
    scale =inscale;

#ifdef WIN32
    sin_p10 = sin(lat_origin);
    cos_p10 = cos(lat_origin);
#else
    sincos(lat_origin, &sin_p10, &cos_p10);
#endif

    image_width = imagewidth;
    image_height = imageheight;

    if (imagewidth != imageptrs->ptrimageProjection->width() || imageheight != imageptrs->ptrimageProjection->height())
    {
        delete imageptrs->ptrimageProjection;
        imageptrs->ptrimageProjection = new QImage(imagewidth, imageheight, QImage::Format_ARGB32);
    }
    imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
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
    false_easting =  easting;
    false_northing = northing;

}

void StereoGraphic::CreateMapFromGeostationary()
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

    //imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    qDebug() << QString("Start StereoGraphic::CreateMapFromMeteosat");

    SegmentListGeostationary *sl;

    sl = segs->getActiveSegmentList();
    if(sl == NULL)
        return;

    double sub_lon = sl->geosatlon;

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

    if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8)
    {
        int LECA = 11136 - sl->LowerEastColumnActual;
        int LSLA = 11136 - sl->LowerSouthLineActual;
        int LWCA = 11136 - sl->LowerWestColumnActual;
        int LNLA = 11136 - sl->LowerNorthLineActual;

        int UECA = 11136 - sl->UpperEastColumnActual;
        int USLA = 11136 - sl->UpperSouthLineActual;
        int UWCA = 11136 - sl->UpperWestColumnActual;
        int UNLA = 11136 - sl->UpperNorthLineActual;
    }

    qDebug() << QString("sl->areatype = %1   hrvmap = %2").arg(sl->areatype).arg(hrvmap);
    qDebug() << QString("LECA = %1").arg(LECA);
    qDebug() << QString("LSLA = %1").arg(LSLA);
    qDebug() << QString("LWCA = %1").arg(LWCA);
    qDebug() << QString("LNLA = %1").arg(LNLA);
    qDebug() << QString("UECA = %1").arg(UECA);
    qDebug() << QString("USLA = %1").arg(USLA);
    qDebug() << QString("UWCA = %1").arg(UWCA);
    qDebug() << QString("UNLA = %1").arg(UNLA);



    for (int j = 0; j < imageptrs->ptrimageProjection->height(); j++)
    {
        for (int i = 0; i < imageptrs->ptrimageProjection->width(); i++)
        {
            if (this->map_inverse(i, j, lon_rad, lat_rad))
            {
                if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8)
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
                                if( picrow >= 0 && picrow < 5*464)
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
                            else
                            {
                                if( row < (sl->areatype == 0 ? 5*464 : 11136))
                                {
                                    scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(picrow);

                                    if (row >=LNLA ) //LOWER
                                    {
                                        if( col > LWCA && col < LECA)
                                            piccol = col - LWCA;
                                        else
                                            piccol = -1;
                                    }
                                    else //UPPER
                                    {
                                        if( col > UWCA && col < UECA)
                                            piccol = col - UWCA;
                                        else
                                            piccol = -1;
                                    }

                                    if(piccol >= 0 && piccol < 5568)
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
                        }
                    }
                }
            }
        }
    }

    fb_painter.end();

    QApplication::restoreOverrideCursor();
}

void StereoGraphic::CreateMapFromAVHRR(int inputchannel, eSegmentType type)
{

    if (type == SEG_NOAA)
        segs->seglnoaa->ComposeSGProjection(inputchannel);
    else if( type == SEG_METOP)
        segs->seglmetop->ComposeSGProjection(inputchannel);
    else if( type == SEG_GAC)
        segs->seglgac->ComposeSGProjection(inputchannel);
    else if( type == SEG_HRP)
        segs->seglhrp->ComposeSGProjection(inputchannel);
    else if( type == SEG_HRPT_METOPA)
        segs->seglmetopAhrpt->ComposeSGProjection(inputchannel);
    else if( type == SEG_HRPT_METOPB)
        segs->seglmetopBhrpt->ComposeSGProjection(inputchannel);
    else if( type == SEG_HRPT_NOAA19)
        segs->seglnoaa19hrpt->ComposeSGProjection(inputchannel);
    else if( type == SEG_HRPT_M01)
        segs->seglM01hrpt->ComposeSGProjection(inputchannel);
    else if( type == SEG_HRPT_M02)
        segs->seglM02hrpt->ComposeSGProjection(inputchannel);

    if(opts.smoothprojectiontype == 1)
        imageptrs->SmoothProjectionImage();
    else if(opts.smoothprojectiontype == 2)
    {
        if (type == SEG_NOAA)
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

void StereoGraphic::CreateMapFromVIIRS(eSegmentType type, bool combine)
{
    if (type == SEG_VIIRSM)
        segs->seglviirsm->ComposeSGProjection(0);
    else if( type == SEG_VIIRSDNB)
        segs->seglviirsdnb->ComposeSGProjection(0);

    if(opts.smoothprojectiontype == 1)
        imageptrs->SmoothProjectionImage();
    else if(opts.smoothprojectiontype == 2)
    {
        if (type == SEG_VIIRSM)
            segs->seglviirsm->SmoothVIIRSImage(combine);
        else if( type == SEG_VIIRSDNB)
            segs->seglviirsdnb->SmoothVIIRSImage(combine);
    }

}

void StereoGraphic::CreateMapFromOLCI(eSegmentType type, bool combine, int histogrammethod, bool normalized)
{
    if (type == SEG_OLCIEFR)
        segs->seglolciefr->ComposeSGProjection(0, histogrammethod, normalized);
    else if( type == SEG_OLCIERR)
        segs->seglolcierr->ComposeSGProjection(0, histogrammethod, normalized);

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

bool StereoGraphic::map_forward(double lon_rad, double lat_rad, double &map_x, double &map_y)
{

    double x, y;
    double dist=acos(sin_p10*sin(lat_rad)+cos_p10*cos(lat_rad)*cos(lon_center-lon_rad));
    if(dist > opts.mapsgradius*PI/180.0) return false;

    bool ret = this->forward(lon_rad, lat_rad, x, y);
    if(ret)
    {
        map_y = false_northing + mapdeltay + (map_radius * scale - y) / (map_radius * scale / (map_height/2));
        map_x = false_easting + mapdeltax + (x + map_radius * scale) / (map_radius * scale / (map_width/2));
        if(map_x < 0 || map_x >= image_width || map_y < 0 || map_y >= image_height)
            ret = false;

    }

    return ret;
}

bool StereoGraphic::map_forward_neg_coord(double lon_rad, double lat_rad, double &map_x, double &map_y)
{

    double x, y;
    double dist=acos(sin_p10*sin(lat_rad)+cos_p10*cos(lat_rad)*cos(lon_center-lon_rad));
    if(dist > opts.mapsgradius*PI/180.0) return false;

    bool ret = this->forward(lon_rad, lat_rad, x, y);
    if(ret)
    {
        map_y = false_northing + mapdeltay + (map_radius * scale - y) / (map_radius * scale / (map_height/2));
        map_x = false_easting + mapdeltax + (x + map_radius * scale) / (map_radius * scale / (map_width/2));
    }

    return ret;
}

bool StereoGraphic::forward(double lon_rad, double lat_rad, double &x, double &y)
{
    double sinphi, cosphi;	/* sin and cos value				*/
    double dlon;		    /* delta longitude value			*/
    double coslon;		    /* cos of longitude				*/
    double ksp;		        /* scale factor					*/
    double g;

    /* Forward equations
      -----------------*/
    dlon = adjust_lon_rad(lon_rad - lon_center);

#ifdef WIN32
    sinphi = sin(lat_rad);
    cosphi = cos(lat_rad);
#else
    sincos(lat_rad, &sinphi, &cosphi);
#endif

    coslon = cos(dlon);
    g = sin_p10 * sinphi + cos_p10 * cosphi * coslon;
    if (fabs(g + 1.0) <= EPSLN)
        return(false);
    else
    {
       ksp = 2.0 / (1.0 + g);
       x = r_major * ksp * cosphi * sin(dlon);
       y = r_major * ksp * (cos_p10 * sinphi - sin_p10 *
                cosphi * coslon);
    }
    return(true);
}

bool StereoGraphic::map_inverse(double map_x, double map_y, double &lon_rad, double &lat_rad)
{
    double x, y;


    y = map_radius * scale * (- 2*map_y + 2*(mapdeltay+false_northing) + map_height) / map_height;
    x = map_radius * scale * (2*map_x - 2*(mapdeltax+false_easting) - map_width) / map_width;

    //qDebug() << QString("x=%1 y=%2 map_radius=%3 map_height=%4 map_width=%5").arg(x).arg(y).arg(map_radius).arg(map_height).arg(map_width);

    bool ret = this->inverse(x, y, lon_rad, lat_rad);
    return ret;
}

bool StereoGraphic::inverse(double x, double y, double &lon_rad, double &lat_rad)
{
    double rh;  		/* height above ellipsoid			*/
    double z;           /* angle					*/
    double sinz,cosz;	/* sin of z and cos of z			*/
    double con;

    /* Inverse equations
      -----------------*/
    x -= 0.0; //false_easting;
    y -= 0.0; //false_northing;
    rh = sqrt(x * x + y * y);
    z = 2.0 * atan(rh / (2.0 * r_major));

#ifdef WIN32 && __GNUC__
    sinz = sin(z);
    cosz = cos(z);
#else
    sincos(z, &sinz, &cosz);
#endif

    lon_rad = lon_center;
    if (fabs(rh) <= EPSLN)
    {
       lat_rad = lat_origin;
       return(true);
    }
    else
    {
       lat_rad = asin(cosz * sin_p10 + (y * sinz * cos_p10) / rh);
       con = fabs(lat_origin) - HALF_PI;
       if (fabs(con) <= EPSLN)
       {
         if (lat_origin >= 0.0)
         {
           lon_rad = adjust_lon_rad(lon_center + atan2(x, -y));
           return(true);
         }
         else
         {
           lon_rad = adjust_lon_rad(lon_center - atan2(-x, y));
           return(true);
         }
       }
       else
       {
         con = cosz - sin_p10 * sin(lat_rad);
         if ((fabs(con) < EPSLN) && (fabs(x) < EPSLN))
            return(true);
         else
           lon_rad = adjust_lon_rad(lon_center + atan2((x * sinz * cos_p10), (con * rh)));
       }
     }

    return(true);
}

