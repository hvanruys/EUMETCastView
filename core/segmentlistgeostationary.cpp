#include <QApplication>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include "segmentlistgeostationary.h"
#include "segmentimage.h"

#include "MSG_HRIT.h"
#include <QMutex>

extern QMutex g_mutex;
extern Options opts;
extern SegmentImage *imageptrs;
// Meteosat
// Height = 3712 / 8 = 464 Width = 3712
// Height = 11136 / 24 = 464 Width = 7502
// Electro_n1
// Height = 2784 / 6 = 464 Width = 2784

void doComposeMeteosat(SegmentListGeostationary *sm, QString segment_path, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    sm->ComposeSegmentImage(segment_path, channelindex, spectrumvector, inversevector);
}

SegmentListGeostationary::SegmentListGeostationary(QObject *parent) :
    QObject(parent)
{

    for (int i = 0; i < 8; i++)
    {
        maxvalueRed[i] = 0;
        minvalueRed[i] = 0;
        maxvalueGreen[i] = 0;
        minvalueGreen[i] = 0;
        maxvalueBlue[i] = 0;
        minvalueBlue[i] = 0;
    }

    kindofimage = "";
    this->SetupContrastStretch( 0, 0, 250, 60, 750, 200, 1023, 255);
    COFF = 0;
    LOFF = 0;
    CFAC = 0;
    LFAC = 0;

    areatype = 0;
    ResetSegments();
    this->bActiveSegmentList = false;
    this->bisRSS = false;
    this->m_GeoSatellite = MET_10;

}

void SegmentListGeostationary::ResetSegments()
{
    for( int i = 0; i < 8; i++)
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

bool SegmentListGeostationary::ComposeImage(QFileInfo fileinfo, QVector<QString> spectrumvector, QVector<bool> inversevector)
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
        QFuture<void> future = QtConcurrent::run(doComposeMeteosat, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
        watcherHRV[filesequence].setFuture(future);
    }
    else if(m_GeoSatellite == MET_7 || m_GeoSatellite == ELECTRO_N1 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT)
    {
        QFuture<void> future = QtConcurrent::run(doComposeMeteosat, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
        watcherMono[filesequence].setFuture(future);
    }
    else
    {
        if( spectrumvector.at(1) == "" && spectrumvector.at(2) == "")
        {
            QFuture<void> future = QtConcurrent::run(doComposeMeteosat, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
            watcherRed[filesequence].setFuture(future);
        }
        else
        {
            if(spectrumvector.at(0) == filespectrum)
            {
                QFuture<void> future = QtConcurrent::run(doComposeMeteosat, this, fileinfo.filePath(), 0, spectrumvector, inversevector);
                watcherRed[filesequence].setFuture(future);
            }
            else if(spectrumvector.at(1) == filespectrum)
            {
                QFuture<void> future = QtConcurrent::run(doComposeMeteosat, this, fileinfo.filePath(), 1, spectrumvector, inversevector);
                watcherGreen[filesequence].setFuture(future);
            }
            else if(spectrumvector.at(2) == filespectrum)
            {
                QFuture<void> future = QtConcurrent::run(doComposeMeteosat, this, fileinfo.filePath(), 2, spectrumvector, inversevector);
                watcherBlue[filesequence].setFuture(future);
            }
        }
    }

    return true;
}

void SegmentListGeostationary::InsertPresent( QVector<QString> spectrumvector, QString filespectrum, int filesequence)
{
    qDebug() << QString("InsertPresent ; spectrum %1 %2 %3    filespectrum  %4  fileseq %5").arg(spectrumvector[0]).arg(spectrumvector[1]).arg(spectrumvector[2]).arg(filespectrum).arg(filesequence);
    if(m_GeoSatellite == MET_10 || m_GeoSatellite == MET_9)
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
    else if(m_GeoSatellite == MET_7 || m_GeoSatellite == ELECTRO_N1 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT)
    {
        isPresentMono[filesequence] = true;
    }

}

void SegmentListGeostationary::ComposeSegmentImage( QString filepath, int channelindex, QVector<QString> spectrumvector, QVector<bool> inversevector )
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

    qDebug() << QString("---->[%1] SegmentListGeostationary::ComposeSegmentImage() planned end = %2 npix = %3 nlin = %4 fileseqeunce = %5").arg(kindofimage).arg(planned_end_segment).arg(number_of_columns).arg(number_of_lines).arg(filesequence);

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
    else if(m_GeoSatellite == MET_7 || m_GeoSatellite == ELECTRO_N1 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT)
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

    //ComposeWithGamma();

    for(int line = 0; line < nlin; line++)
    {
        //qDebug() << QString("filesequence = %1 ; nlin * totalsegs - 1 - startLine[filesequence] - line = %2").arg(filesequence).arg(nlin * totalsegs - 1 - startLine[filesequence] - line);

        if(m_GeoSatellite == ELECTRO_N1 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT)
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

            valgamma = pow( c, gamma) * gammafactor;
            valcontrast = ContrastStretch(valgamma);

            // qDebug() << QString("npix - 1 - pixelx = %1").arg(npix - 1 - pixelx);



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
                if(m_GeoSatellite == ELECTRO_N1 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT)
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
    else if(m_GeoSatellite == MET_7 || m_GeoSatellite == ELECTRO_N1 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT)
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

void SegmentListGeostationary::ComposeWithGamma()
{
    QRgb *row_col;

    int nlin = number_of_lines;
    int npix = number_of_columns;

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


//    for(int line = 0; line < nlin; line++)
//    {
//        //qDebug() << QString("filesequence = %1 ; nlin * totalsegs - 1 - startLine[filesequence] - line = %2").arg(filesequence).arg(nlin * totalsegs - 1 - startLine[filesequence] - line);

//        if(m_GeoSatellite == ELECTRO_N1 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT)
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

//            valgamma = pow( c, gamma) * gammafactor;
//            valcontrast = ContrastStretch(valgamma);

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
//                if(m_GeoSatellite == ELECTRO_N1 || m_GeoSatellite == GOES_13 || m_GeoSatellite == GOES_15 || m_GeoSatellite == MTSAT)
//                    row_col[pixelx] = qRgb(r,g,b);
//                else
//                    row_col[npix - 1 - pixelx] = qRgb(r,g,b);

//            }
//        }
//    }
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
    else if(m_GeoSatellite == ELECTRO_N1)
    {
        npixHRV = 5568*5*464;
    }


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
    qDebug() << QString("SegmentListMeteosat::allHRVColorSegmentsReceived()");

    for(int i = (m_GeoSatellite == MET_9 ? 5 : 0); i < 8; i++)
    {
        if (isPresentRed[i] && issegmentcomposedRed[i] == false)
            return false;
        if (isPresentGreen[i] && issegmentcomposedGreen[i] == false)
            return false;
        if (isPresentBlue[i] && issegmentcomposedBlue[i] == false)
            return false;
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

    int pbCounter = 0;

    if (this->getKindofImage() == "VIS_IR Color")
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
        else if(m_GeoSatellite == ELECTRO_N1)
        {
            for(int i = 0; i < 6; i++)
            {
                if (isPresentMono[i] && issegmentcomposedMono[i] == true)
                    pbCounter++;
            }
        }
        else if(m_GeoSatellite == GOES_13 || m_GeoSatellite == ELECTRO_N1)
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
    }
    else if (this->getKindofImage() == "HRV" || this->getKindofImage() == "HRV Color")
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

    qDebug() << QString("SegmentListGeostationary::allSegmentsReceived() pbCounter = %1").arg(pbCounter);

    emit progressCounter(pbCounter);

    if (this->getKindofImage() == "VIS_IR Color")
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
        else if(m_GeoSatellite == ELECTRO_N1)
        {
            for(int i = 0; i < 6; i++)
            {
                if (isPresentMono[i] && issegmentcomposedMono[i] == false)
                    return false;
            }
        }
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

    }
    else if (this->getKindofImage() == "HRV" || this->getKindofImage() == "HRV Color")
    {
        for(int i = 19; i < 24; i++)
        {
            if (isPresentHRV[i] && issegmentcomposedHRV[i] == false)
                return false;
        }
    }

    qDebug() << "SegmentListGeostationary::allSegmentsReceived() returns true";

    return true;
}


void SegmentListGeostationary::CalculateMinMax()
{
    for (int i = 0; i < 8; i++)
    {
        minvalueRed[i] = 65535;
        maxvalueRed[i] = 0;
        minvalueGreen[i] = 65535;
        maxvalueGreen[i] = 0;
        minvalueBlue[i] = 65535;
        maxvalueBlue[i] = 0;
    }
   // qDebug() << QString("in SegmentListMeteosat::CalculateMinMax() this->segmentcurVIS006[i]->minvalue = %1").arg(this->segmentcurVIS006[0]->minvalue);
/*

    for( int i = 0; i < 8; i++)
    {
        if (this->minvalue < minvalueRed)
            minvalueRed = this->segmentRed[i]->minvalue;
        if (this->segmentRed[i]->maxvalue > maxvalueRed)
            maxvalueRed = this->segmentRed[i]->maxvalue;
    }

    for( int i = 0; i < 8; i++)
    {
        if (this->segmentGreen[i]->minvalue < minvalueGreen)
            minvalueGreen = this->segmentGreen[i]->minvalue;
        if (this->segmentGreen[i]->maxvalue > maxvalueGreen)
            maxvalueGreen = this->segmentGreen[i]->maxvalue;
    }

    for( int i = 0; i < 8; i++)
    {
        if (this->segmentBlue[i]->minvalue < minvalueBlue)
            minvalueBlue = this->segmentBlue[i]->minvalue;
        if (this->segmentBlue[i]->maxvalue > maxvalueBlue)
            maxvalueBlue = this->segmentBlue[i]->maxvalue;
    }

    qDebug() << QString("Globe::displayMinMax() Red min = %1  max = %2").arg(minvalueRed).arg(maxvalueRed);
    qDebug() << QString("Globe::displayMinMax() Green min = %1  max = %2").arg(minvalueGreen).arg(maxvalueGreen);
    qDebug() << QString("Globe::displayMinMax() Blue min = %1  max = %2").arg(minvalueBlue).arg(maxvalueBlue);
*/
}

void SegmentListGeostationary::SetupContrastStretch(quint16 x1, quint16 y1, quint16 x2, quint16 y2, quint16 x3, quint16 y3, quint16 x4, quint16 y4)
{
    //Q_ASSERT(val < 1023 && x1 < 1023 && x2 < 1023 && x3 < 1023 && x4 < 1023 && y1 < 255 && y2 < 255 && y3 < 255 && y4 < 255 && x1 < x2 < x3 < x4 && y1 < y2 < y3 < y4);

    this->d_x1 = (double)x1;
    this->d_x2 = (double)x2;
    this->d_x3 = (double)x3;
    this->d_x4 = (double)x4;
    this->d_y1 = (double)y1;
    this->d_y2 = (double)y2;
    this->d_y3 = (double)y3;
    this->d_y4 = (double)y4;

    A1 = (d_y2 - d_y1)/(d_x2 - d_x1);
    B1 = (d_y2 - (A1*d_x2));
    A2 = (d_y3 - d_y2)/(d_x3 - d_x2);
    B2 = (d_y3 - (A2*d_x3));
    A3 = (d_y4 - d_y3)/(d_x4 - d_x3);
    B3 = (d_y4 - (A3*d_x4));
    //qDebug() << QString("A1 = %1;B1 = %2;A2 = %3;B2 = %4;A3 = %5;B3 = %6").arg(A1).arg(B1).arg(A2).arg(B2).arg(A3).arg(B3);
}

quint8 SegmentListGeostationary::ContrastStretch(quint16 val)
{
    double res;
    if (val >= d_x1 && val < d_x2 )
    {
        res = double(val)*A1 + B1;
        return quint8(res);
    }
    else if(val >= d_x2 && val < d_x3 )
    {
        res = double(val)*A2 + B2;
        return quint8(res);
    }
    else if(val >= d_x3 && val <= d_x4 )
    {
        res = double(val)*A3 + B3;
        return quint8(res);
    }
}




