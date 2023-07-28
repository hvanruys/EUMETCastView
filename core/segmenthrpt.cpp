#include "segmenthrpt.h"
#include "options.h"
#include "segmentimage.h"
#include "sgp4sdp4.h"
#include "globals.h"
#include "Matrices.h"

extern Options opts;
extern SegmentImage *imageptrs;

SegmentHRPT::SegmentHRPT(eSegmentType type, QFile *filesegment, QObject *parent) :
    Segment(parent)
{
    fileInfo.setFile(*filesegment);
    segtype = type;
    this->earth_views_per_scanline = 2048;

    this->year = fileInfo.baseName().mid(0,4).toInt();
    if(type == SEG_HRPT_METOPA)
        segment_type = "HRPTMETOPA";
    else if(type == SEG_HRPT_METOPB)
        segment_type = "HRPTMETOPB";
    else if(type == SEG_HRPT_NOAA19)
        segment_type = "HRPTNOAA19";
    else if(type == SEG_HRPT_M01)
        segment_type = "HRPTM01";
    else if(type == SEG_HRPT_M02)
        segment_type = "HRPTM02";

    qDebug() << "HRPT file = " << fileInfo.fileName() << " size = " << fileInfo.size();

    ReadNbrOfLines();
    qDebug() << "Nbr of lines = " << this->NbrOfLines;
    qDebug() << "sensingstart = " << qsensingstart.toString() << " sensingend = " << qsensingend.toString();

    julian_sensing_start = qsensingstart.Julian();
    julian_sensing_end = qsensingend.Julian();

    Satellite *sat;
    bool ok;

    if(segtype == SEG_HRPT_METOPA || segtype == SEG_HRPT_M02)      // Metop-A
        sat = satellitelist.GetSatellite(29499, &ok);
    else if(segtype == SEG_HRPT_METOPB || segtype == SEG_HRPT_M01) // Metop-B
        sat = satellitelist.GetSatellite(38771, &ok);
    else if(segtype == SEG_HRPT_NOAA19)                            // Noaa19
        sat = satellitelist.GetSatellite(33591, &ok);

    line1 = sat->line1;
    line2 = sat->line2;

    qtle.reset(new QTle(fileInfo.fileName().mid(0,15), line1, line2, QTle::wgs72));
    qsgp4.reset(new QSgp4( *qtle ));

    julian_state_vector = qtle->Epoch();

    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MINUTES_PER_DAY;
    minutes_sensing = (julian_sensing_end - julian_sensing_start) * MINUTES_PER_DAY;

    QEci eci;

    qsgp4->getPosition(minutes_since_state_vector, eci);
    QGeodetic geo = eci.ToGeo();

    lon_start_rad = geo.longitude;
    lat_start_rad = geo.latitude;
    lon_start_deg = lon_start_rad * 180.0 / PI;
    lat_start_deg = lat_start_rad * 180.0 /PI;

    CalculateCornerPoints();
    if(segtype == SEG_HRPT_METOPA || segtype == SEG_HRPT_METOPB || segtype == SEG_HRPT_NOAA19)
        CalculateDetailCornerPoints();


}

int SegmentHRPT::ReadNbrOfLines()
{
    int counter = 0;
    int filesize = 1;
    QFile file(fileInfo.filePath());
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    QByteArray filedata;
    filedata.resize(13864);

    filesize = in.readRawData(filedata.data(),filedata.size());
    bool framesyncok = CheckFramesync(filedata);
    while(filesize > 0)
    {
        if(framesyncok)
        {
            if(filesize>0)
            {
                if( filesize != 13864)
                {
                    qDebug() << "filesize != 13864";
                }
                else
                {
                    if(counter == 0)
                        DecodeHRPTDate(filedata, this->qsensingstart);
                    else
                        DecodeHRPTDate(filedata, this->qsensingend);
                    counter++;
                }
            }else
            {
                qDebug() << "Error reading file";
                break;
            }
        }
        filesize=in.readRawData(filedata.data(),filedata.size());
        framesyncok = CheckFramesync(filedata);
    }

    this->NbrOfLines = counter;

    file.close();
    return(counter);
}

Segment *SegmentHRPT::ReadSegmentInMemory()
{
    int heightinsegment = 0;
    int filesize = 1;
    QFile file(fileInfo.filePath());
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    QByteArray filedata;
    filedata.resize(13864);

    filesize = in.readRawData(filedata.data(),filedata.size());
    bool framesyncok = CheckFramesync(filedata);
    while(filesize > 0)
    {
        if(framesyncok)
        {
            if(filesize>0)
            {
                if( filesize != 13864)
                {
                    qDebug() << "filesize != 13864";
                }
                else
                {
                    DecodeHRPTline(filedata, heightinsegment);
                    heightinsegment++;
                }
            }else
            {
                qDebug() << "Error reading file";
                break;
            }
        }
        filesize=in.readRawData(filedata.data(),filedata.size());
        framesyncok = CheckFramesync(filedata);
    }

    file.close();
    return(this);
}

void SegmentHRPT::DecodeHRPTDate(QByteArray badata, QSgp4Date &date)
{
    quint16 val, val1, val2;
    long day, month;

    val1 = 0xFF & badata.at(10);
    val2 = 0xC0 & badata.at(11);
    val=(val1<<2) | (val2>>6);
    quint16 fs1 = val;

    val1 = 0x3F & badata.at(11);
    val2 = 0xF0 & badata.at(12);
    val=(val1<<4) | (val2>>4);
    quint16 fs2 = val;

    val1 = 0x0F & badata.at(12);
    val2 = 0xFC & badata.at(13);
    val=(val1<<6) | (val2>>2);
    quint16 fs3 = val;

    val1 = 0x03 & badata.at(13);
    val2 = 0xFF & badata.at(14);
    val=(val1<<8) | val2;
    quint16 fs4 = val;

    quint16 day_count = fs1 >> 1;
    quint64 msec_of_day = 0x007F & fs2;
    quint64 msec_of_day_count = fs3;
    quint64 remainder_msec_of_day_count = fs4;
    quint64 msecday = (msec_of_day << 20) | (msec_of_day_count << 10) | remainder_msec_of_day_count;

    double hoursday = (double)msecday / (60000.0 * 60.0);
    int hours = (int)hoursday;
    double dminutes = (hoursday - hours)*60.0;
    int minutes = (int)dminutes;
    double dseconds = (dminutes - minutes)*60.0;

    QSgp4Date::DayOfYearToDayAndMonth(day_count, QSgp4Date::IsLeap(this->year, true), day, month);
    date.Set(this->year, month, day, hours, minutes, dseconds, true);

}

bool SegmentHRPT::CheckFramesync(QByteArray badata)
{
    quint16 val, val1, val2;
    quint16 valline[5][2048];


    val1 = 0xFF & badata.at(0);
    val2 = 0xC0 & badata.at(1);
    val=(val1<<2) | (val2>>6);
    quint16 fs1 = val;

    val1 = 0x3F & badata.at(1);
    val2 = 0xF0 & badata.at(2);
    val=(val1<<4) | (val2>>4);
    quint16 fs2 = val;

    val1 = 0x0F & badata.at(2);
    val2 = 0xFC & badata.at(3);
    val=(val1<<6) | (val2>>2);
    quint16 fs3 = val;

    val1 = 0x03 & badata.at(3);
    val2 = 0xFF & badata.at(4);
    val=(val1<<8) | val2;
    quint16 fs4 = val;


    // Frame sync 1 = 644
    //            2 = 367
    //            3 = 860
    //            4 = 413
    //            5 = 527
    //            6 = 149

    if(fs1 == 644 && fs2 == 367 && fs3 == 860 && fs4 == 413)
        return(true);
    else
        return(false);
}

void SegmentHRPT::DecodeHRPTline(QByteArray badata, int heightinsegment)
{

    int n=0;
    int i,j,c,p;

    quint16 val, val1, val2;
    quint16 valline[5][2048];


    val1 = 0xFF & badata.at(0);
    val2 = 0xC0 & badata.at(1);
    val=(val1<<2) | (val2>>6);
    quint16 fs1 = val;

    val1 = 0x3F & badata.at(1);
    val2 = 0xF0 & badata.at(2);
    val=(val1<<4) | (val2>>4);
    quint16 fs2 = val;

    val1 = 0x0F & badata.at(2);
    val2 = 0xFC & badata.at(3);
    val=(val1<<6) | (val2>>2);
    quint16 fs3 = val;

    val1 = 0x03 & badata.at(3);
    val2 = 0xFF & badata.at(4);
    val=(val1<<8) | val2;
    quint16 fs4 = val;


    // Frame sync 1 = 644
    //            2 = 367
    //            3 = 860
    //            4 = 413
    //            5 = 527
    //            6 = 149

    val1 = 0xFF & badata.at(10);
    val2 = 0xC0 & badata.at(11);
    val=(val1<<2) | (val2>>6);
    fs1 = val;

    val1 = 0x3F & badata.at(11);
    val2 = 0xF0 & badata.at(12);
    val=(val1<<4) | (val2>>4);
    fs2 = val;

    val1 = 0x0F & badata.at(12);
    val2 = 0xFC & badata.at(13);
    val=(val1<<6) | (val2>>2);
    fs3 = val;

    val1 = 0x03 & badata.at(13);
    val2 = 0xFF & badata.at(14);
    val=(val1<<8) | val2;
    fs4 = val;

    quint16 day_count = fs1 >> 1;
    quint64 msec_of_day = 0x007F & fs2;
    quint64 msec_of_day_count = fs3;
    quint64 remainder_msec_of_day_count = fs4;
    quint64 msecday = (msec_of_day << 20) | (msec_of_day_count << 10) | remainder_msec_of_day_count;

    double hoursday = (double)msecday / (60000.0 * 60.0);
    int hours = (int)hoursday;
    double dminutes = (hoursday - hours)*60.0;
    int minutes = (int)dminutes;
    double dseconds = (dminutes - minutes)*60.0;
    int seconds = (int)dseconds;

    //if(counter == 0)
    //qDebug() << " msecday = " << msecday << " hours = " << hours << " min = " << minutes << " sec = " << seconds << "desec = " << dseconds;

    val1 = 0xFF & badata.at(935);
    val2 = 0xC0 & badata.at(936);
    val=(val1<<2) | (val2>>6);
    fs1 = val;

    val1 = 0x3F & badata.at(936);
    val2 = 0xF0 & badata.at(937);
    val=(val1<<4) | (val2>>4);
    fs2 = val;

    val1 = 0x0F & badata.at(937);
    val2 = 0xFC & badata.at(938);
    val=(val1<<6) | (val2>>2);
    fs3 = val;

    val1 = 0x03 & badata.at(938);
    val2 = 0xFF & badata.at(939);
    val=(val1<<8) | val2;
    fs4 = val;

    n=935;
    c=0;
    p=0;
    j=0;
    for (i=-2; i<2048*5; )
    {
        QByteArray tmp = badata.mid( n, 5);
        n += 5;

        if (i>=0)
        {
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

}

void SegmentHRPT::ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(inputchannel, LCC);
}

void SegmentHRPT::ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(inputchannel, GVP);
}

void SegmentHRPT::ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(inputchannel, SG);
}

void SegmentHRPT::ComposeProjection(int inputchannel, eProjections proj)
{
    qDebug() << QString("SegmentHRPT::ComposeProjection startLineNbr = %1").arg(this->startLineNbr);
    int startheight = this->startLineNbr;

    initializeProjectionCoord();

    inputchannel = (inputchannel == 0 ? 6 : inputchannel);

    QEci eciref;

    double angular_velocity = TWOPI/qtle->Period(); // period in seconds
    double e = qtle->Eccentricity();
    double epow2 = e * e;
    double epow3 = e * e * e;

    double epochcorrection = 0;
    double yawcorrection = 0.02;

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

        roll *= 2;

        if( nbrLine == 0 || nbrLine == this->NbrOfLines - 1)
            qDebug() << QString("nbrline = %1 Pitch = %2  Roll = %3 Yaw = %4").arg(nbrLine).arg(pitch).arg(roll).arg(yaw);

        this->RenderSegmentlineInProjectionAlternative(inputchannel, nbrLine, startheight + nbrLine, eciref, angular_velocity, pitch, roll, yaw + yawcorrection, proj);
        //this->RenderSegmentlineInProjection(inputchannel, startheight + nbrLine, eciref, angular_velocity, proj );
    }
}

void SegmentHRPT::RenderSegmentlineInProjectionAlternative(int channel, int nbrLine, int heightintotalimage, QEci eciref, double ang_vel, double pitch, double roll, double yaw, eProjections proj)
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
    }
}


SegmentHRPT::~SegmentHRPT()
{

}
