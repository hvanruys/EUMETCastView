#include "segmentolciefr.h"
#include "segmentimage.h"
#include "options.h"
#include <QDebug>
#include "archive_entry.h"
#include <netcdf>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

extern Options opts;
extern SegmentImage *imageptrs;

#include <QMutex>
extern QMutex g_mutex;


SegmentOLCIefr::SegmentOLCIefr(QFile *filesegment, SatelliteList *satl, QObject *parent) :
  Segment(parent)
{

    bool ok;

    satlist = satl;

    fileInfo.setFile(*filesegment);
    segment_type = "OLCIEFR";
    segtype = eSegmentType::SEG_OLCIEFR;

    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
    //0         1         2         3         4         5         6         7         8         9         10
    //S3A_OL_1_EFR____20161026T161414_20161026T161714_20161026T175243_0179_010_168_4319_MAR_O_NR_002.SEN3.tar
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

    invertthissegment[0] = false;
    invertthissegment[1] = false;
    invertthissegment[2] = false;


}

Segment *SegmentOLCIefr::ReadSegmentInMemory()
{

    QString fname1;
    QString fname2;
    QString fname3;
    QByteArray array1;
    QByteArray array2;
    QByteArray array3;
    const char* pfname1;
    const char* pfname2;
    const char* pfname3;
    QString var1;
    QString var2;
    QString var3;
    const char* pvar1;
    const char* pvar2;
    const char* pvar3;
    NcVar radiance1;
    NcVar radiance2;
    NcVar radiance3;

    bool iscolorimage = this->bandlist.at(0);


    qDebug() << "Starting netCDF geo_coordinates";


    QString geofile = this->fileInfo.baseName() + ".SEN3/geo_coordinates.nc";
    QByteArray arraygeo = geofile.toUtf8();
    const char *pgeofile = arraygeo.constData();
    NcFile geoFile(pgeofile, NcFile::read);
    if(geoFile.isNull()) qDebug() << "error opening geoFile";


    NcDim coldim = geoFile.getDim("columns");
    NcDim rowdim = geoFile.getDim("rows");
    int columns = coldim.getSize();
    int rows = rowdim.getSize();


    this->longitude.reset(new int[columns * rows]);
    this->latitude.reset(new int[columns * rows]);

    NcVar geolon = geoFile.getVar("longitude");
    if(geolon.isNull()) qDebug() << "error getVar geolon";
    geolon.getVar(this->longitude.data());

    NcVar geolat = geoFile.getVar("latitude");
    if(geolat.isNull()) qDebug() << "error getVar geolat";
    geolat.getVar(this->latitude.data());

    if(iscolorimage)
    {
        qDebug() << "Starting netCDF color";
        getDatasetNameFromColor(0, &fname1, &var1);
        getDatasetNameFromColor(1, &fname2, &var2);
        getDatasetNameFromColor(2, &fname3, &var3);

        qDebug() << "getDatasetNameFromBand fname1 = " << fname1 << " var1 = " << var1;
        qDebug() << "getDatasetNameFromBand fname2 = " << fname2 << " var2 = " << var2;
        qDebug() << "getDatasetNameFromBand fname3 = " << fname3 << " var3 = " << var3;


        array1 = fname1.toUtf8();
        array2 = fname2.toUtf8();
        array3 = fname3.toUtf8();
        pfname1 = array1.constData();
        pfname2 = array2.constData();
        pfname3 = array3.constData();

        NcFile dataFile1(pfname1, NcFile::read);
        if(dataFile1.isNull()) qDebug() << "error opening dataFile1";

        NcFile dataFile2(pfname2, NcFile::read);
        if(dataFile2.isNull()) qDebug() << "error opening dataFile2";

        NcFile dataFile3(pfname3, NcFile::read);
        if(dataFile3.isNull()) qDebug() << "error opening dataFile3";


        NcDim coldim = dataFile1.getDim("columns");
        NcDim rowdim = dataFile1.getDim("rows");
        this->earth_views_per_scanline = coldim.getSize();
        this->NbrOfLines = rowdim.getSize();


        this->initializeMemory();

        for(int k = 0; k < 3; k++)
        {
            stat_max_ch[k] = 0;
            stat_min_ch[k] = 9999999;
            active_pixels[k] = 0;
        }


        array1 = var1.toUtf8();
        array2 = var2.toUtf8();
        array3 = var3.toUtf8();
        pvar1 = array1.constData();
        pvar2 = array2.constData();
        pvar3 = array3.constData();


        radiance1 = dataFile1.getVar(pvar1);
        if(radiance1.isNull()) qDebug() << "error getVar radiance1";
        radiance1.getVar(ptrbaOLCI[0].data());

        radiance2 = dataFile2.getVar(pvar2);
        if(radiance2.isNull()) qDebug() << "error getVar radiance2";
        radiance2.getVar(ptrbaOLCI[1].data());

        radiance3 = dataFile3.getVar(pvar3);
        if(radiance3.isNull()) qDebug() << "error getVar radiance3";
        radiance3.getVar(ptrbaOLCI[2].data());

    }
    else
    {
        qDebug() << "Starting netCDF mono";

        getDatasetNameFromBand(&fname1, &var1);
        qDebug() << "getDatasetNameFromBand fname1 = " << fname1 << " var1 = " << var1;
        array1 = fname1.toUtf8();
        pfname1 = array1.constData();
        NcFile dataFile1(pfname1, NcFile::read);
        if(dataFile1.isNull()) qDebug() << "error opening dataFile1";


        NcDim coldim = dataFile1.getDim("columns");
        NcDim rowdim = dataFile1.getDim("rows");
        this->earth_views_per_scanline = coldim.getSize();
        this->NbrOfLines = rowdim.getSize();

        qDebug() << QString("num_dims = %1 num_vars = %2 num_atts = %3 group_dim = %4")
                            .arg(dataFile1.getDimCount())
                            .arg(dataFile1.getVarCount())
                            .arg(dataFile1.getAttCount())
                            .arg(dataFile1.getGroupCount());

        this->initializeMemory();
        for(int k = 0; k < 3; k++)
        {
            stat_max_ch[k] = 0;
            stat_min_ch[k] = 9999999;
            active_pixels[k] = 0;
        }

        array1 = var1.toUtf8();
        pvar1 = array1.constData();

        radiance1 = dataFile1.getVar(pvar1);
        if(radiance1.isNull()) qDebug() << "error getVar radiance1";
        radiance1.getVar(ptrbaOLCI[0].data());

    }


//    for(int k = 0; k < (iscolorimage ? 3 : 1) ; k++)
//    {
//        fname[k] = iscolorimage ? getDatasetNameFromColor(k).toLatin1() : getDatasetNameFromBand().toLatin1();

//        qDebug() << "fname[k] = " << fname[k];

//      //  ncFile[k].setFileName(fname[k]);
//      //  tempfileexist = ncFile[k].exists();

//        QByteArray array = fname[k].toUtf8();
//        const char* pfname = array.constData();    bool tempfileexist;
//        mydataFile("S3A_OL_1_EFR____20161103T102748_20161103T103048_20161103T122733_0179_010_279_2160_MAR_O_NR_002.SEN3/Oa10_radiance.nc", NcFile::read);


//        if(mydataFile.isNull())
//            qDebug() << "error";
//        else
//            qDebug() << "file open";


//    try {
//        qDebug() << QString("num_dims = %1 num_vars = %2 num_atts = %3 group_dim = %4")
//                    .arg(mydataFile.getDimCount())
//                    .arg(mydataFile.getVarCount())
//                    .arg(mydataFile.getAttCount())
//                    .arg(mydataFile.getGroupCount());
//    }catch(NcException& e)
//         {
//           e.what();
//           cout<<"FAILURE*************************************"<<endl;
//           }

//        NcDim coldim = mydataFile.getDim("columns");
//        NcDim rowdim = mydataFile.getDim("rows");
//        this->earth_views_per_scanline = coldim.getSize();
//        this->NbrOfLines = rowdim.getSize();
//    }

//    this->initializeMemory();
//    for(int k = 0; k < 3; k++)
//    {
//        stat_max_ch[k] = 0;
//        stat_min_ch[k] = 9999999;
//        active_pixels[k] = 0;
//    }


//    for(int k = 0; k < (iscolorimage ? 3 : 1) ; k++)
//    {

//        QString varName = getVariableNameFromBand();

//        qDebug() << "varName = " << varName;
//        QByteArray array = varName.toUtf8();
//        const char* pvarname = array.constData();


//        radiance[k] = dataFile[k].getVar(pvarname);
//        if(radiance[k].isNull()) qDebug() << "error getVar";
//        radiance[k].getVar(ptrbaOLCI[k].data());
//    }

//    for(int k = 0; k < (iscolorimage ? 3 : 1); k++)
//    {
//        for(int j=0; j < NbrOfLines; j++)
//        {
//            for(int i=0; i < earth_views_per_scanline; i++)
//            {
//                if(ptrbaOLCI[k][j*earth_views_per_scanline + i] < 65535)
//                {
//                    if(ptrbaOLCI[k][j*earth_views_per_scanline + i] > stat_max_ch[k])
//                        stat_max_ch[k] = ptrbaOLCI[k][j*earth_views_per_scanline + i];
//                    if(ptrbaOLCI[k][j*earth_views_per_scanline + i] < stat_min_ch[k])
//                        stat_min_ch[k] = ptrbaOLCI[k][j*earth_views_per_scanline + i];
//                    active_pixels[k]++;
//                }

//            }
//        }
//    }



//    try
//    {

//        NcFile dataFile (pfname, NcFile::read);
//        if(dataFile.isNull())
//            qDebug() << "error";
//        else
//            qDebug() << "file open";
//        qDebug() << QString("num_dims = %1 num_vars = %2 num_atts = %3 group_dim = %4")
//                    .arg(dataFile.getDimCount())
//                    .arg(dataFile.getVarCount())
//                    .arg(dataFile.getAttCount())
//                    .arg(dataFile.getGroupCount());


//        NcDim coldim = dataFile.getDim("columns");
//        NcDim rowdim = dataFile.getDim("rows");
//        this->earth_views_per_scanline = coldim.getSize();
//        this->NbrOfLines = rowdim.getSize();

//        this->initializeMemory();

//        for(int k = 0; k < 3; k++)
//        {
//            stat_max_ch[k] = 0;
//            stat_min_ch[k] = 9999999;
//            active_pixels[k] = 0;
//        }

//        qDebug() <<  "Retrieve the variable named 'Oa01_radiance' ";
//        NcVar radiance = dataFile.getVar("Oa03_radiance");
//        if(radiance.isNull()) qDebug() << "error getVar";
//        radiance.getVar(ptrbaOLCI[0].data());

    for(int k = 0; k < (iscolorimage ? 3 : 1); k++)
    {
        for(int j=0; j < NbrOfLines; j++)
        {
            for(int i=0; i < earth_views_per_scanline; i++)
            {
                if(ptrbaOLCI[k][j*earth_views_per_scanline + i] < 65535)
                {
                    if(ptrbaOLCI[k][j*earth_views_per_scanline + i] > stat_max_ch[k])
                        stat_max_ch[k] = ptrbaOLCI[k][j*earth_views_per_scanline + i];
                    if(ptrbaOLCI[k][j*earth_views_per_scanline + i] < stat_min_ch[k])
                        stat_min_ch[k] = ptrbaOLCI[k][j*earth_views_per_scanline + i];
                    active_pixels[k]++;
                }

            }
        }
    }

    qDebug() << QString("ptrbaOLCI min_ch[0] = %1 max_ch[0] = %2").arg(stat_min_ch[0]).arg(stat_max_ch[0]);
    if(iscolorimage)
    {
        qDebug() << QString("ptrbaOLCI min_ch[1] = %1 max_ch[1] = %2").arg(stat_min_ch[1]).arg(stat_max_ch[1]);
        qDebug() << QString("ptrbaOLCI min_ch[2] = %1 max_ch[2] = %2").arg(stat_min_ch[2]).arg(stat_max_ch[2]);

    }


}

void SegmentOLCIefr::getDatasetNameFromColor(int colorindex, QString *datasetname, QString *variablename)
{
    qDebug() << "getDatasetNameFromColor colorindex = " << colorindex;

    Q_ASSERT(colorindex >=0 && colorindex < 3);
    colorindex++; // 1, 2 or 3

    if(colorlist.at(0) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(0);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa01_radiance.nc";
        *variablename = "Oa01_radiance";
    }
    else if(colorlist.at(1) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(1);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa02_radiance.nc";
        *variablename = "Oa02_radiance";
    }
    else if(colorlist.at(2) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(2);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa03_radiance.nc";
        *variablename = "Oa03_radiance";
    }
    else if(colorlist.at(3) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(3);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa04_radiance.nc";
        *variablename = "Oa04_radiance";
    }
    else if(colorlist.at(4) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(4);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa05_radiance.nc";
        *variablename = "Oa05_radiance";
    }
    else if(colorlist.at(5) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(5);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa06_radiance.nc";
        *variablename = "Oa06_radiance";
    }
    else if(colorlist.at(6) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(6);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa07_radiance.nc";
        *variablename = "Oa07_radiance";
    }
    else if(colorlist.at(7) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(7);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa08_radiance.nc";
        *variablename = "Oa08_radiance";
    }
    else if(colorlist.at(8) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(8);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa09_radiance.nc";
        *variablename = "Oa09_radiance";
    }
    else if(colorlist.at(9) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(9);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa10_radiance.nc";
        *variablename = "Oa10_radiance";
   }
    else if(colorlist.at(10) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(10);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa11_radiance.nc";
        *variablename = "Oa11_radiance";
    }
    else if(colorlist.at(11) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(11);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa12_radiance.nc";
        *variablename = "Oa12_radiance";
    }
    else if(colorlist.at(12) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(12);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa13_radiance.nc";
        *variablename = "Oa13_radiance";
    }
    else if(colorlist.at(13) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(13);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa14_radiance.nc";
        *variablename = "Oa14_radiance";
    }
    else if(colorlist.at(14) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(14);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa15_radiance.nc";
        *variablename = "Oa15_radiance";
    }
    else if(colorlist.at(15) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(15);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa16_radiance.nc";
        *variablename = "Oa16_radiance";
    }
    else if(colorlist.at(16) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(16);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa17_radiance.nc";
        *variablename = "Oa17_radiance";
    }
    else if(colorlist.at(17) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(17);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa18_radiance.nc";
        *variablename = "Oa18_radiance";
    }
    else if(colorlist.at(18) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(18);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa19_radiance.nc";
        *variablename = "Oa19_radiance";
    }
    else if(colorlist.at(19) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(19);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa20_radiance.nc";
        *variablename = "Oa20_radiance";
    }
    else if(colorlist.at(20) == colorindex)
    {
        invertthissegment[colorindex-1] = invertlist.at(20);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa21_radiance.nc";
        *variablename = "Oa21_radiance";
    }
}

void SegmentOLCIefr::getDatasetNameFromBand(QString *datasetname, QString *variablename)
{
    if(bandlist.at(1))
    {
        invertthissegment[0] = invertlist.at(0);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa01_radiance.nc";
        *variablename = "Oa01_radiance";
    }
    else if(bandlist.at(2))
    {
        invertthissegment[0] = invertlist.at(1);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa02_radiance.nc";
        *variablename = "Oa02_radiance";
    }
    else if(bandlist.at(3))
    {
        invertthissegment[0] = invertlist.at(2);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa03_radiance.nc";
        *variablename = "Oa03_radiance";
    }
    else if(bandlist.at(4))
    {
        invertthissegment[0] = invertlist.at(3);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa04_radiance.nc";
        *variablename = "Oa04_radiance";
    }
    else if(bandlist.at(5))
    {
        invertthissegment[0] = invertlist.at(4);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa05_radiance.nc";
        *variablename = "Oa05_radiance";
    }
    else if(bandlist.at(6))
    {
       invertthissegment[0] = invertlist.at(5);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa06_radiance.nc";
        *variablename = "Oa06_radiance";
    }
    else if(bandlist.at(7))
    {
        invertthissegment[0] = invertlist.at(6);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa07_radiance.nc";
        *variablename = "Oa07_radiance";
    }
    else if(bandlist.at(8))
    {
        invertthissegment[0] = invertlist.at(7);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa08_radiance.nc";
        *variablename = "Oa08_radiance";
    }
    else if(bandlist.at(9))
    {
        invertthissegment[0] = invertlist.at(8);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa09_radiance.nc";
        *variablename = "Oa09_radiance";
    }
    else if(bandlist.at(10))
    {
        invertthissegment[0] = invertlist.at(9);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa10_radiance.nc";
        *variablename = "Oa10_radiance";
    }
    else if(bandlist.at(11))
    {
        invertthissegment[0] = invertlist.at(10);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa11_radiance.nc";
        *variablename = "Oa11_radiance";
    }
    else if(bandlist.at(12))
    {
        invertthissegment[0] = invertlist.at(11);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa12_radiance.nc";
        *variablename = "Oa12_radiance";
    }
    else if(bandlist.at(13))
    {
        invertthissegment[0] = invertlist.at(12);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa13_radiance.nc";
        *variablename = "Oa13_radiance";
    }
    else if(bandlist.at(14))
    {
        invertthissegment[0] = invertlist.at(13);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa14_radiance.nc";
        *variablename = "Oa14_radiance";
    }
    else if(bandlist.at(15))
    {
        invertthissegment[0] = invertlist.at(14);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa15_radiance.nc";
        *variablename = "Oa15_radiance";
    }
    else if(bandlist.at(16))
    {
        invertthissegment[0] = invertlist.at(15);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa16_radiance.nc";
        *variablename = "Oa16_radiance";
    }
    else if(bandlist.at(17))
    {
        invertthissegment[0] = invertlist.at(16);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa17_radiance.nc";
        *variablename = "Oa17_radiance";
    }
    else if(bandlist.at(18))
    {
        invertthissegment[0] = invertlist.at(17);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa18_radiance.nc";
        *variablename = "Oa18_radiance";
    }
    else if(bandlist.at(19))
    {
        invertthissegment[0] = invertlist.at(18);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa19_radiance.nc";
        *variablename = "Oa19_radiance";
    }
    else if(bandlist.at(20))
    {
        invertthissegment[0] = invertlist.at(19);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa20_radiance.nc";
        *variablename = "Oa20_radiance";
    }
    else if(bandlist.at(21))
    {
        invertthissegment[0] = invertlist.at(20);
        *datasetname = this->fileInfo.baseName() + ".SEN3/Oa21_radiance.nc";
        *variablename = "Oa21_radiance";
    }

}


void SegmentOLCIefr::RenderSegmentlineInTextureOLCIefr( int nbrLine, QRgb *row )
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

    float flon, flat, fflon, fflat;

    for (int pix = 0 ; pix < earthviews; pix+=8)
    {
        pixval[0] = ptrbaOLCI[0][nbrLine * earthviews + pix];
        valok[0] = pixval[0] < 65535;
        if(color)
        {
            pixval[1] = ptrbaOLCI[1][nbrLine * earthviews + pix];
            pixval[2] = ptrbaOLCI[2][nbrLine * earthviews + pix];
            valok[1] = pixval[1] < 65535;
            valok[2] = pixval[2] < 65535;
        }


        if( valok[0] && (color ? valok[1] && valok[2] : true))
        {
            fflon = (float)(this->longitude[nbrLine * earthviews + pix])/1000000.0;
            fflat = (float)(this->latitude[nbrLine * earthviews + pix])/1000000.0;
            flon = fflon * PI/180.0;
            flat = fflat * PI/180.0;
            sphericalToPixel( flon, flat, posx, posy, devwidth, devheight );
            rgb.setRgb(qRed(row[pix]), qGreen(row[pix]), qBlue(row[pix]));
            fb_painter.setPen(rgb);
            fb_painter.drawPoint( posx , posy );
        }
    }

    fb_painter.end();
    g_mutex.unlock();
}

int SegmentOLCIefr::UntarSegmentToTemp()
{

    int flags = ARCHIVE_EXTRACT_TIME;
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;

    QString intarfile = this->fileInfo.absoluteFilePath();

    qDebug() << "Start UntarSegmentToTemp 1 for file " + intarfile;

    QByteArray array = intarfile.toUtf8();
    const char* p = array.constData();

    a = archive_read_new();
    ext = archive_write_disk_new();
    //archive_read_support_filter_all(a);
    archive_read_support_format_tar(a);

    archive_write_disk_set_options(ext, flags);

    // r = archive_read_open_filename(a, p, 10240); // Note 1
    r = archive_read_open_filename(a, p, 20480);
    if (r != ARCHIVE_OK)
    {
        qDebug() << "file niet gevonden ....";
        return(1);
    }

//    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
//    {
//      qDebug() << QString("%1").arg(archive_entry_pathname(entry));
//      archive_read_data_skip(a);  // Note 2
//    }

    int nbrblocks = 1;

    for (;;)
    {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r != ARCHIVE_OK)
            qDebug() << "archive_read_next_header() " << QString(archive_error_string(a));
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK)
            qDebug() << "archive_write_header() " << QString(archive_error_string(ext));
        else
        {
            qDebug() << QString("Start copy_data ....%1").arg(nbrblocks);

            copy_data(a, ext);
            r = archive_write_finish_entry(ext);
            if (r != ARCHIVE_OK)
                qDebug() << "archive_write_finish_entry() " << QString(archive_error_string(ext));
            nbrblocks++;
        }
    }




    archive_read_close(a);
    archive_read_free(a);

    archive_write_close(ext);
    archive_write_free(ext);

    return(0);
}


int SegmentOLCIefr::copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif


    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r != ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK) {
            qDebug() << "archive_write_data_block() " << QString(archive_error_string(aw));
            return (r);
        }
    }
}

void SegmentOLCIefr::initializeMemory()
{
    qDebug() << "Initializing OLCI memory";

    bool color = this->bandlist.at(0);

    for(int k = 0; k < (color ? 3 : 1); k++)
    {
        if(ptrbaOLCI[k].isNull())
        {
            ptrbaOLCI[k].reset(new ushort[earth_views_per_scanline * NbrOfLines]);
            qDebug() << QString("Initializing OLCI memory earth views = %1 nbr of lines = %2").arg(earth_views_per_scanline).arg(NbrOfLines);
        }
    }
    qDebug() << "End Initializing OLCI memory";

}



void SegmentOLCIefr::ComposeSegmentImage()
{

    QRgb *row;
    int indexout[3];

    qDebug() << QString("SegmentOLCI::ComposeSegmentImage() segm->startLineNbr = %1").arg(this->startLineNbr);
    qDebug() << QString("SegmentOLCI::ComposeSegmentImage() color = %1 ").arg(bandlist.at(0));
    qDebug() << QString("SegmentOLCI::ComposeSegmentImage() invertthissegment[0] = %1").arg(invertthissegment[0]);
    qDebug() << QString("SegmentOLCI::ComposeSegmentImage() invertthissegment[1] = %1").arg(invertthissegment[1]);
    qDebug() << QString("SegmentOLCI::ComposeSegmentImage() invertthissegment[2] = %1").arg(invertthissegment[2]);

    int pixval[3];
    int r, g, b;

    bool color = bandlist.at(0);
    bool valok[3];

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        row = (QRgb*)imageptrs->ptrimageOLCIefr->scanLine(this->startLineNbr + line);
        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
        {
            pixval[0] = *(this->ptrbaOLCI[0].data() + line * earth_views_per_scanline + pixelx);
            if(color)
            {
                pixval[1] = *(this->ptrbaOLCI[1].data() + line * earth_views_per_scanline + pixelx);
                pixval[2] = *(this->ptrbaOLCI[2].data() + line * earth_views_per_scanline + pixelx);
            }

            valok[0] = pixval[0] < 65535;
            valok[1] = pixval[1] < 65535;
            valok[2] = pixval[2] < 65535;

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
                    {
                        r = imageptrs->lut_ch[0][indexout[0]];
                     //   r = indexout[0];
                    }
                    if(invertthissegment[1])
                    {
                        g = 255 - imageptrs->lut_ch[1][indexout[1]];
                    }
                    else
                    {
                        g = imageptrs->lut_ch[1][indexout[1]];
                     //   g = indexout[1];
                    }
                    if(invertthissegment[2])
                    {
                        b = 255 - imageptrs->lut_ch[2][indexout[2]];
                    }
                    else
                    {
                        b = imageptrs->lut_ch[2][indexout[2]];
                        //b = indexout[2];
                    }

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

        if(opts.imageontextureOnOLCIefr && line % 2 == 0)
        {
            this->RenderSegmentlineInTextureOLCIefr( line, row );
            opts.texture_changed = true;
        }

    }
}

void SegmentOLCIefr::ComposeSegmentGVProjection(int inputchannel)
{
    ComposeProjection(GVP);
}

void SegmentOLCIefr::ComposeProjection(eProjections proj)
{

    double map_x, map_y;

    float lonpos1, latpos1;

    //g_mutex.lock();

    int pixval[3];

    bool color = bandlist.at(0);
    bool valok[3];

    projectionCoordX.reset(new qint32[NbrOfLines * earth_views_per_scanline]);
    projectionCoordY.reset(new qint32[NbrOfLines * earth_views_per_scanline]);
    projectionCoordValue.reset(new QRgb[NbrOfLines * earth_views_per_scanline]);

    for( int i = 0; i < NbrOfLines; i++)
    {
        for( int j = 0; j < earth_views_per_scanline; j++ )
        {
            projectionCoordX[i * earth_views_per_scanline + j] = 65535;
            projectionCoordY[i * earth_views_per_scanline + j] = 65535;
            projectionCoordValue[i * earth_views_per_scanline + j] = qRgba(0, 0, 0, 0);
        }
    }
    qDebug() << "SegmentOLCIefr::ComposeProjection(eProjections proj)";

    for( int i = 0; i < this->NbrOfLines; i++)
    {
        for( int j = 0; j < this->earth_views_per_scanline ; j++ )
        {
            pixval[0] = ptrbaOLCI[0][i * earth_views_per_scanline + j];
            valok[0] = pixval[0] > 0 && pixval[0] < 65535;

            if(color)
            {
                pixval[1] = ptrbaOLCI[1][i * earth_views_per_scanline + j];
                pixval[2] = ptrbaOLCI[2][i * earth_views_per_scanline + j];
                valok[1] = pixval[1] > 0 && pixval[1] < 65535;
                valok[2] = pixval[2] > 0 && pixval[2] < 65535;
            }

            if( valok[0] && (color ? valok[1] && valok[2] : true))
            {
                latpos1 = (float)latitude[i * earth_views_per_scanline + j]/1000000.0;
                lonpos1 = (float)longitude[i * earth_views_per_scanline + j]/1000000.0;

                if(proj == LCC) // Lambert
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
                projectionCoordX[i * earth_views_per_scanline + j] = 65535;
                projectionCoordY[i * earth_views_per_scanline + j] = 65535;
                projectionCoordValue[i * earth_views_per_scanline + j] = qRgba(0, 0, 0, 0);
            }
        }
    }

}

void SegmentOLCIefr::MapPixel( int lines, int views, double map_x, double map_y, bool color)
{
    int indexout[3];
    int pixval[3];
    int r, g, b;
    QRgb rgbvalue = qRgb(0,0,0);

    pixval[0] = ptrbaOLCI[0][lines * earth_views_per_scanline + views];
    if(color)
    {
        pixval[1] = ptrbaOLCI[1][lines * earth_views_per_scanline + views];
        pixval[2] = ptrbaOLCI[2][lines * earth_views_per_scanline + views];
    }

    if (map_x > -15 && map_x < imageptrs->ptrimageProjection->width() + 15 && map_y > -15 && map_y < imageptrs->ptrimageProjection->height() + 15)
    {

        projectionCoordX[lines * earth_views_per_scanline + views] = (qint32)map_x;
        projectionCoordY[lines * earth_views_per_scanline + views] = (qint32)map_y;

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
                projectionCoordValue[lines * earth_views_per_scanline + views] = rgbvalue;

            }
        }
        else
        {
            if (map_x >= 0 && map_x < imageptrs->ptrimageProjection->width() && map_y >= 0 && map_y < imageptrs->ptrimageProjection->height())
                imageptrs->ptrimageProjection->setPixel((int)map_x, (int)map_y, rgbvalue);
            projectionCoordValue[lines * earth_views_per_scanline + views] = rgbvalue;
        }
    }
}

SegmentOLCIefr::~SegmentOLCIefr()
{
    resetMemory();
}
