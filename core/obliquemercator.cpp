#include <float.h>
#include <math.h>

#include "obliquemercator.h"
#include "globals.h"
#include "options.h"
#include "pixgeoconversion.h"
#include <QtConcurrent/QtConcurrent>
#include <qmessagebox.h>

#include <QDebug>

extern Options opts;
extern SegmentImage *imageptrs;
extern gshhsData *gshhsdata;

ObliqueMercator::ObliqueMercator(QObject *parent, AVHRRSatellite *seglist) :
    QObject(parent)
{
    segs = seglist;
    image_width = 0;
    image_height = 0;
    ellipsoid = true;

    qDebug() << QString("constructor ObliqueMercator");
}

ObliqueMercator::~ObliqueMercator()
{
}

void ObliqueMercator::Initialize(double r_maj, double r_min, eProjectionType projtype, int imgwidth, int imgheight)
{
    if(opts.bellipsoid)
    {
        this->ellipsoid = true;
        InitializeEllipsoid(r_maj, r_min, projtype, imgwidth, imgheight);
    }
    else
    {
        this->ellipsoid = false;
        InitializeSpherical(projtype);
    }
}

void ObliqueMercator::InitializeEllipsoid(double r_maj, double r_min, eProjectionType projtype, int imgwidth, int imgheight)
{
    double temp;			/* temporary variable		*/
    double con,com;
    double ts;
    double ts1,ts2;
    double h,l;
    double j,p,dlon;
    double f,g,gama;
    double sinphi;
    double lon1_r;			/* fist point to define central line in rad	*/
    double lat1_r;			/* fist point to define central line	*/
    double lon2_r;			/* second point to define central line	*/
    double lat2_r;			/* second point to define central line	*/

    double lon1_d;			/* fist point to define central line in deg	*/
    double lat1_d;			/* fist point to define central line	*/
    double lon2_d;			/* second point to define central line	*/
    double lat2_d;			/* second point to define central line	*/

    lon1_d = 65535.0;
    lat1_d = 65535.0;
    lon2_d = 65535.0;
    lat2_d = 65535.0;

    qDebug() << "ObliqueMercator::Initialize input image = " << projtype;

    if(projtype != PROJ_AVHRR && projtype != PROJ_MERSI && projtype != PROJ_VIIRSM )
    {
        QMessageBox msgBox;
        msgBox.setText("Only possible for MERSI and VIIRS M projections ! (Ellipsoid)");
        msgBox.exec();
        return;
    }

    if(projtype == PROJ_VIIRSM)
    {
        if(opts.buttonVIIRSM)
            segs->seglviirsm->GetCentralCoords(&lon1_d, &lat1_d, &lon2_d, &lat2_d);
        else if(opts.buttonVIIRSMNOAA20)
            segs->seglviirsmnoaa20->GetCentralCoords(&lon1_d, &lat1_d, &lon2_d, &lat2_d);
    }
    else if(projtype == PROJ_MERSI)
    {
        segs->seglmersi->GetCentralCoords(&lon1_d, &lat1_d, &lon2_d, &lat2_d);
    }
    else if(projtype == PROJ_AVHRR)
    {
        if(opts.buttonMetop)
            segs->seglmetop->GetCentralCoords(&lon1_d, &lat1_d, &lon2_d, &lat2_d);
    }

    qDebug() << "lon1_d = " << lon1_d << " lat1_d = " << lat1_d << " lon2_d = " << lon2_d << " lat2_d = " << lat2_d;
    lon1_r = deg2rad(lon1_d);
    lat1_r = deg2rad(lat1_d);
    lon2_r = deg2rad(lon2_d);
    lat2_r = deg2rad(lat2_d);

    double lat_origin_deg = (lat2_d - lat1_d)/2 + lat1_d;
//    double lat_origin_deg = lat2_d;
    lat_origin = deg2rad(lat_origin_deg);
    qDebug() << "latitude origin = " << lat_origin_deg;

    ///////////////////////////////////////
    double P1 = cos(lat1_r)*sin(lat2_r)*cos(lon1_r) - sin(lat1_r)*cos(lat2_r)*cos(lon2_r);
    double P2 = sin(lat1_r)*cos(lat2_r)*sin(lon2_r) - cos(lat1_r)*cos(lat2_r)*sin(lon1_r);
    double lon_p1 = atan(P1/P2);
    double lat_p1 = atan( - cos(lon_p1 - lon1_r)/tan(lat1_r));

    qDebug() << "Pole 1 = (" << rad2deg(lat_p1) << ", " << rad2deg(lon_p1) << ")  Pole 2 = (" << rad2deg(-lat_p1) << ", " << rad2deg(lon_p1 + PI) << ")";
    qDebug() << "Pole 1 = (" << rad2deg(lat_p1) << ", " << rad2deg(lon_p1) << ")  Pole 2 = (" << rad2deg(-lat_p1) << ", " << rad2deg(lon_p1 - PI) << ")";

    if(lat_p1 < 0)
    {
        lat_p1 = - lat_p1;
        if(lon_p1 + PI > PI)
            lon_p1 = lon_p1 - PI;
        else
            lon_p1 = lon_p1 + PI;
    }

    qDebug() << "Pole = (" << rad2deg(lat_p1) << ", " << rad2deg(lon_p1) << ")";

    ////////////////////////////////////////////////////////////

    if(projtype == PROJ_VIIRSM)
    {
        image_width = imgwidth; //imageptrs->ptrimageViirsM->width();
        image_height = imgheight; //imageptrs->ptrimageViirsM->height();

    }
    else if(projtype == PROJ_MERSI)
    {
        image_width = imgwidth; //imageptrs->ptrimageMERSI->width();
        image_height = imgheight; //imageptrs->ptrimageMERSI->height();

    }
    else if(projtype == PROJ_AVHRR)
    {
        if(opts.buttonMetop)
        {
            image_width = imgwidth; //imageptrs->ptrimagecomp_ch[0]->width();
            image_height = imgheight; //imageptrs->ptrimagecomp_ch[0]->height();
        }
    }

    QPoint imagepoint(image_width, image_height);
    emit aspectratioChanged(imagepoint);

    if (image_width != imageptrs->ptrimageProjection->width() || image_height != imageptrs->ptrimageProjection->height())
    {
        delete imageptrs->ptrimageProjection;
        imageptrs->ptrimageProjection = new QImage(image_width, image_height, QImage::Format_ARGB32);
        imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    }

    imageptrs->ptrimageProjectionRed.reset(new quint16[image_width * image_height]);
    imageptrs->ptrimageProjectionGreen.reset(new quint16[image_width * image_height]);
    imageptrs->ptrimageProjectionBlue.reset(new quint16[image_width * image_height]);
    imageptrs->ptrimageProjectionAlpha.reset(new quint16[image_width * image_height]);

//    for(int i = 0; i < image_width * image_height; i++)
//    {
//        imageptrs->ptrimageProjectionRed[i] = 0;
//        imageptrs->ptrimageProjectionGreen[i] = 0;
//        imageptrs->ptrimageProjectionBlue[i] = 0;
//        imageptrs->ptrimageProjectionAlpha[i] = 0;
//    }

    r_major = r_maj;
    r_minor = r_min;
    scale_factor = 1;

    false_northing = 0;
    false_easting = 0;

    temp = r_minor / r_major;
    es = 1.0 - temp*temp;
    e = sqrt(es); // eccentricity e^2 = es

#ifdef WIN32 //&& __GNUC__
    sin_p20 = sin(lat_origin);
    cos_p20 = cos(lat_origin);
#else
    sincos(lat_origin, &sin_p20,&cos_p20);
#endif

    con = 1.0 - es * sin_p20 * sin_p20;
    com = sqrt(1.0 - es);
    bl = sqrt(1.0 + es * pow(cos_p20,4.0)/(1.0 - es));
    al = r_major * bl * scale_factor * com / con;
    if (fabs(lat_origin) < EPSLN)
    {
        ts = 1.0;
        d = 1.0;
        el = 1.0;
    }
    else
    {
        ts = tsfnz(e,lat_origin,sin_p20);
        con = sqrt(con);
        d = bl * com / (cos_p20 * con);
        if ((d * d - 1.0) > 0.0)
        {
            if (lat_origin >= 0.0)
                f = d + sqrt(d * d - 1.0);
            else
                f = d - sqrt(d * d - 1.0);
        }
        else
            f = d;
        el = f * pow(ts,bl);
    }

    sinphi = sin(lat1_r);
    ts1 = tsfnz(e,lat1_r,sinphi);
    sinphi = sin(lat2_r);
    ts2 = tsfnz(e,lat2_r,sinphi);
    h = pow(ts1,bl);
    l = pow(ts2,bl);
    f = el/h;
    g = .5 * (f - 1.0/f);
    j = (el * el - l * h)/(el * el + l * h);
    p = (l - h) / (l + h);
    dlon = lon1_r - lon2_r;
    if (dlon < -PI)
        lon2_r = lon2_r - 2.0 * PI;
    if (dlon > PI)
        lon2_r = lon2_r + 2.0 * PI;
    dlon = lon1_r - lon2_r;
    lon_origin = .5 * (lon1_r + lon2_r) - atan(j * tan(.5 * bl * dlon)/p)/bl;
    dlon  = adjust_lon_rad(lon1_r - lon_origin);
    gama = atan(sin(bl * dlon)/g);
    azimuth = asinz(d * sin(gama));

    qDebug() << "Azimuth = " << rad2deg(azimuth);

    if (fabs(lat1_r - lat2_r) <= EPSLN)
    {
        qDebug() << "Input data error","omer-init";
        return;
    }
    else
        con = fabs(lat1_r);
    if ((con <= EPSLN) || (fabs(con - HALF_PI) <= EPSLN))
    {
        qDebug() << "Input data error","omer-init";
        return;
    }
    else if (fabs(fabs(lat_origin) - HALF_PI) <= EPSLN)
    {
        qDebug() << "Input data error","omer-init";
        return;
    }

    sincos(gama,&singam,&cosgam);
    sincos(azimuth,&sinaz,&cosaz);
    if (lat_origin >= 0)
        u =  (al/bl) * atan(sqrt(d * d - 1.0)/cosaz);
    else
        u = -(al/bl) * atan(sqrt(d * d - 1.0)/cosaz);


    ////////////////////////////////////////////

    double map_x_1, map_y_1;
    double map_x_2, map_y_2;

    omerfor(lon1_r, lat1_r, &map_x_1, &map_y_1);
    qDebug() << "For central 1 : map_x_1 =" << map_x_1 << " map_y_1 = " << map_y_1;
    omerfor(lon2_r, lat2_r, &map_x_2, &map_y_2);
    qDebug() << "For central 2 : map_x_2 =" << map_x_2 << " map_y_2 = " << map_y_2;


    if(projtype == PROJ_VIIRSM)
    {
        if(opts.buttonVIIRSM)
            GetMinMaxXBoundingBox(SEG_VIIRSM, &boundingbox_min_x, &boundingbox_max_x, &boundingbox_min_y, &boundingbox_max_y);
        else if(opts.buttonVIIRSMNOAA20)
            GetMinMaxXBoundingBox(SEG_VIIRSMNOAA20, &boundingbox_min_x, &boundingbox_max_x, &boundingbox_min_y, &boundingbox_max_y);
    } else if(projtype == PROJ_MERSI)
    {
        GetMinMaxXBoundingBox(SEG_MERSI, &boundingbox_min_x, &boundingbox_max_x, &boundingbox_min_y, &boundingbox_max_y);
    }
    else if(projtype == PROJ_AVHRR)
    {
        if(opts.buttonMetop)
            GetMinMaxXBoundingBoxAVHRR(SEG_METOP, &boundingbox_min_x, &boundingbox_max_x, &boundingbox_min_y, &boundingbox_max_y);
    }


    qDebug() << "bounding box max_x = " << boundingbox_max_x << " min_x = " << boundingbox_min_x;
    qDebug() << "bounding box max_y = " << boundingbox_max_y << " min_y = " << boundingbox_min_y;

    map_extend_x = abs(boundingbox_max_x - boundingbox_min_x);
    map_extend_y = abs(boundingbox_max_y - boundingbox_min_y);

    qDebug() << "map_extend_x = " << map_extend_x;
    qDebug() << "map_extend_y = " << map_extend_y;

}

void ObliqueMercator::InitializeSpherical(eProjectionType projtype)
{

    double temp;			/* temporary variable		*/
    double con,com;
    double ts;
    double ts1,ts2;
    double h,l;
    double j,p,dlon;
    double f,g,gama;
    double sinphi;
    double lon1_r;			/* fist point to define central line in rad	*/
    double lat1_r;			/* fist point to define central line	*/
    double lon2_r;			/* second point to define central line	*/
    double lat2_r;			/* second point to define central line	*/

    double lon1_d;			/* fist point to define central line in deg	*/
    double lat1_d;			/* fist point to define central line	*/
    double lon2_d;			/* second point to define central line	*/
    double lat2_d;			/* second point to define central line	*/

    lon1_d = 65535.0;
    lat1_d = 65535.0;
    lon2_d = 65535.0;
    lat2_d = 65535.0;

    qDebug() << "ObliqueMercator::Initialize input image = " << projtype;

    if(projtype != PROJ_AVHRR && projtype != PROJ_MERSI && projtype != PROJ_VIIRSM )
    {
        QMessageBox msgBox;
        msgBox.setText("Only possible for MERSI and VIIRS M projections ! (Spherical)");
        msgBox.exec();
        return;
    }

    if(projtype == PROJ_VIIRSM)
    {
        if(opts.buttonVIIRSM)
            segs->seglviirsm->GetCentralCoords(&lon1_d, &lat1_d, &lon2_d, &lat2_d);
        else if(opts.buttonVIIRSMNOAA20)
            segs->seglviirsmnoaa20->GetCentralCoords(&lon1_d, &lat1_d, &lon2_d, &lat2_d);
    }
    else if(projtype == PROJ_MERSI)
    {
        segs->seglmersi->GetCentralCoords(&lon1_d, &lat1_d, &lon2_d, &lat2_d);
    }
    else if(projtype == PROJ_AVHRR)
    {
        if(opts.buttonMetop)
            segs->seglmetop->GetCentralCoords(&lon1_d, &lat1_d, &lon2_d, &lat2_d);
    }

    qDebug() << "lon1_d = " << lon1_d << " lat1_d = " << lat1_d << " lon2_d = " << lon2_d << " lat2_d = " << lat2_d;
    lon1_r = deg2rad(lon1_d);
    lat1_r = deg2rad(lat1_d);
    lon2_r = deg2rad(lon2_d);
    lat2_r = deg2rad(lat2_d);

    double lat_origin_deg = (lat2_d - lat1_d)/2 + lat1_d;
    lat_origin = deg2rad(lat2_d);
    qDebug() << "latitude origin = " << lat_origin_deg;

    double P1 = cos(lat1_r)*sin(lat2_r)*cos(lon1_r) - sin(lat1_r)*cos(lat2_r)*cos(lon2_r);
    double P2 = sin(lat1_r)*cos(lat2_r)*sin(lon2_r) - cos(lat1_r)*cos(lat2_r)*sin(lon1_r);

    qDebug() << "P1 = " << P1 << " P2 = " << P2;

    lon_p1 = atan(P1/P2);
    lat_p1 = atan( - cos(lon_p1 - lon1_r)/tan(lat1_r));

    if(lat_p1 < 0)
    {
        lat_p1 = - lat_p1;
        if(lon_p1 + PI > PI)
            lon_p1 = lon_p1 - PI;
        else
            lon_p1 = lon_p1 + PI;
    }

    qDebug() << "Pole = (" << rad2deg(lat_p1) << ", " << rad2deg(lon_p1) << ")";

    sinazimuth = sin(PIO2 - lat_p1);
    cosazimuth = cos(PIO2 - lat_p1);

    lon_ori = lon_p1 + PIO2;


    if(projtype == PROJ_VIIRSM)
    {
        image_width = imageptrs->ptrimageViirsM->width();
        image_height = imageptrs->ptrimageViirsM->height();
    }
    else if(projtype == PROJ_MERSI)
    {
        image_width = imageptrs->ptrimageMERSI->width();
        image_height = imageptrs->ptrimageMERSI->height();

    }
    else if(projtype == PROJ_AVHRR)
    {
        if(opts.buttonMetop)
        {
            image_width = imageptrs->ptrimagecomp_ch[0]->width();
            image_height = imageptrs->ptrimagecomp_ch[0]->height();
        }
    }

    if (image_width != imageptrs->ptrimageProjection->width() || image_height != imageptrs->ptrimageProjection->height())
    {
        delete imageptrs->ptrimageProjection;
        imageptrs->ptrimageProjection = new QImage(image_width, image_height, QImage::Format_ARGB32);
        imageptrs->ptrimageProjection->fill(qRgba(0, 0, 0, 250));
    }

    imageptrs->ptrimageProjectionRed.reset(new quint16[image_width * image_height]);
    imageptrs->ptrimageProjectionGreen.reset(new quint16[image_width * image_height]);
    imageptrs->ptrimageProjectionBlue.reset(new quint16[image_width * image_height]);
    imageptrs->ptrimageProjectionAlpha.reset(new quint16[image_width * image_height]);

    for(int i = 0; i < image_width * image_height; i++)
    {
        imageptrs->ptrimageProjectionRed[i] = 0;
        imageptrs->ptrimageProjectionGreen[i] = 0;
        imageptrs->ptrimageProjectionBlue[i] = 0;
        imageptrs->ptrimageProjectionAlpha[i] = 0;
    }



    ////////////////////////////////////////////

    double map_x_1, map_y_1;
    double map_x_2, map_y_2;

    omerforspherical(lon1_r, lat1_r, &map_x_1, &map_y_1);
    qDebug() << "For central 1 : map_x_1 =" << map_x_1 << " map_y_1 = " << map_y_1;
    omerforspherical(lon2_r, lat2_r, &map_x_2, &map_y_2);
    qDebug() << "For central 2 : map_x_2 =" << map_x_2 << " map_y_2 = " << map_y_2;


    if(projtype == PROJ_VIIRSM)
    {
        if(opts.buttonVIIRSM)
            GetMinMaxXBoundingBox(SEG_VIIRSM, &boundingbox_min_x, &boundingbox_max_x, &boundingbox_min_y, &boundingbox_max_y);
        else if(opts.buttonVIIRSMNOAA20)
            GetMinMaxXBoundingBox(SEG_VIIRSMNOAA20, &boundingbox_min_x, &boundingbox_max_x, &boundingbox_min_y, &boundingbox_max_y);
    } else if(projtype == PROJ_MERSI)
    {
        GetMinMaxXBoundingBox(SEG_MERSI, &boundingbox_min_x, &boundingbox_max_x, &boundingbox_min_y, &boundingbox_max_y);
    }
    else if(projtype == PROJ_AVHRR)
    {
        if(opts.buttonMetop)
            GetMinMaxXBoundingBoxAVHRRSpherical(SEG_METOP, &boundingbox_min_x, &boundingbox_max_x, &boundingbox_min_y, &boundingbox_max_y);
    }


    qDebug() << "bounding box max_x = " << boundingbox_max_x << " min_x = " << boundingbox_min_x;
    qDebug() << "bounding box max_y = " << boundingbox_max_y << " min_y = " << boundingbox_min_y;

    map_extend_x = abs(boundingbox_max_x - boundingbox_min_x);
    map_extend_y = abs(boundingbox_max_y - boundingbox_min_y);

    qDebug() << "map_extend_x = " << map_extend_x;
    qDebug() << "map_extend_y = " << map_extend_y;

}

void ObliqueMercator::GetMinMaxXBoundingBox(eSegmentType type, double *boundingbox_min_x, double *boundingbox_max_x, double *boundingbox_min_y, double *boundingbox_max_y)
{
    SegmentList *seglist;
    double min_x, max_x;
    double min_y, max_y;
    double map_x, map_y;
    float geolon_d, geolat_d;
    float geolon_r, geolat_r;
    bool ret;

    min_x = DBL_MAX;
    max_x = -DBL_MAX;
    min_y = DBL_MAX;
    max_y = -DBL_MAX;

    if(type == SEG_VIIRSM)
    {
        seglist = (SegmentList *)segs->seglviirsm;
    }
    else if( type == SEG_VIIRSMNOAA20)
    {
        seglist = (SegmentList *)segs->seglviirsmnoaa20;
    }
    else if( type == SEG_MERSI)
    {
        seglist = (SegmentList *)segs->seglmersi;
    }
    else
        seglist = NULL;

    QList<Segment *>::iterator segsel;
    segsel = seglist->GetSegsSelectedptr()->begin();
    while ( segsel != seglist->GetSegsSelectedptr()->end() )
    {
        Segment *segm = (Segment *)(*segsel);
        for(int i = 0; i < segm->NbrOfLines; i++)
        {
            for(int j = 0; j < segm->earth_views_per_scanline; j++)
            {
                geolon_d = segm->geolongitude[i * segm->earth_views_per_scanline + j];
                geolat_d = segm->geolatitude[i * segm->earth_views_per_scanline + j];
                if(abs(geolon_d) <= 180.0 && abs(geolat_d) <= 90.0)
                {
                    geolon_r = deg2rad(geolon_d);
                    geolat_r = deg2rad(geolat_d);
                    ret = omerfor(geolon_r, geolat_r, &map_x, &map_y);

                    if(ret)
                    {
                        if(map_x < min_x)
                            min_x = map_x;
                        if(map_x > max_x)
                            max_x = map_x;
                        if(map_y < min_y)
                            min_y = map_y;
                        if(map_y > max_y)
                            max_y = map_y;
                    }
                    break;
                }
            }
            for(int j = segm->earth_views_per_scanline - 1; j >= 0; j--)
            {
                geolon_d = segm->geolongitude[i * segm->earth_views_per_scanline + j];
                geolat_d = segm->geolatitude[i * segm->earth_views_per_scanline + j];
                if(abs(geolon_d) <= 180.0 && abs(geolat_d) <= 90.0)
                {
                    geolon_r = deg2rad(geolon_d);
                    geolat_r = deg2rad(geolat_d);
                    ret = omerfor(geolon_r, geolat_r, &map_x, &map_y);

                    if(ret)
                    {
                        if(map_x < min_x)
                            min_x = map_x;
                        if(map_x > max_x)
                            max_x = map_x;
                        if(map_y < min_y)
                            min_y = map_y;
                        if(map_y > max_y)
                            max_y = map_y;

                    }
                    break;
                }
            }

        }

        ++segsel;
    }

    *boundingbox_min_x = min_x;
    *boundingbox_max_x = max_x;
    *boundingbox_min_y = min_y;
    *boundingbox_max_y = max_y;

}

void ObliqueMercator::GetMinMaxXBoundingBoxAVHRR(eSegmentType type, double *boundingbox_min_x, double *boundingbox_max_x, double *boundingbox_min_y, double *boundingbox_max_y)
{

    SegmentList *seglist;
    double min_x, max_x;
    double min_y, max_y;
    double map_x, map_y;
    float geolon_d, geolat_d;
    float geolon_r, geolat_r;
    bool ret;

    min_x = DBL_MAX;
    max_x = -DBL_MAX;
    min_y = DBL_MAX;
    max_y = -DBL_MAX;

    if( type == SEG_METOP)
    {
        seglist = (SegmentList *)segs->seglmetop;
    }
    else
        seglist = NULL;

    QList<Segment *>::iterator segsel;
    segsel = seglist->GetSegsSelectedptr()->begin();
    while ( segsel != seglist->GetSegsSelectedptr()->end() )
    {
        Segment *segm = (Segment *)(*segsel);
        for(int i = 0; i < segm->NbrOfLines; i++)
        {
            for(int j = 0; j < 103; j++)
            {
                geolon_d = segm->earthloc_lon[i * 103 + j];
                geolat_d = segm->earthloc_lat[i * 103 + j];
                if(abs(geolon_d) <= 180.0 && abs(geolat_d) <= 90.0)
                {
                    geolon_r = deg2rad(geolon_d);
                    geolat_r = deg2rad(geolat_d);
                    ret = omerfor(geolon_r, geolat_r, &map_x, &map_y);

                    if(ret)
                    {
                        if(map_x < min_x)
                            min_x = map_x;
                        if(map_x > max_x)
                            max_x = map_x;
                        if(map_y < min_y)
                            min_y = map_y;
                        if(map_y > max_y)
                            max_y = map_y;
                    }
                    break;
                }
            }
            for(int j = 103 - 1; j >= 0; j--)
            {
                geolon_d = segm->earthloc_lon[i * 103 + j];
                geolat_d = segm->earthloc_lat[i * 103 + j];
                if(abs(geolon_d) <= 180.0 && abs(geolat_d) <= 90.0)
                {
                    geolon_r = deg2rad(geolon_d);
                    geolat_r = deg2rad(geolat_d);
                    ret = omerfor(geolon_r, geolat_r, &map_x, &map_y);

                    if(ret)
                    {
                        if(map_x < min_x)
                            min_x = map_x;
                        if(map_x > max_x)
                            max_x = map_x;
                        if(map_y < min_y)
                            min_y = map_y;
                        if(map_y > max_y)
                            max_y = map_y;

                    }
                    break;
                }
            }

        }

        ++segsel;
    }



    *boundingbox_min_x = min_x;
    *boundingbox_max_x = max_x;
    *boundingbox_min_y = min_y;
    *boundingbox_max_y = max_y;

}

void ObliqueMercator::GetMinMaxXBoundingBoxAVHRRSpherical(eSegmentType type, double *boundingbox_min_x, double *boundingbox_max_x, double *boundingbox_min_y, double *boundingbox_max_y)
{

    SegmentList *seglist;
    double min_x, max_x;
    double min_y, max_y;
    double map_x, map_y;
    float geolon_d, geolat_d;
    float geolon_r, geolat_r;
    bool ret;

    min_x = DBL_MAX;
    max_x = -DBL_MAX;
    min_y = DBL_MAX;
    max_y = -DBL_MAX;

    if( type == SEG_METOP)
    {
        seglist = (SegmentList *)segs->seglmetop;
    }
    else
        seglist = NULL;

    QList<Segment *>::iterator segsel;
    segsel = seglist->GetSegsSelectedptr()->begin();
    while ( segsel != seglist->GetSegsSelectedptr()->end() )
    {
        Segment *segm = (Segment *)(*segsel);
        for(int i = 0; i < segm->NbrOfLines; i++)
        {
            for(int j = 0; j < 103; j++)
            {
                geolon_d = segm->earthloc_lon[i * 103 + j];
                geolat_d = segm->earthloc_lat[i * 103 + j];
                if(abs(geolon_d) <= 180.0 && abs(geolat_d) <= 90.0)
                {
                    geolon_r = deg2rad(geolon_d);
                    geolat_r = deg2rad(geolat_d);
                    ret = omerforspherical(geolon_r, geolat_r, &map_x, &map_y);

                    if(ret)
                    {
                        if(map_x < min_x)
                            min_x = map_x;
                        if(map_x > max_x)
                            max_x = map_x;
                        if(map_y < min_y)
                            min_y = map_y;
                        if(map_y > max_y)
                            max_y = map_y;
                    }
                    break;
                }
            }
            for(int j = 103 - 1; j >= 0; j--)
            {
                geolon_d = segm->earthloc_lon[i * 103 + j];
                geolat_d = segm->earthloc_lat[i * 103 + j];
                if(abs(geolon_d) <= 180.0 && abs(geolat_d) <= 90.0)
                {
                    geolon_r = deg2rad(geolon_d);
                    geolat_r = deg2rad(geolat_d);
                    ret = omerforspherical(geolon_r, geolat_r, &map_x, &map_y);

                    if(ret)
                    {
                        if(map_x < min_x)
                            min_x = map_x;
                        if(map_x > max_x)
                            max_x = map_x;
                        if(map_y < min_y)
                            min_y = map_y;
                        if(map_y > max_y)
                            max_y = map_y;

                    }
                    break;
                }
            }

        }

        ++segsel;
    }



    *boundingbox_min_x = min_x;
    *boundingbox_max_x = max_x;
    *boundingbox_min_y = min_y;
    *boundingbox_max_y = max_y;

}

void ObliqueMercator::CreateMapFromAVHRR(eSegmentType type, int inputchannel)
{

    if( type == SEG_METOP)
        segs->seglmetop->ComposeOMProjection(inputchannel);


    if(opts.smoothprojectiontype == 1)
        imageptrs->SmoothProjectionImage();
    else if(opts.smoothprojectiontype == 2)
    {
        if( type == SEG_METOP)
            segs->seglmetop->SmoothProjectionImageBilinear();
    }
}

void ObliqueMercator::CreateMapFromVIIRS(eSegmentType type, bool combine)
{
    if (type == SEG_VIIRSM)
        segs->seglviirsm->ComposeOMProjection(0);
    else if( type == SEG_VIIRSMNOAA20)
        segs->seglviirsmnoaa20->ComposeOMProjection(0);

    if(opts.smoothprojectiontype == 1)
        imageptrs->SmoothProjectionImage();
    else if(opts.smoothprojectiontype == 2)
    {
        if (type == SEG_VIIRSM)
        {
            segs->seglviirsm->SmoothVIIRSImage(combine);
        }
        else if (type == SEG_VIIRSMNOAA20)
        {
            segs->seglviirsmnoaa20->SmoothVIIRSImage(combine);
        }
    }

}

void ObliqueMercator::CreateMapFromMERSI(eSegmentType type, bool combine)
{

    segs->seglmersi->ComposeOMProjection(0);

    if(opts.smoothprojectiontype == 1)
        imageptrs->SmoothProjectionImage();
    else if(opts.smoothprojectiontype == 2)
    {
        segs->seglmersi->SmoothMERSIImage(combine);
    }

}

bool ObliqueMercator::omerfor(double lon, double lat, double *x, double *y)
/* (I) Longitude 		*/
/* (I) Latitude 		*/
/* (O) X projection coordinate 	*/
/* (O) Y projection coordinate 	*/
{
    double sin_phi;
    double t;
    double con;
    double q,us,vl;
    double ul,vs;
    double s;
    double dlon;
    double ts1;

    /* Forward equations
  -----------------*/
    sin_phi = sin(lat);
    dlon = adjust_lon_rad(lon - lon_origin);
    vl = sin(bl * dlon);
    if (fabs(fabs(lat) - HALF_PI) > EPSLN)
    {
        ts1 = tsfnz(e,lat,sin_phi);
        q = el / (pow(ts1,bl));
        s = .5 * (q - 1.0 / q);
        t = .5 * (q + 1.0/ q);
        ul = (s * singam - vl * cosgam) / t;
        con = cos(bl * dlon);
        if (fabs(con) < .0000001)
        {
            us = al * bl * dlon;
        }
        else
        {
            us = al * atan((s * cosgam + vl * singam) / con)/bl;
            if (con < 0)
                us = us + PI * al / bl;
        }
    }
    else
    {
        if (lat >= 0)
            ul = singam;
        else
            ul = -singam;
        us = al * lat / bl;
    }
    if (fabs(fabs(ul) - 1.0) <= EPSLN)
    {
        //   qDebug() << "Point projects into infinity ; omer-for";
        return false;
    }
    vs = .5 * al * log((1.0 - ul)/(1.0 + ul)) / bl;
    us = us - u;

    //2D Rotation over angle = azimuth
    //    *x = false_easting + vs * cosaz + us * sinaz;
    //    *y = false_northing + us * cosaz - vs * sinaz;

    double xx = false_easting + vs * cosaz + us * sinaz;
    double yy = false_northing + us * cosaz - vs * sinaz;

    *x = xx * cosaz - yy * sinaz;
    *y = xx * sinaz + yy * cosaz;

    return true;
}

bool ObliqueMercator::omerforspherical(double lon, double lat, double *x, double *y)
/* (I) Longitude 		*/
/* (I) Latitude 		*/
/* (O) X projection coordinate 	*/
/* (O) Y projection coordinate 	*/
{


    double K = tan(lat)*cos(lat_p1) + sin(lat_p1)*sin(lon - lon_ori);
    double L = cos(lon - lon_ori);

    double xx = atan(K/L);

    if(L < 0)
        xx = xx - PI;
    double A = sin(lat_p1)*sin(lat) - cos(lat_p1)*cos(lat)*sin(lon - lon_ori);
    double yy = atanh(A);

    *x = xx * cosazimuth - yy * sinazimuth;
    *y = xx * sinazimuth + yy * cosazimuth;

    *x = xx;
    *y = yy;

    return true;
}

bool ObliqueMercator::map_forward(double lon_rad, double lat_rad, double &map_x, double &map_y)
{


    double x, y;
    double x1, y1;
    bool ret;

    double lon_d = rad2deg(lon_rad);
    double lat_d = rad2deg(lat_rad);

    if(ellipsoid)
        ret = this->omerfor(lon_rad, lat_rad, &x, &y);
    else
        ret = this->omerforspherical(lon_rad, lat_rad, &x, &y);

    double scale_x, scale_y;
    double map_x1, map_y1;
    scale_x = 1;
    scale_y = 1;

    if(ret)
    {

//        map_x = image_width * (x + boundingbox_min_x)/map_extend_x;
//        map_y = image_height - image_height * (y + boundingbox_min_y)/map_extend_y;
//        if(map_x < 0 || map_x > image_width || map_y < 0 || map_y > image_height)
//            ret = false;
        map_x = image_width * (x - boundingbox_min_x)/map_extend_x;
        map_y = image_height - image_height * (y - boundingbox_min_y)/map_extend_y;
        if(map_x < 0 || map_x > image_width || map_y < 0 || map_y > image_height)
            ret = false;


    }

    return ret;
}




double ObliqueMercator::tsfnz(double eccent, double phi, double sinphi)
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

bool ObliqueMercator::map_inverse(double map_x, double map_y, double &lon_rad, double &lat_rad)
{
    //    double x, y;


    //    y = map_radius * scale * (- 2*map_y + 2*(mapdeltay+false_northing) + map_height) / map_height;
    //    x = map_radius * scale * (2*map_x - 2*(mapdeltax+false_easting) - map_width) / map_width;

    //    //qDebug() << QString("x=%1 y=%2 map_radius=%3 map_height=%4 map_width=%5").arg(x).arg(y).arg(map_radius).arg(map_height).arg(map_width);

    //    bool ret = this->inverse(x, y, lon_rad, lat_rad);
    //    return ret;

    return false;
}

