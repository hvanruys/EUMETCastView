/* ------------------------------------------------------------------
   This file is part of EUMETCastView.

   Copyright (C) 2018 Hugo Van Ruyskensvelde <hvanruys@tvvlaanderen.net>

   This program is released under the terms of the license contained
   in the file LICENSE.

   Some parts of the code (for the calculations of radiances and brightness temperatures)
   are taken from 'seviri_util'. (https://github.com/gmcgarragh/seviri_util)
   (Written by Greg McGarragh and Simon Proud)

   ------------------------------------------------------------------ */


#include <QApplication>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <iomanip>

#include "segmentlistgeostationary.h"
#include "qcompressor.h"
#include "pixgeoconversion.h"
#include "misc_util.h"
#include "internal.h"

#ifdef _WIN32
#include <hdf5.h>
#else
#include <hdf5.h>
#endif
#include <netcdf.h>

#include "MSG_HRIT.h"
#include <QMutex>

#define ERR(e) {printf("Error: %s\n", nc_strerror(e));}

#define BYTE_SWAP4(x) \
    (((x & 0xFF000000) >> 24) | \
    ((x & 0x00FF0000) >> 8)  | \
    ((x & 0x0000FF00) << 8)  | \
    ((x & 0x000000FF) << 24))

#define BYTE_SWAP2(x) \
    (((x & 0xFF00) >> 8) | \
    ((x & 0x00FF) << 8))

quint16 _htons(quint16 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP2(x);
    }
}

quint16 _ntohs(quint16 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP2(x);
    }
}

quint32 _htonl(quint32 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP4(x);
    }
}

quint32 _ntohl(quint32 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP4(x);
    }
}

extern Options opts;
extern SegmentImage *imageptrs;
extern gshhsData *gshhsdata;

// Meteosat
// Height = 3712 / 8 = 464 Width = 3712
// Height = 11136 / 24 = 464 Width = 7502
// Electro_n1
// Height = 2784 / 6 = 464 Width = 2784

void SegmentListGeostationary::doComposeGeostationaryHDFInThread(SegmentListGeostationary *sm, QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    sm->ComposeSegmentImageHDFInThread(filelist, spectrumvector, inversevector);
}

void SegmentListGeostationary::doComposeGeostationarynetCDFInThread(SegmentListGeostationary *sm) //, QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod)
{
    sm->ComposeSegmentImagenetCDFInThread(); //strlist, spectrumvector, inversevector, histogrammethod);
}

void SegmentListGeostationary::doComposeGeostationarynetCDFMTGInThread(SegmentListGeostationary *sm) //, QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod)
{
    sm->ComposeSegmentImagenetCDFMTGInThread1();  //Concurrent();
}

void SegmentListGeostationary::doComposeGeostationaryXRITMSGInThread(SegmentListGeostationary *sm) //, QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod)
{
    sm->ComposeSegmentImageXRITMSGInThreadConcurrent();
}

void SegmentListGeostationary::doComposeGeoRGBRecipe(SegmentListGeostationary *sm, int recipe)
{
    sm->ComposeGeoRGBRecipeInThread(recipe);
}

SegmentListGeostationary::SegmentListGeostationary(QObject *parent, int geoindex) :
    QObject(parent)
{

    qDebug() << QString("in constructor SegmentListGeostationary");

    this->geoindex = geoindex;

    for(int i = 0; i < 3; i++)
    {
        this->stat_min[i] = 0;
        this->stat_max[i] = 0;

    }
    kindofimage = "VIS_IR";

    COFF = opts.geosatellites.at(geoindex).coff;
    LOFF = opts.geosatellites.at(geoindex).loff;
    CFAC = opts.geosatellites.at(geoindex).cfac;
    LFAC = opts.geosatellites.at(geoindex).lfac;


    areatype = 0;
    this->bActiveSegmentList = false;
    this->bisRSS = false;
    this->SetupContrastStretch( 0, 0, 1023, 255);

    nbr_lines_MTG << 278 << 278 << 279 << 278 << 279 << 278 << 278 << 279 << 278 << 279;
    nbr_lines_MTG << 278 << 278 << 279 << 278 << 279 << 278 << 278 << 279 << 278 << 279;
    nbr_lines_MTG << 278 << 278 << 279 << 278 << 279 << 298 << 298 << 300 << 277 << 258;
    nbr_lines_MTG << 258 << 259 << 279 << 278 << 279 << 278 << 278 << 279 << 278 << 279;

}

eGeoSatellite SegmentListGeostationary::getGeoSatellite()
{
    return m_GeoSatellite;
}

QString SegmentListGeostationary::getSeviribandfromChannel(int channel)
{
    //VIS006 1
    //VIS008 2
    //IR_016 3
    //IR_039 4
    //WV_062 5
    //WV_073 6
    //IR_087 7
    //IR_097 8
    //IR_108 9
    //IR_120 10
    //IR_134 11


    if(channel == 1)
        return "VIS006";
    else if(channel == 2)
        return "VIS008";
    else if(channel == 3)
        return "IR_016";
    else if(channel == 4)
        return "IR_039";
    else if(channel == 5)
        return "WV_062";
    else if(channel == 6)
        return "WV_073";
    else if(channel == 7)
        return "IR_087";
    else if(channel == 8)
        return "IR_097";
    else if(channel == 9)
        return "IR_108";
    else if(channel == 10)
        return "IR_120";
    else if(channel == 11)
        return "IR_134";
    else
        return "unkwnown";
}

void SegmentListGeostationary::setGeoSatellite(int geoindex)
{
    this->geoindex = geoindex;
    str_GeoSatellite = opts.geosatellites.at(geoindex).shortname;

    if(str_GeoSatellite == "MET_11")
    {
        this->m_GeoSatellite = eGeoSatellite::MET_11;
    }
    else if(str_GeoSatellite == "MET_10")
    {
        this->m_GeoSatellite = eGeoSatellite::MET_10;
    }
    else if(str_GeoSatellite == "MET_9")
    {
        this->m_GeoSatellite = eGeoSatellite::MET_9;
    }
    else if(str_GeoSatellite == "MET_8")
    {
        this->m_GeoSatellite = eGeoSatellite::MET_8;
    }
    else if(str_GeoSatellite == "GOMS3")
    {
        this->m_GeoSatellite = eGeoSatellite::GOMS3;
    }
    else if(str_GeoSatellite == "FY2H")
    {
        this->m_GeoSatellite = eGeoSatellite::FY2H;
    }
    else if(str_GeoSatellite == "FY2G")
    {
        this->m_GeoSatellite = eGeoSatellite::FY2G;
    }
    else if(str_GeoSatellite == "GOES_18")
    {
        this->m_GeoSatellite = eGeoSatellite::GOES_18;
    }
    else if(str_GeoSatellite == "GOES_16")
    {
        this->m_GeoSatellite = eGeoSatellite::GOES_16;
    }
    else if(str_GeoSatellite == "GOES_17")
    {
        this->m_GeoSatellite = eGeoSatellite::GOES_17;
    }
    else if(str_GeoSatellite == "H9")
    {
        this->m_GeoSatellite = eGeoSatellite::H9;
    }
    else if(str_GeoSatellite == "MTG-I1")
    {
        this->m_GeoSatellite = eGeoSatellite::MTG_I1;
    }
}

void SegmentListGeostationary::getFilenameParameters(QFileInfo fileinfo, QString *filespectrum, QString *filedate, int *filesequence, int *channelindex)
{

    //qDebug() << "SegmentListGeostationary::getFilenameParameters at geoindex = " << geoindex;

    int index = opts.geosatellites.at(geoindex).indexspectrumhrv;
    int length = opts.geosatellites.at(geoindex).spectrumhrv.length();
    QString spectrum = fileinfo.fileName().mid(index, length);
    if(spectrum.length() > 0 && spectrum == opts.geosatellites.at(geoindex).spectrumhrv)
    {
        *filespectrum = spectrum;
        *filedate = fileinfo.fileName().mid(opts.geosatellites.at(geoindex).indexdatehrv, opts.geosatellites.at(geoindex).lengthdatehrv);
        *filesequence = fileinfo.fileName().mid(opts.geosatellites.at(geoindex).indexfilenbrhrv, opts.geosatellites.at(geoindex).lengthfilenbrhrv).toInt()-1;
    }
    else
    {
        for(int i = 0; i < opts.geosatellites.at(geoindex).spectrumlist.count(); i++)
        {
            spectrum = fileinfo.fileName().mid(opts.geosatellites.at(geoindex).indexspectrum, opts.geosatellites.at(geoindex).spectrumlist.at(i).length());
            if(spectrum.length() > 0 && spectrum == opts.geosatellites.at(geoindex).spectrumlist.at(i))
            {
                *filespectrum = spectrum;
                *filedate = fileinfo.fileName().mid(opts.geosatellites.at(geoindex).indexdate, opts.geosatellites.at(geoindex).lengthdate);
                *filesequence = fileinfo.fileName().mid(opts.geosatellites.at(geoindex).indexfilenbr, opts.geosatellites.at(geoindex).lengthfilenbr).toInt()-1;
                break;
            }
        }
    }

    if( *filespectrum  == "HRV")
    {
        *channelindex = 0;
    }
    else if( spectrumvector.at(1) == "" && spectrumvector.at(2) == "")
    {
        *channelindex = 0;
    }
    else
    {
        if(spectrumvector.at(0) == *filespectrum)
        {
            *channelindex = 0;
        }
        else if(spectrumvector.at(1) == *filespectrum)
        {
            *channelindex = 1;
        }
        else if(spectrumvector.at(2) == *filespectrum)
        {
            *channelindex = 2;
        }
    }


}

void SegmentListGeostationary::getFilenameParameters(QString file, QString *filespectrum, QString *filedate, int *filesequence, int *channelindex)
{


    int index = opts.geosatellites.at(geoindex).indexspectrumhrv;
    int length = opts.geosatellites.at(geoindex).spectrumhrv.length();
    QString spectrum = file.mid(index, length);
    if(spectrum.length() > 0 && spectrum == opts.geosatellites.at(geoindex).spectrumhrv)
    {
        *filespectrum = spectrum;
        *filedate = file.mid(opts.geosatellites.at(geoindex).indexdatehrv, opts.geosatellites.at(geoindex).lengthdatehrv);
        *filesequence = file.mid(opts.geosatellites.at(geoindex).indexfilenbrhrv, opts.geosatellites.at(geoindex).lengthfilenbrhrv).toInt()-1;
    }
    else
    {
        for(int i = 0; i < opts.geosatellites.at(geoindex).spectrumlist.count(); i++)
        {
            spectrum = file.mid(opts.geosatellites.at(geoindex).indexspectrum, opts.geosatellites.at(geoindex).spectrumlist.at(i).length());
            if(spectrum.length() > 0 && spectrum == opts.geosatellites.at(geoindex).spectrumlist.at(i))
            {
                *filespectrum = spectrum;
                *filedate = file.mid(opts.geosatellites.at(geoindex).indexdate, opts.geosatellites.at(geoindex).lengthdate);
                *filesequence = file.mid(opts.geosatellites.at(geoindex).indexfilenbr, opts.geosatellites.at(geoindex).lengthfilenbr).toInt()-1;
                break;
            }
        }
    }

    if( *filespectrum  == "HRV")
    {
        *channelindex = 0;
    }
    else if( spectrumvector.at(1) == "" && spectrumvector.at(2) == "")
    {
        *channelindex = 0;
    }
    else
    {
        if(spectrumvector.at(0) == *filespectrum)
        {
            *channelindex = 0;
        }
        else if(spectrumvector.at(1) == *filespectrum)
        {
            *channelindex = 1;
        }
        else if(spectrumvector.at(2) == *filespectrum)
        {
            *channelindex = 2;
        }
    }

    //qDebug() << "SegmentListGeostationary::getFilenameParameters at geoindex = " << geoindex << " channelindex = " << *channelindex;

}


bool SegmentListGeostationary::ComposeImageHDFInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector)
{

    //"Z_SATE_C_BABJ_20150809101500_O_FY2E_FDI_IR1_001_NOM.HDF.gz"
    qDebug() << QString("SegmentListGeostationary::ComposeImageHDFInThread spectrumvector = %1 %2 %3").arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));

    QApplication::setOverrideCursor(( Qt::WaitCursor));
    QtConcurrent::run(doComposeGeostationaryHDFInThread, this, strlist, spectrumvector, inversevector);
    return true;
}

void SegmentListGeostationary::setThreadParametersnetCDF(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor)
{
    this->segmentfilelist = strlist;
    this->segmentfilelisthrv = QStringList();
    this->spectrumvector = spectrumvector;
    this->inversevector = inversevector;
    this->histogrammethod = histogrammethod;
    this->pseudocolor = pseudocolor;

}

void SegmentListGeostationary::setThreadParametersXRIT(QStringList strlistvis_ir, QStringList strlisthvr, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod)
{
    this->segmentfilelist = strlistvis_ir;
    this->segmentfilelisthrv = strlisthvr;
    this->spectrumvector = spectrumvector;
    this->inversevector = inversevector;
    this->histogrammethod = histogrammethod;
    this->pseudocolor = false;

}

bool SegmentListGeostationary::ComposeImagenetCDFInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor)
{

    qDebug() << QString("SegmentListGeostationary::ComposeImagenetCDFInThread spectrumvector = %2 %3 %4").arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));

    setThreadParametersnetCDF(strlist, spectrumvector, inversevector, histogrammethod, pseudocolor);
    QtConcurrent::run(doComposeGeostationarynetCDFInThread, this); //, strlist, spectrumvector, inversevector, histogrammethod);
    return true;
}

bool SegmentListGeostationary::ComposeImagenetCDFMTGInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor)
{
    qDebug() << QString("SegmentListGeostationary::ComposeImagenetCDFMTGInThread spectrumvector = %2 %3 %4").arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));

    //    for(int i = 0; i < strlist.length(); i++)
    //        qDebug() << i << " " << strlist.at(i);

    QApplication::setOverrideCursor(( Qt::WaitCursor));
    //setThreadParametersnetCDF(strlist, spectrumvector, inversevector, histogrammethod, pseudocolor);
    QtConcurrent::run(doComposeGeostationarynetCDFMTGInThread, this);
    return true;
}

bool SegmentListGeostationary::ComposeImageXRITMSGInThread(QStringList strlistVIS_IR, QStringList strlistHVR, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod)
{
    qDebug() << QString("SegmentListGeostationary::ComposeImageXRITMSGInThread spectrumvector = %2 %3 %4").arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));

    setThreadParametersXRIT(strlistVIS_IR, strlistHVR, spectrumvector, inversevector, histogrammethod);
    QtConcurrent::run(doComposeGeostationaryXRITMSGInThread, this);
    return true;
}

/*
bool SegmentListGeostationary::ComposeImageHDFSerial(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector)
{

    int filesequence = 1;
    QString filespectrum = fileinfo.fileName().mid(40, 3);
    QString filedate = fileinfo.fileName().mid(14, 12);

    qDebug() << QString("SegmentListGeostationary::ComposeImageHDFSerial filePath = %1").arg(fileinfo.filePath());
    qDebug() << QString("SegmentListGeostationary::ComposeImageHDFSerial spectrumvector = %1 %2 %3").arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));
    qDebug() << QString("SegmentListGeostationary::ComposeImageHDFSerial kindofimage = %1").arg(kindofimage);

    if(kindofimage == "VIS_IR" || kindofimage == "VIS_IR Color")
    {

        if( spectrumvector.at(1) == "" && spectrumvector.at(2) == "")
        {
            this->ComposeSegmentImageHDF(fileinfo.filePath(), 0, spectrumvector, inversevector);
        }
        else
        {
            if(spectrumvector.at(0) == filespectrum)
            {
                this->ComposeSegmentImageHDF(fileinfo.filePath(), 0, spectrumvector, inversevector);
            }
            else if(spectrumvector.at(1) == filespectrum)
            {
                this->ComposeSegmentImageHDF(fileinfo.filePath(), 1, spectrumvector, inversevector);
            }
            else if(spectrumvector.at(2) == filespectrum)
            {
                this->ComposeSegmentImageHDF(fileinfo.filePath(), 2, spectrumvector, inversevector);
            }
        }
    } else if(kindofimage == "HRV")
    {
        this->ComposeSegmentImageHDF(fileinfo.filePath(), 0, spectrumvector, inversevector);
    }

    return true;
}
*/




//void SegmentListGeostationary::ComposeSegmentImageXRITHimawari( QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector )
//{
//    //IMG_DK01B04_201510090000_001.bz2
//    //012345678901234567890123456789

//    MSG_header *header;
//    MSG_data *msgdat;
//    int     nBuf;
//    char    buf[ 32768 ];
//    BZFILE* bzfile;
//    int     bzerror;

//    qDebug() << QString("-------> SegmentListGeostationary::ComposeSegmentImageHimawari() %1").arg(filepath);

//    header = new MSG_header();
//    msgdat = new MSG_data();


//    QFile filein(filepath);
//    QFileInfo fileinfo(filein);
//    QString basename = fileinfo.baseName();


//    int filesequence = fileinfo.fileName().mid(25, 3).toInt()-1;
//    QString filespectrum = fileinfo.fileName().mid(8, 3);
//    QString filedate = fileinfo.fileName().mid(12, 11) + "0";

//    QFile fileout(basename);
//    fileout.open(QIODevice::WriteOnly);
//    QDataStream streamout(&fileout);


//    if((bzfile = BZ2_bzopen(fileinfo.absoluteFilePath().toLatin1(),"rb"))==NULL)
//    {
//        qDebug() << "error in BZ2_bzopen";
//    }

//    bzerror = BZ_OK;
//    while ( bzerror == BZ_OK )
//    {
//        nBuf = BZ2_bzRead ( &bzerror, bzfile, buf, 32768 );
//        if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
//        {
//            streamout.writeRawData(buf, nBuf);
//        }
//    }

//    BZ2_bzclose ( bzfile );

//    fileout.close();

//    //QByteArray ba = basename.toLatin1();
//    //const char *c_segname = ba.data();

//    std::ifstream hrit(basename.toStdString(), (std::ios::binary | std::ios::in) );
//    if (hrit.fail())
//    {
//        std::cerr << "Cannot open input Himawari file "
//                  << filepath.toStdString() << std::endl;
//        return;
//    }

//    header->read_from(hrit);
//    msgdat->read_from_himawari(hrit, *header);
//    hrit.close();

//    if (header->segment_id->data_field_format == MSG_NO_FORMAT)
//    {
//        qDebug() << "Product dumped in binary format.";
//        return;
//    }

//    int planned_end_segment = 10;

//    int npix = number_of_columns = header->image_structure->number_of_columns;
//    int nlin = number_of_lines = header->image_structure->number_of_lines;
//    size_t npixperseg = number_of_columns*number_of_lines;

//    qDebug() << QString("---->[%1] SegmentListGeostationary::ComposeSegmentImageXRITHimawari() planned end = %2 npix = %3 nlin = %4 filesequence = %5 channelindex = %6").arg(kindofimage).arg(planned_end_segment).arg(number_of_columns).arg(number_of_lines).arg(filesequence).arg(channelindex);

//    MSG_SAMPLE *pixels = new MSG_SAMPLE[npixperseg];
//    memset(pixels, 0, npixperseg*sizeof(MSG_SAMPLE));
//    memcpy(pixels, msgdat->image->data, npixperseg*sizeof(MSG_SAMPLE));

//    for(int i = 0; i < npixperseg; i++)
//        pixels[i] =  BYTE_SWAP2(pixels[i]);

//    QImage *im;
//    im = imageptrs->ptrimageGeostationary;

//    quint16 c;

//    if(channelindex == 0)
//    {
//        imageptrs->ptrRed[filesequence] = new quint16[number_of_lines * number_of_columns];
//        memset(imageptrs->ptrRed[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
//    }
//    else if(channelindex == 1)
//    {
//        imageptrs->ptrGreen[filesequence] = new quint16[number_of_lines * number_of_columns];
//        memset(imageptrs->ptrGreen[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
//    }
//    else if(channelindex == 2)
//    {
//        imageptrs->ptrBlue[filesequence] = new quint16[number_of_lines * number_of_columns];
//        memset(imageptrs->ptrBlue[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
//    }

//    quint16 maxpic = 0;
//    quint16 minpic = 65535;

//    for(int line = 0; line < nlin; line++)
//    {
//        for (int pixelx = 0 ; pixelx < npix; pixelx++)
//        {
//            c = *(pixels + line * npix + pixelx);
//            if(maxpic < c)
//                maxpic = c;
//            if(minpic > c)
//                minpic = c;
//            if(channelindex == 0)
//                *(imageptrs->ptrRed[filesequence] + line * npix + pixelx) = c;
//            else if(channelindex == 1)
//                *(imageptrs->ptrGreen[filesequence] + line * npix + pixelx) = c;
//            else if(channelindex == 2)
//                *(imageptrs->ptrBlue[filesequence] + line * npix + pixelx) = c;

//        }
//    }


//    if(allVIS_IRSegmentsReceived())
//    {
//        ComposeVISIRHimawari();
//        emit signalcomposefinished(kindofimage, geoindex);
//    }

//    delete header;
//    delete msgdat;
//    delete [] pixels;


//}

/*
void SegmentListGeostationary::recalcHimawari()
{
    QRgb *row_col;
    int r, g, b;
    QImage *im;
    im = imageptrs->ptrimageGeostationary;

    quint32 totminred = 0;
    quint32 totmaxred = 0;
    quint32 totmingreen = 0;
    quint32 totmaxgreen = 0;
    quint32 totminblue = 0;
    quint32 totmaxblue = 0;

    for(int i = 0; i < 10; i++)
    {
        totminred += minvalueRed[i];
        totmaxred += maxvalueRed[i];
        totmingreen += minvalueGreen[i];
        totmaxgreen += maxvalueGreen[i];
        totminblue += minvalueBlue[i];
        totmaxblue += maxvalueBlue[i];
        qDebug() << QString("minred = %1 maxred = %2 i = %3").arg(minvalueRed[i]).arg(maxvalueRed[i]).arg(i);
        qDebug() << QString("mingreen = %1 maxgreen = %2 i = %3").arg(minvalueGreen[i]).arg(maxvalueGreen[i]).arg(i);
        qDebug() << QString("minblue = %1 maxblue = %2  i = %3").arg(minvalueBlue[i]).arg(maxvalueBlue[i]).arg(i);

    }

    totminred /= 10;
    totmaxred /= 10;
    totmingreen /= 10;
    totmaxgreen /= 10;
    totminblue /= 10;
    totmaxblue /= 10;

    qDebug() << QString("totminred = %1 totmaxred = %2").arg(totminred).arg(totmaxred);
    qDebug() << QString("totmingreen = %1 totmaxgreen = %2").arg(totmingreen).arg(totmaxgreen);
    qDebug() << QString("totminblue = %1 totmaxblue = %2").arg(totminblue).arg(totmaxblue);

    int nlin = 550;
    int npix = 5500;
    quint16 c;

    quint32 valcontrastred;
    quint32 valcontrastgreen;
    quint32 valcontrastblue;

    for(int filesequence = 0; filesequence < 10; filesequence++)
    {
        for(int line = 0; line < nlin; line++)
        {
            row_col = (QRgb*)im->scanLine(nlin * filesequence + line);
            for (int pixelx = 0 ; pixelx < npix; pixelx++)
            {

                if(kindofimage == "VIS_IR Color")
                {
                    this->SetupContrastStretch( totminred, 0, totmaxred, 255);
                    c = *(imageptrs->ptrRed[filesequence] + line * npix + pixelx);
                    valcontrastred = ContrastStretch(c);

                    this->SetupContrastStretch( totmingreen, 0, totmaxgreen, 255);
                    c = *(imageptrs->ptrGreen[filesequence] + line * npix + pixelx);
                    valcontrastgreen = ContrastStretch(c);

                    this->SetupContrastStretch( totminblue, 0, totmaxblue, 255);
                    c = *(imageptrs->ptrBlue[filesequence] + line * npix + pixelx);
                    valcontrastblue = ContrastStretch(c);

                    r = quint8(valcontrastred);
                    g = quint8(valcontrastgreen);
                    b = quint8(valcontrastblue);
                    row_col[pixelx] = qRgb(r,g,b);
                }
                else if( kindofimage == "VIS_IR")
                {
                    this->SetupContrastStretch( totminred, 0, totmaxred, 255);
                    c = *(imageptrs->ptrRed[filesequence] + line * npix + pixelx);
                    valcontrastred = ContrastStretch(c);

                    r = quint8(valcontrastred);
                    g = quint8(valcontrastred);
                    b = quint8(valcontrastred);
                    row_col[pixelx] = qRgb(r,g,b);
                }
            }
        }
    }


}
*/
/*
void SegmentListGeostationary::ComposeSegmentImageHDF( QFileInfo fileinfo, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector )
{

    double gamma = opts.meteosatgamma;
    quint16 valgamma;
    quint8 valcontrast;
    double gammafactor = 1023 / pow(1023, gamma);
    QRgb *row_col;
    quint16 c;
    QRgb pix;
    int r,g, b;

    QImage *im;
    QString outfilename;

    im = imageptrs->ptrimageGeostationary;

    {
        QFile inFile(fileinfo.filePath());
        inFile.open(QIODevice::ReadOnly);
        QByteArray compressed = inFile.readAll();

        QByteArray decompressed;
        outfilename = fileinfo.fileName().mid(0, fileinfo.fileName().length()-3);

        QFile outfile(outfilename);
        if(QCompressor::gzipDecompress(compressed, decompressed))
        {
            outfile.open(QIODevice::WriteOnly);
            QDataStream out(&outfile);
            out.writeRawData(decompressed.constData(), decompressed.length());
        }
        else
            qDebug() << "-----> gzipDecompress failed !";
    }

    qDebug() << "fileinfo.filepath = " << fileinfo.filePath();
    qDebug() << "outfilename = " << outfilename;

    QString DatasetName;


    if(kindofimage == "VIS_IR")
    {
        imageptrs->ptrRed[0] = new quint16[2288 * 2288];
        memset(imageptrs->ptrRed[0], 0, 2288 * 2288 * sizeof(quint16));
        DatasetName = "/NOMChannel" + spectrumvector.at(0);
    }
    else if(kindofimage == "VIS_IR Color")
    {
        if(channelindex == 0)
        {
            imageptrs->ptrRed[0] = new quint16[2288 * 2288];
            memset(imageptrs->ptrRed[0], 0, 2288 * 2288 * sizeof(quint16));
            DatasetName = "/NOMChannel" + spectrumvector.at(0);
        }
        else if(channelindex == 1)
        {
            imageptrs->ptrGreen[0] = new quint16[2288 * 2288];
            memset(imageptrs->ptrGreen[0], 0, 2288 * 2288 * sizeof(quint16));
            DatasetName = "/NOMChannel" + spectrumvector.at(1);
        }
        else if(channelindex == 2)
        {
            imageptrs->ptrBlue[0] = new quint16[2288 * 2288];
            memset(imageptrs->ptrBlue[0], 0, 2288 * 2288 * sizeof(quint16));
            DatasetName = "/NOMChannel" + spectrumvector.at(2);
        }
    } else if(kindofimage == "HRV")
    {
        imageptrs->ptrRed[0] = new quint16[9152 * 9152];
        memset(imageptrs->ptrRed[0], 0, 9152 * 9152 * sizeof(quint16));
        DatasetName = "/NOMChannelVIS1KM";
    }


    qDebug() << "channelindex = " << channelindex  << " datasetname = " << DatasetName;

    hid_t   h5_file_id, nomfileinfo_id, nomchannel_id;
    herr_t  h5_status;

    if( (h5_file_id = H5Fopen(outfilename.toLatin1(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
        qDebug() << "File " << outfilename << " not open !!";


    if( (nomfileinfo_id = H5Dopen2(h5_file_id, "/NomFileInfo", H5P_DEFAULT)) < 0)
        qDebug() << "/NomFileInfo does not exist";
    else
        qDebug() << "Dataset '/NomFileInfo' is open";

    if( (nomchannel_id = H5Dopen2(h5_file_id, DatasetName.toLatin1(), H5P_DEFAULT)) < 0)
        qDebug() << DatasetName + " does not exist";
    else
        qDebug() << "Dataset '" << DatasetName << "' is open";

    if(kindofimage == "VIS_IR")
    {
        if((h5_status = H5Dread (nomchannel_id, H5T_NATIVE_INT16_g, H5S_ALL, H5S_ALL,
                                 H5P_DEFAULT, imageptrs->ptrRed[0])) < 0)
            qDebug() << "Unable to read NOMChannel" << spectrumvector.at(0) << " dataset";
    }
    else if(kindofimage == "VIS_IR Color")
    {
        if(channelindex == 0)
        {
            if((h5_status = H5Dread (nomchannel_id, H5T_NATIVE_INT16_g, H5S_ALL, H5S_ALL,
                                     H5P_DEFAULT, imageptrs->ptrRed[0])) < 0)
                qDebug() << "Unable to read NOMChannel" << spectrumvector.at(0) << " dataset";
        } else if(channelindex == 1)
        {
            if((h5_status = H5Dread (nomchannel_id, H5T_NATIVE_INT16_g, H5S_ALL, H5S_ALL,
                                     H5P_DEFAULT, imageptrs->ptrGreen[0])) < 0)
                qDebug() << "Unable to read NOMChannel" << spectrumvector.at(1) << " dataset";
        } else if(channelindex == 2)
        {
            if((h5_status = H5Dread (nomchannel_id, H5T_NATIVE_INT16_g, H5S_ALL, H5S_ALL,
                                     H5P_DEFAULT, imageptrs->ptrBlue[0])) < 0)
                qDebug() << "Unable to read NOMChannel" << spectrumvector.at(2) << " dataset";
        }
    } else if(kindofimage == "HRV")
    {
        if((h5_status = H5Dread (nomchannel_id, H5T_NATIVE_INT16_g, H5S_ALL, H5S_ALL,
                                 H5P_DEFAULT, imageptrs->ptrRed[0])) < 0)
            qDebug() << "Unable to read NOMChannel" << spectrumvector.at(0) << " dataset";
    }


    quint16 stat_min;
    quint16 stat_max;

    if(kindofimage == "VIS_IR")
        CalculateMinMax(2288, 2288, imageptrs->ptrRed[0], stat_min, stat_max);
    else if(kindofimage == "VIS_IR Color")
    {
        if(channelindex == 0)
            CalculateMinMax(2288, 2288, imageptrs->ptrRed[0], stat_min, stat_max);
        else if(channelindex == 1)
            CalculateMinMax(2288, 2288, imageptrs->ptrGreen[0], stat_min, stat_max);
        else if(channelindex == 2)
            CalculateMinMax(2288, 2288, imageptrs->ptrBlue[0], stat_min, stat_max);

    } else if(kindofimage == "HRV")
    {
        CalculateMinMax(9152, 9152, imageptrs->ptrRed[0], stat_min, stat_max);
    }

    qDebug() << QString("stat min = %1 stat max = %2").arg(stat_min).arg(stat_max);
    this->SetupContrastStretch( stat_min, 0, stat_max+1, 255);

    g_mutex.lock();

    int nlin, npix;

    if(kindofimage == "VIS_IR" || kindofimage == "VIS_IR Color")
    {
        nlin = 2288;
        npix = 2288;
    } else if(kindofimage == "HRV")
    {
        nlin = 9152;
        npix = 9152;
    }

    for(int line = 0; line < nlin; line++)
    {

        row_col = (QRgb*)im->scanLine(line);

        for (int pixelx = 0 ; pixelx < npix; pixelx++)
        {
            if(kindofimage == "VIS_IR Color")
            {
                if(channelindex == 0)
                    c = *(imageptrs->ptrRed[0] + line * npix + pixelx);
                else if(channelindex == 1)
                    c = *(imageptrs->ptrGreen[0] + line * npix + pixelx);
                else if(channelindex == 2)
                    c = *(imageptrs->ptrBlue[0] + line * npix + pixelx);

            }
            else if(kindofimage == "VIS_IR")
                c = *(imageptrs->ptrRed[0] + line * npix + pixelx);
            else if(kindofimage == "HRV")
                c = *(imageptrs->ptrRed[0] + line * npix + pixelx);

            if( c >= 65528 || c == 0 )
                valcontrast = 0;
            else
                valcontrast = ContrastStretch(c);


            if(kindofimage == "VIS_IR Color")
            {
                pix = row_col[pixelx];

                if(channelindex == 2)
                {
                    if(inversevector[2])
                        valcontrast = 255 - valcontrast;
                    r = qRed(pix);
                    g = qGreen(pix);
                    b = quint8(valcontrast);
                }
                else if(channelindex == 1)
                {
                    if(inversevector[1])
                        valcontrast = 255 - valcontrast;
                    r = qRed(pix);
                    g = quint8(valcontrast);
                    b = qBlue(pix);
                }
                else if(channelindex == 0)
                {
                    if(inversevector[0])
                        valcontrast = 255 - valcontrast;
                    r = quint8(valcontrast);
                    g = qGreen(pix);
                    b = qBlue(pix);
                }
                row_col[pixelx] = qRgb(r,g,b);

            }
            else if( kindofimage == "VIS_IR" || kindofimage == "HRV")
            {
                if(inversevector[0])
                    valcontrast = 255 - valcontrast;

                r = quint8(valcontrast);
                g = quint8(valcontrast);
                b = quint8(valcontrast);
                row_col[pixelx] = qRgb(r,g,b);

            }
        }
    }


    if(m_GeoSatellite == FY2E || m_GeoSatellite == FY2G)
    {
        this->issegmentcomposedMono[0] = true;
    }

    g_mutex.unlock();

    H5Dclose(nomfileinfo_id);
    H5Dclose(nomchannel_id);
    H5Fclose(h5_file_id);

    emit imagefinished();

}
*/

void SegmentListGeostationary::ComposeSegmentImageHDFInThread(QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector )
{

    quint8 valcontrastred = 0, valcontrastgreen = 0, valcontrastblue = 0;
    QRgb *row_col;
    quint16 rc, gc, bc;

    int r,g, b;

    QImage *im;
    QStringList strlistout;
    QString inFileName;
    QString outFileName;

    im = imageptrs->ptrimageGeostationary;


    for(int j = 0; j < filelist.size(); j++)
    {
        QString teststring = filelist.at(j).mid(filelist.at(j).length()-3);
        inFileName = this->getImagePath() + "/" + filelist.at(j);
        outFileName = filelist.at(j);

        if(teststring == ".gz" || teststring == ".GZ") // compressed HDF
        {

            QFile inFile(inFileName);
            inFile.open(QIODevice::ReadOnly);
            QByteArray compressed = inFile.readAll();

            QByteArray decompressed;
            strlistout.append(filelist.at(j).mid(0, filelist.at(j).length()-3));
            QFile outFile(filelist.at(j).mid(0, filelist.at(j).length()-3));

            if(QCompressor::gzipDecompress(compressed, decompressed))
            {
                outFile.open(QIODevice::WriteOnly);
                QDataStream out(&outFile);
                out.writeRawData(decompressed.constData(), decompressed.length());
                qDebug() << "writing decompressed file " << strlistout.at(j);
            }
            else
                qDebug() << "-----> gzipDecompress failed !";
        }
        else  // already decompressed HDF
        {
            strlistout.append(filelist.at(j));

            QFile::copy(inFileName, outFileName);
        }


    }

    QStringList DatasetName;

    if(kindofimage == "VIS_IR")
    {
        imageptrs->ptrRed[0] = new quint16[2288 * 2288];
        memset(imageptrs->ptrRed[0], 0, 2288 * 2288 * sizeof(quint16));
        DatasetName.append("/NOMChannel" + filelist.at(0).mid(40, 3));
    }
    else if(kindofimage == "VIS_IR Color")
    {
        imageptrs->ptrRed[0] = new quint16[2288 * 2288];
        memset(imageptrs->ptrRed[0], 0, 2288 * 2288 * sizeof(quint16));
        DatasetName.append("/NOMChannel" + filelist.at(0).mid(40, 3));
        imageptrs->ptrGreen[0] = new quint16[2288 * 2288];
        memset(imageptrs->ptrGreen[0], 0, 2288 * 2288 * sizeof(quint16));
        DatasetName.append("/NOMChannel" + filelist.at(1).mid(40, 3));
        imageptrs->ptrBlue[0] = new quint16[2288 * 2288];
        memset(imageptrs->ptrBlue[0], 0, 2288 * 2288 * sizeof(quint16));
        DatasetName.append("/NOMChannel" + filelist.at(2).mid(40, 3));
    }
    else if(kindofimage == "HRV")
    {
        imageptrs->ptrHRV[0] = new quint16[9152 * 9152];
        memset(imageptrs->ptrHRV[0], 0, 9152 * 9152 * sizeof(quint16));
        DatasetName.append("/NOMChannelVIS1KM");
    }


    emit this->progressCounter(10);

    hid_t   h5_file_id[3], nomfileinfo_id[3], nomchannel_id[3];
    herr_t  h5_status[3];

    for(int j = 0; j < filelist.size(); j++)
    {

        if( (h5_file_id[j] = H5Fopen(strlistout.at(j).toLatin1(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
            qDebug() << "File " << strlistout.at(j) << " not open !!";


        if( (nomfileinfo_id[j] = H5Dopen2(h5_file_id[j], "/NomFileInfo", H5P_DEFAULT)) < 0)
            qDebug() << "/NomFileInfo does not exist";
        else
            qDebug() << "Dataset '/NomFileInfo' is open";

        if( (nomchannel_id[j] = H5Dopen2(h5_file_id[j], DatasetName.at(j).toLatin1(), H5P_DEFAULT)) < 0)
            qDebug() << DatasetName.at(j) + " does not exist";
        else
            qDebug() << "Dataset '" << DatasetName.at(j) << "' is open";
    }

    if(kindofimage == "VIS_IR")
    {
        if((h5_status[0] = H5Dread (nomchannel_id[0], H5T_NATIVE_INT16_g, H5S_ALL, H5S_ALL,
                                    H5P_DEFAULT, imageptrs->ptrRed[0])) < 0)
            qDebug() << "Unable to read NOMChannel" << filelist.at(0).mid(40, 3) << " dataset";
    }
    else if(kindofimage == "VIS_IR Color")
    {
        if((h5_status[0] = H5Dread (nomchannel_id[0], H5T_NATIVE_INT16_g, H5S_ALL, H5S_ALL,
                                    H5P_DEFAULT, imageptrs->ptrRed[0])) < 0)
            qDebug() << "Unable to read NOMChannel" << filelist.at(0).mid(40, 3) << " dataset";
        if((h5_status[1] = H5Dread (nomchannel_id[1], H5T_NATIVE_INT16_g, H5S_ALL, H5S_ALL,
                                    H5P_DEFAULT, imageptrs->ptrGreen[0])) < 0)
            qDebug() << "Unable to read NOMChannel" << filelist.at(1).mid(40, 3) << " dataset";
        if((h5_status[2] = H5Dread (nomchannel_id[2], H5T_NATIVE_INT16_g, H5S_ALL, H5S_ALL,
                                    H5P_DEFAULT, imageptrs->ptrBlue[0])) < 0)
            qDebug() << "Unable to read NOMChannel" << filelist.at(2).mid(40, 3) << " dataset";
    } else if(kindofimage == "HRV")
    {
        if((h5_status[0] = H5Dread (nomchannel_id[0], H5T_NATIVE_INT16_g, H5S_ALL, H5S_ALL,
                                    H5P_DEFAULT, imageptrs->ptrHRV[0])) < 0)
            qDebug() << "Unable to read NOMChannelVIS1KM dataset";
    }

    emit this->progressCounter(20);

    quint16 stat_min[3];
    quint16 stat_max[3];

    if(kindofimage == "VIS_IR")
    {
        CalculateMinMax(2288, 2288, imageptrs->ptrRed[0], stat_min[0], stat_max[0]);
        qDebug() << QString("0 stat min = %1 stat max = %2").arg(stat_min[0]).arg(stat_max[0]);
    }
    else if(kindofimage == "VIS_IR Color")
    {
        CalculateMinMax(2288, 2288, imageptrs->ptrRed[0], stat_min[0], stat_max[0]);
        CalculateMinMax(2288, 2288, imageptrs->ptrGreen[0], stat_min[1], stat_max[1]);
        CalculateMinMax(2288, 2288, imageptrs->ptrBlue[0], stat_min[2], stat_max[2]);
        qDebug() << QString("0 stat min = %1 stat max = %2").arg(stat_min[0]).arg(stat_max[0]);
        qDebug() << QString("1 stat min = %1 stat max = %2").arg(stat_min[1]).arg(stat_max[1]);
        qDebug() << QString("2 stat min = %1 stat max = %2").arg(stat_min[2]).arg(stat_max[2]);

    } else if(kindofimage == "HRV")
    {
        CalculateMinMax(9152, 9152, imageptrs->ptrHRV[0], stat_min[0], stat_max[0]);
        qDebug() << QString("0 stat min = %1 stat max = %2").arg(stat_min[0]).arg(stat_max[0]);
    }

    int nlin = 0, npix = 0;
    int deltaprogress, deltaprogresscounter = 30;

    if(kindofimage == "VIS_IR" || kindofimage == "VIS_IR Color")
    {
        nlin = 2288;
        npix = 2288;
        deltaprogress = 2281/70;
    } else if(kindofimage == "HRV")
    {
        nlin = 9152;
        npix = 9152;
        deltaprogress = 9152/70;
    }

    emit this->progressCounter(30);

    for(int line = 0; line < nlin; line++)
    {
        if( line % deltaprogress == 0)
        {
            deltaprogresscounter++;
            emit this->progressCounter(deltaprogresscounter);
        }

        row_col = (QRgb*)im->scanLine(line);

        for (int pixelx = 0 ; pixelx < npix; pixelx++)
        {
            if(kindofimage == "VIS_IR Color")
            {
                rc = *(imageptrs->ptrRed[0] + line * npix + pixelx);
                gc = *(imageptrs->ptrGreen[0] + line * npix + pixelx);
                bc = *(imageptrs->ptrBlue[0] + line * npix + pixelx);

                if( rc >= 65528 || rc == 0 )
                    valcontrastred = 0;
                else
                {
                    this->SetupContrastStretch( stat_min[0], 0, stat_max[0], 255);
                    valcontrastred = ContrastStretch(rc);
                }
                if( gc >= 65528 || gc == 0 )
                    valcontrastgreen = 0;
                else
                {
                    this->SetupContrastStretch( stat_min[1], 0, stat_max[1], 255);
                    valcontrastgreen = ContrastStretch(gc);
                }
                if( bc >= 65528 || bc == 0 )
                    valcontrastblue = 0;
                else
                {
                    this->SetupContrastStretch( stat_min[2], 0, stat_max[2], 255);
                    valcontrastblue = ContrastStretch(bc);
                }

            }
            else if(kindofimage == "VIS_IR")
            {
                rc = *(imageptrs->ptrRed[0] + line * npix + pixelx);

                if( rc >= 65528 || rc == 0 )
                    valcontrastred = 0;
                else
                {
                    this->SetupContrastStretch( stat_min[0], 0, stat_max[0], 255);
                    valcontrastred = ContrastStretch(rc);
                }
                *(imageptrs->ptrRed[0] + line * npix + pixelx) = valcontrastred;

            }
            else if(kindofimage == "HRV")
            {
                rc = *(imageptrs->ptrHRV[0] + line * npix + pixelx);

                if( rc >= 65528 || rc == 0 )
                    valcontrastred = 0;
                else
                {
                    this->SetupContrastStretch( stat_min[0], 0, stat_max[0], 255);
                    valcontrastred = ContrastStretch(rc);
                }

            }

            if(kindofimage == "VIS_IR Color")
            {

                if(inversevector[0])
                    valcontrastred = 255 - valcontrastred;
                if(inversevector[1])
                    valcontrastgreen = 255 - valcontrastgreen;
                if(inversevector[2])
                    valcontrastblue = 255 - valcontrastblue;

                r = quint8(valcontrastred);
                g = quint8(valcontrastgreen);
                b = quint8(valcontrastblue);

                row_col[pixelx] = qRgb(r,g,b);

            }
            else if( kindofimage == "VIS_IR" || kindofimage == "HRV")
            {
                if(inversevector[0])
                    valcontrastred = 255 - valcontrastred;

                r = quint8(valcontrastred);
                g = quint8(valcontrastred);
                b = quint8(valcontrastred);
                row_col[pixelx] = qRgb(r,g,b);

            }
        }
    }

    emit this->progressCounter(100);

    for(int j = 0; j < filelist.size(); j++)
    {

        H5Dclose(nomfileinfo_id[j]);
        H5Dclose(nomchannel_id[j]);
        H5Fclose(h5_file_id[j]);
    }

    emit signalcomposefinished(kindofimage, geoindex);

}

void SegmentListGeostationary::ComposeSegmentImagenetCDFInThread() //(QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector , int histogrammethod)
{
    QString ncfile[3];
    QByteArray arrayncfile[3];
    const char* pncfile[3];
    int ncfileid[3];
    int retval;
    int varid;
    QImage *im;
    float max_radiance_value_of_valid_pixels[3];
    float mean_radiance_value_of_valid_pixels[3];
    float min_radiance_value_of_valid_pixels[3];

    emit this->progressCounter(0);

    nc_type rh_type;
    int rh_ndims;
    int  rh_dimids[NC_MAX_VAR_DIMS];
    int rh_natts;
    size_t xdim=0, ydim=0;
    float scale_factor[3];
    float add_offset[3];
    int fillvalue[3];
    float nominal_satellite_subpoint_lon;


    QStringList filelistsorted;

    for(int i = 0; i < this->spectrumvector.count(); i++)
    {
        for(int j = 0; j < this->segmentfilelist.count(); j++)
        {
            if(this->segmentfilelist[j].mid(opts.geosatellites.at(geoindex).indexspectrum, opts.geosatellites.at(geoindex).spectrumlist.at(0).length()) == this->spectrumvector.at(i))
                filelistsorted.append(this->segmentfilelist[j]);
        }
    }


    for(int j = 0; j < filelistsorted.size(); j++)
    {
        ncfile[j] = this->getImagePath() + "/" + filelistsorted.at(j);
        arrayncfile[j] = ncfile[j].toUtf8();
        pncfile[j] = arrayncfile[j].constData();

        //qDebug() << "Starting netCDF file " + ncfile[j];
        retval = nc_open(pncfile[j], NC_NOWRITE, &ncfileid[j]);
        if(retval != NC_NOERR) qDebug() << "error opening netCDF file " << this->segmentfilelist.at(j);

        retval = nc_inq_varid(ncfileid[j], "max_radiance_value_of_valid_pixels", &varid);
        if (retval != NC_NOERR) qDebug() << "error reading max_radiance_value_of_valid_pixels id";
        retval = nc_get_var_float(ncfileid[j], varid, &max_radiance_value_of_valid_pixels[j]);
        if (retval != NC_NOERR) qDebug() << "error reading max_radiance_value_of_valid_pixels values";

        retval = nc_inq_varid(ncfileid[j], "mean_radiance_value_of_valid_pixels", &varid);
        if (retval != NC_NOERR) qDebug() << "error reading mean_radiance_value_of_valid_pixels id";
        retval = nc_get_var_float(ncfileid[j], varid, &mean_radiance_value_of_valid_pixels[j]);
        if (retval != NC_NOERR) qDebug() << "error reading mean_radiance_value_of_valid_pixels values";

        retval = nc_inq_varid(ncfileid[j], "min_radiance_value_of_valid_pixels", &varid);
        if (retval != NC_NOERR) qDebug() << "error reading min_radiance_value_of_valid_pixels id";
        retval = nc_get_var_float(ncfileid[j], varid, &min_radiance_value_of_valid_pixels[j]);
        if (retval != NC_NOERR) qDebug() << "error reading min_radiance_value_of_valid_pixels values";


        retval = nc_inq_varid(ncfileid[j], "Rad", &varid);
        if (retval != NC_NOERR) qDebug() << "error reading Rad id";
        retval = nc_inq_var (ncfileid[j], varid, 0, &rh_type, &rh_ndims, rh_dimids, &rh_natts);
        if (retval != NC_NOERR) qDebug() << "error reading inq var";
        retval = nc_inq_dimlen(ncfileid[j], rh_dimids[0], &xdim);
        if (retval != NC_NOERR) qDebug() << "error reading xdim";
        retval = nc_inq_dimlen(ncfileid[j], rh_dimids[1], &ydim);
        if (retval != NC_NOERR) qDebug() << "error reading ydim";
        retval = nc_get_att_float(ncfileid[j], varid, "scale_factor", &scale_factor[j]);
        if (retval != NC_NOERR) qDebug() << "error reading scale_factor";
        retval = nc_get_att_float(ncfileid[j], varid, "add_offset", &add_offset[j]);
        if (retval != NC_NOERR) qDebug() << "error reading add_offset";
        retval = nc_get_att_int(ncfileid[j], varid, "_FillValue", &fillvalue[j]);
        if (retval != NC_NOERR) qDebug() << "error reading _FillValue (1324)";


        qDebug() << QString("scale_factor = %1").arg(scale_factor[j]);
        qDebug() << QString("add_offset = %1").arg(add_offset[j]);
        qDebug() << QString("_FillValue = %1").arg(fillvalue[j]);

        this->number_of_columns = xdim;
        this->number_of_lines = ydim;


        if(j==0)
        {
            imageptrs->fillvalue[j] = fillvalue[j];
            imageptrs->ptrRed[0] = new quint16[xdim * ydim];
            memset(imageptrs->ptrRed[0], 0, xdim * ydim * sizeof(quint16));
            retval = nc_get_var_ushort(ncfileid[j], varid, imageptrs->ptrRed[0]);
            if (retval != NC_NOERR)
                qDebug() << "error reading Rad values";
            else
            {
                CalculateMinMax(j, xdim, ydim, imageptrs->ptrRed[0], fillvalue[0]);
                //CalculateLUTGeo(j);
                normalizeMinMax(xdim, ydim, imageptrs->ptrRed[0], stat_min[0], stat_max[0], fillvalue[0], 1023);
                equalizeHistogram(imageptrs->ptrRed[0], xdim, ydim, j, fillvalue[0], 1023);
            }
        } else if(j==1)
        {
            imageptrs->fillvalue[j] = fillvalue[j];
            imageptrs->ptrGreen[0] = new quint16[xdim * ydim];
            memset(imageptrs->ptrGreen[0], 0, xdim * ydim * sizeof(quint16));
            retval = nc_get_var_ushort(ncfileid[j], varid, imageptrs->ptrGreen[0]);
            if (retval != NC_NOERR)
                qDebug() << "error reading Rad values";
            else
            {
                CalculateMinMax(j, xdim, ydim, imageptrs->ptrGreen[0], fillvalue[1]);
                //CalculateLUTGeo(j);
                normalizeMinMax(xdim, ydim, imageptrs->ptrGreen[0], stat_min[1], stat_max[1], fillvalue[1], 1023);
                equalizeHistogram(imageptrs->ptrGreen[0], xdim, ydim, j, fillvalue[1], 1023);
            }
        } else if(j == 2)
        {
            imageptrs->fillvalue[j] = fillvalue[j];
            imageptrs->ptrBlue[0] = new quint16[xdim * ydim];
            memset(imageptrs->ptrBlue[0], 0, xdim * ydim * sizeof(quint16));
            retval = nc_get_var_ushort(ncfileid[j], varid, imageptrs->ptrBlue[0]);
            if (retval != NC_NOERR)
                qDebug() << "error reading Rad values";
            else
            {
                CalculateMinMax(j, xdim, ydim, imageptrs->ptrBlue[0], fillvalue[2]);
                //CalculateLUTGeo(j);
                normalizeMinMax(xdim, ydim, imageptrs->ptrBlue[0], stat_min[2], stat_max[2], fillvalue[2], 1023);
                equalizeHistogram(imageptrs->ptrBlue[0], xdim, ydim, j, fillvalue[2], 1023);
            }
        }

        long cnt_dqf0, cnt_dqf1, cnt_dqf2, cnt_dqf3;
        cnt_dqf0 = 0;
        cnt_dqf1 = 0;
        cnt_dqf2 = 0;
        cnt_dqf3 = 0;


        //        retval = nc_inq_varid(ncfileid[j], "DQF", &varid);
        //        if (retval != NC_NOERR) qDebug() << "error reading DQF id";

        //        imageptrs->ptrDQF[j] = new qint8[xdim * ydim];
        //        memset(imageptrs->ptrDQF[j], 0, xdim * ydim * sizeof(qint8));
        //        retval = nc_get_var(ncfileid[j], varid, imageptrs->ptrDQF[0]);
        //        if (retval != NC_NOERR)
        //            qDebug() << "error reading DQF values";
        //        else
        //        {
        //            qDebug() << "No errors reading DQF values";
        //            for(int i = 0; i < xdim * ydim; i++)
        //            {
        //                if( imageptrs->ptrDQF[j][i] == 0)
        //                    cnt_dqf0++;
        //                else if( imageptrs->ptrDQF[j][i] == 1)
        //                    cnt_dqf1++;
        //                else if( imageptrs->ptrDQF[j][i] == 2)
        //                    cnt_dqf2++;
        //                else if( imageptrs->ptrDQF[j][i] == 3)
        //                    cnt_dqf3++;

        //            }
        //            qDebug() << QString("# DQF 0 = %1").arg(cnt_dqf0);
        //            qDebug() << QString("# DQF 1 = %1").arg(cnt_dqf1);
        //            qDebug() << QString("# DQF 2 = %1").arg(cnt_dqf2);
        //            qDebug() << QString("# DQF 3 = %1").arg(cnt_dqf3);


        //        }



        qDebug() << "type is = " << rh_type;
        qDebug() << "number of dimensions = " << rh_ndims;
        qDebug() << "rh_dimids[0] = " << rh_dimids[0] << " " << rh_dimids[1];
        qDebug() << "rh_natts = " << rh_natts;
        qDebug() << "xdim = " << xdim << " ydim = " << ydim;


        qDebug() << "min_radiance_value_of_valid_pixels = " << min_radiance_value_of_valid_pixels[j];
        qDebug() << "max_radiance_value_of_valid_pixels = " << max_radiance_value_of_valid_pixels[j];
        qDebug() << "mean_radiance_value_of_valid_pixels = " << mean_radiance_value_of_valid_pixels[j];


        qDebug() << "stat_min = " << stat_min[j];
        qDebug() << "stat_max = " << stat_max[j];

        retval = nc_inq_varid(ncfileid[j], "nominal_satellite_subpoint_lon", &varid);
        if (retval != NC_NOERR) qDebug() << "error reading nominal_satellite_subpoint_lon id";
        retval = nc_get_var_float(ncfileid[j], varid, &nominal_satellite_subpoint_lon);
        if (retval != NC_NOERR) qDebug() << "error reading nominal_satellite_subpoint_lon values";

        qDebug() << "nominal_satellite_subpoint_lon = " << nominal_satellite_subpoint_lon;
        //this->geosatlon = nominal_satellite_subpoint_lon;
        //opts.geosatellites[(int)eGeoSatellite::GOES_16].longitude = nominal_satellite_subpoint_lon;

        float scale_factor_x, add_offset_x;
        float scale_factor_y, add_offset_y;

        retval = nc_inq_varid(ncfileid[j], "x", &varid);
        if (retval != NC_NOERR) qDebug() << "error reading x id";
        retval = nc_get_att_float(ncfileid[j], varid, "scale_factor", &scale_factor_x);
        if (retval != NC_NOERR) qDebug() << "error reading scale_factor x";
        retval = nc_get_att_float(ncfileid[j], varid, "add_offset", &add_offset_x);
        if (retval != NC_NOERR) qDebug() << "error reading add_offset x";

        retval = nc_inq_varid(ncfileid[j], "y", &varid);
        if (retval != NC_NOERR) qDebug() << "error reading y id";
        retval = nc_get_att_float(ncfileid[j], varid, "scale_factor", &scale_factor_y);
        if (retval != NC_NOERR) qDebug() << "error reading scale_factor y";
        retval = nc_get_att_float(ncfileid[j], varid, "add_offset", &add_offset_y);
        if (retval != NC_NOERR) qDebug() << "error reading add_offset y";

        qDebug() << QString("scale_factor_x = %1, add_offset_x = %2").arg(scale_factor_x).arg(add_offset_x);
        qDebug() << QString("scale_factor_y = %1, add_offset_y = %2").arg(scale_factor_y).arg(add_offset_y);


        retval = nc_close(ncfileid[j]);
        if (retval != NC_NOERR) qDebug() << "error closing file1";

    }

    quint16 rc, gc, bc;
    quint16 true_green;
    quint16 indexoutrc = 0, indexoutgc = 0, indexoutbc = 0;
    int r = 0, g = 0, b = 0;
    QRgb *row_col;
    //    qint8 dqfvalue[3];

    imageptrs->InitializeImageGeostationary(xdim, ydim);
    im = imageptrs->ptrimageGeostationary;

    int progcounter = 30;
    emit this->progressCounter(progcounter);


    if(this->pseudocolor) // R -> C02; G -> C03; B -> C01
    {
        for(int line = 0; line < ydim; line++)
        {
            for (int pixelx = 0 ; pixelx < xdim; pixelx++)
            {
                if(kindofimage == "VIS_IR Color")
                {
                    rc = *(imageptrs->ptrRed[0] + line * xdim + pixelx);
                    gc = *(imageptrs->ptrGreen[0] + line * xdim + pixelx);
                    bc = *(imageptrs->ptrBlue[0] + line * xdim + pixelx);

                    if(rc != fillvalue[0] && gc != fillvalue[1] && bc != fillvalue[2] )
                    {
                        true_green = (quint16)qMin(qMax(qRound(0.48358168 * (float)rc + 0.45706946 * (float)bc + 0.06038137 * (float)gc), 0), 1023);
                        *(imageptrs->ptrGreen[0] + line * xdim + pixelx) = true_green;
                    }
                }
            }
        }
    }

    for(int line = 0; line < ydim; line++)
    {
        row_col = (QRgb*)im->scanLine(line);

        for (int pixelx = 0 ; pixelx < xdim; pixelx++)
        {
            if(kindofimage == "VIS_IR Color")
            {

                rc = *(imageptrs->ptrRed[0] + line * xdim + pixelx);
                gc = *(imageptrs->ptrGreen[0] + line * xdim + pixelx);
                bc = *(imageptrs->ptrBlue[0] + line * xdim + pixelx);

                //                dqfvalue[0] = *(imageptrs->ptrDQF[0] + line * xdim + pixelx);
                //                dqfvalue[1] = *(imageptrs->ptrDQF[1] + line * xdim + pixelx);
                //                dqfvalue[2] = *(imageptrs->ptrDQF[2] + line * xdim + pixelx);

                if(this->histogrammethod == CMB_HISTO_NONE_95)
                {
                    if(rc != fillvalue[0])
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - imageptrs->minRadianceIndex[0] ) / (float)(imageptrs->maxRadianceIndex[0] - imageptrs->minRadianceIndex[0])), 0), 1023);
                    if(gc != fillvalue[1])
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(gc - imageptrs->minRadianceIndex[1] ) / (float)(imageptrs->maxRadianceIndex[1] - imageptrs->minRadianceIndex[1])), 0), 1023);
                    if(bc != fillvalue[2])
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(bc - imageptrs->minRadianceIndex[2] ) / (float)(imageptrs->maxRadianceIndex[2] - imageptrs->minRadianceIndex[2])), 0), 1023);
                }
                else if(this->histogrammethod == CMB_HISTO_NONE_100)
                {
                    if(rc != fillvalue[0])
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(gc - this->stat_min[1] ) / (float)(this->stat_max[1] - this->stat_min[1])), 0), 1023);
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(bc - this->stat_min[2] ) / (float)(this->stat_max[2] - this->stat_min[2])), 0), 1023);
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( rc != fillvalue[0])
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(gc - this->stat_min[1] ) / (float)(this->stat_max[1] - this->stat_min[1])), 0), 1023);
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(bc - this->stat_min[2] ) / (float)(this->stat_max[2] - this->stat_min[2])), 0), 1023);

                        indexoutrc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[0][rc]), 0), 1023);
                        indexoutgc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[1][gc]), 0), 1023);
                        indexoutbc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[2][bc]), 0), 1023);
                    }
                }

                //                if( (rc == fillvalue[0] && dqfvalue[0] == -1) || (gc == fillvalue[1] && dqfvalue[1] == -1) || (bc == fillvalue[2] && dqfvalue[2] == -1))
                if( (rc == fillvalue[0]) || (gc == fillvalue[1]) || (bc == fillvalue[2]))
                {
                    r = 0;
                    g = 0;
                    b = 0;
                }
                else
                {
                    r = quint16(this->inversevector[0] ? (1023 - indexoutrc)/4 : indexoutrc/4);
                    g = quint16(this->inversevector[1] ? (1023 - indexoutgc)/4 : indexoutgc/4);
                    b = quint16(this->inversevector[2] ? (1023 - indexoutbc)/4 : indexoutbc/4);
                }

            }
            else if(kindofimage == "VIS_IR")
            {
                rc = *(imageptrs->ptrRed[0] + line * xdim + pixelx);
                //                dqfvalue[0] = *(imageptrs->ptrDQF[0] + line * xdim + pixelx);

                if(this->histogrammethod == CMB_HISTO_NONE_95)
                {
                    if(rc != fillvalue[0])
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - imageptrs->minRadianceIndex[0] ) / (float)(imageptrs->maxRadianceIndex[0] - imageptrs->minRadianceIndex[0])), 0), 1023);
                }
                else if(this->histogrammethod == CMB_HISTO_NONE_100)
                {
                    if(rc != fillvalue[0])
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);
                }
                else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( rc != fillvalue[0])
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);
                        indexoutrc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[0][indexoutrc]), 0), 1023);
                    }
                }

                if( rc == fillvalue[0] ) //&& dqfvalue[0] == -1)
                {
                    r = 0;
                    g = 0;
                    b = 0;
                }
                else if( rc == fillvalue[0] ) // && dqfvalue[0] == 2 )
                {
                    r = 255;
                    g = 255;
                    b = 255;

                }
                else
                {
                    r = quint16(this->inversevector[0] ? (1023 - indexoutrc)/4 : indexoutrc/4);
                    g = r;
                    b = r;
                }

            }
            row_col[pixelx] = qRgb(r,g,b);

        }
        if(line % 160 == 0)
        {
            progcounter++;
            emit this->progressCounter(progcounter);
        }
    }


    emit this->progressCounter(100);
    emit signalcomposefinished(kindofimage, geoindex);
}

void SegmentListGeostationary::ComposeSegmentImagenetCDFMTGInThread()
{

    QString ncfile;
    QByteArray arrayncfile;
    const char* pncfile;
    int ncfileid;
    int grp_data;
    int grp_channel;
    int grp_measured;
    int grp_spectrum;

    int retval;
    int varid;
    QImage *im;
    float max_radiance_value_of_valid_pixels[3];
    float mean_radiance_value_of_valid_pixels[3];
    float min_radiance_value_of_valid_pixels[3];

    emit this->progressCounter(10);

    nc_type rh_type;
    int rh_ndims;
    int  rh_dimids[NC_MAX_VAR_DIMS];
    int rh_natts;
    size_t xdim=0, ydim=0;
    float scale_factor[3];
    float add_offset[3];
    quint16 fillvalue[3];
    float nominal_satellite_subpoint_lon;
    bool trailfilefound = false;

    ushort end_position_row;
    ushort end_position_column;
    ushort start_position_row;
    ushort start_position_column;
    ushort total_number_of_rows;
    ushort total_number_of_columns;

    double gamma = opts.meteosatgamma;
    double gammafactor = 255 / pow(255, gamma);
    quint16 valgamma;
    quint8 valcontrast;


    ushort *effective_radiance_data;
    int geoindex = this->getGeoSatelliteIndex();

    QStringList spectrumlist = opts.geosatellites.at(geoindex).spectrumlist;
    effective_radiance_data = NULL;


    qDebug() << "Start SegmentListGeostationary::ComposeSegmentImagenetCDFMTGInThread() size = " << this->segmentfilelist.size();
    qDebug() << "Spectrum vector count = " << this->spectrumvector.count() << " kindofimage = " << kindofimage;
    if(this->histogrammethod == CMB_HISTO_NONE_95)
        qDebug() << "histgrammethod = CMB_HISTO_NONE_95";
    else if(this->histogrammethod == CMB_HISTO_NONE_100)
        qDebug() << "histgrammethod = CMB_HISTO_NONE_100";
    else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
        qDebug() << "histgrammethod = CMB_HISTO_EQUALIZE";

    QElapsedTimer timer;
    timer.start();

    // in include file we have the following definition : quint 16 imageptrs->ptrRed[10];
    // ptrBlue and ptrGreen are not used
    for(int j = 0; j < this->segmentfilelist.size(); j++)
    {
        if(this->segmentfilelist.at(j).contains("TRAIL"))
        {
            trailfilefound = true;
            ncfile = this->getImagePath() + "/" + this->segmentfilelist.at(j);
            arrayncfile = ncfile.toUtf8();
            pncfile = arrayncfile.constData();

            //qDebug() << "Starting netCDF file " + ncfile;
            retval = nc_open(pncfile, NC_NOWRITE, &ncfileid);
            if(retval != NC_NOERR) qDebug() << "error opening netCDF file " << this->segmentfilelist.at(j);

            retval = nc_inq_ncid(ncfileid, "data", &grp_data);
            if(retval != NC_NOERR) qDebug() << "error opening data group";

            for(int i = 0; i < (kindofimage == "VIS_IR Color" ? 3 : 1); i++)
            {

                QString strspectrum = "data/" + this->spectrumvector.at(i);
                QByteArray ba = strspectrum.toLocal8Bit();
                const char *c_channel = ba.data();
                retval = nc_inq_grp_full_ncid(ncfileid, c_channel, &grp_spectrum);
                if(retval != NC_NOERR) qDebug() << "error opening " << strspectrum;

                if ((retval = nc_inq_varid(grp_spectrum, "number_of_rows", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_spectrum, varid, &total_number_of_rows)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_spectrum, "number_of_columns", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_spectrum, varid, &total_number_of_columns)))
                    ERR(retval);
                imageptrs->mtg_total_number_of_columns[i] = total_number_of_columns;
                imageptrs->mtg_total_number_of_rows[i] = total_number_of_rows;
                imageptrs->ptrRed[i] = new quint16[total_number_of_columns * total_number_of_rows];

                qDebug() << QString("Total number of rows = %1, columns = %2").arg(total_number_of_rows).arg(total_number_of_columns);
            }
        }
    }

    for(int j = 0; j < this->segmentfilelist.size(); j++)
    {
        if(this->segmentfilelist.at(j).contains("BODY"))
        {
            ncfile = this->getImagePath() + "/" + this->segmentfilelist.at(j);
            arrayncfile = ncfile.toUtf8();
            pncfile = arrayncfile.constData();

            int ind = ncfile.indexOf(".nc");
            QString findex = ncfile.mid(ind - 4, 4);

            qDebug() << "Starting netCDF file : " + findex;
            // Starting netCDF file : /media/hugo/Data-Linux/MTG_examples/compressed_full_TD-360/2017/09/20/W_XX-EUMETSAT-Darmstadt,IMG+SAT,MTI1+FCI-1C-RRAD-FDHSI-FD--CHK-BODY---NC4E_C_EUMT_20170920120515_GTT_DEV_20170920120008_20170920120015_N_JLS_T_0073_0001.nc
            retval = nc_open(pncfile, NC_NOWRITE, &ncfileid);
            if(retval != NC_NOERR) qDebug() << "error opening netCDF file " << this->segmentfilelist.at(j);

            retval = nc_inq_ncid(ncfileid, "data", &grp_data);
            if(retval != NC_NOERR) qDebug() << "error opening data group";

            for(int i = 0; i < (kindofimage == "VIS_IR Color" ? 3 : 1); i++)
            {
                //qDebug() << "reading radiance from channel " << this->spectrumvector.at(i);

                QString strmeasured = "data/" + this->spectrumvector.at(i) + "/measured";
                QByteArray ba = strmeasured.toLocal8Bit();
                const char *c_channel = ba.data();
                retval = nc_inq_grp_full_ncid(ncfileid, c_channel, &grp_measured);
                if(retval != NC_NOERR) qDebug() << "error opening " << strmeasured;


                if ((retval = nc_inq_varid(grp_measured, "start_position_row", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &start_position_row)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_measured, "start_position_column", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &start_position_column)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_measured, "end_position_row", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &end_position_row)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_measured, "end_position_column", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &end_position_column)))
                    ERR(retval);
                //                if(retval == 0 && i == 0)
                //                {
                //                    qDebug() << QString("start position row = %1 column = %2").arg(start_position_row).arg(start_position_column);
                //                    qDebug() << QString("end position row = %1 column = %2").arg(end_position_row).arg(end_position_column);
                //                    qDebug() << QString("nbr of rows = %1 column = %2").arg(end_position_row - start_position_row + 1).arg(
                //                        end_position_column - start_position_column + 1);
                //                }

                imageptrs->mtg_start_position_row[i][j] = start_position_row;
                imageptrs->mtg_end_position_row[i][j] = end_position_row;

                imageptrs->mtg_start_position_column[i][j] = start_position_column;
                imageptrs->mtg_end_position_column[i][j] = end_position_column;


                int nbr_rows = end_position_row - start_position_row + 1;
                int nbr_col = end_position_column - start_position_column + 1;

                effective_radiance_data = new quint16[nbr_rows * nbr_col];

                retval = nc_inq_varid(grp_measured, "effective_radiance", &varid);
                if(retval != NC_NOERR) qDebug() << "error opening effective radiance from channel " << strmeasured;
                retval = nc_get_att_ushort(grp_measured, varid, "_FillValue", &fillvalue[i]);
                if (retval != NC_NOERR) qDebug() << "error reading _FillValue (1809)";
                imageptrs->fillvalue[i] = fillvalue[i];
                //qDebug() << QString("FillValue for color %1 = %2").arg(i).arg(fillvalue[i]);


                retval = nc_get_var_ushort(grp_measured, varid, effective_radiance_data);
                if(retval != NC_NOERR) qDebug() << "error reading effective radiance from channel " << strmeasured;

                //                if(i == 0)
                //                    qDebug() << "copy to ptrRed start_position = " << (start_position_row - 1) * nbr_col;
                memcpy(imageptrs->ptrRed[i] + nbr_col * (start_position_row - 1), effective_radiance_data, sizeof(quint16)*nbr_rows*nbr_col);

                if(effective_radiance_data != NULL)
                    delete [] effective_radiance_data;

            }
            retval = nc_close(ncfileid);
            if (retval != NC_NOERR) qDebug() << "error closing file " << ncfile;
        }

    }

    //qDebug() << "===> The reading of 41 files took " << timer.restart() << "milliseconds";

    quint16 pix[3];

    //    long count_no_fillvalue[3];

    //    count_no_fillvalue[0]= 0;
    //    count_no_fillvalue[1]= 0;
    //    count_no_fillvalue[2]= 0;

    //    for(int line = 0; line < this->total_number_of_rows[0]; line++)
    //    {
    //        for (int pixelx = 0 ; pixelx < this->total_number_of_columns[0]; pixelx++)
    //        {
    //            for(int k = 0; k < 3; k++)
    //            {
    //                pix[k] = *(imageptrs->ptrRed[k] + line * this->total_number_of_columns[0] + pixelx);
    //                if(pix[k] != imageptrs->fillvalue[k])
    //                    count_no_fillvalue[k]++;
    //            }
    //        }
    //    }

    //    qDebug() << "count_no_fillvalue 0 = " << count_no_fillvalue[0];
    //    qDebug() << "count_no_fillvalue 1 = " << count_no_fillvalue[1];
    //    qDebug() << "count_no_fillvalue 2 = " << count_no_fillvalue[2];

    for(int j = 0; j < (kindofimage == "VIS_IR Color" ? 3 : 1); j++)
    {
        CalculateMinMax(j, imageptrs->mtg_total_number_of_columns[j], imageptrs->mtg_total_number_of_rows[j], imageptrs->ptrRed[j], imageptrs->fillvalue[j]);
        CalculateLUTGeoMTG(j);
    }

    //qDebug() << "===> MinMax + LUT took " << timer.restart() << "milliseconds";

    for(int i = 0; i < 3; i++)
    {
        qDebug() << QString("stat_min[%1] = %2 stat_max[%3] = %4").arg(i).arg(stat_min[i]).arg(i).arg(stat_max[i]);
    }

    for (int j = 0; j < 1024; j=j+20)
    {
        qDebug() << "LUT " << j << " " << imageptrs->lut_ch[0][j];
    }

    qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4 active_pixels = %5")
                .arg(0).arg(imageptrs->minRadianceIndex[0]).arg(0).arg(imageptrs->maxRadianceIndex[0])
            .arg(this->active_pixels[0]);


    quint16 rc, gc, bc;
    quint16 true_green;
    quint16 indexoutrc = 0, indexoutgc = 0, indexoutbc = 0;
    int r = 0, g = 0, b = 0;
    QRgb *row_col;

    imageptrs->InitializeImageGeostationary(imageptrs->mtg_total_number_of_columns[0], imageptrs->mtg_total_number_of_rows[0]);
    im = imageptrs->ptrimageGeostationary;

    int progcounter = 30;
    emit this->progressCounter(progcounter);



    for(int line = 0; line < imageptrs->mtg_total_number_of_rows[0]; line++)
    {
        row_col = (QRgb*)im->scanLine(line);

        for (int pixelx = 0; pixelx < imageptrs->mtg_total_number_of_columns[0]; pixelx++)
        {
            if(kindofimage == "VIS_IR Color")
            {
                rc = *(imageptrs->ptrRed[0] + (imageptrs->mtg_total_number_of_rows[0] - 1 - line) * imageptrs->mtg_total_number_of_columns[0] + pixelx);
                gc = *(imageptrs->ptrRed[1] + (imageptrs->mtg_total_number_of_rows[0] - 1 - line) * imageptrs->mtg_total_number_of_columns[0] + pixelx);
                bc = *(imageptrs->ptrRed[2] + (imageptrs->mtg_total_number_of_rows[0] - 1 - line) * imageptrs->mtg_total_number_of_columns[0] + pixelx);

                if(this->histogrammethod == CMB_HISTO_NONE_95)
                {
                    if(rc != imageptrs->fillvalue[0])
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - imageptrs->minRadianceIndex[0] ) / (float)(imageptrs->maxRadianceIndex[0] - imageptrs->minRadianceIndex[0])), 0), 1023);
                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                    if(gc != imageptrs->fillvalue[1])
                    {
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(gc - imageptrs->minRadianceIndex[1] ) / (float)(imageptrs->maxRadianceIndex[1] - imageptrs->minRadianceIndex[1])), 0), 1023);
                        valgamma = pow( indexoutgc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        g = quint8(valcontrast);
                        if (g > 255)
                            g = 255;
                    }
                    if(bc != imageptrs->fillvalue[2])
                    {
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(bc - imageptrs->minRadianceIndex[2] ) / (float)(imageptrs->maxRadianceIndex[2] - imageptrs->minRadianceIndex[2])), 0), 1023);
                        valgamma = pow( indexoutbc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        b = quint8(valcontrast);
                        if (b > 255)
                            b = 255;
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_NONE_100)
                {
                    if(rc != imageptrs->fillvalue[0])
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);
                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                    if(gc != imageptrs->fillvalue[1])
                    {
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(gc - this->stat_min[1] ) / (float)(this->stat_max[1] - this->stat_min[1])), 0), 1023);
                        valgamma = pow( indexoutgc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        g = quint8(valcontrast);
                        if (g > 255)
                            g = 255;
                    }
                    if(bc != imageptrs->fillvalue[2])
                    {
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(bc - this->stat_min[2] ) / (float)(this->stat_max[2] - this->stat_min[2])), 0), 1023);
                        valgamma = pow( indexoutbc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        b = quint8(valcontrast);
                        if (b > 255)
                            b = 255;
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( rc != imageptrs->fillvalue[0])
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);
                        indexoutrc = (quint16)(qMin(qMax(qRound((float)imageptrs->lut_ch[0][indexoutrc]), 0), 1023));
                        Q_ASSERT(indexoutrc < 1024);

                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                    if( gc != imageptrs->fillvalue[1])
                    {
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(gc - this->stat_min[1] ) / (float)(this->stat_max[1] - this->stat_min[1])), 0), 1023);
                        indexoutgc = (quint16)(qMin(qMax(qRound((float)imageptrs->lut_ch[1][indexoutgc]), 0), 1023));
                        Q_ASSERT(indexoutgc < 1024);

                        valgamma = pow( indexoutgc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        g = quint8(valcontrast);
                        if (g > 255)
                            g = 255;
                    }
                    if( bc != imageptrs->fillvalue[2])
                    {
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(bc - this->stat_min[2] ) / (float)(this->stat_max[2] - this->stat_min[2])), 0), 1023);
                        indexoutbc = (quint16)(qMin(qMax(qRound((float)imageptrs->lut_ch[2][indexoutbc]), 0), 1023));
                        Q_ASSERT(indexoutbc < 1024);
                        valgamma = pow( indexoutbc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        b = quint8(valcontrast);
                        if (b > 255)
                            b = 255;
                    }
                }

                if(rc == imageptrs->fillvalue[0]) r = 0;
                if(gc == imageptrs->fillvalue[1]) g = 0;
                if(bc == imageptrs->fillvalue[2]) b = 0;

                r = quint16(this->inversevector[0] ? (255 - r) : r);
                g = quint16(this->inversevector[1] ? (255 - g) : g);
                b = quint16(this->inversevector[2] ? (255 - b) : b);

                row_col[pixelx] = qRgb(r,g,b);

            }
            else if(kindofimage == "VIS_IR")
            {
                rc = *(imageptrs->ptrRed[0] + (imageptrs->mtg_total_number_of_rows[0] - 1 - line) * imageptrs->mtg_total_number_of_columns[0] + pixelx);

                if(this->histogrammethod == CMB_HISTO_NONE_95)
                {
                    if(rc != imageptrs->fillvalue[0])
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - imageptrs->minRadianceIndex[0] ) / (float)(imageptrs->maxRadianceIndex[0] - imageptrs->minRadianceIndex[0])), 0), 1023);
                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_NONE_100)
                {
                    if(rc != imageptrs->fillvalue[0])
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);

                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( rc != imageptrs->fillvalue[0])
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);
                        indexoutrc = (quint16)(qMin(qMax(qRound((float)imageptrs->lut_ch[0][indexoutrc]), 0), 1023));
                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                }

                //                if(rc == imageptrs->fillvalue[0])
                //                    r = 0;
                //                else
                //                    r = quint16(this->inversevector[0] ? (255 - r) : r);
                //                g = r;
                //                b = r;
                //                row_col[pixelx] = qRgb(r,g,b);

                if(rc != imageptrs->fillvalue[0])
                {
                    r = quint16(this->inversevector[0] ? (255 - r) : r);
                    g = r;
                    b = r;
                    row_col[pixelx] = qRgb(r,g,b);
                }


            }
        }
        if(line % 280 == 0)
        {
            progcounter++;
            emit this->progressCounter(progcounter);
        }
    }



    delete [] imageptrs->ptrRed[0];

    if(kindofimage == "VIS_IR Color")
    {
        delete [] imageptrs->ptrRed[1];
        delete [] imageptrs->ptrRed[2];
    }
    imageptrs->ptrRed[0] = NULL;
    imageptrs->ptrRed[1] = NULL;
    imageptrs->ptrRed[2] = NULL;


    qDebug() << "===> The image process for NO concurrent took " << timer.elapsed() << "milliseconds";

    emit signalcomposefinished(kindofimage, geoindex);
    emit this->progressCounter(100);

    qDebug() << "****************************************";

}

void SegmentListGeostationary::ComposeSegmentImageXRITMSGInThreadConcurrent()
{
    qDebug() << "start ComposeSegmentImageXRITMSGInThreadConcurrent()";

    emit this->progressCounter(20);

    if(m_GeoSatellite == eGeoSatellite::H9)
    {
        if(kindofimage == "VIS_IR" || kindofimage == "VIS_IR Color")
        {
            for(int i = 0; i < this->segmentfilelist.size(); i++)
            {
                this->concurrentReadFilelistHimawari(this, this->segmentfilelist.at(i));
            }

            //            auto callbackMethod = std::bind(this->concurrentReadFilelistHimawari, this, std::placeholders::_1);
            //            QtConcurrent::blockingMap(this->segmentfilelist, callbackMethod);

            emit this->progressCounter(50);

            //            for (int i = 0; i < this->segmentfilelist.size(); ++i)
            //                this->concurrentReadFilelistHimawari(this, this->segmentfilelist.at(i));

            this->ComposeVISIRHimawari();
            emit this->progressCounter(100);

            emit signalcomposefinished(kindofimage, geoindex);


        }
    }
    else
    {
        if(kindofimage == "VIS_IR" || kindofimage == "VIS_IR Color" || kindofimage == "HRV Color")
        {
            for(int i = 0; i < this->segmentfilelist.size(); i++)
            {
                this->concurrentReadFilelist(this, this->segmentfilelist.at(i));
            }
        }
        if(kindofimage == "HRV" || kindofimage == "HRV Color")
        {
            for(int i = 0; i < this->segmentfilelisthrv.size(); i++)
            {
                this->concurrentReadFilelist(this, this->segmentfilelisthrv.at(i));
            }
        }

        //        auto callbackMethod = std::bind(this->concurrentReadFilelist, this, std::placeholders::_1);

        //        if(kindofimage == "VIS_IR" || kindofimage == "VIS_IR Color" || kindofimage == "HRV Color")
        //        {
        //            QtConcurrent::blockingMap(this->segmentfilelist, callbackMethod);
        //        }

        //        if(kindofimage == "HRV" || kindofimage == "HRV Color")
        //        {
        //            QtConcurrent::blockingMap(this->segmentfilelisthrv, callbackMethod);
        //        }

        emit this->progressCounter(50);

        if(kindofimage == "HRV" || kindofimage == "HRV Color")
        {
            this->ComposeHRV();
            emit this->progressCounter(100);

            emit signalcomposefinished(kindofimage, geoindex);
        }
        else if(kindofimage == "VIS_IR" || kindofimage == "VIS_IR Color")
        {
            this->ComposeVISIR();
            emit this->progressCounter(100);

            emit signalcomposefinished(kindofimage, geoindex);
        }
    }
    qDebug() << "einde ComposeSegmentImageXRITMSGInThreadConcurrent()";
}

void SegmentListGeostationary::concurrentReadFilelist(SegmentListGeostationary *sm, QString llFile)
{

    MSG_header *header;
    MSG_data *msgdat;

    header = new MSG_header();
    msgdat = new MSG_data();

    QString filepath = QString("%1/%2").arg(sm->getImagePath(), llFile);

    int filesequence;
    QString filespectrum;
    QString filedate;
    int channelindex;

    sm->getFilenameParameters(llFile, &filespectrum, &filedate, &filesequence, &channelindex);

    std::ifstream hrit(filepath.toStdString(), (std::ios::binary | std::ios::in) );
    if (hrit.fail())
    {
        std::cerr << "Cannot open input hrit file " << filepath.toStdString() << std::endl;
        delete header;
        delete msgdat;

        return;
    }

    header->read_from(hrit);
    msgdat->read_from(hrit, *header);
    hrit.close();

    //std::cout << *header;

    if (header->segment_id->data_field_format == MSG_NO_FORMAT)
    {
        qDebug() << "Product dumped in binary format. " << (int)header->segment_id->data_field_format;
        delete header;
        delete msgdat;

        return;
    }

    int planned_end_segment = header->segment_id->planned_end_segment_sequence_number;

    int npix = sm->number_of_columns = header->image_structure->number_of_columns;
    int nlin = sm->number_of_lines = header->image_structure->number_of_lines;

    size_t npixperseg = npix*nlin;

    //qDebug() << QString("[%1] concurrentReadFilelist() planned end = %2 npix = %3 nlin = %4 filesequence = %5")
    //            .arg(sm->kindofimage).arg(planned_end_segment).arg(sm->number_of_columns).arg(sm->number_of_lines).arg(filesequence);
    qDebug() << QString("[%1] concurrentReadFilelist() file = %1 channelindex = %2 filesequence = %3")
                .arg(llFile).arg(channelindex).arg(filesequence);

    MSG_SAMPLE *pixels = new MSG_SAMPLE[npixperseg];
    memset(pixels, 0, npixperseg*sizeof(MSG_SAMPLE));
    memcpy(pixels, msgdat->image->data, npixperseg*sizeof(MSG_SAMPLE));

    quint16 c;

    if (filespectrum == "HRV")
    {
        imageptrs->ptrHRV[filesequence] = new quint16[nlin * npix];
        memset(imageptrs->ptrHRV[filesequence], 0, nlin * npix * sizeof(quint16));
    }
    else
    {
        if(channelindex == 0)
        {
            imageptrs->ptrRed[filesequence] = new quint16[nlin * npix];
            memset(imageptrs->ptrRed[filesequence], 0, nlin * npix * sizeof(quint16));
        }
        else if(channelindex == 1)
        {
            imageptrs->ptrGreen[filesequence] = new quint16[nlin * npix];
            memset(imageptrs->ptrGreen[filesequence], 0, nlin * npix * sizeof(quint16));
        }
        else if(channelindex == 2)
        {
            imageptrs->ptrBlue[filesequence] = new quint16[nlin * npix];
            memset(imageptrs->ptrBlue[filesequence], 0, nlin * npix * sizeof(quint16));
        }
    }

    for(int line = 0; line < nlin; line++)
    {
        //qDebug() << QString("filesequence = %1 ; nlin * totalsegs - 1 - startLine[filesequence] - line = %2").arg(filesequence).arg(nlin * totalsegs - 1 - startLine[filesequence] - line);

        for (int pixelx = 0 ; pixelx < npix; pixelx++)
        {
            c = *(pixels + line * npix + pixelx);
            if (filespectrum == "HRV")
            {
                *(imageptrs->ptrHRV[filesequence] + line * npix + pixelx) = c;
            }
            else
            {
                if(channelindex == 0)
                    *(imageptrs->ptrRed[filesequence] + line * npix + pixelx) = c;
                else if(channelindex == 1)
                    *(imageptrs->ptrGreen[filesequence] + line * npix + pixelx) = c;
                else if(channelindex == 2)
                    *(imageptrs->ptrBlue[filesequence] + line * npix + pixelx) = c;
            }
        }
    }

    delete header;
    delete msgdat;
    delete [ ] pixels;

}

void SegmentListGeostationary::concurrentReadFilelistHimawari(SegmentListGeostationary *sm, QString llFile)
{
    MSG_header *header;
    MSG_data *msgdat;
    int     nBuf;
    char    buf[ 32768 ];
    BZFILE* bzfile;
    int     bzerror;


    header = new MSG_header();
    msgdat = new MSG_data();

    QString filepath = QString("%1/%2").arg(sm->getImagePath(), llFile);

    QFile filein(filepath);
    QFileInfo fileinfo(filein);
    QString basename = fileinfo.baseName();


    int filesequence;
    QString filespectrum;
    QString filedate;
    int channelindex;

    sm->getFilenameParameters(llFile, &filespectrum, &filedate, &filesequence, &channelindex);

    QFile fileout(basename);
    fileout.open(QIODevice::WriteOnly);
    QDataStream streamout(&fileout);


    if((bzfile = BZ2_bzopen(fileinfo.absoluteFilePath().toLatin1(),"rb"))==NULL)
    {
        qDebug() << "error in BZ2_bzopen";
    }

    bzerror = BZ_OK;
    while ( bzerror == BZ_OK )
    {
        nBuf = BZ2_bzRead ( &bzerror, bzfile, buf, 32768 );
        if ( bzerror == BZ_OK || bzerror == BZ_STREAM_END)
        {
            streamout.writeRawData(buf, nBuf);
        }
    }

    BZ2_bzclose ( bzfile );

    fileout.close();

    //QByteArray ba = basename.toLatin1();
    //const char *c_segname = ba.data();

    std::ifstream hrit(basename.toStdString(), (std::ios::binary | std::ios::in) );
    if (hrit.fail())
    {
        std::cerr << "Cannot open input Himawari file "
                  << filepath.toStdString() << std::endl;
        return;
    }

    header->read_from(hrit);
    msgdat->read_from_himawari(hrit, *header);
    hrit.close();

    if (header->segment_id->data_field_format == MSG_NO_FORMAT)
    {
        qDebug() << "Product dumped in binary format.";
        delete header;
        delete msgdat;

        return;
    }

    int planned_end_segment = header->segment_id->planned_end_segment_sequence_number;

    int npix = sm->number_of_columns = header->image_structure->number_of_columns;
    int nlin = sm->number_of_lines = header->image_structure->number_of_lines;

    size_t npixperseg = sm->number_of_columns*sm->number_of_lines;

    qDebug() << QString("---->[%1] SegmentListGeostationary::concurrentReadFilelistHimawari() planned end = %2 npix = %3 nlin = %4 filesequence = %5 channelindex = %6").arg(sm->kindofimage).arg(planned_end_segment).arg(sm->number_of_columns).arg(sm->number_of_lines).arg(filesequence).arg(channelindex);

    MSG_SAMPLE *pixels = new MSG_SAMPLE[npixperseg];
    memset(pixels, 0, npixperseg*sizeof(MSG_SAMPLE));
    memcpy(pixels, msgdat->image->data, npixperseg*sizeof(MSG_SAMPLE));

    //    for(int i = 0; i < npixperseg; i++)
    //        pixels[i] = BYTE_SWAP2(pixels[i]);

    //    for (int i = 0; i < 10; ++i)
    //        std::cout << std::hex << std::setfill('0') << std::setw(2) << pixels[i] << " ";
    //    std::cout << std::endl;

    quint16 c;

    if(channelindex == 0)
    {
        imageptrs->ptrRed[filesequence] = new quint16[nlin*npix];
        memset(imageptrs->ptrRed[filesequence], 0, nlin*npix*sizeof(quint16));
    }
    else if(channelindex == 1)
    {
        imageptrs->ptrGreen[filesequence] = new quint16[nlin*npix];
        memset(imageptrs->ptrGreen[filesequence], 0, nlin*npix*sizeof(quint16));
    }
    else if(channelindex == 2)
    {
        imageptrs->ptrBlue[filesequence] = new quint16[nlin*npix];
        memset(imageptrs->ptrBlue[filesequence], 0, nlin*npix*sizeof(quint16));
    }

    for(int line = 0; line < nlin; line++)
    {
        for (int pixelx = 0 ; pixelx < npix; pixelx++)
        {
            c = *(pixels + line * npix + pixelx);
            if(channelindex == 0)
                *(imageptrs->ptrRed[filesequence] + line * npix + pixelx) = BYTE_SWAP2(c);
            else if(channelindex == 1)
                *(imageptrs->ptrGreen[filesequence] + line * npix + pixelx) = BYTE_SWAP2(c);
            else if(channelindex == 2)
                *(imageptrs->ptrBlue[filesequence] + line * npix + pixelx) = BYTE_SWAP2(c);

        }
    }

    delete header;
    delete msgdat;
    delete [] pixels;
}

        //No use of QtConcurrent
void SegmentListGeostationary::ComposeSegmentImagenetCDFMTGInThread1()
{
    QString ncfile;
    QByteArray arrayncfile;
    const char* pncfile;
    int ncfileid;
    int grp_data;
    int grp_channel;
    int grp_measured;
    int grp_spectrum;

    int retval;
    int varid;

    float max_radiance_value_of_valid_pixels[3];
    float mean_radiance_value_of_valid_pixels[3];
    float min_radiance_value_of_valid_pixels[3];

    int ndimsp, nvarsp, ngattsp, unlimdimidp;
    //ndimsp	Pointer to location for returned number of dimensions defined for this netCDF dataset. Ignored if NULL.
    //nvarsp	Pointer to location for returned number of variables defined for this netCDF dataset. Ignored if NULL.
    //nattsp	Pointer to location for returned number of global attributes defined for this netCDF dataset. Ignored if NULL.
    //unlimdimidp	Pointer to location for returned ID of the unlimited dimension, if there is one for this netCDF dataset.
    //              If no unlimited length dimension has been defined, -1 is returned. Ignored if NULL.
    //              If there are multiple unlimited dimensions (possible only for netCDF-4 files), only a pointer to the first is returned,
    //              for backward compatibility. If you want them all, use nc_inq_unlimids().

    double geospatial_lat_min, geospatial_lat_max;

    emit this->progressCounter(10);

    nc_type rh_type;
    int rh_ndims;
    int  rh_dimids[NC_MAX_VAR_DIMS];
    int rh_natts;
    size_t xdim=0, ydim=0;
    float scale_factor[3];
    float add_offset[3];
    quint16 fillvalue[3];
    float nominal_satellite_subpoint_lon;
    bool trailfilefound = false;

    ushort end_position_row;
    ushort end_position_column;
    ushort start_position_row;
    ushort start_position_column;
    ushort total_number_of_rows;
    ushort total_number_of_columns;

    double gamma = opts.meteosatgamma;
    double gammafactor = 255 / pow(255, gamma);
    quint16 valgamma;
    quint16 valcontrast;
    int progcounter = 0;

    QVector<int> vec;

    int geoindex = this->getGeoSatelliteIndex();

    QStringList spectrumlist = opts.geosatellites.at(geoindex).spectrumlist;

    emit this->progressCounter(progcounter);

    qDebug() << "Start SegmentListGeostationary::ComposeSegmentImagenetCDFMTGInThreadConcurrent() size = " << this->segmentfilelist.size();
    qDebug() << "Spectrum vector count = " << this->spectrumvector.count() << " kindofimage = " << kindofimage;
    if(this->histogrammethod == CMB_HISTO_NONE_95)
        qDebug() << "histgrammethod = CMB_HISTO_NONE_95";
    else if(this->histogrammethod == CMB_HISTO_NONE_100)
        qDebug() << "histgrammethod = CMB_HISTO_NONE_100";
    else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
        qDebug() << "histgrammethod = CMB_HISTO_EQUALIZE";

    QElapsedTimer timer;
    timer.start();

    // in include file we have the following definition : quint 16 imageptrs->ptrRed[10];
    // ptrBlue and ptrGreen are not used
    // the reading of the netcdf files is sequential
    for(int j = 0; j < this->segmentfilelist.size(); j++)
    {
        if(this->segmentfilelist.at(j).contains("TRAIL"))
        {
            trailfilefound = true;
            ncfile = this->getImagePath() + "/" + this->segmentfilelist.at(j);
            arrayncfile = ncfile.toUtf8();
            pncfile = arrayncfile.constData();

            //qDebug() << "Starting netCDF file " + ncfile;
            retval = nc_open(pncfile, NC_NOWRITE, &ncfileid);
            if(retval != NC_NOERR) qDebug() << "error opening netCDF file " << this->segmentfilelist.at(j);


            retval = nc_inq_ncid(ncfileid, "data", &grp_data);
            if(retval != NC_NOERR) qDebug() << "error opening data group";

            for(int i = 0; i < (kindofimage == "VIS_IR Color" ? 3 : 1); i++)
            {
                QString strspectrum = "data/" + this->spectrumvector.at(i);
                QByteArray ba = strspectrum.toLocal8Bit();
                const char *c_channel = ba.data();
                retval = nc_inq_grp_full_ncid(ncfileid, c_channel, &grp_spectrum);
                if(retval != NC_NOERR) qDebug() << "error opening " << strspectrum;

                if ((retval = nc_inq_varid(grp_spectrum, "number_of_rows", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_spectrum, varid, &total_number_of_rows)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_spectrum, "number_of_columns", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_spectrum, varid, &total_number_of_columns)))
                    ERR(retval);
                imageptrs->mtg_total_number_of_columns[i] = total_number_of_columns;
                imageptrs->mtg_total_number_of_rows[i] = total_number_of_rows;

                qDebug() << QString("Total number of rows = %1, columns = %2").arg(total_number_of_rows).arg(total_number_of_columns);
            }
        }
    }

    progcounter += 10;
    emit this->progressCounter(10);


    for(int j = 0; j < this->segmentfilelist.size(); j++)
    {
        if(this->segmentfilelist.at(j).contains("BODY"))
        {
            ncfile = this->getImagePath() + "/" + this->segmentfilelist.at(j);
            arrayncfile = ncfile.toUtf8();
            pncfile = arrayncfile.constData();

            //qDebug() << "Starting netCDF file " + ncfile;
            int ind = ncfile.indexOf(".nc");
            int findex = ncfile.midRef(ind - 4, 4).toInt();

            vec.append(findex);

            retval = nc_open(pncfile, NC_NOWRITE, &ncfileid);
            if(retval != NC_NOERR) qDebug() << "error opening netCDF file " << this->segmentfilelist.at(j);

            retval = nc_inq(ncfileid, &ndimsp, &nvarsp, &ngattsp, &unlimdimidp);
            if(retval != NC_NOERR) qDebug() << "error nc_inq " << this->segmentfilelist.at(j);

            retval = nc_get_att_double(ncfileid, NC_GLOBAL, "geospatial_lat_min", &geospatial_lat_min);
            if(retval != NC_NOERR) qDebug() << "error nc_get_att_double for geospatial_lat_min";

            retval = nc_get_att_double(ncfileid, NC_GLOBAL, "geospatial_lat_max", &geospatial_lat_max);
            if(retval != NC_NOERR) qDebug() << "error nc_get_att_double for geospatial_lat_max";

            qDebug() << QString("index = %1 geospatial lat min = %2 lat max = %3 nbr of global att = %4").arg(j).arg(geospatial_lat_min)
                        .arg(geospatial_lat_max).arg(ngattsp);

            retval = nc_inq_ncid(ncfileid, "data", &grp_data);
            if(retval != NC_NOERR) qDebug() << "error opening data group";

            for(int i = 0; i < (kindofimage == "VIS_IR Color" ? 3 : 1); i++)
            {
                //qDebug() << "reading radiance from channel " << this->spectrumvector.at(i);

                QString strmeasured = "data/" + this->spectrumvector.at(i) + "/measured";
                QByteArray ba = strmeasured.toLocal8Bit();
                const char *c_channel = ba.data();
                retval = nc_inq_grp_full_ncid(ncfileid, c_channel, &grp_measured);
                if(retval != NC_NOERR) qDebug() << "error opening " << strmeasured;


                if ((retval = nc_inq_varid(grp_measured, "start_position_row", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &start_position_row)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_measured, "start_position_column", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &start_position_column)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_measured, "end_position_row", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &end_position_row)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_measured, "end_position_column", &varid)))
                    ERR(retval);
                retval = nc_get_att_ushort(grp_measured, varid, "_FillValue", &fillvalue[i]);
                if (retval != NC_NOERR) qDebug() << "error reading _FillValue (2605)";
                imageptrs->fillvalue[i] = fillvalue[i];


                if ((retval = nc_get_var_ushort(grp_measured, varid, &end_position_column)))
                    ERR(retval);
                if(retval == 0 && i == 0)
                {
                // qDebug() << QString("start position row = %1 column = %2").arg(start_position_row).arg(start_position_column);
                // qDebug() << QString("end position row = %1 column = %2").arg(end_position_row).arg(end_position_column);
                //qDebug() << QString("j = %1 findex = %2 nbr of rows = %3 column = %4").arg(j).arg(findex).arg(end_position_row - start_position_row + 1).arg(
                //                end_position_column - start_position_column + 1);
                }

                imageptrs->mtg_start_position_row[i][findex - 1] = start_position_row;
                imageptrs->mtg_end_position_row[i][findex - 1] = end_position_row;

                imageptrs->mtg_start_position_column[i][findex -1] = start_position_column;
                imageptrs->mtg_end_position_column[i][findex - 1] = end_position_column;

                imageptrs->mtg_nbr_of_rows[i][findex - 1] = end_position_row - start_position_row + 1;
                imageptrs->mtg_nbr_of_columns[i][findex - 1] = end_position_column - start_position_column + 1;

                imageptrs->ptrMTG[i][findex - 1] = new quint16[imageptrs->mtg_nbr_of_rows[i][findex - 1] * imageptrs->mtg_nbr_of_columns[i][findex - 1]];

                retval = nc_inq_varid(grp_measured, "effective_radiance", &varid);
                if(retval != NC_NOERR) qDebug() << "error opening effective radiance from channel " << strmeasured;
                //qDebug() << QString("FillValue for color %1 = %2").arg(i).arg(fillvalue[i]);


                retval = nc_get_var_ushort(grp_measured, varid, imageptrs->ptrMTG[i][findex - 1]);
                if(retval != NC_NOERR) qDebug() << "error reading effective radiance from channel " << strmeasured << " findex = " << findex << " error = " << retval;
                //qDebug() << "reading to i = " << i << " findex = " << findex - 1;

                //                if(i == 0)
                //                    qDebug() << "copy to ptrRed start_position = " << (start_position_row - 1) * nbr_col;
                //memcpy(imageptrs->ptrRed[i] + nbr_col * (start_position_row - 1), effective_radiance_data, sizeof(quint16)*nbr_rows*nbr_col);


            }
            retval = nc_close(ncfileid);
            if (retval != NC_NOERR) qDebug() << "error closing file " << ncfile;

            emit this->progressCounter(progcounter += 1);

        }

    }

    for(int i = 0; i < 40 ; i++)
    {
       if(nbr_lines_MTG[i] != imageptrs->mtg_nbr_of_rows[0][i])
           qDebug() << "===> imageptrs->mtg_nbr_of_rows != nbr_linesMTG";
    }
    //    auto callbackMethod = std::bind(this->concurrentMinMaxMTG, this, std::placeholders::_1);
    //    QtConcurrent::blockingMap(vec, callbackMethod);

    for(int i = 0; i < vec.length(); i++)
    {
        this->concurrentMinMaxMTG(this, vec[i]);
    }


    emit this->progressCounter(progcounter += 10);

    for(int i = 0; i < 3; i++)
    {
        stat_min[i] = 65535;
        stat_max[i] = 0;
    }

    for(int i = 0; i < (this->kindofimage == "VIS_IR Color" ? 3 : 1); i++) {
        for (int j = 0; j < vec.length(); j++)
        {
            quint16 val = imageptrs->mtg_stat_min[i][vec[j]-1];
            if(val != imageptrs->fillvalue[i])
            {
                if(val < stat_min[i])
                    stat_min[i] = val;
            }
            val = imageptrs->mtg_stat_max[i][vec[j]-1];
            if(val != 0)
            {
                if(val > stat_max[i])
                    stat_max[i] = val;
            }

        }
        qDebug() << QString("stat_min [%1] = %2 stat_max [%3] = %4")
                    .arg(i).arg(stat_min[i]).arg(i).arg(stat_max[i]);

    }


//    auto callbackMethod1 = std::bind(this->concurrentLUTGeoMTG, this, std::placeholders::_1);
//    QtConcurrent::blockingMap(vec, callbackMethod1);

    for(int i = 0; i < vec.length(); i++)
    {
        this->concurrentLUTGeoMTG(this, vec[i]);
    }

    emit this->progressCounter(progcounter += 10);

    for(int colorindex = 0; colorindex < (this->kindofimage == "VIS_IR Color" ? 3 : 1); colorindex++) {
        this->active_pixels[colorindex] = 0;
        for (int index = 0; index < vec.length(); index++)
        {
            this->active_pixels[colorindex] += imageptrs->mtg_active_pixels[colorindex][vec[index]-1];
        }
        qDebug() << QString("active_pixels[%1] = %2").arg(colorindex).arg(this->active_pixels[colorindex]);
    }

    //    for(int i = 0; i < (this->kindofimage == "VIS_IR Color" ? 3 : 1); i++)
    //    {
    //        qDebug() << QString("stat_min[%1] = %2 stat_max[%3] = %4 active_pixels[%5] = %6").arg(i).arg(stat_min[i]).arg(i).arg(stat_max[i]).arg(i).arg(this->active_pixels[i]);
    //    }

    double newscale;
    long histogram[3][4096];

    for(int colorindex = 0; colorindex < (this->kindofimage == "VIS_IR Color" ? 3 : 1); colorindex++) {
        for (int i = 0; i < 4096; i++) {
            histogram[colorindex][i] = 0;
        }
    }

    long long totpixels[3];
    for(int i = 0; i < 3; i++)
        totpixels[i] = 0;

    for(int colorindex = 0; colorindex < (this->kindofimage == "VIS_IR Color" ? 3 : 1); colorindex++) {
        for (int index = 0; index < vec.length(); index++) {
            for (int i = 0; i < 4096; i++) {
                histogram[colorindex][i] += imageptrs->mtg_histogram[colorindex][vec[index]-1][i];
                totpixels[colorindex] += imageptrs->mtg_histogram[colorindex][vec[index]-1][i];
            }
        }
        qDebug() << QString("totpixels[%1] = %2 active_pixels[%3] = %4").arg(colorindex).arg(totpixels[colorindex]).arg(colorindex).arg(this->active_pixels[colorindex]);
    }


    //    for (int j = 0; j < 4096; j++)
    //    {
    //        qDebug() << "histogram " << j << " " << histogram[0][j];
    //    }

    emit this->progressCounter(progcounter += 10);

    for(int colorindex = 0; colorindex < (this->kindofimage == "VIS_IR Color" ? 3 : 1); colorindex++)
    {
        //        newscale = (double)(4095.0 / (double)(this->active_pixels[colorindex] - stat_min[colorindex]));
        newscale = (double)(4095.0 / (double)(totpixels[colorindex] - stat_min[colorindex]));

        qDebug() << QString("newscale = %1 active pixels = %2 11136*11136 = %3").arg(newscale).arg(this->active_pixels[colorindex]).arg(11136*11136);


        unsigned long long sum_ch = 0;
        bool okmin, okmax;

        okmin = false;
        okmax = false;

        imageptrs->minRadianceIndex[colorindex] = 65535;
        imageptrs->maxRadianceIndex[colorindex] = 65535;

        // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
        // min/maxRadianceIndex = index of 95% ( 2.5% of 4096 = 102, 97.5% of 4096 = 3993 )

        for( int i = 0; i < 4096; i++)
        {
            sum_ch += histogram[colorindex][i];
            imageptrs->lut_mtg[colorindex][i] = qRound((sum_ch - stat_min[colorindex]) * newscale);
            imageptrs->lut_mtg[colorindex][i] = (imageptrs->lut_mtg[colorindex][i] > 4095 ? 4095 : imageptrs->lut_mtg[colorindex][i]);
            //        qDebug() << QString("stats_ch[0][%1] = %2 lut_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(imageptrs->lut_ch[0][i]);
            if(imageptrs->lut_mtg[colorindex][i] > 102 && okmin == false)
            {
                okmin = true;
                imageptrs->minRadianceIndex[colorindex] = i;
            }
            if(imageptrs->lut_mtg[colorindex][i] > 3993 && okmax == false)
            {
                okmax = true;
                imageptrs->maxRadianceIndex[colorindex] = i;
            }
        }


        //        for (int j = 0; j < 4096; j++)
        //        {
        //            qDebug() << QString("histogram[%1][%2] = %3 LUT[%4][%5] = %6").arg(colorindex).arg(j).arg(histogram[colorindex][j])
        //                        .arg(colorindex).arg(j).arg(imageptrs->lut_mtg[colorindex][j]);
        //        }


        qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4 active_pixels = %5")
                    .arg(colorindex).arg(imageptrs->minRadianceIndex[colorindex]).arg(colorindex).arg(imageptrs->maxRadianceIndex[colorindex])
                    .arg(this->active_pixels[colorindex]);

    }

    imageptrs->InitializeImageGeostationary(imageptrs->mtg_total_number_of_columns[0], imageptrs->mtg_total_number_of_rows[0]);

    this->COFF = imageptrs->mtg_total_number_of_columns[0] == 11136 ? opts.geosatellites.at(geoindex).coffhrv : opts.geosatellites.at(geoindex).coff;
    this->LOFF = imageptrs->mtg_total_number_of_columns[0] == 11136 ? opts.geosatellites.at(geoindex).loffhrv : opts.geosatellites.at(geoindex).loff;
    this->CFAC = imageptrs->mtg_total_number_of_columns[0] == 11136 ? opts.geosatellites.at(geoindex).cfachrv : opts.geosatellites.at(geoindex).cfac;
    this->LFAC = imageptrs->mtg_total_number_of_columns[0] == 11136 ? opts.geosatellites.at(geoindex).lfachrv : opts.geosatellites.at(geoindex).lfac;

    emit this->progressCounter(progcounter += 10);

    qDebug() << "progressCounter = " << progcounter;

    this->SetupContrastStretch( 0, 0, 1023, 255);

    auto callbackMethod2 = std::bind(this->concurrentImageMTG, this, std::placeholders::_1);
    QtConcurrent::blockingMap(vec, callbackMethod2);

//    for(int i = 0; i < vec.length(); i++)
//    {
//        this->concurrentImageMTG(this, vec[i]);
//    }

    for(int i = 0; i < (kindofimage == "VIS_IR Color" ? 3 : 1); i++)
    {
        for(int j = 0; j < vec.length(); j++)
        {
            if(imageptrs->ptrMTG[i][vec[j]-1] != NULL)
            {
                delete [] imageptrs->ptrMTG[i][vec[j]-1];
                imageptrs->ptrMTG[i][vec[j]-1] = NULL;
            }
        }
    }

    qDebug() << "===> The image process for concurrent took " << timer.elapsed() << "milliseconds";

    emit signalcomposefinished(kindofimage, geoindex);
    emit this->progressCounter(100);

    return;
}

void SegmentListGeostationary::ComposeSegmentImagenetCDFMTGInThreadConcurrent()
{

    QString ncfile;
    QByteArray arrayncfile;
    const char* pncfile;
    int ncfileid;
    int grp_data;
    int grp_channel;
    int grp_measured;
    int grp_spectrum;

    int retval;
    int varid;

    float max_radiance_value_of_valid_pixels[3];
    float mean_radiance_value_of_valid_pixels[3];
    float min_radiance_value_of_valid_pixels[3];

    emit this->progressCounter(10);

    nc_type rh_type;
    int rh_ndims;
    int  rh_dimids[NC_MAX_VAR_DIMS];
    int rh_natts;
    size_t xdim=0, ydim=0;
    float scale_factor[3];
    float add_offset[3];
    quint16 fillvalue[3];
    float nominal_satellite_subpoint_lon;
    bool trailfilefound = false;

    ushort end_position_row;
    ushort end_position_column;
    ushort start_position_row;
    ushort start_position_column;
    ushort total_number_of_rows;
    ushort total_number_of_columns;

    double gamma = opts.meteosatgamma;
    double gammafactor = 255 / pow(255, gamma);
    quint16 valgamma;
    quint16 valcontrast;
    int progcounter = 0;


    int geoindex = this->getGeoSatelliteIndex();

    QStringList spectrumlist = opts.geosatellites.at(geoindex).spectrumlist;

    emit this->progressCounter(progcounter);

    qDebug() << "Start SegmentListGeostationary::ComposeSegmentImagenetCDFMTGInThreadConcurrent() size = " << this->segmentfilelist.size();
    qDebug() << "Spectrum vector count = " << this->spectrumvector.count() << " kindofimage = " << kindofimage;
    if(this->histogrammethod == CMB_HISTO_NONE_95)
        qDebug() << "histgrammethod = CMB_HISTO_NONE_95";
    else if(this->histogrammethod == CMB_HISTO_NONE_100)
        qDebug() << "histgrammethod = CMB_HISTO_NONE_100";
    else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
        qDebug() << "histgrammethod = CMB_HISTO_EQUALIZE";

    QElapsedTimer timer;
    timer.start();

    QVector<int> vec;

    // in include file we have the following definition : quint 16 imageptrs->ptrRed[10];
    // ptrBlue and ptrGreen are not used
    // the reading of the netcdf files is sequential
    for(int j = 0; j < this->segmentfilelist.size(); j++)
    {
        if(this->segmentfilelist.at(j).contains("TRAIL"))
        {
            trailfilefound = true;
            ncfile = this->getImagePath() + "/" + this->segmentfilelist.at(j);
            arrayncfile = ncfile.toUtf8();
            pncfile = arrayncfile.constData();

            //qDebug() << "Starting netCDF file " + ncfile;
            retval = nc_open(pncfile, NC_NOWRITE, &ncfileid);
            if(retval != NC_NOERR) qDebug() << "error opening netCDF file " << this->segmentfilelist.at(j);

            retval = nc_inq_ncid(ncfileid, "data", &grp_data);
            if(retval != NC_NOERR) qDebug() << "error opening data group";

            for(int i = 0; i < (kindofimage == "VIS_IR Color" ? 3 : 1); i++)
            {
                QString strspectrum = "data/" + this->spectrumvector.at(i);
                QByteArray ba = strspectrum.toLocal8Bit();
                const char *c_channel = ba.data();
                retval = nc_inq_grp_full_ncid(ncfileid, c_channel, &grp_spectrum);
                if(retval != NC_NOERR) qDebug() << "error opening " << strspectrum;

                if ((retval = nc_inq_varid(grp_spectrum, "number_of_rows", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_spectrum, varid, &total_number_of_rows)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_spectrum, "number_of_columns", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_spectrum, varid, &total_number_of_columns)))
                    ERR(retval);
                imageptrs->mtg_total_number_of_columns[i] = total_number_of_columns;
                imageptrs->mtg_total_number_of_rows[i] = total_number_of_rows;

                qDebug() << QString("Total number of rows = %1, columns = %2").arg(total_number_of_rows).arg(total_number_of_columns);
            }
        }
    }

    progcounter += 10;
    emit this->progressCounter(10);


    for(int j = 0; j < this->segmentfilelist.size(); j++)
    {
        if(this->segmentfilelist.at(j).contains("BODY"))
        {
            ncfile = this->getImagePath() + "/" + this->segmentfilelist.at(j);
            arrayncfile = ncfile.toUtf8();
            pncfile = arrayncfile.constData();

            //qDebug() << "Starting netCDF file " + ncfile;
            int ind = ncfile.indexOf(".nc");
            int findex = ncfile.midRef(ind - 4, 4).toInt();

            vec.append(findex);

            retval = nc_open(pncfile, NC_NOWRITE, &ncfileid);
            if(retval != NC_NOERR) qDebug() << "error opening netCDF file " << this->segmentfilelist.at(j);

            retval = nc_inq_ncid(ncfileid, "data", &grp_data);
            if(retval != NC_NOERR) qDebug() << "error opening data group";

            for(int i = 0; i < (kindofimage == "VIS_IR Color" ? 3 : 1); i++)
            {
                //qDebug() << "reading radiance from channel " << this->spectrumvector.at(i);

                QString strmeasured = "data/" + this->spectrumvector.at(i) + "/measured";
                QByteArray ba = strmeasured.toLocal8Bit();
                const char *c_channel = ba.data();
                retval = nc_inq_grp_full_ncid(ncfileid, c_channel, &grp_measured);
                if(retval != NC_NOERR) qDebug() << "error opening " << strmeasured;


                if ((retval = nc_inq_varid(grp_measured, "start_position_row", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &start_position_row)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_measured, "start_position_column", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &start_position_column)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_measured, "end_position_row", &varid)))
                    ERR(retval);
                if ((retval = nc_get_var_ushort(grp_measured, varid, &end_position_row)))
                    ERR(retval);
                if ((retval = nc_inq_varid(grp_measured, "end_position_column", &varid)))
                    ERR(retval);
                retval = nc_get_att_ushort(grp_measured, varid, "_FillValue", &fillvalue[i]);
                if (retval != NC_NOERR) qDebug() << "error reading _FillValue (2605)";
                imageptrs->fillvalue[i] = fillvalue[i];


                if ((retval = nc_get_var_ushort(grp_measured, varid, &end_position_column)))
                    ERR(retval);
                //if(retval == 0 && i == 0)
                //{
                // qDebug() << QString("start position row = %1 column = %2").arg(start_position_row).arg(start_position_column);
                // qDebug() << QString("end position row = %1 column = %2").arg(end_position_row).arg(end_position_column);
                //qDebug() << QString("j = %1 findex = %2 nbr of rows = %3 column = %4").arg(j).arg(findex).arg(end_position_row - start_position_row + 1).arg(
                //                end_position_column - start_position_column + 1);
                //}

                imageptrs->mtg_start_position_row[i][findex - 1] = start_position_row;
                imageptrs->mtg_end_position_row[i][findex - 1] = end_position_row;

                imageptrs->mtg_start_position_column[i][findex -1] = start_position_column;
                imageptrs->mtg_end_position_column[i][findex - 1] = end_position_column;

                imageptrs->mtg_nbr_of_rows[i][findex - 1] = end_position_row - start_position_row + 1;
                imageptrs->mtg_nbr_of_columns[i][findex - 1] = end_position_column - start_position_column + 1;

                imageptrs->ptrMTG[i][findex - 1] = new quint16[imageptrs->mtg_nbr_of_rows[i][findex - 1] * imageptrs->mtg_nbr_of_columns[i][findex - 1]];

                retval = nc_inq_varid(grp_measured, "effective_radiance", &varid);
                if(retval != NC_NOERR) qDebug() << "error opening effective radiance from channel " << strmeasured;
                //qDebug() << QString("FillValue for color %1 = %2").arg(i).arg(fillvalue[i]);


                retval = nc_get_var_ushort(grp_measured, varid, imageptrs->ptrMTG[i][findex - 1]);
                if(retval != NC_NOERR) qDebug() << "error reading effective radiance from channel " << strmeasured << " findex = " << findex << " error = " << retval;
                //qDebug() << "reading to i = " << i << " findex = " << findex - 1;

                //                if(i == 0)
                //                    qDebug() << "copy to ptrRed start_position = " << (start_position_row - 1) * nbr_col;
                //memcpy(imageptrs->ptrRed[i] + nbr_col * (start_position_row - 1), effective_radiance_data, sizeof(quint16)*nbr_rows*nbr_col);


            }
            retval = nc_close(ncfileid);
            if (retval != NC_NOERR) qDebug() << "error closing file " << ncfile;

            emit this->progressCounter(progcounter += 1);

        }

    }

    //    for( int i = 0; i < vec.length(); i++)
    //        qDebug() << QString("vec[%1] = %2").arg(i).arg(vec[i]);

    auto callbackMethod = std::bind(this->concurrentMinMaxMTG, this, std::placeholders::_1);
    QtConcurrent::blockingMap(vec, callbackMethod);

    emit this->progressCounter(progcounter += 10);

    for(int i = 0; i < 3; i++)
    {
        stat_min[i] = 65535;
        stat_max[i] = 0;
    }

    for(int i = 0; i < (this->kindofimage == "VIS_IR Color" ? 3 : 1); i++) {
        for (int j = 0; j < vec.length(); j++)
        {
            if(vec[j] < 41)
            {
                quint16 val = imageptrs->mtg_stat_min[i][vec[j]];
                if(val != imageptrs->fillvalue[i])
                {
                    if(val < stat_min[i])
                        stat_min[i] = val;
                }
                val = imageptrs->mtg_stat_max[i][vec[j]];
                if(val != 0)
                {
                    if(val > stat_max[i])
                        stat_max[i] = val;
                }
            }
        }
        qDebug() << QString("stat_min [%1] = %2 stat_max [%3] = %4")
                    .arg(i).arg(stat_min[i]).arg(i).arg(stat_max[i]);

    }


    auto callbackMethod1 = std::bind(this->concurrentLUTGeoMTG, this, std::placeholders::_1);
    QtConcurrent::blockingMap(vec, callbackMethod1);

    emit this->progressCounter(progcounter += 10);

    for(int colorindex = 0; colorindex < (this->kindofimage == "VIS_IR Color" ? 3 : 1); colorindex++) {
        this->active_pixels[colorindex] = 0;
        for (int index = 0; index < 40; index++)
        {
            this->active_pixels[colorindex] += imageptrs->mtg_active_pixels[colorindex][index];
        }
        qDebug() << QString("active_pixels[%1] = %2").arg(colorindex).arg(this->active_pixels[colorindex]);
    }

    //    for(int i = 0; i < (this->kindofimage == "VIS_IR Color" ? 3 : 1); i++)
    //    {
    //        qDebug() << QString("stat_min[%1] = %2 stat_max[%3] = %4 active_pixels[%5] = %6").arg(i).arg(stat_min[i]).arg(i).arg(stat_max[i]).arg(i).arg(this->active_pixels[i]);
    //    }

    double newscale;
    long histogram[3][4096];

    for(int colorindex = 0; colorindex < (this->kindofimage == "VIS_IR Color" ? 3 : 1); colorindex++) {
        for (int i = 0; i < 4096; i++) {
            histogram[colorindex][i] = 0;
        }
    }

    long long totpixels[3];
    for(int i = 0; i < 3; i++)
        totpixels[i] = 0;

    for(int colorindex = 0; colorindex < (this->kindofimage == "VIS_IR Color" ? 3 : 1); colorindex++) {
        for (int index = 0; index < 40; index++) {
            for (int i = 0; i < 4096; i++) {
                histogram[colorindex][i] += imageptrs->mtg_histogram[colorindex][index][i];
                totpixels[colorindex] += imageptrs->mtg_histogram[colorindex][index][i];
            }
        }
        qDebug() << QString("totpixels[%1] = %2 active_pixels[%3] = %4").arg(colorindex).arg(totpixels[colorindex]).arg(colorindex).arg(this->active_pixels[colorindex]);
    }


    //    for (int j = 0; j < 4096; j++)
    //    {
    //        qDebug() << "histogram " << j << " " << histogram[0][j];
    //    }

    emit this->progressCounter(progcounter += 10);


    for(int colorindex = 0; colorindex < (this->kindofimage == "VIS_IR Color" ? 3 : 1); colorindex++)
    {
        //        newscale = (double)(4095.0 / (double)(this->active_pixels[colorindex] - stat_min[colorindex]));
        newscale = (double)(4095.0 / (double)(totpixels[colorindex] - stat_min[colorindex]));

        qDebug() << QString("newscale = %1 active pixels = %2 11136*11136 = %3").arg(newscale).arg(this->active_pixels[colorindex]).arg(11136*11136);


        unsigned long long sum_ch = 0;
        bool okmin, okmax;

        okmin = false;
        okmax = false;

        imageptrs->minRadianceIndex[colorindex] = 65535;
        imageptrs->maxRadianceIndex[colorindex] = 65535;

        // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
        // min/maxRadianceIndex = index of 95% ( 2.5% of 4096 = 102, 97.5% of 4096 = 3993 )

        for( int i = 0; i < 4096; i++)
        {
            sum_ch += histogram[colorindex][i];
            imageptrs->lut_mtg[colorindex][i] = qRound((sum_ch - stat_min[colorindex]) * newscale);
            imageptrs->lut_mtg[colorindex][i] = (imageptrs->lut_mtg[colorindex][i] > 4095 ? 4095 : imageptrs->lut_mtg[colorindex][i]);
            //        qDebug() << QString("stats_ch[0][%1] = %2 lut_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(imageptrs->lut_ch[0][i]);
            if(imageptrs->lut_mtg[colorindex][i] > 102 && okmin == false)
            {
                okmin = true;
                imageptrs->minRadianceIndex[colorindex] = i;
            }
            if(imageptrs->lut_mtg[colorindex][i] > 3993 && okmax == false)
            {
                okmax = true;
                imageptrs->maxRadianceIndex[colorindex] = i;
            }
        }


        //        for (int j = 0; j < 4096; j++)
        //        {
        //            qDebug() << QString("histogram[%1][%2] = %3 LUT[%4][%5] = %6").arg(colorindex).arg(j).arg(histogram[colorindex][j])
        //                        .arg(colorindex).arg(j).arg(imageptrs->lut_mtg[colorindex][j]);
        //        }


        qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4 active_pixels = %5")
                    .arg(colorindex).arg(imageptrs->minRadianceIndex[colorindex]).arg(colorindex).arg(imageptrs->maxRadianceIndex[colorindex])
                    .arg(this->active_pixels[colorindex]);

    }

    imageptrs->InitializeImageGeostationary(imageptrs->mtg_total_number_of_columns[0], imageptrs->mtg_total_number_of_rows[0]);

    this->COFF = imageptrs->mtg_total_number_of_columns[0] == 11136 ? opts.geosatellites.at(geoindex).coffhrv : opts.geosatellites.at(geoindex).coff;
    this->LOFF = imageptrs->mtg_total_number_of_columns[0] == 11136 ? opts.geosatellites.at(geoindex).loffhrv : opts.geosatellites.at(geoindex).loff;
    this->CFAC = imageptrs->mtg_total_number_of_columns[0] == 11136 ? opts.geosatellites.at(geoindex).cfachrv : opts.geosatellites.at(geoindex).cfac;
    this->LFAC = imageptrs->mtg_total_number_of_columns[0] == 11136 ? opts.geosatellites.at(geoindex).lfachrv : opts.geosatellites.at(geoindex).lfac;

    progcounter += 10;
    emit this->progressCounter(progcounter);

    this->SetupContrastStretch( 0, 0, 1023, 255);

    auto callbackMethod2 = std::bind(this->concurrentImageMTG, this, std::placeholders::_1);
    QtConcurrent::blockingMap(vec, callbackMethod2);

    for(int i = 0; i < (kindofimage == "VIS_IR Color" ? 3 : 1); i++)
    {
        for(int j = 0; j < 40; j++)
        {
            if(imageptrs->ptrMTG[i][j] != NULL)
            {
                delete [] imageptrs->ptrMTG[i][j];
                imageptrs->ptrMTG[i][j] = NULL;
            }
        }
    }

    qDebug() << "===> The image process for concurrent took " << timer.elapsed() << "milliseconds";

    emit signalcomposefinished(kindofimage, geoindex);
    emit this->progressCounter(100);

    return;
}


int SegmentListGeostationary::concurrentMinMaxMTG(SegmentListGeostationary *sm, const int &index)
{

    //qDebug() << "concurrentMinMaxMTG for index = " << index;

    for(int j = 0; j < (sm->kindofimage == "VIS_IR Color" ? 3 : 1); j++)
    {
        sm->CalculateMinMaxMTG(j, index);
    }


    return(index);

}

int SegmentListGeostationary::concurrentLUTGeoMTG(SegmentListGeostationary *sm, const int &index)
{
    for(int j = 0; j < (sm->kindofimage == "VIS_IR Color" ? 3 : 1); j++)
    {
        sm->CalculateLUTGeoMTGConcurrent(j, index);
    }

    return(index);
}

int SegmentListGeostationary::concurrentImageMTG(SegmentListGeostationary *sm, const int &index)
{
    sm->CalculateImageMTGConcurrent(index);
    return(index);
}


void SegmentListGeostationary::CalculateLUTGeo(int colorindex)
{
    qDebug() << "start SegmentListGeostationary::CalculateLUTGeo()";
    long histogram[1024];

    for (int j = 0; j < 1024; j++)
    {
        histogram[j] = 0;
    }

    quint16 pixel;
    for (int line = 0; line < number_of_lines; line++)
    {
        for (int pixelx = 0; pixelx < number_of_columns; pixelx++)
        {
            if(colorindex == 0)
                pixel = *(imageptrs->ptrRed[0] + line * number_of_columns + pixelx);
            else if(colorindex == 1)
                pixel = *(imageptrs->ptrGreen[0] + line * number_of_columns + pixelx);
            else if(colorindex == 2)
                pixel = *(imageptrs->ptrBlue[0] + line * number_of_columns + pixelx);
            else
                pixel = 0;


            quint16 indexout = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - this->stat_min[colorindex])/(float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 1023);
            histogram[indexout]++;
        }
    }


    //    for(int i = 0; i < 1024; i++)
    //    {
    //        qDebug() << QString("stats_ch[0][%1] = %2 ; stats_norm_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(stats_norm_ch[0][i]);
    //    }


    // float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    double newscale = (double)(1024.0 / this->active_pixels[colorindex]);

    qDebug() << QString("newscale = %1 active pixels = %2").arg(newscale).arg(this->active_pixels[colorindex]);

    unsigned long long sum_ch = 0;

    bool okmin, okmax;

    okmin = false;
    okmax = false;

    // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
    for( int i = 0; i < 1024; i++)
    {
        sum_ch += histogram[i];
        imageptrs->lut_ch[colorindex][i] = (quint16)((double)sum_ch * newscale);
        imageptrs->lut_ch[colorindex][i] = ( imageptrs->lut_ch[colorindex][i] > 1023 ? 1023 : imageptrs->lut_ch[colorindex][i]);
        if(imageptrs->lut_ch[colorindex][i] > 25 && okmin == false)
        {
            okmin = true;
            imageptrs->minRadianceIndex[colorindex] = i;
        }
        if(imageptrs->lut_ch[colorindex][i] > 997 && okmax == false)
        {
            okmax = true;
            imageptrs->maxRadianceIndex[colorindex] = i;
        }
    }


    //    for(int i = 0; i < 1024; i++)
    //    {
    //        qDebug() << QString("stats_ch[0][%1] = %2 lut_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(imageptrs->lut_ch[0][i]);
    //    }


    qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4").arg(colorindex).arg(imageptrs->minRadianceIndex[colorindex]).arg(colorindex).arg(imageptrs->maxRadianceIndex[colorindex]);
}

void SegmentListGeostationary::equalizeHistogram(quint16* pdata, int width, int height, int max_val = 1023)
{
    int total = width*height;
    int n_bins = max_val + 1;

    // Compute histogram
    std::vector<int> hist(n_bins, 0);
    for (int i = 0; i < total; ++i) {
        hist[pdata[i]]++;
    }

    // Build LUT from cumulative histrogram

    // Find first non-zero bin
    int i = 0;
    while (!hist[i]) ++i;

    if (hist[i] == total) {
        for (int j = 0; j < total; ++j) {
            pdata[j] = i;
        }
        return;
    }

    // Compute scale
    float scale = (n_bins - 1.f) / (total - hist[i]);

    // Initialize lut
    std::vector<int> lut(n_bins, 0);
    i++;

    int sum = 0;
    for (; i < hist.size(); ++i) {
        sum += hist[i];
        // the value is saturated in range [0, max_val]
        lut[i] = std::max(0, std::min(int(round(sum * scale)), max_val));
    }



    // Apply equalization
    for (int i = 0; i < total; ++i) {
        pdata[i] = lut[pdata[i]];
    }
}

void SegmentListGeostationary::equalizeHistogram(quint16* pdata, int width, int height, int colorindex, quint16 fillvalue, int max_val)
{
    int total = width*height;
    int n_bins = max_val + 1;
    int active_pixels = 0;
    // Compute histogram
    std::vector<int> hist(n_bins, 0);
    for (int i = 0; i < total; ++i) {
        if(pdata[i] != fillvalue)
        {
            hist[pdata[i]]++;
            active_pixels++;
        }
    }

    // Build LUT from cumulative histrogram

    // Find first non-zero bin
    int i = 0;
    while (!hist[i]) ++i;

    if (hist[i] == total) {
        for (int j = 0; j < total; ++j) {
            pdata[j] = i;
        }
        return;
    }

    // Compute scale
    float scale = (n_bins - 1.f) / (active_pixels - hist[i]);

    i++;

    bool okmin = false;
    bool okmax = false;
    int sum = 0;
    for (; i < hist.size(); ++i) {
        sum += hist[i];
        // the value is saturated in range [0, max_val]
        imageptrs->lut_ch[colorindex][i] = std::max(0, std::min(int(round(sum * scale)), max_val));
        if(imageptrs->lut_ch[colorindex][i] > 25 && okmin == false)
        {
            okmin = true;
            imageptrs->minRadianceIndex[colorindex] = i;
        }
        if(imageptrs->lut_ch[colorindex][i] > 997 && okmax == false)
        {
            okmax = true;
            imageptrs->maxRadianceIndex[colorindex] = i;
        }

    }

    qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4").arg(colorindex).arg(imageptrs->minRadianceIndex[colorindex]).arg(colorindex).arg(imageptrs->maxRadianceIndex[colorindex]);

    //    for(int i = 0; i < 1024; i++)
    //    {
    //        qDebug() << QString("lut[%1] = %2").arg(i).arg(imageptrs->lut_ch[colorindex][i]);
    //    }


    // Apply equalization
    //    for (int i = 0; i < total; ++i) {
    //        pdata[i] = lut[pdata[i]];
    //    }
}

void SegmentListGeostationary::CalculateLUTGeo(int colorindex, quint16 *ptr, quint16 fillvalue)
{
    qDebug() << "start SegmentListGeostationary::CalculateLUTGeo()";
    long histogram[1024];

    for (int j = 0; j < 1024; j++)
    {
        histogram[j] = 0;
    }

    quint16 pixel;
    for (int line = 0; line < opts.geosatellites[geoindex].imageheight; line++)
    {
        for (int pixelx = 0; pixelx < opts.geosatellites[geoindex].imagewidth; pixelx++)
        {
            pixel = ptr[line * number_of_columns + pixelx];
            if(pixel != fillvalue && pixel > 25)
            {
                quint16 indexout = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - this->stat_min[colorindex])/(float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 1023);
                //quint16 indexout = pixel;
                histogram[indexout]++;
            }
        }
    }

    //    if(colorindex == 0)
    //    {
    //        for (int j = 0; j < 1024; j++)
    //        {
    //            qDebug() << QString("histogram[%1] = %2").arg(j).arg(histogram[j]);
    //        }
    //    }

    //double newscale = (double)(1024.0 / (3712*3712));    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    double newscale = (double)(1024.0 / (double)this->active_pixels[colorindex]);

    qDebug() << QString("newscale = %1 active pixels = %2").arg(newscale).arg(this->active_pixels[colorindex]);

    unsigned long long sum_ch = 0;
    bool okmin, okmax;

    okmin = false;
    okmax = false;

    // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
    for( int i = 0; i < 1024; i++)
    {
        sum_ch += histogram[i];
        //imageptrs->lut_ch[colorindex][i] = (quint16)((double)sum_ch * newscale);
        imageptrs->lut_ch[colorindex][i] = (quint16)((double)sum_ch * newscale);
        imageptrs->lut_ch[colorindex][i] = (imageptrs->lut_ch[colorindex][i] > 1023 ? 1023 : imageptrs->lut_ch[colorindex][i]);
        //        qDebug() << QString("stats_ch[0][%1] = %2 lut_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(imageptrs->lut_ch[0][i]);
        if(imageptrs->lut_ch[colorindex][i] > 25 && okmin == false)
        {
            okmin = true;
            imageptrs->minRadianceIndex[colorindex] = i;
        }
        if(imageptrs->lut_ch[colorindex][i] > 997 && okmax == false)
        {
            okmax = true;
            imageptrs->maxRadianceIndex[colorindex] = i;
        }
        if(colorindex == 0)
            qDebug() << QString("lut_ch %1 = %2").arg(i).arg(imageptrs->lut_ch[colorindex][i]);
    }

    if(okmax == false)
    {
        for(int i = 1023; i > -1; i--)
        {
            if(imageptrs->lut_ch[colorindex][i] > imageptrs->lut_ch[colorindex][i-1])
            {
                imageptrs->maxRadianceIndex[colorindex] = i;
                break;
            }
        }
    }

    qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4").arg(colorindex).arg(imageptrs->minRadianceIndex[colorindex]).arg(colorindex).arg(imageptrs->maxRadianceIndex[colorindex]);
}

void SegmentListGeostationary::CalculateLUTGeoMTG(int colorindex)
{
    qDebug() << "start SegmentListGeostationary::CalculateLUTGeoMTG()";
    long histogram[1024];

    for (int j = 0; j < 1024; j++)
    {
        histogram[j] = 0;
    }

    quint16 pixel;
    for (int line = 0; line < imageptrs->mtg_total_number_of_rows[0]; line++)
    {
        for (int pixelx = 0; pixelx < imageptrs->mtg_total_number_of_columns[0]; pixelx++)
        {
            if(colorindex == 0)
                pixel = *(imageptrs->ptrRed[0] + line * imageptrs->mtg_total_number_of_columns[0] + pixelx);
            else if(colorindex == 1)
                pixel = *(imageptrs->ptrRed[1] + line * imageptrs->mtg_total_number_of_columns[0] + pixelx);
            else if(colorindex == 2)
                pixel = *(imageptrs->ptrRed[2] + line * imageptrs->mtg_total_number_of_columns[0] + pixelx);
            else
                pixel = imageptrs->fillvalue[colorindex];

            if(pixel != imageptrs->fillvalue[colorindex])
            {
                quint16 indexout = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - this->stat_min[colorindex])/(float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 1023);
                //quint16 indexout = pixel;
                histogram[indexout]++;
            }
        }
    }

    //    for (int j = 0; j < 4096; j=j+20)
    //    {
    //        qDebug() << "histogram " << j << " " << histogram[j];
    //    }


    // float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    double newscale = (double)(1024.0 / (double)this->active_pixels[colorindex]);

    qDebug() << QString("newscale = %1 active pixels = %2").arg(newscale).arg(this->active_pixels[colorindex]);

    unsigned long long sum_ch = 0;
    bool okmin, okmax;

    okmin = false;
    okmax = false;

    // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
    // min/maxRadianceIndex = index of 95% ( 2.5% of 4096 = 102, 97.5% of 4096 = 3993 )
    for( int i = 0; i < 1024; i++)
    {
        sum_ch += histogram[i];
        imageptrs->lut_ch[colorindex][i] = (quint16)((double)sum_ch * newscale);
        imageptrs->lut_ch[colorindex][i] = ( imageptrs->lut_ch[colorindex][i] > 1023 ? 1023 : imageptrs->lut_ch[colorindex][i]);
        if(imageptrs->lut_ch[colorindex][i] > 25 && okmin == false)
        {
            okmin = true;
            imageptrs->minRadianceIndex[colorindex] = i;
        }
        if(imageptrs->lut_ch[colorindex][i] > 997 && okmax == false)
        {
            okmax = true;
            imageptrs->maxRadianceIndex[colorindex] = i;
        }
    }

    qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4").arg(colorindex).arg(imageptrs->minRadianceIndex[colorindex]).arg(colorindex).arg(imageptrs->maxRadianceIndex[colorindex]);
}



void SegmentListGeostationary::CalculateLUTGeoMTGConcurrent(int colorindex, int index)
{

    //qDebug() << "start SegmentListGeostationary::CalculateLUTGeoMTGConcurrent() colorindex = " << colorindex << " index = " << index;

    //assert(colorindex == 0);

    quint16 *ptr = imageptrs->ptrMTG[colorindex][index-1];

    for (int j = 0; j < 4096; j++)
    {
        imageptrs->mtg_histogram[colorindex][index-1][j] = 0;
    }

    quint16 pixel;
    for (int line = 0; line < imageptrs->mtg_nbr_of_rows[colorindex][index-1]; line++)
    {
        for (int pixelx = 0; pixelx < imageptrs->mtg_nbr_of_columns[colorindex][index-1]; pixelx++)
        {
            pixel = ptr[line * imageptrs->mtg_nbr_of_columns[colorindex][index-1] + pixelx];
            if(pixel != imageptrs->fillvalue[colorindex])
            {
                //quint16 indexout = (quint16)qMin(qMax(qRound(4095.0 * (float)(pixel - this->stat_min[colorindex]) /
                //                      (float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 4095);
                quint16 indexout = qMin(pixel, (quint16)4095);
                imageptrs->mtg_histogram[colorindex][index-1][indexout]++;
            }
        }
    }

    return;

}

void SegmentListGeostationary::CalculateImageMTGConcurrent(int index)
{
    quint16 pixel[3];
    quint16 indexoutpixel[3];
    quint8 pixelout[3];
    QRgb *row_col;
    QImage *im;
    quint16 valgamma;
    quint8 valcontrast;
    double gamma = opts.meteosatgamma;
    double gammafactor = 1023 / pow(1023, gamma);

    im = imageptrs->ptrimageGeostationary;

    int linelocal = 0;

    for(int line = imageptrs->mtg_start_position_row[0][index-1] - 1; line < imageptrs->mtg_end_position_row[0][index-1]; line++)
    {
        row_col = (QRgb*)im->scanLine(imageptrs->mtg_total_number_of_rows[0] - 1 - line);

        for (int pixelx = imageptrs->mtg_start_position_column[0][index-1] - 1; pixelx < imageptrs->mtg_end_position_column[0][index-1]; pixelx++)
        {
            for(int colorindex = 0; colorindex < (this->kindofimage == "VIS_IR Color" ? 3 : 1); colorindex++)
            {
                pixel[colorindex] = *(imageptrs->ptrMTG[colorindex][index-1] + ((imageptrs->mtg_nbr_of_columns[colorindex][index-1]) * linelocal) + pixelx);

                if(this->histogrammethod == CMB_HISTO_NONE_95)
                {
                    if(pixel[colorindex] != imageptrs->fillvalue[colorindex])
                    {
                        indexoutpixel[colorindex] = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel[colorindex] - imageptrs->minRadianceIndex[colorindex] ) / (float)(imageptrs->maxRadianceIndex[colorindex] - imageptrs->minRadianceIndex[colorindex])), 0), 1023);
                        valgamma = pow( indexoutpixel[colorindex], gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        pixelout[colorindex] = (valcontrast);
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_NONE_100)
                {
                    if(pixel[colorindex] != imageptrs->fillvalue[colorindex])
                    {
                        indexoutpixel[colorindex] = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel[colorindex] - this->stat_min[colorindex] ) / (float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 1023);
                        valgamma = pow( indexoutpixel[colorindex], gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        pixelout[colorindex] = (valcontrast);
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( pixel[colorindex] != imageptrs->fillvalue[colorindex])
                    {
                        quint16 val = (quint16)qMin(qMax(qRound(4095.0 * (float)(pixel[colorindex] - this->stat_min[colorindex] ) / (float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 1023);
                        indexoutpixel[colorindex] = (quint16)(qMin(qMax((int)imageptrs->lut_mtg[colorindex][val], 0), 4095));
                        valgamma = pow( indexoutpixel[colorindex]/4, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        pixelout[colorindex] = (valcontrast);
                    }
                }
            }

            if(this->kindofimage == "VIS_IR")
            {
                if(pixel[0] != imageptrs->fillvalue[0])
                {
                    pixelout[0] = quint16(this->inversevector[0] ? (255 - pixelout[0]) : pixelout[0]);
                    pixelout[1] = pixelout[0];
                    pixelout[2] = pixelout[0];
                    row_col[pixelx] = qRgb(pixelout[0], pixelout[1], pixelout[2]);
                }
            }
            else if(this->kindofimage == "VIS_IR Color")
            {
                if(pixel[0] != imageptrs->fillvalue[0])
                {
                    pixelout[0] = quint16(this->inversevector[0] ? (255 - pixelout[0]) : pixelout[0]);
                    pixelout[1] = quint16(this->inversevector[1] ? (255 - pixelout[1]) : pixelout[1]);
                    pixelout[2] = quint16(this->inversevector[2] ? (255 - pixelout[2]) : pixelout[2]);
                    row_col[pixelx] = qRgb(pixelout[0], pixelout[1], pixelout[2]);
                }
            }
        }

        linelocal++;
    }
}

void SegmentListGeostationary::CalculateLUTGeoMTG256(int colorindex)
{
    qDebug() << "start SegmentListGeostationary::CalculateLUTGeoMTG()";
    long histogram[256];

    for (int j = 0; j < 256; j++)
    {
        histogram[j] = 0;
    }

    quint16 pixel;
    for (int line = 0; line < imageptrs->mtg_total_number_of_rows[0]; line++)
    {
        for (int pixelx = 0; pixelx < imageptrs->mtg_total_number_of_columns[0]; pixelx++)
        {
            if(colorindex == 0)
                pixel = *(imageptrs->ptrRed[0] + line * imageptrs->mtg_total_number_of_columns[0] + pixelx);
            else if(colorindex == 1)
                pixel = *(imageptrs->ptrRed[1] + line * imageptrs->mtg_total_number_of_columns[0] + pixelx);
            else if(colorindex == 2)
                pixel = *(imageptrs->ptrRed[2] + line * imageptrs->mtg_total_number_of_columns[0] + pixelx);
            else
                pixel = imageptrs->fillvalue[colorindex];

            if(pixel != imageptrs->fillvalue[colorindex])
            {
                //quint16 indexout = (quint16)qMin(qMax(qRound(255.0 * (float)(pixel - this->stat_min[colorindex])/(float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 255);
                quint16 indexout = (quint16)(pixel/16);
                histogram[indexout]++;
            }
        }
    }


    // float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    double newscale = (double)(256.0 / (double)this->active_pixels[colorindex]);

    qDebug() << QString("newscale = %1 active pixels = %2").arg(newscale).arg(this->active_pixels[colorindex]);

    unsigned long long sum_ch = 0;
    bool okmin, okmax;

    okmin = false;
    okmax = false;

    // min/maxRadianceIndex = index of 95% ( 2.5% of 256 = 102, 97.5% of 256 = 3993 )
    for( int i = 0; i < 256; i++)
    {
        sum_ch += histogram[i];
        imageptrs->lut_ch[colorindex][i] = (quint16)((double)sum_ch * newscale);
        imageptrs->lut_ch[colorindex][i] = ( imageptrs->lut_ch[colorindex][i] > 255 ? 255 : imageptrs->lut_ch[colorindex][i]);
        if(imageptrs->lut_ch[colorindex][i] > 6 && okmin == false)
        {
            okmin = true;
            imageptrs->minRadianceIndex[colorindex] = i;
        }
        if(imageptrs->lut_ch[colorindex][i] > 250 && okmax == false)
        {
            okmax = true;
            imageptrs->maxRadianceIndex[colorindex] = i;
        }
    }

    for( int i = 0; i < 256; i++)
    {
        qDebug() << QString("lut_ch[%1][%2] = %3").arg(colorindex).arg(i).arg(imageptrs->lut_ch[colorindex][i]);
    }

    qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4").arg(colorindex).arg(imageptrs->minRadianceIndex[colorindex]).arg(colorindex).arg(imageptrs->maxRadianceIndex[colorindex]);
}


void SegmentListGeostationary::ComposeVISIR()
{

    qDebug() << "==========start ComposeVISIR geoindex = " << geoindex << " ==================";

    int width = opts.geosatellites[geoindex].imagewidth;
    int height = opts.geosatellites[geoindex].imageheight;
    size_t nbrpix = width*height;
    int nbroflinespersegment = opts.geosatellites[geoindex].segmentlength;

    quint16 *pixelsRed = NULL;
    quint16 *pixelsGreen = NULL;
    quint16 *pixelsBlue = NULL;

    pixelsRed = new quint16[nbrpix];
    if(kindofimage == "VIS_IR Color")
    {
        pixelsGreen = new quint16[nbrpix];
        pixelsBlue = new quint16[nbrpix];
    }

    for( int i = (bisRSS ? 5 : 0); i < opts.geosatellites[geoindex].maxsegments + (bisRSS ? 5 : 0); i++)
    {
        if(imageptrs->ptrRed[i] != NULL)
            memcpy(pixelsRed + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrRed[i], nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
        else
            memset(pixelsRed + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, 0, nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));

    }

    if(kindofimage == "VIS_IR Color")
    {
        for( int i = (bisRSS ? 5 : 0); i < opts.geosatellites[geoindex].maxsegments + (bisRSS ? 5 : 0); i++)
        {
            if(imageptrs->ptrGreen[i] != NULL)
                memcpy(pixelsGreen + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrGreen[i], nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
            else
                memset(pixelsGreen + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, 0, nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
        }
        for( int i = (bisRSS ? 5 : 0); i < opts.geosatellites[geoindex].maxsegments + (bisRSS ? 5 : 0); i++)
        {
            if(imageptrs->ptrBlue[i] != NULL)
                memcpy(pixelsBlue + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrBlue[i], nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
            else
                memset(pixelsBlue + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, 0, nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
        }

    }

    CalculateMinMax(0, width, height, pixelsRed, 0);
    //CalculateLUTGeo(0, pixelsRed, 0);
    equalizeHistogram(pixelsRed, width, height, 0, 0, 1023);
    if(kindofimage == "VIS_IR Color")
    {
        CalculateMinMax(1, width, height, pixelsGreen, 0);
        //CalculateLUTGeo(1, pixelsGreen, 0);
        equalizeHistogram(pixelsGreen, width, height, 1, 0, 1023);
        CalculateMinMax(2, width, height, pixelsBlue, 0);
        //CalculateLUTGeo(2, pixelsBlue, 0);
        equalizeHistogram(pixelsBlue, width, height, 2, 0, 1023);
    }

    computeGeoImage(pixelsRed, pixelsGreen, pixelsBlue);

    //    QVector<int> vec;

    //    for(int i = 0; i < opts.geosatellites[geoindex].imageheight; i++)
    //    {
    //        vec.append(i);
    //    }

    //    auto callbackMethod = std::bind(this->concurrentSetRed, this, std::placeholders::_1, 50);
    //    QtConcurrent::blockingMap(vec, callbackMethod);


    delete [] pixelsRed;
    if(kindofimage == "VIS_IR Color")
    {
        delete [] pixelsGreen;
        delete [] pixelsBlue;
    }

}

void SegmentListGeostationary::concurrentSetRed(SegmentListGeostationary *sm, const int &line, const int &value)
{
    QRgb *row_col;
    quint16 r,g, b;

    float red = 1.0/value;
    row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
    for(int pixelx = 0; pixelx < opts.geosatellites[sm->geoindex].imagewidth; pixelx++)
    {
        r = qRound(qRed(row_col[pixelx]) * red);
        g = qGreen(row_col[pixelx]);
        b = qBlue(row_col[pixelx]);
        row_col[pixelx] = qRgb(r, g , b);
    }
}

void SegmentListGeostationary::computeGeoImage(quint16 *pixelsRed, quint16 *pixelsGreen, quint16 *pixelsBlue)
{
    QRgb *row_col;
    QRgb *row_col_bitmap;
    quint16 cred, cgreen, cblue;
    quint16 r = 0, g = 0, b = 0;
    quint16 indexoutrc, indexoutgc, indexoutbc;

    double gamma = opts.meteosatgamma;
    double gammafactor = 1023 / pow(1023, gamma);
    quint16 valgamma;
    quint8 valcontrast;
    QImage *im = NULL;

    qDebug() << "SegmentListGeostationary::computeGeoImage";

    int nbroflinespersegment = opts.geosatellites[geoindex].segmentlength;

    //if(m_GeoSatellite != eGeoSatellite::H9)
        im = CalculateBitMap();

    unsigned int i_image;

    for (int line = (m_GeoSatellite != eGeoSatellite::H9 ? opts.geosatellites[geoindex].maxsegments*nbroflinespersegment - 1 : 0);
         (m_GeoSatellite != eGeoSatellite::H9 ? line >= 0 : line < opts.geosatellites[geoindex].maxsegments*nbroflinespersegment);
         (m_GeoSatellite != eGeoSatellite::H9 ? line-- : line++))
    {
        if(m_GeoSatellite == eGeoSatellite::GOMS3)
            row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
        else
        {
            if(m_GeoSatellite != eGeoSatellite::H9)
            {
                row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(opts.geosatellites[geoindex].imageheight - 1 - line);
                row_col_bitmap = (QRgb*)imageptrs->ptrimagebitmap->scanLine(opts.geosatellites[geoindex].imageheight - 1 - line);
            }
            else
            {
                row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
                row_col_bitmap = (QRgb*)imageptrs->ptrimagebitmap->scanLine(line);
            }
        }

        for (int pixelx = (m_GeoSatellite != eGeoSatellite::H9 ? opts.geosatellites[geoindex].imagewidth - 1 : 0);
             (m_GeoSatellite != eGeoSatellite::H9 ? pixelx >= 0 : pixelx < opts.geosatellites[geoindex].imagewidth);
             (m_GeoSatellite != eGeoSatellite::H9 ?  pixelx-- : pixelx++))
        {
            i_image = line * opts.geosatellites[geoindex].imagewidth + pixelx;
            if(kindofimage == "VIS_IR Color")
            {
                cred = *(pixelsRed + line * opts.geosatellites[geoindex].imagewidth + pixelx);
                cgreen = *(pixelsGreen+ line * opts.geosatellites[geoindex].imagewidth + pixelx);
                cblue = *(pixelsBlue+ line * opts.geosatellites[geoindex].imagewidth + pixelx);

                if(this->histogrammethod == CMB_HISTO_NONE_95)
                {
                    if(cred < 65528)
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - imageptrs->minRadianceIndex[0] ) / (float)(imageptrs->maxRadianceIndex[0] - imageptrs->minRadianceIndex[0])), 0), 1023);
                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                    else {
                        r = 0;
                    }

                    if(cgreen < 65528)
                    {
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cgreen - imageptrs->minRadianceIndex[1] ) / (float)(imageptrs->maxRadianceIndex[1] - imageptrs->minRadianceIndex[1])), 0), 1023);
                        valgamma = pow( indexoutgc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        g = quint8(valcontrast);
                        if (g > 255)
                            g = 255;
                    }
                    else {
                        g = 0;
                    }
                    if(cblue < 65528)
                    {
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cblue - imageptrs->minRadianceIndex[2] ) / (float)(imageptrs->maxRadianceIndex[2] - imageptrs->minRadianceIndex[2])), 0), 1023);
                        valgamma = pow( indexoutbc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        b = quint8(valcontrast);
                        if (b > 255)
                            b = 255;
                    }
                    else {
                        b = 0;
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_NONE_100)
                {
                    if(cred < 65528)
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);
                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                    else {
                        r = 0;
                    }
                    if(cgreen < 65528)
                    {
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cgreen - this->stat_min[1] ) / (float)(this->stat_max[1] - this->stat_min[1])), 0), 1023);
                        valgamma = pow( indexoutgc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        g = quint8(valcontrast);
                        if (g > 255)
                            g = 255;
                    }
                    else {
                        g = 0;
                    }
                    if(cblue < 65528)
                    {
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cblue - this->stat_min[2] ) / (float)(this->stat_max[2] - this->stat_min[2])), 0), 1023);
                        valgamma = pow( indexoutbc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        b = quint8(valcontrast);
                        if (b > 255)
                            b = 255;
                    }
                    else {
                        b = 0;
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( cred < 65528)
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);
                        indexoutrc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[0][indexoutrc]), 0), 1023);
                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                    else {
                        r = 0;
                    }

                    if( cgreen < 65528)
                    {
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cgreen - this->stat_min[1] ) / (float)(this->stat_max[1] - this->stat_min[1])), 0), 1023);
                        indexoutgc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[1][indexoutgc]), 0), 1023);
                        valgamma = pow( indexoutgc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        g = quint8(valcontrast);
                        if (g > 255)
                            g = 255;
                    }
                    else {
                        g = 0;
                    }
                    if( cblue < 65528)
                    {
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cblue - this->stat_min[2] ) / (float)(this->stat_max[2] - this->stat_min[2])), 0), 1023);
                        indexoutbc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[2][indexoutbc]), 0), 1023);
                        valgamma = pow( indexoutbc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        b = quint8(valcontrast);
                        if (b > 255)
                            b = 255;
                    }
                    else {
                        b = 0;
                    }

                }

                if(m_GeoSatellite == eGeoSatellite::GOMS3 )
                    row_col[pixelx] = qRgb(r,g,b);
                else
                {
//                                        if(qRed(row_col_bitmap[opts.geosatellites[geoindex].imagewidth - 1 - pixelx]) == 0)
//                                        {
//                                            r = 255;
//                                            g = 0;
//                                            b = 0;
//                                        }
//                                        else if(qRed(row_col_bitmap[opts.geosatellites[geoindex].imagewidth - 1 - pixelx]) == 255)
//                                        {
//                                            r = quint16(this->inversevector[0] ? (r == 255 ? 0 : r) : r);
//                                            g = quint16(this->inversevector[1] ? (g == 255 ? 0 : g) : g);
//                                            b = quint16(this->inversevector[2] ? (b == 255 ? 0 : b) : b);
//                                        }
//                                        else
                    {
                        r = quint16(this->inversevector[0] ? (255 - r) : r);
                        g = quint16(this->inversevector[1] ? (255 - g) : g);
                        b = quint16(this->inversevector[2] ? (255 - b) : b);
                    }

//                    if(qRed(row_col_bitmap[opts.geosatellites[geoindex].imagewidth - 1 - pixelx]) == 0)
//                    {
//                        r = 255;
//                        g = 0;
//                        b = 0;
//                    }
//                    else if(qRed(row_col_bitmap[opts.geosatellites[geoindex].imagewidth - 1 - pixelx]) == 255)
//                    {
//                        r = 0;
//                        g = 255;
//                        b = 255;
//                    }
//                    else if(qRed(row_col_bitmap[opts.geosatellites[geoindex].imagewidth - 1 - pixelx]) == 1)
//                    {
//                        r = 0;
//                        g = 255;
//                        b = 255;
//                    }
                }

                    if(m_GeoSatellite != eGeoSatellite::H9)
                        row_col[opts.geosatellites[geoindex].imagewidth - 1 - pixelx] = qRgb(r,g,b);
                    else
                        row_col[pixelx] = qRgb(r,g,b);



            }
            else if(kindofimage == "VIS_IR")
            {
                cred = *(pixelsRed + line * opts.geosatellites[geoindex].imagewidth + pixelx);

                if(this->histogrammethod == CMB_HISTO_NONE_95)
                {
                    if(cred < 65528)
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - imageptrs->minRadianceIndex[0] ) / (float)(imageptrs->maxRadianceIndex[0] - imageptrs->minRadianceIndex[0])), 0), 1023);
                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                }
                else if(this->histogrammethod == CMB_HISTO_NONE_100)
                {
                    if(cred < 65528)
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);
                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                }
                else // if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( cred < 65528)
                    {
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - this->stat_min[0] ) / (float)(this->stat_max[0] - this->stat_min[0])), 0), 1023);
                        indexoutrc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[0][indexoutrc]), 0), 1023);
                        valgamma = pow( indexoutrc, gamma) * gammafactor;
                        if (valgamma > 1023)
                            valgamma = 1023;

                        valcontrast = ContrastStretch(valgamma);
                        r = quint8(valcontrast);
                        if (r > 255)
                            r = 255;
                    }
                }

                g = r;
                b = r;

                if(m_GeoSatellite == eGeoSatellite::GOMS3 )
                    row_col[pixelx] = qRgb(r,g,b);
                else
                {
                    if(m_GeoSatellite != eGeoSatellite::H9)
                    {
                        if(qRed(row_col_bitmap[opts.geosatellites[geoindex].imagewidth - 1 - pixelx]) == 0)
                        {
                            r = 0;
                            g = 0;
                            b = 0;
                        }
                        else if(qRed(row_col_bitmap[opts.geosatellites[geoindex].imagewidth - 1 - pixelx]) == 255)
                        {
                            r = quint16(this->inversevector[0] ? (r == 255 ? 0 : r) : r);
                            g = r;
                            b = r;
                        }
                        else
                        {
                            r = quint16(this->inversevector[0] ? (255 - r) : r);
                            g = r;
                            b = r;
                        }

                        row_col[opts.geosatellites[geoindex].imagewidth - 1 - pixelx] =  qRgb(r,g,b);
                    }
                    else
                    {
                        r = quint16(this->inversevector[0] ? (255 - r) : r);
                        g = r;
                        b = r;
                        row_col[pixelx] = qRgb(r,g,b);
                    }
                }
            }
        }
    }

    if(im != NULL)
    {
        delete im;
        im = NULL;
    }


}

void SegmentListGeostationary::ComposeVISIR_Alt()
{

    QRgb *row_col;
    quint16 cred, cgreen, cblue;
    quint16 r,g, b;
    quint16 indexoutrc, indexoutgc, indexoutbc;

    qDebug() << "==========start ComposeVISIR ==================";

    int width = opts.geosatellites[geoindex].imagewidth;
    int height = opts.geosatellites[geoindex].imageheight;
    size_t nbrpix = width*height;
    int nbroflinespersegment = 464;

    //    if(m_GeoSatellite == eGeoSatellite::GOES_15)
    //        nbroflinespersegment = 580;
    //    else
    //        nbroflinespersegment = 464;

    quint16 *pixelsRed;
    quint16 *pixelsGreen;
    quint16 *pixelsBlue;

    pixelsRed = new quint16[nbrpix];
    if(kindofimage == "VIS_IR Color")
    {
        pixelsGreen = new quint16[nbrpix];
        pixelsBlue = new quint16[nbrpix];
    }

    for( int i = (bisRSS ? 5 : 0); i < opts.geosatellites[geoindex].maxsegments + (bisRSS ? 5 : 0); i++)
    {
        if(imageptrs->ptrRed[i] != NULL)
            memcpy(pixelsRed + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrRed[i], nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
        else
            memset(pixelsRed + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, 0, nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
    }
    if(kindofimage == "VIS_IR Color")
    {
        for( int i = (bisRSS ? 5 : 0); i < opts.geosatellites[geoindex].maxsegments + (bisRSS ? 5 : 0); i++)
        {
            if(imageptrs->ptrGreen[i] != NULL)
                memcpy(pixelsGreen + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrGreen[i], nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
            else
                memset(pixelsGreen + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, 0, nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
        }
        for( int i = (bisRSS ? 5 : 0); i < opts.geosatellites[geoindex].maxsegments + (bisRSS ? 5 : 0); i++)
        {
            if(imageptrs->ptrBlue[i] != NULL)
                memcpy(pixelsBlue + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrBlue[i], nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
            else
                memset(pixelsBlue + (bisRSS ? i - 5 : i) * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, 0, nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
        }

    }

    int fillValue[3];
    fillValue[0] = 0;
    fillValue[1] = 0;
    fillValue[2] = 0;

    CalculateMinMax(0, width, height, pixelsRed, 0);
    normalizeMinMax(width, height,pixelsRed, this->stat_min[0], this->stat_max[0], fillValue[0], 255 );

    //    CalculateLUTGeo(0, pixelsRed, 0);
    if(kindofimage == "VIS_IR Color")
    {
        CalculateMinMax(1, width, height, pixelsGreen, 0);
        normalizeMinMax(width, height,pixelsGreen, this->stat_min[1], this->stat_max[1], fillValue[1], 255 );
        //        CalculateLUTGeo(1, pixelsGreen, 0);
        CalculateMinMax(2, width, height, pixelsBlue, 0);
        normalizeMinMax(width, height,pixelsBlue, this->stat_min[2], this->stat_max[2], fillValue[2], 255 );
        //        CalculateLUTGeo(2, pixelsBlue, 0);
    }

    imageptrs->CLAHE(pixelsRed, 3712, 3712, this->stat_min[0], this->stat_max[0], 16, 16, 255, 3.5);
    if(kindofimage == "VIS_IR Color")
    {
        imageptrs->CLAHE(pixelsGreen, 3712, 3712, this->stat_min[1], this->stat_max[1], 16, 16, 255, 3.5);
        imageptrs->CLAHE(pixelsBlue, 3712, 3712, this->stat_min[2], this->stat_max[2], 16, 16, 255, 3.5);
    }

    long count_cred = 0;
    quint16 nindexred, nindexgreen, nindexblue;

    for (int line = opts.geosatellites[geoindex].maxsegments*nbroflinespersegment - 1; line >= 0; line--)
    {
        if(m_GeoSatellite == eGeoSatellite::GOMS3)
            row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
        else
            row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(opts.geosatellites[geoindex].maxsegments*nbroflinespersegment - 1 - line);

        for (int pixelx = opts.geosatellites[geoindex].imagewidth - 1 ; pixelx >= 0; pixelx--)
        {

            if(kindofimage == "VIS_IR Color")
            {
                cred = *(pixelsRed + line * opts.geosatellites[geoindex].imagewidth + pixelx);
                cgreen = *(pixelsGreen+ line * opts.geosatellites[geoindex].imagewidth + pixelx);
                cblue = *(pixelsBlue+ line * opts.geosatellites[geoindex].imagewidth + pixelx);


                //                if( (cred == 65535) || (cgreen == 65535) || (cblue == 65535))
                if( (cred == 0) || (cgreen == 0) || (cblue == 0))
                {
                    r = 0;
                    g = 0;
                    b = 0;
                }
                else
                {
                    r = quint16(this->inversevector[0] ? 255 - cred : cred);
                    g = quint16(this->inversevector[1] ? 255 - cgreen : cgreen);
                    b = quint16(this->inversevector[2] ? 255 - cblue : cblue);
                }
            }
            else if(kindofimage == "VIS_IR")
            {
                cred = *(pixelsRed + line * opts.geosatellites[geoindex].imagewidth + pixelx);


                if( cred == 0)
                {
                    r = 0;
                    g = 0;
                    b = 0;
                }
                else
                {
                    r = quint16(this->inversevector[0] ? 255 - cred : cred);
                    g = r;
                    b = r;
                }
            }
            if(m_GeoSatellite == eGeoSatellite::GOMS3 )
                row_col[pixelx] = qRgb(r,g,b);
            else
                row_col[opts.geosatellites[geoindex].imagewidth - 1 - pixelx] = qRgb(r,g,b);

        }
    }

    qDebug() << "count_cred = " << count_cred;

    delete [] pixelsRed;
    if(kindofimage == "VIS_IR Color")
    {
        delete [] pixelsGreen;
        delete [] pixelsBlue;
    }

}

void SegmentListGeostationary::ComposeVISIRHimawari()
{

    QRgb *row_col;
    quint16 cred, cgreen, cblue;
    quint16 r,g, b;
    quint16 valcontrast;

    qDebug() << "==========start ComposeVISIRHimawari geoindex = " << geoindex << " ==================";

    int width = opts.geosatellites[geoindex].imagewidth;
    int height = opts.geosatellites[geoindex].imageheight;
    size_t nbrpix = width*height;
    int nbroflinespersegment = opts.geosatellites[geoindex].segmentlength;

    quint16 *pixelsRed = NULL;
    quint16 *pixelsGreen = NULL;
    quint16 *pixelsBlue = NULL;

    pixelsRed = new quint16[nbrpix];
    if(kindofimage == "VIS_IR Color")
    {
        pixelsGreen = new quint16[nbrpix];
        pixelsBlue = new quint16[nbrpix];
    }

    for( int i = 0; i < 10; i++)
    {
        if(imageptrs->ptrRed[i] != NULL)
            memcpy(pixelsRed + i * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrRed[i], nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
        else
            memset(pixelsRed + i * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, 0, nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
    }


    if(kindofimage == "VIS_IR Color")
    {
        for( int i = 0; i < 10; i++)
        {
            if(imageptrs->ptrGreen[i] != NULL)
                memcpy(pixelsGreen + i * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrGreen[i], nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
            else
                memset(pixelsGreen + i * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, 0, nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
        }
        for( int i = 0; i < 10; i++)
        {
            if(imageptrs->ptrBlue[i] != NULL)
                memcpy(pixelsBlue + i * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrBlue[i], nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
            else
                memset(pixelsBlue + i * nbroflinespersegment * opts.geosatellites[geoindex].imagewidth, 0, nbroflinespersegment * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
        }

    }

#if 0
    CalculateMinMax(0, width, height, pixelsRed, 0);
    CalculateLUTGeo(0, pixelsRed, 0);
    if(kindofimage == "VIS_IR Color")
    {
        CalculateMinMax(1, width, height, pixelsGreen, 0);
        CalculateLUTGeo(1, pixelsGreen, 0);
        CalculateMinMax(2, width, height, pixelsBlue, 0);
        CalculateLUTGeo(2, pixelsBlue, 0);
    }
#else
    emit this->progressCounter(20);

    CalculateMinMax(0, width, height, pixelsRed, 0);
    equalizeHistogram(pixelsRed, opts.geosatellites[geoindex].imagewidth, opts.geosatellites[geoindex].imagewidth, 0, 0, 1023);
    if(kindofimage == "VIS_IR Color")
    {
        CalculateMinMax(1, width, height, pixelsGreen, 0);
        equalizeHistogram(pixelsGreen, opts.geosatellites[geoindex].imagewidth, opts.geosatellites[geoindex].imagewidth, 1, 0, 1023);
        CalculateMinMax(2, width, height, pixelsBlue, 0);
        equalizeHistogram(pixelsBlue, opts.geosatellites[geoindex].imagewidth, opts.geosatellites[geoindex].imagewidth, 2, 0, 1023);
    }
#endif

#if 1
    emit this->progressCounter(50);

    computeGeoImage(pixelsRed, pixelsGreen, pixelsBlue);
#else
    for (int line = 0; line < 5500; line++)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);

        for (int pixelx = 0 ; pixelx < 5500; pixelx++)
        {
            cred = *(pixelsRed + line * 5500 + pixelx);
            valcontrast = ContrastStretch(cred);
            if(inversevector[0])
                valcontrast = 255 - valcontrast;
            r = quint8(valcontrast);
            g = quint8(valcontrast);
            b = quint8(valcontrast);

            if(kindofimage == "VIS_IR Color")
            {
                cgreen = *(pixelsGreen + line * 5500 + pixelx);
                valcontrast = ContrastStretch(cgreen);
                if(inversevector[1])
                    valcontrast = 255 - valcontrast;
                g = quint8(valcontrast);

                cblue = *(pixelsBlue + line * 5500 + pixelx);
                valcontrast = ContrastStretch(cblue);
                if(inversevector[2])
                    valcontrast = 255 - valcontrast;
                b = quint8(valcontrast);

            }

            row_col[pixelx] = qRgb(r,g,b);
        }
    }
#endif

    delete [] pixelsRed;
    if(kindofimage == "VIS_IR Color")
    {
        delete [] pixelsGreen;
        delete [] pixelsBlue;
    }




}

void SegmentListGeostationary::ComposeHRV()
{
    QRgb *row_col;
    quint16 cred, cgreen, cblue, c, clum;
    quint16 r,g, b;

    double gamma = opts.meteosatgamma;
    double gammafactor = 1023 / pow(1023, gamma);
    quint16 valgamma;
    quint8 valcontrast;


    int width = opts.geosatellites[geoindex].imagewidth;
    int height = opts.geosatellites[geoindex].imageheight;
    size_t npix = 3712*3712;
    size_t npixHRV = 0;

    if (this->areatype == 1)
        npixHRV = 5568*11136;
    else
        npixHRV = 5568*5*464;

    if(opts.geosatellites.at(geoindex).rss)
    {
        npixHRV = 5568*5*464;
    }

    quint16 *pixelsHRV;
    quint16 *pixelsRed;
    quint16 *pixelsGreen;
    quint16 *pixelsBlue;

    pixelsHRV = new quint16[npixHRV];

    if(kindofimage == "HRV Color")
    {
        pixelsRed = new quint16[npix];
        pixelsGreen = new quint16[npix];
        pixelsBlue = new quint16[npix];
    }


    for( int i = 0, k = 0; i < (opts.geosatellites.at(geoindex).rss ? 5 : (this->areatype == 1 ? 24 : 5)); i++)
    {
        k = (opts.geosatellites.at(geoindex).rss ? 19 + i : (this->areatype == 1 ? i : 19 + i));

        if(imageptrs->ptrHRV[k] != NULL)
            memcpy(pixelsHRV + i * 464 * 5568, imageptrs->ptrHRV[k], 464 * 5568 * sizeof(quint16));
        else
            memset(pixelsHRV + i * 464 * 5568, 0, 464 * 5568 * sizeof(quint16));
    }

    if(kindofimage == "HRV Color")
    {
        for( int i = (opts.geosatellites.at(geoindex).rss ? 5 : 0); i < 8; i++)
        {
            if(imageptrs->ptrRed[i] != NULL)
                memcpy(pixelsRed + i * 464 * 3712, imageptrs->ptrRed[i], 464 * 3712 * sizeof(quint16));
            else
                memset(pixelsRed + i * 464 * 3712, 0, 464 * 3712 * sizeof(quint16));
        }
        for( int i = (opts.geosatellites.at(geoindex).rss ? 5 : 0); i < 8; i++)
        {
            if(imageptrs->ptrGreen[i] != NULL)
                memcpy(pixelsGreen + i * 464 * 3712, imageptrs->ptrGreen[i], 464 * 3712 * sizeof(quint16));
            else
                memset(pixelsGreen + i * 464 * 3712, 0, 464 * 3712 * sizeof(quint16));
        }
        for( int i = (opts.geosatellites.at(geoindex).rss ? 5 : 0); i < 8; i++)
        {
            if(imageptrs->ptrBlue[i] != NULL)
                memcpy(pixelsBlue + i * 464 * 3712, imageptrs->ptrBlue[i], 464 * 3712 * sizeof(quint16));
            else
                memset(pixelsBlue + i * 464 * 3712, 0, 464 * 3712 * sizeof(quint16));
        }
    }

    emit this->progressCounter(10);


    if(kindofimage == "HRV Color")
    {
        imageptrs->CLAHE(pixelsRed, 3712, 3712, 0, 1023, 16, 16, 256, 3);
        imageptrs->CLAHE(pixelsGreen, 3712, 3712, 0, 1023, 16, 16, 256, 3);
        imageptrs->CLAHE(pixelsBlue, 3712, 3712, 0, 1023, 16, 16, 256, 3);
    }

    emit this->progressCounter(20);

    if(opts.geosatellites.at(geoindex).rss)
        imageptrs->CLAHE(pixelsHRV, 5568, 5*464, 0, 1023, 16, 16, 256, 4);
    else
    {
        if(this->areatype == 1)
            imageptrs->CLAHE(pixelsHRV, 5568, 11136, 0, 1023, 16, 16, 256, 3);
        else
            imageptrs->CLAHE(pixelsHRV, 5568, 5*464, 0, 1023, 16, 16, 256, 3);
    }

    emit this->progressCounter(50);

    for (int line = (opts.geosatellites.at(geoindex).rss ? 5 : (this->areatype == 1 ? 24 : 5))*464 - 1; line >= 0; line--)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine((opts.geosatellites.at(geoindex).rss ? 5 : (this->areatype == 1 ? 24 : 5))*464 - 1 - line);

        for (int pixelx = 5568 - 1 ; pixelx >= 0; pixelx--)
        {
            c = *(pixelsHRV + line * 5568 + pixelx);
            //c = 255;

            if(kindofimage == "HRV Color")
            {
                if(opts.geosatellites.at(geoindex).rss)
                {
                    int diff = (19*464 + line)/3 * 3712 + LowerEastColumnActual/3 + pixelx/3;
                    cred = *(pixelsRed + diff);
                    cgreen = *(pixelsGreen + diff);
                    cblue = *(pixelsBlue + diff);
                    clum = (cred+cgreen+cblue)/3;
                    if( clum == 0)
                        clum = 1;

                }
                else
                {
                    if(this->areatype == 0)
                    {
                        cred = *(pixelsRed + (19*464 + line)/3 * 3712 + UpperEastColumnActual/3 + pixelx/3);
                        cgreen = *(pixelsGreen + (19*464 + line)/3 * 3712 + UpperEastColumnActual/3 + pixelx/3);
                        cblue = *(pixelsBlue + (19*464 + line)/3 * 3712 + UpperEastColumnActual/3 + pixelx/3);
                        clum = (cred+cgreen+cblue)/3;
                        if( clum == 0)
                            clum = 1;
                    }
                    else
                    {
                        if(line > UpperSouthLineActual)
                        {
                            cred = *(pixelsRed + line/3 * 3712 + UpperEastColumnActual/3 + pixelx/3);
                            cgreen = *(pixelsGreen + line/3 * 3712 + UpperEastColumnActual/3 + pixelx/3);
                            cblue = *(pixelsBlue + line/3 * 3712 + UpperEastColumnActual/3 + pixelx/3);
                            clum = (cred+cgreen+cblue)/3;
                            if( clum == 0)
                                clum = 1;
                        }
                        else
                        {
                            cred = *(pixelsRed + line/3 * 3712 + LowerEastColumnActual/3 + pixelx/3);
                            cgreen = *(pixelsGreen + line/3 * 3712 + LowerEastColumnActual/3 + pixelx/3);
                            cblue = *(pixelsBlue + line/3 * 3712 + LowerEastColumnActual/3 + pixelx/3);
                            clum = (cred+cgreen+cblue)/3;
                            if( clum == 0)
                                clum = 1;
                        }
                    }
                }

                valgamma = pow( c*cred/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                r = quint8(valcontrast);
                if (r > 255)
                    r = 255;

                valgamma = pow( c*cgreen/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                g = quint8(valcontrast);
                if (g > 255)
                    g = 255;

                valgamma = pow( c*cblue/clum, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                b = quint8(valcontrast);
                if (b > 255)
                    b = 255;
                row_col[5568 - 1 - pixelx] = qRgb(r,g,b);
            }
            else if(kindofimage == "HRV")
            {
                valgamma = pow( c, gamma) * gammafactor;
                if (valgamma >= 1024)
                    valgamma = 1023;

                valcontrast = ContrastStretch(valgamma);
                r = quint8(valcontrast);
                if (r > 255)
                    r = 255;

                row_col[5568 - 1 - pixelx] = qRgb(r,r,r);

            }
        }
    }

    emit this->progressCounter(100);

    delete [] pixelsHRV;
    if(kindofimage == "HRV Color")
    {
        delete [] pixelsRed;
        delete [] pixelsGreen;
        delete [] pixelsBlue;
    }
}

//bool SegmentListGeostationary::allHRVColorSegmentsReceived()
//{

//    qDebug() << QString("SegmentListGeostationary::allHRVColorSegmentsReceived()");

//    if (this->getKindofImage() == "HRV Color")
//    {
//        for(int i = (opts.geosatellites.at(geoindex).rss ? 5 : 0); i < 8; i++)
//        {
//            if (isPresentRed[i] && issegmentcomposedRed[i] == false)
//                return false;
//            if (isPresentGreen[i] && issegmentcomposedGreen[i] == false)
//                return false;
//            if (isPresentBlue[i] && issegmentcomposedBlue[i] == false)
//                return false;
//        }

//        for(int i = (opts.geosatellites.at(geoindex).rss ? 19 : (this->areatype == 0 ? 19 : 0)); i < 24; i++)
//        {
//            qDebug() << QString("index = %1 isPresent = %2 issegmentcomposedHRV = %3").arg(i).arg(isPresentHRV[i]).arg(issegmentcomposedHRV[i]);
//            if (isPresentHRV[i] && issegmentcomposedHRV[i] == false)
//                return false;
//        }
//    }
//    else if(this->getKindofImage() == "HRV")
//    {
//        for(int i = (opts.geosatellites.at(geoindex).rss ? 19 : (this->areatype == 0 ? 19 : 0)); i < 24; i++)
//        {
//            qDebug() << QString("index = %1 isPresent = %2 issegmentcomposedHRV = %3").arg(i).arg(isPresentHRV[i]).arg(issegmentcomposedHRV[i]);
//            if (isPresentHRV[i] && issegmentcomposedHRV[i] == false)
//                return false;
//        }
//    }

//    return true;
//}

//bool SegmentListGeostationary::allVIS_IRSegmentsReceived()
//{

//    qDebug() << QString("SegmentListGeostationary::allVIS_IRSegmentsReceived()");

//    int pbCounter = 0;
//    int pbCounterRed = 0;
//    int pbCounterGreen = 0;
//    int pbCounterBlue = 0;


//    if (this->getKindofImage() == "VIS_IR Color")
//    {
//        if(m_GeoSatellite == eGeoSatellite::FY2H || m_GeoSatellite == eGeoSatellite::FY2G || m_GeoSatellite == eGeoSatellite::GOES_16 || m_GeoSatellite == eGeoSatellite::GOES_17 || m_GeoSatellite == eGeoSatellite::GOES_18)
//        {
//            return true;
//        }
//        else if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_9 || m_GeoSatellite == eGeoSatellite::MET_8)
//        {
//            for(int i = (this->bisRSS ? 5 : 0) ; i < 8; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
//                {
//                    pbCounter++;
//                    pbCounterRed++;
//                }
//                if (isPresentGreen[i] && issegmentcomposedGreen[i] == true)
//                {
//                    pbCounter++;
//                    pbCounterGreen++;
//                }
//                if (isPresentBlue[i] && issegmentcomposedBlue[i] == true)
//                {
//                    pbCounter++;
//                    pbCounterBlue++;
//                }
//            }
//        }
//        else if(m_GeoSatellite == eGeoSatellite::GOMS3)
//        {
//            for(int i = 0 ; i < 6; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
//                    pbCounter++;
//                if (isPresentGreen[i] && issegmentcomposedGreen[i] == true)
//                    pbCounter++;
//                if (isPresentBlue[i] && issegmentcomposedBlue[i] == true)
//                    pbCounter++;
//            }
//        }
//        else if(m_GeoSatellite == eGeoSatellite::H8)
//        {
//            for(int i = 0; i < 10; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
//                    pbCounter++;
//                if (isPresentGreen[i] && issegmentcomposedGreen[i] == true)
//                    pbCounter++;
//                if (isPresentBlue[i] && issegmentcomposedBlue[i] == true)
//                    pbCounter++;
//            }
//        }
//    }
//    else if (this->getKindofImage() == "VIS_IR" )
//    {
//        if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_8)
//        {
//            for(int i = 0; i < 8; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
//                    pbCounter++;
//            }
//        }
//        else if(m_GeoSatellite == eGeoSatellite::GOMS3)
//        {
//            for(int i = 0; i < 6; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
//                    pbCounter++;
//            }
//        }
//        else if(m_GeoSatellite == eGeoSatellite::MET_9)
//        {
//            for(int i = 0; i < 8; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
//                    pbCounter++;
//            }
//        }
//        else if(m_GeoSatellite == eGeoSatellite::FY2H || m_GeoSatellite == eGeoSatellite::FY2G)
//        {
//            if (isPresentRed[0] && issegmentcomposedRed[0] == true)
//                pbCounter++;
//        }
//        else if(m_GeoSatellite == eGeoSatellite::H8)
//        {
//            for(int i = 0; i < 10; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
//                    pbCounter++;
//            }
//        }
//    }
//    else if (this->getKindofImage() == "HRV" || this->getKindofImage() == "HRV Color")
//    {
//        if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_9 || m_GeoSatellite == eGeoSatellite::MET_8)
//        {
//            for(int i = (this->bisRSS ? 5 : 0) ; i < 8; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
//                    pbCounter++;
//                if (isPresentGreen[i] && issegmentcomposedGreen[i] == true)
//                    pbCounter++;
//                if (isPresentBlue[i] && issegmentcomposedBlue[i] == true)
//                    pbCounter++;
//            }
//            for(int i = (this->areatype == 0 ? 0 : 19); i < 24; i++)
//            {
//                if (isPresentHRV[i] && issegmentcomposedHRV[i] == true)
//                    pbCounter++;
//            }
//        }
//        else if(m_GeoSatellite == eGeoSatellite::FY2H || m_GeoSatellite == eGeoSatellite::FY2G)
//        {
//            if (isPresentHRV[0] && issegmentcomposedHRV[0] == true)
//                pbCounter++;
//        }
//    }

//    qDebug() << QString("SegmentListGeostationary::allVIS_IRSegmentsReceived() pbCounter = %1").arg(pbCounter);

//    if(pbCounterRed == 8)
//        qDebug() << QString("pbCounterRed =  8");
//    if(pbCounterGreen == 8)
//        qDebug() << QString("pbCounterGreen =  8");
//    if(pbCounterBlue == 8)
//        qDebug() << QString("pbCounterBlue =  8");


//    emit progressCounter(pbCounter);

//    if (this->getKindofImage() == "VIS_IR Color")
//    {
//        if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_9 || m_GeoSatellite == eGeoSatellite::MET_8)
//        {
//            for(int i = (this->bisRSS ? 5 : 0) ; i < 8; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
//                    return false;
//                if (isPresentGreen[i] && issegmentcomposedGreen[i] == false)
//                    return false;
//                if (isPresentBlue[i] && issegmentcomposedBlue[i] == false)
//                    return false;
//            }
//        } else if(m_GeoSatellite == eGeoSatellite::GOMS3)
//        {
//            for(int i = 0 ; i < 6; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
//                    return false;
//                if (isPresentGreen[i] && issegmentcomposedGreen[i] == false)
//                    return false;
//                if (isPresentBlue[i] && issegmentcomposedBlue[i] == false)
//                    return false;
//            }
//        } else if(m_GeoSatellite == eGeoSatellite::H8)
//        {
//            for(int i = 0 ; i < 10; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
//                    return false;
//                if (isPresentGreen[i] && issegmentcomposedGreen[i] == false)
//                    return false;
//                if (isPresentBlue[i] && issegmentcomposedBlue[i] == false)
//                    return false;
//            }
//        }

//    }
//    else if (this->getKindofImage() == "VIS_IR" )
//    {
//        if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_8)
//        {
//            for(int i = 0; i < 8; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
//                    return false;
//            }
//        }
//        else if(m_GeoSatellite == eGeoSatellite::MET_9)
//        {
//            for(int i = 0; i < 8; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
//                    return false;
//            }
//        }
//        else if(m_GeoSatellite == eGeoSatellite::GOMS3)
//        {
//            for(int i = 0; i < 6; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
//                    return false;
//            }
//        }
//        else if(m_GeoSatellite == eGeoSatellite::FY2H || m_GeoSatellite == eGeoSatellite::FY2G || m_GeoSatellite == eGeoSatellite::GOES_16
//                || m_GeoSatellite == eGeoSatellite::GOES_17 || m_GeoSatellite == eGeoSatellite::GOES_18)
//        {
//            if (isPresentRed[0] && issegmentcomposedRed[0] == false)
//                return false;

//        }
//        else if(m_GeoSatellite == eGeoSatellite::H8)
//        {
//            //            qDebug() << "=======================================";
//            //            for(int i = 0; i < 10; i++)
//            //            {
//            //                qDebug() << QString("%1 isPresentRed = %2 issegmentcomposedRed = %3").arg(i).arg(isPresentRed[i]).arg(issegmentcomposedRed[i]);

//            //            }
//            //            qDebug() << "=======================================";
//            for(int i = 0; i < 10; i++)
//            {
//                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
//                    return false;
//            }
//        }
//    }
//    else if (this->getKindofImage() == "HRV" || this->getKindofImage() == "HRV Color")
//    {
//        if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_9 || m_GeoSatellite == eGeoSatellite::MET_8)
//        {
//            for(int i = 19; i < 24; i++)
//            {
//                if (isPresentHRV[i] && issegmentcomposedHRV[i] == false)
//                    return false;
//            }
//        }
//        else if(m_GeoSatellite == eGeoSatellite::FY2H || m_GeoSatellite == eGeoSatellite::FY2G)
//        {
//            if (isPresentHRV[0] && issegmentcomposedHRV[0] == false)
//                return false;
//        }
//    }

//    qDebug() << "SegmentListGeostationary::allVIS_IRSegmentsReceived() returns true";

//    emit progressCounter(100);
//    return true;
//}

void SegmentListGeostationary::CalculateMinMaxMTG(int colorindex, int index)
{
    //qDebug() << QString("CalculateMinMaxMTG colorindex = %1 index-1 = %2").arg(colorindex).arg(index-1);

    imageptrs->mtg_active_pixels[colorindex][index-1] = 0;

    imageptrs->mtg_stat_min[colorindex][index-1] = imageptrs->fillvalue[colorindex];
    imageptrs->mtg_stat_max[colorindex][index-1] = 0;

    quint16 *ptr = imageptrs->ptrMTG[colorindex][index-1];
    for (int j = 0; j < imageptrs->mtg_nbr_of_rows[colorindex][index-1]; j++) {
        for (int i = 0; i < imageptrs->mtg_nbr_of_columns[colorindex][index-1]; i++)
        {
            quint16 val = ptr[j * imageptrs->mtg_nbr_of_columns[colorindex][index-1] + i];
            if(val != imageptrs->fillvalue[colorindex])
            {
                if(val >= imageptrs->mtg_stat_max[colorindex][index-1])
                    imageptrs->mtg_stat_max[colorindex][index-1] = val;
                if(val < imageptrs->mtg_stat_min[colorindex][index-1])
                    imageptrs->mtg_stat_min[colorindex][index-1] = val;

                imageptrs->mtg_active_pixels[colorindex][index-1]++;
            }
        }
    }

    //qDebug() << QString("imageptrs->mtg_active_pixels[%1][%2] = %3").arg(colorindex).arg(index).arg(imageptrs->mtg_active_pixels[colorindex][index]);

}


void SegmentListGeostationary::CalculateMinMax(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max)
{
    stat_min = 65535;
    stat_max = 0;

    long count_zero = 0;

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++)
        {
            quint16 val = ptr[j * height + i];
            if(val > 0 && val < 65528)
            {
                if(val >= stat_max)
                    stat_max = val;
                if(val < stat_min)
                    stat_min = val;
            }
            else
                count_zero++;
        }
    }

}

void SegmentListGeostationary::CalculateMinMax(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue)
{
    long cnt = 0;
    this->stat_min[colorindex] = 65535;
    this->stat_max[colorindex] = 0;

    this->active_pixels[colorindex] = 0;

    qDebug() << QString("start CalculateMinMax colorindex = %1").arg(colorindex);

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            quint16 val = ptr[j * width + i];

            //                        if(colorindex == 0 && j == 1000 && i < 300)
            //                            qDebug() << val;

            if(val != fillvalue)
            {
                if(val >= this->stat_max[colorindex])
                    this->stat_max[colorindex] = val;
                if(val < this->stat_min[colorindex])
                    this->stat_min[colorindex] = val;

                this->active_pixels[colorindex]++;
            }
            else
                cnt++;
        }
    }

    qDebug() << QString("CalculateMinMax color = %1 stat_min = %2 stat_max = %3 active pixels = %4 pixels with fillvalue = %5")
                .arg(colorindex).arg(stat_min[colorindex]).arg(stat_max[colorindex]).arg(this->active_pixels[colorindex]).arg(cnt);

}

void SegmentListGeostationary::normalizeMinMax(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max, int &fillvalue, int maxvalue)
{

    quint16 pixval, pixvalnorm;

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++)
        {
            pixval = ptr[j * height + i];
            if(pixval != fillvalue)
            {
                pixvalnorm = (quint16)qMin(qMax(qRound((float)maxvalue * (float)(pixval - stat_min ) / (float)(stat_max - stat_min)), 0), maxvalue);
                ptr[j * height + i] = pixvalnorm;
            }
        }
    }

    stat_min = 0;
    stat_max = maxvalue;

}

//void SegmentListGeostationary::CalculateMinMaxHimawari(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max)
//{
//    stat_min = 65535;
//    stat_max = 0;


//    for (int j = 0; j < height; j++) {
//        for (int i = 0; i < width; i++)
//        {
//            quint16 val = ptr[j * height + i];
//            if(val >= stat_max)
//                stat_max = val;
//            if(val < stat_min)
//                stat_min = val;
//        }
//    }

//}

//void SegmentListGeostationary::SetupContrastStretch(quint16 x1, quint16 y1, quint16 x2, quint16 y2, quint16 x3, quint16 y3, quint16 x4, quint16 y4)
//{
//    //Q_ASSERT(val < 1023 && x1 < 1023 && x2 < 1023 && x3 < 1023 && x4 < 1023 && y1 < 255 && y2 < 255 && y3 < 255 && y4 < 255 && x1 < x2 < x3 < x4 && y1 < y2 < y3 < y4);

//    this->d_x1 = (double)x1;
//    this->d_x2 = (double)x2;
//    this->d_x3 = (double)x3;
//    this->d_x4 = (double)x4;
//    this->d_y1 = (double)y1;
//    this->d_y2 = (double)y2;
//    this->d_y3 = (double)y3;
//    this->d_y4 = (double)y4;

//    A1 = (d_y2 - d_y1)/(d_x2 - d_x1);
//    B1 = (d_y2 - (A1*d_x2));
//    A2 = (d_y3 - d_y2)/(d_x3 - d_x2);
//    B2 = (d_y3 - (A2*d_x3));
//    A3 = (d_y4 - d_y3)/(d_x4 - d_x3);
//    B3 = (d_y4 - (A3*d_x4));
//    //qDebug() << QString("A1 = %1;B1 = %2;A2 = %3;B2 = %4;A3 = %5;B3 = %6").arg(A1).arg(B1).arg(A2).arg(B2).arg(A3).arg(B3);
//}

//quint16 SegmentListGeostationary::ContrastStretch(quint16 val)
//{
//    double res;
//    if (val >= d_x1 && val < d_x2 )
//    {
//        res = double(val)*A1 + B1;
//    }
//    else if(val >= d_x2 && val < d_x3 )
//    {
//        res = double(val)*A2 + B2;
//    }
//    else if(val >= d_x3 && val <= d_x4 )
//    {
//        res = double(val)*A3 + B3;
//    }
//    return (res > 255.0 ? 255 : quint16(res));
//}

quint16 SegmentListGeostationary::ContrastStretch(quint16 val)
{
    double res;
    res = double(val)*A1 + B1;
    return (res > 255.0 ? 255 : (res < 0 ? 0 : quint16(res)));
}

void SegmentListGeostationary::SetupContrastStretch(quint16 x1, quint16 y1, quint16 x2, quint16 y2)
{
    //Q_ASSERT(val < 1023 && x1 < 1023 && x2 < 1023 && x3 < 1023 && x4 < 1023 && y1 < 255 && y2 < 255 && y3 < 255 && y4 < 255 && x1 < x2 < x3 < x4 && y1 < y2 < y3 < y4);

    this->d_x1 = (double)x1;
    this->d_x2 = (double)x2;
    this->d_y1 = (double)y1;
    this->d_y2 = (double)y2;

    A1 = (d_y2 - d_y1)/(d_x2 - d_x1);
    B1 = (d_y2 - (A1*d_x2));
    //qDebug() << QString("A1 = %1;B1 = %2;A2 = %3;B2 = %4;A3 = %5;B3 = %6").arg(A1).arg(B1).arg(A2).arg(B2).arg(A3).arg(B3);
}

void SegmentListGeostationary::ComposeGeoRGBRecipe(int recipe, QString tex)
{
    this->tex = tex;
    QtConcurrent::run(doComposeGeoRGBRecipe, this, recipe);
}

void SegmentListGeostationary::ComposeGeoRGBRecipeInThread(int recipe)
{
    qDebug() << "start SegmentListGeostationary::ComposeGeoRGBRecipeInThread(int recipe)";
    qDebug() << "geoindex = " << geoindex;
    emit progressCounter(10);

    QStringList redbandlist = imageptrs->rgbrecipes[recipe].Colorvector.at(0).channels;
    QStringList greenbandlist = imageptrs->rgbrecipes[recipe].Colorvector.at(1).channels;
    QStringList bluebandlist = imageptrs->rgbrecipes[recipe].Colorvector.at(2).channels;
    QList<seviriunits> uniqueunitlist;
    seviriunits redunits = imageptrs->rgbrecipes[recipe].Colorvector.at(0).units;
    seviriunits greenunits = imageptrs->rgbrecipes[recipe].Colorvector.at(1).units;
    seviriunits blueunits = imageptrs->rgbrecipes[recipe].Colorvector.at(2).units;
    QList<int> uniquechannelnbrlist;

    QStringList uniquebandlist;

    for(int i = 0; i < redbandlist.length(); i++)
    {
        if(!uniquebandlist.contains(redbandlist.at(i)))
        {
            uniquebandlist.append(redbandlist.at(i));
            uniqueunitlist.append(redunits);
            uniquechannelnbrlist.append(imageptrs->rgbrecipes[recipe].Colorvector.at(0).spectral_channel_nbr.at(i));
        }
    }
    for(int i = 0; i < greenbandlist.length(); i++)
    {
        if(!uniquebandlist.contains(greenbandlist.at(i)))
        {
            uniquebandlist.append(greenbandlist.at(i));
            uniqueunitlist.append(greenunits);
            uniquechannelnbrlist.append(imageptrs->rgbrecipes[recipe].Colorvector.at(1).spectral_channel_nbr.at(i));
        }
    }
    for(int i = 0; i < bluebandlist.length(); i++)
    {
        if(!uniquebandlist.contains(bluebandlist.at(i)))
        {
            uniquebandlist.append(bluebandlist.at(i));
            uniqueunitlist.append(blueunits);
            uniquechannelnbrlist.append(imageptrs->rgbrecipes[recipe].Colorvector.at(2).spectral_channel_nbr.at(i));
        }
    }


    QString recipename = imageptrs->rgbrecipes[recipe].Name;
    qDebug() << "recipename = " << recipename;
    qDebug() << "uniquebandlist contains = " << uniquebandlist.count();
    for(int i = 0; i < uniquebandlist.length(); i++)
    {
        qDebug() << "channels at " << uniquechannelnbrlist.at(i) << " = " << uniquebandlist.at(i) << " unit = " << (int)uniqueunitlist.at(i);
    }


    qDebug() << "tex = " << this->tex << " imagepath = " << this->getImagePath();
    //tex =  "2018-02-13   14:45"

    imageptrs->ResetPtrImage();

    QString directory = this->getImagePath();
    QString productid1 = opts.geosatellites.at(geoindex).searchstring.mid(6, 4);
    QString productid2 = "IR_016";
    QString timing = tex.mid(0, 4) + tex.mid(5, 2) + tex.mid(8, 2) + tex.mid(13, 2) + tex.mid(16, 2);

    qDebug() << "directory = " << directory;
    qDebug() << "productid1 = " << productid1;
    qDebug() << "productid2 = " << productid2;
    qDebug() << "timing = " << timing;

    int totalpixels = 3712 * 3712;

    if(imageptrs->rgbrecipes[recipe].needsza)
    {
        time = new double[totalpixels];
        lat = new float[totalpixels];
        lon = new float[totalpixels];
        sza = new float[totalpixels];

        snu_init_array_d(time, totalpixels, FILL_VALUE_F);
        snu_init_array_f(lat,  totalpixels, FILL_VALUE_F);
        snu_init_array_f(lon,  totalpixels, FILL_VALUE_F);
        snu_init_array_f(sza,  totalpixels, FILL_VALUE_F);

        emit progressCounter(13);

        saa = new float[totalpixels];
        vza = new float[totalpixels];
        vaa = new float[totalpixels];

        snu_init_array_f(saa,  totalpixels, FILL_VALUE_F);
        snu_init_array_f(vza,  totalpixels, FILL_VALUE_F);
        snu_init_array_f(vaa,  totalpixels, FILL_VALUE_F);

        emit progressCounter(18);

    }

    result[0] = new float[totalpixels];
    result[1] = new float[totalpixels];
    result[2] = new float[totalpixels];

    snu_init_array_f(result[0], totalpixels, FILL_VALUE_F);
    snu_init_array_f(result[1], totalpixels, FILL_VALUE_F);
    snu_init_array_f(result[2], totalpixels, FILL_VALUE_F);

    MsgFileAccess fa(directory, "H", productid1, productid2, timing);
    MsgDataAccess da;
    MSG_data prodata;
    MSG_data epidata;
    MSG_header header;

    QApplication::setOverrideCursor( Qt::WaitCursor ); // this might take time

    da.scan(fa, prodata, epidata, header);
    emit progressCounter(20);

    double jtime_start = epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanStart.get_jtime();
    double jtime_end = epidata.epilogue->product_stats.ActualScanningSummary.ForwardScanEnd.get_jtime();
    satid =  (int)header.segment_id->spacecraft_id - 321 ;
    double jtime = (jtime_start + jtime_end) / 2.;

    struct tm cdate;
    Calendar_Date(jtime, &cdate);
    double day_of_year = jtime - QSgp4Date::DateToJD(cdate.tm_year, 1, 0, true);

    qDebug() << "Julian day = " << jtime;
    qDebug() << "day_of_year = " << day_of_year;

    double jtime2;
    double jtime_start2;
    double jtime_end2;

    double slope, offset;
    bool toint;

    lon0 = prodata.prologue->image_description.ProjectionDescription.LongitudeOfSSP;

    for(int i = 0; i < uniquebandlist.length(); i++)
    {

        bandstorage newband;
        newband.listindex = i;
        newband.spectral_channel_nbr = uniquechannelnbrlist.at(i);
        prodata.prologue->radiometric_proc.get_slope_offset(newband.spectral_channel_nbr, slope, offset, toint);

        newband.data = new float[totalpixels];
        snu_init_array_f(newband.data,  totalpixels, FILL_VALUE_F);
        newband.min = 0; newband.max = 0;
        newband.directory = directory;
        newband.productid1 = productid1;
        newband.productid2 = uniquebandlist.at(i);
        newband.timing = timing;
        newband.day_of_year = day_of_year;
        newband.units = uniqueunitlist.at(i);
        newband.slope = slope;
        newband.offset = offset;
        bands.append(newband);
    }

    if(imageptrs->rgbrecipes[recipe].needsza)
    {

        /*-------------------------------------------------------------------------
        * Compute the satellite position vector in Cartesian coordinates (km).
        *-----------------------------------------------------------------------*/
        int i;
        for (i = 0; i < 100; ++i) {
            jtime_start2 = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].StartTime.get_jtime();
            jtime_end2   = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].EndTime.get_jtime();
            if (jtime >= jtime_start2 && jtime <= jtime_end2)
                break;
        }

        if (i == 100) {
            fprintf(stderr, "ERROR: Image time is out of range of supplied orbit "
                            "polynomials\n");
            return;
        }

        double t, X, Y, Z;
        t = (jtime - (jtime_start2 + jtime_end2) / 2.) / ((jtime_end2   - jtime_start2) / 2.);

        X = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].X[0] +
                prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].X[1] * t;
        Y = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Y[0] +
                prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Y[1] * t;
        Z = prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Z[0] +
                prodata.prologue->sat_status.Orbit.OrbitPoliniomal[i].Z[1] * t;
        /*-------------------------------------------------------------------------
        * Compute latitude and longitude and solar and sensor zenith and azimuth
        * angles.
        *-----------------------------------------------------------------------*/

        lon0 = prodata.prologue->image_description.ProjectionDescription.LongitudeOfSSP;
        qDebug() << "longitude SSP = " << lon0;

        double mu0;
        double theta0;
        double phi0;

        quint16 seglines = header.image_structure->number_of_lines;
        quint16 columns = header.image_structure->number_of_columns;

        unsigned int i_image;

        for (int i = 0; i < 3712; ++i)
        {
            jtime2 = jtime_start + (double) i / (double) (3712 - 1) * (jtime_end - jtime_start);
            for (int j = 0; j < 3712; ++j)
            {
                i_image = i * 3712 + j;

                snu_line_column_to_lat_lon(i, j, &lat[i_image], &lon[i_image], lon0, &nav_scaling_factors_vir);

                if (lat[i_image] != FILL_VALUE_F && lon[i_image] != FILL_VALUE_F)
                {
                    time[i_image] = jtime2;

                    snu_solar_params2(jtime2, lat[i_image] * D2R, lon[i_image] * D2R, &mu0, &theta0, &phi0, NULL);
                    sza[i_image] = theta0 * R2D;
                    saa[i_image] = phi0   * R2D;

                    saa[i_image] = saa[i_image] + 180.;
                    if (saa[i_image] > 360.)
                        saa[i_image] = saa[i_image] - 360.;

                    snu_vza_and_vaa(lat[i_image], lon[i_image], 0., X, Y, Z, &vza[i_image], &vaa[i_image]);

                    vaa[i_image] = vaa[i_image] + 180.;
                    if (vaa[i_image] > 360.)
                        vaa[i_image] = vaa[i_image] - 360.;

                }
                else
                {
                    for(int k = 0; k < bands.length(); k++)
                        bands[k].data[i_image] = FILL_VALUE_F;
                }
            }
        }

        //        PrintResults(sza, "sza");
        //        PrintResults(vza, "vza");
    }

    emit progressCounter(30);

    //0 "Airmass RGB"
    //1 "Dust RGB"
    //2 "24 hours Microphysics RGB"
    //3 "Ash RGB"
    //4 "Day Microphysics RGB Summer"
    //5 "Severe Storms RGB"
    //6 "Snow RGB"
    //7 "Natural Colors RGB"
    //8 "Night Microphysics RGB";
    //9 "IR_39 sun reflected";
    //10 "Day Microphysics RGB winter"

    if(recipe != 4 && recipe != 6 && recipe != 9 && recipe != 10)
    {
        QtConcurrent::blockingMap(bands, [this] (bandstorage &data) { CalculateGeoRadiances(data); });

        //Printbands();

        bool pixelok = false;

        for(int colorindex = 0; colorindex < 3; colorindex++)
        {
            emit progressCounter(40 + colorindex * 10);

            for(int i = 0; i < imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).channels.length(); i++)
            {
                for(int j = 0; j < bands.length(); j++)
                {
                    if(bands[j].spectral_channel_nbr == imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).spectral_channel_nbr.at(i))
                    {
                        qDebug() << colorindex << " " << imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).channels.at(i) << " " <<
                                    imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).spectral_channel_nbr.at(i) << " " <<
                                    imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).subtract.at(i);
                        for (int line = 0; line < 3712; line++)
                        {
                            for (int pixelx = 0; pixelx < 3712; pixelx++)
                            {
                                int i_image = line * 3712 + pixelx;
                                pixelok = true;
                                for(int j1 = 0; j1 < bands.length();j1++)
                                {
                                    if(bands[j1].data[i_image] == FILL_VALUE_F || bands[j1].data[i_image] == FILL_VALUE_US )
                                    {
                                        pixelok = false;
                                        break;
                                    }
                                }
                                if(pixelok)
                                {
                                    if(imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).subtract.at(i))
                                    {
                                        if(result[colorindex][i_image] == FILL_VALUE_F)
                                            result[colorindex][i_image] = - bands[j].data[i_image];
                                        else
                                            result[colorindex][i_image] -= bands[j].data[i_image];
                                    }
                                    else
                                    {
                                        if(result[colorindex][i_image] == FILL_VALUE_F)
                                            result[colorindex][i_image] = bands[j].data[i_image];
                                        else
                                            result[colorindex][i_image] += bands[j].data[i_image];
                                    }
                                }
                                else
                                {
                                    result[colorindex][i_image] = FILL_VALUE_F;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if(recipe == 4 || recipe == 10)
        {
            ComposeDayMicrophysicsRGB(bands[0], jtime);
        }
        else if(recipe == 6)
        {
            ComposeSnowRGB(bands[0], jtime);
        }
        else if(recipe == 9)
        {
            ComposeIR_039sunreflected(bands[0], jtime);
        }
    }
    //PrintResults();

    float resultmax[3];
    float resultmin[3];
    float resultmax_perc[3];
    float resultmin_perc[3];
    long countresultfill[3];

    for(int i = 0; i < 3; i++)
    {
        resultmin[i] = 999999999;
        resultmax[i] = -999999999;
        countresultfill[i] = 0;
    }

    emit progressCounter(70);

    for(int colorindex = 0; colorindex < 3; colorindex++)
    {
        for (int line = 0; line < 3712; line++)
        {
            for (int pixelx = 0; pixelx < 3712; pixelx++)
            {
                int i_image = line * 3712 + pixelx;

                if(result[colorindex][i_image] != FILL_VALUE_F)
                {
                    if(result[colorindex][i_image] < resultmin[colorindex]) resultmin[colorindex] = result[colorindex][i_image];
                    if(result[colorindex][i_image] > resultmax[colorindex]) resultmax[colorindex] = result[colorindex][i_image];
                }
                else
                    countresultfill[colorindex]++;
            }
        }
    }

    for(int colorindex = 0; colorindex < 3; colorindex++ )
    {
        qDebug() << QString("---> %1 resultmin = %2 resultmax = %3").arg(colorindex).arg(resultmin[colorindex]).arg(resultmax[colorindex]);
        qDebug() << QString("---> countresultfill = %1").arg(countresultfill[colorindex]);
    }

    QRgb *row_col;
    float red, green, blue;
    imageptrs->InitializeImageGeostationary(opts.geosatellites.at(geoindex).imagewidth, opts.geosatellites.at(geoindex).imageheight);


    for(int colorindex = 0; colorindex < 3; colorindex++)
    {
        for (int line = 0; line < 3712; line++)
        {
            for (int pixelx = 0; pixelx < 3712; pixelx++)
            {
                int i_image = line * 3712 + pixelx;
                if(result[colorindex][i_image] != FILL_VALUE_F )
                {
                    if(imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).dimension == "K")
                    {
                        if(result[colorindex][i_image] > imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangeto )
                            result[colorindex][i_image] = imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangeto;
                        if(result[colorindex][i_image] < imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangefrom )
                            result[colorindex][i_image] = imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangefrom;
                    }
                    if(imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).dimension == "%")
                    {
                        //both positive
                        resultmin_perc[colorindex] = resultmin[colorindex]*imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangefrom;
                        resultmax_perc[colorindex] = resultmax[colorindex]*imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangeto;

                        if(imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangefrom == 0)
                        {
                            if(result[colorindex][i_image] > resultmax_perc[colorindex])
                                result[colorindex][i_image] = resultmax_perc[colorindex];
                        }
                        else if(result[colorindex][i_image] < 0)
                        {
                            if(result[colorindex][i_image] < - resultmin_perc[colorindex])
                                result[colorindex][i_image] = - resultmin_perc[colorindex];
                        }
                        else if(result[colorindex][i_image] > 0)
                        {
                            if(result[colorindex][i_image] > resultmax_perc[colorindex])
                                result[colorindex][i_image] = resultmax_perc[colorindex];
                        }

                    }
                }
            }
        }
    }

    for(int colorindex = 0; colorindex < 3; colorindex++ )
    {
        if(imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).dimension == "K")
        {
            resultmin[colorindex] = imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangefrom;
            resultmax[colorindex] = imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).rangeto;
        }
        if(imageptrs->rgbrecipes[recipe].Colorvector.at(colorindex).dimension == "%")
        {
            resultmin[colorindex] = - resultmin_perc[colorindex];
            resultmax[colorindex] = resultmax_perc[colorindex];
        }
    }

    emit progressCounter(80);

    for(int colorindex = 0; colorindex < 3; colorindex++ )
        qDebug() << QString("%1 resultmin = %2 resultmax = %3").arg(colorindex).arg(resultmin[colorindex]).arg(resultmax[colorindex]);


    //    for (int line = 0; line < 3712; line++)
    //    {
    //        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(3711 - line);
    //        for (int pixelx = 0; pixelx < 3712; pixelx++)
    //        {
    //            int i_image = line * 3712 + pixelx;

    //            if(result[0][i_image] == FILL_VALUE_F || result[1][i_image] == FILL_VALUE_F || result[2][i_image] == FILL_VALUE_F )
    //            {
    //                red = 0.0;
    //                green = 0.0;
    //                blue = 0.0;
    //            }
    //            else
    //            {
    //                if(imageptrs->rgbrecipes[recipe].Colorvector.at(0).inverse.at(0))
    //                    red   = 255.0 - 255.0 * pow((result[0][i_image] - resultmin[0]) / (resultmax[0] - resultmin[0]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(0).gamma);
    //                else
    //                    red   = 255.0 * pow((result[0][i_image] - resultmin[0]) / (resultmax[0] - resultmin[0]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(0).gamma);
    //                if(imageptrs->rgbrecipes[recipe].Colorvector.at(1).inverse.at(0))
    //                    green = 255.0 - 255.0 * pow((result[1][i_image] - resultmin[1]) / (resultmax[1] - resultmin[1]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(1).gamma);
    //                else
    //                    green = 255.0 * pow((result[1][i_image] - resultmin[1]) / (resultmax[1] - resultmin[1]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(1).gamma);
    //                if(imageptrs->rgbrecipes[recipe].Colorvector.at(2).inverse.at(0))
    //                    blue  = 255 - 255.0 * pow((result[2][i_image] - resultmin[2]) / (resultmax[2] - resultmin[2]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(2).gamma);
    //                else
    //                    blue  = 255.0 * pow((result[2][i_image] - resultmin[2]) / (resultmax[2] - resultmin[2]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(2).gamma);
    //            }
    //            int i_image1 = (3711 - line) * 3712 + (3711 - pixelx);

    //            imageptrs->ptrimageRGBRecipeRed[i_image1] = (quint8)red;
    //            imageptrs->ptrimageRGBRecipeGreen[i_image1] = (quint8)green;
    //            imageptrs->ptrimageRGBRecipeBlue[i_image1] = (quint8)blue;

    //            row_col[3711 - pixelx] = qRgb((int)red, (int)green, (int)blue);
    //        }
    //    }


    for (int line = (opts.geosatellites.at(geoindex).rss ? 5 * 464 : 0); line < 3712; line++)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(opts.geosatellites.at(geoindex).rss ? 8 * 464 - 1 - line : 3711 - line);
        for (int pixelx = 0; pixelx < 3712; pixelx++)
        {
            int i_image = line * 3712 + pixelx;

            if(result[0][i_image] == FILL_VALUE_F || result[1][i_image] == FILL_VALUE_F || result[2][i_image] == FILL_VALUE_F )
            {
                red = 0.0;
                green = 0.0;
                blue = 0.0;
            }
            else
            {
                if(imageptrs->rgbrecipes[recipe].Colorvector.at(0).inverse.at(0))
                    red   = 255.0 - 255.0 * pow((result[0][i_image] - resultmin[0]) / (resultmax[0] - resultmin[0]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(0).gamma);
                else
                    red   = 255.0 * pow((result[0][i_image] - resultmin[0]) / (resultmax[0] - resultmin[0]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(0).gamma);
                if(imageptrs->rgbrecipes[recipe].Colorvector.at(1).inverse.at(0))
                    green = 255.0 - 255.0 * pow((result[1][i_image] - resultmin[1]) / (resultmax[1] - resultmin[1]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(1).gamma);
                else
                    green = 255.0 * pow((result[1][i_image] - resultmin[1]) / (resultmax[1] - resultmin[1]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(1).gamma);
                if(imageptrs->rgbrecipes[recipe].Colorvector.at(2).inverse.at(0))
                    blue  = 255 - 255.0 * pow((result[2][i_image] - resultmin[2]) / (resultmax[2] - resultmin[2]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(2).gamma);
                else
                    blue  = 255.0 * pow((result[2][i_image] - resultmin[2]) / (resultmax[2] - resultmin[2]), 1.0/imageptrs->rgbrecipes[recipe].Colorvector.at(2).gamma);
            }
            int i_image1 = (3711 - line) * 3712 + (3711 - pixelx);

            imageptrs->ptrimageRGBRecipeRed[i_image1] = (quint8)red;
            imageptrs->ptrimageRGBRecipeGreen[i_image1] = (quint8)green;
            imageptrs->ptrimageRGBRecipeBlue[i_image1] = (quint8)blue;

            row_col[3711 - pixelx] = qRgb((int)red, (int)green, (int)blue);
        }
    }


    qDebug() << "Kind of image = " << kindofimage;
    emit signalcomposefinished(kindofimage, geoindex);
    emit progressCounter(100);

    //////////////////////////////////////////////////
    // Cleaup
    //////////////////////////////////////////////////

    //    delete [] pixelsRed;
    //    delete [] pixelsGreen;
    //    delete [] pixelsBlue;

    for(int i = 0; i < uniquebandlist.length(); i++)
    {
        delete [] bands[i].data;
    }

    bands.clear();

    if(imageptrs->rgbrecipes[recipe].needsza)
    {
        delete [] time;
        delete [] lat;
        delete [] lon;
        delete [] sza;
        delete [] saa;
        delete [] vza;
        delete [] vaa;
    }

    delete [] result[0];
    delete [] result[1];
    delete [] result[2];

    qDebug() << "CreateRGBrecipeImageThread Finished !";
}


void SegmentListGeostationary::ComposeDayMicrophysicsRGB(bandstorage &bs, double julian_day)
{

    // Red   VIS008    0 --> 100   % Gamma 1.0
    // Green IR3.9Refl 0 --> 60    % Gamma 2.5
    // Blue  IR10.8    203 --> 323 K Gamma 1.0

    //VIS006 1
    //VIS008 2
    //IR_016 3
    //IR_039 4
    //WV_062 5
    //WV_073 6
    //IR_087 7
    //IR_097 8
    //IR_108 9
    //IR_120 10
    //IR_134 11

    // REFL(IR3.9) = 100 * (Rtot - Rtherm)/(TOARAD - Rtherm)
    //
    int totalpixels = 3712 * 3712;

    float *ref008 = new float[totalpixels];
    float *rad039  = new float[totalpixels];
    float *rad039_from_bt108  = new float[totalpixels];
    float *rad039_corr  = new float[totalpixels];
    float *bt108  = new float[totalpixels];
    float *bt134  = new float[totalpixels];
    float *r_therm  = new float[totalpixels];
    float *toarad = new float[totalpixels];

    snu_init_array_f(ref008, totalpixels, FILL_VALUE_F);
    snu_init_array_f(rad039,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(rad039_from_bt108,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(rad039_corr,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(bt108, totalpixels, FILL_VALUE_F);
    snu_init_array_f(bt134,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(r_therm,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(toarad,  totalpixels, FILL_VALUE_F);

    GetRadBT(SEVIRI_UNIT_REF, 2, bs, ref008);

    emit progressCounter(45);

    //PrintResults(ref008, "ref008");

    qDebug() << "scanning ir_108";

    GetRadBT(SEVIRI_UNIT_BT, 9, bs, bt108);

    emit progressCounter(50);

    GetRadBT(SEVIRI_UNIT_BT, 11, bs, bt134);

    //PrintResults(bt134, "bt134");

    emit progressCounter(55);

    qDebug() << "get radiance for ir_039";

    GetRadBT(SEVIRI_UNIT_RAD, 4, bs, rad039);

    qDebug() << "calculating radiance ir_039 from bt108";

    // Radiance from IR_039 channel = 4
    // using bt108 and parameters from IR_039
    int channel = 4;
    double a = bt_A[satid][channel - 1];
    double b = bt_B[satid][channel - 1];
    double nu = bt_nu_c[satid][channel - 1];
    double nu3 = nu * nu * nu;
    const double c1 = 1.19104e-5; // [mW m^-2 sr^-1 (cm^-1)^-4
    const double c2 = 1.43877; // K (cm^-1)^-1

    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (bt108[i_image] != FILL_VALUE_F && bt108[i_image] > 0)
            {
                rad039_from_bt108[i_image] =  c1 * nu3 / (exp( c2 * nu / (a * bt108[i_image] + b)) - 1);
            }
        }
    }

    emit progressCounter(60);

    //PrintResults(rad039, "calculating radiance ir_039");

    qDebug() << "calculating co2 correction";

    //CO2 correction
    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (bt108[i_image] != FILL_VALUE_F && bt108[i_image] > 0)
            {
                rad039_corr[i_image] =  pow((bt108[i_image] - 0.25 * (bt108[i_image] - bt134[i_image])), 4.0) / pow(bt108[i_image], 4.0);
            }
        }
    }

    //PrintResults(rad039_corr, "calculating co2 correction");

    qDebug() << "calculating r_therm";

    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (rad039_from_bt108[i_image] != FILL_VALUE_F && rad039_from_bt108[i_image] > 0 && rad039_corr[i_image] != FILL_VALUE_F && rad039_corr[i_image] > 0)
            {
                r_therm[i_image] =  rad039_from_bt108[i_image] * rad039_corr[i_image];
                if(r_therm[i_image] > rad039[i_image])
                    r_therm[i_image] = rad039[i_image];
            }
        }
    }

    //PrintResults(r_therm, "calculating r_therm");

    qDebug() << "calculating co2 correction solar constant";

    //The CO2-corrected, solar constant at the Top of the Atmosphere
    //in Channel 04 (IR3.9)
    double ESD = 1.0 - 0.0167 * cos( 2 * PIE * (julian_day - 3)/ 365.0);
    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;
            if (rad039_corr[i_image] != FILL_VALUE_F && rad039_corr[i_image] > 0 && sza[i_image] < 89.0 && vza[i_image] < 89.0)
            {
                toarad[i_image] =  4.92 / ESD * ESD;
                float costeta = cos(sza[i_image] * D2R);
                float cossat = cos(vza[i_image] * D2R);
                toarad[i_image] *= costeta;

                toarad[i_image] *= exp(-(1.0 - rad039_corr[i_image]));
                toarad[i_image] *= exp(-(1.0 - rad039_corr[i_image]) * costeta / cossat);

            }
        }
    }

    //PrintResults(toarad, "calculating co2 correction solar constant");

    emit progressCounter(65);

    qDebug() << "calculating result";

    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (rad039[i_image] != FILL_VALUE_F && rad039[i_image] > 0 && vza[i_image] < 89.0)
            {
                result[0][i_image] =  ref008[i_image];
                //                result[1][i_image] =  100 * (rad039[i_image] - r_therm[i_image]) / (toarad[i_image] - r_therm[i_image]);
                result[1][i_image] =  100 * (rad039[i_image] - r_therm[i_image]); // / cos(vza[i_image] * D2R);
                result[2][i_image] =  bt108[i_image];
            }
        }
    }

    delete [] ref008;
    delete [] rad039;
    delete [] rad039_from_bt108;
    delete [] rad039_corr;
    delete [] bt108;
    delete [] bt134;
    delete [] r_therm;
    delete [] toarad;

}

void SegmentListGeostationary::GetRadBT(int unit, int channel, bandstorage &bs, float *container)
{

    //VIS008
    MsgFileAccess fa(bs.directory, "H", bs.productid1, getSeviribandfromChannel(channel), bs.timing);
    MsgDataAccess da;
    MSG_header header;
    MSG_data prodata;
    MSG_data epidata;

    da.scan(fa, prodata, epidata, header);

    double slope, offset;
    bool toint;
    prodata.prologue->radiometric_proc.get_slope_offset(channel, slope, offset, toint);

    for (size_t j = 0; j < da.segnames.size(); ++j)
    {
        std::cout << "Segment " << j << ": ";
        MSG_data* d = da.segment(j);
        if (!d)
        {
            std::cout << "missing.\n";
            continue;
        } else {
            for (size_t k = 0; k < da.npixperseg; ++k)
            {
                if(d->image->data[k] > 0)
                {
                    container[j*da.npixperseg + k] = d->image->data[k];
                }
            }
            std::cout << std::flush;
        }
    }

    double a = PIE / snu_solar_distance_factor2(bs.day_of_year);
    double b = a / band_solar_irradiance[satid][channel - 1];
    const double c1 = 1.19104e-5; // [mW m^-2 sr^-1 (cm^-1)^-4
    const double c2 = 1.43877; // K (cm^-1)^-1

    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if(unit == SEVIRI_UNIT_REF || unit == SEVIRI_UNIT_BRF)
            {
                if (container[i_image] != FILL_VALUE_F && container[i_image] > 0)
                {
                    if(sza[i_image] >= 0. && sza[i_image] < 89.)
                    {
                        double R = container[i_image] * slope + offset;
                        container[i_image] = b * R;
                        if(unit == SEVIRI_UNIT_BRF)
                            container[i_image] /= cos(sza[i_image] * D2R);
                        container[i_image] = container[i_image] < 0 ? 0 : container[i_image];
                    }
                    else
                        container[i_image] = FILL_VALUE_F;
                }
            }
            else if(unit == SEVIRI_UNIT_BT)
            {

                double nu = bt_nu_c[satid][channel - 1];
                double nu3 = nu * nu * nu;

                double a = bt_A[satid][channel - 1];
                double b = bt_B[satid][channel - 1];

                if (container[i_image] != FILL_VALUE_F && container[i_image] > 0)
                {
                    double L = container[i_image] * slope + offset;

                    container[i_image] =  (c2 * nu / log(1. + nu3 * c1 / L) - b) / a;
                }
            }
            else if(unit == SEVIRI_UNIT_RAD)
            {
                if (container[i_image] != FILL_VALUE_F && container[i_image] > 0)
                {
                    double L = container[i_image] * slope + offset;
                    container[i_image] =  L;
                }
            }

        }

    }
}

void SegmentListGeostationary::ComposeSnowRGB(bandstorage &bs, double julian_day)
{
    // Red   VIS008    0 --> 100   % Gamma 1.7
    // Green IR_016    0 --> 70    % Gamma 1.7
    // Blue  IR3.9Refl 0 --> 30    % Gamma 1.7
    //VIS006 1
    //VIS008 2
    //IR_016 3
    //IR_039 4
    //WV_062 5
    //WV_073 6
    //IR_087 7
    //IR_097 8
    //IR_108 9
    //IR_120 10
    //IR_134 11

    int totalpixels = 3712 * 3712;

    float *ref008 = new float[totalpixels];
    float *ref016 = new float[totalpixels];
    float *rad039  = new float[totalpixels];
    float *rad039_from_bt108  = new float[totalpixels];
    float *rad039_corr  = new float[totalpixels];
    float *bt108  = new float[totalpixels];
    float *bt134  = new float[totalpixels];
    float *r_therm  = new float[totalpixels];
    float *toarad = new float[totalpixels];

    snu_init_array_f(ref008, totalpixels, FILL_VALUE_F);
    snu_init_array_f(ref016, totalpixels, FILL_VALUE_F);
    snu_init_array_f(rad039,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(rad039_from_bt108,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(rad039_corr,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(bt108, totalpixels, FILL_VALUE_F);
    snu_init_array_f(bt134,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(r_therm,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(toarad,  totalpixels, FILL_VALUE_F);

    GetRadBT(SEVIRI_UNIT_REF, 2, bs, ref008);

    emit progressCounter(45);

    GetRadBT(SEVIRI_UNIT_REF, 3, bs, ref016);

    emit progressCounter(48);

    GetRadBT(SEVIRI_UNIT_BT, 9, bs, bt108);

    emit progressCounter(50);

    GetRadBT(SEVIRI_UNIT_BT, 11, bs, bt134);

    emit progressCounter(55);

    GetRadBT(SEVIRI_UNIT_RAD, 4, bs, rad039);

    // Radiance from IR_039 channel = 4
    // using bt108 and parameters from IR_039
    int channel = 4;
    double a = bt_A[satid][channel - 1];
    double b = bt_B[satid][channel - 1];
    double nu = bt_nu_c[satid][channel - 1];
    double nu3 = nu * nu * nu;
    const double c1 = 1.19104e-5; // [mW m^-2 sr^-1 (cm^-1)^-4
    const double c2 = 1.43877; // K (cm^-1)^-1

    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (bt108[i_image] != FILL_VALUE_F && bt108[i_image] > 0)
            {
                rad039_from_bt108[i_image] =  c1 * nu3 / (exp( c2 * nu / (a * bt108[i_image] + b)) - 1);
            }
        }
    }

    emit progressCounter(60);

    //CO2 correction
    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (bt108[i_image] != FILL_VALUE_F && bt108[i_image] > 0)
            {
                rad039_corr[i_image] =  pow((bt108[i_image] - 0.25 * (bt108[i_image] - bt134[i_image])), 4.0) / pow(bt108[i_image], 4.0);
            }
        }
    }

    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (rad039_from_bt108[i_image] != FILL_VALUE_F && rad039_from_bt108[i_image] > 0 && rad039_corr[i_image] != FILL_VALUE_F && rad039_corr[i_image] > 0)
            {
                r_therm[i_image] =  rad039_from_bt108[i_image] * rad039_corr[i_image];
                if(r_therm[i_image] > rad039[i_image])
                    r_therm[i_image] = rad039[i_image];
            }
        }
    }

    //The CO2-corrected, solar constant at the Top of the Atmosphere
    //in Channel 04 (IR3.9)
    double ESD = 1.0 - 0.0167 * cos( 2 * PIE * (julian_day - 3)/ 365.0);
    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;
            if (rad039_corr[i_image] != FILL_VALUE_F && rad039_corr[i_image] > 0 && sza[i_image] < 89.0 && vza[i_image] < 89.0)
            {
                toarad[i_image] =  4.92 / ESD * ESD;
                float costeta = cos(sza[i_image] * D2R);
                float cossat = cos(vza[i_image] * D2R);
                toarad[i_image] *= costeta;

                toarad[i_image] *= exp(-(1.0 - rad039_corr[i_image]));
                toarad[i_image] *= exp(-(1.0 - rad039_corr[i_image]) * costeta / cossat);

            }
        }
    }

    emit progressCounter(65);

    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (rad039[i_image] != FILL_VALUE_F && rad039[i_image] > 0 && vza[i_image] < 89.0)
            {
                result[0][i_image] =  ref008[i_image];
                result[1][i_image] =  ref016[i_image];
                result[2][i_image] =  100 * (rad039[i_image] - r_therm[i_image]); // / cos(vza[i_image] * D2R);
            }
        }
    }

    delete [] ref008;
    delete [] ref016;
    delete [] rad039;
    delete [] rad039_from_bt108;
    delete [] rad039_corr;
    delete [] bt108;
    delete [] bt134;
    delete [] r_therm;
    delete [] toarad;

}

void SegmentListGeostationary::ComposeIR_039sunreflected(bandstorage &bs, double julian_day)
{

    //VIS006 1
    //VIS008 2
    //IR_016 3
    //IR_039 4
    //WV_062 5
    //WV_073 6
    //IR_087 7
    //IR_097 8
    //IR_108 9
    //IR_120 10
    //IR_134 11

    // REFL(IR3.9) = 100 * (Rtot - Rtherm)/(TOARAD - Rtherm)
    //
    int totalpixels = 3712 * 3712;

    float *rad039  = new float[totalpixels];
    float *rad039_from_bt108  = new float[totalpixels];
    float *rad039_corr  = new float[totalpixels];
    float *bt108  = new float[totalpixels];
    float *bt134  = new float[totalpixels];
    float *r_therm  = new float[totalpixels];
    float *toarad = new float[totalpixels];

    snu_init_array_f(rad039,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(rad039_from_bt108,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(rad039_corr,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(bt108, totalpixels, FILL_VALUE_F);
    snu_init_array_f(bt134,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(r_therm,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(toarad,  totalpixels, FILL_VALUE_F);


    emit progressCounter(45);

    qDebug() << "scanning ir_108";

    GetRadBT(SEVIRI_UNIT_BT, 9, bs, bt108);

    emit progressCounter(50);

    GetRadBT(SEVIRI_UNIT_BT, 11, bs, bt134);

    //PrintResults(bt134, "bt134");

    emit progressCounter(55);

    qDebug() << "get radiance for ir_039";

    GetRadBT(SEVIRI_UNIT_RAD, 4, bs, rad039);

    qDebug() << "calculating radiance ir_039 from bt108";

    // Radiance from IR_039 channel = 4
    // using bt108 and parameters from IR_039
    int channel = 4;
    double a = bt_A[satid][channel - 1];
    double b = bt_B[satid][channel - 1];
    double nu = bt_nu_c[satid][channel - 1];
    double nu3 = nu * nu * nu;
    const double c1 = 1.19104e-5; // [mW m^-2 sr^-1 (cm^-1)^-4
    const double c2 = 1.43877; // K (cm^-1)^-1

    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (bt108[i_image] != FILL_VALUE_F && bt108[i_image] > 0)
            {
                rad039_from_bt108[i_image] =  c1 * nu3 / (exp( c2 * nu / (a * bt108[i_image] + b)) - 1);
            }
        }
    }

    emit progressCounter(60);

    qDebug() << "calculating co2 correction";

    //CO2 correction
    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (bt108[i_image] != FILL_VALUE_F && bt108[i_image] > 0)
            {
                rad039_corr[i_image] =  pow((bt108[i_image] - 0.25 * (bt108[i_image] - bt134[i_image])), 4.0) / pow(bt108[i_image], 4.0);
            }
        }
    }

    qDebug() << "calculating r_therm";

    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (rad039_from_bt108[i_image] != FILL_VALUE_F && rad039_from_bt108[i_image] > 0 && rad039_corr[i_image] != FILL_VALUE_F && rad039_corr[i_image] > 0)
            {
                r_therm[i_image] =  rad039_from_bt108[i_image] * rad039_corr[i_image];
                if(r_therm[i_image] > rad039[i_image])
                    r_therm[i_image] = rad039[i_image];
            }
        }
    }

    qDebug() << "calculating co2 correction solar constant";

    //The CO2-corrected, solar constant at the Top of the Atmosphere
    //in Channel 04 (IR3.9)
    double ESD = 1.0 - 0.0167 * cos( 2 * PIE * (julian_day - 3)/ 365.0);
    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;
            if (rad039_corr[i_image] != FILL_VALUE_F && rad039_corr[i_image] > 0 && sza[i_image] < 89.0 && vza[i_image] < 89.0)
            {
                toarad[i_image] =  4.92 / ESD * ESD;
                float costeta = cos(sza[i_image] * D2R);
                float cossat = cos(vza[i_image] * D2R);
                toarad[i_image] *= costeta;

                toarad[i_image] *= exp(-(1.0 - rad039_corr[i_image]));
                toarad[i_image] *= exp(-(1.0 - rad039_corr[i_image]) * costeta / cossat);

            }
        }
    }


    emit progressCounter(65);

    qDebug() << "calculating result";

    for (int j = 0; j < 3712; ++j)
    {
        for (int k = 0; k < 3712; ++k)
        {
            int i_image = j * 3712 + k;

            if (rad039[i_image] != FILL_VALUE_F && rad039[i_image] > 0 && vza[i_image] < 89.0)
            {
                result[0][i_image] =  100 * (rad039[i_image] - r_therm[i_image]);
                result[1][i_image] =  result[0][i_image];
                result[2][i_image] =  result[0][i_image];
            }
        }
    }

    delete [] rad039;
    delete [] rad039_from_bt108;
    delete [] rad039_corr;
    delete [] bt108;
    delete [] bt134;
    delete [] r_therm;
    delete [] toarad;

}

void SegmentListGeostationary::Printbands()
{
    for(int k = 0; k < bands.length(); k++)
    {
        std::cout << "band " << k << " " << bands[k].productid2.toStdString() << std::endl;
        for (int i = 1000; i < 1000 + 10; ++i)
        {
            for (int j = 1856; j < 1856 + 10; ++j)
            {
                int i_image = i * 3712 + j;

                std::cout << bands[k].data[i_image] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    //    std::cout << "sza " << std::endl;
    //    for (int i = 1000; i < 1000 + 10; ++i)
    //    {
    //        for (int j = 1856; j < 1856 + 10; ++j)
    //        {
    //            int i_image = i * 3712 + j;

    //            std::cout << sza[i_image] << " ";
    //        }
    //        std::cout << std::endl;
    //    }

}

void SegmentListGeostationary::PrintResults()
{
    for(int k = 0; k < 3; k++)
    {
        std::cout << "color " << k << std::endl;
        for (int i = 1000; i < 1000 + 10; ++i)
        {
            for (int j = 1856; j < 1856 + 10; ++j)
            {
                int i_image = i * 3712 + j;

                std::cout << result[k][i_image] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

void SegmentListGeostationary::PrintResults(float *ptr, QString title)
{

    int fromY = 1856;
    int fromX = 20;
    std::cout << title.toStdString() << std::endl;
    for (int y = fromY; y < fromY + 10; ++y)
    {
        for (int x = fromX; x < fromX + 30; ++x)
        {
            int i_image = y * 3712 + x;

            std::cout << ptr[i_image] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl << std::flush;

}

void SegmentListGeostationary::CalculateGeoRadiances(bandstorage &bs)
{
    qDebug() << QString("ComposeGeoRGBRecipe %1 %2").arg(bs.listindex).arg(bs.productid2);

    MsgFileAccess fac(bs.directory, "H", bs.productid1, bs.productid2, bs.timing);
    MsgDataAccess da;
    MSG_header header;

    da.scan(fac, header);
    MSG_SAMPLE compmin = 0xffff, compmax = 0;
    for (size_t j = 0; j < da.segnames.size(); ++j)
    {
        std::cout << "Segment " << j << ": ";
        MSG_data* d = da.segment(j);
        MSG_SAMPLE min = 0xffff, max = 0;
        if (!d)
        {
            std::cout << "missing.\n";
            continue;
        } else {
            for (size_t k = 0; k < da.npixperseg; ++k)
            {
                if(d->image->data[k] > 0)
                {
                    bands[bs.listindex].data[j*da.npixperseg + k] = d->image->data[k];
                    if (d->image->data[k] < min) min = d->image->data[k];
                    if (d->image->data[k] > max) max = d->image->data[k];
                }
            }
            std::cout << "min " << min << " max " << max << std::endl;
        }
        if (min < compmin) compmin = min;
        if (max > compmax) compmax = max;
    }
    std::cout << "compmin = " << compmin << " compmax = " << compmax << std::endl;
    bands[bs.listindex].min = (float)compmin;
    bands[bs.listindex].max = (float)compmax;


    /*-------------------------------------------------------------------------
    * Compute radiance for the bands requested.
    *
    * Ref: PDF_TEN_05105_MSG_IMG_DATA, Page 26
    *-----------------------------------------------------------------------*/

    double slope, offset;
    bool toint;

    if(bands[bs.listindex].units == SEVIRI_UNIT_RAD) // mW*pow(m,-2)*pow(sr,-1)*pow(pow(cm,-1)), -1)
    {
        bands[bs.listindex].min = std::numeric_limits<float>::max();
        bands[bs.listindex].max = std::numeric_limits<float>::min();
        for (int j = 0; j < 3712; ++j)
        {
            for (int k = 0; k < 3712; ++k)
            {
                int i_image = j * 3712 + k;

                if (bands[bs.listindex].data[i_image] != FILL_VALUE_F && bands[bs.listindex].data[i_image] > 0)
                {
                    bands[bs.listindex].data[i_image] = bands[bs.listindex].data[i_image] * bs.slope + bs.offset;
                    bands[bs.listindex].data[i_image] = bands[bs.listindex].data[i_image] < 0 ? 0 : bands[bs.listindex].data[i_image];
                    if(bands[bs.listindex].data[i_image] < bands[bs.listindex].min) bands[bs.listindex].min = bands[bs.listindex].data[i_image];
                    if(bands[bs.listindex].data[i_image] > bands[bs.listindex].max) bands[bs.listindex].max = bands[bs.listindex].data[i_image];
                }
            }
        }
    }


    if(bands[bs.listindex].units == SEVIRI_UNIT_RAD)
    {
        qDebug() << QString("Radiances %1 minimum = %2 maximum = %3").arg(bands[bs.listindex].productid2).arg(bands[bs.listindex].min).arg(bands[bs.listindex].max);
    }

    /*-------------------------------------------------------------------------
    * Compute reflectance or bidirectional reflectance factor (BRF) for the
    * bands requested.
    *
    * Ref: PDF_MSG_SEVIRI_RAD2REFL, Page 8
    *-----------------------------------------------------------------------*/
    //double dd = 1. / sqrt(snu_solar_distance_factor2(day_of_year)); // distance earth-sun
    //double a = PIE * dd * dd;
    double a = PIE / snu_solar_distance_factor2(bs.day_of_year);

    int satid =  (int)header.segment_id->spacecraft_id - 321 ;

    if (bands[bs.listindex].units == SEVIRI_UNIT_REF || bands[bs.listindex].units == SEVIRI_UNIT_BRF)
    {
        bands[bs.listindex].min = std::numeric_limits<float>::max();
        bands[bs.listindex].max = std::numeric_limits<float>::min();

        double b = a / band_solar_irradiance[satid][bands[bs.listindex].spectral_channel_nbr - 1];

        for (int j = 0; j < 3712; ++j)
        {
            for (int k = 0; k < 3712; ++k)
            {
                int i_image = j * 3712 + k;

                if (bands[bs.listindex].units == SEVIRI_UNIT_BRF && bands[bs.listindex].data[i_image] != FILL_VALUE_F && bands[bs.listindex].data[i_image] > 0)
                {
                    if(sza[i_image] >= 0. && sza[i_image] < 89.0)
                    {
                        double R = bands[bs.listindex].data[i_image] * bs.slope + bs.offset;

                        bands[bs.listindex].data[i_image] = b * R;

                        if (bands[bs.listindex].units == SEVIRI_UNIT_BRF)
                            bands[bs.listindex].data[i_image] /= cos(sza[i_image] * D2R);

                        bands[bs.listindex].data[i_image] = bands[bs.listindex].data[i_image] < 0.0 ? 0.0 : bands[bs.listindex].data[i_image];

                        if(bands[bs.listindex].data[i_image] < bands[bs.listindex].min) bands[bs.listindex].min = bands[bs.listindex].data[i_image];
                        if(bands[bs.listindex].data[i_image] > bands[bs.listindex].max) bands[bs.listindex].max = bands[bs.listindex].data[i_image];
                    }
                    else
                        bands[bs.listindex].data[i_image] = FILL_VALUE_F;
                }
                else if (bands[bs.listindex].units == SEVIRI_UNIT_REF && bands[bs.listindex].data[i_image] != FILL_VALUE_F && bands[bs.listindex].data[i_image] > 0)
                {
                    double R = bands[bs.listindex].data[i_image] * bs.slope + bs.offset;

                    bands[bs.listindex].data[i_image] = b * R;

                    if(bands[bs.listindex].data[i_image] < bands[bs.listindex].min) bands[bs.listindex].min = bands[bs.listindex].data[i_image];
                    if(bands[bs.listindex].data[i_image] > bands[bs.listindex].max) bands[bs.listindex].max = bands[bs.listindex].data[i_image];
                }
            }
        }
    }

    const double c1 = 1.19104e-5; // [mW m^-2 sr^-1 (cm^-1)^-4
    const double c2 = 1.43877; // K (cm^-1)^-1

    /*-------------------------------------------------------------------------
    * Compute brightness temperature for the bands requested.
    *
    * Ref: PDF_TEN_05105_MSG_IMG_DATA, Page 26
    * Ref: The Conversion from Effective Radiances
    *      to Equivalent Brightness Temperatures (EUM/MET/TEN/11/0569)
    *-----------------------------------------------------------------------*/
    if (bands[bs.listindex].units == SEVIRI_UNIT_BT)
    {
        bands[bs.listindex].min = std::numeric_limits<float>::max();
        bands[bs.listindex].max = std::numeric_limits<float>::min();

        qDebug() << bands[bs.listindex].spectral_channel_nbr << " " << bands[bs.listindex].productid2 << " slope = " << bs.slope << " offset = " << bs.offset;
        /*
             nu = 1.e4 / channel_center_wavelength[d->image.band_ids[bs.listindex] - 1];
*/
        double nu = bt_nu_c[satid][bands[bs.listindex].spectral_channel_nbr - 1];
        double nu3 = nu * nu * nu;

        double a = bt_A[satid][bands[bs.listindex].spectral_channel_nbr - 1];
        double b = bt_B[satid][bands[bs.listindex].spectral_channel_nbr - 1];

        for (int j = 0; j < 3712; ++j)
        {
            for (int k = 0; k < 3712; ++k)
            {
                int i_image = j * 3712 + k;

                if (bands[bs.listindex].data[i_image] != FILL_VALUE_F && bands[bs.listindex].data[i_image] > 0)
                {
                    double L = bands[bs.listindex].data[i_image] * bs.slope + bs.offset;

                    bands[bs.listindex].data[i_image] =  (c2 * nu / log(1. + nu3 * c1 / L) - b) / a;

                    if(bands[bs.listindex].data[i_image] < bands[bs.listindex].min) bands[bs.listindex].min = bands[bs.listindex].data[i_image];
                    if(bands[bs.listindex].data[i_image] > bands[bs.listindex].max) bands[bs.listindex].max = bands[bs.listindex].data[i_image];
                }
            }
        }
    }

    qDebug() << QString("%1 minimum = %2 maximum = %3").arg(bands[bs.listindex].productid2).arg(bands[bs.listindex].min).arg(bands[bs.listindex].max);

}

void SegmentListGeostationary::CalculateLonLat()
{
    unsigned int i_image;
    int totalpixels = 3712 * 3712;

    lon = new float[totalpixels];
    lat = new float[totalpixels];
    snu_init_array_f(lon,  totalpixels, FILL_VALUE_F);
    snu_init_array_f(lat,  totalpixels, FILL_VALUE_F);

    pixgeoConversion pixconv;

    double templat;
    double templon;


    //    for (int y = 0; y < 3712; ++y)
    //    {
    //        for (int x = 0; x < 3712; ++x)
    //        {
    //            i_image = y * 3712 + x;

    //            if(pixconv.pixcoord2geocoord( lon0, x, y, nav_scaling_factors_vir.COFF, nav_scaling_factors_vir.LOFF,
    //                                          nav_scaling_factors_vir.CFAC, nav_scaling_factors_vir.LFAC, &templat, &templon) == 0)
    //            {
    //                lat[i_image] = templat;
    //                lon[i_image] = templon;
    //            }
    //            else
    //            {
    //                lat[i_image] = FILL_VALUE_F;
    //                lon[i_image] = FILL_VALUE_F;

    //            }
    //        }
    //    }


    for (int y = 0; y < 3712; ++y)
    {
        for (int x = 0; x < 3712; ++x)
        {
            i_image = y * 3712 + x;

            snu_line_column_to_lat_lon(y, x, &lat[i_image], &lon[i_image], lon0, &nav_scaling_factors_vir);
        }
    }


}

QImage *SegmentListGeostationary::CalculateBitMap()
{
    // bitmap created for inverse infrared images
    imageptrs->ptrimagebitmap = new QImage(imageptrs->ptrimageGeostationary->width(), imageptrs->ptrimageGeostationary->height(), QImage::Format_ARGB32);
    QPainter painter(imageptrs->ptrimagebitmap);

//    painter.translate(imageptrs->ptrimageGeostationary->width()/2, imageptrs->ptrimageGeostationary->height()/2);

//    int diameter = (m_GeoSatellite != eGeoSatellite::H9 ? 3630 : 5445); // 3615;
//    painter.setPen(Qt::NoPen);
//    painter.setBrush(Qt::red);
//    painter.drawEllipse(QRect(-diameter / 2, -diameter / 2, diameter, diameter));

//    diameter = (m_GeoSatellite != eGeoSatellite::H9 ? 3600 : 5400); // 3615;
//    //painter.setPen(QPen(QColor(1,1,1)));
//    painter.setBrush(QColor(1,1,1));
//    painter.drawEllipse(QRect(-diameter / 2, -diameter / 2, diameter, diameter));

    QPoint pt(opts.geosatellites.at(geoindex).coff, opts.geosatellites.at(geoindex).loff);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(1,1,1));
    painter.drawEllipse(pt, opts.geosatellites.at(geoindex).coff - 28, opts.geosatellites.at(geoindex).loff - 40);


    return(imageptrs->ptrimagebitmap);
}
