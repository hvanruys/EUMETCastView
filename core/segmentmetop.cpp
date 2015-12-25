#include "segmentmetop.h"

#include "sgp4sdp4.h"
#include "globals.h"
#include "segmentimage.h"


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
#include <QDebug>
#include <QBitArray>
#include <QByteArray>

extern Options opts;
extern SegmentImage *imageptrs;
#include <QMutex>

extern QMutex g_mutex;

SegmentMetop::SegmentMetop(QFile *filesegment, SatelliteList *satl, QObject *parent) :
    Segment(parent)
{
    bool ok;
    satlist = satl;
    fileInfo.setFile(*filesegment);

    this->satname = fileInfo.baseName().mid(12, 3);

    segment_type = "Metop";
    segtype = eSegmentType::SEG_METOP;
    this->earth_views_per_scanline = 2048;

    //AVHR_xxx_1B_M01_20130701051903Z_20130701052203Z_N_O_20130701054640Z

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
    qsensingend.AddMin(3.0);

    julian_sensing_start = qsensingstart.Julian();
    julian_sensing_end = qsensingend.Julian();

    Satellite metop_sat;

    if(fileInfo.fileName().mid(0,15) == "AVHR_xxx_1B_M02")  // Metop-A
        ok = satlist->GetSatellite(29499, &metop_sat);
    else if(fileInfo.fileName().mid(0,15) == "AVHR_xxx_1B_M01") // Metop-B
        ok = satlist->GetSatellite(38771, &metop_sat);

    if(!ok)
    {
        qInfo() << "EUMETCastView needs TLE's";
        return;
    }

    line1 = metop_sat.line1;
    line2 = metop_sat.line2;

    qtle.reset(new QTle(fileInfo.fileName().mid(0,15), line1, line2, QTle::wgs72));
    qsgp4.reset(new QSgp4( *qtle ));

    julian_state_vector = qtle->Epoch();

    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MINUTES_PER_DAY;
    minutes_sensing = 3;

    QEci eci;

    qsgp4->getPosition(minutes_since_state_vector, eci);
    QGeodetic geo = eci.ToGeo();

    lon_start_rad = geo.longitude;
    lat_start_rad = geo.latitude;
    lon_start_deg = lon_start_rad * 180.0 / PI;
    lat_start_deg = lat_start_rad * 180.0 /PI;

    NbrOfLines = 1080;

    CalculateCornerPoints();
}

SegmentMetop::~SegmentMetop()
{
}

void SegmentMetop::SegmentMetopBz2(QFile &filesegment)
{
    FILE*   f;
    BZFILE* b;
    int     nBuf;
    char    buf[26660];
    char    bufheader[20];
    int     bzerror;
    quint64 nextrecordlength = 0;
    quint32 nextres = 0;

    f = fopen( filesegment.fileName().toLatin1(), "rb" );
    if ( !f )
    {
        qDebug() << QString("file %1 not found ! ").arg(this->fileInfo.absoluteFilePath());
        segmentok = false;
        return;
    }

    if((b = BZ2_bzopen(filesegment.fileName().toLatin1(),"rb"))==NULL)
    {
        segmentok = false;
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
         if(get_next_header(dataheader, &nextres))
         {
            nextrecordlength += nextres;
            nBuf = BZ2_bzRead ( &bzerror, b, buf, nextres - 20 );
            if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
            {
                if ((dataheader.at(0) & 0xFF) == 0x01)
                {
                    QByteArray mphr_record = QByteArray::fromRawData(buf, nBuf);
                    inspectMPHRrecord(mphr_record);
                    break;
                }
             }
         }
         else
         {
             segmentok = false;
             qDebug() << QString("Bad file ! = %1").arg(filesegment.fileName());
         }
      }
      else
      {
          segmentok = false;
          qDebug() << QString("Bad compression ! = %1").arg(filesegment.fileName());
      }

    }

    BZ2_bzclose ( b );
    fclose(f);

}


void SegmentMetop::inspectMPHRrecord(QByteArray mphr_record)
{

    bool ok;

/*            qDebug() <<  mphr_record.mid(0, 99);
            qDebug() <<  mphr_record.mid(100, 99);
            qDebug() <<  mphr_record.mid(200, 99);
            qDebug() <<  mphr_record.mid(300, 99);
            qDebug() <<  mphr_record.mid(400, 99);

            qDebug() <<  mphr_record.mid(500, 36);
            qDebug() <<  mphr_record.mid(537, 35);
            qDebug() <<  mphr_record.mid(573, 35);
            qDebug() <<  mphr_record.mid(609, 34);
            qDebug() <<  mphr_record.mid(644, 35);

            qDebug() <<  mphr_record.mid(1477, 51);

            qDebug() <<  mphr_record.mid(2364, 43);
            qDebug() <<  mphr_record.mid(2408, 43);
            qDebug() <<  mphr_record.mid(2452, 43);
            qDebug() <<  mphr_record.mid(2496, 43);

            qDebug() << mphr_record.mid(1225, 35);  // receiving_ground_station
*/
    lat_start_deg = mphr_record.mid( 2396, 11).toDouble()/1000;
    lat_start_rad = deg2rad( lat_start_deg );
    lon_start_deg = mphr_record.mid( 2440, 11).toDouble()/1000;
    lon_start_rad = deg2rad( lon_start_deg );

    lat_end_deg = mphr_record.mid( 2484, 11).toDouble()/1000;
    lat_end_rad = deg2rad( lat_end_deg );
    lon_end_deg = mphr_record.mid( 2528, 11).toDouble()/1000;
    lon_end_rad = deg2rad( lon_end_deg );

    //            qDebug() << QString(" lat start = %1").arg(lat_start_deg, 0, 'f', 3);
    //            qDebug() << QString(" lon start = %1").arg(lon_start_deg, 0, 'f', 3);
    //            qDebug() << QString(" lat end = %1").arg(lat_end_deg, 0, 'f', 3);
    //            qDebug() << QString(" lon end = %1").arg(lon_end_deg, 0, 'f', 3);


    state_vector_year = mphr_record.mid(1509, 4).toInt( &ok, 10);
    state_vector_month = mphr_record.mid(1513, 2).toInt( &ok, 10);
    state_vector_day = mphr_record.mid(1515, 2).toInt( &ok, 10);
    state_vector_hour = mphr_record.mid(1517, 2).toInt( &ok, 10);
    state_vector_minute = mphr_record.mid(1519, 2).toInt( &ok, 10);
    state_vector_second = mphr_record.mid(1521, 2).toInt( &ok, 10);

    int day_of_year = DOY( state_vector_year, state_vector_month, state_vector_day );
    double fraction_of_day = Fraction_of_Day( state_vector_hour, state_vector_minute, state_vector_second );
    //state_vector = (double)day_of_year + fraction_of_day;

    double semi_major_axis = mphr_record.mid(1560, 11).toDouble()/1000000;  // in KM
    double eccentricity = mphr_record.mid(1604, 11).toDouble()/1000000;
    double inclination = mphr_record.mid(1648, 11).toDouble()/1000;
    double perigee_argument = mphr_record.mid(1692, 11).toDouble()/1000;
    double right_ascension = mphr_record.mid(1736, 11).toDouble()/1000;
    double mean_anomaly = mphr_record.mid(1780, 11).toDouble()/1000;
    quint32 orbit_start =  mphr_record.mid(1389, 5).toInt( &ok, 10);

    // qDebug() << QString("inclination = %1").arg(inclination, 0, 'f', 6);

    //double mean_motion_period = 0.0099520140503 * pow( semi_major_axis, 3/2 );
    double mean_motion_period = TWOPI * pow( pow( semi_major_axis, 3 ) / 398600.4418, 0.5); // in seconds
    //qDebug() << QString("mean motion period = %1").arg(mean_motion_period, 0, 'f', 6);
    double revs_per_day = SEC_PER_DAY / mean_motion_period;
    //qDebug() << QString("revs_per_day = %1").arg(revs_per_day, 0, 'f', 6);
    // quint64 state_vector_time = mphr_record.mid(1509, 17).toLongLong( &ok, 10);  // state vector time

    subsat_latitude_start = mphr_record.mid(2396, 11).toDouble()/1000;
    subsat_longitude_start = mphr_record.mid(2440, 11).toDouble()/1000;
    subsat_latitude_end = mphr_record.mid(2484, 11).toDouble()/1000;
    subsat_longitude_end = mphr_record.mid(2528, 11).toDouble()/1000;
    //                qDebug() <<  mphr_record.mid(2364, 43) << " ; " << subsat_latitude_start; // subsat latitude start
    //                qDebug() <<  mphr_record.mid(2408, 43) << " ; " << subsat_longitude_start; // subsat longitude start
    //                qDebug() <<  mphr_record.mid(2452, 43) << " ; " << subsat_latitude_end; // subsat latitude end
    //                qDebug() <<  mphr_record.mid(2496, 43) << " ; " << subsat_longitude_end; // subsat longitude end


    //            qDebug() <<  mphr_record.mid(1528, 43); // semi major axis
    //            qDebug() <<  mphr_record.mid(1572, 43); // eccentricity
    //            qDebug() <<  mphr_record.mid(1616, 43); // inclination
    //            qDebug() <<  mphr_record.mid(1660, 43); // perigee_argument
    //            qDebug() <<  mphr_record.mid(1704, 43); // right_ascension
    //            qDebug() <<  mphr_record.mid(1748, 43); // mean anomaly
    //
    //            qDebug() <<  mphr_record.mid(1357, 37); // orbit_start
    //            qDebug() <<  mphr_record.mid(1395, 37); // orbit_end
    //nextrecordlength
    // qDebug() <<  mphr_record.mid(712, 15); // sensing_start
    // qDebug() <<  mphr_record.mid(760, 15); // sensing_end

    // qDebug() <<  mphr_record.mid(680, 47); // sensing_start
    // qDebug() <<  mphr_record.mid(728, 47); // sensing_end
    // qDebug() << "-----------------------";
    // 1 NNNNNU NNNNNAAA NNNNN.NNNNNNNN +.NNNNNNNN +NNNNN-N +NNNNN-N N NNNNN
    // 1234567890123456789012345678901234567890123456789012345678901234567890
    // METOP-A
    // 1 29499U 06044A   09193.45016836 -.00000150  00000-0 -48574-4 0  8553
    // 1 12345U 99001    14229.25857639 -.00000000  00000-0  00000-0 0  8553"

    const QChar fill = '0';
    double total_time = day_of_year + fraction_of_day;

    line1.append( "1 12345U 99001    ");
    line1.append( mphr_record.mid(1511, 2) );
    line1.append(QString("%1").arg( total_time, 12, 'f', 8));
    line1.append(" -.00000000  00000-0  00000-0 0  8553");

    //qDebug() << "line 1 = " << line1;

    line2.append( "2 12345 " );

    line2.append(QString("%1").arg( inclination, 8, 'f', 4));
    line2.append( " " );
    line2.append(QString("%1").arg( right_ascension, 8, 'f', 4));
    line2.append( " " );
    QString ecc_str = QString("%1").arg( eccentricity/10, 8, 'f', 7);

    line2.append(QString("%1").arg( ecc_str.mid(2) ));
    line2.append( " " );
    line2.append(QString("%1").arg( perigee_argument, 8, 'f', 4));

    line2.append( " " );
    line2.append(QString("%1").arg( mean_anomaly, 8, 'f', 4));
    line2.append( " " );

    line2.append(QString("%1").arg( revs_per_day, 11, 'f', 8));
    line2.append(QString("%1").arg( orbit_start, 5, 10, fill));
    line2.append( "0" );


    qtle.reset(new QTle("Metop", line1, line2, QTle::wgs72));
    qsgp4.reset(new QSgp4( *qtle ));

    int sensing_start_year = mphr_record.mid(712, 4).toInt( &ok, 10);
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

    QDateTime sensing_start(QDate(sensing_start_year,sensing_start_month, sensing_start_day), QTime(sensing_start_hour, sensing_start_minute, sensing_start_second));
    QDateTime sensing_end(QDate(sensing_end_year,sensing_end_month, sensing_end_day), QTime(sensing_end_hour, sensing_end_minute, sensing_end_second));

    // julian_state_vector = Julian_Date_of_Year(sensing_start_year) + (double)day_of_year + fraction_of_day;
    julian_state_vector = qtle->Epoch();
/*
    julian_sensing_start = Julian_Date_of_Year(sensing_start_year) +
            DOY( sensing_start_year, sensing_start_month, sensing_start_day ) +
            Fraction_of_Day( sensing_start_hour, sensing_start_minute, sensing_start_second );

    julian_sensing_end = Julian_Date_of_Year(sensing_end_year) +
            DOY( sensing_end_year, sensing_end_month, sensing_end_day ) +
            Fraction_of_Day( sensing_end_hour, sensing_end_minute, sensing_end_second );
*/
    //sensingstart = cJulian(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, sensing_start_second);
    //sensingend = cJulian(sensing_end_year, sensing_end_month, sensing_end_day, sensing_end_hour, sensing_end_minute, sensing_end_second);

    qsensingstart = QSgp4Date(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, sensing_start_second);
    qsensingend = QSgp4Date(sensing_end_year, sensing_end_month, sensing_end_day, sensing_end_hour, sensing_end_minute, sensing_end_second);

    julian_sensing_start = qsensingstart.Julian();
    julian_sensing_end = qsensingend.Julian();

    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MIN_PER_DAY;
    minutes_sensing = (double)(sensing_start.secsTo(sensing_end))/60;

    qdatetime_start.setDate(QDate(sensing_start_year, sensing_start_month, sensing_start_day));
    qdatetime_start.setTime(QTime(sensing_start_hour,sensing_start_minute, sensing_start_second));
    NbrOfLines = mphr_record.mid(2969, 4).toInt( &ok, 10 );

    // qDebug() << "Bz2 file " + this->segment_name + " ; " + this->absolute_path + " is open " << QString::number(NbrOfLines);

    //qDebug() <<  mphr_record.mid(2969, 4); //nbr of mdr records
    //qDebug() << QString("nbr of mdr records = %1").arg(NbrOfLines);

    //qDebug() << QString("minutes since state vector %1").arg(minutes_since_state_vector);
    //qDebug() << QString("minutes_sensing = %1").arg(minutes_sensing);
    //qDebug() << QString("sensing start = %1").arg(sensing_start, 0, 'f', 8);
    //qDebug() << QString("sensing_start_year = %1").arg(sensing_start_year, 0, 'f', 8);
    //qDebug() << "------";
    //qDebug() << QString("julian_sensing_start  = %1").arg(julian_sensing_start);

    CalculateCornerPoints();
}

int SegmentMetop::ReadNbrOfLines()
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

void SegmentMetop::inspectSolarAngle(QByteArray *mdr_record)
{

    //quint16 val1_ch[5], val2_ch[5],tot_ch[5];
    //QByteArray picture_line;
    quint16 num1=0, num2=0;

    Q_ASSERT( mdr_record->length() == 26640);

    QByteArray earth_views_per_scanline = mdr_record->mid( 2, 2 );

    num1 = 0XFF & earth_views_per_scanline.at(0);   // 0X8FFF;
    num2 = 0XFF & earth_views_per_scanline.at(1);

    quint16 earth_views  = (num1 <<= 8) | num2;
    this->earth_views_per_scanline = earth_views;

    num1 = 0xFF & mdr_record->at(20534);
    num2 = 0xFF & mdr_record->at(20535);
    quint16 num_navigation_points  = (num1 <<= 8) | num2;

    //qDebug() << QString("earth views per scanline = %1 num navigation points = %2").arg(earth_views).arg(num_navigation_points);


    qint16 solar_zenith_angle[103];
    qint16 satellite_zenith_angle[103];
    qint16 solar_azimuth_angle[103];
    qint16 satellite_azimuth_angle[103];

    // nbr_navigation_points = 103
    // 5 + (20 x {0 ... 102})

    // nbr_navigation_points = 51
    // 25 + (40 x {0 ... 50})

    double dsolar;
    for(int i = 0; i < num_navigation_points; i++)
    {
        num1 = 0xFF & mdr_record->at(20536 + i*8);
        num2 = 0xFF & mdr_record->at(20537 + i*8);
        solar_zenith_angle[i] = (num1 <<= 8) | num2;

        num1 = 0xFF & mdr_record->at(20538 + i*8);
        num2 = 0xFF & mdr_record->at(20539 + i*8);
        satellite_zenith_angle[i] = (num1 <<= 8) | num2;

        num1 = 0xFF & mdr_record->at(20540 + i*8);
        num2 = 0xFF & mdr_record->at(20541 + i*8);
        solar_azimuth_angle[i] = (num1 <<= 8) | num2;

        num1 = 0xFF & mdr_record->at(20542 + i*8);
        num2 = 0xFF & mdr_record->at(20543 + i*8);
        satellite_azimuth_angle[i] = (num1 <<= 8) | num2;

//        for( int s = 5 + 20*i; (s <= 5 + 20*(i+1)) && (s <= 2048); s++)
//        {
//            dsolar = (double)solar_zenith_angle[i]/9000;
//            solar_factor[s] = (double)2 * dsolar + 1;
//        }

    }

//    for( int s = 0; s < 5; s++)
//    {
//        dsolar = (double)solar_zenith_angle[0]/9000;
//        solar_factor[s] = (double)2 * dsolar + 1;
//    }



    //qDebug() << QString("sol zen %1 sat zen %2 sol az %3 sat az %4").arg((double)solar_zenith_angle[52]/100).arg((double)satellite_zenith_angle[52]/100).arg((double)solar_azimuth_angle[52]/100).arg((double)satellite_azimuth_angle[52]/100);
    //qDebug() << QString("%1 %2 %3 %4 || %5 %6 %7 %8 || ...").arg(solar_zenith_angle[0]).arg(satellite_zenith_angle[0]).arg(solar_azimuth_angle[0]).arg(satellite_azimuth_angle[0])
    //            .arg(solar_zenith_angle[1]).arg(satellite_zenith_angle[1]).arg(solar_azimuth_angle[1]).arg(satellite_azimuth_angle[1]);
    //qDebug() << QString("lat %1 lon %2  || lat %3 lon %4 || ...").arg(earth_location_lat[0]).arg(earth_location_lon[0])
    //            .arg(earth_location_lat[102]).arg(earth_location_lon[102]);
   //qDebug() << QString("%1 solzen %2 %3 %4").arg(heightinsegment).arg((double)solar_zenith_angle[0]/100).arg((double)solar_zenith_angle[52]/100).arg((double)solar_zenith_angle[102]/100);


}



void SegmentMetop::inspectEarthLocations(QByteArray *mdr_record, int heightinsegment)
{

    quint16 num1=0, num2=0;
    qint32 num32_1=0, num32_2=0, num32_3=0, num32_4=0;

    Q_ASSERT( mdr_record->length() == 26640);

    QByteArray earth_views_per_scanline = mdr_record->mid( 2, 2 );

    num1 = 0XFF & earth_views_per_scanline.at(0);   // 0X8FFF;
    num2 = 0XFF & earth_views_per_scanline.at(1);

    quint16 earth_views  = (num1 <<= 8) | num2;
    this->earth_views_per_scanline = earth_views;

    num1 = 0xFF & mdr_record->at(20534);
    num2 = 0xFF & mdr_record->at(20535);
    num_navigation_points  = (num1 <<= 8) | num2;

    char digital_b_data_1 = 0xFF & mdr_record->at(26590);
    char digital_b_data_2 = 0xFF & mdr_record->at(26591);

    QByteArray digital_b_data;
    QBitArray bitarray(8);
    for(int b=0; b<8;b++)
    {
       bitarray.setBit(b, digital_b_data_2&(1<<(7-b)));
    }

    channel_3a_3b[heightinsegment] = bitarray.testBit(0); // 0 = channel 3b , 1 = channel 3a
                                                          // channel 3a = visible channel

//    digital_b_data.append(digital_b_data_1);
//    digital_b_data.append(digital_b_data_2);
//    QBitArray bits(16);
//    for(int i=0; i<2; ++i) {
//        for(int b=0; b<8;b++) {
//            bits.setBit(i*8+b, digital_b_data.at(i)&(1<<(7-b)));
//        }
//    }

//    bool channel_3a_3b = bits.at(8);

    //qDebug() << QString("-----------length qbytearray = %1").arg(digital_b_data.length());


    long llat_deg, llon_deg;

    for(int i = 0; i < num_navigation_points; i++)
    {

        num32_1 = 0xFF & mdr_record->at(21360 + i*8);
        num32_2 = 0xFF & mdr_record->at(21361 + i*8);
        num32_3 = 0xFF & mdr_record->at(21362 + i*8);
        num32_4 = 0xFF & mdr_record->at(21363 + i*8);
        llat_deg = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
        earthloc_lat[heightinsegment*103 + i] = (float)llat_deg/10000;

        num32_1 = 0xFF & mdr_record->at(21364 + i*8);
        num32_2 = 0xFF & mdr_record->at(21365 + i*8);
        num32_3 = 0xFF & mdr_record->at(21366 + i*8);
        num32_4 = 0xFF & mdr_record->at(21367 + i*8);
        llon_deg = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
        earthloc_lon[heightinsegment*103 + i] = (float)llon_deg/10000;
    }

    if(num_navigation_points == 103 && (heightinsegment == 0 || heightinsegment == 540 || heightinsegment == 1079))
    {
        qDebug() << QString("------>IEL height = %1  lon[0] = %2, lon[52] = %3, lon[102] = %4").arg(heightinsegment).arg( earthloc_lon[heightinsegment*103]).arg( earthloc_lon[heightinsegment*103 + 52]).arg( earthloc_lon[heightinsegment * 103 + 102]);
        qDebug() << QString("------>IEL height = %1  lat[0] = %2, lat[52] = %3, lat[102] = %4").arg(heightinsegment).arg( earthloc_lat[heightinsegment*103]).arg( earthloc_lat[heightinsegment*103 + 52]).arg( earthloc_lat[heightinsegment * 103 + 102]);
    }

}

quint32 SegmentMetop::get_next_header( QByteArray ba )
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

bool SegmentMetop::get_next_header( QByteArray ba, quint32 *reclength )
{

    QString rectype;
    if (ba.at(0) == 0x01)
    {
        rectype = "MPHR";
        cnt_mphr++;
    }
    else if (ba.at(0) == 0x02)
    {
        rectype = "SPHR";
        cnt_sphr++;
    }
    else if (ba.at(0) == 0x03)
    {
        rectype = "IPR";
        cnt_ipr++;
    }
    else if (ba.at(0) == 0x04)
    {
        rectype = "GEADR";
        cnt_geadr++;
    }
    else if (ba.at(0) == 0x05)
    {
        rectype = "GIADR";
        cnt_giadr++;
    }
    else if (ba.at(0) == 0x06)
    {
        rectype = "VEADR";
        cnt_veadr++;
    }
    else if (ba.at(0) == 0x07)
    {
        rectype = "VIADR";
        cnt_viadr++;
    }
    else if (ba.at(0) == 0x08)
    {
        rectype = "MDR";
        cnt_mdr++;
    }
    else
    {
        cnt_unknown++;
        return false;
    }

    QByteArray tes = ba.mid(4, 4);
    quint32 num1=0, num2=0, num3=0, num4=0;

    num1 = 0XFF & tes.at(0);   // 0X8FFF;
    num2 = 0XFF & tes.at(1);
    num3 = 0XFF & tes.at(2);   // 0X8FFF;
    num4 = 0XFF & tes.at(3);

    quint32 recl = (num1 <<= 24) | (num2 <<= 16) | (num3 <<= 8) | num4;

    *reclength = recl;

    //qDebug() << QString("de nieuwe recordlengte = %1 class = %2 record_class = %3 instrument_group = %4 subclass_version = %5")
    //            .arg(reclengte, 0, 10).arg(rectype).arg(ba.at(0), 0, 16).arg(ba.at(1), 0, 16).arg(ba.at(2), 0, 16);
    return true;

}

//void SegmentMetop::RenderEarthLocationsGL()
//{
//    //glPushAttrib(GL_LIGHTING_BIT);
//    //glDisable(GL_LIGHTING);
//    glColor3ub(255, 255, 0);

//    RenderSegmentContourline(earthloc_lat[0*103 + 51] * PI/180.0, earthloc_lon[0*103 + 51] * PI/180.0, earthloc_lat[1079*103 + 51] * PI/180.0, earthloc_lon[1079*103 + 51] * PI/180.0);
//    RenderSegmentContourline(earthloc_lat[0*103 + 0] * PI/180.0, earthloc_lon[0*103 + 0] * PI/180.0, earthloc_lat[1079*103 + 0] * PI/180.0, earthloc_lon[1079*103 + 0] * PI/180.0);
//    RenderSegmentContourline(earthloc_lat[0*103 + 102] * PI/180.0, earthloc_lon[0*103 + 102] * PI/180.0, earthloc_lat[1079*103 + 102] * PI/180.0, earthloc_lon[1079*103 + 102] * PI/180.0);
//    //RenderSegmentContourline(earth_loc_lat[0][0] * PI/180.0, earth_loc_lon[0][0] * PI/180.0, earth_loc_lat[0][102] * PI/180.0, earth_loc_lon[0][102] * PI/180.0);
//    //RenderSegmentContourline(earth_loc_lat[1079][0] * PI/180.0, earth_loc_lon[1079][0] * PI/180.0, earth_loc_lat[1079][102] * PI/180.0, earth_loc_lon[1079][102] * PI/180.0);

//    //glPopAttrib();


//}


void SegmentMetop::RenderSegmentlineInTexture( int channel, int nbrLine, int nbrTotalLine )
{
    RenderSegmentlineInTextureRad( channel, this->earth_loc_lat_first[nbrLine], earth_loc_lon_first[nbrLine],
                                   earth_loc_lat_last[nbrLine], earth_loc_lon_last[nbrLine], earth_loc_altitude[nbrLine], nbrTotalLine);
}

Segment *SegmentMetop::ReadSegmentInMemory()
{
    FILE*   f;
    BZFILE* b;
    int     nBuf;
    char    buf[26660];
    char    bufheader[20];
    int     bzerror;
    quint64 nextrecordlength = 0;
    quint32 nextres = 0;
    quint32 num32_1=0, num32_2=0, num32_3=0, num32_4=0;
    quint16 val1_ch[5], val2_ch[5],tot_ch[5];
    QByteArray picture_line;

    int heightinsegment = 0;

    f = fopen( this->fileInfo.absoluteFilePath().toLatin1(), "rb" );
    if ( !f )
    {
        qDebug() << QString("file %1 not found ! ").arg(this->fileInfo.absoluteFilePath());
        this->segmentok = false;
        return this;
    }

    qDebug() << "Bz2 file " + this->fileInfo.absoluteFilePath() + " is open";

/*    b = BZ2_bzReadOpen ( &bzerror, f, 0, NULL, 0, 0 );
    if ( bzerror != BZ_OK )
    {
        BZ2_bzReadClose ( &bzerror, b );
        qDebug() << "error in BZ2_ReadOpen";
    }
*/
    if((b = BZ2_bzopen(this->fileInfo.absoluteFilePath().toLatin1(),"rb"))==NULL)
    {
        this->segmentok = false;
        qDebug() << "error in BZ2_bzopen";
        return this;
    }

    bzerror = BZ_OK;

    earthloc_lon.reset(new float[1080*103]);
    earthloc_lat.reset(new float[1080*103]);

    while ( bzerror == BZ_OK )
    {
      nBuf = BZ2_bzRead ( &bzerror, b, bufheader, 20 );
      //qDebug() << QString("nBuf header = %1").arg(nBuf);

      if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
      {
         QByteArray dataheader = QByteArray::fromRawData(bufheader, sizeof(bufheader));
         if(get_next_header(dataheader, &nextres))
         {
            nextrecordlength += nextres;
            nBuf = BZ2_bzRead ( &bzerror, b, buf, nextres - 20 );
            if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
            {
                if ((dataheader.at(0) & 0xFF) == 0x01)
                {
                    QByteArray mphr_record = QByteArray::fromRawData(buf, nBuf);
                    inspectMPHRrecord(mphr_record);
                }

                if ((dataheader.at(0) & 0xFF) == 0x08 && nextres == 26660)
                {
                    QByteArray mdr_record = QByteArray::fromRawData(buf, nBuf);
                    //qDebug() << QString("line at 0 = mdr heightintotalimage = %1").arg(heightintotalimage);
                    //qDebug() << QString("mdr_record length = %1").arg(mdr_record.length());
                    //inspectSolarAngle(&mdr_record);
                    inspectEarthLocations(&mdr_record, heightinsegment);

                    //mdr_record = QByteArray::fromRawData(buf, nBuf);

                    QByteArray earth_views_per_scanline = mdr_record.mid( 2, 2 );
                    picture_line = mdr_record.mid( 4, 20480 );

                    for (int i=0, j=0; i < 4096; i+=2, j++)
                    {

                        val1_ch[0] = 0xFF & picture_line.at(i);
                        val2_ch[0] = 0xFF & picture_line.at(i+1);

                        val1_ch[1] = 0xFF & picture_line.at(i+4096);
                        val2_ch[1] = 0xFF & picture_line.at(i+1+4096);

                        val1_ch[2] = 0xFF & picture_line.at(i+8192);
                        val2_ch[2] = 0xFF & picture_line.at(i+1+8192);

                        val1_ch[3] = 0xFF & picture_line.at(i+12288);
                        val2_ch[3] = 0xFF & picture_line.at(i+1+12288);

                        val1_ch[4] = 0xFF & picture_line.at(i+16384);
                        val2_ch[4] = 0xFF & picture_line.at(i+1+16384);

                        if(val1_ch[1] == 255 && val1_ch[0] == 0)
                        {
                            val1_ch[1] = 0;
                            val2_ch[1] = 0;
                        }
                        if(val1_ch[2] == 255 && val1_ch[0] == 0)
                        {
                            val1_ch[2] = 0;
                            val2_ch[2] = 0;
                        }

                        tot_ch[0] = (val1_ch[0] <<= 8) | val2_ch[0];
                        tot_ch[1] = (val1_ch[1] <<= 8) | val2_ch[1];
                        tot_ch[2] = (val1_ch[2] <<= 8) | val2_ch[2];
                        tot_ch[3] = (val1_ch[3] <<= 8) | val2_ch[3];
                        tot_ch[4] = (val1_ch[4] <<= 8) | val2_ch[4];

                        *(this->ptrbaChannel[0].data() + heightinsegment * 2048 + j) = tot_ch[0];
                        *(this->ptrbaChannel[1].data() + heightinsegment * 2048 + j) = tot_ch[1];
                        *(this->ptrbaChannel[2].data() + heightinsegment * 2048 + j) = tot_ch[2];
                        *(this->ptrbaChannel[3].data() + heightinsegment * 2048 + j) = tot_ch[3];
                        *(this->ptrbaChannel[4].data() + heightinsegment * 2048 + j) = tot_ch[4];


                        for (int k=0; k < 5; k++)
                        {
                            if (tot_ch[k] < stat_min_ch[k] )
                                stat_min_ch[k] = tot_ch[k];
                            if (tot_ch[k] > stat_max_ch[k] )
                                stat_max_ch[k] = tot_ch[k];
//                            if(k == 3)
//                            {
//                                if(channel_3a_3b[heightinsegment] == false)
//                                {
//                                if (tot_ch[k] < stat_3_0_min_ch )
//                                    stat_3_0_min_ch = tot_ch[k];
//                                if (tot_ch[k] > stat_3_0_max_ch )
//                                    stat_3_0_max_ch = tot_ch[k];
//                                }
//                                else
//                                {
//                                    if (tot_ch[k] < stat_3_1_min_ch )
//                                        stat_3_1_min_ch = tot_ch[k];
//                                    if (tot_ch[k] > stat_3_1_max_ch )
//                                        stat_3_1_max_ch = tot_ch[k];
//                                }
//                            }
                        }

                    }

                    num32_1 = 0xFF & mdr_record.at(20498);
                    num32_2 = 0xFF & mdr_record.at(20499);
                    num32_3 = 0xFF & mdr_record.at(20500);
                    num32_4 = 0xFF & mdr_record.at(20501);
                    qint32 loc_alt = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
                    earth_loc_altitude[heightinsegment] = (double)(loc_alt /10);

                    num32_1 = 0xFF & mdr_record.at(20518);
                    num32_2 = 0xFF & mdr_record.at(20519);
                    num32_3 = 0xFF & mdr_record.at(20520);
                    num32_4 = 0xFF & mdr_record.at(20521);
                    qint32 loc_lat_first = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
                    earth_loc_lat_first[heightinsegment] = (double)(loc_lat_first * PI/1800000);

                    num32_1 = 0xFF & mdr_record.at(20522);
                    num32_2 = 0xFF & mdr_record.at(20523);
                    num32_3 = 0xFF & mdr_record.at(20524);
                    num32_4 = 0xFF & mdr_record.at(20525);
                    qint32 loc_lon_first = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
                    earth_loc_lon_first[heightinsegment] = (double)(loc_lon_first * PI/1800000);

                    num32_1 = 0xFF & mdr_record.at(20526);
                    num32_2 = 0xFF & mdr_record.at(20527);
                    num32_3 = 0xFF & mdr_record.at(20528);
                    num32_4 = 0xFF & mdr_record.at(20529);
                    qint32 loc_lat_last = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
                    earth_loc_lat_last[heightinsegment] = (double)(loc_lat_last * PI/1800000);

                    num32_1 = 0xFF & mdr_record.at(20530);
                    num32_2 = 0xFF & mdr_record.at(20531);
                    num32_3 = 0xFF & mdr_record.at(20532);
                    num32_4 = 0xFF & mdr_record.at(20533);
                    qint32 loc_lon_last = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
                    earth_loc_lon_last[heightinsegment] = (double)(loc_lon_last * PI/1800000);

                    heightinsegment++;
                 }
             }
         }
      }
    }

    BZ2_bzclose ( b );
    fclose(f);

    return this;
}

void SegmentMetop::initializeProjectionCoord()
{
    projectionCoordX.reset(new int[1080 * 2048]);
    projectionCoordY.reset(new int[1080 * 2048]);
    projectionCoordValue.reset(new QRgb[1080 * 2048]);

    for( int i = 0; i < 1080; i++)
    {
        for( int j = 0; j < 2048 ; j++ )
        {
            projectionCoordX[i * 2048 + j] = 65535;
            projectionCoordY[i * 2048 + j] = 65535;
            projectionCoordValue[i * 2048 + j] = qRgba(0, 0, 0, 0);
        }
    }

}

void SegmentMetop::ComposeSegmentGVProjection(int inputchannel)
{

    qDebug() << QString("ComposeSegmentGVProjection startLineNbr = %1").arg(this->startLineNbr);
    int startheight = this->startLineNbr;

    initializeProjectionCoord();

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        this->RenderSegmentlineInGVP( (inputchannel == 0 ? 6 : inputchannel), line, startheight + line );
    }

    QApplication::processEvents();

}

void SegmentMetop::RenderSegmentlineInGVP( int channel, int nbrLine, int heightintotalimage )
{
    double lonpos1, latpos1;
    double map_x, map_y;
    double dtot;

    QRgb *row_col;
    QRgb rgbvalue1 = qRgba(0,0,0,255);

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

    g_mutex.lock();

    /*    from pt 5 --> pt 2045
        = 5 + 20 * 102 total of 103 pts
        to = 5 + 20 * 102 + 3 = 2048
    */

    if(num_navigation_points == 103)
    {
        for( int i = 0; i < num_navigation_points-1; i++)
        {
            dtot = 2 * asin(sqrt(pow((sin((earthloc_lat[nbrLine*103 + i]*PI/180.0 - earthloc_lat[nbrLine*103 + i+1]*PI/180.0) / 2)), 2) + cos(earthloc_lat[nbrLine*103 + i]*PI/180.0) * cos(earthloc_lat[nbrLine*103 + i+1]*PI/180.0) * pow(sin((earthloc_lon[nbrLine*103 + i]*PI/180.0-earthloc_lon[nbrLine*103 + i+1]*PI/180.0) / 2), 2)));
            for( int j = 0; j < 20 ; j++ )
            {
                intermediatePoint(earthloc_lat[nbrLine*103 + i]*PI/180.0, earthloc_lon[nbrLine*103 + i]*PI/180.0, earthloc_lat[nbrLine*103 + i+1]*PI/180.0, earthloc_lon[nbrLine*103 + i+1]*PI/180.0, imageptrs->fraction[4 + i*20 + j], &latpos1, &lonpos1, dtot);
                if(imageptrs->gvp->map_forward_neg_coord(lonpos1, latpos1, map_x, map_y))
                {
                    projectionCoordX[nbrLine * 2048 + i * 20 + j + 4] = (int)map_x;
                    projectionCoordY[nbrLine * 2048 + i * 20 + j + 4] = (int)map_y;

                    //if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                    {
                        rgbvalue1 = row_col[4 + i * 20 + j];
                        QColor col(rgbvalue1);
                        col.setAlpha(255);
                        if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                            imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, col.rgb());
                        projectionCoordValue[nbrLine * 2048 + i * 20 + j + 4] = col.rgb();
                    }
                }
                else
                {
                    projectionCoordX[nbrLine * 2048 + i * 20 + j + 4] = 65535;
                    projectionCoordY[nbrLine * 2048 + i * 20 + j + 4] = 65535;
                    projectionCoordValue[nbrLine * 2048 + i * 20 + j + 4] = qRgb(0,0,0);
                }
            }
        }
    }

    g_mutex.unlock();

}

void SegmentMetop::ComposeSegmentSGProjection(int inputchannel)
{

    qDebug() << QString("ComposeSegmentSGProjection startLineNbr = %1").arg(this->startLineNbr);

    initializeProjectionCoord();

    int startheight = this->startLineNbr;

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        this->RenderSegmentlineInSG( (inputchannel == 0 ? 6 : inputchannel), line, startheight + line );
    }

    QApplication::processEvents();

}

void SegmentMetop::RenderSegmentlineInSG( int channel, int nbrLine, int heightintotalimage )
{

    double lonpos1, latpos1;
    double map_x, map_y;
    double dtot;

    QRgb *row_col;
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

    g_mutex.lock();


    if(num_navigation_points == 103)
    {
        for( int i = 0; i < num_navigation_points-1; i++)
        {
            dtot = 2 * asin(sqrt(pow((sin((earthloc_lat[nbrLine*103 + i]*PI/180.0 - earthloc_lat[nbrLine*103 + i+1]*PI/180.0) / 2)), 2) + cos(earthloc_lat[nbrLine*103 + i]*PI/180.0) * cos(earthloc_lat[nbrLine*103 + i+1]*PI/180.0) * pow(sin((earthloc_lon[nbrLine*103 + i]*PI/180.0-earthloc_lon[nbrLine*103 + i+1]*PI/180.0) / 2), 2)));
            for( int j = 0; j < 20 ; j++ )
            {
                intermediatePoint(earthloc_lat[nbrLine*103 + i]*PI/180.0, earthloc_lon[nbrLine*103 + i]*PI/180.0, earthloc_lat[nbrLine*103 + i+1]*PI/180.0, earthloc_lon[nbrLine*103 + i+1]*PI/180.0, imageptrs->fraction[4 + i*20 + j], &latpos1, &lonpos1, dtot);
                if(imageptrs->sg->map_forward_neg_coord(lonpos1, latpos1, map_x, map_y))
                {
                    projectionCoordX[nbrLine * 2048 + i * 20 + j + 4] = (int)map_x;
                    projectionCoordY[nbrLine * 2048 + i * 20 + j + 4] = (int)map_y;

                    //if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                    {
                        rgbvalue = row_col[4 + i * 20 + j];
                        if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                            imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                        projectionCoordValue[nbrLine * 2048 + i * 20 + j + 4] = rgbvalue;

                    }
                }
                else
                {
                    projectionCoordX[nbrLine * 2048 + i * 20 + j + 4] = 65535;
                    projectionCoordY[nbrLine * 2048 + i * 20 + j + 4] = 65535;
                    projectionCoordValue[nbrLine * 2048 + i * 20 + j + 4] = qRgb(0,0,0);
                }

            }
        }
    }

    g_mutex.unlock();

}

void SegmentMetop::ComposeSegmentLCCProjection(int inputchannel)
{

    qDebug() << QString("SegmentMetop::ComposeSegmentLCCProjection startLineNbr = %1").arg(this->startLineNbr);

    initializeProjectionCoord();

    inputchannel = (inputchannel == 0 ? 6 : inputchannel);
    int startheight = this->startLineNbr;

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        this->RenderSegmentlineInLCC( inputchannel, line, startheight + line );
    }
}


void SegmentMetop::RenderSegmentlineInLCC( int channel, int nbrLine, int heightintotalimage )
{

    double lonpos1, latpos1;
    double map_x, map_y;

    QRgb *row_col;
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

    g_mutex.lock();

    double dtot;
    int pointx;

/*    from pt 5 --> pt 2045
    = 5 + 20 * 102 total of 103 pts
    to = 5 + 20 * 102 + 3 = 2048
*/
    if(num_navigation_points == 103)
    {
        for( int i = 0; i < num_navigation_points-1; i++)
        {
            dtot = 2 * asin(sqrt(pow((sin((earthloc_lat[nbrLine*103 + i]*PI/180.0 - earthloc_lat[nbrLine*103 + i+1]*PI/180.0) / 2)), 2) + cos(earthloc_lat[nbrLine*103 + i]*PI/180.0) * cos(earthloc_lat[nbrLine*103 + i+1]*PI/180.0) * pow(sin((earthloc_lon[nbrLine*103 + i]*PI/180.0-earthloc_lon[nbrLine*103 + i+1]*PI/180.0) / 2), 2)));

            for( int j = 0; j < 20 ; j++ )
            {
                pointx = 4 + i*20 + j;
                intermediatePoint(earthloc_lat[nbrLine*103 + i]*PI/180.0, earthloc_lon[nbrLine*103 + i]*PI/180.0, earthloc_lat[nbrLine*103 + i+1]*PI/180.0, earthloc_lon[nbrLine*103 + i+1]*PI/180.0, imageptrs->fraction[4 + i*20 + j], &latpos1, &lonpos1, dtot);
                if(imageptrs->lcc->map_forward_neg_coord(lonpos1, latpos1, map_x, map_y))
                {
                    projectionCoordX[nbrLine * 2048 + i * 20 + j + 4] = (int)map_x;
                    projectionCoordY[nbrLine * 2048 + i * 20 + j + 4] = (int)map_y;

                    //if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                    {
                        if(opts.sattrackinimage)
                        {
                            if(pointx == 1022 || pointx == 1023 || pointx == 1024 || pointx == 1025 )
                            {
                                rgbvalue = qRgb(250, 0, 0);
                            }
                            else
                            {
                                rgbvalue =row_col[pointx];
                            }

                        }
                        else
                        {
                            rgbvalue =row_col[pointx];
                        }
                        QColor col(rgbvalue);
                        col.setAlpha(255);

                        if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                            imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                        projectionCoordValue[nbrLine * 2048 + i * 20 + j + 4] = col.rgb();

                    }
                }
                else
                {
                    projectionCoordX[nbrLine * 2048 + i * 20 + j + 4] = 65535;
                    projectionCoordY[nbrLine * 2048 + i * 20 + j + 4] = 65535;
                    projectionCoordValue[nbrLine * 2048 + i * 20 + j + 4] = qRgb(0,0,0);
                }
            }
        }
    }

    g_mutex.unlock();

}


/*
void SegmentMetop::RenderSegmentlineInGVPRad(int channel, double lat_first, double lon_first, double lat_last, double lon_last, double altitude, int heightintotalimage)
{

    QRgb *row_col;

    double latdiff = sin((lat_first-lat_last)/2);
    double londiff = sin((lon_first-lon_last)/2);

    double sinpsi = sqrt(latdiff * latdiff + cos(lat_first)*cos(lat_last)*londiff * londiff);
    double psi = asin(sinpsi);
    double delta = atan(sinpsi/((( XKMPER + altitude)/ XKMPER) - cos(psi)));

    // qDebug() << QString("earth_views_per_scanline = %1").arg(earth_views_per_scanline);

    //double deltax = delta / 204;
    double deltax = delta / (earth_views_per_scanline / 2);
    double psix;
    double psix1, psix2;
    double dx;
    double r = XKMPER + altitude;  // earth_location_altitude[0]/10;
    double sindeltax;
    double tc;
    double lonpos1, latpos1, lonpos2, latpos2, dlon1, dlon2;

    tc = fmod(atan2(sin(lon_first-lon_last)*cos(lat_last), cos(lat_first)*sin(lat_last)-sin(lat_first)*cos(lat_last)*cos(lon_first-lon_last)) , 2 * PI);

    sindeltax = sin(delta);
    dx = r * cos(delta) - sqrt( XKMPER * XKMPER - r * r * sindeltax * sindeltax );
    psix = asin( dx * sindeltax / XKMPER );



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


    QColor rgb;
    QColor rgb1, rgb2;
    QRgb rgbvalue1 = qRgb(0,0,0);
    QRgb rgbvalue2 = qRgb(0,0,0);

    int posx, posy;
    int posx1, posy1, posx2, posy2;
    double map_x, map_y;


    g_mutex.lock();

    for (int pix = 0 ; pix < (earth_views_per_scanline/2); pix+=1)
    {

        sindeltax = sin(deltax * pix);
        dx = r * cos(deltax * pix) - sqrt( XKMPER * XKMPER - r * r * sindeltax * sindeltax );
        psix1 = psi + asin( dx * sindeltax / XKMPER );
        psix2 = psi - asin( dx * sindeltax / XKMPER );

        latpos1 = asin(sin(lat_first)*cos(psix1)+cos(lat_first)*sin(psix1)*cos(tc));
        dlon1=atan2(sin(tc)*sin(psix1)*cos(lat_first),cos(psix1)-sin(lat_first)*sin(latpos1));
        lonpos1=fmod( lon_first-dlon1 + PI,2*PI )-PI;

        latpos2 = asin(sin(lat_first)*cos(psix2)+cos(lat_first)*sin(psix2)*cos(tc));
        dlon2=atan2(sin(tc)*sin(psix2)*cos(lat_first),cos(psix2)-sin(lat_first)*sin(latpos2));
        lonpos2=fmod( lon_first-dlon2 + PI,2*PI )-PI;
        //qDebug() << QString("lonpos1 = %1 latpos1 = %2").arg(lonpos1).arg(latpos1);

        if(imageptrs->gvp->map_general_forward(lonpos1, latpos1, map_x, map_y))
        {
            if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
            {
                rgbvalue1 =qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));
                imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue1);
                // qDebug() << QString("map_x = %1 map_y = %2").arg(map_x).arg(map_y);
            }
        }


        if(imageptrs->gvp->map_general_forward(lonpos2, latpos2, map_x, map_y))
        {
            if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
            {
                rgbvalue2 = qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));
                imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue2);
            }
        }

    }

    g_mutex.unlock();

    //qDebug() << QString("lonpos1 = %1 ; latpos1 = %2 ; map_x = %3 ; map_y = %4").arg(lonpos1).arg(latpos1).arg(map_x).arg(map_y);


}
*/
/*
    from pt 5 --> pt 2045
    = 5 + 20 * 102 total of 103 pts

Latitude of point on GC

Intermediate points {lat,lon} lie on the great circle connecting points 1 and 2 when:

lat=atan((sin(lat1)*cos(lat2)*sin(lon-lon2)
     -sin(lat2)*cos(lat1)*sin(lon-lon1))/(cos(lat1)*cos(lat2)*sin(lon1-lon2)))

(not applicable for meridians. i.e if sin(lon1-lon2)=0)


Here we find points (lat,lon) a given fraction of the distance (d) between them. Suppose the starting point is (lat1,lon1)
and the final point (lat2,lon2) and we want the point a fraction f along the great circle route. f=0 is point 1. f=1 is point 2.
The two points cannot be antipodal ( i.e. lat1+lat2=0 and abs(lon1-lon2)=pi) because then the route is undefined.
The intermediate latitude and longitude is then given by:

        A=sin((1-f)*d)/sin(d)
        B=sin(f*d)/sin(d)
        x = A*cos(lat1)*cos(lon1) +  B*cos(lat2)*cos(lon2)
        y = A*cos(lat1)*sin(lon1) +  B*cos(lat2)*sin(lon2)
        z = A*sin(lat1)           +  B*sin(lat2)
        lat=atan2(z,sqrt(x^2+y^2))
        lon=atan2(y,x)
*/

void SegmentMetop::intermediatePoint(double lat1, double lng1, double lat2, double lng2, double f, double *lat, double *lng, double d)
{

//    d=2*asin(sqrt((sin((lat1-lat2)/2))^2 + cos(lat1)*cos(lat2)*(sin((lon1-lon2)/2))^2))
  //double d = 2 * asin(sqrt(pow((sin((lat1 - lat2) / 2)), 2) + cos(lat1) * cos(lat2) * pow(sin((lng1-lng2) / 2), 2)));
  double A = sin((1 - f) * d) / sin(d);
  double B = sin(f * d) / sin(d);
  double x = A * cos(lat1) * cos(lng1) + B * cos(lat2) * cos(lng2);
  double y = A * cos(lat1) * sin(lng1) + B * cos(lat2) * sin(lng2);
  double z = A * sin(lat1) + B * sin(lat2);
  *lat = atan2(z, sqrt(pow(x, 2) + pow(y, 2)));
  *lng = atan2(y, x);
}
