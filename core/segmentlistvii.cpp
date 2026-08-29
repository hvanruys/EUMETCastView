#include "segmentlistvii.h"
#include "segmentimage.h"
#include "options.h"
#include <QtConcurrent>

extern Options opts;
extern SegmentImage *imageptrs;
extern bool ptrimagebusy;
extern SatelliteList satellitelist;

// 3144 divides exactly by 8, so unlike OLCI the CLAHE pass needs no cropping
// in the across track direction.
static const int kClaheRegionsX = 8;
static const int kClaheRegionsY = 16;

void SegmentListVII::doComposeVIIImageInThread(SegmentListVII *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{
    t->ComposeVIIImageInThread(bandlist, colorlist, invertlist);
}


SegmentListVII::SegmentListVII(eSegmentType type, QObject *parent) :
    SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListVII");

    seglisttype = type;
    histogrammethod = 0; // 0 none , 1 equalize
    normalized = false;
    watchervii = 0;
}

bool SegmentListVII::ComposeVIIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, int histogrammethod, bool normalized)
{
    qDebug() << QString("SegmentListVII::ComposeVIIImage");
    qDebug() << QString("bandlist has nbr of items : %1").arg(bandlist.count());

    this->bandlist = bandlist;
    this->colorlist = colorlist;
    this->invertlist = invertlist;
    this->histogrammethod = histogrammethod;
    this->normalized = normalized;

    ptrimagebusy = true;
    QApplication::setOverrideCursor(( Qt::WaitCursor));
    watchervii = new QFutureWatcher<void>(this);

    // Composing an image connects the watcher, and this runs once per image, so
    // without UniqueConnection the connections pile up and finishedviirs() is
    // called once for every image composed this session - the fifth compose runs
    // it five times, each one reprojecting and redrawing from the start.
    connect(watchervii, SIGNAL(finished()), this, SLOT(finishedvii()), Qt::UniqueConnection);

    QFuture<void> future;
    future = QtConcurrent::run(doComposeVIIImageInThread, this, bandlist, colorlist, invertlist);
    watchervii->setFuture(future);

    return true;

}

bool SegmentListVII::ComposeVIIImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{


    // No QApplication calls on a QtConcurrent worker: ComposeVIIImage sets the
    // wait cursor before starting this and finishedvii restores it.
    progressresultready = 0;

    this->totalnbroflines = 0;


    emit progressCounter(10);

    for (int i=0; i < 3; i++)
    {
        for (int j=0; j < 1024; j++)
        {
            imageptrs->lut_ch[i][j] = 0;
            imageptrs->lut_norm_ch[i][j] = 0;
        }
    }

    for(int k = 0; k < 3; k++)
    {
        imageptrs->stat_min = 99999999;
        imageptrs->stat_max = 0;
        imageptrs->stat_max_ch[k] = 0;
        imageptrs->stat_min_ch[k] = 9999999;
        this->stat_max_ch[k] = 0;
        this->stat_min_ch[k] = 9999999;
        imageptrs->stat_max_norm_ch[k] = 0;
        imageptrs->stat_min_norm_ch[k] = 9999999;
        this->stat_max_norm_ch[k] = 0;
        this->stat_min_norm_ch[k] = 9999999;
        imageptrs->minRadianceIndex[k] = 999999;
        imageptrs->maxRadianceIndex[k] = 0;
        imageptrs->minRadianceIndexNormalized[k] = 999999;
        imageptrs->maxRadianceIndexNormalized[k] = 0;
    }

    // Reset memory
    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);
        segm->resetMemory();
        ++segsel;
    }
    segsselected.clear();

    int startlinenbr = 0;
    int totalnbrofsegments = 0;

    QList<Segment*>::iterator segit = segmentlist.begin();

    // create QList of selected segments
    while ( segit != segmentlist.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segit);
        if (segm->segmentselected)
        {
            segsselected.append(segm);
            totalnbrofsegments++;
        }
        ++segit;
    }

    if(totalnbrofsegments == 0)
    {
        emit progressCounter(100);
        return true;
    }

    int deltaprogress = 99 / (totalnbrofsegments*3);
    int totalprogress = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        segm->setBandandColor(bandlist, colorlist, invertlist);
        segm->ReadSegmentInMemory();

        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        if(segsel == segsselected.begin())
            this->earth_views_per_scanline = segm->getEarthViewsPerScanline();
        ++segsel;
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        segm->setStartLineNbr(startlinenbr);
        startlinenbr += segm->NbrOfLines;
        totalnbroflines += segm->NbrOfLines;
        ++segsel;
    }

    // image pointers always = new QImage()
    if(imageptrs->ptrimageVII != NULL)
    {
        delete imageptrs->ptrimageVII;
        imageptrs->ptrimageVII = NULL;
    }

    imageptrs->ptrimageVII = new QImage(this->earth_views_per_scanline, totalnbroflines, QImage::Format_ARGB32);
    qDebug() << QString("ptrimageVII created %1 x %2").arg(this->earth_views_per_scanline).arg(totalnbroflines);


    bool composecolor = false;

    long cnt_active_pixels = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        composecolor = segm->composeColorImage();


        for(int i = 0; i < (composecolor ? 3 : 1); i++)
        {
            if( segm->stat_max_ch[i] > this->stat_max_ch[i])
                this->stat_max_ch[i] = segm->stat_max_ch[i];
            if( segm->stat_min_ch[i] < this->stat_min_ch[i])
                this->stat_min_ch[i] = segm->stat_min_ch[i];
            if( segm->stat_max_norm_ch[i] > this->stat_max_norm_ch[i])
                this->stat_max_norm_ch[i] = segm->stat_max_norm_ch[i];
            if( segm->stat_min_norm_ch[i] < this->stat_min_norm_ch[i])
                this->stat_min_norm_ch[i] = segm->stat_min_norm_ch[i];
        }
        cnt_active_pixels += segm->active_pixels[0];
        ++segsel;
    }


    for(int i = 0; i < (composecolor ? 3 : 1); i++)
    {
        imageptrs->stat_max_ch[i] = this->stat_max_ch[i];
        imageptrs->stat_min_ch[i] = this->stat_min_ch[i];
        imageptrs->stat_max_norm_ch[i] = this->stat_max_norm_ch[i];
        imageptrs->stat_min_norm_ch[i] = this->stat_min_norm_ch[i];
    }

    for(int k = 0; k < (composecolor ? 3 : 1); k++)
    {
        if(imageptrs->stat_max_ch[k] > imageptrs->stat_max)
            imageptrs->stat_max = imageptrs->stat_max_ch[k];
        if(imageptrs->stat_min_ch[k] < imageptrs->stat_min)
            imageptrs->stat_min = imageptrs->stat_min_ch[k];
    }

    imageptrs->active_pixels = cnt_active_pixels;

    qDebug() << QString("imageptrs stat_min_ch[0] = %1 stat_max_ch[0] = %2").arg(imageptrs->stat_min_ch[0]).arg(imageptrs->stat_max_ch[0]);
    if(composecolor)
    {
        qDebug() << QString("imageptrs stat_min_ch[1] = %1 stat_max_ch[1] = %2").arg(imageptrs->stat_min_ch[1]).arg(imageptrs->stat_max_ch[1]);
        qDebug() << QString("imageptrs stat_min_ch[2] = %1 stat_max_ch[2] = %2").arg(imageptrs->stat_min_ch[2]).arg(imageptrs->stat_max_ch[2]);
    }
    qDebug() << QString("imageptrs stat_min_norm_ch[0] = %1 stat_max_norm_ch[0] = %2").arg(imageptrs->stat_min_norm_ch[0]).arg(imageptrs->stat_max_norm_ch[0]);
    if(composecolor)
    {
        qDebug() << QString("imageptrs stat_min_norm_ch[1] = %1 stat_max_norm_ch[1] = %2").arg(imageptrs->stat_min_norm_ch[1]).arg(imageptrs->stat_max_norm_ch[1]);
        qDebug() << QString("imageotrs stat_min_norm_ch[2] = %1 stat_max_norm_ch[2] = %2").arg(imageptrs->stat_min_norm_ch[2]).arg(imageptrs->stat_max_norm_ch[2]);
    }
    qDebug() << QString("imageptrs stat_min = %1 stat_max = %2").arg(imageptrs->stat_min).arg(imageptrs->stat_max);


    // Without these the LUTs stay all zero and every equalized or 95% stretched
    // image comes out black.
    if(imageptrs->active_pixels > 0)
    {
        CalculateLUTAlt();
        CalculateLUTFull();
    }
    else
        qDebug() << "SegmentListVII::ComposeVIIImageInThread : no valid pixels, skipping the LUTs";

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        segm->ComposeSegmentImage(this->histogrammethod, this->normalized);
        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        // processEvents drives the calling thread's event loop, and this thread
        // has nothing in one.
        ++segsel;
    }

    qDebug() << " SegmentListVII::ComposeVIIImageInThread Finished !!";

    // The finished slot emits this, on the GUI thread and after the
    // compose state is consistent. Emitting it here as well made every
    // composed image reproject and redraw twice.
    return true;
}

void SegmentListVII::finishedvii()
{

    qDebug() << "=============>SegmentListVII::finishedvii()";
    emit progressCounter(100);
    opts.texture_changed = true;
    ptrimagebusy = false;
    delete watchervii;
    watchervii = 0;
    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
}

void SegmentListVII::progressreadvalue(int progress)
{
    int totalcount = segsselected.count();
    if(totalcount == 0)
        return;
    this->progressresultready += 100 / totalcount;

    emit progressCounter(this->progressresultready);

    qDebug() << QString("SegmentListVII::progressreadvalue( %1 )").arg(progress);
}

bool SegmentListVII::searchLatLon(int mapx, int mapy, float &lon, float &lat)
{
    QList<Segment*>::iterator segsel = segsselected.begin();

    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        if( mapy >= segm->startLineNbr && mapy < segm->endLineNbr
            && mapx >= 0 && mapx < segm->earth_views_per_scanline
            && !segm->geolatitude.isNull())
        {
            lat = segm->geolatitude[(mapy - segm->startLineNbr) * segm->earth_views_per_scanline + mapx];
            lon = segm->geolongitude[(mapy - segm->startLineNbr) * segm->earth_views_per_scanline + mapx];
            return(true);
        }

        ++segsel;
    }
    return(false);
}

bool SegmentListVII::ChangeHistogramMethod()
{

    qDebug() << "bool SegmentListVII::ChangeHistogramMethod() started";

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    emit progressCounter(10);

    // image pointers always = new QImage()
    if(imageptrs->ptrimageVII != NULL)
    {
        delete imageptrs->ptrimageVII;
        imageptrs->ptrimageVII = NULL;
    }

    imageptrs->ptrimageVII = new QImage(this->earth_views_per_scanline, this->totalnbroflines, QImage::Format_ARGB32);
    qDebug() << QString("ptrimageVII created %1 x %2").arg(this->earth_views_per_scanline).arg(totalnbroflines);

    if (this->histogrammethod == CMB_HISTO_NONE_95 || this->histogrammethod == CMB_HISTO_NONE_100 || this->histogrammethod == CMB_HISTO_EQUALIZE)
        ComposeSegments();
    else if (this->histogrammethod == CMB_HISTO_CLAHE)
        RecalculateCLAHEVII();


    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
    emit progressCounter(100);
    return true;
}

void SegmentListVII::ComposeSegments()
{
    QList<Segment*>::iterator segsel = segsselected.begin();

    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        segm->ComposeSegmentImage(this->histogrammethod, this->normalized);
        QApplication::processEvents();
        ++segsel;
    }

}

void SegmentListVII::Compose48bitPNG(QString fileName, bool mapto65535)
{
    int height = NbrOfSegmentLinesSelected();
    int width = earth_views_per_scanline;

    if(height == 0 || width == 0)
        return;

    // initialize the FreeImage library
    FreeImage_Initialise();

    FIBITMAP *bitmap = FreeImage_AllocateT(FIT_RGB16, width, height);

    QList<Segment*>::iterator segsel = segsselected.begin();
    int heightinsegment = 0;
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        Compose48bitPNGSegment(segm, bitmap, heightinsegment, mapto65535);
        heightinsegment += segm->GetNbrOfLines();
        ++segsel;
    }


    QByteArray array = fileName.toLocal8Bit();
    char* pfileName = array.data();

    if(FreeImage_Save(FIF_PNG,bitmap, pfileName,0))
    {
        qDebug() << "bitmap successfully saved!";
    }

    FreeImage_Unload(bitmap);

    FreeImage_DeInitialise();

}


void SegmentListVII::Compose48bitPNGSegment(SegmentVII *segm, FIBITMAP *bitmap, int heightinsegment, bool mapto65535)
{

    quint16 pixval[3];
    bool iscolor = bandlist.at(0);

    for (int line = 0; line < segm->GetNbrOfLines(); line++)
    {
        FIRGB16 *bits = (FIRGB16 *)FreeImage_GetScanLine(bitmap, totalnbroflines - line - heightinsegment - 1);
        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
        {
            for(int k = 0; k < (iscolor ? 3 : 1); k++)
            {
                const quint16 raw = segm->ptrbaVII[k][line * earth_views_per_scanline + pixelx];
                if(raw == 65535)          // no data
                    pixval[k] = 0;
                else if(mapto65535)
                {
                    // The values already span the channel's full valid range, so
                    // "mapped to 0-65535" means stretching what this image
                    // actually contains, not rescaling the packing.
                    const long span = imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k];
                    pixval[k] = span > 0
                        ? (quint16)qBound(0, qRound(65535.0 * (double)(raw - imageptrs->stat_min_ch[k]) / (double)span), 65535)
                        : raw;
                }
                else
                    pixval[k] = raw;
            }

            bits[pixelx].red = pixval[0];
            bits[pixelx].green = iscolor ? pixval[1] : pixval[0];
            bits[pixelx].blue = iscolor ? pixval[2] : pixval[0];

        }
    }
}

long SegmentListVII::NbrOfSaturatedPixels()
{
    QList<Segment*>::iterator segsel = segsselected.begin();
    long nbrofpixels = 0;
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        nbrofpixels += segm->nbrsaturatedpixels;
        ++segsel;
    }
    return nbrofpixels;
}


void SegmentListVII::RecalculateCLAHEVII()
{
    quint16 *pixels[3];
    QRgb *row;

    int pixval[3];

    bool iscolor = bandlist.at(0);
    const int width = earth_views_per_scanline;

    qDebug() << " earth_views_per_scanline = " << width << " totalnbroflines = " << totalnbroflines;

    // CLAHE refuses a height that is not a multiple of the region count
    const int nbroflinesreduced = (totalnbroflines / kClaheRegionsY) * kClaheRegionsY;
    if(nbroflinesreduced == 0 || width % kClaheRegionsX != 0)
    {
        qDebug() << "SegmentListVII::RecalculateCLAHEVII : geometry does not fit the CLAHE regions";
        return;
    }
    const size_t npix = (size_t)nbroflinesreduced * width;

    // image pointers always = new QImage()
    if(imageptrs->ptrimageVII != NULL)
    {
        delete imageptrs->ptrimageVII;
        imageptrs->ptrimageVII = NULL;
    }

    imageptrs->ptrimageVII = new QImage(width, nbroflinesreduced, QImage::Format_ARGB32);
    qDebug() << QString("ptrimageVII created %1 x %2").arg(width).arg(nbroflinesreduced);

    for(int k = 0; k < 3; k++)
        pixels[k] = new quint16[npix];

    // CLAHE works on a 0..1023 range, so the packed radiances have to be
    // stretched into it rather than clipped - at 16 bits a clip would leave
    // almost the whole image saturated.
    int lineoffset = 0;
    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        for(int i = 0; i < segm->GetNbrOfLines() && lineoffset + i < nbroflinesreduced; i++)
        {
            for(int j = 0; j < width; j++)
            {
                for(int k = 0; k < (iscolor ? 3 : 1); k++)
                {
                    const quint16 raw = normalized
                        ? segm->ptrbaVIInormalized[k][i * width + j]
                        : segm->ptrbaVII[k][i * width + j];
                    const long lo = normalized ? imageptrs->stat_min_norm_ch[k] : imageptrs->stat_min_ch[k];
                    const long hi = normalized ? imageptrs->stat_max_norm_ch[k] : imageptrs->stat_max_ch[k];
                    quint16 out = 0;
                    if(raw < 65535 && hi > lo)
                        out = (quint16)qBound(0, qRound(1023.0 * (double)(raw - lo) / (double)(hi - lo)), 1023);
                    pixels[k][(size_t)(lineoffset + i) * width + j] = out;
                }
            }
        }
        lineoffset += segm->GetNbrOfLines();
        ++segsel;
    }

    // CLAHE reports a refused geometry through its return value and leaves the
    // buffer untouched, which would come out as a plain linear stretch with
    // nothing to say why.
    for(int k = 0; k < (iscolor ? 3 : 1); k++)
    {
        const int ret = imageptrs->CLAHE(pixels[k], width, nbroflinesreduced, 0, 1024, kClaheRegionsX, kClaheRegionsY, 256, 6.9);
        if(ret != 0)
            qWarning() << QString("SegmentListVII::RecalculateCLAHEVII : CLAHE refused channel %1 with %2 for %3 x %4")
                          .arg(k).arg(ret).arg(width).arg(nbroflinesreduced);
    }

    for (int line = 0; line < nbroflinesreduced; line++)
    {
        row = (QRgb*)imageptrs->ptrimageVII->scanLine(line);
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            for(int k = 0; k < (iscolor ? 3 : 1); k++)
            {
                pixval[k] = pixels[k][(size_t)line * width + pixelx] / 4;
                pixval[k] = pixval[k] > 255 ? 255 : pixval[k];
            }
            row[pixelx] = qRgba(pixval[0], iscolor ? pixval[1] : pixval[0], iscolor ? pixval[2] : pixval[0], 255 );
        }
    }

    for(int k = 0; k < 3; k++)
        delete [] pixels[k];
}

void SegmentListVII::CalculateLUT()
{
    qDebug() << "start SegmentListVII::CalculateLUT()";
    int earth_views = this->earth_views_per_scanline;
    long stats_ch[3][256];

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 256; j++)
        {
            stats_ch[k][j] = 0;
        }
    }

    bool composecolor = false;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
                    quint16 pixel = *(segm->ptrbaVII[k].data() + line * earth_views + pixelx);
                    if(pixel < 65535)
                    {
                        int pixcalc = 255 * (pixel - imageptrs->stat_min_ch[k]) / qMax(1L, imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k]);
                        pixcalc = ( pixcalc < 0 ? 0 : pixcalc);
                        pixcalc = ( pixcalc > 255 ? 255 : pixcalc );
                        stats_ch[k][pixcalc]++;
                    }
                }
            }
        }
        ++segsel;
    }

    float newscale = 256.0 / imageptrs->active_pixels;

    unsigned long long sum_ch[3];

    for (int i=0; i < 3; i++)
    {
        sum_ch[i] = 0;
    }


    for( int i = 0; i < 256; i++)
    {
        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            sum_ch[k] += stats_ch[k][i];
            imageptrs->lut_ch[k][i] = (quint16)(sum_ch[k] * newscale);
            imageptrs->lut_ch[k][i] = ( imageptrs->lut_ch[k][i] > 255 ? 255 : imageptrs->lut_ch[k][i]);
        }
    }
}

void SegmentListVII::CalculateLUTFull()
{
    qDebug() << "start SegmentListVII::CalculateLUTFull()";
    int earth_views = this->earth_views_per_scanline;
    long stats_ch[3][1024];
    long stats_norm_ch[3][1024];

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 1024; j++)
        {
            stats_ch[k][j] = 0;
            stats_norm_ch[k][j] = 0;
        }
    }

    bool composecolor = false;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
                    quint16 pixel = *(segm->ptrbaVII[k].data() + line * earth_views + pixelx) ;
                    if(pixel == 65535)
                        continue;
                    quint16 indexout = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - imageptrs->stat_min_ch[k])/(float)qMax(1L, imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k])), 0), 1023);
                    stats_ch[k][indexout]++;
                    quint16 pixelnorm = *(segm->ptrbaVIInormalized[k].data() + line * earth_views + pixelx) ;
                    quint16 indexoutnorm = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixelnorm - imageptrs->stat_min_norm_ch[k])/(float)qMax(1L, imageptrs->stat_max_norm_ch[k] - imageptrs->stat_min_norm_ch[k])), 0), 1023);
                    stats_norm_ch[k][indexoutnorm]++;
                }
            }
        }
        ++segsel;
    }


    double newscale = (double)(1024.0 / imageptrs->active_pixels);

    qDebug() << QString("newscale = %1 active pixels = %2").arg(newscale).arg(imageptrs->active_pixels);

    unsigned long long sum_ch[3];
    unsigned long long sum_norm_ch[3];

    for (int i=0; i < 3; i++)
    {
        sum_ch[i] = 0;
        sum_norm_ch[i] = 0;
    }


    bool okmin[3], okmax[3];

    for(int k = 0; k < 3; k++)
    {
        okmin[k] = false;
        okmax[k] = false;
    }

    // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
    for( int i = 0; i < 1024; i++)
    {
        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            sum_ch[k] += stats_ch[k][i];
            imageptrs->lut_ch[k][i] = (quint16)((double)sum_ch[k] * newscale);
            imageptrs->lut_ch[k][i] = ( imageptrs->lut_ch[k][i] > 1023 ? 1023 : imageptrs->lut_ch[k][i]);
            if(imageptrs->lut_ch[k][i] > 25 && okmin[k] == false)
            {
                okmin[k] = true;
                imageptrs->minRadianceIndex[k] = i;
            }
            if(imageptrs->lut_ch[k][i] > 997 && okmax[k] == false)
            {
                okmax[k] = true;
                imageptrs->maxRadianceIndex[k] = i;
            }
        }
    }

    for(int k = 0; k < 3; k++)
    {
        okmin[k] = false;
        okmax[k] = false;
    }

    for( int i = 0; i < 1024; i++)
    {
        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            sum_norm_ch[k] += stats_norm_ch[k][i];
            imageptrs->lut_norm_ch[k][i] = (quint16)((double)sum_norm_ch[k] * newscale);
            imageptrs->lut_norm_ch[k][i] = ( imageptrs->lut_norm_ch[k][i] > 1023 ? 1023 : imageptrs->lut_norm_ch[k][i]);
            if(imageptrs->lut_norm_ch[k][i] > 25 && okmin[k] == false)
            {
                okmin[k] = true;
                imageptrs->minRadianceIndexNormalized[k] = i;
            }
            if(imageptrs->lut_norm_ch[k][i] > 997 && okmax[k] == false)
            {
                okmax[k] = true;
                imageptrs->maxRadianceIndexNormalized[k] = i;
            }
        }
    }

    for(int k = 0; k < (composecolor ? 3 : 1); k++)
    {
        qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4").arg(k).arg(imageptrs->minRadianceIndex[k]).arg(k).arg(imageptrs->maxRadianceIndex[k]);
        qDebug() << QString("minRadianceIndexNormalized [%1] = %2 maxRadianceIndexNormalized [%3] = %4").arg(k).arg(imageptrs->minRadianceIndexNormalized[k]).arg(k).arg(imageptrs->maxRadianceIndexNormalized[k]);
    }

}

void SegmentListVII::CalculateLUTAlt()
{
    qDebug() << "start SegmentListVII::CalculateLUTAlt()";
    int earth_views = this->earth_views_per_scanline;
    long stats[256];

    for (int j = 0; j < 256; j++)
    {
        stats[j] = 0;
    }


    bool composecolor;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        composecolor = segm->composeColorImage();

        if(composecolor)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
                    quint16 pixel0 = *(segm->ptrbaVII[0].data() + line * earth_views + pixelx);
                    quint16 pixel1 = *(segm->ptrbaVII[1].data() + line * earth_views + pixelx);
                    quint16 pixel2 = *(segm->ptrbaVII[2].data() + line * earth_views + pixelx);
                    if(pixel0 == 65535 || pixel1 == 65535 || pixel2 == 65535)
                        continue;
                    quint16 pixcalc0 = 256 * (pixel0 - imageptrs->stat_min_ch[0]) / qMax(1L, imageptrs->stat_max_ch[0] - imageptrs->stat_min_ch[0]);
                    quint16 pixcalc1 = 256 * (pixel1 - imageptrs->stat_min_ch[1]) / qMax(1L, imageptrs->stat_max_ch[1] - imageptrs->stat_min_ch[1]);
                    quint16 pixcalc2 = 256 * (pixel2 - imageptrs->stat_min_ch[2]) / qMax(1L, imageptrs->stat_max_ch[2] - imageptrs->stat_min_ch[2]);

                    int pixel = (int)((float)pixcalc0 * 0.299 + (float)pixcalc1 * 0.587 + (float)pixcalc2 * 0.114);

                    pixel = ( pixel < 0 ? 0 : pixel);
                    pixel = ( pixel > 255 ? 255 : pixel );
                    stats[pixel]++;
                }
            }
        }
        else
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
                    quint16 pixel0 = *(segm->ptrbaVII[0].data() + line * earth_views + pixelx);
                    if(pixel0 == 65535)
                        continue;
                    int pixel = 256 * (pixel0 - imageptrs->stat_min_ch[0]) / qMax(1L, imageptrs->stat_max_ch[0] - imageptrs->stat_min_ch[0]);

                    pixel = ( pixel < 0 ? 0 : pixel);
                    pixel = ( pixel > 255 ? 255 : pixel );
                    stats[pixel]++;
                }
            }

        }
        ++segsel;
    }

    float newscale = 256.0 / imageptrs->active_pixels;

    unsigned long long sum_ch = 0;

    for( int i = 0; i < 256; i++)
    {
        sum_ch += stats[i];
        imageptrs->lut_sentinel[i] = (quint16)(sum_ch * newscale);
        imageptrs->lut_sentinel[i] = ( imageptrs->lut_sentinel[i] > 255 ? 255 : imageptrs->lut_sentinel[i]);
    }
}

void SegmentListVII::CalculateProjectionLUT()
{
    qDebug() << "start SegmentListVII::CalculateProjectionLUT()";
    long stats_ch[3][1024];
    long cnt_active_pixels = 0;

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 1024; j++)
        {
            stats_ch[k][j] = 0;
        }
    }

    for (int i=0; i < 3; i++)
    {
        for (int j=0; j < 1024; j++)
        {
            imageptrs->lut_proj_ch[i][j] = 0;
        }
    }

    for(int k = 0; k < 3; k++)
    {
        imageptrs->stat_max_proj_ch[k] = 0;
        imageptrs->stat_min_proj_ch[k] = 9999999;
        this->stat_max_proj_ch[k] = 0;
        this->stat_min_proj_ch[k] = 9999999;
    }

    bool composecolor = false;

    int x, y;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        segm->recalculateStatsInProjection(this->normalized);
        ++segsel;
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int i = 0; i < (composecolor ? 3 : 1); i++)
        {
            if( segm->stat_max_projection[i] > this->stat_max_proj_ch[i])
                this->stat_max_proj_ch[i] = segm->stat_max_projection[i];
            if( segm->stat_min_projection[i] < this->stat_min_proj_ch[i])
                this->stat_min_proj_ch[i] = segm->stat_min_projection[i];
        }
        cnt_active_pixels += segm->active_pixels_projection;
        ++segsel;
    }


    for(int i = 0; i < (composecolor ? 3 : 1); i++)
    {
        imageptrs->stat_max_proj_ch[i] = this->stat_max_proj_ch[i];
        imageptrs->stat_min_proj_ch[i] = this->stat_min_proj_ch[i];
    }

    imageptrs->active_pixels_proj = cnt_active_pixels;

    if(cnt_active_pixels == 0)
        return;

    quint16 pixel;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < segm->earth_views_per_scanline; pixelx++)
                {
                    x = segm->getProjectionX(line, pixelx);
                    y = segm->getProjectionY(line, pixelx);
                    if(x >= 0 && x < imageptrs->ptrimageProjection->width() && y >= 0 && y < imageptrs->ptrimageProjection->height())
                    {
                        if(normalized) pixel = segm->ptrbaVIInormalized[k][line * segm->earth_views_per_scanline + pixelx];
                        else pixel = segm->ptrbaVII[k][line * segm->earth_views_per_scanline + pixelx];

                        int pixcalc = 1023 * (pixel - imageptrs->stat_min_proj_ch[k]) / qMax(1L, imageptrs->stat_max_proj_ch[k] - imageptrs->stat_min_proj_ch[k]);
                        pixcalc = ( pixcalc < 0 ? 0 : pixcalc);
                        pixcalc = ( pixcalc > 1023 ? 1023 : pixcalc );
                        stats_ch[k][pixcalc]++;
                    }
                }
            }
        }

        ++segsel;
    }

    float scale = 1024.0 / (float)imageptrs->active_pixels_proj;

    unsigned long long sum_ch[3];

    for (int i=0; i < 3; i++)
    {
        sum_ch[i] = 0;
    }


    for( int i = 0; i < 1024; i++)
    {
        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            sum_ch[k] += stats_ch[k][i];
            imageptrs->lut_proj_ch[k][i] = (quint16)(sum_ch[k] * scale);
            imageptrs->lut_proj_ch[k][i] = ( imageptrs->lut_proj_ch[k][i] > 1023 ? 1023 : imageptrs->lut_proj_ch[k][i]);
        }
    }
}

void SegmentListVII::ComposeGVProjection(int inputchannel, int histogrammethod, bool normalized)
{
    this->histogrammethod = histogrammethod;
    this->normalized = normalized;

    qDebug() << "SegmentListVII::ComposeGVProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentGVProjection(inputchannel, histogrammethod, normalized);
        emit segmentprojectionfinished(false);
        ++segit;
    }

    //the following code calculates a new LUT that only takes
    //the pixels in the projection into account and not the complete segment(s).
    if(histogrammethod == CMB_HISTO_EQUALIZE_PROJ)
    {
        CalculateProjectionLUT();
        segit = segsselected.begin();
        while ( segit != segsselected.end() )
        {
            ((SegmentVII *)(*segit))->RecalculateProjection(normalized);
            emit segmentprojectionfinished(false);
            ++segit;
        }
    }

}

void SegmentListVII::ComposeLCCProjection(int inputchannel, int histogrammethod, bool normalized)
{
    this->histogrammethod = histogrammethod;
    this->normalized = normalized;

    qDebug() << "SegmentListVII::ComposeLCCProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentLCCProjection(inputchannel, histogrammethod, normalized);
        emit segmentprojectionfinished(false);
        ++segit;
    }

    if(histogrammethod == CMB_HISTO_EQUALIZE_PROJ)
    {
        CalculateProjectionLUT();
        segit = segsselected.begin();
        while ( segit != segsselected.end() )
        {
            ((SegmentVII *)(*segit))->RecalculateProjection(normalized);
            emit segmentprojectionfinished(false);
            ++segit;
        }
    }

}

void SegmentListVII::ComposeSGProjection(int inputchannel, int histogrammethod, bool normalized)
{
    this->histogrammethod = histogrammethod;
    this->normalized = normalized;

    qDebug() << "SegmentListVII::ComposeSGProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentSGProjection(inputchannel, histogrammethod, normalized);
        emit segmentprojectionfinished(false);
        ++segit;
    }

    if(histogrammethod == CMB_HISTO_EQUALIZE_PROJ)
    {
        CalculateProjectionLUT();
        segit = segsselected.begin();
        while ( segit != segsselected.end() )
        {
            ((SegmentVII *)(*segit))->RecalculateProjection(normalized);
            emit segmentprojectionfinished(false);
            ++segit;
        }
    }

}

void SegmentListVII::ComposeOMProjection(int inputchannel, int histogrammethod, bool normalized)
{
    this->histogrammethod = histogrammethod;
    this->normalized = normalized;

    qDebug() << "SegmentListVII::ComposeOMProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentOMProjection(inputchannel, histogrammethod, normalized);
        emit segmentprojectionfinished(false);
        ++segit;
    }

    if(histogrammethod == CMB_HISTO_EQUALIZE_PROJ)
    {
        CalculateProjectionLUT();
        segit = segsselected.begin();
        while ( segit != segsselected.end() )
        {
            ((SegmentVII *)(*segit))->RecalculateProjection(normalized);
            emit segmentprojectionfinished(false);
            ++segit;
        }
    }

}

// The central line of the oblique mercator runs from the centre of the first
// selected segment to the centre of the last, so it follows the ground track.
void SegmentListVII::GetCentralCoords(double *startcentrallon, double *startcentrallat, double *endcentrallon, double *endcentrallat)
{
    double slon, slat, elon, elat;
    double save_slon = 65535.0, save_slat = 65535.0;
    double save_elon = 65535.0, save_elat = 65535.0;

    bool first = true;

    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        segm->getCentralCoords(&slon, &slat, &elon, &elat);

        if(fabs(slon) <= 180.0 && fabs(slat) <= 90.0 && fabs(elon) <= 180.0 && fabs(elat) <= 90.0)
        {
            if(first)
            {
                first = false;
                save_slon = slon;
                save_slat = slat;
            }
            save_elon = elon;
            save_elat = elat;
        }

        ++segsel;
    }

    *startcentrallon = save_slon;
    *startcentrallat = save_slat;
    *endcentrallon = save_elon;
    *endcentrallat = save_elat;
}

void SegmentListVII::SmoothVIIImage(bool combine)
{

    qDebug() << "start SegmentListVII::SmoothVIIImage()";
    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentVII *segmsave = NULL;

    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments(segmsave, segm, combine);
        segmsave = segm;
        BilinearInterpolation(segm, combine);
        ++segsel;
    }
}

void SegmentListVII::SmoothVIIImage12bits()
{

    qDebug() << "start SegmentListVII::SmoothVIIImage12bits()";

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentVII *segmsave = NULL;

    while ( segsel != segsselected.end() )
    {
        SegmentVII *segm = (SegmentVII *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments12bits(segmsave, segm);
        segmsave = segm;
        BilinearInterpolation12bits(segm);
        ++segsel;
    }
}
