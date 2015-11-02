#include "lambertconformalconic.h"
#include "globals.h"
#include "options.h"
#include "pixgeoconversion.h"

#include <QDebug>

extern Options opts;
extern SegmentImage *imageptrs;
extern gshhsData *gshhsdata;

LambertConformalConic::LambertConformalConic(QObject *parent, AVHRRSatellite *seglist) :
    QObject(parent)
{
    segs = seglist;
    min_x = 9999999999.0;
    max_x = 0.0;
    min_y = 9999999999.0;
    max_y = 0.0;

     qDebug() << QString("constructor LambertConformalConic");

}

LambertConformalConic::~LambertConformalConic()
{

}

void LambertConformalConic::Initialize(double r_maj, double r_min, double stdlat1, double stdlat2, double c_lon, double c_lat, int imagewidth, int imageheight, int corrX, int corrY)
{
    /* double r_maj;                   major axis                           */
    /* double r_min;                   minor axis                           */
    /* double lat1;                    first standard parallel              */
    /* double lat2;                    second standard parallel             */
    /* double c_lon;                   center longitude                     */
    /* double c_lat;                   center latitude                      */
    /* double false_east;              x offset in meters                   */
    /* double false_north;             y offset in meters                   */

    image_width = imagewidth;
    image_height = imageheight;

    double lat1 = stdlat1*PI/180.0;
    double lat2 = stdlat2*PI/180.0;

    center_lon = c_lon * PI/180.0;
    center_lat = c_lat * PI/180.0;
    r_major = r_maj;
    r_minor = r_min;
    f0 = F;

    if (imagewidth != imageptrs->ptrimageProjection->width() || imageheight != imageptrs->ptrimageProjection->height())
    {
        delete imageptrs->ptrimageProjection;
        imageptrs->ptrimageProjection = new QImage(imagewidth, imageheight, QImage::Format_ARGB32);
        imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
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

    qDebug() << " mapwh = " << mapwh;


    double sin_po;                  /* sin value                            */
    double cos_po;                  /* cos value                            */
    double con;                     /* temporary variable                   */
    double ms1;                     /* small m 1                            */
    double ms2;                     /* small m 2                            */
    double temp;                    /* temporary variable                   */
    double ts0;                     /* small t 0                            */
    double ts1;                     /* small t 1                            */
    double ts2;                     /* small t 2                            */

    r_major = r_maj;
    r_minor = r_min;
    false_northing = corrX * 10000.0;
    false_easting = corrY * 10000.0;

    /* Standard Parallels cannot be equal and on opposite sides of the equator
    ------------------------------------------------------------------------*/
    if (fabs(lat1+lat2) < EPSLN)
       {
           qDebug() << "Equal latitudes for St. Parallels on opposite sides of equator";
           return;
       }

    temp = r_minor / r_major;
    es = 1.0 - temp*temp;
    e = sqrt(es);

#ifdef WIN32 && __GNUC__
    sin_po = sin(lat1);
    cos_po = cos(lat1);
#else
    sincos(lat1,&sin_po,&cos_po);
#endif

    con = sin_po;
    ms1 = msfnz(e,sin_po,cos_po);
    ts1 = tsfnz(e,lat1,sin_po);

#ifdef WIN32 && __GNUC__
    sin_po = sin(lat2);
    cos_po = cos(lat2);
#else
    sincos(lat2,&sin_po,&cos_po);
#endif

    ms2 = msfnz(e,sin_po,cos_po);
    ts2 = tsfnz(e,lat2,sin_po);
    sin_po = sin(center_lat);
    ts0 = tsfnz(e,center_lat,sin_po);

    if (fabs(lat1 - lat2) > EPSLN)
        ns = log (ms1/ms2)/ log (ts1/ts2);
    else
        ns = con;
    f0 = ms1 / (ns * pow(ts1,ns));
    rh = r_major * f0 * pow(ts0,ns);
    calc_map_extents();

    qDebug() << QString("LambertConformalConic::Initialize %1 x %2 ").arg(imagewidth).arg(imageheight);
}


void LambertConformalConic::CreateMapFromAVHRR(int inputchannel, eSegmentType type)
{

    calc_map_extents();

    if (type == SEG_NOAA)
        segs->seglnoaa->ComposeLCCProjection(inputchannel);
    else if( type == SEG_METOP)
        segs->seglmetop->ComposeLCCProjection(inputchannel);
    else if( type == SEG_GAC)
        segs->seglgac->ComposeLCCProjection(inputchannel);
    else if( type == SEG_HRP)
        segs->seglhrp->ComposeLCCProjection(inputchannel);


    if(opts.smoothprojectiontype == 1)
        imageptrs->SmoothProjectionImage();
    else if(opts.smoothprojectiontype == 2)
    {
        if (type == SEG_NOAA)
            segs->seglnoaa->SmoothProjectionImageBilinear();
        else if( type == SEG_METOP)
            segs->seglmetop->SmoothProjectionImageBilinear();
        else if( type == SEG_GAC)
//            segs->seglgac->SmoothProjectionImage();
            imageptrs->SmoothProjectionImage();
        else if( type == SEG_HRP)
//            segs->seglhrp->SmoothProjectionImage();
            imageptrs->SmoothProjectionImage();
    }
 }


void LambertConformalConic::CreateMapFromVIIRS(eSegmentType type)
{
    calc_map_extents();

    if (type == SEG_VIIRSM)
        segs->seglviirsm->ComposeLCCProjection(0);
    else if( type == SEG_VIIRSDNB)
        segs->seglviirsdnb->ComposeLCCProjection(0);

    if(opts.smoothprojectiontype == 1)
        imageptrs->SmoothProjectionImage();
    else if(opts.smoothprojectiontype == 2)
    {
        if (type == SEG_VIIRSM)
            segs->seglviirsm->SmoothVIIRSImage();
        else if( type == SEG_VIIRSDNB)
            segs->seglviirsdnb->SmoothVIIRSImage();
    }

}


void LambertConformalConic::CreateMapFromGeostationary()
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
    qDebug() << QString("Start LambertConformalConic::CreateMapFromMeteosat");

    SegmentListGeostationary *sl;
    double sub_lon;
    sl = segs->getActiveSegmentList();
    if(sl == NULL)
        return;

    qDebug() << QString("COFF = %1 COFF_HRV = %2 COFF_NON_HRV = %3").arg(sl->COFF).arg(COFF_HRV).arg(COFF_NONHRV);
    qDebug() << QString("LOFF = %1 LOFF_HRV = %2 LOFF_NON_HRV = %3").arg(sl->LOFF).arg(LOFF_HRV).arg(LOFF_NONHRV);
    qDebug() << QString("CFAC = %1 CFAC_HRV = %2 CFAC_NON_HRV = %3").arg(sl->CFAC).arg(CFAC_HRV).arg(CFAC_NONHRV);
    qDebug() << QString("LFAC = %1 LFAC_HRV = %2 LFAC_NON_HRV = %3").arg(sl->LFAC).arg(LFAC_HRV).arg(LFAC_NONHRV);

    sub_lon = sl->geosatlon;

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

    if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9)
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

    qDebug() << QString("sl->areatype = %1   hrvmap = %2  KindOfImage = %3").arg(sl->areatype).arg(hrvmap).arg(sl->getKindofImage());
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
                if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9)
                {
                    if(pixconv.geocoord2pixcoord(sub_lon, lat_rad*180.0/PI, lon_rad*180.0/PI, sl->COFF, sl->LOFF, sl->CFAC, sl->LFAC, &col, &row) == 0)
                    //if(pixconv.geocoord2pixcoord(sub_lon, lat_rad*180.0/PI, lon_rad*180.0/PI, COFF_HRV, LOFF_HRV, CFAC_HRV, LFAC_HRV, &col, &row) == 0)
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
                                    //qDebug() << QString("picrow = %1").arg(picrow);

                                    scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(picrow);

                                    if( col > LWCA && col < LECA)
                                        piccol = col - LWCA;
                                    else
                                        piccol = -1;

                                    if(piccol >= 0 && piccol < 5568)
                                    {
                                        rgbval = scanl[piccol];
                                        fb_painter.setPen(rgbval);
                                        fb_painter.drawPoint(i,j);
                                    }
                                }

                            }
                            else
                            {

                                if( picrow < (sl->areatype == 0 ? 5*464 : 11136))
                                {
                                    scanl = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(picrow);

                                    if (picrow >= LNLA ) //LOWER
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
                                        //qDebug() << "piccol = " << piccol << " rowcol = " << picrow;
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





void LambertConformalConic::calc_map_extents()
{
    double x, y;

    min_x = 99999999999.0;
    max_x = -999999999999.0;
    min_y = 99999999999.0;
    max_y = -999999999999.0;

    if(this->lamccfor(opts.mapextenteast*PI/180.0, opts.mapextentnorth*PI/180.0, &x, &y))
    {
        if(x > max_x)
            max_x = x;
        if(x < min_x)
            min_x = x;
        if(y > max_y)
            max_y = y;
        if(y < min_y)
            min_y = y;
    }

    if(this->lamccfor(opts.mapextentwest*PI/180.0, opts.mapextentnorth*PI/180.0, &x, &y))
    {
        if(x > max_x)
            max_x = x;
        if(x < min_x)
            min_x = x;
        if(y > max_y)
            max_y = y;
        if(y < min_y)
            min_y = y;
    }

    if(this->lamccfor(opts.mapextentwest*PI/180.0, opts.mapextentsouth*PI/180.0, &x, &y))
    {
        if(x > max_x)
            max_x = x;
        if(x < min_x)
            min_x = x;
        if(y > max_y)
            max_y = y;
        if(y < min_y)
            min_y = y;
    }

    if(this->lamccfor(opts.mapextenteast*PI/180.0, opts.mapextentsouth*PI/180.0, &x, &y))
    {
        if(x > max_x)
            max_x = x;
        if(x < min_x)
            min_x = x;
        if(y > max_y)
            max_y = y;
        if(y < min_y)
            min_y = y;
    }

    if(this->lamccfor(opts.centralmeridian*PI/180.0, opts.mapextentsouth*PI/180.0, &x, &y))
    {
        if(x > max_x)
            max_x = x;
        if(x < min_x)
            min_x = x;
        if(y > max_y)
            max_y = y;
        if(y < min_y)
            min_y = y;
    }

    if(this->lamccfor(opts.centralmeridian*PI/180.0, opts.mapextentnorth*PI/180.0, &x, &y))
    {
        if(x > max_x)
            max_x = x;
        if(x < min_x)
            min_x = x;
        if(y > max_y)
            max_y = y;
        if(y < min_y)
            min_y = y;
    }

    Ay = opts.maplccscaley * map_height;
    Ax = opts.maplccscalex * map_width;
    Dy = max_y - min_y;
    Dx = max_x - min_x;


    qDebug() << QString("min_x = %1 max_x = %2 min_y = %3 max_y = %4").arg(min_x).arg(max_x).arg(min_y).arg(max_y);

}

bool LambertConformalConic::map_forward(double lon_rad, double lat_rad, double &map_x, double &map_y)
{

    double x, y;
    bool ret = false;

    ret = this->lamccfor(lon_rad, lat_rad, &x, &y);

    if(ret)
    {
        map_y = mapdeltay + (max_y - y)*Ay/Dy;
        map_x = mapdeltax + (x*Ax - min_x*map_width) / Dx;
        if(map_x < 0 || map_x >= image_width || map_y < 0 || map_y >= image_height)
            ret = false;

    }

    return ret;
}

bool LambertConformalConic::map_forward_neg_coord(double lon_rad, double lat_rad, double &map_x, double &map_y)
{

    double x, y;
    bool ret = false;

    ret = this->lamccfor(lon_rad, lat_rad, &x, &y);

    if(ret)
    {
        map_y = mapdeltay + (max_y - y)*Ay/Dy;
        map_x = mapdeltax + (x*Ax - min_x*map_width) / Dx;

    }

    return ret;
}

bool LambertConformalConic::map_inverse(double map_x, double map_y, double &lon_rad, double &lat_rad)
{

    double x, y;
    bool ret = false;

    x = min_x * map_width / Ax + (map_x - mapdeltax) * Dx/Ax;
    y = max_y - (map_y - mapdeltay)*Dy/Ay;

    ret = this->lamccinv(x, y, &lon_rad, &lat_rad);
    return ret;
}

void LambertConformalConic::testmap()
{
    double map_x, map_y;
    double lon_rad, lat_rad;

    map_forward(3.88*PI/180.0, 50.9*PI/180.0, map_x, map_y);

    qDebug() << QString("testmap map_x = %1 map_y = %2").arg(map_x).arg(map_y);

    map_inverse(map_x, map_y, lon_rad, lat_rad);

    qDebug() << QString("testmap lon = %1 lat = %2").arg(lon_rad*180.0/PI).arg(lat_rad*180.0/PI);

}

/* Lambert Conformal conic forward equations--mapping lat,long to x,y
  -----------------------------------------------------------------*/
bool LambertConformalConic::lamccfor(double lon, double lat, double *x, double *y)
{

    double con;                     /* temporary angle variable             */
    double rh1;                     /* height above ellipsoid               */
    double sinphi;                  /* sin value                            */
    double theta;                   /* angle                                */
    double ts;                      /* small value t                        */

    con  = fabs( fabs(lat) - PIO2);
    if (con > EPSLN)
    {
        sinphi = sin(lat);
        ts = tsfnz(e,lat,sinphi);
        rh1 = r_major * f0 * pow(ts,ns);
    }
    else
    {
        con = lat * ns;
        if (con <= 0)
        {
            return(false);
        }
        rh1 = 0;
    }

    theta = ns * adjust_lon_rad(lon - center_lon);
    *x = rh1 * sin(theta) + false_easting;
    *y = rh - rh1 * cos(theta) + false_northing;

    return(true);
}

bool LambertConformalConic::lamccinv(double x, double y, double *lon, double *lat)
{

    double rh1;			/* height above ellipsoid	*/
    double con;			/* sign variable		    */
    double ts;			/* small t			        */
    double theta;		/* angle			        */

    x -= false_easting;
    y = rh - y + false_northing;
    if (ns > 0)
    {
        rh1 = sqrt (x * x + y * y);
        con = 1.0;
    }
    else
    {
        rh1 = -sqrt (x * x + y * y);
        con = -1.0;
    }
    theta = 0.0;
    if (rh1 != 0)
        theta = atan2((con * x),(con * y));
    if ((rh1 != 0) || (ns > 0.0))
    {
        con = 1.0/ns;
        ts = pow((rh1/(r_major * f0)),con);
        *lat = phi2z(e,ts);
    }
    else
        *lat = -HALF_PI;
    *lon = adjust_lon_rad(theta/ns + center_lon);
    return(true);
}

double LambertConformalConic::tsfnz(double eccent, double phi, double sinphi)
{
  /* double eccent;	 Eccentricity of the spheroid*/
  /* double phi;	 Latitude phi                */
  /* double sinphi;	 Sine of the latitude        */
  double con;
  double com;

  con = eccent * sinphi;
  com = .5 * eccent;
  con = pow(((1.0 - con) / (1.0 + con)),com);
  return (tan(.5 * (PIO2 - phi))/con);

}

double LambertConformalConic::msfnz (double eccent, double sinphi, double cosphi)
{
    double con;
    con = eccent * sinphi;
    return((cosphi / (sqrt (1.0 - con * con))));
}

/* Function to compute the latitude angle, phi2, for the inverse of the
   Lambert Conformal Conic and Polar Stereographic projections.
----------------------------------------------------------------*/
double LambertConformalConic::phi2z(double eccent, double ts)
/* double eccent    Spheroid eccentricity		*/
/* double ts        Constant value t			*/
/* long *flag       Error flag number			*/
{
    double eccnth;
    double phi;
    double con;
    double dphi;
    double sinpi;
    long i;

    eccnth = .5 * eccent;
    phi = HALF_PI - 2 * atan(ts);
    for (i = 0; i <= 15; i++)
    {
        sinpi = sin(phi);
        con = eccent * sinpi;
        dphi = HALF_PI - 2 * atan(ts *(pow(((1.0 - con)/(1.0 + con)),eccnth))) - phi;
        phi += dphi;
        if (fabs(dphi) <= .0000000001)
            return(phi);
    }
    qDebug() << QString("Convergence error ; phi2z-conv");
    return 0;
}


