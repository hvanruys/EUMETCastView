#include "segmentdatahub.h"
#include <QDebug>

extern Options opts;
extern SatelliteList satellitelist;


SegmentDatahub::SegmentDatahub(eSegmentType type, QString name,  QObject *parent) : Segment(parent)
{
    segtype = type;
    this->name = name;
    this->fileInfo.setFile(name);
    filedownloaded = false;
    quicklookdownloaded = false;
    bool ok;

    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    //0         1         2         3         4         5         6         7         8         9         10
    //S3A_OL_1_EFR____20161026T161414_20161026T161714_20161026T175243_0179_010_168_4319_MAR_O_NR_002.SEN3.tar
    int sensing_start_year = name.mid(16, 4).toInt( &ok , 10);
    int sensing_start_month = name.mid(20, 2).toInt( &ok, 10);
    int sensing_start_day = name.mid(22, 2).toInt( &ok, 10);
    int sensing_start_hour = name.mid(25, 2).toInt( &ok, 10);
    int sensing_start_minute = name.mid(27, 2).toInt( &ok, 10);
    int sensing_start_second = name.mid(29, 2).toInt( &ok, 10);

    int sensing_end_year = name.mid(32, 4).toInt( &ok , 10);
    int sensing_end_month = name.mid(36, 2).toInt( &ok, 10);
    int sensing_end_day = name.mid(38, 2).toInt( &ok, 10);
    int sensing_end_hour = name.mid(41, 2).toInt( &ok, 10);
    int sensing_end_minute = name.mid(43, 2).toInt( &ok, 10);
    int sensing_end_second = name.mid(45, 2).toInt( &ok, 10);

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


    Satellite *s3_sat;
    if(name.mid(0,3) == "S3A")  // S3A
        s3_sat = satellitelist.GetSatellite(41335, &ok);
    else if(name.mid(0,3) == "S3B") // S3B
        s3_sat = satellitelist.GetSatellite(43437, &ok);

    line1 = s3_sat->line1;
    line2 = s3_sat->line2;

    //line1 = "1 33591U 09005A   11039.40718334  .00000086  00000-0  72163-4 0  8568";
    //line2 = "2 33591  98.8157 341.8086 0013952 344.4168  15.6572 14.11126791103228";
    double epoch = line1.mid(18,14).toDouble(&ok);
    julian_state_vector = Julian_Date_of_Epoch(epoch);

    qtle.reset(new QTle(s3_sat->sat_name, line1, line2, QTle::wgs72));
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

    // qDebug() << QString("---> lon = %1 lat = %2  minutes_since_state_vector = %3").arg(lon_start_deg).arg(lat_start_deg).arg( minutes_since_state_vector);

    CalculateCornerPoints();
    if(segtype == SEG_DATAHUB_OLCIERR)
    {
        CalculateDetailCornerPoints();
    }


}

void SegmentDatahub::addtovector(QVector<QGeodetic> *vec, QString latlonstr)
{

    QString longstr, latstr;

    int ind = latlonstr.indexOf(" ");
    longstr = latlonstr.mid(0, ind);
    latstr = latlonstr.mid(ind + 1);

    QGeodetic ll;
    ll.longitude = deg2rad(longstr.toFloat());
    ll.latitude = deg2rad(latstr.toFloat());
    vec->append(ll);


}

void SegmentDatahub::setFootprint(QString footprint)
{
    QStringList stringlist;

    QVector<QGeodetic> invec;


    this->footprint = footprint;
    if(footprint.mid(0, 12) != "MULTIPOLYGON" && footprint.mid(0, 7) != "POLYGON")
        return;


    int indend, indsep, indfrom;
    int nbrofpolygons = 1;

    if(footprint.mid(0, 12) == "MULTIPOLYGON")
    {
        indend = footprint.indexOf(")))");
        nbrofpolygons = footprint.count(")),") + 1;
        indfrom = footprint.indexOf("(((") + 3;
    }
    else if(footprint.mid(0, 7) == "POLYGON")
    {
        indend = footprint.indexOf("))");
        nbrofpolygons = 1;
        indfrom = footprint.indexOf("((") + 2;
    }


    for(int i = 0; i < nbrofpolygons; i++)
    {
        indsep = footprint.indexOf(")), ((", indfrom);
        if(indsep == -1) // last substring
        {
            stringlist.append(footprint.mid(indfrom, indend - indfrom));
        }
        else
        {
            stringlist.append(footprint.mid(indfrom, indsep - indfrom));
            indfrom = indsep + 6;
        }
    }

    int commaindex1, commaindex2;
    QString lonlatstr;

    for(int i = 0; i < nbrofpolygons; i++)
    {
        if(!stringlist.at(i).isEmpty())
        {
            invec.clear();
            commaindex2 = 0;
            while(true)
            {
                commaindex1 = stringlist.at(i).indexOf(",", commaindex2 );
                if(commaindex1 == -1)
                {
                    lonlatstr = stringlist.at(i).mid(commaindex2).trimmed();
                    addtovector(&invec, lonlatstr );
                    break;
                }
                lonlatstr = stringlist.at(i).mid(commaindex2, commaindex1 - commaindex2 ).trimmed();
                addtovector(&invec, lonlatstr );

                commaindex2 = commaindex1+1;
            }
            listvect.append(invec);
        }

    }

//    for(int i = 0; i < stringlist.count(); i++)
//    {
//        qDebug() << stringlist.at(i);
//    }

//    for(int i = 0; i < listvect.count(); i++)
//    {
//        for(int j = 0; j < listvect.at(i).count(); j++)
//        {
//            qDebug() << i << " " << j << " " << listvect.at(i).at(j).longitude << ";" << listvect.at(i).at(j).latitude;
//        }
//    }

}
