#include "segmentlistolciefr.h"
#include "segmentolciefr.h"
#include "segmentimage.h"
#include "options.h"
#include <QtConcurrent>

extern Options opts;
extern SegmentImage *imageptrs;


void doComposeOLCIefrImageInThread(SegmentListOLCIefr *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool readnew)
{
    t->ComposeOLCIImageInThread(bandlist, colorlist, invertlist, readnew);
}


SegmentListOLCIefr::SegmentListOLCIefr(SatelliteList *satl, QObject *parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListOLCIefr");

    satlist = satl;
    seglisttype = eSegmentType::SEG_OLCIEFR;


}

bool SegmentListOLCIefr::ComposeOLCIefrImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool untarfiles)
{
    qDebug() << QString("SegmentListOLCIefr::ComposeOLCIefrImage");

    this->bandlist = bandlist;
    this->colorlist = colorlist;
    this->inverselist = invertlist;

    QApplication::setOverrideCursor(( Qt::WaitCursor));
    watcherolci = new QFutureWatcher<void>(this);
    connect(watcherolci, SIGNAL(finished()), this, SLOT(finishedolci()));

    QFuture<void> future;
    future = QtConcurrent::run(doComposeOLCIefrImageInThread, this, bandlist, colorlist, invertlist, untarfiles);
    watcherolci->setFuture(future);

    return true;

}

bool SegmentListOLCIefr::ComposeOLCIImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool untarfiles)
{

    qDebug() << "bool SegmentListOLCIefr::ComposeOLCIImageInThread() started";

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    emit progressCounter(10);

    for (int i=0; i < 3; i++)
    {
        for (int j=0; j < 1024; j++)
        {
            imageptrs->lut_ch[i][j] = 0;
        }
    }

    for(int k = 0; k < 3; k++)
    {
        imageptrs->stat_max_ch[k] = 0;
        imageptrs->stat_min_ch[k] = 9999999;
        this->stat_max_ch[k] = 0;
        this->stat_min_ch[k] = 9999999;
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
    int totalnbroflines = 0;
    int totalnbrofsegments = 0;

    QList<Segment*>::iterator segit = segmentlist.begin();

    // create QList of selected segments
    while ( segit != segmentlist.end() )
    {
        SegmentOLCIefr *segm = (SegmentOLCIefr *)(*segit);
        if (segm->segmentselected)
        {
             segsselected.append(segm);
             totalnbrofsegments++;
        }
        ++segit;
    }

    int deltaprogress = 99 / (totalnbrofsegments*3);
    int totalprogress = 0;

    if(untarfiles)
    {
        segsel = segsselected.begin();
        while ( segsel != segsselected.end() )
        {
            SegmentOLCIefr *segm = (SegmentOLCIefr *)(*segsel);
            segm->UntarSegmentToTemp();
            totalprogress += deltaprogress;
            emit progressCounter(totalprogress);
            ++segsel;
        }
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCIefr *segm = (SegmentOLCIefr *)(*segsel);
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
        SegmentOLCIefr *segm = (SegmentOLCIefr *)(*segsel);
        segm->setStartLineNbr(startlinenbr);
        startlinenbr += segm->NbrOfLines;
        totalnbroflines += segm->NbrOfLines;
        ++segsel;
    }

    // image pointers always = new QImage()
    if(imageptrs->ptrimageOLCIefr != NULL)
    {
        delete imageptrs->ptrimageOLCIefr;
        imageptrs->ptrimageOLCIefr = NULL;
    }

    imageptrs->ptrimageOLCIefr = new QImage(this->earth_views_per_scanline, totalnbroflines, QImage::Format_ARGB32);
    qDebug() << QString("ptrimageOLCIefr created %1 x %2").arg(this->earth_views_per_scanline).arg(totalnbroflines);


    bool composecolor;

    long cnt_active_pixels = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCIefr *segm = (SegmentOLCIefr *)(*segsel);
        composecolor = segm->composeColorImage();


        for(int i = 0; i < (composecolor ? 3 : 1); i++)
        {
            if( segm->stat_max_ch[i] > this->stat_max_ch[i])
                this->stat_max_ch[i] = segm->stat_max_ch[i];
            if( segm->stat_min_ch[i] < this->stat_min_ch[i])
                this->stat_min_ch[i] = segm->stat_min_ch[i];
        }
        cnt_active_pixels += segm->active_pixels[0];
        ++segsel;
    }


    for(int i = 0; i < (composecolor ? 3 : 1); i++)
    {
        imageptrs->stat_max_ch[i] = this->stat_max_ch[i];
        imageptrs->stat_min_ch[i] = this->stat_min_ch[i];
    }

    imageptrs->active_pixels = cnt_active_pixels;

    CalculateLUT();

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCIefr *segm = (SegmentOLCIefr *)(*segsel);
        segm->ComposeSegmentImage();
        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        QApplication::processEvents();
        ++segsel;
    }

    qDebug() << " SegmentListOLCIefr::ComposeOLCIImageInThread Finished !!";


    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
    emit progressCounter(100);
    return true;
}

void SegmentListOLCIefr::finishedolci()
{

    qDebug() << "=============>SegmentListOLCIefr::finishedolci()";
    emit progressCounter(100);
    opts.texture_changed = true;
    delete watcherolci;
    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
}

void SegmentListOLCIefr::progressreadvalue(int progress)
{
    int totalcount = segsselected.count();
    this->progressresultready += 100 / totalcount;

    emit progressCounter(this->progressresultready);

    qDebug() << QString("SegmentListOLCIefr::progressreadvalue( %1 )").arg(progress);
}

void SegmentListOLCIefr::CalculateLUT()
{
    qDebug() << "start SegmentListOLCIefr::CalculateLUT()";
    int earth_views = this->earth_views_per_scanline;
    long stats_ch[3][256];

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 256; j++)
        {
            stats_ch[k][j] = 0;
        }
    }

    bool composecolor;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCIefr *segm = (SegmentOLCIefr *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
                    int pixel = *(segm->ptrbaOLCI[k].data() + line * earth_views + pixelx);
                    int pixcalc = 256 * (pixel - imageptrs->stat_min_ch[k]) / (imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k]);
                    pixcalc = ( pixcalc < 0 ? 0 : pixcalc);
                    pixcalc = ( pixcalc > 255 ? 255 : pixcalc );
                    stats_ch[k][pixcalc]++;
                }
            }
        }


        ++segsel;
    }

    // float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
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

void SegmentListOLCIefr::ComposeGVProjection(int inputchannel)
{

    qDebug() << "SegmentListOLCIefr::ComposeGVProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentGVProjection(inputchannel);
        emit segmentprojectionfinished(false);
        ++segit;
    }
}

void SegmentListOLCIefr::SmoothOLCIImage(bool combine)
{

    qDebug() << "start SegmentListOLCIefr::SmoothOLCIImage()";

    int lineimage = 0;

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentOLCIefr *segmsave;

    while ( segsel != segsselected.end() )
    {
        SegmentOLCIefr *segm = (SegmentOLCIefr *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments(segmsave, segm, combine);
        segmsave = segm;
        BilinearInterpolation(segm, combine);
        ++segsel;
        lineimage += segm->NbrOfLines;
    }
}

