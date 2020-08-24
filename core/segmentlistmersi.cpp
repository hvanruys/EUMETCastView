#include "segmentlistmersi.h"
#include "options.h"
#include "segmentimage.h"
#include "globals.h"

extern Options opts;
extern SegmentImage *imageptrs;
extern bool ptrimagebusy;

void doComposeMERSIImageInThread(SegmentListMERSI *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles)
{
    t->ComposeMERSIImageInThread(bandlist, colorlist, invertlist, decompressfiles);
}

SegmentListMERSI::SegmentListMERSI(SatelliteList *satl, QObject *parent) :
    SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListMERSI");

    satlist = satl;
    seglisttype = eSegmentType::SEG_MERSI;
    histogrammethod = 0; // 0 none , 1 equalize
    normalized = false;
}


bool SegmentListMERSI::ComposeMERSIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles)
{
    qDebug() << QString("SegmentListMERSI::ComposeMERSIImage");

    this->bandlist = bandlist;
    this->colorlist = colorlist;
    this->inverselist = invertlist;

    ptrimagebusy = true;
    QApplication::setOverrideCursor(( Qt::WaitCursor));
    watchermersi = new QFutureWatcher<void>(this);
    connect(watchermersi, SIGNAL(finished()), this, SLOT(finishedmersi()));

    QFuture<void> future;
    future = QtConcurrent::run(doComposeMERSIImageInThread, this, bandlist, colorlist, invertlist, decompressfiles);
    watchermersi->setFuture(future);

    return true;

}

bool SegmentListMERSI::ComposeMERSIImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles)
{

    // bandlist 16 items ; bandlist[0] = composecolor
    // colorlist 15 items R = 1 G = 2 B = 3 ; rest is 0
    // invertlist 15 items
    qDebug() << "bool SegmentListMERSI::ComposeMERSIImageInThread() started";

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    this->totalnbroflines = 0;

    emit progressCounter(10);

    this->bandlist = bandlist;
    this->colorlist = colorlist;
    this->invertlist = invertlist;

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
        SegmentMERSI *segm = (SegmentMERSI *)(*segit);
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
            SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
            segm->DecompressSegmentToTemp();
            totalprogress += deltaprogress;
            emit progressCounter(totalprogress);
            ++segsel;
        }
    }

    bool composecolor = bandlist.at(0);

    getIndexFromColor(1);
    getIndexFromColor(2);
    getIndexFromColor(3);

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
        segm->setBandandColor(bandlist, colorlist, invertlist);
        segm->initializeMemory();
        segm->ReadSegmentInMemory(composecolor, colorarrayindex);

        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        if(segsel == segsselected.begin())
            this->earth_views_per_scanline = segm->getEarthViewsPerScanline();
        ++segsel;
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
        segm->setStartLineNbr(startlinenbr);
        startlinenbr += segm->NbrOfLines;
        totalnbroflines += segm->NbrOfLines;
        ++segsel;
    }

    // image pointers always = new QImage()
    if(imageptrs->ptrimageMERSI != NULL)
    {
        delete imageptrs->ptrimageMERSI;
        imageptrs->ptrimageMERSI = NULL;
    }

    imageptrs->ptrimageMERSI = new QImage(this->earth_views_per_scanline, totalnbroflines, QImage::Format_ARGB32);
    qDebug() << QString("ptrimageMERSI created %1 x %2").arg(this->earth_views_per_scanline).arg(totalnbroflines);



    long cnt_active_pixels = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);

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
        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
        segm->ComposeSegmentImage(colorarrayindex, invertarrayindex, this->histogrammethod, this->normalized, this->totalnbroflines);
        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        QApplication::processEvents();
        ++segsel;
    }

    qDebug() << " SegmentListMERSI::ComposeMERSIImageInThread Finished !!";

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
    emit progressCounter(100);
    return true;
}

void SegmentListMERSI::finishedmersi()
{

    qDebug() << "=============>SegmentListMERSI::finishedmersi()";
    emit progressCounter(100);
    opts.texture_changed = true;
    ptrimagebusy = false;
    delete watchermersi;
    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
}

void SegmentListMERSI::progressreadvalue(int progress)
{
    int totalcount = segsselected.count();
    this->progressresultready += 100 / totalcount;

    emit progressCounter(this->progressresultready);

    qDebug() << QString("SegmentListMERSI::progressreadvalue( %1 )").arg(progress);
}

//void SegmentListMERSI::ShowImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
//{

//    progressresultready = 0;
//    QApplication::setOverrideCursor( Qt::WaitCursor );

//    emit progressCounter(10);

//    for (int i=0; i < 3; i++)
//    {
//        for (int j=0; j < 1024; j++)
//        {
//            imageptrs->lut_ch[i][j] = 0;
//        }
//    }

//    for(int k = 0; k < 3; k++)
//    {
//        imageptrs->stat_max_ch[k] = 0;
//        imageptrs->stat_min_ch[k] = 9999999;
//        this->stat_max_ch[k] = 0;
//        this->stat_min_ch[k] = 9999999;
//    }
////    this->minBrightnessTemp = 9999999.9;
////    this->maxBrightnessTemp = 0.0;


//    QList<Segment*>::iterator segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
//        segm->setBandandColor(bandlist, colorlist, invertlist);
//        segm->initializeMemory();
//        ++segsel;
//    }

//    int totalnbrofsegments = this->NbrOfSegmentsSelected();

//    int deltaprogress = 99 / (totalnbrofsegments*2);
//    int totalprogress = 0;


//    segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
//        segm->ReadDatasetsInMemory();
//        totalprogress += deltaprogress;
//        emit progressCounter(totalprogress);
//        ++segsel;
//    }

//    bool composecolor;

//    long cnt_active_pixels = 0;

//    segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
//        composecolor = segm->composeColorImage();

////        if(segm->maxBrightnessTemp > this->maxBrightnessTemp)
////            this->maxBrightnessTemp = segm->maxBrightnessTemp;
////        if(segm->minBrightnessTemp < this->minBrightnessTemp)
////            this->minBrightnessTemp = segm->minBrightnessTemp;

//        for(int i = 0; i < (composecolor ? 3 : 1); i++)
//        {
//            if( segm->stat_max_ch[i] > this->stat_max_ch[i])
//                this->stat_max_ch[i] = segm->stat_max_ch[i];
//            if( segm->stat_min_ch[i] < this->stat_min_ch[i])
//                this->stat_min_ch[i] = segm->stat_min_ch[i];
//        }
//        cnt_active_pixels += segm->active_pixels[0];
//        ++segsel;
//    }


//    for(int i = 0; i < (composecolor ? 3 : 1); i++)
//    {
//        imageptrs->stat_max_ch[i] = this->stat_max_ch[i];
//        imageptrs->stat_min_ch[i] = this->stat_min_ch[i];
//    }

//    imageptrs->active_pixels = cnt_active_pixels;
////    imageptrs->minBrightnessTemp = this->minBrightnessTemp;
////    imageptrs->maxBrightnessTemp = this->maxBrightnessTemp;

////    CalculateLUT();

//    segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
//        segm->ComposeSegmentImage(0, false);
//        totalprogress += deltaprogress;
//        emit progressCounter(totalprogress);
//        QApplication::processEvents();
//        ++segsel;
//    }
//    qDebug() << " SegmentListMERSI::ShowImageSerial Finished !!";

//    QApplication::restoreOverrideCursor();

//    emit segmentlistfinished(true);
//    emit progressCounter(100);
//}

int SegmentListMERSI::getIndexFromColor(int colorindex)
{
    qDebug() << "getIndexFromColor colorindex = " << colorindex;

    Q_ASSERT(colorindex > 0 && colorindex < 4); // R = 1, G = 2, B = 3

    for(int i = 0; i < 15; i++)
    {
        if(colorlist.at(i) == colorindex)
        {
            invertarrayindex[colorindex-1] = invertlist.at(i);
            colorarrayindex[colorindex - 1] = i;
            return i;
        }
    }

}

void SegmentListMERSI::SmoothMERSIImage(bool combine)
{

    qDebug() << "start SegmentListMERSI::SmoothMERSIImage()";

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentMERSI *segmsave;

    while ( segsel != segsselected.end() )
    {
        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments(segmsave, segm, combine);
        segmsave = segm;
        BilinearInterpolation(segm, combine);
        ++segsel;
    }
}

void SegmentListMERSI::ComposeGVProjection(int inputchannel)
{

    qDebug() << "SegmentListMERSI::ComposeGVProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentGVProjection(inputchannel, 0, false);
        emit segmentprojectionfinished(false);
        ++segit;
    }
}


void SegmentListMERSI::ComposeLCCProjection(int inputchannel)
{

    qDebug() << "SegmentListMERSI::ComposeLCCProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentLCCProjection(inputchannel, 0, false);
        emit segmentprojectionfinished(false);
        ++segit;
    }
}

void SegmentListMERSI::ComposeSGProjection(int inputchannel)
{

    qDebug() << "SegmentListMERSI::ComposeSGProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentSGProjection(inputchannel, 0, false);
        emit segmentprojectionfinished(false);
        ++segit;
    }
}


