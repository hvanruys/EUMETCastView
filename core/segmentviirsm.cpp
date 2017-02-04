#include "segmentviirsm.h"
#include "segmentimage.h"



#include <QDebug>

extern Options opts;
extern SegmentImage *imageptrs;
#include <QMutex>

extern QMutex g_mutex;

SegmentVIIRSM::SegmentVIIRSM(QFile *filesegment, SatelliteList *satl, QObject *parent) :
    Segment(parent)
{
    bool ok;

    satlist = satl;

    fileInfo.setFile(*filesegment);
    segment_type = "VIIRSM";
    segtype = eSegmentType::SEG_VIIRSM;

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
    this->NbrOfLines = 768;

    Satellite nss_2;
    ok = satlist->GetSatellite(37849, &nss_2);
    line1 = nss_2.line1;
    line2 = nss_2.line2;

    //line1 = "1 33591U 09005A   11039.40718334  .00000086  00000-0  72163-4 0  8568";
    //line2 = "2 33591  98.8157 341.8086 0013952 344.4168  15.6572 14.11126791103228";
    double epoch = line1.mid(18,14).toDouble(&ok);
    julian_state_vector = Julian_Date_of_Epoch(epoch);

    qtle.reset(new QTle(nss_2.sat_name, line1, line2, QTle::wgs72));
    qsgp4.reset(new QSgp4( *qtle ));


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


    latMax = 0.0;
    lonMax = 0.0;
    latMin = 999.0;
    lonMin = 999.0;
    CalculateCornerPoints();
    invertthissegment[0] = false;
    invertthissegment[1] = false;
    invertthissegment[2] = false;

}

SegmentVIIRSM::~SegmentVIIRSM()
{
    resetMemory();
}

void SegmentVIIRSM::initializeMemory()
{
    qDebug() << "Initializing VIIRSM memory";

    for(int i = 0; i < (bandlist.at(0) ? 3 : 1); i++)
    {
        if(ptrbaVIIRS[i].isNull())
        {
            ptrbaVIIRS[i].reset(new unsigned short[earth_views_per_scanline * NbrOfLines]);
            qDebug() << QString("Initializing VIIRSM memory earth views = %1 nbr of lines = %2").arg(earth_views_per_scanline).arg(NbrOfLines);
        }
    }
}

Segment *SegmentVIIRSM::ReadSegmentInMemory()
{

    FILE*   f = NULL;
    BZFILE* b;
    int     nBuf;
    char    buf[ 32768 ];
    int     bzerror;
    hid_t   h5_file_id;
    herr_t  h5_status;

    bool tempfileexist;

    QString basename = this->fileInfo.baseName() + ".h5";

    {
        QFile tfile(basename);
        tempfileexist = tfile.exists();
    }

    qDebug() << QString("file %1  tempfileexist = %2").arg(basename).arg(tempfileexist);

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


    if( (h5_file_id = H5Fopen(basename.toLatin1(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
        qDebug() << "File " << basename << " not open !!";


    ReadVIIRSM_SDR_All(h5_file_id);
    ReadVIIRSM_GEO_All(h5_file_id);


    int i, j;

    for( j = 0; j < 16; j++)
        s[j] = (float)((j + 0.5)/16.0);


    for(int itrack = 0; itrack < 48; itrack++)
    {
        for(int iscan = 0; iscan < 200; iscan++)
        {
            CalcGeoLocations(itrack, iscan);
        }
    }

    this->LonLatMax();


    // For a 16 bit integer we choose [1; 65527] instead of the possible range [0; 65535] as 0 is an error identificator
    // and values between [65528; 65535] are used as fill values.

    for(int k = 0; k < 3; k++)
    {
        stat_max_ch[k] = 0;
        stat_min_ch[k] = 9999999;
        active_pixels[k] = 0;
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
                    active_pixels[k]++;
                }
            }
        }
    }

    minBrightnessTemp = getBrightnessTemp(stat_min_ch[0]);
    maxBrightnessTemp = getBrightnessTemp(stat_max_ch[0]);

    qDebug() << QString("ptrbaVIIRS min_ch[0] = %1 max_ch[0] = %2").arg(stat_min_ch[0]).arg(stat_max_ch[0]);
    qDebug() << QString("Radiance min = %1 W/sr*cm*cm max = %2 W/sr*cm*cm").arg(getRadiance(stat_min_ch[0])).arg(getRadiance(stat_max_ch[0]));
    qDebug() << QString("Brightness Temp min = %1 Kelvin  max = %2 Kelvin").arg(getBrightnessTemp(stat_min_ch[0])).arg(getBrightnessTemp(stat_max_ch[0]));

    if(this->bandlist.at(0))
    {
        qDebug() << QString("ptrbaVIIRS min_ch[1] = %1 max_ch[1] = %2").arg(stat_min_ch[1]).arg(stat_max_ch[1]);
        qDebug() << QString("ptrbaVIIRS min_ch[2] = %1 max_ch[2] = %2").arg(stat_min_ch[2]).arg(stat_max_ch[2]);

    }

    h5_status = H5Fclose (h5_file_id);

    return this;
}


/************************************************************

  Operator function.  Prints the name and type of the object
  being examined.

 ************************************************************/
//herr_t SegmentVIIRSM::op_func(hid_t loc_id, const char *name, const H5L_info_t *info, void *operator_data)
//{
//    herr_t          status;
//    H5O_info_t      infobuf;

//    /*
//     * Get type of the object and display its name and type.
//     * The name of the object is passed to this function by
//     * the Library.
//     */
//    status = H5Oget_info_by_name (loc_id, name, &infobuf, H5P_DEFAULT);
//    switch (infobuf.type) {
//        case H5O_TYPE_GROUP:
//            printf ("  Group: %s\n", name);
//            break;
//        case H5O_TYPE_DATASET:
//            printf ("  Dataset: %s\n", name);
//            break;
//        case H5O_TYPE_NAMED_DATATYPE:
//            printf ("  Datatype: %s\n", name);
//            break;
//        default:
//            printf ( "  Unknown: %s\n", name);
//    }

//    return 0;
//}

void SegmentVIIRSM::ReadVIIRSM_SDR_All(hid_t h5_file_id)
{
    hid_t   radiance_id[3];
    hid_t   h5_id;
    herr_t  h5_status;


    bool iscolorimage = this->bandlist.at(0);

    for(int k = 0; k < (iscolorimage ? 3 : 1) ; k++)
    {
        if((radiance_id[k] = H5Dopen2(h5_file_id, (iscolorimage ? getDatasetNameFromColor(k).toLatin1() : getDatasetNameFromBand().toLatin1() ), H5P_DEFAULT)) < 0)
            qDebug() << "Dataset " << (iscolorimage ? getDatasetNameFromColor(k) : getDatasetNameFromBand()) << " is not open !!";
        else
            qDebug() << "Dataset " << (iscolorimage ? getDatasetNameFromColor(k) : getDatasetNameFromBand() ) << " is open !!  ok ok ok ";

        if((h5_status = H5Dread (radiance_id[k], H5T_NATIVE_USHORT, H5S_ALL, H5S_ALL,
                                 H5P_DEFAULT, ptrbaVIIRS[k].data())) < 0)
            qDebug() << "Unable to read radiance dataset";

    }

    for(int k = 0; k < (iscolorimage ? 3 : 1) ; k++)
    {
        h5_id = H5Aopen(radiance_id[k], "Threshold", H5P_DEFAULT);
        h5_status = H5Aread(h5_id, H5T_NATIVE_INT, &this->threshold[k]);
        h5_status =  H5Aclose(h5_id);

        h5_id = H5Aopen(radiance_id[k], "RadianceOffsetHigh", H5P_DEFAULT);
        h5_status = H5Aread(h5_id, H5T_NATIVE_FLOAT, &this->radianceoffsethigh[k]);
        h5_status =  H5Aclose(h5_id);

        h5_id = H5Aopen(radiance_id[k], "RadianceOffsetLow", H5P_DEFAULT);
        h5_status = H5Aread(h5_id, H5T_NATIVE_FLOAT, &this->radianceoffsetlow[k]);
        h5_status =  H5Aclose(h5_id);

        h5_id = H5Aopen(radiance_id[k], "RadianceScaleHigh", H5P_DEFAULT);
        h5_status = H5Aread(h5_id, H5T_NATIVE_FLOAT, &this->radiancescalehigh[k]);
        h5_status =  H5Aclose(h5_id);

        h5_id = H5Aopen(radiance_id[k], "RadianceScaleLow", H5P_DEFAULT);
        h5_status = H5Aread(h5_id, H5T_NATIVE_FLOAT, &this->radiancescalelow[k]);
        h5_status =  H5Aclose(h5_id);

        if(H5Aexists(radiance_id[k], "BandCorrectionCoefficientA"))
        {
            h5_id = H5Aopen(radiance_id[k], "BandCorrectionCoefficientA", H5P_DEFAULT);
            h5_status = H5Aread(h5_id, H5T_NATIVE_DOUBLE, &this->bandcorrectioncoefficientA[k]);
            if(h5_status < 0)
                qDebug() << "BandCorrectionCoefficientA not read !";
            h5_status =  H5Aclose(h5_id);

            h5_id = H5Aopen(radiance_id[k], "BandCorrectionCoefficientB", H5P_DEFAULT);
            h5_status = H5Aread(h5_id, H5T_NATIVE_DOUBLE, &this->bandcorrectioncoefficientB[k]);
            if(h5_status < 0)
                qDebug() << "BandCorrectionCoefficientB not read !";
            h5_status =  H5Aclose(h5_id);

            h5_id = H5Aopen(radiance_id[k], "CentralWaveLength", H5P_DEFAULT);
            h5_status = H5Aread(h5_id, H5T_NATIVE_DOUBLE, &this->centralwavelength[k]);
            if(h5_status < 0)
                qDebug() << "CentralWaveLength not read !";
            h5_status =  H5Aclose(h5_id);
        }
        else
        {
            bandcorrectioncoefficientA[k] = 0.0;
            bandcorrectioncoefficientB[k] = 0.0;
            centralwavelength[k] = 0.0;
        }

    }

    qDebug() << QString("BandCorrectionCoefficientA = %1").arg(bandcorrectioncoefficientA[0]);
    qDebug() << QString("BandCorrectionCoefficientB = %1").arg(bandcorrectioncoefficientB[0]);
    qDebug() << QString("CentralWaveLength = %1").arg(centralwavelength[0]);

    if(iscolorimage)
    {
        h5_status = H5Dclose (radiance_id[0]);
        h5_status = H5Dclose (radiance_id[1]);
        h5_status = H5Dclose (radiance_id[2]);
    }
    else
        h5_status = H5Dclose (radiance_id[0]);
}

void SegmentVIIRSM::ReadVIIRSM_GEO_All(hid_t h5_file_id)
{
    hid_t   tiepoints_lat_id, tiepoints_lon_id;
    hid_t   aligncoef_id, expanscoef_id;
    herr_t  h5_status;

    tiepoints_lat.reset(new float[96 * 201]);
    tiepoints_lon.reset(new float[96 * 201]);
    aligncoef.reset(new float[200]);
    expanscoef.reset(new float[200]);
    geolongitude.reset(new float[768 * 3200]);
    geolatitude.reset(new float[768 * 3200]);


    tiepoints_lat_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-MOD-GEO_All/Latitude", H5P_DEFAULT);
    tiepoints_lon_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-MOD-GEO_All/Longitude", H5P_DEFAULT);
    aligncoef_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-MOD-GEO_All/AlignmentCoefficient", H5P_DEFAULT);
    expanscoef_id = H5Dopen2(h5_file_id, "/All_Data/VIIRS-MOD-GEO_All/ExpansionCoefficient", H5P_DEFAULT);

    if((h5_status = H5Dread (tiepoints_lat_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, tiepoints_lat.data())) < 0)
        fprintf(stderr, "unable to read latitude dataset");

    if((h5_status = H5Dread (tiepoints_lon_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, tiepoints_lon.data())) < 0)
        fprintf(stderr, "unable to read longitude dataset");

    if((h5_status = H5Dread (aligncoef_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, aligncoef.data())) < 0)
        fprintf(stderr, "unable to read AlignmentCoefficient dataset");

    if((h5_status = H5Dread (expanscoef_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, expanscoef.data())) < 0)
        fprintf(stderr, "unable to read ExpansionCoefficient dataset");

    h5_status = H5Dclose (tiepoints_lat_id);
    h5_status = H5Dclose (tiepoints_lon_id);
    h5_status = H5Dclose (aligncoef_id);
    h5_status = H5Dclose (expanscoef_id);


}

Segment *SegmentVIIRSM::ReadDatasetsInMemory()
{
    qDebug() << "Segment *SegmentVIIRS::ReadDatasetsInMemory()";

    hid_t   h5_file_id;
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

    ReadVIIRSM_SDR_All(h5_file_id);

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

    minBrightnessTemp = getBrightnessTemp(stat_min_ch[0]);
    maxBrightnessTemp = getBrightnessTemp(stat_max_ch[0]);

    h5_status = H5Fclose (h5_file_id);

    qDebug() << QString("ptrbaVIIRS min_ch[0] = %1 max_ch[0] = %2").arg(stat_min_ch[0]).arg(stat_max_ch[0]);
    qDebug() << QString("Radiance min = %1 W/sr*cm*cm max = %2 W/sr*cm*cm").arg(getRadiance(stat_min_ch[0])).arg(getRadiance(stat_max_ch[0]));
    qDebug() << QString("Brightness Temp min = %1 Kelvin  max = %2 Kelvin").arg(getBrightnessTemp(stat_min_ch[0])).arg(getBrightnessTemp(stat_max_ch[0]));


    return this;

}


QString SegmentVIIRSM::getDatasetNameFromBand()
{

    if(bandlist.at(1))
    {
        invertthissegment[0] = invertlist.at(0);
        return("/All_Data/VIIRS-M1-SDR_All/Radiance");
    }
    else if(bandlist.at(2))
    {
        invertthissegment[0] = invertlist.at(1);
        return("/All_Data/VIIRS-M2-SDR_All/Radiance");
    }
    else if(bandlist.at(3))
    {
        invertthissegment[0] = invertlist.at(2);
        return("/All_Data/VIIRS-M3-SDR_All/Radiance");
    }
    else if(bandlist.at(4))
    {
        invertthissegment[0] = invertlist.at(3);
        return("/All_Data/VIIRS-M4-SDR_All/Radiance");
    }
    else if(bandlist.at(5))
    {
        invertthissegment[0] = invertlist.at(4);
        return("/All_Data/VIIRS-M5-SDR_All/Radiance");
    }
    else if(bandlist.at(6))
    {
        invertthissegment[0] = invertlist.at(5);
        return("/All_Data/VIIRS-M6-SDR_All/Radiance");
    }
    else if(bandlist.at(7))
    {
        invertthissegment[0] = invertlist.at(6);
        return("/All_Data/VIIRS-M7-SDR_All/Radiance");
    }
    else if(bandlist.at(8))
    {
        invertthissegment[0] = invertlist.at(7);
        return("/All_Data/VIIRS-M8-SDR_All/Radiance");
    }
    else if(bandlist.at(9))
    {
        invertthissegment[0] = invertlist.at(8);
        return("/All_Data/VIIRS-M9-SDR_All/Radiance");
    }
    else if(bandlist.at(10))
    {
        invertthissegment[0] = invertlist.at(9);
        return("/All_Data/VIIRS-M10-SDR_All/Radiance");
    }
    else if(bandlist.at(11))
    {
        invertthissegment[0] = invertlist.at(10);
        return("/All_Data/VIIRS-M11-SDR_All/Radiance");
    }
    else if(bandlist.at(12))
    {
        invertthissegment[0] = invertlist.at(11);
        return("/All_Data/VIIRS-M12-SDR_All/Radiance");
    }
    else if(bandlist.at(13))
    {
        invertthissegment[0] = invertlist.at(12);
        return("/All_Data/VIIRS-M13-SDR_All/Radiance");
    }
    else if(bandlist.at(14))
    {
        invertthissegment[0] = invertlist.at(13);
        return("/All_Data/VIIRS-M14-SDR_All/Radiance");
    }
    else if(bandlist.at(15))
    {
        invertthissegment[0] = invertlist.at(14);
        return("/All_Data/VIIRS-M15-SDR_All/Radiance");
    }
    else if(bandlist.at(16))
    {
        invertthissegment[0] = invertlist.at(15);
        return("/All_Data/VIIRS-M16-SDR_All/Radiance");
    }
    else
        return "";


}

QString SegmentVIIRSM::getDatasetNameFromColor(int colorindex)
{
    Q_ASSERT(colorindex >=0 && colorindex < 3);
    colorindex++; // 1, 2 or 3

    if(colorlist.at(0) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(0);
        return("/All_Data/VIIRS-M1-SDR_All/Radiance");
    }
    else if(colorlist.at(1) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(1);
        return("/All_Data/VIIRS-M2-SDR_All/Radiance");
    }
    else if(colorlist.at(2) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(2);
        return("/All_Data/VIIRS-M3-SDR_All/Radiance");
    }
    else if(colorlist.at(3) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(3);
        return("/All_Data/VIIRS-M4-SDR_All/Radiance");
    }
    else if(colorlist.at(4) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(4);
        return("/All_Data/VIIRS-M5-SDR_All/Radiance");
    }
    else if(colorlist.at(5) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(5);
        return("/All_Data/VIIRS-M6-SDR_All/Radiance");
    }
    else if(colorlist.at(6) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(6);
        return("/All_Data/VIIRS-M7-SDR_All/Radiance");
    }
    else if(colorlist.at(7) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(7);
        return("/All_Data/VIIRS-M8-SDR_All/Radiance");
    }
    else if(colorlist.at(8) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(8);
        return("/All_Data/VIIRS-M9-SDR_All/Radiance");
    }
    else if(colorlist.at(9) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(9);
        return("/All_Data/VIIRS-M10-SDR_All/Radiance");
    }
    else if(colorlist.at(10) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(10);
        return("/All_Data/VIIRS-M11-SDR_All/Radiance");
    }
    else if(colorlist.at(11) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(11);
        return("/All_Data/VIIRS-M12-SDR_All/Radiance");
    }
    else if(colorlist.at(12) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(12);
        return("/All_Data/VIIRS-M13-SDR_All/Radiance");
    }
    else if(colorlist.at(13) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(13);
        return("/All_Data/VIIRS-M14-SDR_All/Radiance");
    }
    else if(colorlist.at(14) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(14);
        return("/All_Data/VIIRS-M15-SDR_All/Radiance");
    }
    else if(colorlist.at(15) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(15);
        return("/All_Data/VIIRS-M16-SDR_All/Radiance");
    }
}


void SegmentVIIRSM::GetAlpha( float &ascan, float &atrack, int rels, int relt, int iscan)
{
    ascan = s[rels] + s[rels] * (1 - s[rels]) * expanscoef[iscan] + s[relt] * (1 - s[relt]) * aligncoef[iscan];
    atrack = s[relt];
}

void SegmentVIIRSM::CalcGeoLocations(int itrack, int iscan)  // 0 <= itrack < 48 ; 0 <= iscan < 200
{
    int iA, iB, iC, iD;
    int jA, jB, jC, jD;
    float lat_A, lat_B, lat_C, lat_D;
    float lon_A, lon_B, lon_C, lon_D;
    quint16 val_A, val_B, val_C, val_D;

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

//    if(itrack == 0)
//        qDebug() << QString("itrack = %1 iscan = %2 Lat tiepoint A = %3 B = %4 C = %5 D = %6").arg(itrack).arg(iscan).arg(lat_A).arg(lat_B).arg(lat_C).arg(lat_D);

    lon_A = tiepoints_lon[iA * 201 + jA];
    lon_B = tiepoints_lon[iB * 201 + jB];
    lon_C = tiepoints_lon[iC * 201 + jC];
    lon_D = tiepoints_lon[iD * 201 + jD];


    val_A = ptrbaVIIRS[0][((itrack * 16)) * 3200 + (iscan * 16)];
    val_B = ptrbaVIIRS[0][((itrack * 16)) * 3200 + (iscan * 16) + 15];
    val_C = ptrbaVIIRS[0][((itrack * 16) + 15) * 3200 + (iscan * 16) + 15];
    val_D = ptrbaVIIRS[0][((itrack * 16) + 15) * 3200 + (iscan * 16)];

    if( val_A == 0 || val_A > 65528 || val_B == 0 || val_B > 65528 || val_C == 0 || val_C > 65528 || val_D == 0 || val_D > 65528)
    {
        quint16 minval = Min(val_A, val_B, val_C, val_D);

        for(int relt = 0; relt < 16; relt++)
        {
            for(int rels = 0; rels < 16; rels++)
            {
                if(ptrbaVIIRS[0][((itrack * 16) + relt) * 3200 + (iscan * 16) + rels] == 0 || ptrbaVIIRS[0][((itrack * 16) + relt) * 3200 + (iscan * 16) + rels] >= 65528)
                {
                    geolatitude[((itrack * 16) + relt) * 3200 + (iscan * 16) + rels] = 65535;
                    geolongitude[((itrack * 16) + relt) * 3200 + (iscan * 16) + rels] = 65535;
                }
            }
        }
    }

//    if(itrack == 0)
//        qDebug() << QString("itrack = %1 iscan = %2 Lon tiepoint A = %3 B = %4 C = %5 D = %6").arg(itrack).arg(iscan).arg(lon_A).arg(lon_B).arg(lon_C).arg(lon_D);

    float themin = Minf(lon_A, lon_B, lon_C, lon_D);
    float themax = Maxf(lon_A, lon_B, lon_C, lon_D);


    if (Maxf(abs(lat_A), abs(lat_B), abs(lat_C), abs(lat_D)) > 60.0 || (themax - themin) > 90.0)
        interpolateViaVector(itrack, iscan, lon_A, lon_B, lon_C, lon_D, lat_A, lat_B, lat_C, lat_D);
    else
        interpolateViaLonLat(itrack, iscan, lon_A, lon_B, lon_C, lon_D, lat_A, lat_B, lat_C, lat_D);

}

void SegmentVIIRSM::interpolateViaLonLat(int itrack, int iscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D)
{

    float ascan, atrack;
    float lat_1, lat_2, lat;
    float lon_1, lon_2, lon;

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

//    if( itrack == 0 && (iscan == 0 || iscan == 1))
//    {
//        for (int relt = 0; relt < 16; relt++) {
//            for (int rels = 0; rels < 16; rels++)
//               cout << " " <<  geolatitude[((itrack * 16) + relt) * 3200 + (iscan * 16) + rels];
//            cout << endl;
//        }
//    }

}

void SegmentVIIRSM::interpolateViaVector(int itrack, int iscan, float lon_A, float lon_B, float lon_C, float lon_D, float lat_A, float lat_B, float lat_C, float lat_D)
{
    float ascan, atrack;
    float lon, lat;

    // Earth Centred vectors
    float lat_A_rad = lat_A * PI / 180.0;
    float lon_A_rad = lon_A * PI / 180.0;
    float lat_B_rad = lat_B * PI / 180.0;
    float lon_B_rad = lon_B * PI / 180.0;
    float lat_C_rad = lat_C * PI / 180.0;
    float lon_C_rad = lon_C * PI / 180.0;
    float lat_D_rad = lat_D * PI / 180.0;
    float lon_D_rad = lon_D * PI / 180.0;

    float x_A_ec = cos(lat_A_rad) * cos(lon_A_rad);
    float y_A_ec = cos(lat_A_rad) * sin(lon_A_rad);
    float z_A_ec = sin(lat_A_rad);

    float x_B_ec = cos(lat_B_rad) * cos(lon_B_rad);
    float y_B_ec = cos(lat_B_rad) * sin(lon_B_rad);
    float z_B_ec = sin(lat_B_rad);

    float x_C_ec = cos(lat_C_rad) * cos(lon_C_rad);
    float y_C_ec = cos(lat_C_rad) * sin(lon_C_rad);
    float z_C_ec = sin(lat_C_rad);

    float x_D_ec = cos(lat_D_rad) * cos(lon_D_rad);
    float y_D_ec = cos(lat_D_rad) * sin(lon_D_rad);
    float z_D_ec = sin(lat_D_rad);


    float x1, y1, z1;
    float x2, y2, z2;
    float x, y, z;
    float lon_deg, lat_deg;

    for(int relt = 0; relt < 16; relt++)
    {
        for(int rels = 0; rels < 16; rels++)
        {
            GetAlpha(ascan, atrack, rels, relt, iscan);
            // 96 x 201

            x1 = (1 - ascan) * x_A_ec + ascan * x_B_ec;
            y1 = (1 - ascan) * y_A_ec + ascan * y_B_ec;
            z1 = (1 - ascan) * z_A_ec + ascan * z_B_ec;

            x2 = (1 - ascan) * x_D_ec + ascan * x_C_ec;
            y2 = (1 - ascan) * y_D_ec + ascan * y_C_ec;
            z2 = (1 - ascan) * z_D_ec + ascan * z_C_ec;

            x = (1 - atrack) * x1 + atrack * x2;
            y = (1 - atrack) * y1 + atrack * y2;
            z = (1 - atrack) * z1 + atrack * z2;

            lon_deg = atan2(y, x) * 180.0/PI;
            lat_deg = atan2(z, sqrt(x * x + y * y)) * 180.0/PI;

            geolatitude[((itrack * 16) + relt) * 3200 + (iscan * 16) + rels] = lat_deg;
            geolongitude[((itrack * 16) + relt) * 3200 + (iscan * 16) + rels] = lon_deg;


        }
    }

//    if( itrack == 0)
//    {
//        cout << "geolatitude" << endl;
//        for (int relt = 0; relt < 16; relt++) {
//            for (int rels = 0; rels < 16; rels++)
//               cout << " " <<  geolatitude[((itrack * 16) + relt) * 3200 + (iscan * 16) + rels];
//            cout << endl;
//        }
//        cout << "geolongitude" << endl;
//        for (int relt = 0; relt < 16; relt++) {
//            for (int rels = 0; rels < 16; rels++)
//               cout << " " <<  geolongitude[((itrack * 16) + relt) * 3200 + (iscan * 16) + rels];
//            cout << endl;
//        }
//    }

}

int SegmentVIIRSM::ReadNbrOfLines()
{
    return NbrOfLines;
}

void SegmentVIIRSM::ComposeSegmentImage()
{

    QRgb *row;
    int indexout[3];

    qDebug() << QString("SegmentVIIRS::ComposeSegmentImage() segm->startLineNbr = %1").arg(this->startLineNbr);
    qDebug() << QString("SegmentVIIRS::ComposeSegmentImage() color = %1 ").arg(bandlist.at(0));
    qDebug() << QString("SegmentVIIRS::ComposeSegmentImage() invertthissegment[0] = %1").arg(invertthissegment[0]);
    qDebug() << QString("SegmentVIIRS::ComposeSegmentImage() invertthissegment[1] = %1").arg(invertthissegment[1]);
    qDebug() << QString("SegmentVIIRS::ComposeSegmentImage() invertthissegment[2] = %1").arg(invertthissegment[2]);

    int pixval[3];
    int r, g, b;

    bool color = bandlist.at(0);
    bool valok[3];

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        row = (QRgb*)imageptrs->ptrimageViirsM->scanLine(this->startLineNbr + line);
        for (int pixelx = 0; pixelx < 3200; pixelx++)
        {
            pixval[0] = *(this->ptrbaVIIRS[0].data() + line * 3200 + pixelx);
            if(color)
            {
                pixval[1] = *(this->ptrbaVIIRS[1].data() + line * 3200 + pixelx);
                pixval[2] = *(this->ptrbaVIIRS[2].data() + line * 3200 + pixelx);
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
                {
                    if(invertthissegment[0])
                    {
                        r = 255 - imageptrs->lut_ch[0][indexout[0]];
                    }
                    else
                        r = imageptrs->lut_ch[0][indexout[0]];
                    if(invertthissegment[1])
                    {
                        g = 255 - imageptrs->lut_ch[1][indexout[1]];
                    }
                    else
                        g = imageptrs->lut_ch[1][indexout[1]];
                    if(invertthissegment[2])
                    {
                        b = 255 - imageptrs->lut_ch[2][indexout[2]];
                    }
                    else
                        b = imageptrs->lut_ch[2][indexout[2]];

                    row[pixelx] = qRgb(r, g, b );
                }
                else
                {
                    if(invertthissegment[0])
                    {
                        r = 255 - imageptrs->lut_ch[0][indexout[0]];
                    }
                    else
                        r = imageptrs->lut_ch[0][indexout[0]];

                    row[pixelx] = qRgb(r, r, r );
                }

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

void SegmentVIIRSM::recalculateStatsInProjection()
{
    bool color = bandlist.at(0);
    int x, y;

    int statmax[3], statmin[3];
    long active_pixels[3];

    for(int k = 0; k < 3; k++)
    {
        statmax[k] = 0;
        statmin[k] = 999999;
        active_pixels[k] = 0;
    }

    for(int k = 0; k < (color ? 3 : 1); k++)
    {
        for (int j = 0; j < 768; j++)
        {
            for (int i = 0; i < 3200; i++)
            {
                x = *(this->projectionCoordX.data() + j * 3200 + i);
                y = *(this->projectionCoordY.data() + j * 3200 + i);
                if(x >= 0 && x < imageptrs->ptrimageProjection->width() && y >= 0 && y < imageptrs->ptrimageProjection->height())
                {
                    if(ptrbaVIIRS[k][j * 3200 + i] >= statmax[k])
                        statmax[k] = ptrbaVIIRS[k][j * 3200 + i];
                    if(ptrbaVIIRS[k][j * 3200 + i] < statmin[k])
                        statmin[k] = ptrbaVIIRS[k][j * 3200 + i];
                    active_pixels[k]++;
                }
            }
        }
    }

    for(int k = 0; k < 3; k++)
    {
        stat_max_projection[k] = statmax[k];
        stat_min_projection[k] = statmin[k];
    }
    active_pixels_projection = active_pixels[0];

}


void SegmentVIIRSM::ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(LCC, histogrammethod, normalized);
}

void SegmentVIIRSM::ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized)
{
    ComposeProjection(GVP, histogrammethod, normalized);
}

void SegmentVIIRSM::ComposeSegmentSGProjection(int inputchannel, int histogrammethod,  bool normalized)
{
    ComposeProjection(SG, histogrammethod, normalized);
}

void SegmentVIIRSM::ComposeProjection(eProjections proj, int histogrammethod, bool normalized)
{

    double map_x, map_y;

    float lonpos1, latpos1;


    int pixval[3];

    bool color = bandlist.at(0);
    bool valok[3];

    projectionCoordX.reset(new qint32[768 * 3200]);
    projectionCoordY.reset(new qint32[768 * 3200]);
    projectionCoordValue.reset(new QRgb[768 * 3200]);

    for( int i = 0; i < 768; i++)
    {
        for( int j = 0; j < 3200 ; j++ )
        {
            projectionCoordX[i * 3200 + j] = 65535;
            projectionCoordY[i * 3200 + j] = 65535;
            projectionCoordValue[i * 3200 + j] = qRgba(0, 0, 0, 0);
        }
    }
    qDebug() << "SegmentVIIRS::ComposeProjection(eProjections proj)";

    for( int i = 0; i < this->NbrOfLines; i++)
    {
        for( int j = 0; j < this->earth_views_per_scanline ; j++ )
        {
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
                latpos1 = geolatitude[i * 3200 + j];
                lonpos1 = geolongitude[i * 3200 + j];

                if(proj == LCC) //Lambert
                {
                    if(imageptrs->lcc->map_forward_neg_coord(lonpos1 * PI / 180.0, latpos1 * PI / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color);
                    }
                }
                else if(proj == GVP) // General Vertical Perspecitve
                {
                    if(imageptrs->gvp->map_forward_neg_coord(lonpos1 * PI / 180.0, latpos1 * PI / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color);
                    }

                }
                else if(proj == SG) // Stereographic
                {
                    if(imageptrs->sg->map_forward_neg_coord(lonpos1 * PI / 180.0, latpos1 * PI / 180.0, map_x, map_y))
                    {
                        MapPixel( i, j, map_x, map_y, color);
                    }
                }
            } else
            {
                projectionCoordX[i * 3200 + j] = 65535;
                projectionCoordY[i * 3200 + j] = 65535;
                projectionCoordValue[i * 3200 + j] = qRgba(0, 0, 0, 0);
            }
        }
    }

}

void SegmentVIIRSM::RecalculateProjection()
{

    int indexout[3];
    int pixval[3];
    int r, g, b;
    QRgb rgbvalue = qRgb(0,0,0);

    int map_x, map_y;

    bool color = bandlist.at(0);
    bool valok[3];

    for( int i = 0; i < this->NbrOfLines; i++)
    {
        for( int j = 0; j < this->earth_views_per_scanline ; j++ )
        {
            pixval[0] = ptrbaVIIRS[0][i * 3200 + j];

            if(color)
            {
                pixval[1] = ptrbaVIIRS[1][i * 3200 + j];
                pixval[2] = ptrbaVIIRS[2][i * 3200 + j];
            }


            map_x = projectionCoordX[i * 3200 + j];
            map_y = projectionCoordY[i * 3200 + j];

            if (map_x > -15 && map_x < imageptrs->ptrimageProjection->width() + 15 && map_y > -15 && map_y < imageptrs->ptrimageProjection->height() + 15)
            {


                for(int i = 0; i < (color ? 3 : 1); i++)
                {
                    indexout[i] =  (int)(255 * ( pixval[i] - imageptrs->stat_min_ch[i] ) / (imageptrs->stat_max_ch[i] - imageptrs->stat_min_ch[i]));
                    indexout[i] = ( indexout[i] > 255 ? 255 : indexout[i] );
                }

                if(color)
                {
                    if(invertthissegment[0])
                    {
                        r = 255 - imageptrs->lut_ch[0][indexout[0]];
                    }
                    else
                        r = imageptrs->lut_ch[0][indexout[0]];
                    if(invertthissegment[1])
                    {
                        g = 255 - imageptrs->lut_ch[1][indexout[1]];
                    }
                    else
                        g = imageptrs->lut_ch[1][indexout[1]];
                    if(invertthissegment[2])
                    {
                        b = 255 - imageptrs->lut_ch[2][indexout[2]];
                    }
                    else
                        b = imageptrs->lut_ch[2][indexout[2]];

                    //rgbvalue  = qRgb(imageptrs->lut_ch[0][indexout[0]], imageptrs->lut_ch[1][indexout[1]], imageptrs->lut_ch[2][indexout[2]] );
                    rgbvalue = qRgba(r, g, b, 255);

                }
                else
                {
                    if(invertthissegment[0])
                    {
                        r = 255 - imageptrs->lut_ch[0][indexout[0]];
                    }
                    else
                        r = imageptrs->lut_ch[0][indexout[0]];

                    rgbvalue = qRgba(r, r, r, 255);
                }

                if(opts.sattrackinimage)
                {
                    if(j == 1598 || j == 1599 || j == 1600 || j == 1601 )
                    {
                        rgbvalue = qRgb(250, 0, 0);
                        if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
                            imageptrs->ptrimageProjection->setPixel(map_x, map_y, rgbvalue);
                    }
                    else
                    {
                        if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
                            imageptrs->ptrimageProjection->setPixel(map_x, map_y, rgbvalue);

                    }
                }
                else
                {
                    if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
                        imageptrs->ptrimageProjection->setPixel(map_x, map_y, rgbvalue);
                }

            }
        }
    }

}

//    long cntcoord = 0;
//    long cnttotal = 0;

//    for( int i = 0; i < this->NbrOfLines; i++)
//    {
//        for( int j = 0; j < this->earth_views_per_scanline ; j++ )
//        {
//            if(projectionCoordX[i * 3200 + j] != 65535)
//                cntcoord++;
//            cnttotal++;

//        }
//    }

//    qDebug() << QString("Nbr of pixels in projection active = %1  ; total = %2").arg(cntcoord).arg(cnttotal);

/*
    int maxX = 0;
    int maxY = 0;

    fprintf(stderr, "projectionCoordX \n");
    for( int i = 0; i < 16; i++) //this->NbrOfLines - 1; i++)
    {
        for( int j = 0; j < 16; j++) //this->earth_views_per_scanline - 1 ; j++ )
        {
            if(projectionCoordX[i * 3200 + j] < 10000 && projectionCoordX[i * 3200 + j + 1] < 10000)
            {
                if(maxX < abs(projectionCoordX[i * 3200 + j] - projectionCoordX[i * 3200 + j + 1]) )
                    maxX = abs(projectionCoordX[i * 3200 + j] - projectionCoordX[i * 3200 + j + 1]);
                if(maxY < abs(projectionCoordY[i * 3200 + j] - projectionCoordY[i * 3200 + j + 1]) )
                    maxY = abs(projectionCoordY[i * 3200 + j] - projectionCoordY[i * 3200 + j + 1]);
            }

            fprintf(stderr, "%u ", projectionCoordX[i * 3200 + j]);
        }

        fprintf(stderr, "\n");
    }

    fprintf(stderr, "projectionCoordY \n");

    for( int i = 0; i < 16; i++) //this->NbrOfLines - 1; i++)
    {
        for( int j = 0; j < 16; j++) //this->earth_views_per_scanline - 1 ; j++ )
        {
            fprintf(stderr, "%u ", projectionCoordY[i * 3200 + j]);
        }

        fprintf(stderr, "\n");
    }

    qDebug() << QString("===============>>>>>>>>>>> maxX = %1 maxY = %2").arg(maxX).arg(maxY);

*/


   // g_mutex.unlock();



//void SegmentVIIRS::ComposeProjectionConcurrent()
//{
//    if( geolatitude == 0x0)
//    {
//        qDebug() << "pointer to geolatitude = 0x0 !!!!!!!!!!!";
//    }

//    bool color = bandlist.at(0);
//    bool valok[3];
//    int pixval[3];

//    LonLatMax();

//    int col, row;
//    double lon_rad, lat_rad;
//    int counter = 0;
//    qDebug() << "=====> start SegmentVIIRS::ComposeProjectionConcurrent";
//    int cntTotal = imageptrs->ptrimageProjection->height() * imageptrs->ptrimageProjection->width();

//    //testlookupLonLat(55.888, 73.900, col, row);
//    //return;

//    g_mutex.lock();

//    for (int j = 0; j < imageptrs->ptrimageProjection->height(); j++)
//    {
//        for (int i = 0; i < imageptrs->ptrimageProjection->width(); i++)
//        {
//            if (imageptrs->gvp->map_inverse(i, j, lon_rad, lat_rad))
//            {
//                if(this->lookupLonLat(lon_rad*180.0/PI, lat_rad*180/PI, col, row))
//                {
//                    if(col == 1500 && row == 400)
//                        counter++;
//                    pixval[0] = ptrbaVIIRS[0][col * 3200 + row];
//                    valok[0] = pixval[0] > 0 && pixval[0] < 65528;
//                    if(color)
//                    {
//                        pixval[1] = ptrbaVIIRS[1][col * 3200 + row];
//                        pixval[2] = ptrbaVIIRS[2][col * 3200 + row];
//                        valok[1] = pixval[1] > 0 && pixval[1] < 65528;
//                        valok[2] = pixval[2] > 0 && pixval[2] < 65528;
//                    }

//                    if( valok[0] && (color ? valok[1] && valok[2] : true))
//                        MapPixel(col, row, i, j, color);
//                }

//            }
//        }
//    }

//    g_mutex.unlock();

//    qDebug() << "=====> end SegmentVIIRS::ComposeProjectionConcurrent counter = " << counter << " from total = " << cntTotal;

//}

void SegmentVIIRSM::LonLatMax()
{

    lonMin = +180.0;
    lonMax = -180.0;
    latMin = +90.0;
    latMax = -90.0;

    for (int j = 0; j < 768; j+=1)
    {
        for (int i = 0; i < 3200; i+=1)
        {
           if(geolatitude[j * 3200 + i] > latMax)
               latMax = geolatitude[j * 3200 + i];
           if(geolatitude[j * 3200 + i] <= latMin)
               latMin = geolatitude[j * 3200 + i];
           if(geolongitude[j * 3200 + i] > lonMax)
               lonMax = geolongitude[j * 3200 + i];
           if(geolongitude[j * 3200 + i] <= lonMin)
               lonMin = geolongitude[j * 3200 + i];
        }
     }

    qDebug() << QString("Minimum Latitude = %1 ; Maximum Latitude = %2").arg(latMin).arg(latMax);
    qDebug() << QString("Minimum Longitude = %1 ; Maximum Longitude = %2").arg(lonMin).arg(lonMax);


}


void SegmentVIIRSM::RenderSegmentlineInTextureVIIRS( int nbrLine, QRgb *row )
{

    QColor rgb;
    int posx, posy;

    QMutexLocker locker(&g_mutex);

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

}


void SegmentVIIRSM::MapPixel( int lines, int views, double map_x, double map_y, bool color)
{
    int indexout[3];
    int pixval[3];
    int r, g, b;
    QRgb rgbvalue = qRgb(0,0,0);

    pixval[0] = ptrbaVIIRS[0][lines * 3200 + views];
    if(color)
    {
        pixval[1] = ptrbaVIIRS[1][lines * 3200 + views];
        pixval[2] = ptrbaVIIRS[2][lines * 3200 + views];
    }

    if (map_x > -15 && map_x < imageptrs->ptrimageProjection->width() + 15 && map_y > -15 && map_y < imageptrs->ptrimageProjection->height() + 15)
    {

        projectionCoordX[lines * 3200 + views] = (qint32)map_x;
        projectionCoordY[lines * 3200 + views] = (qint32)map_y;

        for(int i = 0; i < (color ? 3 : 1); i++)
        {
            indexout[i] =  (int)(255 * ( pixval[i] - imageptrs->stat_min_ch[i] ) / (imageptrs->stat_max_ch[i] - imageptrs->stat_min_ch[i]));
            indexout[i] = ( indexout[i] > 255 ? 255 : indexout[i] );
        }

        if(color)
        {
            if(invertthissegment[0])
            {
                r = 255 - imageptrs->lut_ch[0][indexout[0]];
            }
            else
                r = imageptrs->lut_ch[0][indexout[0]];
            if(invertthissegment[1])
            {
                g = 255 - imageptrs->lut_ch[1][indexout[1]];
            }
            else
                g = imageptrs->lut_ch[1][indexout[1]];
            if(invertthissegment[2])
            {
                b = 255 - imageptrs->lut_ch[2][indexout[2]];
            }
            else
                b = imageptrs->lut_ch[2][indexout[2]];

            //rgbvalue  = qRgb(imageptrs->lut_ch[0][indexout[0]], imageptrs->lut_ch[1][indexout[1]], imageptrs->lut_ch[2][indexout[2]] );
            rgbvalue = qRgba(r, g, b, 255);

        }
        else
        {
            if(invertthissegment[0])
            {
                r = 255 - imageptrs->lut_ch[0][indexout[0]];
            }
            else
                r = imageptrs->lut_ch[0][indexout[0]];

             rgbvalue = qRgba(r, r, r, 255);
        }

        if(opts.sattrackinimage)
        {
            if(views == 1598 || views == 1599 || views == 1600 || views == 1601 )
            {
                rgbvalue = qRgb(250, 0, 0);
                if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
            }
            else
            {
                if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
                    imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
                projectionCoordValue[lines * 3200 + views] = rgbvalue;

            }
        }
        else
        {
            if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
                imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
            projectionCoordValue[lines * 3200 + views] = rgbvalue;
        }
        if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
        {
            float tmp = getBrightnessTemp(lines, views);
            imageptrs->ptrProjectionBrightnessTemp[(int)map_y * imageptrs->ptrimageProjection->width() + (int)map_x] = tmp;
        }
    }
}

float SegmentVIIRSM::getBrightnessTemp(int lines, int views)
{
    float radiancefloat;
    int radint = this->ptrbaVIIRS[0][lines * 3200 + views];

    if(radint >= 0 && radint < threshold[0])
        radiancefloat = radianceoffsetlow[0] + radiancescalelow[0] * radint;
    else if(radint < 65527 && radint >= threshold[0])
        radiancefloat = radianceoffsethigh[0] + radiancescalehigh[0] * radint;
    else
        return -1.0;

    float factor1 = 1438768660.333E-11;
    float factor2 = 119.104393402E-19;
    double thepow = pow(centralwavelength[0], 5);
    float ln = log(1 + factor2/(radiancefloat*10.0E4 * thepow));
    float bt = (factor1 * bandcorrectioncoefficientA[0]/(centralwavelength[0]*ln)) + bandcorrectioncoefficientB[0];

    return bt;
}

float SegmentVIIRSM::getBrightnessTemp(int radiance)
{

    float radiancefloat;
    if(radiance >= 0 && radiance < threshold[0])
        radiancefloat = radianceoffsetlow[0] + radiancescalelow[0] * radiance;
    else if(radiance < 65527 && radiance >= threshold[0])
        radiancefloat = radianceoffsethigh[0] + radiancescalehigh[0] * radiance;
    else
        return -1.0;

    float factor1 = 1438768660.333E-11;
    float factor2 = 119.104393402E-19;
    double thepow = pow(centralwavelength[0], 5);
    float ln = log(1 + factor2/(radiancefloat * 10.0E4 * thepow)); // radiance in W/sr*m*m
    float bt = (factor1 * bandcorrectioncoefficientA[0]/(centralwavelength[0]*ln)) + bandcorrectioncoefficientB[0];

    return bt;
}

float SegmentVIIRSM::getRadiance(int lines, int views) // in W/sr*cm*cm
{

    float radiancefloat;
    int radint = this->ptrbaVIIRS[0][lines * 3200 + views];

    if(radint >= 0 && radint < threshold[0])
        radiancefloat = radianceoffsetlow[0] + radiancescalelow[0] * radint;
    else if(radint < 65527 && radint >= threshold[0])
        radiancefloat = radianceoffsethigh[0] + radiancescalehigh[0] * radint;
    else
        radiancefloat = -1.0;


    return radiancefloat;

}

float SegmentVIIRSM::getRadiance(int radiance) // in W/sr*cm*cm
{

    float radiancefloat;
    if(radiance >= 0 && radiance < threshold[0])
        radiancefloat = radianceoffsetlow[0] + radiancescalelow[0] * radiance;
    else if(radiance < 65527 && radiance >= threshold[0])
        radiancefloat = radianceoffsethigh[0] + radiancescalehigh[0] * radiance;
    else
        radiancefloat = -1.0;

    return radiancefloat;

}

float SegmentVIIRSM::Minf(const float v11, const float v12, const float v21, const float v22)
{
    float Minimum = v11;

    if( Minimum > v12 )
            Minimum = v12;
    if( Minimum > v21 )
            Minimum = v21;
    if( Minimum > v22 )
            Minimum = v22;

    return Minimum;
}

float SegmentVIIRSM::Maxf(const float v11, const float v12, const float v21, const float v22)
{
    int Maximum = v11;

    if( Maximum < v12 )
            Maximum = v12;
    if( Maximum < v21 )
            Maximum = v21;
    if( Maximum < v22 )
            Maximum = v22;

    return Maximum;
}

qint32 SegmentVIIRSM::Min(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22)
{
    qint32 Minimum = v11;

    if( Minimum > v12 )
            Minimum = v12;
    if( Minimum > v21 )
            Minimum = v21;
    if( Minimum > v22 )
            Minimum = v22;

    return Minimum;
}

qint32 SegmentVIIRSM::Max(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22)
{
    int Maximum = v11;

    if( Maximum < v12 )
            Maximum = v12;
    if( Maximum < v21 )
            Maximum = v21;
    if( Maximum < v22 )
            Maximum = v22;

    return Maximum;
}

//    float m00_A = -sin(lon_A);
//    float m01_A = cos(lon_A);
//    float m02_A = 0;
//    float m10_A = -sin(lat_A)*cos(lon_A);
//    float m11_A = -sin(lat_A)*sin(lon_A);
//    float m12_A = cos(lat_A);
//    float m20_A = cos(lat_A) * cos(lon_A);
//    float m21_A = cos(lat_A) * sin(lon_A);
//    float m22_A = sin(lat_A);

//    float m00_B = -sin(lon_B);
//    float m01_B = cos(lon_B);
//    float m02_B = 0;
//    float m10_B = -sin(lat_B)*cos(lon_B);
//    float m11_B = -sin(lat_B)*sin(lon_B);
//    float m12_B = cos(lat_B);
//    float m20_B = cos(lat_B) * cos(lon_B);
//    float m21_B = cos(lat_B) * sin(lon_B);
//    float m22_B = sin(lat_B);

//    float m00_C = -sin(lon_C);
//    float m01_C = cos(lon_C);
//    float m02_C = 0;
//    float m10_C = -sin(lat_C)*cos(lon_C);
//    float m11_C = -sin(lat_C)*sin(lon_C);
//    float m12_C = cos(lat_C);
//    float m20_C = cos(lat_C) * cos(lon_C);
//    float m21_C = cos(lat_C) * sin(lon_C);
//    float m22_C = sin(lat_C);

//    float m00_D = -sin(lon_D);
//    float m01_D = cos(lon_D);
//    float m02_D = 0;
//    float m10_D = -sin(lat_D)*cos(lon_D);
//    float m11_D = -sin(lat_D)*sin(lon_D);
//    float m12_D = cos(lat_D);
//    float m20_D = cos(lat_D) * cos(lon_D);
//    float m21_D = cos(lat_D) * sin(lon_D);
//    float m22_D = sin(lat_D);

//// Pixel centred
//    float x_A_pc = m00_A * x_A_ec + m10_A * y_A_ec + m20_A * z_A_ec;
//    float y_A_pc = m01_A * x_A_ec + m11_A * y_A_ec + m21_A * z_A_ec;
//    float z_A_pc = m02_A * x_A_ec + m12_A * y_A_ec + m22_A * z_A_ec;

//    float x_B_pc = m00_B * x_B_ec + m10_B * y_B_ec + m20_B * z_B_ec;
//    float y_B_pc = m01_B * x_B_ec + m11_B * y_B_ec + m21_B * z_B_ec;
//    float z_B_pc = m02_B * x_B_ec + m12_B * y_B_ec + m22_B * z_B_ec;

//    float x_C_pc = m00_C * x_C_ec + m10_C * y_C_ec + m20_C * z_C_ec;
//    float y_C_pc = m01_C * x_C_ec + m11_C * y_C_ec + m21_C * z_C_ec;
//    float z_C_pc = m02_C * x_C_ec + m12_C * y_C_ec + m22_C * z_C_ec;

//    float x_D_pc = m00_D * x_D_ec + m10_D * y_D_ec + m20_D * z_D_ec;
//    float y_D_pc = m01_D * x_D_ec + m11_D * y_D_ec + m21_D * z_D_ec;
//    float z_D_pc = m02_D * x_D_ec + m12_D * y_D_ec + m22_D * z_D_ec;
