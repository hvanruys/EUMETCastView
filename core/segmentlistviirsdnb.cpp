#include <QtConcurrent>
#include "segmentlistviirsdnb.h"
#include "segmentimage.h"
#include "options.h"

#include <QDir>
#include <QDebug>
#include <QPainter>
#include <math.h>

#include <iostream>
#include <iomanip>

extern Options opts;
extern SegmentImage *imageptrs;

void doComposeVIIRSDNBImageInThread(SegmentListVIIRSDNB *t)
{
    t->ComposeVIIRSImageInThread();
}

//void SegmentListVIIRS::doReadSegmentInMemoryVIIRS(Segment *t)
//{
//    t->ReadSegmentInMemory();
//}

//void SegmentListVIIRS::doComposeSegmentImageVIIRS(Segment *t)
//{
//    t->ComposeSegmentImage();
//}

//void SegmentListVIIRS::doComposeProjection(Segment *t)
//{
//    t->ComposeProjectionConcurrent();
//}

SegmentListVIIRSDNB::SegmentListVIIRSDNB(SatelliteList *satl, QObject *parent, eSegmentType type)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListVIIRSDNB");

    satlist = satl;
    segtype = type;

    earthviews = 4064;


}

void SegmentListVIIRSDNB::GetFirstLastVisibleSegmentData( QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast)
{

    QString first_filename;
    QString last_filename;

    //SVMC_npp_d20141117_t0837599_e0839241_b15833_c20141117084501709131_eum_ops
    if( segmentlist.count() > 0)
    {
        //segmentlist.at(indexfirstvisible)->
        first_filename = segmentlist.at(indexfirstvisible)->fileInfo.fileName();
        last_filename = segmentlist.at(indexlastvisible)->fileInfo.fileName();
        if(last_filename.mid(0,10) == "SVDNBC_npp")
        {
            *satnamelast = last_filename.mid(5, 3);
            *segdatelast = QString("%1-%2-%3").arg(last_filename.mid(12,4)).arg(last_filename.mid(16,2)).arg(last_filename.mid(18,2));
            *segtimelast = QString("%1:%2:%3").arg(last_filename.mid(22,2)).arg(last_filename.mid(24,2)).arg(last_filename.mid(26,2));
        }
    }
    else
    {
        *satnamefirst = QString("");
        *segdatefirst = QString("");
        *segtimefirst = QString("");
        *satnamelast = QString("");
        *segdatelast = QString("");
        *segtimelast = QString("");
    }
}

bool SegmentListVIIRSDNB::ComposeVIIRSImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{
    qDebug() << QString("SegmentListVIIRSDNB::ComposeVIIRSImage");

    QApplication::setOverrideCursor(( Qt::WaitCursor));
    watcherviirs = new QFutureWatcher<void>(this);
    connect(watcherviirs, SIGNAL(finished()), this, SLOT(finishedviirs()));

    QFuture<void> future;
    future = QtConcurrent::run(doComposeVIIRSDNBImageInThread, this);
    watcherviirs->setFuture(future);

    return true;

}

#if 0
bool SegmentListVIIRSDNB::ComposeVIIRSImageConcurrent(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor ); // reset in composefinishedviirs

    for (int i=0; i < 3; i++)
    {
        for (int j=0; j < 1024; j++)
        {
            imageptrs->segment_stats_ch[i][j] = 0;
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

    QList<Segment*>::iterator segit = segmentlist.begin();

    // create QList of selected segments
    while ( segit != segmentlist.end() )
    {
        SegmentVIIRS *segm = (SegmentVIIRS *)(*segit);

        if (segm->segmentselected)
        {
             segsselected.append(segm);
             totalnbroflines += 768;
        }
        ++segit;
    }


    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
        segm->setBandandColor(bandlist, colorlist, invertlist);
        segm->setStartLineNbr(startlinenbr);
        segm->initializeMemory();
        startlinenbr += segm->NbrOfLines;
        ++segsel;
    }

    emit progressCounter(10);

    // image pointers always = new QImage()
    if(imageptrs->ptrimageViirs != NULL)
    {
        delete imageptrs->ptrimageViirs;
        imageptrs->ptrimageViirs = NULL;
    }

    imageptrs->ptrimageViirs = new QImage(earthviews, totalnbroflines, QImage::Format_ARGB32);


    watcherreadviirs = new QFutureWatcher<void>(this);
    connect(watcherreadviirs, SIGNAL(finished()), this, SLOT(readfinishedviirs()));
    connect(watcherreadviirs, SIGNAL(progressValueChanged(int)), this, SLOT(progressreadvalue(int)));

    watcherreadviirs->setFuture(QtConcurrent::map( segsselected.begin(), segsselected.end(), &SegmentListVIIRS::doReadSegmentInMemoryVIIRS));

    return true;
}

#endif


bool SegmentListVIIRSDNB::ComposeVIIRSImageInThread()
{

    qDebug() << "bool SegmentListVIIRS::ComposeVIIRSImageInThread() started";

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    emit progressCounter(10);


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
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segit);
        if (segm->segmentselected)
        {
             segsselected.append(segm);
             totalnbroflines += 768;
             totalnbrofsegments++;
        }
        ++segit;
    }


    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);
        segm->setStartLineNbr(startlinenbr);
        segm->initializeMemory();
        startlinenbr += segm->NbrOfLines;
        ++segsel;
    }


    // image pointers always = new QImage()
    if(imageptrs->ptrimageViirs != NULL)
    {
        delete imageptrs->ptrimageViirs;
        imageptrs->ptrimageViirs = NULL;
    }

    imageptrs->ptrimageViirs = new QImage(earthviews, totalnbroflines, QImage::Format_ARGB32);

    int deltaprogress = 99 / (totalnbrofsegments*2);
    int totalprogress = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);
        segm->ReadSegmentInMemory();

        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        ++segsel;
    }

/*
    stat_max_dnb = -1.0E31;
    stat_min_dnb = 1.0E31;


    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);

        if( segm->stat_max > this->stat_max_dnb)
            this->stat_max_dnb = segm->stat_max;
        if( segm->stat_min < this->stat_min_dnb)
            this->stat_min_dnb = segm->stat_min;
        ++segsel;
    }


    imageptrs->stat_max_dnb = this->stat_max_dnb;
    imageptrs->stat_min_dnb = this->stat_min_dnb;

    qDebug() << QString("imageptrs stat_min = %1 stat_max = %2").arg(imageptrs->stat_min_dnb).arg(imageptrs->stat_max_dnb);
*/

    //CalculateLUTDNB();

    float lowerlimit = pow(10, opts.dnbsbvalue/20.0)/pow(10, opts.dnbspbwindowsvalue);
    float upperlimit = pow(10, opts.dnbsbvalue/20.0)*pow(10, opts.dnbspbwindowsvalue);

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);

        segm->ComposeSegmentImageWindow(lowerlimit, upperlimit);

        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        ++segsel;
    }
    qDebug() << " SegmentListVIIRS::ComposeVIIRSDNBImageInThread Finished !!";

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
    emit progressCounter(100);


    return true;

}

void SegmentListVIIRSDNB::CalculateLUT()
{
    qDebug() << "start SegmentListVIIRSDNB::CalculateLUT()";
    int earth_views;
    long stats_ch[256];

    for (int j = 0; j < 256; j++)
    {
        stats_ch[j] = 0;
    }


    for( int exp = -15; exp < -4; exp++)
    {
        float topow = pow(10, exp);
        int teller = 0;
        float dexpo = topow;
        float dexpo1 = 0.5 * topow;
        int index = (15 + exp) * 20;

        qDebug() << QString("index = %1 dexpo = %2 dexpo1 = %3").arg(index).arg(dexpo).arg(dexpo1);

        teller++;

        for(float man = 0.5; man < 10.0; man+=0.5)
        {
            float dexpo = man * topow;
            float dexpo1 = (man + 0.5) * topow;
            int index = (15 + exp) * 20 + teller;

            qDebug() << QString("index = %1 dexpo = %2 dexpo1 = %3").arg(index).arg(dexpo).arg(dexpo1);
            teller++;
        }
    }

    int counter;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);
        earth_views = segm->earth_views_per_scanline;

        for (int line = 0; line < segm->NbrOfLines; line++)
        {
            for (int pixelx = 0; pixelx < segm->earth_views_per_scanline; pixelx++)
            {
                float pixel = *(segm->ptrbaVIIRSDNB.data() + line * segm->earth_views_per_scanline + pixelx);
               {
//                    float delta = stat_max_dnb - stat_min_dnb;
//                    float deltapixel = pixel - stat_min_dnb;
//                    float breuk = deltapixel / delta;
//                    if(line == 100 && pixelx > 2000 && pixelx < 2011)
//                    {
//                         qDebug() << QString("pixel = %1 deltapixel = %2 delta = %3 breuk = %4 stat_max_dnb = %5 stat_min_dnb = %6")
//                                             .arg(pixel).arg(deltapixel).arg(delta).arg(breuk).arg(stat_max_dnb).arg(stat_min_dnb);
//                    }
//                    float pp = (pixel - stat_min_dnb) / (stat_max_dnb - stat_min_dnb);
//                    int pixcalc = (int)(255 * (pixel - stat_min_dnb) / (stat_max_dnb - stat_min_dnb));
//                    pixcalc = ( pixcalc < 0 ? 0 : pixcalc);
//                    pixcalc = ( pixcalc > 255 ? 255 : pixcalc );
//                    stats_ch[pixcalc]++;

                    //float thepow[]



                    for( int exp = -15; exp < -4; exp++)
                    {
                        float topow = pow(10, exp);
                        float dexpo = topow;
                        float dexpo1 = 0.5 * topow;
                        if(pixel >= dexpo && pixel < dexpo1)
                        {
                            int index = (15 + exp) * 20;
                            stats_ch[index]++;
                        }
                        counter = 1;

                        for(float man = 0.5; man < 10.0; man+=0.5)
                        {
                            dexpo = man * topow;
                            dexpo1 = (man + 0.5) * topow;
                            if(pixel >= dexpo && pixel < dexpo1)
                            {
                                int index = (15 + exp) * 20 + counter;
                                stats_ch[index]++;
                            }
                            counter++;
                        }
                    }
                }

            }
        }

        ++segsel;
    }

//    float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
//    float scale = 256.0 / imageptrs->active_pixels;

//    unsigned long long sum_ch = 0;

//    for( int i = 0; i < 256; i++)
//    {
//        sum_ch += stats_ch[i];
//        imageptrs->lut_ch[0][i] = (quint16)(sum_ch * scale);
//        imageptrs->lut_ch[0][i] = ( imageptrs->lut_ch[0][i] > 255 ? 255 : imageptrs->lut_ch[0][i]);
//        qDebug() << i << " " << lut_ch[0][i];
//    }

    for (int j = 0; j < 256; j++)
    {
        qDebug() << j << " " << stats_ch[j];
    }

}


#if 0
bool SegmentListVIIRSDNB::ComposeVIIRSImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{

    qDebug() << "bool SegmentListVIIRS::ComposeVIIRSImageSerial(QList<bool> bandlist, QList<int> colorlist) started";

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    for (int i=0; i < 3; i++)
    {
        for (int j=0; j < 1024; j++)
        {
            imageptrs->segment_stats_ch[i][j] = 0;
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
        SegmentVIIRS *segm = (SegmentVIIRS *)(*segit);
        if (segm->segmentselected)
        {
             segsselected.append(segm);
             totalnbroflines += 768;
             totalnbrofsegments++;
        }
        ++segit;
    }


    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
        segm->setBandandColor(bandlist, colorlist, invertlist);
        segm->setStartLineNbr(startlinenbr);
        segm->initializeMemory();
        startlinenbr += segm->NbrOfLines;
        ++segsel;
    }


    // image pointers always = new QImage()
    if(imageptrs->ptrimageViirs != NULL)
    {
        delete imageptrs->ptrimageViirs;
        imageptrs->ptrimageViirs = NULL;
    }

    imageptrs->ptrimageViirs = new QImage(earthviews, totalnbroflines, QImage::Format_ARGB32);

    int deltaprogress = 99 / (totalnbrofsegments*2);
    int totalprogress = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
        segm->ReadSegmentInMemory();

        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);

        QApplication::processEvents();
        ++segsel;
    }

    bool composecolor;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int i = 0; i < (composecolor ? 3 : 1); i++)
        {
            if( segm->stat_max_ch[i] > this->stat_max_ch[i])
                this->stat_max_ch[i] = segm->stat_max_ch[i];
            if( segm->stat_min_ch[i] < this->stat_min_ch[i])
                this->stat_min_ch[i] = segm->stat_min_ch[i];
        }
        ++segsel;
    }


    for(int i = 0; i < (composecolor ? 3 : 1); i++)
    {
        imageptrs->stat_max_ch[i] = this->stat_max_ch[i];
        imageptrs->stat_min_ch[i] = this->stat_min_ch[i];
    }

    CalculateLUT();

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
        segm->ComposeSegmentImage();

        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);

        QApplication::processEvents();
        ++segsel;
    }
    qDebug() << " SegmentListVIIRS::ComposeVIIRSImageSerial Finished !!";

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished();
    emit progressCounter(100);


    return true;
}


 Problems with reopen the hdf5 files in a concurrent environment .... does not work.
 Serial solution works.
bool SegmentListVIIRS::ShowImage(QList<bool> bandlist, QList<int> colorlist)
{

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor ); // reset in composefinishedviirs

    for (int i=0; i < 3; i++)
    {
        for (int j=0; j < 1024; j++)
        {
            imageptrs->segment_stats_ch[i][j] = 0;
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


    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
        segm->setBandandColor(bandlist, colorlist);
        segm->initializeMemory();
        ++segsel;
    }


    watcherreadviirs = new QFutureWatcher<void>(this);
    connect(watcherreadviirs, SIGNAL(resultReadyAt(int)), SLOT(resultisreadyviirs(int)));
    connect(watcherreadviirs, SIGNAL(finished()), SLOT(readfinishedviirs()));

    watcherreadviirs->setFuture(QtConcurrent::map( segsselected.begin(), segsselected.end(), &SegmentListVIIRS::doReadDatasetsInMemoryVIIRS));

    return true;

}

#endif

float SegmentListVIIRSDNB::getMoonIllumination()
{
    qDebug() << "SegmentListVIIRSDNB::getMoonIllumination()";
    int count = 0;
    float totillum = 0;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);
        totillum += segm->MoonIllumFraction;
        count++;
        ++segsel;
    }

    return totillum/count;

}

void SegmentListVIIRSDNB::finishedviirs()
{

    qDebug() << "=============>SegmentListVIIRSDNB::readfinishedviirs()";
    emit progressCounter(100);
    opts.texture_changed = true;
    QApplication::restoreOverrideCursor();
    delete watcherviirs;

    emit segmentprojectionfinished(true);
}

void SegmentListVIIRSDNB::progressreadvalue(int progress)
{
    int totalcount = segsselected.count();
    this->progressresultready += 100 / totalcount;

    emit progressCounter(this->progressresultready);

    qDebug() << QString("SegmentListVIIRS::progressreadvalue( %1 )").arg(progress);
}



void SegmentListVIIRSDNB::ShowImageSerial()
{

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    for (int i=0; i < 3; i++)
    {
        for (int j=0; j < 1024; j++)
        {
            imageptrs->segment_stats_ch[i][j] = 0;
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


    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);
        segm->initializeMemory();
        ++segsel;
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);
        segm->ReadDatasetsInMemory();
        ++segsel;
    }


    long cnt_active_pixels = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);

//        for(int i = 0; i < (composecolor ? 3 : 1); i++)
//        {
//            if( segm->stat_max_ch[i] > this->stat_max_ch[i])
//                this->stat_max_ch[i] = segm->stat_max_ch[i];
//            if( segm->stat_min_ch[i] < this->stat_min_ch[i])
//                this->stat_min_ch[i] = segm->stat_min_ch[i];
//        }
        cnt_active_pixels += segm->active_pixels[0];
        ++segsel;
    }


//    for(int i = 0; i < (composecolor ? 3 : 1); i++)
//    {
//        imageptrs->stat_max_ch[i] = this->stat_max_ch[i];
//        imageptrs->stat_min_ch[i] = this->stat_min_ch[i];
//    }

    imageptrs->active_pixels = cnt_active_pixels;

//    CalculateLUTDNB();

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);
        segm->ComposeSegmentImage();
        ++segsel;
    }
    qDebug() << " SegmentListVIIRS::ShowImageSerialDNB Finished !!";

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
    emit progressCounter(100);
}


void SegmentListVIIRSDNB::SmoothVIIRSImage()
{

    qDebug() << "start SegmentListVIIRSDNB::SmoothVIIRSImage()";

    int lineimage = 0;

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentVIIRSDNB *segmsave;

    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments(segmsave, segm);
        segmsave = segm;
        BilinearInterpolation(segm);
        ++segsel;
        lineimage += segm->NbrOfLines;
    }
}

void SegmentListVIIRSDNB::printData(SegmentVIIRSDNB *segm, int linesfrom, int viewsfrom)
{
    int yaaa = 0;
    int xaaa = 0;
    fprintf(stderr, "projectionCoordX \n");
    for( int i = linesfrom + 0; i < linesfrom + 32; i++) //this->NbrOfLines - 1; i++)
    {
        for( int j = viewsfrom + 0; j < viewsfrom + 16; j++) //this->earth_views_per_scanline - 1 ; j++ )
        {

            fprintf(stderr, "%u, ", segm->getProjectionX(i,j));
        }

        fprintf(stderr, "\n");
    }


    fprintf(stderr, "projectionCoordY \n");
    for( int i = linesfrom + 0; i < linesfrom + 32; i++) //this->NbrOfLines - 1; i++)
    {
        for( int j = viewsfrom + 0; j < viewsfrom + 16; j++) //this->earth_views_per_scanline - 1 ; j++ )
        {

            fprintf(stderr, "%u, ", segm->getProjectionY(i,j));
        }

        fprintf(stderr, "\n");
    }

    fprintf(stderr, "projectionCoordValues \n");
    for( int i = linesfrom + 0; i < linesfrom + 32; i++) //this->NbrOfLines - 1; i++)
    {
        for( int j = viewsfrom + 0; j < viewsfrom + 16; j++) //this->earth_views_per_scanline - 1 ; j++ )
        {

            fprintf(stderr, "%u, ", qRed(segm->getProjectionValue(i,j)));
        }

        fprintf(stderr, "\n");
    }

}

inline bool SegmentListVIIRSDNB::PixelOK(int pix)
{
    return (pix > 0 && pix < 65528 ? true : false);
}

void SegmentListVIIRSDNB::sliderCentreBandChanged(int val)
{
    float fval = val/20.0;
    qDebug() << QString("Value sliderCentreBandChanged value = %1").arg(pow(10,fval));


    float lowerlimit = pow(10, val/20.0)/pow(10, opts.dnbspbwindowsvalue);
    float upperlimit = pow(10, val/20.0)*pow(10, opts.dnbspbwindowsvalue);

    qDebug() << QString("lowerlimit = %1").arg(lowerlimit);
    qDebug() << QString("upperlimit = %1").arg(upperlimit);

    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);
        segm->ComposeSegmentImageWindow(lowerlimit, upperlimit);
        ++segsel;
    }
    emit segmentlistfinished(true);

}

void SegmentListVIIRSDNB::spbWindowValueChanged(int spbwindowval, int slcentreband)
{
    float fval = spbwindowval;

    qDebug() << QString("Value WindowBandChanged value = %1").arg(pow(10, fval));

    opts.dnbspbwindowsvalue = spbwindowval;

    float lowerlimit = pow(10, slcentreband/20.0)/pow(10, opts.dnbspbwindowsvalue);
    float upperlimit = pow(10, slcentreband/20.0)*pow(10, opts.dnbspbwindowsvalue);

    qDebug() << QString("lowerlimit = %1").arg(lowerlimit);
    qDebug() << QString("upperlimit = %1").arg(upperlimit);


    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);
        segm->ComposeSegmentImageWindow(lowerlimit, upperlimit);
        ++segsel;
    }
    emit segmentlistfinished(true);

}
