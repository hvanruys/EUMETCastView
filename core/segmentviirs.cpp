#include "segmentviirs.h"
#include "segmentimage.h"

#include "hdf5.h"

#include <QDebug>

extern Options opts;
extern SegmentImage *imageptrs;
#include <QMutex>

extern QMutex g_mutex;

SegmentVIIRS::SegmentVIIRS(QFile *filesegment, SatelliteList *satl, QObject *parent) :
    Segment(parent)
{
    bool ok;

    satlist = satl;

    fileInfo.setFile(*filesegment);
    segment_type = "VIIRS";

    //SVMC_npp_d20141117_t0837599_e0839241_b15833_c20141117084501709131_eum_ops

    int sensing_start_year = fileInfo.fileName().mid(10, 4).toInt( &ok , 10);
    int sensing_start_month = fileInfo.fileName().mid(14, 2).toInt( &ok, 10);
    int sensing_start_day = fileInfo.fileName().mid(16, 2).toInt( &ok, 10);
    int sensing_start_hour = fileInfo.fileName().mid(20, 2).toInt( &ok, 10);
    int sensing_start_minute = fileInfo.fileName().mid(22, 2).toInt( &ok, 10);
    int sensing_start_second = fileInfo.fileName().mid(24, 2).toInt( &ok, 10);
    int sensing_start_msecond = fileInfo.fileName().mid(26, 1).toInt( &ok, 10);
    double d_sensing_start_second = (double)sensing_start_second + (double)sensing_start_msecond / 10.0;


    //this->sensing_start_year = sensing_start_year;
    qdatetime_start.setDate(QDate(sensing_start_year, sensing_start_month, sensing_start_day));
    qdatetime_start.setTime(QTime(sensing_start_hour,sensing_start_minute, sensing_start_second,sensing_start_msecond * 100));

    julian_sensing_start = Julian_Date_of_Year(sensing_start_year) +
            DOY( sensing_start_year, sensing_start_month, sensing_start_day ) +
            Fraction_of_Day( sensing_start_hour, sensing_start_minute, d_sensing_start_second )
            + 5.787037e-06; /* Round up to nearest 1 sec */

    julian_sensing_end = julian_sensing_start + Fraction_of_Day( 0, 1, 0);

    qsensingstart = QSgp4Date(sensing_start_year, sensing_start_month, sensing_start_day, sensing_start_hour, sensing_start_minute, d_sensing_start_second);
    qsensingend = qsensingstart;
    qsensingend.AddMin(1.0);

    this->earth_views_per_scanline = 3200;

    Satellite nss_2 = satlist->GetSatellite(37849);
    line1 = nss_2.line1;
    line2 = nss_2.line2;

    //line1 = "1 33591U 09005A   11039.40718334  .00000086  00000-0  72163-4 0  8568";
    //line2 = "2 33591  98.8157 341.8086 0013952 344.4168  15.6572 14.11126791103228";
    double epoch = line1.mid(18,14).toDouble(&ok);
    julian_state_vector = Julian_Date_of_Epoch(epoch);

    qtle = new QTle(nss_2.sat_name, line1, line2, QTle::wgs72);
    qsgp4 = new QSgp4( *qtle );


    minutes_since_state_vector = ( julian_sensing_start - julian_state_vector ) * MIN_PER_DAY; //  + (1.0/12.0) / 60.0;
    minutes_sensing = 86.0/60.0;

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

    //qDebug() << QString("---> lon = %1 lat = %2  hours_since_state_vector = %3").arg(lon_start_deg).arg(lat_start_deg).arg( hours_since_state_vector);

    NbrOfLines = 768;
    geolatitude = NULL;
    geolongitude = NULL;

    for(int i = 0; i < 3; i++)
    {
        ptrbaVIIRS[i] = NULL;
    }

    tiepoints_lat = NULL;
    tiepoints_lon = NULL;
    aligncoef = NULL;
    expanscoef = NULL;

    CalculateCornerPoints();

}

SegmentVIIRS::~SegmentVIIRS()
{
    qDebug() << "destructor SegmentVIIRS";
    cleanupMemory();
}

void SegmentVIIRS::initializeMemory()
{
    qDebug() << "Initializing VIIRS memory";

    for(int i = 0; i < (bandlist.at(0) ? 3 : 1); i++)
    {
        if(ptrbaVIIRS[i] == NULL)
        {
            ptrbaVIIRS[i] = new unsigned short[earth_views_per_scanline * NbrOfLines];
            qDebug() << QString("Initializing VIIRS memory earth views = %1 nbr of lines = %2").arg(earth_views_per_scanline).arg(NbrOfLines);
            bImageMemory = true;
        }
    }
}

void SegmentVIIRS::resetMemory()
{
    if( geolatitude != NULL)
    {
        delete [] geolatitude;
        geolatitude = NULL;
    }
    if( geolongitude != NULL)
    {
        delete [] geolongitude;
        geolongitude = NULL;
    }

    for(int i = 0; i < 3; i++)
    {
        if(ptrbaVIIRS[i] != NULL)
        {
            delete [] ptrbaVIIRS[i];
            ptrbaVIIRS[i] = NULL;
        }
    }

    bImageMemory = false;
}

void SegmentVIIRS::cleanupMemory()
{
    resetMemory();
}

void SegmentVIIRS::setBandandColor(QList<bool> band, QList<int> color)
{
    bandlist = band;
    colorlist = color;
}

Segment *SegmentVIIRS::ReadSegmentInMemory()
{

    FILE*   f = NULL;
    BZFILE* b;
    int     nBuf;
    char    buf[ 32768 ];
    int     bzerror;
    hid_t   h5_file_id, radiance_id[3], latitude_id, longitude_id;
    hid_t   aligncoef_id, expanscoef_id;
    herr_t  h5_status;

    bool tempfileexist;

    QString basename = this->fileInfo.baseName() + ".h5";

    {
        QFile tfile(basename);
        tempfileexist = tfile.exists();
    }

    qDebug() << QString("file %1  tempfileexist = %2").arg(basename).arg(tempfileexist);

//    if(!tempfileexist)
//    {
        QFile fileout(basename);
        fileout.open(QIODevice::WriteOnly);
        QDataStream streamout(&fileout);


        if((b = BZ2_bzopen(this->fileInfo.absoluteFilePath().toLatin1(),"rb"))==NULL)
        {
            qDebug() << "error in BZ2_bzopen";
        }

        bzerror = BZ_OK;
        while ( bzerror == BZ_OK )
        {
            nBuf = BZ2_bzRead ( &bzerror, b, buf, 32768 );
            if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
            {
                streamout.writeRawData(buf, nBuf);
            }
        }

        BZ2_bzclose ( b );

        fileout.close();

/*    }
    else
    {
        qDebug() << "file " << basename << " already exist !";
    }
*/
    tiepoints_lat = new float[96 * 201];
    tiepoints_lon = new float[96 * 201];
    aligncoef = new float[200];
    expanscoef = new float[200];
    geolongitude = new float[768 * 3200];
    geolatitude = new float[768 * 3200];


    if( (h5_file_id = H5Fopen(basename.toLatin1(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
        qDebug() << "File " << basename << " not open !!";


    bool iscolorimage = this->bandlist.at(0);

    for(int k = 0; k < (iscolorimage ? 3 : 1) ; k++)
    {
        if((radiance_id[k] = H5Dopen2(h5_file_id, (iscolorimage ? getDatasetNameFromColor(k).toLatin1() : getDatasetNameFromBand().toLatin1() ), H5P_DEFAULT)) < 0)
            qDebug() << "Dataset " << (iscolorimage ? getDatasetNameFromColor(k) : getDatasetNameFromBand()) << " is not open !!";
        else
            qDebug() << "Dataset " << (iscolorimage ? getDatasetNameFromColor(k) : getDatasetNameFromBand() ) << " is open !!  ok ok ok ";

        if((h5_status = H5Dread (radiance_id[k], H5T_NATIVE_USHORT, H5S_ALL, H5S_ALL,
                                 H5P_DEFAULT, ptrbaVIIRS[k])) < 0)
            qDebug() << "Unable to read radiance dataset";

    }

    latitude_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-MOD-GEO_All/Latitude", H5P_DEFAULT);
    longitude_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-MOD-GEO_All/Longitude", H5P_DEFAULT);
    aligncoef_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-MOD-GEO_All/AlignmentCoefficient", H5P_DEFAULT);
    expanscoef_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-MOD-GEO_All/ExpansionCoefficient", H5P_DEFAULT);

/*    hid_t threshold_id = H5Aopen(radiance_id[0], "Threshold", H5P_DEFAULT);
    h5_status = H5Aread(threshold_id, H5T_NATIVE_INT, &this->threshold[0]);
    qDebug() << "The value of the threshold = " << this->threshold[0];
    h5_status =  H5Aclose(threshold_id);

    threshold_id = H5Aopen(radiance_id[1], "Threshold", H5P_DEFAULT);
    h5_status = H5Aread(threshold_id, H5T_NATIVE_INT, &this->threshold[1]);
    qDebug() << "The value of the threshold = " << this->threshold[1];
    h5_status =  H5Aclose(threshold_id);

    threshold_id = H5Aopen(radiance_id[2], "Threshold", H5P_DEFAULT);
    h5_status = H5Aread(threshold_id, H5T_NATIVE_INT, &this->threshold[2]);
    qDebug() << "The value of the threshold = " << this->threshold[2];
    h5_status =  H5Aclose(threshold_id);
*/


    if((h5_status = H5Dread (latitude_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, tiepoints_lat)) < 0)
        fprintf(stderr, "unable to read latitude dataset");

    if((h5_status = H5Dread (longitude_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, tiepoints_lon)) < 0)
        fprintf(stderr, "unable to read longitude dataset");

    if((h5_status = H5Dread (aligncoef_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, aligncoef)) < 0)
        fprintf(stderr, "unable to read AlignmentCoefficient dataset");

    if((h5_status = H5Dread (expanscoef_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, expanscoef)) < 0)
        fprintf(stderr, "unable to read ExpansionCoefficient dataset");

    int i, j;

    for( j = 0; j < 16; j++)
        s[j] = (float)((j + 0.5)/16.0);

/*
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 6; i++)
           cout << " " <<  ptrbaVIIRS[j * 3200 + i];
        cout << endl;
    }
*/
/*    cout  << "tie point latitude : " << endl;
    for (j = 0; j < 2; j++) {
        for (i = 100; i < 102; i++)
           cout << " " <<  tiepoints_lat[j * 201 + i];
        cout << endl;
    }

*/
    cout  << "Calc geo lat and lon" << endl;

    for(int itrack = 0; itrack < 48; itrack++)
    {
        for(int iscan = 0; iscan < 200; iscan++)
        {
            CalcGeoLocations(itrack, iscan);
        }
    }


/*    cout << "alpha voor iscan = 100 :" << endl;
    for (j = 0; j < 16; j++) {
        for (i = 0; i < 16; i++)
           cout << " " <<  alpha[j][i];
        cout << endl;
    }


    cout << "geolatitude  :" << endl;
    for (j = 0; j < 16; j++) {
        for (i = 1600; i < 1616; i++)
           cout << " " <<  geolatitude[j * 3200 + i];
        cout << endl;
    }

*/

    // For a 16 bit integer we choose [1; 65527] instead of the possible range [0; 65535] as 0 is an error identificator
    // and values between [65528; 65535] are used as fill values.

    for(int k = 0; k < 3; k++)
    {
        stat_max_ch[k] = 0;
        stat_min_ch[k] = 9999999;
    }


    for(int k = 0; k < (this->bandlist.at(0) ? 3 : 1); k++)
    {
        for (j = 0; j < 768; j++) {
            for (i = 0; i < 3200; i++)
            {
                if(ptrbaVIIRS[k][j * 3200 + i] > 0 && ptrbaVIIRS[k][j * 3200 + i] < 65528)
                {
                    if(ptrbaVIIRS[k][j * 3200 + i] >= stat_max_ch[k])
                        stat_max_ch[k] = ptrbaVIIRS[k][j * 3200 + i];
                    if(ptrbaVIIRS[k][j * 3200 + i] < stat_min_ch[k])
                        stat_min_ch[k] = ptrbaVIIRS[k][j * 3200 + i];
                }
            }
        }
    }

    qDebug() << QString("ptrbaVIIRS min_ch[0] = %1 max_ch[0] = %2").arg(stat_min_ch[0]).arg(stat_max_ch[0]);
    if(this->bandlist.at(0))
    {
        qDebug() << QString("ptrbaVIIRS min_ch[1] = %1 max_ch[1] = %2").arg(stat_min_ch[1]).arg(stat_max_ch[1]);
        qDebug() << QString("ptrbaVIIRS min_ch[2] = %1 max_ch[2] = %2").arg(stat_min_ch[2]).arg(stat_max_ch[2]);

    }

    delete [] tiepoints_lat;
    delete [] tiepoints_lon;
    delete [] aligncoef;
    delete [] expanscoef;

    tiepoints_lat = NULL;
    tiepoints_lon = NULL;
    aligncoef = NULL;
    expanscoef = NULL;

    if(iscolorimage)
    {
        h5_status = H5Dclose (radiance_id[0]);
        h5_status = H5Dclose (radiance_id[1]);
        h5_status = H5Dclose (radiance_id[2]);
    }
    else
        h5_status = H5Dclose (radiance_id[0]);

    h5_status = H5Dclose (latitude_id);
    h5_status = H5Dclose (longitude_id);
    h5_status = H5Dclose (aligncoef_id);
    h5_status = H5Dclose (expanscoef_id);

    h5_status = H5Fclose (h5_file_id);

    return this;
}

Segment *SegmentVIIRS::ReadDatasetsInMemory()
{
    qDebug() << "Segment *SegmentVIIRS::ReadDatasetsInMemory()";

    hid_t   h5_file_id, radiance_id[3];
    herr_t  h5_status;

    bool tempfileexist;

    QString basename = this->fileInfo.baseName() + ".h5";

    {
        QFile tfile(basename);
        tempfileexist = tfile.exists();
    }

    qDebug() << QString("trying H5Fopen basename = %1 exist = %2").arg(basename).arg(tempfileexist);

    if( (h5_file_id = H5Fopen(basename.toLatin1(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
        qDebug() << "File " << basename << " not open !!";
    else
        qDebug() << "File " << basename << " is open !! ------------";


    bool iscolorimage = this->bandlist.at(0);

    for(int k = 0; k < (iscolorimage ? 3 : 1) ; k++)
    {
        if((radiance_id[k] = H5Dopen2(h5_file_id, (iscolorimage ? getDatasetNameFromColor(k).toLatin1() : getDatasetNameFromBand().toLatin1() ), H5P_DEFAULT)) < 0)
            qDebug() << "Dataset " << (iscolorimage ? getDatasetNameFromColor(k) : getDatasetNameFromBand()) << " is not open !!";
        else
            qDebug() << "Dataset " << (iscolorimage ? getDatasetNameFromColor(k) : getDatasetNameFromBand() ) << " is open !!  ok ok ok ";

        if((h5_status = H5Dread (radiance_id[k], H5T_NATIVE_USHORT, H5S_ALL, H5S_ALL,
                                 H5P_DEFAULT, ptrbaVIIRS[k])) < 0)
            qDebug() << "Unable to read radiance dataset";

    }

    for(int k = 0; k < 3; k++)
    {
        stat_max_ch[k] = 0;
        stat_min_ch[k] = 9999999;
    }


    for(int k = 0; k < (this->bandlist.at(0) ? 3 : 1); k++)
    {
        for (int j = 0; j < 768; j++) {
            for (int i = 0; i < 3200; i++)
            {
                if(ptrbaVIIRS[k][j * 3200 + i] > 0 && ptrbaVIIRS[k][j * 3200 + i] < 65528)
                {
                    if(ptrbaVIIRS[k][j * 3200 + i] >= stat_max_ch[k])
                        stat_max_ch[k] = ptrbaVIIRS[k][j * 3200 + i];
                    if(ptrbaVIIRS[k][j * 3200 + i] < stat_min_ch[k])
                        stat_min_ch[k] = ptrbaVIIRS[k][j * 3200 + i];
                }
            }
        }
    }

    if(iscolorimage)
    {
        h5_status = H5Dclose (radiance_id[0]);
        h5_status = H5Dclose (radiance_id[1]);
        h5_status = H5Dclose (radiance_id[2]);
    }
    else
        h5_status = H5Dclose (radiance_id[0]);

    h5_status = H5Fclose (h5_file_id);

    return this;

}


QString SegmentVIIRS::getDatasetNameFromBand()
{
    if(bandlist.at(1))
        return("/All_Data/VIIRS-M1-SDR_All/Radiance");
    else if(bandlist.at(2))
        return("/All_Data/VIIRS-M2-SDR_All/Radiance");
    else if(bandlist.at(3))
        return("/All_Data/VIIRS-M3-SDR_All/Radiance");
    else if(bandlist.at(4))
        return("/All_Data/VIIRS-M4-SDR_All/Radiance");
    else if(bandlist.at(5))
        return("/All_Data/VIIRS-M5-SDR_All/Radiance");
    else if(bandlist.at(6))
        return("/All_Data/VIIRS-M6-SDR_All/Radiance");
    else if(bandlist.at(7))
        return("/All_Data/VIIRS-M7-SDR_All/Radiance");
    else if(bandlist.at(8))
        return("/All_Data/VIIRS-M8-SDR_All/Radiance");
    else if(bandlist.at(9))
        return("/All_Data/VIIRS-M9-SDR_All/Radiance");
    else if(bandlist.at(10))
        return("/All_Data/VIIRS-M10-SDR_All/Radiance");
    else if(bandlist.at(11))
        return("/All_Data/VIIRS-M11-SDR_All/Radiance");
    else if(bandlist.at(12))
        return("/All_Data/VIIRS-M12-SDR_All/Radiance");
    else if(bandlist.at(13))
        return("/All_Data/VIIRS-M13-SDR_All/Radiance");
    else if(bandlist.at(14))
        return("/All_Data/VIIRS-M14-SDR_All/Radiance");
    else if(bandlist.at(15))
        return("/All_Data/VIIRS-M15-SDR_All/Radiance");
    else if(bandlist.at(16))
        return("/All_Data/VIIRS-M16-SDR_All/Radiance");


}

QString SegmentVIIRS::getDatasetNameFromColor(int colorindex)
{
    Q_ASSERT(colorindex >=0 && colorindex < 3);
    colorindex++; // 1, 2 or 3

    if(colorlist.at(0) == colorindex)
        return("/All_Data/VIIRS-M1-SDR_All/Radiance");
    else if(colorlist.at(1) == colorindex)
        return("/All_Data/VIIRS-M2-SDR_All/Radiance");
    else if(colorlist.at(2) == colorindex)
        return("/All_Data/VIIRS-M3-SDR_All/Radiance");
    else if(colorlist.at(3) == colorindex)
        return("/All_Data/VIIRS-M4-SDR_All/Radiance");
    else if(colorlist.at(4) == colorindex)
        return("/All_Data/VIIRS-M5-SDR_All/Radiance");
    else if(colorlist.at(5) == colorindex)
        return("/All_Data/VIIRS-M6-SDR_All/Radiance");
    else if(colorlist.at(6) == colorindex)
        return("/All_Data/VIIRS-M7-SDR_All/Radiance");
    else if(colorlist.at(7) == colorindex)
        return("/All_Data/VIIRS-M8-SDR_All/Radiance");
    else if(colorlist.at(8) == colorindex)
        return("/All_Data/VIIRS-M9-SDR_All/Radiance");
    else if(colorlist.at(9) == colorindex)
        return("/All_Data/VIIRS-M10-SDR_All/Radiance");
    else if(colorlist.at(10) == colorindex)
        return("/All_Data/VIIRS-M11-SDR_All/Radiance");
    else if(colorlist.at(11) == colorindex)
        return("/All_Data/VIIRS-M12-SDR_All/Radiance");
    else if(colorlist.at(12) == colorindex)
        return("/All_Data/VIIRS-M13-SDR_All/Radiance");
    else if(colorlist.at(13) == colorindex)
        return("/All_Data/VIIRS-M14-SDR_All/Radiance");
    else if(colorlist.at(14) == colorindex)
        return("/All_Data/VIIRS-M15-SDR_All/Radiance");
    else if(colorlist.at(15) == colorindex)
        return("/All_Data/VIIRS-M16-SDR_All/Radiance");
}


void SegmentVIIRS::GetAlpha( float &ascan, float &atrack, int rels, int relt, int iscan)
{
    ascan = s[rels] + s[rels] * (1 - s[rels]) * expanscoef[iscan] + s[relt] * (1 - s[relt]) * aligncoef[iscan];
    atrack = s[relt];
}

void SegmentVIIRS::CalcGeoLocations(int itrack, int iscan)  // 0 <= itrack < 48 ; 0 <= iscan < 200
{
    float ascan, atrack;
    int iA, iB, iC, iD;
    int jA, jB, jC, jD;
    float lat_A, lat_B, lat_C, lat_D;
    float lat_1, lat_2, lat;
    float lon_A, lon_B, lon_C, lon_D;
    float lon_1, lon_2, lon;

    iA = 2 * itrack;
    jA = iscan;
    iB = 2 * itrack;
    jB = iscan + 1;
    iC = 2 * itrack + 1;
    jC = iscan + 1;
    iD = 2 * itrack + 1;
    jD = iscan;

    lat_A = tiepoints_lat[iA * 201 + jA];
    lat_B = tiepoints_lat[iB * 201 + jB];
    lat_C = tiepoints_lat[iC * 201 + jC];
    lat_D = tiepoints_lat[iD * 201 + jD];

    //if(iscan == 100)
    //    qDebug() << QString("itrack = %1 iscan = %2 Lat tiepoint A = %3 B = %4 C = %5 D = %6").arg(itrack).arg(iscan).arg(lat_A).arg(lat_B).arg(lat_C).arg(lat_D);

    lon_A = tiepoints_lon[iA * 201 + jA];
    lon_B = tiepoints_lon[iB * 201 + jB];
    lon_C = tiepoints_lon[iC * 201 + jC];
    lon_D = tiepoints_lon[iD * 201 + jD];

    for(int relt = 0; relt < 16; relt++)
    {
        for(int rels = 0; rels < 16; rels++)
        {
            GetAlpha(ascan, atrack, rels, relt, iscan);
            // 96 x 201

            lat_1 = (1 - ascan) * lat_A + ascan * lat_B;
            lat_2 = (1 - ascan) * lat_D + ascan * lat_C;
            lat = (1 - atrack) * lat_1 + atrack * lat_2;

            lon_1 = (1 - ascan) * lon_A + ascan * lon_B;
            lon_2 = (1 - ascan) * lon_D + ascan * lon_C;
            lon = (1 - atrack) * lon_1 + atrack * lon_2;

            geolatitude[((itrack * 16) + relt) * 3200 + (iscan * 16) + rels] = lat;
            geolongitude[((itrack * 16) + relt) * 3200 + (iscan * 16) + rels] = lon;


        }
    }
}

int SegmentVIIRS::ReadNbrOfLines()
{
    return NbrOfLines;
}

bool SegmentVIIRS::composeColorImage()
{
    return(bandlist.at(0));
}

void SegmentVIIRS::ComposeSegmentImage()
{

    QRgb *row;
    int indexout[3];

    qDebug() << "SegmentVIIRS::ComposeSegmentImage() segm->startLineNbr = " << this->startLineNbr;

    int pixval[3];

    bool color = bandlist.at(0);
    bool valok[3];

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        row = (QRgb*)imageptrs->ptrimageViirs->scanLine(this->startLineNbr + line);
        for (int pixelx = 0; pixelx < 3200; pixelx++)
        {
            pixval[0] = *(this->ptrbaVIIRS[0] + line * 3200 + pixelx);
            if(color)
            {
                pixval[1] = *(this->ptrbaVIIRS[1] + line * 3200 + pixelx);
                pixval[2] = *(this->ptrbaVIIRS[2] + line * 3200 + pixelx);
            }

            valok[0] = pixval[0] < 65528 && pixval[0] > 0;
            valok[1] = pixval[1] < 65528 && pixval[1] > 0;
            valok[2] = pixval[2] < 65528 && pixval[2] > 0;

            if( valok[0] && (color ? valok[1] && valok[2] : true))
            {
                for(int i = 0; i < (color ? 3 : 1); i++)
                {
                    indexout[i] =  (int)(255 * ( pixval[i] - imageptrs->stat_min_ch[i] ) / (imageptrs->stat_max_ch[i] - imageptrs->stat_min_ch[i]));
                    indexout[i] = ( indexout[i] > 255 ? 255 : indexout[i] );
                }
                if(color)
                    row[pixelx] = qRgb(imageptrs->lut_ch[0][indexout[0]], imageptrs->lut_ch[1][indexout[1]], imageptrs->lut_ch[2][indexout[2]] );
                else
                    row[pixelx] = qRgb(imageptrs->lut_ch[0][indexout[0]], imageptrs->lut_ch[0][indexout[0]], imageptrs->lut_ch[0][indexout[0]] );


            }
            else
            {
                if(pixval[0] >= 65528 && pixval[1] >= 65528 && pixval[2] >= 65528)
                    row[pixelx] = qRgba(0, 0, 150, 250);
                else if(pixval[0] == 0 || pixval[1] == 0 || pixval[2] == 0)
                    row[pixelx] = qRgba(150, 0, 0, 250);
                else
                {
                    row[pixelx] = qRgba(0, 150, 0, 250);
                }
            }

        }

        if(opts.imageontextureOnVIIRS) // && ((line + 5 ) % 16 == 0 || (line + 10) % 16 == 0) )
        {
            this->RenderSegmentlineInTextureVIIRS( line, row );
            opts.texture_changed = true;
        }

    }
}


void SegmentVIIRS::ComposeSegmentLCCProjection(int inputchannel)
{
    ComposeProjection(LCC);
}

void SegmentVIIRS::ComposeSegmentGVProjection(int inputchannel)
{
    ComposeProjection(GVP);
}

void SegmentVIIRS::ComposeSegmentSGProjection(int inputchannel)
{
    ComposeProjection(SG);
}

void SegmentVIIRS::ComposeProjection(eProjections proj)
{

    double map_x, map_y;

    float lonpos1, latpos1;

    g_mutex.lock();

    int pixval[3];

    bool color = bandlist.at(0);
    bool valok[3];

    for( int i = 0; i < this->NbrOfLines; i++)
    {
        for( int j = 0; j < this->earth_views_per_scanline ; j++ )
        {
            latpos1 = geolatitude[i * 3200 + j];
            lonpos1 = geolongitude[i * 3200 + j];
            pixval[0] = ptrbaVIIRS[0][i * 3200 + j];
            valok[0] = pixval[0] > 0 && pixval[0] < 65528;
            if(color)
            {
                pixval[1] = ptrbaVIIRS[1][i * 3200 + j];
                pixval[2] = ptrbaVIIRS[2][i * 3200 + j];
                valok[1] = pixval[1] > 0 && pixval[1] < 65528;
                valok[2] = pixval[2] > 0 && pixval[2] < 65528;
            }


            if( valok[0] && (color ? valok[1] && valok[2] : true))
            {
                if(proj == LCC) //Lambert
                {
                    if(imageptrs->lcc->map_forward(lonpos1 * PI / 180.0, latpos1 * PI / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color);
                    }
                }
                else if(proj == GVP) // General Vertical Perspecitve
                {
                    if(imageptrs->gvp->map_forward(lonpos1 * PI / 180.0, latpos1 * PI / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color);
                    }

                }
                else if(proj == SG) // Stereographic
                {
                    if(imageptrs->sg->map_forward(lonpos1 * PI / 180.0, latpos1 * PI / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color);
                    }
                }
            }
        }
    }


    g_mutex.unlock();

}

bool SegmentVIIRS::lookupLonLat(double lon_rad, double lat_rad, int &col, int &row)
{
    float latpos, lonpos;

    if( geolatitude == 0x0)
    {
        qDebug() << "pointer to geolatitude = 0x0 !!!!!!!!!!!";
        return false;
    }
    else
        qDebug() << "pointer to geolatitude = OK !!!!!!!!!!!";

    for( int i = 0; i < this->NbrOfLines; i++)
    {
        for( int j = 0; j < this->earth_views_per_scanline ; j++ )
        {
            latpos = geolatitude[i * 3200 + j];
            lonpos = geolongitude[i * 3200 + j];
        }
    }

    return false;
}

bool SegmentVIIRS::testLonLat()
{
    if( geolatitude == 0x0)
    {
        qDebug() << "pointer to geolatitude = 0x0 !!!!!!!!!!!";
    }
    if( geolongitude == 0x0)
    {
        qDebug() << "pointer to geolongitude = 0x0 !!!!!!!!!!!";
    }
    return true;
}

void SegmentVIIRS::RenderSegmentlineInTextureVIIRS( int nbrLine, QRgb *row )
{

    QColor rgb;
    int posx, posy;

    g_mutex.lock();

    QPainter fb_painter(imageptrs->pmOut);

    int devwidth = (fb_painter.device())->width();
    int devheight = (fb_painter.device())->height();

    fb_painter.setPen( Qt::black );
    fb_painter.setBrush( Qt::NoBrush );

    int earthviews = earth_views_per_scanline;

    int pixval[3];
    bool valok[3];
    bool color = bandlist.at(0);


    for (int pix = 0 ; pix < earthviews; pix+=2)
    {
        pixval[0] = ptrbaVIIRS[0][nbrLine * 3200 + pix];
        valok[0] = pixval[0] < 65528 && pixval[0] > 0;
        if(color)
        {
            pixval[1] = ptrbaVIIRS[1][nbrLine * 3200 + pix];
            pixval[2] = ptrbaVIIRS[2][nbrLine * 3200 + pix];
            valok[1] = pixval[1] < 65528 && pixval[1] > 0;
            valok[2] = pixval[2] < 65528 && pixval[2] > 0;
        }


        if( valok[0] && (color ? valok[1] && valok[2] : true))
        {
            sphericalToPixel( this->geolongitude[nbrLine * 3200 + pix] * PI/180.0, this->geolatitude[nbrLine * 3200 + pix] * PI/180.0, posx, posy, devwidth, devheight );
            rgb.setRgb(qRed(row[pix]), qGreen(row[pix]), qBlue(row[pix]));
            fb_painter.setPen(rgb);
            fb_painter.drawPoint( posx , posy );
        }
    }

    fb_painter.end();
    g_mutex.unlock();
}


void SegmentVIIRS::MapPixel( int lines, int views, double map_x, double map_y, bool color)
{
    int indexout[3];
    int pixval[3];

    QRgb rgbvalue = qRgb(0,0,0);

    pixval[0] = ptrbaVIIRS[0][lines * 3200 + views];
    if(color)
    {
        pixval[1] = ptrbaVIIRS[1][lines * 3200 + views];
        pixval[2] = ptrbaVIIRS[2][lines * 3200 + views];
    }


    if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
    {

        for(int i = 0; i < (color ? 3 : 1); i++)
        {
            indexout[i] =  (int)(255 * ( pixval[i] - imageptrs->stat_min_ch[i] ) / (imageptrs->stat_max_ch[i] - imageptrs->stat_min_ch[i]));
            indexout[i] = ( indexout[i] > 255 ? 255 : indexout[i] );
        }

        if(color)
            rgbvalue  = qRgb(imageptrs->lut_ch[0][indexout[0]], imageptrs->lut_ch[1][indexout[1]], imageptrs->lut_ch[2][indexout[2]] );
        else
            rgbvalue  = qRgb(imageptrs->lut_ch[0][indexout[0]], imageptrs->lut_ch[0][indexout[0]], imageptrs->lut_ch[0][indexout[0]] );

        if(opts.sattrackinimage)
        {
            if(views == 1598 || views == 1599 || views == 1600 || views == 1601 )
            {
                rgbvalue = qRgb(250, 0, 0);
                imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
            }
            else
                imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
        }
        else
            imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);

    }
}

/*
void SegmentVIIRS::MapPixelAlter( int lines, int views, double map_x, double map_y, bool color)
{
    int indexout[3];
    int pixval[3];

    QRgb rgbvalue = qRgb(0,0,0);

    pixval[0] = ptrbaVIIRS[0][lines * 3200 + views];
    if(color)
    {
        pixval[1] = ptrbaVIIRS[1][lines * 3200 + views];
        pixval[2] = ptrbaVIIRS[2][lines * 3200 + views];
    }

    if (map_x > 0 && map_x < imageptrs->ptrimageProjection->width() && map_y > 0 && map_y < imageptrs->ptrimageProjection->height())
    {
        for(int i = 0; i < (color ? 3 : 1); i++)
        {
            indexout[i] =  (int)(255 * ( pixval[i] - imageptrs->stat_min_ch[i] ) / (imageptrs->stat_max_ch[i] - imageptrs->stat_min_ch[i]));
            indexout[i] = ( indexout[i] > 255 ? 255 : indexout[i] );
        }

            rgbvalue  = qRgb(200,200, 200 );

        imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);


    }
}

void SegmentVIIRS::MapPixelInverse( int proj_x, int proj_y, int views, int lines, bool color)
{
    int indexout[3];
    int pixval[3];
    bool valok[0];

    QRgb rgbvalue = qRgb(0,0,0);

    pixval[0] = ptrbaVIIRS[0][lines * 3200 + views];
    valok[0] = pixval[0] > 0 && pixval[0] < 65528;
    if(color)
    {
        pixval[1] = ptrbaVIIRS[1][lines * 3200 + views];
        pixval[2] = ptrbaVIIRS[2][lines * 3200 + views];
        valok[1] = pixval[1] > 0 && pixval[1] < 65528;
        valok[2] = pixval[2] > 0 && pixval[2] < 65528;
    }


    if( valok[0] && (color ? valok[1] && valok[2] : true))
    {

        for(int i = 0; i < (color ? 3 : 1); i++)
        {
            indexout[i] =  (int)(255 * ( pixval[i] - imageptrs->stat_min_ch[i] ) / (imageptrs->stat_max_ch[i] - imageptrs->stat_min_ch[i]));
            indexout[i] = ( indexout[i] > 255 ? 255 : indexout[i] );
        }

        if(color)
            rgbvalue  = qRgb(imageptrs->lut_ch[0][indexout[0]], imageptrs->lut_ch[1][indexout[1]], imageptrs->lut_ch[2][indexout[2]] );
        else
            rgbvalue  = qRgb(imageptrs->lut_ch[0][indexout[0]], imageptrs->lut_ch[0][indexout[0]], imageptrs->lut_ch[0][indexout[0]] );

        imageptrs->ptrimageProjection->setPixel(proj_x, proj_y, rgbvalue);
    }
}

*/
