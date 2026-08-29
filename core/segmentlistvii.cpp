#include "segmentlistvii.h"
#include "segmentimage.h"
#include "options.h"
#include <QtConcurrent>

extern Options opts;
extern SegmentImage *imageptrs;
extern bool ptrimagebusy;
extern SatelliteList satellitelist;

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
}

bool SegmentListVII::ComposeVIIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{
    qDebug() << QString("SegmentListVII::ComposeVIIImage");
    qDebug() << QString("bandlist has nbr of items : %1").arg(bandlist.count());

    this->bandlist = bandlist;
    this->colorlist = colorlist;
    this->invertlist = invertlist;

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


    //CalculateLUTAlt();
    //CalculateLUTFull();

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
    emit progressCounter(100);
    return true;
}

void SegmentListVII::finishedvii()
{

    qDebug() << "=============>SegmentListVII::finishedolci()";
    emit progressCounter(100);
    opts.texture_changed = true;
    ptrimagebusy = false;
    delete watchervii;
    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
}

