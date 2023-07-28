#include <QtConcurrent>
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

    seglisttype = eSegmentType::SEG_MERSI;
    histogrammethod = 0; // 0 none , 1 equalize
    normalized = false;
}


bool SegmentListMERSI::ComposeMERSIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles, int histogrammethod, bool normalized)
{
    qDebug() << QString("SegmentListMERSI::ComposeMERSIImage");

    this->bandlist = bandlist;
    this->colorlist = colorlist;
    this->invertlist = invertlist;
    this->histogrammethod = histogrammethod;
    this->normalized = normalized;

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

    if(bandlist.at(0) == false)
    {
        for(int i = 1; i < 16; i++)
        {
            if(bandlist.at(i) == true)
                bandindex = i;
        }
    }
    else
    {
        bandindex = 0;
    }

    if(this->histogrammethod == CMB_HISTO_NONE_95)
    {
        qDebug() << "ComposeMERSIImageInThread : CMB_HISTO_NONE_95";
    }
    else if(this->histogrammethod == CMB_HISTO_NONE_100)
    {
        qDebug() << "ComposeMERSIImageInThread : CMB_HISTO_NONE_100";
    }
    else if(this->histogrammethod == CMB_HISTO_EQUALIZE)
    {
        qDebug() << "ComposeMERSIImageInThread : CMB_HISTO_EQUALIZE";
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
        segm->setBandandColor(bandlist, colorlist, invertlist);
        segm->setHistogrammethod(this->histogrammethod);
        segm->initializeMemory();
        segm->ReadSegmentInMemory(bandindex, colorarrayindex);

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
    CalculateLUTFull();

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
        segm->ComposeSegmentImage(bandindex, colorarrayindex, invertarrayindex, this->histogrammethod, this->normalized, this->totalnbroflines);
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

void SegmentListMERSI::CalculateLUTFull()
{
    qDebug() << "start SegmentListMERSI::CalculateLUTFull()";
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
    int oneblock = 400 * 2048;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
//                    quint16 pixel = *(segm->ptrbaMERSI[k].data() + line * earth_views + pixelx) ;
                    quint16 pixel = *(segm->ptrbaMERSI.data() + this->colorarrayindex[k] * oneblock + line * earth_views + pixelx) ;
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

    int ret = 0;

    for(int i = 0; i < 15; i++)
    {
        if(colorlist.at(i) == colorindex)
        {
            invertarrayindex[colorindex-1] = invertlist.at(i);
            colorarrayindex[colorindex - 1] = i;
            ret = i;
        }
    }

    return ret;

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

void SegmentListMERSI::ComposeOMProjection(int inputchannel)
{

    qDebug() << "SegmentListMERSI::ComposeOMProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentOMProjection(inputchannel, 0, false);
        emit segmentprojectionfinished(false);
        ++segit;
    }
}

void SegmentListMERSI::GetCentralCoords(double *startcentrallon, double *startcentrallat, double *endcentrallon, double *endcentrallat)
{
    double slon, slat, elon, elat;
    double save_slon, save_slat, save_elon, save_elat;
    int startindex, endindex;

    save_slon = 65535.0;
    save_slat = 65535.0;
    save_elon = 65535.0;
    save_elat = 65535.0;

    bool first = true;

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    while ( segsel != segsselected.end() )
    {
        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
        segm->GetCentralCoords(&slon, &slat, &elon, &elat, &startindex, &endindex);

        if(abs(slon) < 180.0 && abs(slat) < 90.0 && abs(elon) < 180.0 && abs(elat) < 90.0)
        {
            if(first == true)
            {
                first = false;
                save_slon = slon;
                save_slat = slat;
                save_elon = elon;
                save_elat = elat;
            }
            else
            {
                save_elon = elon;
                save_elat = elat;
            }

        }

        QApplication::processEvents();
        ++segsel;
    }

    *startcentrallon = save_slon;
    *startcentrallat = save_slat;
    *endcentrallon = save_elon;
    *endcentrallat = save_elat;

}

void SegmentListMERSI::GetCornerCoords(double *cornerlon1, double *cornerlat1, double *cornerlon2, double *cornerlat2, double *cornerlon3, double *cornerlat3, double *cornerlon4, double *cornerlat4)
{
    double save_cornerlon1, save_cornerlat1, save_cornerlon2, save_cornerlat2;
    double save_cornerlon3, save_cornerlat3, save_cornerlon4, save_cornerlat4;
    int Xcornerindex1, Xcornerindex2, Ycornerindex12;
    int Xcornerindex3, Xcornerindex4, Ycornerindex34;

    save_cornerlon1 = 65535.0;
    save_cornerlat1 = 65535.0;
    save_cornerlon2 = 65535.0;
    save_cornerlat2 = 65535.0;

    save_cornerlon3 = 65535.0;
    save_cornerlat3 = 65535.0;
    save_cornerlon4 = 65535.0;
    save_cornerlat4 = 65535.0;

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    int count = 0;

    while ( segsel != segsselected.end() )
    {
        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
        count++;
        if(count == 1)
        {
            segm->GetStartCornerCoords(&save_cornerlon1, &save_cornerlat1, &save_cornerlon2, &save_cornerlat2, &Xcornerindex1, &Xcornerindex2, &Ycornerindex12 );
            if(abs(save_cornerlon1) < 180.0 && abs(save_cornerlat1) < 90.0 && abs(save_cornerlon2) < 180.0 && abs(save_cornerlat2) < 90.0)
                break;
        }
        ++segsel;
    }

    segsel = segsselected.begin();

    while ( segsel != segsselected.end() )
    {
        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
        count++;

        if(count == segsselected.size())
        {
            segm->GetEndCornerCoords(&save_cornerlon3, &save_cornerlat3, &save_cornerlon4, &save_cornerlat4, &Xcornerindex3, &Xcornerindex4, &Ycornerindex34 );
            if(abs(save_cornerlon3) < 180.0 && abs(save_cornerlat3) < 90.0 && abs(save_cornerlon4) < 180.0 && abs(save_cornerlat4) < 90.0)
                break;
        }
        ++segsel;
    }

    *cornerlon1 = save_cornerlon1;
    *cornerlat1 = save_cornerlat1;
    *cornerlon2 = save_cornerlon2;
    *cornerlat2 = save_cornerlat2;

    *cornerlon3 = save_cornerlon3;
    *cornerlat3 = save_cornerlat3;
    *cornerlon4 = save_cornerlon4;
    *cornerlat4 = save_cornerlat4;

}

//void SegmentListMERSI::GetContourPolygon(QPolygonF *poly)
//{
//    QList<Segment *>::iterator segsel;
//    segsel = segsselected.begin();
//    int segscount = segsselected.size();
//    int count = 0;
//    int nbroflines;

//    while ( segsel != segsselected.end() )
//    {
//        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
//        nbroflines = segm->GetNbrOfLines();
//        count++;
//        if(count == 1)
//        {
//            for(int j = 0; j < nbroflines; j=j+10)
//            {
//                if(segm->geolongitude[j*segm->getEarthViewsPerScanline()] < 180.0 && segm->geolatitude[j*segm->getEarthViewsPerScanline()] < 90.0)
//                {
//                    for(int i = 0; i < segm->getEarthViewsPerScanline(); i++ )
//                        poly->append(QPointF(segm->geolongitude[i + j*segm->getEarthViewsPerScanline()], segm->geolatitude[i + j*segm->getEarthViewsPerScanline()] ));
//                    break;
//                }
//            }
//        }
//        if(count == segscount)
//        {
//            for(int j = nbroflines - 1; j >= 0; j=j-10)
//            {
//                if(segm->geolongitude[j*segm->getEarthViewsPerScanline()] < 180.0 && segm->geolatitude[j*segm->getEarthViewsPerScanline()] < 90.0)
//                {
//                    for(int i = 0; i < segm->getEarthViewsPerScanline(); i++ )
//                        poly->append(QPointF(segm->geolongitude[i + j*segm->getEarthViewsPerScanline()], segm->geolatitude[i + j*segm->getEarthViewsPerScanline()] ));
//                    break;
//                }
//            }
//        }

//        QApplication::processEvents();
//        ++segsel;
//    }
//}

//void SegmentListMERSI::GetTrackPolygon(QPolygonF *poly)
//{
//    QList<Segment *>::iterator segsel;
//    int nbroflines;

//    segsel = segsselected.begin();

//    while ( segsel != segsselected.end() )
//    {
//        SegmentMERSI *segm = (SegmentMERSI *)(*segsel);
//        nbroflines = segm->GetNbrOfLines();
//        for(int j = 0; j < nbroflines; j=j+10)
//        {
//            if(segm->geolongitude[j*segm->getEarthViewsPerScanline()] < 180.0 && segm->geolatitude[j*segm->getEarthViewsPerScanline()] < 90.0)
//            {
//                    poly->append(QPointF(segm->geolongitude[j*segm->getEarthViewsPerScanline() + (int)(segm->getEarthViewsPerScanline()/2)], segm->geolatitude[j*segm->getEarthViewsPerScanline() + (int)(segm->getEarthViewsPerScanline()/2)] ));
//            }
//        }

//    QApplication::processEvents();
//    ++segsel;
//    }
//}
