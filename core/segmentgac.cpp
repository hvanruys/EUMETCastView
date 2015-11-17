#include "segmentgac.h"

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

#include <QMutex>

extern QMutex g_mutex;
extern Options opts;
extern SegmentImage *imageptrs;

SegmentGAC::SegmentGAC(QFile *filesegment, SatelliteList *satl, QObject *parent) :
    Segment(parent)
{
    bool ok;
    satlist = satl;
    fileInfo.setFile(*filesegment);
    segment_type = "GAC";
    segtype = eSegmentType::SEG_GAC;

    this->satname = fileInfo.baseName().mid(12, 3);

    //AVHR_GAC_1B_N19_20130701041003Z_20130701041303Z_N_O_20130701054958Z

    int sensing_start_year = fileInfo.fileName().mid(16, 4).toInt( &ok , 10);
    int sensing_start_month = fileInfo.fileName().mid(20, 2).toInt( &ok, 10);
    int sensing_start_day = fileInfo.fileName().mid(22, 2).toInt( &ok, 10);
    int sensing_start_hour = fileInfo.fileName().mid(24, 2).toInt( &ok, 10);
    int sensing_start_minute = fileInfo.fileName().mid(26, 2).toInt( &ok, 10);
    int sensing_start_second = fileInfo.fileName().mid(28, 2).toInt( &ok, 10);
    //this->sensing_start_year = sensing_start_year;
    qdatetime_start.setDate(QDate(sensing_start_year, sensing_start_month, sensing_start_day));
    qdatetime_start.setTime(QTime(sensing_start_hour,sensing_start_minute, sensing_start_second));

    qsensingstart = QSgp4Date(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, sensing_start_second);
    qsensingend = qsensingstart;
    qsensingend.AddMin(1.0);

    julian_sensing_start = qsensingstart.Julian();
    julian_sensing_end = qsensingend.Julian();

    this->earth_views_per_scanline = 409;


    Satellite noaa19;
    ok = satlist->GetSatellite(33591, &noaa19);

    line1 = noaa19.line1;
    line2 = noaa19.line2;

    //line1 = "1 33591U 09005A   11039.40718334  .00000086  00000-0  72163-4 0  8568";
    //line2 = "2 33591  98.8157 341.8086 0013952 344.4168  15.6572 14.11126791103228";

    qtle.reset(new QTle(noaa19.sat_name, line1, line2, QTle::wgs72));
    qsgp4.reset(new QSgp4( *qtle ));

    julian_state_vector = qtle->Epoch();

    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MIN_PER_DAY; //  + (1.0/12.0) / 60.0;
    minutes_sensing = 3;

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

SegmentGAC::~SegmentGAC()
{
}

/*
    FILE*   f;
    BZFILE* b;
    int     nBuf;
    char    buf[26660];
    char    bufheader[20];
    int     bzerror;
    quint64 nextrecordlength = 0;
    quint32 nextres = 0;
    quint32 num1=0, num2=0;

    fileInfo.setFile(*filesegment);
    segment_type = "GAC";

    f = fopen( (*filesegment).fileName().toLatin1(), "r" );
    if ( !f )
    {
        qDebug() << QString("file %1 not found ! ").arg(this->fileInfo.absoluteFilePath());
        segmentok = false;
        return;
    }

    //qDebug() << "Bz2 file " + fileInfo.fileName() + " is open";

    if((b = BZ2_bzopen((*filesegment).fileName().toLatin1(),"rb"))==NULL)
    {
        qDebug() << "error in BZ2_bzopen";
        segmentok = false;
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
                QByteArray ba_earth_views_per_scanline = mdr_record.mid( 2, 2 );

                num1 = 0XFF & ba_earth_views_per_scanline.at(0);   // 0X8FFF;
                num2 = 0XFF & ba_earth_views_per_scanline.at(1);

                quint16 earth_views  = (num1 <<= 8) | num2;
                this->earth_views_per_scanline = earth_views;

                break;
            }

         }
         else
             segmentok = false;
      }
      else
          segmentok = false;
    }

    //qDebug() << QString("=====> earth_views_per_scanline = %1").arg(earth_views_per_scanline);

    BZ2_bzclose ( b );
    fclose(f);

}
*/

bool SegmentGAC::inspectMPHRrecord(QByteArray mphr_record)
{

    quint64 nextrecordlength = 0;
    quint32 nextres = 0;
    quint32 num32_1=0, num32_2=0, num32_3=0, num32_4=0;
    qint64 offset_mdr;
    long nbrmdr = 0;
    bool ok;


    lat_start_deg = mphr_record.mid( 2396, 11).toDouble()/1000;
    lat_start_rad = deg2rad( lat_start_deg );
    lon_start_deg = mphr_record.mid( 2440, 11).toDouble()/1000;
    lon_start_rad = deg2rad( lon_start_deg );

    lat_end_deg = mphr_record.mid( 2484, 11).toDouble()/1000;
    lat_end_rad = deg2rad( lat_end_deg );
    lon_end_deg = mphr_record.mid( 2528, 11).toDouble()/1000;
    lon_end_rad = deg2rad( lon_end_deg );

    int state_vector_year = mphr_record.mid(1509, 4).toInt( &ok, 10);
    int state_vector_month = mphr_record.mid(1513, 2).toInt( &ok, 10);
    int state_vector_day = mphr_record.mid(1515, 2).toInt( &ok, 10);
    int state_vector_hour = mphr_record.mid(1517, 2).toInt( &ok, 10);
    int state_vector_minute = mphr_record.mid(1519, 2).toInt( &ok, 10);
    int state_vector_second = mphr_record.mid(1521, 2).toInt( &ok, 10);


    int day_of_year = DOY( state_vector_year, state_vector_month, state_vector_day );

    if (day_of_year < 1.0 || day_of_year > 366.0)
        return false;

    double fraction_of_day = Fraction_of_Day( state_vector_hour, state_vector_minute, state_vector_second );
    //state_vector = (double)day_of_year + fraction_of_day;

    double semi_major_axis = mphr_record.mid(1560, 11).toDouble()/1000000;  // in KM
    double eccentricity = mphr_record.mid(1604, 11).toDouble()/1000000;
    double inclination = mphr_record.mid(1648, 11).toDouble()/1000;
    double perigee_argument = mphr_record.mid(1692, 11).toDouble()/1000;
    double right_ascension = mphr_record.mid(1736, 11).toDouble()/1000;
    double mean_anomaly = mphr_record.mid(1780, 11).toDouble()/1000;
    quint32 orbit_start =  mphr_record.mid(1389, 5).toInt( &ok, 10);

    double mean_motion_period = TWOPI * pow( pow( semi_major_axis, 3 ) / 398600.4418, 0.5); // in seconds
    double revs_per_day = SEC_PER_DAY / mean_motion_period;

    const QChar fill = '0';
    double total_time = day_of_year + fraction_of_day;



    line1.append( "1 12345U 99001    ");
    line1.append( mphr_record.mid(1511, 2) );
    line1.append(QString("%1").arg( total_time, 11, 'f', 7));
    line1.append("  -.00000000  00000-0  00000-0 0  8553");

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
    line2.append(QString("%1").arg( revs_per_day, 10, 'f', 7));
    line2.append(QString("%1").arg( orbit_start, 5, 10, fill));

    qtle.reset(new QTle( "GAC", line1, line2, QTle::wgs72));
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


     julian_state_vector = qtle->Epoch();

    qsensingstart = QSgp4Date(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, sensing_start_second);
    qsensingend = QSgp4Date(sensing_end_year, sensing_end_month, sensing_end_day, sensing_end_hour, sensing_end_minute, sensing_end_second);
    julian_sensing_start = qsensingstart.Julian();
    julian_sensing_end = qsensingend.Julian();

    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MIN_PER_DAY;
    minutes_sensing = (double)(sensing_start.secsTo(sensing_end))/60;

    qdatetime_start.setDate(QDate(sensing_start_year, sensing_start_month, sensing_start_day));
    qdatetime_start.setTime(QTime(sensing_start_hour,sensing_start_minute, sensing_start_second));
    NbrOfLines = mphr_record.mid(2969, 4).toInt( &ok, 10 );

    CalculateCornerPoints();

    return true;
}

int SegmentGAC::ReadNbrOfLines()
{
    FILE*   f;
    BZFILE* b;

    quint32 nextres = 0;
    quint64 nextrecordlength = 0;

    char    buf[ 26640 ];
    char    bufheader[20];
    int     bzerror;
    int nBuf;

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

quint32 SegmentGAC::get_next_header( QByteArray ba )
{
    if (ba.at(0) == 0x01)
    {
        //qDebug() << "MPHR";
        cnt_mphr++;
    }
    else if (ba.at(0) == 0x02)
    {
        //qDebug() << "SPHR";
        cnt_sphr++;
    }
    else if (ba.at(0) == 0x03)
    {
        //qDebug() << "IPR";
        cnt_ipr++;
    }
    else if (ba.at(0) == 0x04)
    {
        //qDebug() << "GEADR";
        cnt_geadr++;
    }
    else if (ba.at(0) == 0x05)
    {
        //qDebug() << "GIADR";
        cnt_giadr++;
    }
    else if (ba.at(0) == 0x06)
    {
        //qDebug() << "VEADR";
        cnt_veadr++;
    }
    else if (ba.at(0) == 0x07)
    {
        //qDebug() << "VIADR";
        cnt_viadr++;
    }
    else if (ba.at(0) == 0x08)
    {
        cnt_mdr++;

    }
    else qDebug() << "Unknown record";

    bool ok;
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

Segment *SegmentGAC::ReadSegmentInMemory()
{
    FILE*   f;
    BZFILE* b;
    int     nBuf;
    char    buf[26660];
    char    bufheader[20];
    int     bzerror;
    quint64 nextrecordlength = 0;
    quint32 nextres = 0;
    quint16 val1_ch[5], val2_ch[5],tot_ch[5];
    quint32 num32_1=0, num32_2=0, num32_3=0, num32_4=0;
    quint16 num16_1=0, num16_2=0;
    QByteArray picture_line;

    int heightinsegment = 0;

    earthloc_lon.reset(new float[360*51]);
    earthloc_lat.reset(new float[360*51]);

    f = fopen ( this->fileInfo.absoluteFilePath().toLatin1(), "r" );
    if ( !f )
    {
        qDebug() << QString("file %1 not found ! ").arg(this->fileInfo.absoluteFilePath());
        return this;
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
         //qDebug() << QString("nextres = %1").arg(nextres);

         nBuf = BZ2_bzRead ( &bzerror, b, buf, nextres - 20 );

         if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
         {
             if ((dataheader.at(0) & 0xFF) == 0x01)
             {
                 QByteArray mphr_record = QByteArray::fromRawData(buf, nBuf);
                 if(!inspectMPHRrecord(mphr_record))
                     segmentok = false;
             }

            if ((dataheader.at(0) & 0xFF) == 0x08 && nextres == 6160) // MDR
            {
               QByteArray mdr_record = QByteArray::fromRawData(buf, nBuf);

               Q_ASSERT( mdr_record.length() == 6140);

               inspectEarthLocations(&mdr_record, heightinsegment);

               //QByteArray ba_earth_views_per_scanline = mdr_record.mid( 2, 2 );

               //num1 = 0XFF & ba_earth_views_per_scanline.at(0);   // 0X8FFF;
               //num2 = 0XFF & ba_earth_views_per_scanline.at(1);

               //quint16 earth_views  = (num1 <<= 8) | num2;
               //this->earth_views_per_scanline = earth_views;

               picture_line = mdr_record.mid( 4, 4090 );

               for (int i=0, j=0; i < 818; i+=2, j++)
               {
                   for( int k = 0, l = 0; k < 5; k++, l+=818)
                   {
                       val1_ch[k] = 0xFF & picture_line.at(i+l);
                       val2_ch[k] = 0xFF & picture_line.at(i+l+1);
                       tot_ch[k] = (val1_ch[k] <<= 8) | val2_ch[k];
                       *(this->ptrbaChannel[k].data() + heightinsegment * 409 + j) = tot_ch[k];

                   }
                   for (int k=0; k < 5; k++)
                   {
                       if (tot_ch[k] < stat_min_ch[k] )
                           stat_min_ch[k] = tot_ch[k];
                       if (tot_ch[k] > stat_max_ch[k] )
                           stat_max_ch[k] = tot_ch[k];
                   }

               }

               num32_1 = 0xFF & mdr_record.at(4108);
               num32_2 = 0xFF & mdr_record.at(4109);
               num32_3 = 0xFF & mdr_record.at(4110);
               num32_4 = 0xFF & mdr_record.at(4111);
               qint32 loc_alt = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
               earth_loc_altitude[heightinsegment] = (double)(loc_alt /10);

               num32_1 = 0xFF & mdr_record.at(4128);
               num32_2 = 0xFF & mdr_record.at(4129);
               num32_3 = 0xFF & mdr_record.at(4130);
               num32_4 = 0xFF & mdr_record.at(4131);
               qint32 loc_lat_first = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
               earth_loc_lat_first[heightinsegment] = (double)(loc_lat_first * PI/1800000);

               num32_1 = 0xFF & mdr_record.at(4132);
               num32_2 = 0xFF & mdr_record.at(4133);
               num32_3 = 0xFF & mdr_record.at(4134);
               num32_4 = 0xFF & mdr_record.at(4135);
               qint32 loc_lon_first = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
               earth_loc_lon_first[heightinsegment] = (double)(loc_lon_first * PI/1800000);

               num32_1 = 0xFF & mdr_record.at(4136);
               num32_2 = 0xFF & mdr_record.at(4137);
               num32_3 = 0xFF & mdr_record.at(4138);
               num32_4 = 0xFF & mdr_record.at(4139);
               qint32 loc_lat_last = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
               earth_loc_lat_last[heightinsegment] = (double)(loc_lat_last * PI/1800000);

               num32_1 = 0xFF & mdr_record.at(4140);
               num32_2 = 0xFF & mdr_record.at(4141);
               num32_3 = 0xFF & mdr_record.at(4142);
               num32_4 = 0xFF & mdr_record.at(4143);
               qint32 loc_lon_last = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
               earth_loc_lon_last[heightinsegment] = (double)(loc_lon_last * PI/1800000);

               num16_1 = 0xFF & mdr_record.at(4144);
               num16_2 = 0xFF & mdr_record.at(4145);
               num_navigation_points  = (num16_1 <<= 8) | num16_2;


//               qDebug() << QString("height = %1 earth_loc_altitude = %2 lat = %3 lon = %4").arg(heightinsegment)
//                           .arg(earth_loc_altitude[heightinsegment])
//                           .arg(earth_loc_lat_first[heightinsegment]*180.0/PI)
//                           .arg(earth_loc_lon_first[heightinsegment]*180.0/PI);
               heightinsegment++;

            }
         }
      }
    }

    qDebug() << QString("ReadSegmentInMemory stat_min_ch1 = %1  stat_max_ch1 = %2").arg(stat_min_ch[0]).arg(stat_max_ch[0]);
    qDebug() << QString("ReadSegmentInMemory stat_min_ch2 = %1  stat_max_ch2 = %2").arg(stat_min_ch[1]).arg(stat_max_ch[1]);
    qDebug() << QString("ReadSegmentInMemory stat_min_ch3 = %1  stat_max_ch3 = %2").arg(stat_min_ch[2]).arg(stat_max_ch[2]);
    qDebug() << QString("ReadSegmentInMemory stat_min_ch4 = %1  stat_max_ch4 = %2").arg(stat_min_ch[3]).arg(stat_max_ch[3]);
    qDebug() << QString("ReadSegmentInMemory stat_min_ch5 = %1  stat_max_ch5 = %2").arg(stat_min_ch[4]).arg(stat_max_ch[4]);

    return this;
}

void SegmentGAC::inspectEarthLocations(QByteArray *mdr_record, int heightinsegment)
{

    quint16 num1=0, num2=0;
    qint32 num32_1=0, num32_2=0, num32_3=0, num32_4=0;

    Q_ASSERT( mdr_record->length() == 6140);

    QByteArray earth_views_per_scanline = mdr_record->mid( 2, 2 );

    num1 = 0XFF & earth_views_per_scanline.at(0);   // 0X8FFF;
    num2 = 0XFF & earth_views_per_scanline.at(1);

    quint16 earth_views  = (num1 <<= 8) | num2;
    this->earth_views_per_scanline = earth_views;


    num1 = 0xFF & mdr_record->at(4144);
    num2 = 0xFF & mdr_record->at(4145);
    int num_navigation_points  = (num1 <<= 8) | num2;


    long llat_deg, llon_deg;

    for(int i = 0; i < num_navigation_points; i++)
    {

        num32_1 = 0xFF & mdr_record->at(4554 + i*8);
        num32_2 = 0xFF & mdr_record->at(4555 + i*8);
        num32_3 = 0xFF & mdr_record->at(4556 + i*8);
        num32_4 = 0xFF & mdr_record->at(4557 + i*8);
        llat_deg = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
        earthloc_lat[heightinsegment*num_navigation_points + i] = (float)llat_deg/10000;

        num32_1 = 0xFF & mdr_record->at(4558 + i*8);
        num32_2 = 0xFF & mdr_record->at(4559 + i*8);
        num32_3 = 0xFF & mdr_record->at(4560 + i*8);
        num32_4 = 0xFF & mdr_record->at(4561 + i*8);
        llon_deg = (num32_1 <<= 24) | (num32_2 <<= 16) | (num32_3 <<= 8) | num32_4;
        earthloc_lon[heightinsegment*num_navigation_points + i] = (float)llon_deg/10000;
    }


}

void SegmentGAC::initializeProjectionCoord()
{
    projectionCoordX.reset(new int[360 * 409]);
    projectionCoordY.reset(new int[360 * 409]);
    projectionCoordValue.reset(new QRgb[360 * 409]);

    for( int i = 0; i < 360; i++)
    {
        for( int j = 0; j < 409 ; j++ )
        {
            projectionCoordX[i * 409 + j] = 65535;
            projectionCoordY[i * 409 + j] = 65535;
            projectionCoordValue[i * 409 + j] = qRgba(0, 0, 0, 0);
        }
    }

}

void SegmentGAC::RenderSegmentlineInTexture( int channel, int nbrLine, int nbrTotalLine )
{
    RenderSegmentlineInTextureRad( channel, this->earth_loc_lat_first[nbrLine], earth_loc_lon_first[nbrLine],
                                   earth_loc_lat_last[nbrLine], earth_loc_lon_last[nbrLine], earth_loc_altitude[nbrLine], nbrTotalLine);
}


void SegmentGAC::ComposeSegmentLCCProjection(int inputchannel)
{
    ComposeProjection(inputchannel, LCC);
}

void SegmentGAC::ComposeSegmentGVProjection(int inputchannel)
{
    ComposeProjection(inputchannel, GVP);
}

void SegmentGAC::ComposeSegmentSGProjection(int inputchannel)
{
    qDebug() << QString("ComposeSegmentSGProjection startLineNbr = %1").arg(this->startLineNbr);
    int startheight = this->startLineNbr;

    initializeProjectionCoord();

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        this->RenderSegmentlineInSG( (inputchannel == 0 ? 6 : inputchannel), line, startheight + line );
    }

    QApplication::processEvents();

}

//void SegmentGAC::ComposeSegmentSGProjection(int inputchannel)
//{
//    ComposeProjection(inputchannel, SG);
//}

void SegmentGAC::ComposeProjection(int inputchannel, eProjections proj)
{

    qDebug() << QString("ComposeProjection startLineNbr = %1").arg(this->startLineNbr);
    int startheight = this->startLineNbr;

    initializeProjectionCoord();

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        this->RenderSegmentlineInProjection( (inputchannel == 0 ? 6 : inputchannel), line, startheight + line, proj );
    }

    QApplication::processEvents();

}

void SegmentGAC::intermediatePoint(double lat1, double lng1, double lat2, double lng2, double f, double *lat, double *lng, double d)
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

void SegmentGAC::RenderSegmentlineInSG( int channel, int nbrLine, int heightintotalimage )
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

    /*    from pt 5 --> pt 405
        = 5 + 8 * 50 total of 51 pts
        to = 5 + 8 * 50 + 4 = 409
    */

    if(num_navigation_points == 51)
    {
        for( int i = 0; i < num_navigation_points-1; i++)
        {
            dtot = 2 * asin(sqrt(pow((sin((earthloc_lat[nbrLine*51 + i]*PI/180.0 - earthloc_lat[nbrLine*51 + i+1]*PI/180.0) / 2)), 2) + cos(earthloc_lat[nbrLine*51 + i]*PI/180.0) * cos(earthloc_lat[nbrLine*51 + i+1]*PI/180.0) * pow(sin((earthloc_lon[nbrLine*51 + i]*PI/180.0-earthloc_lon[nbrLine*51 + i+1]*PI/180.0) / 2), 2)));
            for( int j = 0; j < 8 ; j++ )
            {
                intermediatePoint(earthloc_lat[nbrLine*51 + i]*PI/180.0, earthloc_lon[nbrLine*51 + i]*PI/180.0, earthloc_lat[nbrLine*51 + i+1]*PI/180.0, earthloc_lon[nbrLine*51 + i+1]*PI/180.0, imageptrs->fractionGAC[5 + i*8 + j], &latpos1, &lonpos1, dtot);
                if(imageptrs->sg->map_forward(lonpos1, latpos1, map_x, map_y))
                {
                    projectionCoordX[nbrLine * 409 + i * 8 + j + 4] = (int)map_x;
                    projectionCoordY[nbrLine * 409 + i * 8 + j + 4] = (int)map_y;

                    //if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                    {
                        rgbvalue = row_col[4 + i * 8 + j];
                        if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                            imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                        projectionCoordValue[nbrLine * 409 + i * 8 + j + 4] = rgbvalue;
                    }
                }
                else
                {
                    projectionCoordX[nbrLine * 409 + i * 8 + j + 4] = 65535;
                    projectionCoordY[nbrLine * 409 + i * 8 + j + 4] = 65535;
                    projectionCoordValue[nbrLine * 409 + i * 8 + j + 4] = qRgb(0,0,0);
                }
            }
        }
    }

    if(nbrLine == 0)
    {
        for( int i = 0; i < num_navigation_points-1; i++)
        {
            for( int j = 0; j < 8 ; j++ )
            {

               int mx = projectionCoordX[nbrLine * 409 + i * 8 + j + 4];
               int my = projectionCoordY[nbrLine * 409 + i * 8 + j + 4];

               qDebug() << QString("index = %1 mx = %2 my = %3").arg(nbrLine * 409 + i * 8 + j + 4).arg(mx).arg(my);
            }
        }
    }

    g_mutex.unlock();

}

void SegmentGAC::RenderSegmentlineInProjection( int channel, int nbrLine, int heightintotalimage, eProjections proj )
{


    //void Segment::RenderSegmentlineInTextureRad(int channel, double lat_first, double lon_first, double lat_last, double lon_last, double altitude, int heightintotalimage)
    //RenderSegmentlineInTextureRad( channel, this->earth_loc_lat_first[nbrLine], earth_loc_lon_first[nbrLine],
    //                               earth_loc_lat_last[nbrLine], earth_loc_lon_last[nbrLine], earth_loc_altitude[nbrLine], nbrTotalLine);
    QRgb *row_col;

    double latdiff = sin((this->earth_loc_lat_first[nbrLine]-this->earth_loc_lat_last[nbrLine])/2);
    double londiff = sin((this->earth_loc_lon_first[nbrLine]-this->earth_loc_lon_last[nbrLine])/2);

    double sinpsi = sqrt(latdiff * latdiff + cos(this->earth_loc_lat_first[nbrLine])*cos(this->earth_loc_lat_last[nbrLine])*londiff * londiff);
    double psi = asin(sinpsi);
    double delta = atan(sinpsi/((( XKMPER + this->earth_loc_altitude[nbrLine])/ XKMPER) - cos(psi)));

    // qDebug() << QString("earth_views_per_scanline = %1").arg(earth_views_per_scanline);

    double deltax = delta / 204.5;
    double psix;
    double psix1, psix2;
    double dx;
    double r = XKMPER + this->earth_loc_altitude[nbrLine];  // earth_location_altitude[0]/10;
    double sindeltax;
    double tc;
    double lonpos1, latpos1, lonpos2, latpos2, dlon1, dlon2;

    tc = fmod(atan2(sin(this->earth_loc_lon_first[nbrLine]-this->earth_loc_lon_last[nbrLine])*cos(this->earth_loc_lat_last[nbrLine]), cos(this->earth_loc_lat_first[nbrLine])*sin(this->earth_loc_lat_last[nbrLine])-sin(this->earth_loc_lat_first[nbrLine])*cos(this->earth_loc_lat_last[nbrLine])*cos(this->earth_loc_lon_first[nbrLine]-this->earth_loc_lon_last[nbrLine])) , 2 * PI);

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


    double map_x, map_y;

    g_mutex.lock();

    QRgb rgbvalue1 = qRgb(0,0,0);
    QRgb rgbvalue2 = qRgb(0,0,0);


    for (int pix = 0 ; pix < (earth_views_per_scanline/2); pix+=1)
    {
        sindeltax = sin(deltax * pix);
        dx = r * cos(deltax * pix) - sqrt( XKMPER * XKMPER - r * r * sindeltax * sindeltax );
        psix1 = psi + asin( dx * sindeltax / XKMPER );
        psix2 = psi - asin( dx * sindeltax / XKMPER );

        latpos1 = asin(sin(this->earth_loc_lat_first[nbrLine])*cos(psix1)+cos(this->earth_loc_lat_first[nbrLine])*sin(psix1)*cos(tc));
        dlon1=atan2(sin(tc)*sin(psix1)*cos(this->earth_loc_lat_first[nbrLine]),cos(psix1)-sin(this->earth_loc_lat_first[nbrLine])*sin(latpos1));
        lonpos1=fmod( this->earth_loc_lon_first[nbrLine]-dlon1 + PI,2*PI )-PI;

        latpos2 = asin(sin(this->earth_loc_lat_first[nbrLine])*cos(psix2)+cos(this->earth_loc_lat_first[nbrLine])*sin(psix2)*cos(tc));
        dlon2=atan2(sin(tc)*sin(psix2)*cos(this->earth_loc_lat_first[nbrLine]),cos(psix2)-sin(this->earth_loc_lat_first[nbrLine])*sin(latpos2));
        lonpos2=fmod( this->earth_loc_lon_first[nbrLine]-dlon2 + PI,2*PI )-PI;

        if(proj == LCC)
        {
            if(imageptrs->lcc->map_forward(lonpos1, latpos1, map_x, map_y))
            {
                projectionCoordX[nbrLine * 409 + (earth_views_per_scanline/2)+pix] = (int)map_x;
                projectionCoordY[nbrLine * 409 + (earth_views_per_scanline/2)+pix] = (int)map_y;
                rgbvalue1 = qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));
                projectionCoordValue[nbrLine * 409 + (earth_views_per_scanline/2)+pix] = rgbvalue1;
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue1);
            }

            if(imageptrs->lcc->map_forward(lonpos2, latpos2, map_x, map_y))
            {
                projectionCoordX[nbrLine * 409 + (earth_views_per_scanline/2)-pix] = (int)map_x;
                projectionCoordY[nbrLine * 409 + (earth_views_per_scanline/2)-pix] = (int)map_y;
                rgbvalue2 = qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));
                projectionCoordValue[nbrLine * 409 + (earth_views_per_scanline/2)-pix] = rgbvalue2;
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue2);
            }
        }
        else if(proj == GVP)
        {
            if(imageptrs->gvp->map_forward(lonpos1, latpos1, map_x, map_y))
            {
                projectionCoordX[nbrLine * 409 + (earth_views_per_scanline/2)+pix] = (int)map_x;
                projectionCoordY[nbrLine * 409 + (earth_views_per_scanline/2)+pix] = (int)map_y;
                rgbvalue1 = qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));
                projectionCoordValue[nbrLine * 409 + (earth_views_per_scanline/2)+pix] = rgbvalue1;
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue1);
            }

            if(imageptrs->gvp->map_forward(lonpos2, latpos2, map_x, map_y))
            {
                projectionCoordX[nbrLine * 409 + (earth_views_per_scanline/2)-pix] = (int)map_x;
                projectionCoordY[nbrLine * 409 + (earth_views_per_scanline/2)-pix] = (int)map_y;
                rgbvalue2 = qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));
                projectionCoordValue[nbrLine * 409 + (earth_views_per_scanline/2)-pix] = rgbvalue2;
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue2);
            }

        }
        else if(proj == SG)
        {
            if(imageptrs->sg->map_forward(lonpos1, latpos1, map_x, map_y))
            {
                projectionCoordX[nbrLine * 409 + (earth_views_per_scanline/2)+pix] = (int)map_x;
                projectionCoordY[nbrLine * 409 + (earth_views_per_scanline/2)+pix] = (int)map_y;
                rgbvalue1 = qRgb(qRed(row_col[(earth_views_per_scanline/2)+pix]), qGreen(row_col[(earth_views_per_scanline/2)+pix]), qBlue(row_col[(earth_views_per_scanline/2)+pix]));
                projectionCoordValue[nbrLine * 409 + (earth_views_per_scanline/2)+pix] = rgbvalue1;
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue1);
            }

            if(imageptrs->sg->map_forward(lonpos2, latpos2, map_x, map_y))
            {
                projectionCoordX[nbrLine * 409 + (earth_views_per_scanline/2)-pix] = (int)map_x;
                projectionCoordY[nbrLine * 409 + (earth_views_per_scanline/2)-pix] = (int)map_y;
                rgbvalue2 = qRgb(qRed(row_col[(earth_views_per_scanline/2)-pix]), qGreen(row_col[(earth_views_per_scanline/2)-pix]), qBlue(row_col[(earth_views_per_scanline/2)-pix]));
                projectionCoordValue[nbrLine * 409 + (earth_views_per_scanline/2)-pix] = rgbvalue2;
                if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue2);
            }

        }
    }


    g_mutex.unlock();
}

