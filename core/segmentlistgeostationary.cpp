#include <QApplication>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include "segmentlistgeostationary.h"
#include "segmentimage.h"
#include "qcompressor.h"
#include "hdf5.h"

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

extern QMutex g_mutex;
extern Options opts;
extern SegmentImage *imageptrs;
// Meteosat
// Height = 3712 / 8 = 464 Width = 3712
// Height = 11136 / 24 = 464 Width = 7502
// Electro_n1
// Height = 2784 / 6 = 464 Width = 2784

void doComposeGeostationaryXRIT(SegmentListGeostationary *sm, QString segment_path, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    sm->ComposeSegmentImageXRIT(segment_path, channelindex, spectrumvector, inversevector);
}

void doComposeGeostationaryXRITHimawari(SegmentListGeostationary *sm, QString segment_path, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    sm->ComposeSegmentImageXRITHimawari(segment_path, channelindex, spectrumvector, inversevector);
}

void doComposeGeostationaryHDF(SegmentListGeostationary *sm, QString segment_path, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    sm->ComposeSegmentImageHDF(segment_path, channelindex, spectrumvector, inversevector);
}

void doComposeGeostationaryHDFInThread(SegmentListGeostationary *sm, QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    sm->ComposeSegmentImageHDFInThread(filelist, spectrumvector, inversevector);
}


SegmentListGeostationary::SegmentListGeostationary(QObject *parent) :
    QObject(parent)
{

    for (int i = 0; i < 10; i++)
    {
        maxvalueRed[i] = 0;
        minvalueRed[i] = 0;
        maxvalueGreen[i] = 0;
        minvalueGreen[i] = 0;
        maxvalueBlue[i] = 0;
        minvalueBlue[i] = 0;
    }

    kindofimage = "";
    this->SetupContrastStretch( 0, 0, 1023, 255);
    COFF = 0;
    LOFF = 0;
    CFAC = 0;
    LFAC = 0;

    areatype = 0;
    ResetSegments();
    this->bActiveSegmentList = false;
    this->bisRSS = false;
    this->m_GeoSatellite = MET_10;
    qDebug() << QString("in constructor SegmentListGeostationary");

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
    for( int i = 0; i < 10; i++)
    {
        issegmentcomposedMono[i] = false;
        isPresentMono[i] = false;
    }

}

bool SegmentListGeostationary::ComposeImageXRIT(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    /* qDebug() << QString("ideal threadcount = %1  max threadcount = %2 active threadcount = %3").
                arg(QThread::idealThreadCount()).
                arg(QThreadPool::globalInstance()->maxThreadCount()).
                arg(QThreadPool::globalInstance()->activeThreadCount());

    */

    //QFile file(filepath);
    //QFileInfo fileinfo(file);
    int filesequence = fileinfo.fileName().mid(36, 6).toInt()-1;
    QString filespectrum = fileinfo.fileName().mid(26, 6);
    QString filedate = fileinfo.fileName().mid(46, 12);

    qDebug() << QString("SegmentListGeostationary::ComposeImage filePath = %1").arg(fileinfo.filePath());

    if( filespectrum  == "HRV___")
    {
        QFuture<void> future = QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
        watcherHRV[filesequence].setFuture(future);
    }
    else if(m_GeoSatellite == MET_7 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT)  // m_GeoSatellite == ELECTRO_N1 ||
    {
        QFuture<void> future = QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
        watcherMono[filesequence].setFuture(future);
    }
    else if(m_GeoSatellite == MET_10 || m_GeoSatellite == MET_9)
    {
        if( spectrumvector.at(1) == "" && spectrumvector.at(2) == "")
        {
            QFuture<void> future = QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
            watcherRed[filesequence].setFuture(future);
        }
        else
        {
            if(spectrumvector.at(0) == filespectrum)
            {
                QFuture<void> future = QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
                watcherRed[filesequence].setFuture(future);
            }
            else if(spectrumvector.at(1) == filespectrum)
            {
                QFuture<void> future = QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 1, spectrumvector, inversevector);
                watcherGreen[filesequence].setFuture(future);
            }
            else if(spectrumvector.at(2) == filespectrum)
            {
                QFuture<void> future = QtConcurrent::run(doComposeGeostationaryXRIT, this, fileinfo.filePath(), 2, spectrumvector, inversevector);
                watcherBlue[filesequence].setFuture(future);
            }
        }
    }
    else if(m_GeoSatellite == H8)
    {
        filesequence = fileinfo.fileName().mid(25, 3).toInt()-1;
        filespectrum = fileinfo.fileName().mid(8, 3);
        filedate = fileinfo.fileName().mid(12, 11) + "0";

        if( spectrumvector.at(1) == "" && spectrumvector.at(2) == "")
        {
            QFuture<void> future = QtConcurrent::run(doComposeGeostationaryXRITHimawari, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
            watcherRed[filesequence].setFuture(future);
        }
        else
        {
            if(spectrumvector.at(0) == filespectrum)
            {
                QFuture<void> future = QtConcurrent::run(doComposeGeostationaryXRITHimawari, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
                watcherRed[filesequence].setFuture(future);
            }
            else if(spectrumvector.at(1) == filespectrum)
            {
                QFuture<void> future = QtConcurrent::run(doComposeGeostationaryXRITHimawari, this, fileinfo.filePath(), 1, spectrumvector, inversevector);
                watcherGreen[filesequence].setFuture(future);
            }
            else if(spectrumvector.at(2) == filespectrum)
            {
                QFuture<void> future = QtConcurrent::run(doComposeGeostationaryXRITHimawari, this, fileinfo.filePath(), 2, spectrumvector, inversevector);
                watcherBlue[filesequence].setFuture(future);
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
    QFuture<void> future = QtConcurrent::run(doComposeGeostationaryHDFInThread, this, strlist, spectrumvector, inversevector);
    watcherRed[0].setFuture(future);

    return true;
}

bool SegmentListGeostationary::ComposeImageHDFSerial(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector)
{

    //"Z_SATE_C_BABJ_20150809101500_O_FY2E_FDI_IR1_001_NOM.HDF.gz"
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

void SegmentListGeostationary::InsertPresent( QVector<QString> spectrumvector, QString filespectrum, int filesequence)
{
    qDebug() << QString("InsertPresent ; spectrum %1 %2 %3    filespectrum  %4  fileseq %5").arg(spectrumvector[0]).arg(spectrumvector[1]).arg(spectrumvector[2]).arg(filespectrum).arg(filesequence);
    if(m_GeoSatellite == MET_10 || m_GeoSatellite == MET_9 || m_GeoSatellite == H8)
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
        else if(filespectrum == "HRV___")
        {
            isPresentHRV[filesequence] = true;
        }
    }
    else if(m_GeoSatellite == MET_7 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT)
    {
        isPresentMono[filesequence] = true;
    }
    else if(m_GeoSatellite == FY2E || m_GeoSatellite == FY2G)
    {
        if(filespectrum == "VIS1KM")
            isPresentHRV[filesequence] = true;
        else
            isPresentMono[filesequence] = true;
    }

}

//void SegmentListGeostationary::ComposeSegmentImageXRIT( QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector )
//{

//    QRgb *row_col;

//    MSG_header *header;
//    MSG_data *msgdat;

//    qDebug() << QString("-------> SegmentListGeostationary::ComposeSegmentImage() %1").arg(filepath);

//    header = new MSG_header();
//    msgdat = new MSG_data();

//    QFile file(filepath);
//    QFileInfo fileinfo(file);
//    int filesequence = fileinfo.fileName().mid(36, 6).toInt()-1;
//    QString filespectrum = fileinfo.fileName().mid(26, 6);
//    QString filedate = fileinfo.fileName().mid(46, 12);

//    QByteArray ba = filepath.toLatin1();
//    const char *c_segname = ba.data();

//    std::ifstream hrit(c_segname, (std::ios::binary | std::ios::in) );
//    if (hrit.fail())
//    {
//        std::cerr << "Cannot open input hrit file "
//            << filepath.toStdString() << std::endl;
//        return;
//    }

//    header->read_from(hrit);
//    msgdat->read_from(hrit, *header);
//    hrit.close();

//    if (header->segment_id->data_field_format == MSG_NO_FORMAT)
//    {
//      qDebug() << "Product dumped in binary format.";
//      return;
//    }

//    int planned_end_segment = header->segment_id->planned_end_segment_sequence_number;

//    int npix = number_of_columns = header->image_structure->number_of_columns;
//    int nlin = number_of_lines = header->image_structure->number_of_lines;
//    size_t npixperseg = number_of_columns*number_of_lines;

//    qDebug() << QString("---->[%1] SegmentListGeostationary::ComposeSegmentImage() planned end = %2 npix = %3 nlin = %4 fileseqeunce = %5").arg(kindofimage).arg(planned_end_segment).arg(number_of_columns).arg(number_of_lines).arg(filesequence);

//    MSG_SAMPLE *pixels = new MSG_SAMPLE[npixperseg];
//    memset(pixels, 0, npixperseg*sizeof(MSG_SAMPLE));
//    memcpy(pixels, msgdat->image->data, npixperseg*sizeof(MSG_SAMPLE));

//    QImage *im;
//    im = imageptrs->ptrimageGeostationary;

//    quint16 c;
//    QRgb pix;
//    int r,g, b;

//    double gamma = opts.meteosatgamma;
//    double gammafactor;
//    quint16 valgamma;

//    quint8 valcontrast;

//    if(m_GeoSatellite == MET_7 || m_GeoSatellite == MTSAT)
//        gammafactor = 255 / pow(255, gamma);
//    else
//        gammafactor = 1023 / pow(1023, gamma);


//    if (filespectrum == "HRV___")
//    {
//        imageptrs->ptrHRV[filesequence] = new quint16[number_of_lines * number_of_columns];
//        memset(imageptrs->ptrHRV[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
//    }
//    else if(m_GeoSatellite == MET_7 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT) // m_GeoSatellite == ELECTRO_N1 ||
//    {
//        imageptrs->ptrRed[filesequence] = new quint16[number_of_lines * number_of_columns];
//        memset(imageptrs->ptrRed[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));

//    }
//    else
//    {
//        if(channelindex == 0)
//        {
//            imageptrs->ptrRed[filesequence] = new quint16[number_of_lines * number_of_columns];
//            memset(imageptrs->ptrRed[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
//        }
//        else if(channelindex == 1)
//        {
//            imageptrs->ptrGreen[filesequence] = new quint16[number_of_lines * number_of_columns];
//            memset(imageptrs->ptrGreen[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
//        }
//        else if(channelindex == 2)
//        {
//            imageptrs->ptrBlue[filesequence] = new quint16[number_of_lines * number_of_columns];
//            memset(imageptrs->ptrBlue[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
//        }
//    }


//    g_mutex.lock();

//    for(int line = 0; line < nlin; line++)
//    {
//        //qDebug() << QString("filesequence = %1 ; nlin * totalsegs - 1 - startLine[filesequence] - line = %2").arg(filesequence).arg(nlin * totalsegs - 1 - startLine[filesequence] - line);

//        if( m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT) // m_GeoSatellite == ELECTRO_N1 ||
//            row_col = (QRgb*)im->scanLine(nlin * filesequence + line);
//        else
//            row_col = (QRgb*)im->scanLine( nlin * planned_end_segment - 1 - nlin * filesequence - line);

//        for (int pixelx = 0 ; pixelx < npix; pixelx++)
////        for (int pixelx = npix - 1 ; pixelx >= 0; pixelx--)
//        {
//            c = *(pixels + line * npix + pixelx);

//            if (filespectrum == "HRV___")
//            {
//                *(imageptrs->ptrHRV[filesequence] + line * npix + pixelx) = c;
//            }
//            else
//            {
//                if(channelindex == 0)
//                    *(imageptrs->ptrRed[filesequence] + line * npix + pixelx) = c;
//                else if(channelindex == 1)
//                    *(imageptrs->ptrGreen[filesequence] + line * npix + pixelx) = c;
//                else if(channelindex == 2)
//                    *(imageptrs->ptrBlue[filesequence] + line * npix + pixelx) = c;
//            }

//            //valgamma = pow( c, gamma) * gammafactor;
//            valcontrast = ContrastStretch(c);

//            // qDebug() << QString("npix - 1 - pixelx = %1").arg(npix - 1 - pixelx);



//            if(kindofimage == "VIS_IR Color")
//            {
//                pix = row_col[npix - 1 - pixelx];

//                if(channelindex == 2)
//                {
//                    if(inversevector[2])
//                        valcontrast = 255 - valcontrast;
//                    r = qRed(pix);
//                    g = qGreen(pix);
//                    b = quint8(valcontrast);
//                }
//                else if(channelindex == 1)
//                {
//                    if(inversevector[1])
//                        valcontrast = 255 - valcontrast;
//                    r = qRed(pix);
//                    g = quint8(valcontrast);
//                    b = qBlue(pix);
//                }
//                else if(channelindex == 0)
//                {
//                    if(inversevector[0])
//                        valcontrast = 255 - valcontrast;
//                    r = quint8(valcontrast);
//                    g = qGreen(pix);
//                    b = qBlue(pix);
//                }
//                row_col[npix - 1 - pixelx] = qRgb(r,g,b);

//            }
//            else if( kindofimage == "VIS_IR" || kindofimage == "HRV")
//            {
//                if(inversevector[0])
//                    valcontrast = 255 - valcontrast;

//                r = quint8(valcontrast);
//                g = quint8(valcontrast);
//                b = quint8(valcontrast);
//                if(m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT) // m_GeoSatellite == ELECTRO_N1 ||
//                    row_col[pixelx] = qRgb(r,g,b);
//                else
//                    row_col[npix - 1 - pixelx] = qRgb(r,g,b);

//            }
//        }
//    }



//    if (filespectrum == "HRV___")
//    {
//        this->issegmentcomposedHRV[filesequence] = true;
//    }
//    else if(m_GeoSatellite == MET_7 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT) // m_GeoSatellite == ELECTRO_N1 ||
//    {
//        this->issegmentcomposedMono[filesequence] = true;
//    }
//    else
//    {
//        if(channelindex == 0)
//            this->issegmentcomposedRed[filesequence] = true;
//        else if(channelindex == 1)
//            this->issegmentcomposedGreen[filesequence] = true;
//        else if(channelindex == 2)
//            this->issegmentcomposedBlue[filesequence] = true;
//    }

//    if(kindofimage == "HRV Color" && allHRVColorSegmentsReceived())
//    {
//        qDebug() << "-----> HRV Color and allHRVColorSegmentsReceived";
//        this->ComposeColorHRV();
//    }

//    g_mutex.unlock();


//    delete header;
//    delete msgdat;
//    delete [ ] pixels;

//}

void SegmentListGeostationary::ComposeSegmentImageXRIT( QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector )
{

    QRgb *row_col;

    MSG_header *header;
    MSG_data *msgdat;

    qDebug() << QString("-------> SegmentListGeostationary::ComposeSegmentImage() %1").arg(filepath);

    header = new MSG_header();
    msgdat = new MSG_data();

    QFile file(filepath);
    QFileInfo fileinfo(file);
    int filesequence = fileinfo.fileName().mid(36, 6).toInt()-1;
    QString filespectrum = fileinfo.fileName().mid(26, 6);
    QString filedate = fileinfo.fileName().mid(46, 12);

    QByteArray ba = filepath.toLatin1();
    const char *c_segname = ba.data();

    std::ifstream hrit(c_segname, (std::ios::binary | std::ios::in) );
    if (hrit.fail())
    {
        std::cerr << "Cannot open input hrit file "
            << filepath.toStdString() << std::endl;
        return;
    }

    header->read_from(hrit);
    msgdat->read_from(hrit, *header);
    hrit.close();

    if (header->segment_id->data_field_format == MSG_NO_FORMAT)
    {
      qDebug() << "Product dumped in binary format.";
      return;
    }

    int planned_end_segment = header->segment_id->planned_end_segment_sequence_number;

    int npix = number_of_columns = header->image_structure->number_of_columns;
    int nlin = number_of_lines = header->image_structure->number_of_lines;
    size_t npixperseg = number_of_columns*number_of_lines;

    qDebug() << QString("---->[%1] SegmentListGeostationary::ComposeSegmentImageXRIT() planned end = %2 npix = %3 nlin = %4 fileseqeunce = %5").arg(kindofimage).arg(planned_end_segment).arg(number_of_columns).arg(number_of_lines).arg(filesequence);

    MSG_SAMPLE *pixels = new MSG_SAMPLE[npixperseg];
    memset(pixels, 0, npixperseg*sizeof(MSG_SAMPLE));
    memcpy(pixels, msgdat->image->data, npixperseg*sizeof(MSG_SAMPLE));

    QImage *im;
    im = imageptrs->ptrimageGeostationary;

    quint16 c;
    QRgb pix;
    int r,g, b;

    double gamma = opts.meteosatgamma;
    double gammafactor;
    quint16 valgamma;

    quint8 valcontrast;

    if(m_GeoSatellite == MET_7 || m_GeoSatellite == MTSAT)
        gammafactor = 255 / pow(255, gamma);
    else
        gammafactor = 1023 / pow(1023, gamma);


    if (filespectrum == "HRV___")
    {
        imageptrs->ptrHRV[filesequence] = new quint16[number_of_lines * number_of_columns];
        memset(imageptrs->ptrHRV[filesequence], 0, number_of_lines * number_of_columns *sizeof(quint16));
    }
    else if(m_GeoSatellite == MET_7 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT) // m_GeoSatellite == ELECTRO_N1 ||
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


    g_mutex.lock();

    for(int line = 0; line < nlin; line++)
    {
        //qDebug() << QString("filesequence = %1 ; nlin * totalsegs - 1 - startLine[filesequence] - line = %2").arg(filesequence).arg(nlin * totalsegs - 1 - startLine[filesequence] - line);

        if( m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT) // m_GeoSatellite == ELECTRO_N1 ||
            row_col = (QRgb*)im->scanLine(nlin * filesequence + line);
        else
            row_col = (QRgb*)im->scanLine( nlin * planned_end_segment - 1 - nlin * filesequence - line);

        for (int pixelx = 0 ; pixelx < npix; pixelx++)
//        for (int pixelx = npix - 1 ; pixelx >= 0; pixelx--)
        {
            c = *(pixels + line * npix + pixelx);

            if (filespectrum == "HRV___")
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


//    quint16 stat_min;
//    quint16 stat_max;

//    if(m_GeoSatellite == MET_7 || m_GeoSatellite == MTSAT)
//    {
//        if(kindofimage == "VIS_IR")
//            CalculateMinMax(2288, 2288, imageptrs->ptrRed[0], stat_min, stat_max);
//        else if(kindofimage == "VIS_IR Color")
//        {
//            if(channelindex == 0)
//                CalculateMinMax(2288, 2288, imageptrs->ptrRed[0], stat_min, stat_max);
//            else if(channelindex == 1)
//                CalculateMinMax(2288, 2288, imageptrs->ptrGreen[0], stat_min, stat_max);
//            else if(channelindex == 2)
//                CalculateMinMax(2288, 2288, imageptrs->ptrBlue[0], stat_min, stat_max);

//        } else if(kindofimage == "HRV")
//        {
//            CalculateMinMax(9152, 9152, imageptrs->ptrRed[0], stat_min, stat_max);
//        }

//        qDebug() << QString("stat min = %1 stat max = %2").arg(stat_min).arg(stat_max);
//        this->SetupContrastStretch( stat_min, 0, stat_max+1, 255, stat_max+1, 255, stat_max+1, 255);
//    }
//    else
//        this->SetupContrastStretch( 0, 0, 250, 60, 750, 200, 1023, 255);




    for(int line = 0; line < nlin; line++)
    {
        //qDebug() << QString("filesequence = %1 ; nlin * totalsegs - 1 - startLine[filesequence] - line = %2").arg(filesequence).arg(nlin * totalsegs - 1 - startLine[filesequence] - line);

        if( m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT )  // m_GeoSatellite == ELECTRO_N1 ||
            row_col = (QRgb*)im->scanLine(nlin * filesequence + line);
        else
            row_col = (QRgb*)im->scanLine( nlin * planned_end_segment - 1 - nlin * filesequence - line);

        for (int pixelx = 0 ; pixelx < npix; pixelx++)
        {
            c = *(pixels + line * npix + pixelx);

//            //valgamma = pow( c, gamma) * gammafactor;
//            valcontrast = ContrastStretch(c);

            //valcontrast = pow( c, gamma) * gammafactor;
            valcontrast = ContrastStretch(c);

            if(kindofimage == "VIS_IR Color")
            {
                pix = row_col[npix - 1 - pixelx];

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
                row_col[npix - 1 - pixelx] = qRgb(r,g,b);

            }
            else if( kindofimage == "VIS_IR" || kindofimage == "HRV")
            {
                if(inversevector[0])
                    valcontrast = 255 - valcontrast;

                r = quint8(valcontrast);
                g = quint8(valcontrast);
                b = quint8(valcontrast);
                if(m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT) // m_GeoSatellite == ELECTRO_N1 ||
                    row_col[pixelx] = qRgb(r,g,b);
                else
                    row_col[npix - 1 - pixelx] = qRgb(r,g,b);

            }
        }
    }



    if (filespectrum == "HRV___")
    {
        this->issegmentcomposedHRV[filesequence] = true;
    }
    else if(m_GeoSatellite == MET_7 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT) // m_GeoSatellite == ELECTRO_N1 ||
    {
        this->issegmentcomposedMono[filesequence] = true;
    }
    else
    {
        if(channelindex == 0)
            this->issegmentcomposedRed[filesequence] = true;
        else if(channelindex == 1)
            this->issegmentcomposedGreen[filesequence] = true;
        else if(channelindex == 2)
            this->issegmentcomposedBlue[filesequence] = true;
    }

    if(kindofimage == "HRV Color" && allHRVColorSegmentsReceived())
    {
        qDebug() << "-----> HRV Color and allHRVColorSegmentsReceived";
        this->ComposeColorHRV();
    }

    g_mutex.unlock();


    delete header;
    delete msgdat;
    delete [ ] pixels;

}

void SegmentListGeostationary::ComposeSegmentImageXRITHimawari( QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector )
{
//IMG_DK01B04_201510090000_001.bz2
//012345678901234567890123456789
    QRgb *row_col;

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

    QByteArray ba = basename.toLatin1();
    const char *c_segname = ba.data();

    std::ifstream hrit(c_segname, (std::ios::binary | std::ios::in) );
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
    QRgb pix;
    int r,g, b;
    quint16 valcontrast;

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


    g_mutex.lock();

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


    quint16 stat_min;
    quint16 stat_max;

    if(kindofimage == "VIS_IR")
        CalculateMinMaxHimawari(5500, 550, imageptrs->ptrRed[filesequence], minvalueRed[filesequence], maxvalueRed[filesequence]);
    else if(kindofimage == "VIS_IR Color")
    {
        if(channelindex == 0)
            CalculateMinMaxHimawari(5500, 550, imageptrs->ptrRed[filesequence], minvalueRed[filesequence], maxvalueRed[filesequence]);
        else if(channelindex == 1)
            CalculateMinMaxHimawari(5500, 550, imageptrs->ptrGreen[filesequence], minvalueGreen[filesequence], maxvalueGreen[filesequence]);
        else if(channelindex == 2)
            CalculateMinMaxHimawari(5500, 550, imageptrs->ptrBlue[filesequence], minvalueBlue[filesequence], maxvalueBlue[filesequence]);

    }


    this->SetupContrastStretch( 0, 0, 1023, 255);


    for(int line = 0; line < nlin; line++)
    {

        row_col = (QRgb*)im->scanLine(nlin * filesequence + line);

        for (int pixelx = 0 ; pixelx < npix; pixelx++)
        {
            c = *(pixels + line * npix + pixelx);
            valcontrast = ContrastStretch(c);
            valcontrast = (valcontrast > 255 ? 255 : valcontrast);

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
            else if( kindofimage == "VIS_IR")
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



        if(channelindex == 0)
            this->issegmentcomposedRed[filesequence] = true;
        else if(channelindex == 1)
            this->issegmentcomposedGreen[filesequence] = true;
        else if(channelindex == 2)
            this->issegmentcomposedBlue[filesequence] = true;



    g_mutex.unlock();


    delete header;
    delete msgdat;
    delete [ ] pixels;

}

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

void SegmentListGeostationary::ComposeSegmentImageHDFInThread(QStringList filelist, QVector<QString> spectrumvector, QVector<bool> inversevector )
{

    quint8 valcontrastred, valcontrastgreen, valcontrastblue;
    QRgb *row_col;
    quint16 rc, gc, bc;

    int r,g, b;

    QImage *im;
    QStringList outfilename;

    im = imageptrs->ptrimageGeostationary;

    for(int j = 0; j < filelist.size(); j++)
    {
        QFile inFile(this->getImagePath() + "/" + filelist.at(j));
        inFile.open(QIODevice::ReadOnly);
        QByteArray compressed = inFile.readAll();

        QByteArray decompressed;
        outfilename.append(filelist.at(j).mid(0, filelist.at(j).length()-3));

        QFile outfile(filelist.at(j).mid(0, filelist.at(j).length()-3));
        if(QCompressor::gzipDecompress(compressed, decompressed))
        {
            outfile.open(QIODevice::WriteOnly);
            QDataStream out(&outfile);
            out.writeRawData(decompressed.constData(), decompressed.length());
            qDebug() << "writing decompressed file " << outfilename.at(j);
        }
        else
            qDebug() << "-----> gzipDecompress failed !";

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

        if( (h5_file_id[j] = H5Fopen(outfilename.at(j).toLatin1(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
            qDebug() << "File " << outfilename.at(j) << " not open !!";


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


    //g_mutex.lock();

    int nlin, npix;
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

    this->issegmentcomposedMono[0] = true;

    emit this->progressCounter(100);


    //g_mutex.unlock();

    for(int j = 0; j < filelist.size(); j++)
    {

        H5Dclose(nomfileinfo_id[j]);
        H5Dclose(nomchannel_id[j]);
        H5Fclose(h5_file_id[j]);
    }

    emit imagefinished();

}

void SegmentListGeostationary::ComposeColorHRV()
{
    QRgb *row_col;
    quint16 cred, cgreen, cblue, c, clum;
    quint16 r,g, b;

    double gamma = opts.meteosatgamma;
    double gammafactor = 1023 / pow(1023, gamma);
    quint16 valgamma;
    quint8 valcontrast;


    size_t npix = 3712*3712;
    size_t npixHRV;

    if(m_GeoSatellite == MET_10)
    {
        if (this->areatype == 1)
            npixHRV = 5568*11136;
        else
            npixHRV = 5568*5*464;
    }
    else if(m_GeoSatellite == MET_9)
    {
        npixHRV = 5568*5*464;
    }
/*    else if(m_GeoSatellite == ELECTRO_N1)
    {
        npixHRV = 5568*5*464;
    }
*/

    quint16 *pixelsRed;
    quint16 *pixelsGreen;
    quint16 *pixelsBlue;
    quint16 *pixelsHRV;


    pixelsRed = new quint16[npix];
    pixelsGreen = new quint16[npix];
    pixelsBlue = new quint16[npix];
    pixelsHRV = new quint16[npixHRV];


    for( int i = (m_GeoSatellite == MET_9 ? 5 : 0); i < 8; i++)
    {
        if(isPresentRed[i])
            memcpy(pixelsRed + i * 464 * 3712, imageptrs->ptrRed[i], 464 * 3712 * sizeof(quint16));
    }
    for( int i = (m_GeoSatellite == MET_9 ? 5 : 0); i < 8; i++)
    {
        if(isPresentGreen[i])
            memcpy(pixelsGreen + i * 464 * 3712, imageptrs->ptrGreen[i], 464 * 3712 * sizeof(quint16));
    }
    for( int i = (m_GeoSatellite == MET_9 ? 5 : 0); i < 8; i++)
    {
        if(isPresentBlue[i])
            memcpy(pixelsBlue + i * 464 * 3712, imageptrs->ptrBlue[i], 464 * 3712 * sizeof(quint16));
    }

    for( int i = 0, k = 0; i < (m_GeoSatellite == MET_9 ? 5 : ( this->areatype == 1 ? 24 : 5)); i++)
    {
        k = (m_GeoSatellite == MET_9 ? 19 + i : (this->areatype == 1 ? i : 19 + i));
        if(isPresentHRV[k])
            memcpy(pixelsHRV + i * 464 * 5568, imageptrs->ptrHRV[k], 464 * 5568 * sizeof(quint16));
    }

    //imageptrs->CLAHE(pixelsRed, 3712, 3712, 0, 1023, 16, 16, 256, 15);
    //imageptrs->CLAHE(pixelsGreen, 3712, 3712, 0, 1023, 16, 16, 256, 15);
    //imageptrs->CLAHE(pixelsBlue, 3712, 3712, 0, 1023, 16, 16, 256, 15);

    if(m_GeoSatellite == MET_9)
        imageptrs->CLAHE(pixelsHRV, 5568, 5*464, 0, 1023, 16, 16, 256, 4);
    else
    {
        if(this->areatype == 1)
            imageptrs->CLAHE(pixelsHRV, 5568, 11136, 0, 1023, 16, 16, 256, 4);
        else
            imageptrs->CLAHE(pixelsHRV, 5568, 5*464, 0, 1023, 16, 16, 256, 4);
    }

    for (int line = (m_GeoSatellite == MET_9 ? 5 : (this->areatype == 1 ? 24 : 5))*464 - 1; line >= 0; line--)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine((m_GeoSatellite == MET_9 ? 5 : (this->areatype == 1 ? 24 : 5))*464 - 1 - line);

        for (int pixelx = 5568 - 1 ; pixelx >= 0; pixelx--)
        {
            c = *(pixelsHRV + line * 5568 + pixelx);

            if(m_GeoSatellite == MET_9)
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
    }



    delete [] pixelsRed;
    delete [] pixelsGreen;
    delete [] pixelsBlue;
    delete [] pixelsHRV;

}


bool SegmentListGeostationary::allHRVColorSegmentsReceived()
{
    qDebug() << QString("SegmentListGeostationary::allHRVColorSegmentsReceived()");

    if(m_GeoSatellite == MET_10 || m_GeoSatellite == MET_9)
    {
    for(int i = (m_GeoSatellite == MET_9 ? 5 : 0); i < 8; i++)
    {
        if (isPresentRed[i] && issegmentcomposedRed[i] == false)
            return false;
        if (isPresentGreen[i] && issegmentcomposedGreen[i] == false)
            return false;
        if (isPresentBlue[i] && issegmentcomposedBlue[i] == false)
            return false;
     }
    }

    for(int i = (m_GeoSatellite == MET_9 ? 19 : (this->areatype == 0 ? 19 : 0)); i < 24; i++)
    {
        qDebug() << QString("index = %1 isPresent = %2 issegmentcomposedHRV = %3").arg(i).arg(isPresentHRV[i]).arg(issegmentcomposedHRV[i]);
        if (isPresentHRV[i] && issegmentcomposedHRV[i] == false)
            return false;
    }

    return true;
}

bool SegmentListGeostationary::allSegmentsReceived()
{

    qDebug() << QString("SegmentListGeostationary::allSegmentsReceived()");

    int pbCounter = 0;

    if (this->getKindofImage() == "VIS_IR Color")
    {
        if(m_GeoSatellite == FY2E || m_GeoSatellite == FY2G)
        {
            return true;
        }
        else if(m_GeoSatellite == MET_10 || m_GeoSatellite == MET_9)
        {
            for(int i = (m_GeoSatellite == MET_9 ? 5 : 0) ; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                    pbCounter++;
                if (isPresentGreen[i] && issegmentcomposedGreen[i] == true)
                    pbCounter++;
                if (isPresentBlue[i] && issegmentcomposedBlue[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == H8)
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
        if(m_GeoSatellite == MET_10)
        {
            for(int i = 0; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == MET_9)
        {
            for(int i = 5; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == MET_7)
        {
            for(int i = 0; i < 10; i++)
            {
                if (isPresentMono[i] && issegmentcomposedMono[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == GOES_13) // || m_GeoSatellite == ELECTRO_N1)
        {
            for(int i = 0; i < 7; i++)
            {
                if (isPresentMono[i] && issegmentcomposedMono[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == MTSAT)
        {
            for(int i = 0; i < 6; i++)
            {
                if (isPresentMono[i] && issegmentcomposedMono[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == FY2E || m_GeoSatellite == FY2G)
        {
                if (isPresentMono[0] && issegmentcomposedMono[0] == true)
                    pbCounter++;
        }
        else if(m_GeoSatellite == H8)
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
        if(m_GeoSatellite == MET_10 || m_GeoSatellite == MET_9)
        {
            for(int i = (m_GeoSatellite == MET_9 ? 5 : 0) ; i < 8; i++)
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
        else if(m_GeoSatellite == FY2E || m_GeoSatellite == FY2G)
        {
            if (isPresentHRV[0] && issegmentcomposedHRV[0] == true)
                pbCounter++;
        }
    }

    qDebug() << QString("SegmentListGeostationary::allSegmentsReceived() pbCounter = %1").arg(pbCounter);

    emit progressCounter(pbCounter);

    if (this->getKindofImage() == "VIS_IR Color")
    {
        if(m_GeoSatellite == MET_10 || m_GeoSatellite == MET_9)
        {
            for(int i = (m_GeoSatellite == MET_9 ? 5 : 0) ; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
                if (isPresentGreen[i] && issegmentcomposedGreen[i] == false)
                    return false;
                if (isPresentBlue[i] && issegmentcomposedBlue[i] == false)
                    return false;
            }
        } else if(m_GeoSatellite == H8)
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
        if(m_GeoSatellite == MET_10)
        {
            for(int i = 0; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
            }
        }
        else if(m_GeoSatellite == MET_9)
        {
            for(int i = 5; i < 8; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
            }
        }
        else if(m_GeoSatellite == MET_7)
        {
            for(int i = 0; i < 10; i++)
            {
                if (isPresentMono[i] && issegmentcomposedMono[i] == false)
                    return false;
            }
        }
/*        else if(m_GeoSatellite == ELECTRO_N1)
        {
            for(int i = 0; i < 6; i++)
            {
                if (isPresentMono[i] && issegmentcomposedMono[i] == false)
                    return false;
            }
        }
*/
        else if(m_GeoSatellite == GOES_13)
        {
            for(int i = 0; i < 7; i++)
            {
                if (isPresentMono[i] && issegmentcomposedMono[i] == false)
                    return false;
            }
        }
        else if(m_GeoSatellite == GOES_15)
        {
            for(int i = 0; i < 7; i++)
            {
                if (isPresentMono[i] && issegmentcomposedMono[i] == false)
                    return false;
            }
        }
        else if(m_GeoSatellite == MTSAT)
        {
            for(int i = 0; i < 7; i++)
            {
                if (isPresentMono[i] && issegmentcomposedMono[i] == false)
                    return false;
            }
        }
        else if(m_GeoSatellite == FY2E || m_GeoSatellite == FY2G)
        {
            if (isPresentMono[0] && issegmentcomposedMono[0] == false)
               return false;

        }
        else if(m_GeoSatellite == H8)
        {
            for(int i = 0; i < 10; i++)
            {
                if (isPresentRed[i] && issegmentcomposedRed[i] == false)
                    return false;
            }
        }


    }
    else if (this->getKindofImage() == "HRV" || this->getKindofImage() == "HRV Color")
    {
        if(m_GeoSatellite == MET_10 || m_GeoSatellite == MET_9)
        {

            for(int i = 19; i < 24; i++)
            {
                if (isPresentHRV[i] && issegmentcomposedHRV[i] == false)
                    return false;
            }
        }
        else if(m_GeoSatellite == FY2E || m_GeoSatellite == FY2G)
        {
            if (isPresentHRV[0] && issegmentcomposedHRV[0] == false)
                return false;
        }
    }

    qDebug() << "SegmentListGeostationary::allSegmentsReceived() returns true";

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



