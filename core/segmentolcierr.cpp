#include "segmentolcierr.h"

#include <QDebug>
#include <QMatrix4x4>


SegmentOLCIerr::SegmentOLCIerr(QFile *filesegment, SatelliteList *satl, QObject *parent) :
  Segment(parent)
{


    bool ok;

    satlist = satl;

    fileInfo.setFile(*filesegment);
    segment_type = "OLCIERR";
    segtype = eSegmentType::SEG_OLCIERR;

    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    //0         1         2         3         4         5         6         7         8         9         10
    //S3A_OL_1_ERR____20161026T161414_20161026T161714_20161026T175243_0179_010_168_4319_MAR_O_NR_002.SEN3.tar
    int sensing_start_year = fileInfo.fileName().mid(16, 4).toInt( &ok , 10);
    int sensing_start_month = fileInfo.fileName().mid(20, 2).toInt( &ok, 10);
    int sensing_start_day = fileInfo.fileName().mid(22, 2).toInt( &ok, 10);
    int sensing_start_hour = fileInfo.fileName().mid(25, 2).toInt( &ok, 10);
    int sensing_start_minute = fileInfo.fileName().mid(27, 2).toInt( &ok, 10);
    int sensing_start_second = fileInfo.fileName().mid(29, 2).toInt( &ok, 10);

    int sensing_end_year = fileInfo.fileName().mid(32, 4).toInt( &ok , 10);
    int sensing_end_month = fileInfo.fileName().mid(36, 2).toInt( &ok, 10);
    int sensing_end_day = fileInfo.fileName().mid(38, 2).toInt( &ok, 10);
    int sensing_end_hour = fileInfo.fileName().mid(41, 2).toInt( &ok, 10);
    int sensing_end_minute = fileInfo.fileName().mid(43, 2).toInt( &ok, 10);
    int sensing_end_second = fileInfo.fileName().mid(45, 2).toInt( &ok, 10);

    double d_sensing_start_second = (double)sensing_start_second;
    double d_sensing_end_second = (double)sensing_end_second;

    //this->sensing_start_year = sensing_start_year;
    qdatetime_start.setDate(QDate(sensing_start_year, sensing_start_month, sensing_start_day));
    qdatetime_start.setTime(QTime(sensing_start_hour,sensing_start_minute, sensing_start_second, 0));

    julian_sensing_start = Julian_Date_of_Year(sensing_start_year) +
            DOY( sensing_start_year, sensing_start_month, sensing_start_day ) +
            Fraction_of_Day( sensing_start_hour, sensing_start_minute, d_sensing_start_second )
            + 5.787037e-06; /* Round up to nearest 1 sec */

    julian_sensing_end = Julian_Date_of_Year(sensing_end_year) +
            DOY( sensing_end_year, sensing_end_month, sensing_end_day ) +
            Fraction_of_Day( sensing_end_hour, sensing_end_minute, d_sensing_end_second )
            + 5.787037e-06; /* Round up to nearest 1 sec */


    qsensingstart = QSgp4Date(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, d_sensing_start_second);
    qsensingend = QSgp4Date(sensing_end_year, sensing_end_month, sensing_end_day, sensing_end_hour, sensing_end_minute, d_sensing_end_second);


    this->earth_views_per_scanline = 4865;
    this->NbrOfLines = 4091;

    Satellite s3a;
    ok = satlist->GetSatellite(41335, &s3a);
    line1 = s3a.line1;
    line2 = s3a.line2;

    //line1 = "1 33591U 09005A   11039.40718334  .00000086  00000-0  72163-4 0  8568";
    //line2 = "2 33591  98.8157 341.8086 0013952 344.4168  15.6572 14.11126791103228";
    double epoch = line1.mid(18,14).toDouble(&ok);
    julian_state_vector = Julian_Date_of_Epoch(epoch);

    qtle.reset(new QTle(s3a.sat_name, line1, line2, QTle::wgs72));
    qsgp4.reset(new QSgp4( *qtle ));


    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MIN_PER_DAY; //  + (1.0/12.0) / 60.0;
    minutes_sensing = ( julian_sensing_end - julian_sensing_start ) * MIN_PER_DAY;

    QEci qeci;
    qsgp4->getPosition(minutes_since_state_vector, qeci);
    QGeodetic qgeo = qeci.ToGeo();

    lon_start_rad = qgeo.longitude;
    lat_start_rad = qgeo.latitude;

    lon_start_deg = rad2deg(lon_start_rad);
    if (lon_start_deg > 180)
        lon_start_deg = - (360 - rad2deg(lon_start_rad));

    lat_start_deg = rad2deg(lat_start_rad);


    double hours_since_state_vector = ( julian_sensing_start - julian_state_vector ) * HOURS_PER_DAY;

    // qDebug() << QString("---> lon = %1 lat = %2  hours_since_state_vector = %3").arg(lon_start_deg).arg(lat_start_deg).arg( hours_since_state_vector);

    CalculateCornerPoints();
    CalculateDetailCornerPoints();

    // qDebug() << "Nbr of vector cornerpoints " << this->vectorfirst.count();

}

void SegmentOLCIerr::CalculateDetailCornerPoints()
{


    double statevec = minutes_since_state_vector;
    while(statevec <= minutes_since_state_vector + minutes_sensing)
    {
        setupVector(statevec);
        statevec = statevec + 1.0;
    }

    setupVector(minutes_since_state_vector + minutes_sensing);

}


void SegmentOLCIerr::setupVector(double statevec)
{

    QEci qeci;

    if(qsgp4.isNull())
        qDebug() << "qsgp4 is NULL !!!";
    qsgp4->getPosition(statevec, qeci);
    QGeodetic qgeo = qeci.ToGeo();

    QVector3D pos;
    QVector3D d3pos = qeci.GetPos_f();
    QVector3D d3vel = qeci.GetVel_f();

    QVector3D d3posnorm = d3pos.normalized();
    QMatrix4x4 mat;
    QVector3D d3scan;

    double e = qtle->Eccenticity();
    double epow2 = e * e;
    double epow3 = e * e * e;

    double span = qeci.GetDate().spanSec(qtle->Epoch());
    double M = fmod(qtle->MeanAnomaly() + (TWOPI * (span/qtle->Period())), TWOPI);
    double C = (2*e - epow3/4)*sin(M) + (5*epow2/4)*sin(2*M) + (13*epow3/12)*sin(3*M);
    double trueAnomaly = M + C;
    double PSO = fmod(qtle->ArgumentPerigee() + trueAnomaly, TWOPI);

    double pitch_steering_angle = - 0.002899 * sin( 2 * PSO);
    double roll_steering_angle = 0.00089 * sin(PSO);
    double yaw_factor = 0.068766 * cos(PSO);
    double yaw_steering_angle = 0.068766 * cos(PSO) * (1 - yaw_factor * yaw_factor/3);

    mat.setToIdentity();
    mat.rotate(yaw_steering_angle * 180/PI, d3pos);  // yaw
    mat.rotate(roll_steering_angle * 180/PI, d3vel); // roll
    mat.rotate(pitch_steering_angle * 180/PI, QVector3D::crossProduct(d3pos,d3vel)); // pitch
    d3scan = mat * QVector3D::crossProduct(d3pos,d3vel);

    //d3scan = QVector3D::crossProduct(d3pos,d3vel);

    QVector3D d3scannorm = d3scan.normalized();

    double delta2 = 23.0 * PI / 180.0;
    double delta1 = 47.0 * PI / 180.0;


    double r = d3pos.length();
    double sindelta = sin(-delta1);
    double cosdelta = cos(-delta1);
    double dd = r * cosdelta - sqrt(XKMPER * XKMPER - r * r * sindelta * sindelta);
    QVector3D d3d = - d3posnorm * cosdelta * dd + d3scannorm * sindelta * dd;
    QVector3D d3earthposfirst = d3pos + d3d;

    QEci qecifirst(d3earthposfirst, d3vel, qsensingstart);
    vectorfirst.append(qecifirst.ToGeo());

    sindelta = sin(delta2);
    cosdelta = cos(delta2);
    dd = r * cosdelta - sqrt(XKMPER * XKMPER - r * r * sindelta * sindelta);
    d3d = - d3posnorm * cosdelta * dd + d3scannorm * sindelta * dd;

    QVector3D d3earthposlast = d3pos + d3d;

    QEci qecilast(d3earthposlast, d3vel, qsensingstart);
    vectorlast.append(qecilast.ToGeo());
}

SegmentOLCIerr::~SegmentOLCIerr()
{

}
