#include "segmenthrp.h"

#include "sgp4sdp4.h"
#include "globals.h"
#include "segmentimage.h"
#include "Matrices.h"

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QString>
#include <QColor>
#include <QPen>
#include <QPixmap>
#include <QDate>
#include <QFileInfo>
#include <QMatrix4x4>

extern Options opts;
extern SegmentImage *imageptrs;
extern SatelliteList satellitelist;

SegmentHRP::SegmentHRP(QFile *filesegment, QObject *parent) :
    Segment(parent)
{
    bool ok;
    fileInfo.setFile(*filesegment);
    segtype = eSegmentType::SEG_HRP;
    segment_type = "HRP";

    this->satname = fileInfo.baseName().mid(12, 3);

    this->earth_views_per_scanline = 2048;

    //AVHR_HRP_00_M02_20130701060200Z_20130701060300Z_N_O_20130701061314Z

    int sensing_start_year = fileInfo.fileName().mid(16, 4).toInt( &ok , 10);
    int sensing_start_month = fileInfo.fileName().mid(20, 2).toInt( &ok, 10);
    int sensing_start_day = fileInfo.fileName().mid(22, 2).toInt( &ok, 10);
    int sensing_start_hour = fileInfo.fileName().mid(24, 2).toInt( &ok, 10);
    int sensing_start_minute = fileInfo.fileName().mid(26, 2).toInt( &ok, 10);
    int sensing_start_second = fileInfo.fileName().mid(28, 2).toInt( &ok, 10);

    qdatetime_start.setDate(QDate(sensing_start_year, sensing_start_month, sensing_start_day));
    qdatetime_start.setTime(QTime(sensing_start_hour,sensing_start_minute, sensing_start_second));

    qsensingstart = QSgp4Date(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, sensing_start_second);
    qsensingend = qsensingstart;
    qsensingend.AddMin(1.0);

    julian_sensing_start = qsensingstart.Julian();
    julian_sensing_end = qsensingend.Julian();

    Satellite *sat;

    if(fileInfo.fileName().mid(0,15) == "AVHR_HRP_00_M02")  // Metop-A
        sat = satellitelist.GetSatellite(29499, &ok);
    else if(fileInfo.fileName().mid(0,15) == "AVHR_HRP_00_M01") // Metop-B
        sat = satellitelist.GetSatellite(38771, &ok);
    else if(fileInfo.fileName().mid(0,15) == "AVHR_HRP_00_M03") // Metop-C
        sat = satellitelist.GetSatellite(43689, &ok);

    line1 = sat->line1;
    line2 = sat->line2;

    qtle.reset(new QTle(fileInfo.fileName().mid(0,15), line1, line2, QTle::wgs72));
    qsgp4.reset(new QSgp4( *qtle ));

    julian_state_vector = qtle->Epoch();

    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MINUTES_PER_DAY;
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

SegmentHRP::~SegmentHRP()
{
}

int SegmentHRP::ReadNbrOfLines()
{
    FILE*   f;
    BZFILE* b;

    quint32 nextres = 0;
    quint64 nextrecordlength = 0;

    int     nBuf;
    char    buf[ 26640 ];
    char    bufheader[20];
    int     bzerror;

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
      nBuf = BZ2_bzRead ( &bzerror, b, bufheader, 20 );
      //qDebug() << QString("nBuf header = %1").arg(nBuf);

      if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
      {
         QByteArray dataheader = QByteArray::fromRawData(bufheader, sizeof(bufheader));
         nextres = get_next_header(dataheader);
         nextrecordlength += nextres;
         nBuf = BZ2_bzRead ( &bzerror, b, buf, nextres - 20 );

         if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
         {
             if ((dataheader.at(0) & 0xFF) == 0x08)
             {
                 heightinsegment++;
             }
         }
      }
    }

    BZ2_bzclose ( b );
    fclose(f);

    return heightinsegment;
}


Segment *SegmentHRP::ReadSegmentInMemory()
{

    FILE*   f;
    BZFILE* b;
    int     nBuf;
    char    buf[ 26640 ];
    char    bufheader[20];
    int     bzerror;

    int n=0;
    int i,j,c,p;

    qDebug() << "ReadSegmentInMemory() HRP";
    quint64 nextrecordlength = 0;
    quint32 nextres = 0;
    quint16 val, val1, val2;
    quint16 valline[5][2048];

    int heightinsegment = 0;

    f = fopen ( this->fileInfo.absoluteFilePath().toLatin1(), "rb" );
    if ( !f ) {
        qDebug() << QString("file %1 not found ! ").arg(this->fileInfo.absoluteFilePath());
        return this;
    }
    else
        qDebug() << QString("file %1 is open ! ").arg(this->fileInfo.absoluteFilePath());

    if((b = BZ2_bzopen(this->fileInfo.absoluteFilePath().toLatin1(),"rb"))==NULL)
    {
        qDebug() << "error in BZ2_bzopen";
    }

    bzerror = BZ_OK;

    while ( bzerror == BZ_OK )
    {
      nBuf = BZ2_bzRead ( &bzerror, b, bufheader, 20 );

      if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
      {
         QByteArray dataheader = QByteArray::fromRawData(bufheader, sizeof(bufheader));
         nextres = get_next_header(dataheader);
         nextrecordlength += nextres;

         nBuf = BZ2_bzRead ( &bzerror, b, buf, nextres - 20 );

         if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
         {
             if ((dataheader.at(0) & 0xFF) == 0x01)
             {
                 QByteArray mphr_record = QByteArray::fromRawData(buf, nBuf);
                 if(!inspectMPHRrecord(mphr_record))
                     segmentok = false;
             }

            if ((dataheader.at(0) & 0xFF) == 0x08)
            {

                QByteArray mdr_record = QByteArray::fromRawData(buf, nBuf);

                n=76;
                c=0;
                p=0;
                j=0;
                for (i=-3*5; i<2048*5; )
                {
                    QByteArray tmp = mdr_record.mid( n, 5);
                    n += 5;

                    if (i>=0)
                    {
                        //val=((tmp[0]&0xff)<<2)+((tmp[1]&0xc0)>>6);
                        val1 = 0xFF & tmp.at(0);
                        val2 = 0xC0 & tmp.at(1);
                        val=(val1<<2) | (val2>>6);
                        valline[c][p] = val;

                        c=(c+1)%5;
                        j++; p=j/5;
                    }
                    i++;
                    if (i>=0)
                    {
                        //val=((tmp[1]&0x3f)<<4)+((tmp[2]&0xf0)>>4);
                        val1 = 0x3F & tmp.at(1);
                        val2 = 0xF0 & tmp.at(2);
                        val=(val1<<4) | (val2>>4);
                        valline[c][p] = val;

                        c=(c+1)%5;
                        j++; p=j/5;
                    }
                    i++;
                    if (i>=0)
                    {
                        //val=((tmp[2]&0x0f)<<6)+((tmp[3]&0xfc)>>2);

                        val1 = 0x0F & tmp.at(2);
                        val2 = 0xFC & tmp.at(3);
                        val=(val1<<6) | (val2>>2);
                        valline[c][p] = val;

                        c=(c+1)%5;
                        j++; p=j/5;
                      }
                      i++;
                      if (i>=0)
                      {
                        //val=((tmp[3]&0x03)<<8)+((tmp[4]&0xff)>>0);

                        val1 = 0x03 & tmp.at(3);
                        val2 = 0xFF & tmp.at(4);
                        val=(val1<<8) | val2;
                        valline[c][p] = val;

                        c=(c+1)%5;
                        j++; p=j/5;
                      }
                      i++;

                    }

                    for( int k = 0; k < 5; k++)
                    {
                        for( int j = 0; j < 2048; j++)
                            *(this->ptrbaChannel[k].data() + (heightinsegment) * 2048 + j) = valline[k][j];
                    }

                    for (int i=0; i < 2048; i++)
                    {
                        for( int k = 0; k < 5; k++)
                        {
                            if (valline[k][i] < stat_min_ch[k] )
                                stat_min_ch[k] = valline[k][i];
                            if (valline[k][i] > stat_max_ch[k] )
                                stat_max_ch[k] = valline[k][i];
                        }
                    }


                    heightinsegment++;
                }
            }
       }
    }

    NbrOfLines = heightinsegment;

    BZ2_bzclose ( b );
    fclose(f);


    return this;

}

bool SegmentHRP::inspectMPHRrecord(QByteArray mphr_record)
{
    bool ok;

    int sensing_start_year = mphr_record.mid(712, 4).toInt( &ok , 10);
    int sensing_start_month = mphr_record.mid(716, 2).toInt( &ok, 10);
    int sensing_start_day = mphr_record.mid(718, 2).toInt( &ok, 10);
    int sensing_start_hour = mphr_record.mid(720, 2).toInt( &ok, 10);
    int sensing_start_minute = mphr_record.mid(722, 2).toInt( &ok, 10);
    int sensing_start_second = mphr_record.mid(724, 2).toInt( &ok, 10);

    int sensing_end_year = mphr_record.mid(760, 4).toInt( &ok, 10);
    int sensing_end_month = mphr_record.mid(764, 2).toInt( &ok, 10);
    int sensing_end_day = mphr_record.mid(766, 2).toInt( &ok, 10);
    int sensing_end_hour = mphr_record.mid(768, 2).toInt( &ok, 10);
    int sensing_end_minute = mphr_record.mid(770, 2).toInt( &ok, 10);
    int sensing_end_second = mphr_record.mid(772, 2).toInt( &ok, 10);

    NbrOfLines = mphr_record.mid(2970, 3).toInt( &ok, 10);

    QDateTime sensing_start(QDate(sensing_start_year,sensing_start_month, sensing_start_day), QTime(sensing_start_hour, sensing_start_minute, sensing_start_second));
    QDateTime sensing_end(QDate(sensing_end_year,sensing_end_month, sensing_end_day), QTime(sensing_end_hour, sensing_end_minute, sensing_end_second));

    this->sensing_start_year = sensing_start_year;
    qdatetime_start.setDate(QDate(sensing_start_year, sensing_start_month, sensing_start_day));
    qdatetime_start.setTime(QTime(sensing_start_hour,sensing_start_minute, sensing_start_second));

    qsensingstart = QSgp4Date(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, sensing_start_second);
    qsensingend = QSgp4Date(sensing_end_year, sensing_end_month, sensing_end_day, sensing_end_hour, sensing_end_minute, sensing_end_second);

    Satellite *metop_sat;

    if(fileInfo.fileName().mid(0,15) == "AVHR_HRP_00_M02")  // Metop-A
        metop_sat = satellitelist.GetSatellite(29499, &ok);
    else if(fileInfo.fileName().mid(0,15) == "AVHR_HRP_00_M01") // Metop-B
        metop_sat = satellitelist.GetSatellite(38771, &ok);
    else if(fileInfo.fileName().mid(0,15) == "AVHR_HRP_00_M03") // Metop-C
        metop_sat = satellitelist.GetSatellite(43689, &ok);

    this->earth_views_per_scanline = 2048;

    line1 = metop_sat->line1;
    line2 = metop_sat->line2;

    qtle.reset(new QTle(fileInfo.fileName().mid(0,15), line1, line2, QTle::wgs72));
    qsgp4.reset(new QSgp4( *qtle ));

    // epoch = line1.mid(18,14).toDouble(&ok);
    // julian_state_vector = Julian_Date_of_Epoch(epoch);

    julian_state_vector = qtle->Epoch();
    julian_sensing_start = qsensingstart.Julian();
    julian_sensing_end = qsensingend.Julian();


    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MINUTES_PER_DAY;
    //minutes_since_state_vector -= 1.5 / 60.0;
    minutes_sensing = (double)(sensing_start.secsTo(sensing_end))/60;
    //qDebug() << "arg of perigee = " << QString::number(orbit->ArgPerigee() * 180 / PI) << "  mnAnomaly = " << QString::number(orbit->mnAnomaly() * 180 /PI);

    QEci qeci;
    qsgp4->getPosition(minutes_since_state_vector, qeci);
    QGeodetic qgeo = qeci.ToGeo();

    double lon_1 = qgeo.longitude;
    double lat_1 = qgeo.latitude;
    lon_start_deg = rad2deg(lon_1);
    if (lon_start_deg > 180)
        lon_start_deg = - (360 - rad2deg(lon_1));

    lat_start_deg = rad2deg(lat_1);

    return true;

 }

quint32 SegmentHRP::get_next_header( QByteArray ba )
{
    if (ba.at(0) == 0x01)
        // qDebug() << "MPHR";
        cnt_mphr++;
    else if (ba.at(0) == 0x02)
        //qDebug() << "SPHR";
        cnt_sphr++;
    else if (ba.at(0) == 0x03)
        //qDebug() << "IPR";
        cnt_ipr++;
    else if (ba.at(0) == 0x04)
        //qDebug() << "GEADR";
        cnt_geadr++;
    else if (ba.at(0) == 0x05)
        //qDebug() << "GIADR";
        cnt_giadr++;
    else if (ba.at(0) == 0x06)
        //qDebug() << "VEADR";
        cnt_veadr++;
    else if (ba.at(0) == 0x07)
        //qDebug() << "VIADR";
        cnt_viadr++;
    else if (ba.at(0) == 0x08)
    {
        cnt_mdr++;
    }
    else qDebug() << QString("------>Unknown record for %1").arg(this->fileInfo.absoluteFilePath());

    QByteArray tes = ba.mid(4, 4);

    quint32 num1=0, num2=0, num3=0, num4=0;
    num1 = 0XFF & tes.at(0);   // 0X8FFF;
    num2 = 0XFF & tes.at(1);
    num3 = 0XFF & tes.at(2);   // 0X8FFF;
    num4 = 0XFF & tes.at(3);

    quint32 reclengte = (num1 <<= 24) | (num2 <<= 16) | (num3 <<= 8) | num4;

    // qDebug() << QString("de nieuwe recordlengte = %1").arg(reclengte, 0, 16);
    return reclengte;

}

void SegmentHRP::ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(inputchannel, LCC);
}

void SegmentHRP::ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(inputchannel, GVP);
}

void SegmentHRP::ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(inputchannel, SG);
}

void SegmentHRP::ComposeProjection(int inputchannel, eProjections proj)
{

    qDebug() << QString("SegmentHRP::ComposeProjection startLineNbr = %1").arg(this->startLineNbr);
    int startheight = this->startLineNbr;

    initializeProjectionCoord();

    inputchannel = (inputchannel == 0 ? 6 : inputchannel);

    QEci eciref;

    double angular_velocity = TWOPI/qtle->Period(); // period in seconds
    double e = qtle->Eccentricity();
    double epow2 = e * e;
    double epow3 = e * e * e;

    double epochcorrection = 0;
    double yawcorrection = 0;

    for (int nbrLine = 0; nbrLine < this->NbrOfLines; nbrLine++)
    {
        double reftime = minutes_since_state_vector + (double)nbrLine/360.0;
        qsgp4->getPosition(reftime , eciref );

        double span = eciref.GetDate().spanSec(qtle->Epoch()) + epochcorrection;
        double M = fmod(qtle->MeanAnomaly() + (TWOPI * (span/qtle->Period())), TWOPI);
        double C = (2*e - epow3/4)*sin(M) + (5*epow2/4)*sin(2*M) + (13*epow3/12)*sin(3*M);
        double trueAnomaly = M + C;
        double PSO = fmod(qtle->ArgumentPerigee() + trueAnomaly, TWOPI);

        double pitch = + 0.002899 * sin( 2 * PSO);
        double roll = - 0.00089 * sin(PSO);
        double yaw_factor = 0.068766 * cos(PSO);
        double yaw = yaw_factor * (1 - yaw_factor * yaw_factor/3);

        //roll *= 2;

        if( nbrLine == 0 || nbrLine == this->NbrOfLines - 1)
            qDebug() << QString("nbrline = %1 Pitch = %2  Roll = %3 Yaw = %4").arg(nbrLine).arg(pitch).arg(roll).arg(yaw);

        this->RenderSegmentlineInProjectionAlternative(inputchannel, nbrLine, startheight + nbrLine, eciref, angular_velocity, pitch, roll, yaw + yawcorrection, proj);
        //this->RenderSegmentlineInProjection(inputchannel, startheight + nbrLine, eciref, angular_velocity, proj );
    }
}

/*
void SegmentHRP::ComposeSegmentLCCProjection(int inputchannel)
{

    qDebug() << QString("SegmentHRP::ComposeSegmentLCCProjection startLineNbr = %1").arg(this->startLineNbr);
    int startheight = this->startLineNbr;

    inputchannel = (inputchannel == 0 ? 6 : inputchannel);

    cEci eciref;

    double angular_velocity = TWOPI/orbit->Period(); // period in seconds

    for (int nbrLine = 0; nbrLine < this->NbrOfLines; nbrLine++)
    {
        double reftime = minutes_since_state_vector + (double)nbrLine/360.0;
        orbit->getPosition(reftime , &eciref );
        this->RenderSegmentlineInLCCcEci(inputchannel, startheight + nbrLine, eciref, angular_velocity );
        //this->RenderSegmentlineInLCC(inputchannel, startheight + nbrLine, eciref, angular_velocity );
    }
}
*/

void SegmentHRP::RenderSegmentlineInProjection( int channel, int heightintotalimage, QEci eciref, double ang_vel, eProjections proj)
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

    double e = qtle->Eccentricity();

    double epow2 = e * e;
    double epow3 = e * e * e;
    double M = qtle->MeanAnomaly(dateref);
    double M0 = qtle->MeanAnomaly();

    double C = (2*e - epow3/4)*sin(M) + (5*epow2/4)*sin(2*M) + (13*epow3/12)*sin(3*M);
    double trueAnomaly = M + C;
    double PSO = fmod(qtle->ArgumentPerigee() + trueAnomaly, TWOPI);

        double pitch_steering_angle = + 0.002899 * sin( 2 * PSO);
        double roll_steering_angle = - 0.00089 * sin(PSO);
        double yaw_factor = 0.068766 * cos(PSO);
        double yaw_steering_angle = 0.068766 * cos(PSO) * (1 - yaw_factor * yaw_factor/3);
        //qDebug() << QString("yaw steering angle (degrees) = %1").arg(yaw_steering_angle * 180/PI);

        Matrix4 mat;
        Vector3 scan;
        Vector3 scanref = posref.cross(velref);

        mat.identity();
        mat.rotate(yaw_steering_angle * 180/PI, posref);  // yaw
        mat.rotate(roll_steering_angle * 180/PI, velref); // roll
        mat.rotate(pitch_steering_angle * 180/PI, scanref); // pitch
        scan = mat * scanref;



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

    RenderSegmentlineInProjectionCirc(row_col, geofirst.latitude, geofirst.longitude, geolast.latitude, geolast.longitude, georef.altitude, proj);

}

/*
void SegmentHRP::RenderSegmentlineInLCCcEci( int channel, int heightintotalimage, cEci eciref, double ang_vel)
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

    cJulian dateref = eciref.getDate();
    QVector4D posref4d = eciref.getPos();
    QVector4D velref4d = eciref.getVel();

    QVector3D posref = posref4d.toVector3D();
    QVector3D velref = velref4d.toVector3D();

    cCoordGeo georef = eciref.toGeo();

    double e = orbit->Eccentricity();

    double epow2 = e * e;
    double epow3 = e * e * e;
    double M = orbit->mnAnomaly(dateref);
    double M0 = orbit->mnAnomaly();

    double C = (2*e - epow3/4)*sin(M) + (5*epow2/4)*sin(2*M) + (13*epow3/12)*sin(3*M);
    double trueAnomaly = M + C;
    double PSO = fmod(orbit->ArgPerigee() + trueAnomaly, TWOPI);

        double pitch_steering_angle = + 0.002899 * sin( 2 * PSO);
        double roll_steering_angle = - 0.00089 * sin(PSO);
        double yaw_factor = 0.068766 * cos(PSO);
        double yaw_steering_angle = 0.068766 * cos(PSO) * (1 - yaw_factor * yaw_factor/3);
        //qDebug() << QString("yaw steering angle (degrees) = %1").arg(yaw_steering_angle * 180/PI);

        QMatrix4x4 mat;
        QVector3D scan;
        QVector3D scanref = QVector3D::crossProduct(posref, velref);

        mat.setToIdentity();
        mat.rotate(yaw_steering_angle * 180/PI, posref);  // yaw
        mat.rotate(roll_steering_angle * 180/PI, velref); // roll
        mat.rotate(pitch_steering_angle * 180/PI, scanref); // pitch
        scan = mat * scanref;



    QVector3D posrefnorm = posref.normalized();
    QVector3D scannorm = scan.normalized();

    //double delta = 0.0009457 * 1023.5; //0.0009439882 * 1023.5;
    double delta = 0.0009480 * 1023.5;

    double r = posref.length();
    double sindelta = sin(-delta);
    double dd = r * cos(-delta) - sqrt(this->qtle->radiusearthkm * this->qtle->radiusearthkm - r * r * sindelta * sindelta);
    QVector3D d3d = - posrefnorm * cos(-delta) * dd + scannorm * sin(-delta) * dd;

    //qDebug() << QString("a = %1 b = %2").arg(XKMPER * XKMPER).arg( r * r * sindelta * sindelta);
    QVector3D d3earthposfirst = posref + d3d;

    cEci ecifirst(d3earthposfirst, velref, dateref);
    cCoordGeo geofirst = ecifirst.toGeo();

    sindelta = sin(delta);
    dd = r * cos(delta) - sqrt(this->qtle->radiusearthkm * this->qtle->radiusearthkm - r * r * sindelta * sindelta);
    d3d = - posrefnorm * cos(delta) * dd + scannorm * sin(delta) * dd;

    QVector3D d3earthposlast = posref + d3d;

    cJulian newdate = dateref;

    newdate.addSec( (1/6) * (1.0/ (24.0 * 60.0 * 60.0)));

    cEci ecilast(d3earthposlast, velref, newdate);
    cCoordGeo geolast = ecilast.toGeo();

    RenderSegmentlineInLCCCirc(row_col, geofirst.m_Lat, geofirst.m_Lon, geolast.m_Lat, geolast.m_Lon, georef.m_Alt);

}
*/

void SegmentHRP::RenderSegmentlineInProjectionCirc(QRgb *row_col, double lat_first, double lon_first, double lat_last, double lon_last, double altitude, eProjections proj)
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
            if(imageptrs->lcc->map_forward(lonpos1, latpos1, map_x, map_y))
            {
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
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

                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }

            if(imageptrs->lcc->map_forward(lonpos2, latpos2, map_x, map_y))
            {
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
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

                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }
        }
        else if(proj == GVP)
        {
            if(imageptrs->gvp->map_forward(lonpos1, latpos1, map_x, map_y))
            {
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
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

                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }

            if(imageptrs->gvp->map_forward(lonpos2, latpos2, map_x, map_y))
            {
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
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

                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }

        }
        else if(proj == SG)
        {
            if(imageptrs->sg->map_forward(lonpos1, latpos1, map_x, map_y))
            {
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
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

                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }

            if(imageptrs->sg->map_forward(lonpos2, latpos2, map_x, map_y))
            {
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
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

                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }

        }

    }
}


void SegmentHRP::RenderSegmentlineInProjectionAlternative(int channel, int nbrLine, int heightintotalimage, QEci eciref, double ang_vel, double pitch, double roll, double yaw, eProjections proj)
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

    for ( int nbrPoint = 0; nbrPoint < this->earth_views_per_scanline; nbrPoint++ )
    {
        double deltat = (double)nbrPoint/(6.0 * this->earth_views_per_scanline); // in seconden
        //double deltax = Util::DegreesToRadians(55.2) / ((this->earth_views_per_scanline / 2) - 0.5);
        //double deltax = (double)FOV_STEP_ANGLE;
        double deltax = 0.0009439882; // * 1023.5; //0.0009439882 * 1023.5;
        //double deltax = 0.0009441;
        QEci eciaprox = eciref.CircularApprox(deltat, ang_vel);
        double Off_Nadir_Angle = deltax * ((nbrPoint - (this->earth_views_per_scanline/2))+0.5); // in rad
        Vector3 pos = eciaprox.GetPos();
        Vector3 vel = eciaprox.GetVel();
        QSgp4Date dat = eciaprox.GetDate();
        Vector3 posnorm = pos;
        posnorm = posnorm.normalize();
        Vector3 scan;

        Matrix4 mat;
        QMatrix4x4 qmat;

/*        double e = qtle->Eccenticity();
        double epow2 = e * e;
        double epow3 = e * e * e;
        double M = qtle->MeanAnomaly(dat);
        double C = (2*e - epow3/4)*sin(M) + (5*epow2/4)*sin(2*M) + (13*epow3/12)*sin(3*M);
        double trueAnomaly = M + C;
        double PSO = fmod(qtle->ArgumentPerigee() + trueAnomaly, TWOPI);
*/

/*        Furthermore, from the pattern of the shift it is clear that both, the AVHRR maximum scanning angle
        and the AVHRR attitude have to be corrected. It has been found that the following values provide a
        geolocation accuracy of ±1pixel over the whole AVHRR swath: maximum scanning angle of 55.2°;
        pitch correction: 0.117°, roll correction -0.125°, yaw correction -0.034°.
*/
/*
        double pitch_steering_angle = + 0.002899 * sin( 2 * PSO);
        double roll_steering_angle = - 0.00089 * sin(PSO);
        double yaw_factor = 0.068766 * cos(PSO);
        double yaw_steering_angle = yaw_factor * (1 - yaw_factor * yaw_factor/3);
*/
        //qDebug() << QString("yaw steering angle (degrees) = %1").arg(yaw_steering_angle * 180/PI);
/*        double pitch_steering_angle = Util::DegreesToRadians(0.117);
        double roll_steering_angle = Util::DegreesToRadians(-0.125);
        double yaw_steering_angle = Util::DegreesToRadians(-0.034);
*/

/*
        mat = mat.identity();
        mat.rotate(pitch_steering_angle * 180.0/PI, pos.cross(vel)); // pitch
        mat.rotate(roll_steering_angle * 180.0/PI, vel); // roll
        mat.rotate(yaw_steering_angle * 180.0/PI, pos);  // yaw
        Vector3 scan1 = pos.cross(vel);
        scan = mat * scan1;
*/
        QVector3D d3pos(pos.x, pos.y, pos.z);
        QVector3D d3vel(vel.x, vel.y, vel.z);

        qmat.setToIdentity();
//        qmat.rotate(yaw_steering_angle * 180/PI, d3pos);  // yaw
//        qmat.rotate(roll_steering_angle * 180/PI, d3vel); // roll
//        qmat.rotate(pitch_steering_angle * 180/PI, QVector3D::crossProduct(d3pos,d3vel)); // pitch
        qmat.rotate(yaw * 180/PI, d3pos);  // yaw
        qmat.rotate(roll * 180/PI, d3vel); // roll
        qmat.rotate(pitch * 180/PI, QVector3D::crossProduct(d3pos,d3vel)); // pitch
        QVector3D d3scan = qmat * QVector3D::crossProduct(d3pos,d3vel);
        scan.set(d3scan.x(), d3scan.y(), d3scan.z());

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


        Vector3 rangevector = rangevectornorm * range_distance;
        Vector3 location = pos + rangevector;

        QEci ecilocation(location, vel, dat );
        QGeodetic geolocation = ecilocation.ToGeo();

        double lonpos = geolocation.longitude;
        double latpos = geolocation.latitude;

        if(proj == LCC)
        {
            if(imageptrs->lcc->map_forward_neg_coord(lonpos, latpos, map_x, map_y))
            {
                //if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    projectionCoordX[nbrLine * 2048 + nbrPoint] = (int)map_x;
                    projectionCoordY[nbrLine * 2048 + nbrPoint] = (int)map_y;

                    if(opts.sattrackinimage)
                    {
                        if( nbrPoint == 1023 || nbrPoint == 1024)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            rgbvalue = row_col[nbrPoint];
                    }
                    else
                        rgbvalue = row_col[nbrPoint];

                    projectionCoordValue[nbrLine * 2048 + nbrPoint] = rgbvalue;

                    if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);

                }
            }
        }
        else if(proj == GVP)
        {
            if(imageptrs->gvp->map_forward_neg_coord(lonpos, latpos, map_x, map_y))
            {
                //if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    projectionCoordX[nbrLine * 2048 + nbrPoint] = (int)map_x;
                    projectionCoordY[nbrLine * 2048 + nbrPoint] = (int)map_y;

                    if(opts.sattrackinimage)
                    {
                        if( nbrPoint == 1023 || nbrPoint == 1024)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            //rgbvalue =qRgb(qRed(row_col[nbrPoint]), qGreen(row_col[nbrPoint]), qBlue(row_col[nbrPoint]));
                            rgbvalue = row_col[nbrPoint];
                    }
                    else
                        //rgbvalue =qRgb(qRed(row_col[nbrPoint]), qGreen(row_col[nbrPoint]), qBlue(row_col[nbrPoint]));
                        rgbvalue = row_col[nbrPoint];

                    projectionCoordValue[nbrLine * 2048 + nbrPoint] = rgbvalue;

                    if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);

                }
            }

        }
        else if(proj == SG)
        {
            if(imageptrs->sg->map_forward_neg_coord(lonpos, latpos, map_x, map_y))
            {
                //if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                {
                    projectionCoordX[nbrLine * 2048 + nbrPoint] = (int)map_x;
                    projectionCoordY[nbrLine * 2048 + nbrPoint] = (int)map_y;

                    if(opts.sattrackinimage)
                    {
                        if( nbrPoint == 1023 || nbrPoint == 1024)
                            rgbvalue = qRgb(255, 0, 0);
                        else
                            //rgbvalue =qRgb(qRed(row_col[nbrPoint]), qGreen(row_col[nbrPoint]), qBlue(row_col[nbrPoint]));
                            rgbvalue = row_col[nbrPoint];

                    }
                    else
                        //rgbvalue =qRgb(qRed(row_col[nbrPoint]), qGreen(row_col[nbrPoint]), qBlue(row_col[nbrPoint]));
                        rgbvalue = row_col[nbrPoint];

                    projectionCoordValue[nbrLine * 2048 + nbrPoint] = rgbvalue;

                    if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                }
            }

        }
    }
}

