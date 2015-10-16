#include "segmentnoaa.h"
#include "segmentlistnoaa.h"
#include "segmentimage.h"

#include "qtle.h"
#include "qsgp4.h"
#include "qsgp4date.h"
#include "qeci.h"
#include "qtopocentric.h"
#include "qgeodetic.h"
#include "qobserver.h"
#include "qsgp4utilities.h"


#include <QDebug>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QString>
#include <QColor>
#include <QPen>
#include <QPixmap>
#include <QDate>

#define NEW_NOAA_SWATH 55.37 * PI / 180.0
//#define FOV_STEP_ANGLE 0.0541 * PI / 180.0
#define FOV_STEP_ANGLE 0.0505 * PI / 180.0

extern Options opts;
extern SegmentImage *imageptrs;

SegmentNoaa::SegmentNoaa(QFile *filesegment, SatelliteList *satl, QObject *parent) :
    Segment(parent)
{
    bool ok;
    satlist = satl;
    fileInfo.setFile(*filesegment);
    segment_type = "Noaa";

    int sensing_start_year = fileInfo.fileName().mid(6, 4).toInt( &ok , 10);
    int sensing_start_month = fileInfo.fileName().mid(10, 2).toInt( &ok, 10);
    int sensing_start_day = fileInfo.fileName().mid(12, 2).toInt( &ok, 10);
    int sensing_start_hour = fileInfo.fileName().mid(15, 2).toInt( &ok, 10);
    int sensing_start_minute = fileInfo.fileName().mid(17, 2).toInt( &ok, 10);
    int sensing_start_second = fileInfo.fileName().mid(19, 2).toInt( &ok, 10);
    //this->sensing_start_year = sensing_start_year;
    qdatetime_start.setDate(QDate(sensing_start_year, sensing_start_month, sensing_start_day));
    qdatetime_start.setTime(QTime(sensing_start_hour,sensing_start_minute, sensing_start_second));

    qsensingstart = QSgp4Date(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, sensing_start_second);
    qsensingend = qsensingstart;
    qsensingend.AddMin(1.0);

    julian_sensing_start = qsensingstart.Julian();
    julian_sensing_end = qsensingend.Julian();

    this->earth_views_per_scanline = 2048;

    Satellite noaa19;
    ok = satlist->GetSatellite(33591, &noaa19);
    line1 = noaa19.line1;
    line2 = noaa19.line2;

    //line1 = "1 33591U 09005A   11039.40718334  .00000086  00000-0  72163-4 0  8568";
    //line2 = "2 33591  98.8157 341.8086 0013952 344.4168  15.6572 14.11126791103228";

    qtle = new QTle(noaa19.sat_name, line1, line2, QTle::wgs72);
    qsgp4 = new QSgp4( *qtle );

    julian_state_vector = qtle->Epoch();

    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MIN_PER_DAY; //  + (1.0/12.0) / 60.0;
    minutes_sensing = 1;

    QEci eci;

    qsgp4->getPosition(minutes_since_state_vector, eci);
    QGeodetic geo = eci.ToGeo();

    lon_start_rad = geo.longitude;
    lat_start_rad = geo.latitude;
    lon_start_deg = lon_start_rad * 180.0 / PI;
    lat_start_deg = lat_start_rad * 180.0 /PI;

    NbrOfLines = 360;

    CalculateCornerPoints();

}

int SegmentNoaa::ReadNbrOfLines()
{
    FILE*   f;
    BZFILE* b;
    int     nBuf;
    char    buf[ 11090 * 2 ];
    int     bzerror;

    quint16 val1_ch[5], val2_ch[5],tot_ch[5];
    QByteArray picture_line;

    int heightinsegment = 0;


    f = fopen ( this->fileInfo.absoluteFilePath().toLatin1(), "rb" );
    if ( !f ) {
        qDebug() << QString("file %1 not found ! ").arg(this->fileInfo.absoluteFilePath());
        return 0;
    }

    if((b = BZ2_bzopen(this->fileInfo.absoluteFilePath().toLatin1(),"rb"))==NULL)
    {
        qDebug() << "error in BZ2_bzopen";
    }

    bzerror = BZ_OK;
    while ( bzerror == BZ_OK )
    {
      nBuf = BZ2_bzRead ( &bzerror, b, buf, 11090 * 2 );
      if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
      {
          QByteArray data = QByteArray::fromRawData(buf, sizeof(buf));
          //picture_line = data.mid( 1500, 20480 );

          if ((data.at(0) & 0xFF) == 0x02 && (data.at(1) & 0xFF) == 0x84
              && (data.at(2) & 0xFF) == 0x01 && (data.at(3) & 0xFF) == 0x6f
              && (data.at(4) & 0xFF) == 0x03 && (data.at(5) & 0xFF) == 0x5c
              && (data.at(6) & 0xFF) == 0x01 && (data.at(7) & 0xFF) == 0x9d
              && (data.at(8) & 0xFF) == 0x02 && (data.at(9) & 0xFF) == 0x0f
              && (data.at(10) & 0xFF) == 0x00 && (data.at(11) & 0xFF) == 0x95)
          {
              heightinsegment++;
          }
      }
    }

    BZ2_bzclose ( b );
    fclose(f);

    return heightinsegment;

}

void SegmentNoaa::initializeProjectionCoord()
{
    projectionCoordX = new int[360 * 2048];
    projectionCoordY = new int[360 * 2048];
    projectionCoordValue = new QRgb[360 * 2048];

    for( int i = 0; i < 360; i++)
    {
        for( int j = 0; j < 2048 ; j++ )
        {
            projectionCoordX[i * 2048 + j] = 65535;
            projectionCoordY[i * 2048 + j] = 65535;
            projectionCoordValue[i * 2048 + j] = qRgba(0, 0, 0, 0);
        }
    }

}

Segment *SegmentNoaa::ReadSegmentInMemory()
{
    FILE*   f;
    BZFILE* b;
    int     nBuf;
    char    buf[ 11090 * 2 ];
    int     bzerror;

    quint16 val1_ch[5], val2_ch[5],tot_ch[5];
    QByteArray picture_line;

    int heightinsegment = 0;

    f = fopen ( this->fileInfo.absoluteFilePath().toLatin1(), "rb" );
    if ( !f ) {
        qDebug() << QString("file %1 not found ! ").arg(this->fileInfo.absoluteFilePath());
        return this;
    }

    qDebug() << "Bz2 file " + this->fileInfo.absoluteFilePath() + " is open";

    if((b = BZ2_bzopen(this->fileInfo.absoluteFilePath().toLatin1(),"rb"))==NULL)
    {
        qDebug() << "error in BZ2_bzopen";
    }

    bzerror = BZ_OK;
    while ( bzerror == BZ_OK )
    {
      nBuf = BZ2_bzRead ( &bzerror, b, buf, 11090 * 2 );
      if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
      {
          QByteArray data = QByteArray::fromRawData(buf, sizeof(buf));
          picture_line = data.mid( 1500, 20480 );

          if ((data.at(0) & 0xFF) == 0x02 && (data.at(1) & 0xFF) == 0x84
              && (data.at(2) & 0xFF) == 0x01 && (data.at(3) & 0xFF) == 0x6f
              && (data.at(4) & 0xFF) == 0x03 && (data.at(5) & 0xFF) == 0x5c
              && (data.at(6) & 0xFF) == 0x01 && (data.at(7) & 0xFF) == 0x9d
              && (data.at(8) & 0xFF) == 0x02 && (data.at(9) & 0xFF) == 0x0f
              && (data.at(10) & 0xFF) == 0x00 && (data.at(11) & 0xFF) == 0x95)
          {

              for (int i=0, j = 0; i < 20471; i+=10, j++)
              {
                  for(int k = 0, l = 0; k < 5; k++)
                  {
                    val1_ch[k] = 0xFF & picture_line.at(i+l);
                    l++;
                    val2_ch[k] = 0xFF & picture_line.at(i+l);
                    l++;
                    tot_ch[k] = (val1_ch[k] <<= 8) | val2_ch[k];
                    *(this->ptrbaChannel[k] + heightinsegment * 2048 + j) = tot_ch[k];

                  }

                  for(int k = 0 ; k < 5; k++)
                  {
                      if (tot_ch[k] < stat_min_ch[k] )
                          stat_min_ch[k] = tot_ch[k];
                      if (tot_ch[k] > stat_max_ch[k] )
                          stat_max_ch[k] = tot_ch[k];
                  }
              }
              heightinsegment++;
          }
      }
    }

    //NbrOfLines = heightinsegment;
    BZ2_bzclose ( b );
    fclose(f);

    return this;

}

void SegmentNoaa::ComposeSegmentLCCProjection(int inputchannel)
{
    ComposeProjection(inputchannel, LCC);
}

void SegmentNoaa::ComposeSegmentGVProjection(int inputchannel)
{
    ComposeProjection(inputchannel, GVP);
}

void SegmentNoaa::ComposeSegmentSGProjection(int inputchannel)
{
    ComposeProjection(inputchannel, SG);
}

void SegmentNoaa::ComposeProjection(int inputchannel, eProjections proj)
{

    qDebug() << QString("SegmentNoaa::ComposeProjection startLineNbr = %1 Start").arg(this->startLineNbr);
    int startheight = this->startLineNbr;

    initializeProjectionCoord();

    inputchannel = (inputchannel == 0 ? 6 : inputchannel);

    QEci eciref;

    double angular_velocity = TWOPI/qtle->Period(); // period in seconds

    for ( int nbrLine = 0; nbrLine < this->NbrOfLines; nbrLine++ )
    {
        double reftime = minutes_since_state_vector + (double)nbrLine/360.0;
        qsgp4->getPosition(reftime , eciref );
        this->RenderSegmentlineInProjection( inputchannel, nbrLine, startheight + nbrLine, eciref, angular_velocity, proj );
        //this->RenderSegmentlineInProjectionAlternative( inputchannel, nbrLine, startheight + nbrLine, eciref, angular_velocity, proj);
    }

    qDebug() << QString("SegmentNoaa::ComposeSegmentLCCProjection startLineNbr = %1 Finished").arg(this->startLineNbr);

}


void SegmentNoaa::RenderSegmentlineInProjection( int channel, int nbrLine, int heightintotalimage, QEci eciref, double ang_vel, eProjections proj)
{

    QRgb *row_col;

    if (channel == 6)
        row_col = (QRgb*)imageptrs->ptrimagecomp_col->scanLine(heightintotalimage);
    else if (channel == 1)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[0]->scanLine(heightintotalimage);
    else if (channel == 2)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[1]->scanLine(heightintotalimage);
    else if (channel == 3)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[2]->scanLine(heightintotalimage);
    else if (channel == 4)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[3]->scanLine(heightintotalimage);
    else if (channel == 5)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[4]->scanLine(heightintotalimage);

    QSgp4Date dateref = eciref.GetDate();
    Vector3 posref = eciref.GetPos();
    Vector3 velref = eciref.GetVel();

    QGeodetic georef = eciref.ToGeo();

    Vector3 scan = posref.cross(velref);
    Vector3 posrefnorm = posref;
    posrefnorm = posrefnorm.normalize();

    Vector3 scannorm = scan;
    scannorm = scannorm.normalize();

    //double delta = 0.0009457 * 1023.5; //0.0009439882 * 1023.5;
    double delta = 0.0009480 * 1023.5;

    double r = posref.length();
    double sindelta = sin(-delta);
    double dd = r * cos(-delta) - sqrt(this->qtle->radiusearthkm * this->qtle->radiusearthkm - r * r * sindelta * sindelta);
    Vector3 d3d = - posrefnorm * cos(-delta) * dd + scannorm * sin(-delta) * dd;

    //qDebug() << QString("a = %1 b = %2").arg(XKMPER * XKMPER).arg( r * r * sindelta * sindelta);
    Vector3 d3earthposfirst = posref + d3d;

    QEci ecifirst(d3earthposfirst, velref, dateref, QTle::wgs72);
    QGeodetic geofirst = ecifirst.ToGeo();

    sindelta = sin(delta);
    dd = r * cos(delta) - sqrt(this->qtle->radiusearthkm * this->qtle->radiusearthkm - r * r * sindelta * sindelta);
    d3d = - posrefnorm * cos(delta) * dd + scannorm * sin(delta) * dd;

    Vector3 d3earthposlast = posref + d3d;

    QSgp4Date newdate;

    newdate.Set(dateref.Julian() + (1/6) * (1.0/ (24.0 * 60.0 * 60.0)), false);

    QEci ecilast(d3earthposlast, velref, newdate, QTle::wgs72);
    QGeodetic geolast = ecilast.ToGeo();

    RenderSegmentlineInProjectionCirc(row_col, nbrLine, geofirst.latitude, geofirst.longitude, geolast.latitude, geolast.longitude, georef.altitude, proj);

}

void SegmentNoaa::RenderSegmentlineInProjectionCirc(QRgb *row_col, int nbrLine, double lat_first, double lon_first, double lat_last, double lon_last, double altitude, eProjections proj)
{
    QRgb rgbvalue = qRgb(0,0,0);

    double latdiff = sin((lat_first-lat_last)/2);
    double londiff = sin((lon_first-lon_last)/2);

    double sinpsi = sqrt(latdiff * latdiff + cos(lat_first)*cos(lat_last)*londiff * londiff);
    double psi = asin(sinpsi);
    double delta = atan(sinpsi/((( this->qtle->radiusearthkm + altitude)/ this->qtle->radiusearthkm) - cos(psi)));

    // qDebug() << QString("earth_views_per_scanline = %1").arg(earth_views_per_scanline);

    //double deltax = delta / 204;
    double deltax = delta / (earth_views_per_scanline / 2);
    double psix;
    double psix1, psix2;
    double dx;
    double r = this->qtle->radiusearthkm + altitude;  // earth_location_altitude[0]/10;
    double sindeltax;
    double lonpos, latpos, dlon, tc;
    double lonpos1, latpos1, lonpos2, latpos2, dlon1, dlon2;

    tc = fmod(atan2(sin(lon_first-lon_last)*cos(lat_last), cos(lat_first)*sin(lat_last)-sin(lat_first)*cos(lat_last)*cos(lon_first-lon_last)) , 2 * PI);

    //sindeltax = sin(delta);
    //dx = r * cos(delta) - sqrt( this->qtle->radiusearthkm * this->qtle->radiusearthkm - r * r * sindeltax * sindeltax );
    //psix = asin( dx * sindeltax / this->qtle->radiusearthkm );



    QColor rgb;
    QColor rgb1, rgb2;
    int posx, posy;
    int posx1, posy1, posx2, posy2;

    double map_x, map_y;

    for (int pix = 0 ; pix < (earth_views_per_scanline/2); pix+=1)                    //205 ; pix++)
    {

        sindeltax = sin(deltax * pix);
        dx = r * cos(deltax * pix) - sqrt( this->qtle->radiusearthkm * this->qtle->radiusearthkm - r * r * sindeltax * sindeltax );
        psix1 = psi + asin( dx * sindeltax / this->qtle->radiusearthkm );
        psix2 = psi - asin( dx * sindeltax / this->qtle->radiusearthkm );

        latpos1 = asin(sin(lat_first)*cos(psix1)+cos(lat_first)*sin(psix1)*cos(tc));
        dlon1=atan2(sin(tc)*sin(psix1)*cos(lat_first),cos(psix1)-sin(lat_first)*sin(latpos1));
        lonpos1=fmod( lon_first-dlon1 + PI,2*PI )-PI;

        latpos2 = asin(sin(lat_first)*cos(psix2)+cos(lat_first)*sin(psix2)*cos(tc));
        dlon2=atan2(sin(tc)*sin(psix2)*cos(lat_first),cos(psix2)-sin(lat_first)*sin(latpos2));
        lonpos2=fmod( lon_first-dlon2 + PI,2*PI )-PI;

        if(proj == LCC)
        {

            if(imageptrs->lcc->map_forward_neg_coord(lonpos1, latpos1, map_x, map_y))
            {
                projectionCoordX[nbrLine * 2048 + (earth_views_per_scanline/2)+pix] = (int)map_x;
                projectionCoordY[nbrLine * 2048 + (earth_views_per_scanline/2)+pix] = (int)map_y;

//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    if(opts.sattrackinimage)
                    {
                        if( pix == 0 || pix == 1)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));
                    }
                    else
                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));

                    projectionCoordValue[nbrLine * 2048 + (earth_views_per_scanline/2)+pix] = rgbvalue;

                    if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }

            if(imageptrs->lcc->map_forward_neg_coord(lonpos2, latpos2, map_x, map_y))
            {
                projectionCoordX[nbrLine * 2048 + (earth_views_per_scanline/2)-pix] = (int)map_x;
                projectionCoordY[nbrLine * 2048 + (earth_views_per_scanline/2)-pix] = (int)map_y;

//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    if(opts.sattrackinimage)
                    {
                        if( pix == 0 || pix == 1)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));
                    }
                    else
                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));

                    projectionCoordValue[nbrLine * 2048 + (earth_views_per_scanline/2)-pix] = rgbvalue;

                    if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }
        }
        else if(proj == GVP)
        {
            if(imageptrs->gvp->map_forward_neg_coord(lonpos1, latpos1, map_x, map_y))
            {
                projectionCoordX[nbrLine * 2048 + (earth_views_per_scanline/2)+pix] = (int)map_x;
                projectionCoordY[nbrLine * 2048 + (earth_views_per_scanline/2)+pix] = (int)map_y;

//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    if(opts.sattrackinimage)
                    {
                        if( pix == 0 || pix == 1)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));
                    }
                    else
                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));

                    projectionCoordValue[nbrLine * 2048 + (earth_views_per_scanline/2)+pix] = rgbvalue;

                    if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }

            if(imageptrs->gvp->map_forward_neg_coord(lonpos2, latpos2, map_x, map_y))
            {
                projectionCoordX[nbrLine * 2048 + (earth_views_per_scanline/2)-pix] = (int)map_x;
                projectionCoordY[nbrLine * 2048 + (earth_views_per_scanline/2)-pix] = (int)map_y;

//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    if(opts.sattrackinimage)
                    {
                        if( pix == 0 || pix == 1)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));
                    }
                    else
                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));

                    projectionCoordValue[nbrLine * 2048 + (earth_views_per_scanline/2)-pix] = rgbvalue;

                    if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }

        }
        else if(proj == SG)
        {
            if(imageptrs->sg->map_forward_neg_coord(lonpos1, latpos1, map_x, map_y))
            {
                projectionCoordX[nbrLine * 2048 + (earth_views_per_scanline/2)+pix] = (int)map_x;
                projectionCoordY[nbrLine * 2048 + (earth_views_per_scanline/2)+pix] = (int)map_y;

//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    if(opts.sattrackinimage)
                    {
                        if( pix == 0 || pix == 1)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));
                    }
                    else
                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));

                    projectionCoordValue[nbrLine * 2048 + (earth_views_per_scanline/2)+pix] = rgbvalue;

                    if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }

            if(imageptrs->sg->map_forward_neg_coord(lonpos2, latpos2, map_x, map_y))
            {
                projectionCoordX[nbrLine * 2048 + (earth_views_per_scanline/2)-pix] = (int)map_x;
                projectionCoordY[nbrLine * 2048 + (earth_views_per_scanline/2)-pix] = (int)map_y;

//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    if(opts.sattrackinimage)
                    {
                        if( pix == 0 || pix == 1)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));
                    }
                    else
                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));

                    projectionCoordValue[nbrLine * 2048 + (earth_views_per_scanline/2)-pix] = rgbvalue;

                    if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }
        }


    }


}

//void SegmentNoaa::RenderSegmentlineInProjectionCirc(QRgb *row_col, double lat_first, double lon_first, double lat_last, double lon_last, double altitude, eProjections proj)
//{
//    QRgb rgbvalue = qRgb(0,0,0);

//    double latdiff = sin((lat_first-lat_last)/2);
//    double londiff = sin((lon_first-lon_last)/2);

//    double sinpsi = sqrt(latdiff * latdiff + cos(lat_first)*cos(lat_last)*londiff * londiff);
//    double psi = asin(sinpsi);
//    double delta = atan(sinpsi/((( this->qtle->radiusearthkm + altitude)/ this->qtle->radiusearthkm) - cos(psi)));

//    // qDebug() << QString("earth_views_per_scanline = %1").arg(earth_views_per_scanline);

//    //double deltax = delta / 204;
//    double deltax = delta / (earth_views_per_scanline / 2);
//    double psix;
//    double psix1, psix2;
//    double dx;
//    double r = this->qtle->radiusearthkm + altitude;  // earth_location_altitude[0]/10;
//    double sindeltax;
//    double lonpos, latpos, dlon, tc;
//    double lonpos1, latpos1, lonpos2, latpos2, dlon1, dlon2;

//    tc = fmod(atan2(sin(lon_first-lon_last)*cos(lat_last), cos(lat_first)*sin(lat_last)-sin(lat_first)*cos(lat_last)*cos(lon_first-lon_last)) , 2 * PI);

//    //sindeltax = sin(delta);
//    //dx = r * cos(delta) - sqrt( this->qtle->radiusearthkm * this->qtle->radiusearthkm - r * r * sindeltax * sindeltax );
//    //psix = asin( dx * sindeltax / this->qtle->radiusearthkm );



//    QColor rgb;
//    QColor rgb1, rgb2;
//    int posx, posy;
//    int posx1, posy1, posx2, posy2;

//    double map_x, map_y;

//    for (int pix = 0 ; pix < (earth_views_per_scanline/2); pix+=1)                    //205 ; pix++)
//    {

//        sindeltax = sin(deltax * pix);
//        dx = r * cos(deltax * pix) - sqrt( this->qtle->radiusearthkm * this->qtle->radiusearthkm - r * r * sindeltax * sindeltax );
//        psix1 = psi + asin( dx * sindeltax / this->qtle->radiusearthkm );
//        psix2 = psi - asin( dx * sindeltax / this->qtle->radiusearthkm );

//        latpos1 = asin(sin(lat_first)*cos(psix1)+cos(lat_first)*sin(psix1)*cos(tc));
//        dlon1=atan2(sin(tc)*sin(psix1)*cos(lat_first),cos(psix1)-sin(lat_first)*sin(latpos1));
//        lonpos1=fmod( lon_first-dlon1 + PI,2*PI )-PI;

//        latpos2 = asin(sin(lat_first)*cos(psix2)+cos(lat_first)*sin(psix2)*cos(tc));
//        dlon2=atan2(sin(tc)*sin(psix2)*cos(lat_first),cos(psix2)-sin(lat_first)*sin(latpos2));
//        lonpos2=fmod( lon_first-dlon2 + PI,2*PI )-PI;

//        if(proj == LCC)
//        {
//            if(imageptrs->lcc->map_forward(lonpos1, latpos1, map_x, map_y))
//            {
//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
//                {
//                    if(opts.sattrackinimage)
//                    {
//                        if( pix == 0 || pix == 1)
//                            rgbvalue = qRgb(255, 0, 0);
//                        else
//                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));
//                    }
//                    else
//                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));

//                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
//                }
//            }

//            if(imageptrs->lcc->map_forward(lonpos2, latpos2, map_x, map_y))
//            {
//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
//                {
//                    if(opts.sattrackinimage)
//                    {
//                        if( pix == 0 || pix == 1)
//                            rgbvalue = qRgb(255, 0, 0);
//                        else
//                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));
//                    }
//                    else
//                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));

//                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
//                }
//            }
//        }
//        else if(proj == GVP)
//        {
//            if(imageptrs->gvp->map_forward(lonpos1, latpos1, map_x, map_y))
//            {
//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
//                {
//                    if(opts.sattrackinimage)
//                    {
//                        if( pix == 0 || pix == 1)
//                            rgbvalue = qRgb(255, 0, 0);
//                        else
//                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));
//                    }
//                    else
//                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));

//                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
//                }
//            }

//            if(imageptrs->gvp->map_forward(lonpos2, latpos2, map_x, map_y))
//            {
//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
//                {
//                    if(opts.sattrackinimage)
//                    {
//                        if( pix == 0 || pix == 1)
//                            rgbvalue = qRgb(255, 0, 0);
//                        else
//                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));
//                    }
//                    else
//                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));

//                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
//                }
//            }

//        }
//        else if(proj == SG)
//        {
//            if(imageptrs->sg->map_forward(lonpos1, latpos1, map_x, map_y))
//            {
//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
//                {
//                    if(opts.sattrackinimage)
//                    {
//                        if( pix == 0 || pix == 1)
//                            rgbvalue = qRgb(255, 0, 0);
//                        else
//                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));
//                    }
//                    else
//                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));

//                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
//                }
//            }

//            if(imageptrs->sg->map_forward(lonpos2, latpos2, map_x, map_y))
//            {
//                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
//                {
//                    if(opts.sattrackinimage)
//                    {
//                        if( pix == 0 || pix == 1)
//                            rgbvalue = qRgb(255, 0, 0);
//                        else
//                            rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));
//                    }
//                    else
//                        rgbvalue =qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));

//                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
//                }
//            }
//        }


//    }


//}


void SegmentNoaa::RenderSegmentlineInProjectionAlternative( int channel, int nbrLine, int heightintotalimage, QEci eciref, double ang_vel, eProjections proj)
{

    QRgb *row_col;
    double map_x, map_y;
    QRgb rgbvalue = qRgb(0,0,0);


    if (channel == 6)
        row_col = (QRgb*)imageptrs->ptrimagecomp_col->scanLine(heightintotalimage);
    else if (channel == 1)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[0]->scanLine(heightintotalimage);
    else if (channel == 2)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[1]->scanLine(heightintotalimage);
    else if (channel == 3)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[2]->scanLine(heightintotalimage);
    else if (channel == 4)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[3]->scanLine(heightintotalimage);
    else if (channel == 5)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[4]->scanLine(heightintotalimage);

    QSgp4Date dateref = eciref.GetDate();
    Vector3 posref = eciref.GetPos();
    Vector3 velref = eciref.GetVel();


    for ( int nbrPoint = 0; nbrPoint < this->earth_views_per_scanline; nbrPoint++ )
    {
        double deltat = (double)nbrPoint/(6.0 * this->earth_views_per_scanline); // in seconden
        double deltax = NEW_NOAA_SWATH / ((this->earth_views_per_scanline / 2) - 0.5);
        //double deltax = (double)FOV_STEP_ANGLE;
        //double deltax = 0.0009439882; // * 1023.5; //0.0009439882 * 1023.5;
        //double deltax = 0.0009400;
        QEci eciaprox = eciref.CircularApprox(deltat, ang_vel);
        double Off_Nadir_Angle = deltax * ((nbrPoint - (this->earth_views_per_scanline/2))+0.5); // in rad
        Vector3 pos = eciaprox.GetPos();
        Vector3 vel = eciaprox.GetVel();
        QSgp4Date dat = eciaprox.GetDate();
        Vector3 posnorm = pos;
        posnorm = posnorm.normalize();
        Vector3 scan = pos.cross(vel);
        Vector3 scannorm = scan;
        scannorm = scannorm.normalize();

        double posmag = pos.length();

        // range vector from satellite for a spherical earth
        double cosoffnadir = cos(Off_Nadir_Angle);
        double sinoffnadir = sin(Off_Nadir_Angle);

        Vector3 rangevectornorm = - posnorm * cosoffnadir + scannorm * sinoffnadir;
        double squareradiusearthkm = this->qtle->radiusearthkm * this->qtle->radiusearthkm;
        double squareradiusearthkmminor = this->qtle->radiusearthkmminor * this->qtle->radiusearthkmminor;
        double epsilon = ( squareradiusearthkm / squareradiusearthkmminor) - 1;

        //double firstterm = sqrt(this->qtle->radiusearthkm * this->qtle->radiusearthkm - posmag * posmag * sinoffnadir * sinoffnadir);
        //double range_distance = posmag * cosoffnadir - firstterm;

        double firstterm = 1 + (epsilon * rangevectornorm.z * rangevectornorm.z);
        double secterm = posmag * cosoffnadir - epsilon * pos.z * rangevectornorm.z;
        double thirdterm = squareradiusearthkm - posmag * posmag - epsilon * pos.z * pos.z + secterm * secterm;
        double range_distance = (secterm / firstterm) - sqrt( thirdterm / firstterm ) ;


/*        double firstterm = 1 + (epsilon * rangevectornorm.z * rangevectornorm.z);
        double secterm = epsilon * pos.z * rangevectornorm.z - posmag * cosoffnadir;
        double thirdterm = posmag * posmag - squareradiusearthkm + epsilon * pos.z * pos.z;
        double determinant = secterm * secterm - firstterm * thirdterm;
        double fourthterm = posmag * cosoffnadir - epsilon * pos.z * rangevectornorm.z - sqrt(determinant);
        double range_distance = fourthterm / firstterm;
*/
        Vector3 rangevector = rangevectornorm * range_distance;
        Vector3 location = pos + rangevector;

        ///////////////////////////////////////////
        //const double theta = Util::AcTan(location.y, location.x);
        //double lonpos   = fmod(theta - dat.ToGreenwichSiderealTime(), TWOPI);

        //double r   = sqrt(location.x * location.x + location.y * location.y);
        //double latpos = Util::AcTan(location.z, r);
        ///////////////////////////////////////////////

        QEci ecilocation(location, vel, dat );
        QGeodetic geolocation = ecilocation.ToGeo();

        double lonpos = geolocation.longitude;
        double latpos = geolocation.latitude;
        /////////////////////////////////////////////


        if(proj == LCC)
        {
            if(imageptrs->lcc->map_forward(lonpos, latpos, map_x, map_y))
            {
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    if(opts.sattrackinimage)
                    {
                        if( nbrPoint == 1023 || nbrPoint == 1024)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            rgbvalue =qRgb(qRed(row_col[nbrPoint]), qGreen(row_col[nbrPoint]), qBlue(row_col[nbrPoint]));
                    }
                    else
                        rgbvalue =qRgb(qRed(row_col[nbrPoint]), qGreen(row_col[nbrPoint]), qBlue(row_col[nbrPoint]));

                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                    // qDebug() << QString("map_x = %1 map_y = %2").arg(map_x).arg(map_y);
                }
            }
        }
        else if(proj == GVP)
        {
            if(imageptrs->gvp->map_forward(lonpos, latpos, map_x, map_y))
            {
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    if(opts.sattrackinimage)
                    {
                        if( nbrPoint == 1023 || nbrPoint == 1024)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            rgbvalue =qRgb(qRed(row_col[nbrPoint]), qGreen(row_col[nbrPoint]), qBlue(row_col[nbrPoint]));
                    }
                    else
                        rgbvalue =qRgb(qRed(row_col[nbrPoint]), qGreen(row_col[nbrPoint]), qBlue(row_col[nbrPoint]));

                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                    // qDebug() << QString("map_x = %1 map_y = %2").arg(map_x).arg(map_y);
                }
            }

        }
        else if(proj == SG)
        {
            if(imageptrs->sg->map_forward(lonpos, latpos, map_x, map_y))
            {
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    if(opts.sattrackinimage)
                    {
                        if( nbrPoint == 1023 || nbrPoint == 1024)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            rgbvalue =qRgb(qRed(row_col[nbrPoint]), qGreen(row_col[nbrPoint]), qBlue(row_col[nbrPoint]));
                    }
                    else
                        rgbvalue =qRgb(qRed(row_col[nbrPoint]), qGreen(row_col[nbrPoint]), qBlue(row_col[nbrPoint]));

                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                    // qDebug() << QString("map_x = %1 map_y = %2").arg(map_x).arg(map_y);
                }
            }

        }
        //qDebug() << QString("lonpos = %1 latpos = %2 Angle = %3 posmag = %4 rangvector = %5 firstterm = %6 secterm = %7 posnorm = %8 scan x = %9 y = %10 z = %11").arg(Util::RadiansToDegrees(lonpos)).arg(Util::RadiansToDegrees(latpos))
        //            .arg(Off_Nadir_Angle).arg(posmag).arg(rangevector.Magnitude()).arg(firstterm).arg(secterm).arg(posnorm.Magnitude()).arg(scan.x).arg(scan.y).arg(scan.z);
        //double dotprod = pos.Dot(location);
        //double phi = acos(dotprod / ( pos.Magnitude()*location.Magnitude()));
        //qDebug() << QString("nbrLine = %1 nbrPoint = %2 off_nadir angle = %3 phi = %4 range_disctance = %5").arg(nbrLine).arg(nbrPoint).arg(Util::RadiansToDegrees(Off_Nadir_Angle)).arg(Util::RadiansToDegrees(phi))
        //            .arg(range_distance);

    }
}



