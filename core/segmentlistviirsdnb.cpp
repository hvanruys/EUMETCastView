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


SegmentListVIIRSDNB::SegmentListVIIRSDNB(SatelliteList *satl, QObject *parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListVIIRSDNB");

    satlist = satl;
    seglisttype = eSegmentType::SEG_VIIRSDNB;

    earthviews = 4064;
    moonillumination = 0.0;

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
    if(imageptrs->ptrimageViirsDNB != NULL)
    {
        delete imageptrs->ptrimageViirsDNB;
        imageptrs->ptrimageViirsDNB = NULL;
    }

    imageptrs->ptrimageViirsDNB = new QImage(earthviews, totalnbroflines, QImage::Format_ARGB32);

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

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);
        segm->CalcGraph(&graphvalues);
        ++segsel;
    }

    fitDNBCurve();

    float lowerlimit = pow(10, opts.dnbsbvalue/20.0)/pow(10, opts.dnbspbwindowsvalue);
    float upperlimit = pow(10, opts.dnbsbvalue/20.0)*pow(10, opts.dnbspbwindowsvalue);

    int count = 0;
    float totillum = 0;

    for(int i = 0; i < xDNBcurve.length(); i ++)
    {
        qDebug() << QString("x = %1   y = %2").arg(xDNBcurve.at(i)).arg(yDNBcurve.at(i));
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSDNB *segm = (SegmentVIIRSDNB *)(*segsel);

//        segm->ComposeSegmentImageWindow(lowerlimit, upperlimit);
        segm->ComposeSegmentImageWindowFromCurve(&xDNBcurve, &yDNBcurve);
        totillum += segm->MoonIllumFraction;
        count++;

        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        ++segsel;
    }

    //emit progressCounter(100);
    emit displayDNBGraph();

    moonillumination = totillum/count;

    qDebug() << " SegmentListVIIRS::ComposeVIIRSDNBImageInThread Finished !!";

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);

    return true;
}

void SegmentListVIIRSDNB::fitDNBCurve()
{


    QVector<double> xmax(180), ymax(180); // initialize with entries 0..100

//    for(int i = 149; i >= 0; i--)
//    {

//            double lowind = (double)(i % 10 + 1)/10.0;
//            double upperind = (double)(i % 10)/10.0;
//            double lowerlimit2 = pow(10, -lowind - (int)(i/10.0));
//            double upperlimit2 = pow(10, -upperind - (int)(i/10.0));
//            int index = 149 - i;
//            qDebug() << QString("---- index = %1 lowelimit2 = %2  upperlimit2 = %3")
//                        .arg(index).arg(lowerlimit2).arg(upperlimit2);
//    }


    for(int xzenith = 0; xzenith < 180; xzenith++)
    {
        xmax[xzenith] = xzenith;
        ymax[xzenith] = 0;
        double radval = 0;
        long maxval = 0;
        int jmax = 0;

        for(int j = 149; j >= 0; j--)
        {
            int index = j * 180 + xzenith;
            long val = this->graphvalues.operator [](index);
            if(val > maxval)
            {
                maxval = val;
                jmax = 149 - j;
            }
        }
        //qDebug() << QString("maxval = %1 jmax = %2").arg(maxval).arg(jmax);

        if(maxval > 0)
        {
            double lowind = (double)((jmax % 10) + 1)/10.0;
            double upperind = (double)(jmax % 10)/10.0;
            double lowerlimit = pow(10, -lowind - (int)(jmax/10));
            double upperlimit = pow(10, -upperind - (int)(jmax/10));

            ymax[xzenith] = upperlimit;
          //  qDebug() << QString("lowlimit = %1   upperlimit = %2").arg(lowerlimit, 0, 'E', 3).arg(upperlimit, 0, 'E', 3);
        }
    }

    for(int i = 0; i < 180; i++)
    {
        if(ymax[i] > 0)
        {
            xDNBcurve.append(xmax.at(i));
            yDNBcurve.append(ymax.at(i));
        }
    }
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
        cnt_active_pixels += segm->active_pixels[0];
        ++segsel;
    }

    imageptrs->active_pixels = cnt_active_pixels;

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


void SegmentListVIIRSDNB::SmoothVIIRSImage(bool combine)
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
            BilinearBetweenSegments(segmsave, segm, combine);
        segmsave = segm;
        BilinearInterpolation(segm, combine);
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

    qDebug() << QString("dnbsbvalue = %1, dnbspbwindowsvalue = %2").arg(opts.dnbsbvalue).arg(opts.dnbspbwindowsvalue);

    qDebug() << QString("lowerlimit = %1").arg(lowerlimit, 0, 'E', 2);
    qDebug() << QString("upperlimit = %1").arg(upperlimit, 0, 'E', 2);

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
