#include "segmentlistslstr.h"
#include "segmentimage.h"
#include "options.h"
#include <QtConcurrent>

extern Options opts;
extern SegmentImage *imageptrs;
extern bool ptrimagebusy;

eSLSTRImageView slstrview;

void doComposeSLSTRImageInThread(SegmentListSLSTR *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles)
{
    t->ComposeSLSTRImageInThread(bandlist, colorlist, invertlist, decompressfiles);
}

SegmentListSLSTR::SegmentListSLSTR(SatelliteList *satl, QObject *parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListSLSTR");

    satlist = satl;
    seglisttype = eSegmentType::SEG_SLSTR;
    histogrammethod = 0; // 0 none , 1 equalize
}

bool SegmentListSLSTR::ComposeSLSTRImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles, eSLSTRImageView view)
{
    qDebug() << QString("SegmentListSLSTR::ComposeSLSTRImage");

    this->bandlist = bandlist;
    this->colorlist = colorlist;
    this->inverselist = invertlist;

    slstrview = view;


    ptrimagebusy = true;
    QApplication::setOverrideCursor(( Qt::WaitCursor));
    watcherslstr = new QFutureWatcher<void>(this);
    connect(watcherslstr, SIGNAL(finished()), this, SLOT(finishedslstr()));

    QFuture<void> future;
    future = QtConcurrent::run(doComposeSLSTRImageInThread, this, bandlist, colorlist, invertlist, decompressfiles);
    watcherslstr->setFuture(future);

    return true;

}

bool SegmentListSLSTR::ComposeSLSTRImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles)
{
    qDebug() << "bool SegmentListSLSTR::ComposeSLSTRImageInThread() started";

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

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
    imageptrs->active_pixels = 0;

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
        SegmentOLCI *segm = (SegmentOLCI *)(*segit);
        if (segm->segmentselected)
        {
             segsselected.append(segm);
             totalnbrofsegments++;
        }
        ++segit;
    }

    int deltaprogress = 99 / (totalnbrofsegments*3);
    int totalprogress = 0;

    if(decompressfiles)
    {
        segsel = segsselected.begin();
        while ( segsel != segsselected.end() )
        {
            SegmentSLSTR *segm = (SegmentSLSTR *)(*segsel);
            segm->DecompressSegmentToTemp();
            totalprogress += deltaprogress;
            emit progressCounter(totalprogress);
            ++segsel;
        }
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentSLSTR *segm = (SegmentSLSTR *)(*segsel);
        segm->setBandandColorandView(bandlist, colorlist, invertlist, slstrview);
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
        SegmentSLSTR *segm = (SegmentSLSTR *)(*segsel);
        segm->setStartLineNbr(startlinenbr);
        startlinenbr += segm->NbrOfLines;
        totalnbroflines += segm->NbrOfLines;
        ++segsel;
    }

    // image pointers always = new QImage()
    if(imageptrs->ptrimageSLSTR != NULL)
    {
        delete imageptrs->ptrimageSLSTR;
        imageptrs->ptrimageSLSTR = NULL;
    }

    imageptrs->ptrimageSLSTR = new QImage(this->earth_views_per_scanline, totalnbroflines, QImage::Format_ARGB32);
    if(bandlist.at(0) == true)
        imageptrs->ptrimageSLSTR->fill(qRgba(0, 0, 0, 0));
    else
        imageptrs->ptrimageSLSTR->fill(Qt::blue);
    qDebug() << QString("ptrimageSLSTR created %1 x %2").arg(this->earth_views_per_scanline).arg(totalnbroflines);


    bool composecolor;

    long cnt_active_pixels = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentSLSTR *segm = (SegmentSLSTR *)(*segsel);

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

    qDebug() << QString("imageptrs->active_pixels = %1").arg(imageptrs->active_pixels);

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

    CalculateLUTFull();

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentSLSTR *segm = (SegmentSLSTR *)(*segsel);
        segm->ComposeSegmentImage(this->histogrammethod);
        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        QApplication::processEvents();
        ++segsel;
    }

    qDebug() << " SegmentListSLSTR::ComposeSLSTRImageInThread Finished !!";

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
    emit progressCounter(100);
    return true;
}

void SegmentListSLSTR::CalculateLUTFull()
{
    qDebug() << "start SegmentListSLSTR::CalculateLUTFull()";
    int earth_views = this->earth_views_per_scanline;
    long stats_ch[3][1024];

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 1024; j++)
        {
            stats_ch[k][j] = 0;
        }
    }

    bool composecolor;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentSLSTR *segm = (SegmentSLSTR *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
                    quint16 pixel = *(segm->ptrbaSLSTR[k].data() + line * earth_views + pixelx) ;
                    quint16 indexout = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - imageptrs->stat_min_ch[k])/(float)(imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k])), 0), 1023);
                    stats_ch[k][indexout]++;
                 }
            }
        }
        ++segsel;
    }


    // float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
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

    for(int k = 0; k < (composecolor ? 3 : 1); k++)
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

    for(int k = 0; k < (composecolor ? 3 : 1); k++)
    {
        okmin[k] = false;
        okmax[k] = false;
    }



    for(int k = 0; k < (composecolor ? 3 : 1); k++)
    {
        qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4").arg(k).arg(imageptrs->minRadianceIndex[k]).arg(k).arg(imageptrs->maxRadianceIndex[k]);
    }
    qDebug() << QString("imageptrs stat_min_ch[0] = %1 stat_max_ch[0] = %2").arg(imageptrs->stat_min_ch[0]).arg(imageptrs->stat_max_ch[0]);
    if(composecolor)
    {
        qDebug() << QString("imageptrs stat_min_ch[1] = %1 stat_max_ch[1] = %2").arg(imageptrs->stat_min_ch[1]).arg(imageptrs->stat_max_ch[1]);
        qDebug() << QString("imageptrs stat_min_ch[2] = %1 stat_max_ch[2] = %2").arg(imageptrs->stat_min_ch[2]).arg(imageptrs->stat_max_ch[2]);
    }


}

void SegmentListSLSTR::finishedslstr()
{

    qDebug() << "=============>SegmentListSLSTR::finishedslstr()";
    emit progressCounter(100);
    opts.texture_changed = true;
    ptrimagebusy = false;
    delete watcherslstr;
    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
}

void SegmentListSLSTR::progressreadvalue(int progress)
{
    int totalcount = segsselected.count();
    this->progressresultready += 100 / totalcount;

    emit progressCounter(this->progressresultready);

    qDebug() << QString("SegmentListSLSTR::progressreadvalue( %1 )").arg(progress);
}

bool SegmentListSLSTR::ChangeHistogramMethod()
{

    qDebug() << "bool SegmentListSLSTR::ChangeHistogramMethod() started";

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    emit progressCounter(10);

    // image pointers always = new QImage()
    if(imageptrs->ptrimageSLSTR != NULL)
    {
        delete imageptrs->ptrimageSLSTR;
        imageptrs->ptrimageSLSTR = NULL;
    }

    imageptrs->ptrimageSLSTR = new QImage(this->earth_views_per_scanline, this->totalnbroflines, QImage::Format_ARGB32);
    qDebug() << QString("ptrimageSLSTR created %1 x %2").arg(this->earth_views_per_scanline).arg(totalnbroflines);

    if (this->histogrammethod == CMB_HISTO_NONE_95 || this->histogrammethod == CMB_HISTO_NONE_100 || this->histogrammethod == CMB_HISTO_EQUALIZE)
        ComposeSegments();

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
    emit progressCounter(100);
    return true;
}

void SegmentListSLSTR::ComposeSegments()
{
    QList<Segment*>::iterator segsel = segsselected.begin();

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentSLSTR *segm = (SegmentSLSTR *)(*segsel);
        segm->ComposeSegmentImage(this->histogrammethod);
        QApplication::processEvents();
        ++segsel;
    }

}
