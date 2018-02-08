#include <QApplication>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include "segmentlistgeostationary.h"
#include "segmentimage.h"
#include "qcompressor.h"
#include "pixgeoconversion.h"

#ifdef _WIN32
#include <hdf5.h>
#else
#include <hdf5/serial/hdf5.h>
#endif
#include <netcdf.h>

#include "MSG_HRIT.h"
#include <QMutex>

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

void SegmentListGeostationary::doComposeGeostationaryXRIT(SegmentListGeostationary *sm, QString segment_path, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    sm->ComposeSegmentImageXRIT(segment_path, channelindex, spectrumvector, inversevector);
}

void SegmentListGeostationary::doComposeGeostationaryXRITHimawari(SegmentListGeostationary *sm, QString segment_path, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    sm->ComposeSegmentImageXRITHimawari(segment_path, channelindex, spectrumvector, inversevector);
}

/*
void doComposeGeostationaryHDF(SegmentListGeostationary *sm, QString segment_path, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    sm->ComposeSegmentImageHDF(segment_path, channelindex, spectrumvector, inversevector);
}
*/

void SegmentListGeostationary::doComposeGeostationaryHDFInThread(SegmentListGeostationary *sm, QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    sm->ComposeSegmentImageHDFInThread(filelist, spectrumvector, inversevector);
}

void SegmentListGeostationary::doComposeGeostationarynetCDFInThread(SegmentListGeostationary *sm) //, QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod)
{
    sm->ComposeSegmentImagenetCDFInThread(); //strlist, spectrumvector, inversevector, histogrammethod);
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
    ResetSegments();
    this->bActiveSegmentList = false;
    this->bisRSS = false;
    this->SetupContrastStretch( 0, 0, 1023, 255);

    this->time = NULL;		/* image of Julian Day Number */
    this->lat = NULL;  		/* image of latitude */
    this->lon = NULL;  		/* image of longitude */
    this->sza = NULL;  		/* image of solar zenith angle (degrees: 0.0 -- 180.0) */
    this->saa = NULL;  		/* image of solar azimuth angle  (degrees: 0.0 -- 360.0) */
    this->vza = NULL;       /* image of viewing zenith angle (degrees: 0.0 -- 180.0) */
    this->vaa = NULL;       /* image of viewing azimuth angle  (degrees: 0.0 -- 360.0) */
    this->data = NULL;      /* array of pointers to images of length n_bands */
    this->data2 = NULL;      /* array of image data of length n_bands * n_lines * n_columns */



}


eGeoSatellite SegmentListGeostationary::getGeoSatellite()
{
    if(str_GeoSatellite == "MET_11")
        return eGeoSatellite::MET_11;
    if(str_GeoSatellite == "MET_10")
        return eGeoSatellite::MET_10;
    else if(str_GeoSatellite == "MET_9")
        return eGeoSatellite::MET_9;
    else if(str_GeoSatellite == "MET_8")
        return eGeoSatellite::MET_8;
    else if(str_GeoSatellite == "GOMS2")
        return eGeoSatellite::GOMS2;
    else if(str_GeoSatellite == "FY2E")
        return eGeoSatellite::FY2E;
    else if(str_GeoSatellite == "FY2G")
        return eGeoSatellite::FY2G;
    else if(str_GeoSatellite == "GOES_15")
        return eGeoSatellite::GOES_15;
    else if(str_GeoSatellite == "GOES_16")
        return eGeoSatellite::GOES_16;
    else if(str_GeoSatellite == "H8")
        return eGeoSatellite::H8;
    else
        return eGeoSatellite::NOGEO;
}

void SegmentListGeostationary::setGeoSatellite(int geoindex, QString strgeo)
{
    this->geoindex = geoindex;
    str_GeoSatellite = opts.geosatellites.at(geoindex).shortname;

    if(strgeo == "MET_11")
    {
        this->m_GeoSatellite = eGeoSatellite::MET_11;
    }
    else if(strgeo == "MET_10")
    {
        this->m_GeoSatellite = eGeoSatellite::MET_10;
    }
    else if(strgeo == "MET_9")
    {
        this->m_GeoSatellite = eGeoSatellite::MET_9;
    }
    else if(strgeo == "MET_8")
    {
        this->m_GeoSatellite = eGeoSatellite::MET_8;
    }
    else if(strgeo == "GOMS2")
    {
        this->m_GeoSatellite = eGeoSatellite::GOMS2;
    }
    else if(strgeo == "FY2E")
    {
        this->m_GeoSatellite = eGeoSatellite::FY2E;
    }
    else if(strgeo == "FY2G")
    {
        this->m_GeoSatellite = eGeoSatellite::FY2G;
    }
    else if(strgeo == "GOES_15")
    {
        this->m_GeoSatellite = eGeoSatellite::GOES_15;
    }
    else if(strgeo == "GOES_16")
    {
        this->m_GeoSatellite = eGeoSatellite::GOES_16;
    }
    else if(strgeo == "H8")
    {
        this->m_GeoSatellite = eGeoSatellite::H8;
    }
}

void SegmentListGeostationary::ResetSegments()
{
    for( int i = 0; i < 10; i++)
    {
        issegmentcomposedRed[i] = false;
        issegmentcomposedGreen[i] = false;
        issegmentcomposedBlue[i] = false;
        isPresentRed[i] = false;
        isPresentGreen[i] = false;
        isPresentBlue[i] = false;
    }

    for( int i = 0; i < 24; i++)
    {
        issegmentcomposedHRV[i] = false;
        isPresentHRV[i] = false;
    }
}

void SegmentListGeostationary::getFilenameParameters(QFileInfo fileinfo, QString &filespectrum, QString &filedate, int &filesequence)
{

    int index = opts.geosatellites.at(this->geoindex).indexspectrumhrv;
    int length = opts.geosatellites.at(this->geoindex).spectrumhrv.length();
    QString spectrum = fileinfo.fileName().mid(index, length);
    if(spectrum.length() > 0 && spectrum == opts.geosatellites.at(geoindex).spectrumhrv)
    {
        filespectrum = spectrum;
        filedate = fileinfo.fileName().mid(opts.geosatellites.at(this->geoindex).indexdatehrv, opts.geosatellites.at(this->geoindex).lengthdatehrv);
        filesequence = fileinfo.fileName().mid(opts.geosatellites.at(this->geoindex).indexfilenbrhrv, opts.geosatellites.at(this->geoindex).lengthfilenbrhrv).toInt()-1;
    }
    else
    {
        for(int i = 0; i < opts.geosatellites.at(this->geoindex).spectrumlist.count(); i++)
        {
            spectrum = fileinfo.fileName().mid(opts.geosatellites.at(this->geoindex).indexspectrum, opts.geosatellites.at(this->geoindex).spectrumlist.at(i).length());
            if(spectrum.length() > 0 && spectrum == opts.geosatellites.at(geoindex).spectrumlist.at(i))
            {
                filespectrum = spectrum;
                filedate = fileinfo.fileName().mid(opts.geosatellites.at(this->geoindex).indexdate, opts.geosatellites.at(this->geoindex).lengthdate);
                filesequence = fileinfo.fileName().mid(opts.geosatellites.at(this->geoindex).indexfilenbr, opts.geosatellites.at(this->geoindex).lengthfilenbr).toInt()-1;
                break;
            }
        }
    }

}

bool SegmentListGeostationary::ComposeImageXRIT(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod)
{
    /* qDebug() << QString("ideal threadcount = %1  max threadcount = %2 active threadcount = %3").
                arg(QThread::idealThreadCount()).
                arg(QThreadPool::globalInstance()->maxThreadCount()).
                arg(QThreadPool::globalInstance()->activeThreadCount());

    */
    this->histogrammethod = histogrammethod;
    this->spectrumvector = spectrumvector;
    this->inversevector = inversevector;

    int filesequence = fileinfo.fileName().mid(36, 6).toInt()-1;
    QString filespectrum = fileinfo.fileName().mid(26, 6);
    QString filedate = fileinfo.fileName().mid(46, 12);

    getFilenameParameters(fileinfo, filespectrum, filedate, filesequence);

    qDebug() << QString("SegmentListGeostationary::ComposeImageXRIT filePath = %1 filespectrum = %2").arg(fileinfo.filePath()).arg(filespectrum);

    if( filespectrum  == "HRV")
    {
        QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
    }
    else if( m_GeoSatellite == eGeoSatellite::GOES_15 )
    {
        QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
    }
    else if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_9 || m_GeoSatellite == eGeoSatellite::MET_8 || m_GeoSatellite == eGeoSatellite::GOMS2)
    {
        if( spectrumvector.at(1) == "" && spectrumvector.at(2) == "")
        {
            QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
        }
        else
        {
            if(spectrumvector.at(0) == filespectrum)
            {
                QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
            }
            else if(spectrumvector.at(1) == filespectrum)
            {
                QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 1, spectrumvector, inversevector);
            }
            else if(spectrumvector.at(2) == filespectrum)
            {
                QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 2, spectrumvector, inversevector);
            }
        }
    }
    else if(m_GeoSatellite == eGeoSatellite::H8)
    {
        filesequence = fileinfo.fileName().mid(25, 3).toInt()-1;
        filespectrum = fileinfo.fileName().mid(8, 3);
        filedate = fileinfo.fileName().mid(12, 11) + "0";

        if( spectrumvector.at(1) == "" && spectrumvector.at(2) == "")
        {
            QtConcurrent::run(doComposeGeostationaryXRITHimawari, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
        }
        else
        {
            if(spectrumvector.at(0) == filespectrum)
            {
                QtConcurrent::run(doComposeGeostationaryXRITHimawari, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
            }
            else if(spectrumvector.at(1) == filespectrum)
            {
                QtConcurrent::run(doComposeGeostationaryXRITHimawari, this, fileinfo.filePath(), 1, spectrumvector, inversevector);
            }
            else if(spectrumvector.at(2) == filespectrum)
            {
                QtConcurrent::run(doComposeGeostationaryXRITHimawari, this, fileinfo.filePath(), 2, spectrumvector, inversevector);
            }
        }
    }

    return true;
}

bool SegmentListGeostationary::ComposeImageHDFInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector)
{

    //"Z_SATE_C_BABJ_20150809101500_O_FY2E_FDI_IR1_001_NOM.HDF.gz"
    qDebug() << QString("SegmentListGeostationary::ComposeImageHDFInThread spectrumvector = %1 %2 %3").arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));

    QApplication::setOverrideCursor(( Qt::WaitCursor));
    QtConcurrent::run(doComposeGeostationaryHDFInThread, this, strlist, spectrumvector, inversevector);
    return true;
}

void SegmentListGeostationary::setThreadParameters(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor)
{
    this->segmentfilelist = strlist;
    this->spectrumvector = spectrumvector;
    this->inversevector = inversevector;
    this->histogrammethod = histogrammethod;
    this->pseudocolor = pseudocolor;

}

bool SegmentListGeostationary::ComposeImagenetCDFInThread(QStringList strlist, QVector<QString> spectrumvector, QVector<bool> inversevector, int histogrammethod, bool pseudocolor)
{

    qDebug() << QString("SegmentListGeostationary::ComposeImagenetCDFInThread spectrumvector = %2 %3 %4").arg(spectrumvector.at(0)).arg(spectrumvector.at(1)).arg(spectrumvector.at(2));


    QApplication::setOverrideCursor(( Qt::WaitCursor));
    setThreadParameters(strlist, spectrumvector, inversevector, histogrammethod, pseudocolor);
    QtConcurrent::run(doComposeGeostationarynetCDFInThread, this); //, strlist, spectrumvector, inversevector, histogrammethod);
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


void SegmentListGeostationary::InsertPresent( QVector<QString> spectrumvector, QString filespectrum, int filesequence)
{
    qDebug() << QString("InsertPresent ; spectrum %1 %2 %3    filespectrum  %4  fileseq %5").arg(spectrumvector[0]).arg(spectrumvector[1]).arg(spectrumvector[2]).arg(filespectrum).arg(filesequence);
    if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_9 || m_GeoSatellite == eGeoSatellite::MET_8 || m_GeoSatellite == eGeoSatellite::H8 || m_GeoSatellite == eGeoSatellite::GOES_16 || m_GeoSatellite == eGeoSatellite::GOMS2)
    {
        if(spectrumvector.at(0) == filespectrum)
        {
            isPresentRed[filesequence] = true;
        }
        else if(spectrumvector.at(1) == filespectrum)
        {
            isPresentGreen[filesequence] = true;
        }
        else if(spectrumvector.at(2) == filespectrum)
        {
            isPresentBlue[filesequence] = true;
        }
        else if(filespectrum == "HRV")
        {
            isPresentHRV[filesequence] = true;
        }
    }
    else if(m_GeoSatellite == eGeoSatellite::GOES_15)
    {
        isPresentRed[filesequence] = true;
    }
    else if(m_GeoSatellite == eGeoSatellite::FY2E || m_GeoSatellite == eGeoSatellite::FY2G)
    {
        if(filespectrum == "VIS1KM")
            isPresentHRV[filesequence] = true;
        else
            isPresentRed[filesequence] = true;
    }

}



void SegmentListGeostationary::ComposeSegmentImageXRIT( QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    MSG_header *header;
    MSG_data *msgdat;


    header = new MSG_header();
    msgdat = new MSG_data();


    QFile file(filepath);
    QFileInfo fileinfo(file);

    int filesequence;
    QString filespectrum;
    QString filedate;
    this->getFilenameParameters(fileinfo, filespectrum, filedate, filesequence);

    qDebug() << QString("-------> SegmentListGeostationary::ComposeSegmentImageXRIT() filespectrum = %1 filedate = %2 filesequence = %3")
                .arg(filespectrum).arg(filedate).arg(filesequence);

    //QByteArray ba = filepath.toLatin1();
    //const char *c_segname = ba.data();

    std::ifstream hrit(filepath.toStdString(), (std::ios::binary | std::ios::in) );
    if (hrit.fail())
    {
        std::cerr << "Cannot open input hrit file "
            << filepath.toStdString() << std::endl;
        return;
    }

    header->read_from(hrit);
    msgdat->read_from(hrit, *header);
    hrit.close();

    //cout << *header;

    if (header->segment_id->data_field_format == MSG_NO_FORMAT)
    {
      qDebug() << "Product dumped in binary format.";
      return;
    }

    int planned_end_segment = header->segment_id->planned_end_segment_sequence_number;

    int npix = number_of_columns = header->image_structure->number_of_columns;
    int nlin = number_of_lines = header->image_structure->number_of_lines;
    size_t npixperseg = number_of_columns*number_of_lines;

    qDebug() << QString("---->[%1] SegmentListGeostationary::ComposeSegmentImageXRIT() planned end = %2 npix = %3 nlin = %4 filesequence = %5").arg(kindofimage).arg(planned_end_segment).arg(number_of_columns).arg(number_of_lines).arg(filesequence);

    MSG_SAMPLE *pixels = new MSG_SAMPLE[npixperseg];
    memset(pixels, 0, npixperseg*sizeof(MSG_SAMPLE));
    memcpy(pixels, msgdat->image->data, npixperseg*sizeof(MSG_SAMPLE));

    QImage *im;
    im = imageptrs->ptrimageGeostationary;

    quint16 c;

    if (filespectrum == "HRV")
    {
        imageptrs->ptrHRV[filesequence] = new quint16[number_of_lines * number_of_columns];
        memset(imageptrs->ptrHRV[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
    }
    else if(m_GeoSatellite == eGeoSatellite::GOES_15)
    {
        imageptrs->ptrRed[filesequence] = new quint16[number_of_lines * number_of_columns];
        memset(imageptrs->ptrRed[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
    }
    else
    {
        if(channelindex == 0)
        {
            imageptrs->ptrRed[filesequence] = new quint16[number_of_lines * number_of_columns];
            memset(imageptrs->ptrRed[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
        }
        else if(channelindex == 1)
        {
            imageptrs->ptrGreen[filesequence] = new quint16[number_of_lines * number_of_columns];
            memset(imageptrs->ptrGreen[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
        }
        else if(channelindex == 2)
        {
            imageptrs->ptrBlue[filesequence] = new quint16[number_of_lines * number_of_columns];
            memset(imageptrs->ptrBlue[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
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

    if (filespectrum == "HRV")
    {
        this->issegmentcomposedHRV[filesequence] = true;
    }
    else if(m_GeoSatellite == eGeoSatellite::GOES_15)
    {
        this->issegmentcomposedRed[filesequence] = true;
    }
    else
    {
        if(channelindex == 0)
            this->issegmentcomposedRed[filesequence] = true;
        else if(channelindex == 1)
            this->issegmentcomposedGreen[filesequence] = true;
        else if(channelindex == 2)
            this->issegmentcomposedBlue[filesequence] = true;

        qDebug() << QString("channelindex = %1 filesequence = %2 ").arg(channelindex).arg(filesequence);
    }

    if((kindofimage == "HRV" || kindofimage == "HRV Color") && allHRVColorSegmentsReceived())
    {
        qDebug() << "-----> HRV | HRV Color and allHRVColorSegmentsReceived";
        this->ComposeHRV();
        emit signalcomposefinished(kindofimage);
    }

    if((kindofimage == "VIS_IR" || kindofimage == "VIS_IR Color") && allSegmentsReceived())
    {
        qDebug() << "-----> VIS_IR Color | VIS_IR | HRV and allSegmentsReceived";
        this->ComposeVISIR();
        emit signalcomposefinished(kindofimage);
    }

    delete header;
    delete msgdat;
    delete [ ] pixels;

}

void SegmentListGeostationary::ComposeSegmentImageXRITHimawari( QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector )
{
//IMG_DK01B04_201510090000_001.bz2
//012345678901234567890123456789

    MSG_header *header;
    MSG_data *msgdat;
    int     nBuf;
    char    buf[ 32768 ];
    BZFILE* bzfile;
    int     bzerror;

    qDebug() << QString("-------> SegmentListGeostationary::ComposeSegmentImageHimawari() %1").arg(filepath);

    header = new MSG_header();
    msgdat = new MSG_data();


    QFile filein(filepath);
    QFileInfo fileinfo(filein);
    QString basename = fileinfo.baseName();


    int filesequence = fileinfo.fileName().mid(25, 3).toInt()-1;
    QString filespectrum = fileinfo.fileName().mid(8, 3);
    QString filedate = fileinfo.fileName().mid(12, 11) + "0";

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
      return;
    }

    int planned_end_segment = 10;

    int npix = number_of_columns = header->image_structure->number_of_columns;
    int nlin = number_of_lines = header->image_structure->number_of_lines;
    size_t npixperseg = number_of_columns*number_of_lines;

    qDebug() << QString("---->[%1] SegmentListGeostationary::ComposeSegmentImageXRITHimawari() planned end = %2 npix = %3 nlin = %4 filesequence = %5 channelindex = %6").arg(kindofimage).arg(planned_end_segment).arg(number_of_columns).arg(number_of_lines).arg(filesequence).arg(channelindex);

    MSG_SAMPLE *pixels = new MSG_SAMPLE[npixperseg];
    memset(pixels, 0, npixperseg*sizeof(MSG_SAMPLE));
    memcpy(pixels, msgdat->image->data, npixperseg*sizeof(MSG_SAMPLE));

    for(int i = 0; i < npixperseg; i++)
        pixels[i] =  BYTE_SWAP2(pixels[i]);

    QImage *im;
    im = imageptrs->ptrimageGeostationary;

    quint16 c;

    if(channelindex == 0)
    {
        imageptrs->ptrRed[filesequence] = new quint16[number_of_lines * number_of_columns];
        memset(imageptrs->ptrRed[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
    }
    else if(channelindex == 1)
    {
        imageptrs->ptrGreen[filesequence] = new quint16[number_of_lines * number_of_columns];
        memset(imageptrs->ptrGreen[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
    }
    else if(channelindex == 2)
    {
        imageptrs->ptrBlue[filesequence] = new quint16[number_of_lines * number_of_columns];
        memset(imageptrs->ptrBlue[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
    }

    quint16 maxpic = 0;
    quint16 minpic = 65535;

    for(int line = 0; line < nlin; line++)
    {
        for (int pixelx = 0 ; pixelx < npix; pixelx++)
        {
            c = *(pixels + line * npix + pixelx);
            if(maxpic < c)
                maxpic = c;
            if(minpic > c)
                minpic = c;
            if(channelindex == 0)
                *(imageptrs->ptrRed[filesequence] + line * npix + pixelx) = c;
            else if(channelindex == 1)
                *(imageptrs->ptrGreen[filesequence] + line * npix + pixelx) = c;
            else if(channelindex == 2)
                *(imageptrs->ptrBlue[filesequence] + line * npix + pixelx) = c;

        }
    }


    if(channelindex == 0)
        this->issegmentcomposedRed[filesequence] = true;
    else if(channelindex == 1)
        this->issegmentcomposedGreen[filesequence] = true;
    else if(channelindex == 2)
        this->issegmentcomposedBlue[filesequence] = true;



    if(allSegmentsReceived())
    {
        ComposeVISIRHimawari();
        emit signalcomposefinished(kindofimage);
    }

    delete header;
    delete msgdat;
    delete [ ] pixels;


}

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

    qDebug() << "====================start";

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

    qDebug() << "==============================end";

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
        imageptrs->ptrRed[0] = new quint16[9152 * 9152];
        memset(imageptrs->ptrRed[0], 0, 9152 * 9152 * sizeof(quint16));
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
                                 H5P_DEFAULT, imageptrs->ptrRed[0])) < 0)
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
        CalculateMinMax(9152, 9152, imageptrs->ptrRed[0], stat_min[0], stat_max[0]);
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

    qDebug() << "====================start";

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
            else if(kindofimage == "VIS_IR" || kindofimage == "HRV")
            {
                rc = *(imageptrs->ptrRed[0] + line * npix + pixelx);

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

    qDebug() << "==============================end";

    this->issegmentcomposedRed[0] = true;

    emit this->progressCounter(100);

    for(int j = 0; j < filelist.size(); j++)
    {

        H5Dclose(nomfileinfo_id[j]);
        H5Dclose(nomchannel_id[j]);
        H5Fclose(h5_file_id[j]);
    }

    emit signalcomposefinished(kindofimage);

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
    size_t xdim, ydim;
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

        qDebug() << "Starting netCDF file " + ncfile[j];
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
        if (retval != NC_NOERR) qDebug() << "error reading _FillValue";


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
                CalculateLUTGeo(j);
                normalizeMinMaxGOES16(xdim, ydim, imageptrs->ptrRed[0], stat_min[0], stat_max[0], fillvalue[0], 1023);
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
                CalculateLUTGeo(j);
                normalizeMinMaxGOES16(xdim, ydim, imageptrs->ptrGreen[0], stat_min[1], stat_max[1], fillvalue[1], 1023);
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
                CalculateLUTGeo(j);
                normalizeMinMaxGOES16(xdim, ydim, imageptrs->ptrBlue[0], stat_min[2], stat_max[2], fillvalue[2], 1023);
            }
        }

        long cnt_dqf0, cnt_dqf1, cnt_dqf2, cnt_dqf3;
        cnt_dqf0 = 0;
        cnt_dqf1 = 0;
        cnt_dqf2 = 0;
        cnt_dqf3 = 0;


        retval = nc_inq_varid(ncfileid[j], "DQF", &varid);
        if (retval != NC_NOERR) qDebug() << "error reading DQF id";

        imageptrs->ptrDQF[j] = new qint8[xdim * ydim];
        memset(imageptrs->ptrDQF[j], 0, xdim * ydim * sizeof(qint8));
        retval = nc_get_var(ncfileid[j], varid, imageptrs->ptrDQF[0]);
        if (retval != NC_NOERR)
            qDebug() << "error reading DQF values";
        else
        {
            qDebug() << "No errors reading DQF values";
            for(int i = 0; i < xdim * ydim; i++)
            {
                if( imageptrs->ptrDQF[j][i] == 0)
                    cnt_dqf0++;
                else if( imageptrs->ptrDQF[j][i] == 1)
                    cnt_dqf1++;
                else if( imageptrs->ptrDQF[j][i] == 2)
                    cnt_dqf2++;
                else if( imageptrs->ptrDQF[j][i] == 3)
                    cnt_dqf3++;

            }
            qDebug() << QString("# DQF 0 = %1").arg(cnt_dqf0);
            qDebug() << QString("# DQF 1 = %1").arg(cnt_dqf1);
            qDebug() << QString("# DQF 2 = %1").arg(cnt_dqf2);
            qDebug() << QString("# DQF 3 = %1").arg(cnt_dqf3);


        }



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
        this->geosatlon = nominal_satellite_subpoint_lon;
        opts.geosatellites[(int)eGeoSatellite::GOES_16].longitude = nominal_satellite_subpoint_lon;

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
    qint8 dqfvalue[3];

    imageptrs->InitializeImageGeostationary(xdim, ydim);
    im = imageptrs->ptrimageGeostationary;

    int progcounter = 30;
    emit this->progressCounter(progcounter);


    if(this->pseudocolor)
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

                dqfvalue[0] = *(imageptrs->ptrDQF[0] + line * xdim + pixelx);
                dqfvalue[1] = *(imageptrs->ptrDQF[1] + line * xdim + pixelx);
                dqfvalue[2] = *(imageptrs->ptrDQF[2] + line * xdim + pixelx);

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
                        indexoutrc = rc;
                    if(gc != fillvalue[1])
                        indexoutgc = gc;
                    if(bc != fillvalue[2])
                        indexoutbc = bc;
                }
                else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( rc != fillvalue[0]) indexoutrc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[0][rc]), 0), 1023);
                    if( gc != fillvalue[1]) indexoutgc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[1][gc]), 0), 1023);
                    if( bc != fillvalue[2]) indexoutbc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[2][bc]), 0), 1023);
                }

                if( (rc == fillvalue[0] && dqfvalue[0] == -1) || (gc == fillvalue[1] && dqfvalue[1] == -1) || (bc == fillvalue[2] && dqfvalue[2] == -1))
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
                dqfvalue[0] = *(imageptrs->ptrDQF[0] + line * xdim + pixelx);

                if(this->histogrammethod == CMB_HISTO_NONE_95)
                {
                    if(rc != fillvalue[0])
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(rc - imageptrs->minRadianceIndex[0] ) / (float)(imageptrs->maxRadianceIndex[0] - imageptrs->minRadianceIndex[0])), 0), 1023);
                }
                else if(this->histogrammethod == CMB_HISTO_NONE_100)
                {
                    if(rc != fillvalue[0])
                        indexoutrc = rc;
                }
                else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( rc != fillvalue[0]) indexoutrc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[0][rc]), 0), 1023);
                }

                if( rc == fillvalue[0] && dqfvalue[0] == -1)
                {
                    r = 0;
                    g = 0;
                    b = 0;
                }
                else if( rc == fillvalue[0] && dqfvalue[0] == 2 )
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
    emit signalcomposefinished(kindofimage);
}

void SegmentListGeostationary::CalculateLUTGeo(int colorindex)
{
    qDebug() << "start SegmentListGeostationary::CalculateLUTGeo()";
    long stats_ch[3][1024];

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 1024; j++)
        {
            stats_ch[k][j] = 0;
        }
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
            stats_ch[colorindex][indexout]++;
        }
    }


    //    for(int i = 0; i < 1024; i++)
    //    {
    //        qDebug() << QString("stats_ch[0][%1] = %2 ; stats_norm_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(stats_norm_ch[0][i]);
    //    }


    // float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    double newscale = (double)(1024.0 / this->active_pixels[colorindex]);

    qDebug() << QString("newscale = %1 active pixels = %2").arg(newscale).arg(this->active_pixels[colorindex]);

    unsigned long long sum_ch[3];

    for (int i=0; i < 3; i++)
    {
        sum_ch[i] = 0;
    }


    bool okmin, okmax;

    okmin = false;
    okmax = false;

    // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
    for( int i = 0; i < 1024; i++)
    {
        sum_ch[colorindex] += stats_ch[colorindex][i];
        imageptrs->lut_ch[colorindex][i] = (quint16)((double)sum_ch[colorindex] * newscale);
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

void SegmentListGeostationary::CalculateLUTGeo(int colorindex, quint16 *ptr, quint16 fillvalue)
{
    qDebug() << "start SegmentListGeostationary::CalculateLUTGeo() number_of_lines " << number_of_lines << " columns " << number_of_columns;
    long stats_ch[3][1024];

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 1024; j++)
        {
            stats_ch[k][j] = 0;
        }
    }

    quint16 pixel;
    for (int line = 0; line < opts.geosatellites[geoindex].imageheight; line++)
    {
        for (int pixelx = 0; pixelx < opts.geosatellites[geoindex].imagewidth; pixelx++)
        {
            pixel = ptr[line * number_of_columns + pixelx];
            if(pixel != fillvalue)
            {
                quint16 indexout = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - this->stat_min[colorindex])/(float)(this->stat_max[colorindex] - this->stat_min[colorindex])), 0), 1023);
                stats_ch[colorindex][indexout]++;
            }
        }
    }




    // float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    double newscale = (double)(1024.0 / this->active_pixels[colorindex]);

    qDebug() << QString("newscale = %1 active pixels = %2").arg(newscale).arg(this->active_pixels[colorindex]);

    unsigned long long sum_ch[3];

    for (int i=0; i < 3; i++)
    {
        sum_ch[i] = 0;
    }


    bool okmin, okmax;

    okmin = false;
    okmax = false;

    // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
    for( int i = 0; i < 1024; i++)
    {
        sum_ch[colorindex] += stats_ch[colorindex][i];
        imageptrs->lut_ch[colorindex][i] = (quint16)((double)sum_ch[colorindex] * newscale);
        imageptrs->lut_ch[colorindex][i] = ( imageptrs->lut_ch[colorindex][i] > 1023 ? 1023 : imageptrs->lut_ch[colorindex][i]);
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
    }

//    for(int i = 0; i < 1024; i++)
//    {
//        qDebug() << QString("stats_ch[0][%1] = %2").arg(i).arg(stats_ch[0][i]);
//    }


//        for(int i = 0; i < 1024; i++)
//        {
//            qDebug() << QString("stats_ch[0][%1] = %2 sum_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(sum_ch[0][i]);
//        }


    qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4").arg(colorindex).arg(imageptrs->minRadianceIndex[colorindex]).arg(colorindex).arg(imageptrs->maxRadianceIndex[colorindex]);
}

void SegmentListGeostationary::ComposeVISIR()
{

    QRgb *row_col;
    quint16 cred, cgreen, cblue;
    quint16 r,g, b;
    quint16 indexoutrc, indexoutgc, indexoutbc;

    int width = opts.geosatellites[geoindex].imagewidth;
    int height = opts.geosatellites[geoindex].imageheight;
    size_t nbrpix = width*height;

    quint16 *pixelsRed;
    quint16 *pixelsGreen;
    quint16 *pixelsBlue;

    pixelsRed = new quint16[nbrpix];
    if(kindofimage == "VIS_IR Color")
    {
        pixelsGreen = new quint16[nbrpix];
        pixelsBlue = new quint16[nbrpix];
    }

    for( int i = (bisRSS ? 5 : 0); i < opts.geosatellites[geoindex].maxsegments; i++)
    {
        if(isPresentRed[i])
            memcpy(pixelsRed + (bisRSS ? i - 5 : i) * 464 * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrRed[i], 464 * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
    }
    if(kindofimage == "VIS_IR Color")
    {
        for( int i = (bisRSS ? 5 : 0); i < opts.geosatellites[geoindex].maxsegments; i++)
        {
            if(isPresentGreen[i])
                memcpy(pixelsGreen + (bisRSS ? i - 5 : i) * 464 * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrGreen[i], 464 * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
        }
        for( int i = (bisRSS ? 5 : 0); i < opts.geosatellites[geoindex].maxsegments; i++)
        {
            if(isPresentBlue[i])
                memcpy(pixelsBlue + (bisRSS ? i - 5 : i) * 464 * opts.geosatellites[geoindex].imagewidth, imageptrs->ptrBlue[i], 464 * opts.geosatellites[geoindex].imagewidth * sizeof(quint16));
        }

    }

    CalculateMinMax(0, width, height, pixelsRed, 0);
    CalculateLUTGeo(0, pixelsRed, 0);
    if(kindofimage == "VIS_IR Color")
    {
        CalculateMinMax(1, width, height, pixelsGreen, 0);
        CalculateLUTGeo(1, pixelsGreen, 0);
        CalculateMinMax(2, width, height, pixelsBlue, 0);
        CalculateLUTGeo(2, pixelsBlue, 0);
    }

    for (int line = (bisRSS ? 3 : opts.geosatellites[geoindex].maxsegments)*464 - 1; line >= 0; line--)
    {
        if(m_GeoSatellite == eGeoSatellite::GOES_15 || m_GeoSatellite == eGeoSatellite::GOMS2)
            row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
        else
            row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine((bisRSS ? 3*464 : opts.geosatellites[geoindex].maxsegments*464) - 1 - line);

        for (int pixelx = opts.geosatellites[geoindex].imagewidth - 1 ; pixelx >= 0; pixelx--)
        {

            if(kindofimage == "VIS_IR Color")
            {
                cred = *(pixelsRed + line * opts.geosatellites[geoindex].imagewidth + pixelx);
                cgreen = *(pixelsGreen+ line * opts.geosatellites[geoindex].imagewidth + pixelx);
                cblue = *(pixelsBlue+ line * opts.geosatellites[geoindex].imagewidth + pixelx);

                if(this->histogrammethod == CMB_HISTO_NONE_95)
                {
                    if(cred != 65535)
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - imageptrs->minRadianceIndex[0] ) / (float)(imageptrs->maxRadianceIndex[0] - imageptrs->minRadianceIndex[0])), 0), 1023);
                    if(cgreen != 65535)
                        indexoutgc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cgreen - imageptrs->minRadianceIndex[1] ) / (float)(imageptrs->maxRadianceIndex[1] - imageptrs->minRadianceIndex[1])), 0), 1023);
                    if(cblue != 65535)
                        indexoutbc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cblue - imageptrs->minRadianceIndex[2] ) / (float)(imageptrs->maxRadianceIndex[2] - imageptrs->minRadianceIndex[2])), 0), 1023);
                }
                else if(this->histogrammethod == CMB_HISTO_NONE_100)
                {
                    if(cred != 65535)
                        indexoutrc = cred;
                    if(cgreen != 65535)
                        indexoutgc = cgreen;
                    if(cblue != 65535)
                        indexoutbc = cblue;
                }
                else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( cred != 65535) indexoutrc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[0][cred]), 0), 1023);
                    if( cgreen != 65535) indexoutgc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[1][cgreen]), 0), 1023);
                    if( cblue != 65535) indexoutbc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[2][cblue]), 0), 1023);
                }

                if( (cred == 65535) || (cgreen == 65535) || (cblue == 65535))
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
                cred = *(pixelsRed + line * opts.geosatellites[geoindex].imagewidth + pixelx);

                if(this->histogrammethod == CMB_HISTO_NONE_95)
                {
                    if(cred != 65535)
                        indexoutrc = (quint16)qMin(qMax(qRound(1023.0 * (float)(cred - imageptrs->minRadianceIndex[0] ) / (float)(imageptrs->maxRadianceIndex[0] - imageptrs->minRadianceIndex[0])), 0), 1023);
                }
                else if(this->histogrammethod == CMB_HISTO_NONE_100)
                {
                    if(cred != 65535)
                        indexoutrc = cred;
                }
                else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
                {
                    if( cred != 65535) indexoutrc = (quint16)qMin(qMax(qRound((float)imageptrs->lut_ch[0][cred]), 0), 1023);
                }

                if( cred == 65535)
                {
                    r = 0;
                    g = 0;
                    b = 0;
                }
                else
                {
                    r = quint16(this->inversevector[0] ? (1023 - indexoutrc)/4 : indexoutrc/4);
                    g = r;
                    b = r;
                }
            }
            if(m_GeoSatellite == eGeoSatellite::GOES_15 || m_GeoSatellite == eGeoSatellite::GOMS2 )
                row_col[pixelx] = qRgb(r,g,b);
            else
                row_col[opts.geosatellites[geoindex].imagewidth - 1 - pixelx] = qRgb(r,g,b);

        }
    }

    delete [] pixelsRed;
    if(kindofimage == "VIS_IR Color")
    {
        delete [] pixelsGreen;
        delete [] pixelsBlue;
    }

}

void SegmentListGeostationary::ComposeVISIRHimawari()
{


//    if(kindofimage == "VIS_IR")
//        CalculateMinMaxHimawari(5500, 550, imageptrs->ptrRed[filesequence], minvalueRed[filesequence], maxvalueRed[filesequence]);
//    else if(kindofimage == "VIS_IR Color")
//    {
//        if(channelindex == 0)
//            CalculateMinMaxHimawari(5500, 550, imageptrs->ptrRed[filesequence], minvalueRed[filesequence], maxvalueRed[filesequence]);
//        else if(channelindex == 1)
//            CalculateMinMaxHimawari(5500, 550, imageptrs->ptrGreen[filesequence], minvalueGreen[filesequence], maxvalueGreen[filesequence]);
//        else if(channelindex == 2)
//            CalculateMinMaxHimawari(5500, 550, imageptrs->ptrBlue[filesequence], minvalueBlue[filesequence], maxvalueBlue[filesequence]);

//    }


    QRgb *row_col;
    quint16 cred, cgreen, cblue;
    quint16 r,g, b;
    quint16 valcontrast;

    //quint16 minimum, maximum;
    //CalculateMinMax(npix, nlin, imageptrs->ptrRed[filesequence], minimum, maximum);
    //qDebug() << QString("Filesequence = %1 Minimum = %2 Maximum = %3").arg(filesequence).arg(minimum).arg(maximum);


    size_t npix = 5500*5500;

    quint16 *pixelsRed;
    quint16 *pixelsGreen;
    quint16 *pixelsBlue;

    pixelsRed = new quint16[npix];
    if(kindofimage == "VIS_IR Color")
    {
        pixelsGreen = new quint16[npix];
        pixelsBlue = new quint16[npix];
    }

    for( int i = 0; i < 10; i++)
    {
        if(isPresentRed[i])
            memcpy(pixelsRed + i * 550 * 5500, imageptrs->ptrRed[i], 550 * 5500 * sizeof(quint16));
    }


    if(kindofimage == "VIS_IR Color")
    {
        for( int i = 0; i < 10; i++)
        {
            if(isPresentGreen[i])
                memcpy(pixelsGreen + i * 550 * 5500, imageptrs->ptrGreen[i], 550 * 5500 * sizeof(quint16));
        }
        for( int i = 0; i < 10; i++)
        {
            if(isPresentBlue[i])
                memcpy(pixelsBlue + i * 550 * 5500, imageptrs->ptrBlue[i], 550 * 5500 * sizeof(quint16));
        }

    }


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

    if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_8)
    {
        if (this->areatype == 1)
            npixHRV = 5568*11136;
        else
            npixHRV = 5568*5*464;
    }
    else if(m_GeoSatellite == eGeoSatellite::MET_9)
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


    for( int i = 0, k = 0; i < (m_GeoSatellite == eGeoSatellite::MET_9 ? 5 : (this->areatype == 1 ? 24 : 5)); i++)
    {
        k = (m_GeoSatellite == eGeoSatellite::MET_9 ? 19 + i : (this->areatype == 1 ? i : 19 + i));
        qDebug() << QString("pixelsHRV i = %1 and k = %2 isPresentHRV[k] = %3").arg(i).arg(k).arg(isPresentHRV[k]);

        if(isPresentHRV[k])
            memcpy(pixelsHRV + i * 464 * 5568, imageptrs->ptrHRV[k], 464 * 5568 * sizeof(quint16));
    }

    if(kindofimage == "HRV Color")
    {
        for( int i = (m_GeoSatellite == eGeoSatellite::MET_9 ? 5 : 0); i < 8; i++)
        {
            if(isPresentRed[i])
                memcpy(pixelsRed + i * 464 * 3712, imageptrs->ptrRed[i], 464 * 3712 * sizeof(quint16));
        }
        for( int i = (m_GeoSatellite == eGeoSatellite::MET_9 ? 5 : 0); i < 8; i++)
        {
            if(isPresentGreen[i])
                memcpy(pixelsGreen + i * 464 * 3712, imageptrs->ptrGreen[i], 464 * 3712 * sizeof(quint16));
        }
        for( int i = (m_GeoSatellite == eGeoSatellite::MET_9 ? 5 : 0); i < 8; i++)
        {
            if(isPresentBlue[i])
                memcpy(pixelsBlue + i * 464 * 3712, imageptrs->ptrBlue[i], 464 * 3712 * sizeof(quint16));
        }
    }

//    if(kindofimage == "HRV Color")
//    {
//        QImage testimage(3712, 3*464, QImage::Format_ARGB32);
//        for(int y = 3*464-1; y >= 0; y--)
//        {
//            row_col = (QRgb*)testimage.scanLine(3*464-1-y);

//            for(int x = 3712-1; x >= 0; x--)
//            {
//                cred = *(pixelsRed + y*3712 + x);
//                cgreen = *(pixelsGreen + y*3712 + x);
//                cblue = *(pixelsBlue + y*3712 + x);
//                row_col[3712-1-x] = qRgb(ContrastStretch(cred), ContrastStretch(cgreen), ContrastStretch(cblue));
//            }
//        }

//        testimage.save("testimage.png");
//    }




    //imageptrs->CLAHE(pixelsRed, 3712, 3712, 0, 1023, 16, 16, 256, 15);
    //imageptrs->CLAHE(pixelsGreen, 3712, 3712, 0, 1023, 16, 16, 256, 15);
    //imageptrs->CLAHE(pixelsBlue, 3712, 3712, 0, 1023, 16, 16, 256, 15);

    if(m_GeoSatellite == eGeoSatellite::MET_9)
        imageptrs->CLAHE(pixelsHRV, 5568, 5*464, 0, 1023, 16, 16, 256, 4);
    else
    {
        if(this->areatype == 1)
            imageptrs->CLAHE(pixelsHRV, 5568, 11136, 0, 1023, 16, 16, 256, 4);
        else
            imageptrs->CLAHE(pixelsHRV, 5568, 5*464, 0, 1023, 16, 16, 256, 4);
    }

    for (int line = (m_GeoSatellite == eGeoSatellite::MET_9 ? 5 : (this->areatype == 1 ? 24 : 5))*464 - 1; line >= 0; line--)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine((m_GeoSatellite == eGeoSatellite::MET_9 ? 5 : (this->areatype == 1 ? 24 : 5))*464 - 1 - line);

        for (int pixelx = 5568 - 1 ; pixelx >= 0; pixelx--)
        {
            c = *(pixelsHRV + line * 5568 + pixelx);
            //c = 255;

            if(kindofimage == "HRV Color")
            {
                if(m_GeoSatellite == eGeoSatellite::MET_9)
                {

                    cred = *(pixelsRed + (19*464 + line)/3 * 3712 + LowerEastColumnActual/3 + pixelx/3);
                    cgreen = *(pixelsGreen + (19*464 + line)/3 * 3712 + LowerEastColumnActual/3 + pixelx/3);
                    cblue = *(pixelsBlue + (19*464 + line)/3 * 3712 + LowerEastColumnActual/3 + pixelx/3);
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



bool SegmentListGeostationary::allHRVColorSegmentsReceived()
{
    qDebug() << QString("SegmentListGeostationary::allHRVColorSegmentsReceived()");

    if (this->getKindofImage() == "HRV Color")
    {

        if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_9 || m_GeoSatellite == eGeoSatellite::MET_8)
        {
            for(int i = (m_GeoSatellite == eGeoSatellite::MET_9 ? 5 : 0); i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
                if (isPresentGreen[i] && issegmentcomposedGreen[i] == false)
                    return false;
                if (isPresentBlue[i] && issegmentcomposedBlue[i] == false)
                    return false;
            }
        }

        for(int i = (m_GeoSatellite == eGeoSatellite::MET_9 ? 19 : (this->areatype == 0 ? 19 : 0)); i < 24; i++)
        {
            qDebug() << QString("index = %1 isPresent = %2 issegmentcomposedHRV = %3").arg(i).arg(isPresentHRV[i]).arg(issegmentcomposedHRV[i]);
            if (isPresentHRV[i] && issegmentcomposedHRV[i] == false)
                return false;
        }
    }
    else if(this->getKindofImage() == "HRV")
    {
        for(int i = (m_GeoSatellite == eGeoSatellite::MET_9 ? 19 : (this->areatype == 0 ? 19 : 0)); i < 24; i++)
        {
            qDebug() << QString("index = %1 isPresent = %2 issegmentcomposedHRV = %3").arg(i).arg(isPresentHRV[i]).arg(issegmentcomposedHRV[i]);
            if (isPresentHRV[i] && issegmentcomposedHRV[i] == false)
                return false;
        }
    }

    return true;
}

bool SegmentListGeostationary::allSegmentsReceived()
{

    qDebug() << QString("SegmentListGeostationary::allSegmentsReceived()");

    int pbCounter = 0;
    int pbCounterRed = 0;
    int pbCounterGreen = 0;
    int pbCounterBlue = 0;


    if (this->getKindofImage() == "VIS_IR Color")
    {
        if(m_GeoSatellite == eGeoSatellite::FY2E || m_GeoSatellite == eGeoSatellite::FY2G || m_GeoSatellite == eGeoSatellite::GOES_16)
        {
            return true;
        }
        else if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_9 || m_GeoSatellite == eGeoSatellite::MET_8)
        {
            for(int i = (m_GeoSatellite == eGeoSatellite::MET_9 ? 5 : 0) ; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                {
                    pbCounter++;
                    pbCounterRed++;
                }
                if (isPresentGreen[i] && issegmentcomposedGreen[i] == true)
                {
                    pbCounter++;
                    pbCounterGreen++;
                }
                if (isPresentBlue[i] && issegmentcomposedBlue[i] == true)
                {
                    pbCounter++;
                    pbCounterBlue++;
                }
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::GOMS2)
        {
            for(int i = 0 ; i < 6; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                    pbCounter++;
                if (isPresentGreen[i] && issegmentcomposedGreen[i] == true)
                    pbCounter++;
                if (isPresentBlue[i] && issegmentcomposedBlue[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::H8)
        {
            for(int i = 0; i < 10; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                    pbCounter++;
                if (isPresentGreen[i] && issegmentcomposedGreen[i] == true)
                    pbCounter++;
                if (isPresentBlue[i] && issegmentcomposedBlue[i] == true)
                    pbCounter++;
            }
        }
    }
    else if (this->getKindofImage() == "VIS_IR" )
    {
        if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_8)
        {
            for(int i = 0; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::GOMS2)
        {
            for(int i = 0; i < 6; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::MET_9)
        {
            for(int i = 5; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::GOES_15)
        {
            for(int i = 0; i < 7; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::FY2E || m_GeoSatellite == eGeoSatellite::FY2G)
        {
                if (isPresentRed[0] && issegmentcomposedRed[0] == true)
                    pbCounter++;
        }
        else if(m_GeoSatellite == eGeoSatellite::H8)
        {
            for(int i = 0; i < 10; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                    pbCounter++;
            }
        }


    }
    else if (this->getKindofImage() == "HRV" || this->getKindofImage() == "HRV Color")
    {
        if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_9 || m_GeoSatellite == eGeoSatellite::MET_8)
        {
            for(int i = (m_GeoSatellite == eGeoSatellite::MET_9 ? 5 : 0) ; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                    pbCounter++;
                if (isPresentGreen[i] && issegmentcomposedGreen[i] == true)
                    pbCounter++;
                if (isPresentBlue[i] && issegmentcomposedBlue[i] == true)
                    pbCounter++;
            }
            for(int i = (this->areatype == 0 ? 0 : 19); i < 24; i++)
            {
                if (isPresentHRV[i] && issegmentcomposedHRV[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::FY2E || m_GeoSatellite == eGeoSatellite::FY2G)
        {
            if (isPresentHRV[0] && issegmentcomposedHRV[0] == true)
                pbCounter++;
        }
    }

    qDebug() << QString("SegmentListGeostationary::allSegmentsReceived() pbCounter = %1").arg(pbCounter);

    if(pbCounterRed == 8)
        qDebug() << QString("pbCounterRed =  8");
    if(pbCounterGreen == 8)
        qDebug() << QString("pbCounterGreen =  8");
    if(pbCounterBlue == 8)
        qDebug() << QString("pbCounterBlue =  8");


    emit progressCounter(pbCounter);

    if (this->getKindofImage() == "VIS_IR Color")
    {
        if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_9 || m_GeoSatellite == eGeoSatellite::MET_8)
        {
            for(int i = (m_GeoSatellite == eGeoSatellite::MET_9 ? 5 : 0) ; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
                if (isPresentGreen[i] && issegmentcomposedGreen[i] == false)
                    return false;
                if (isPresentBlue[i] && issegmentcomposedBlue[i] == false)
                    return false;
            }
        } else if(m_GeoSatellite == eGeoSatellite::GOMS2)
        {
            for(int i = 0 ; i < 6; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
                if (isPresentGreen[i] && issegmentcomposedGreen[i] == false)
                    return false;
                if (isPresentBlue[i] && issegmentcomposedBlue[i] == false)
                    return false;
            }
        } else if(m_GeoSatellite == eGeoSatellite::H8)
        {
            for(int i = 0 ; i < 10; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
                if (isPresentGreen[i] && issegmentcomposedGreen[i] == false)
                    return false;
                if (isPresentBlue[i] && issegmentcomposedBlue[i] == false)
                    return false;
            }
        }

    }
    else if (this->getKindofImage() == "VIS_IR" )
    {
        if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_8)
        {
            for(int i = 0; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::MET_9)
        {
            for(int i = 5; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::GOMS2)
        {
            for(int i = 0; i < 6; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::GOES_15)
        {
            for(int i = 0; i < 7; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::FY2E || m_GeoSatellite == eGeoSatellite::FY2G || m_GeoSatellite == eGeoSatellite::GOES_16)
        {
            if (isPresentRed[0] && issegmentcomposedRed[0] == false)
               return false;

        }
        else if(m_GeoSatellite == eGeoSatellite::H8)
        {
//            qDebug() << "=======================================";
//            for(int i = 0; i < 10; i++)
//            {
//                qDebug() << QString("%1 isPresentRed = %2 issegmentcomposedRed = %3").arg(i).arg(isPresentRed[i]).arg(issegmentcomposedRed[i]);

//            }
//            qDebug() << "=======================================";
            for(int i = 0; i < 10; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
            }
        }
    }
    else if (this->getKindofImage() == "HRV" || this->getKindofImage() == "HRV Color")
    {
        if(m_GeoSatellite == eGeoSatellite::MET_11 || m_GeoSatellite == eGeoSatellite::MET_10 || m_GeoSatellite == eGeoSatellite::MET_9 || m_GeoSatellite == eGeoSatellite::MET_8)
        {
            for(int i = 19; i < 24; i++)
            {
                if (isPresentHRV[i] && issegmentcomposedHRV[i] == false)
                    return false;
            }
        }
        else if(m_GeoSatellite == eGeoSatellite::FY2E || m_GeoSatellite == eGeoSatellite::FY2G)
        {
            if (isPresentHRV[0] && issegmentcomposedHRV[0] == false)
                return false;
        }
    }

    qDebug() << "SegmentListGeostationary::allSegmentsReceived() returns true";

    emit progressCounter(100);
    return true;
}


void SegmentListGeostationary::CalculateMinMax(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max)
{
    stat_min = 65535;
    stat_max = 0;


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
        }
    }

}

void SegmentListGeostationary::CalculateMinMax(int colorindex, int width, int height, quint16 *ptr, quint16 fillvalue)
{
    long cnt = 0;
    this->stat_min[colorindex] = 65535;
    this->stat_max[colorindex] = 0;

    this->active_pixels[colorindex] = 0;

    qDebug() << QString("start CalculateMinMax");

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            quint16 val = ptr[j * width + i];
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

void SegmentListGeostationary::normalizeMinMaxGOES16(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max, int &fillvalue, int maxvalue)
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
//    fillvalue = 4096;

}

void SegmentListGeostationary::CalculateMinMaxHimawari(int width, int height, quint16 *ptr, quint16 &stat_min, quint16 &stat_max)
{
    stat_min = 65535;
    stat_max = 0;


    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++)
        {
            quint16 val = ptr[j * height + i];
            if(val >= stat_max)
                stat_max = val;
            if(val < stat_min)
                stat_min = val;
        }
    }

}

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
    return (res > 255.0 ? 255 : quint16(res));
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



