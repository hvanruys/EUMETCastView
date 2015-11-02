#include <QtConcurrent>
#include "segmentlistviirsm.h"
#include "segmentviirsm.h"
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



void doComposeVIIRSMImageInThread(SegmentListVIIRSM *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{
    t->ComposeVIIRSImageInThread(bandlist, colorlist, invertlist);
}


SegmentListVIIRSM::SegmentListVIIRSM(SatelliteList *satl, QObject *parent, eSegmentType type)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListVIIRSM");

    satlist = satl;
    segtype = type;

    earthviews = 3200;
}

void SegmentListVIIRSM::GetFirstLastVisibleSegmentData( QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast)
{

    QString first_filename;
    QString last_filename;

    //SVMC_npp_d20141117_t0837599_e0839241_b15833_c20141117084501709131_eum_ops
    if( segmentlist.count() > 0)
    {
        //segmentlist.at(indexfirstvisible)->
        first_filename = segmentlist.at(indexfirstvisible)->fileInfo.fileName();
        last_filename = segmentlist.at(indexlastvisible)->fileInfo.fileName();
        if(first_filename.mid(0,8) == "SVMC_npp")
        {
            *satnamefirst = first_filename.mid(5, 3);
            *segdatefirst = QString("%1-%2-%3").arg(first_filename.mid(10,4)).arg(first_filename.mid(14,2)).arg(first_filename.mid(16,2));
            *segtimefirst = QString("%1:%2:%3").arg(first_filename.mid(20,2)).arg(first_filename.mid(22,2)).arg(first_filename.mid(24,2));
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

bool SegmentListVIIRSM::ComposeVIIRSImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{
    qDebug() << QString("SegmentListVIIRSM::ComposeVIIRSImage");

    QApplication::setOverrideCursor(( Qt::WaitCursor));
    watcherreadviirs = new QFutureWatcher<void>(this);
    connect(watcherreadviirs, SIGNAL(finished()), this, SLOT(readfinishedviirs()));

    QFuture<void> future;
    future = QtConcurrent::run(doComposeVIIRSMImageInThread, this, bandlist, colorlist, invertlist);
    watcherreadviirs->setFuture(future);

    return true;

}


bool SegmentListVIIRSM::ComposeVIIRSImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{

    qDebug() << "bool SegmentListVIIRSM::ComposeVIIRSImageInThread(QList<bool> bandlist, QList<int> colorlist) started";

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    emit progressCounter(10);

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

    // Reset memory ; segselected can be metop, noaa , hrp, gac, viirsm viirsdnb
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
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segit);
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
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
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
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        segm->ReadSegmentInMemory();

        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);

        //QApplication::processEvents();
        ++segsel;
    }

    bool composecolor;

    long cnt_active_pixels = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
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
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        segm->ComposeSegmentImage();

        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);

        QApplication::processEvents();
        ++segsel;
    }
    qDebug() << " SegmentListVIIRS::ComposeVIIRSMImageInThread Finished !!";

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished();
    emit progressCounter(100);


    return true;

}



void SegmentListVIIRSM::readfinishedviirs()
{

    qDebug() << "=============>SegmentListVIIRSM::readfinishedviirs()";
    emit progressCounter(100);
    opts.texture_changed = true;
    QApplication::restoreOverrideCursor();
    delete watcherreadviirs;

    emit segmentlistfinished();
}

void SegmentListVIIRSM::composefinishedviirs()
{

    qDebug() << "SegmentListVIIRSM::composefinishedviirs()";

    emit progressCounter(100);

    delete watchercomposeviirs;
    opts.texture_changed = true;
    QApplication::restoreOverrideCursor();

    emit segmentlistfinished();

}

void SegmentListVIIRSM::progressreadvalue(int progress)
{
    int totalcount = segsselected.count();
    this->progressresultready += 100 / totalcount;

    emit progressCounter(this->progressresultready);

    qDebug() << QString("SegmentListVIIRSM::progressreadvalue( %1 )").arg(progress);
}

void SegmentListVIIRSM::ShowImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
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
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        segm->setBandandColor(bandlist, colorlist, invertlist);
        segm->initializeMemory();
        ++segsel;
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        segm->ReadDatasetsInMemory();
        ++segsel;
    }

    bool composecolor;

    long cnt_active_pixels = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
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
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        segm->ComposeSegmentImage();
        ++segsel;
    }
    qDebug() << " SegmentListVIIRS::ShowImageSerialM Finished !!";

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished();
    emit progressCounter(100);
}


void SegmentListVIIRSM::CalculateLUT()
{
    qDebug() << "start SegmentListVIIRSM::CalculateLUT()";
    int earth_views;
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
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        composecolor = segm->composeColorImage();
        earth_views = segm->earth_views_per_scanline;

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < segm->earth_views_per_scanline; pixelx++)
                {
                    int pixel = *(segm->ptrbaVIIRS[k].data() + line * segm->earth_views_per_scanline + pixelx);
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


void SegmentListVIIRSM::SmoothVIIRSImage()
{

    qDebug() << "start SegmentListVIIRSM::SmoothVIIRSImage()";

    int lineimage = 0;

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentVIIRSM *segmsave;

    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments(segmsave, segm);
        segmsave = segm;
        BilinearInterpolation(segm);
        //printData(segm);
        ++segsel;
        lineimage += segm->NbrOfLines;
    }

//    QRgb *scanline;
//    QRgb rgbval;
//    long count100 = 0;
//    long count0 = 0;
//    long count250 = 0;
//    long count200 = 0;

//    for(int j = 0; j < imageptrs->ptrimageProjection->height(); j++)
//    {
//        scanline = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(j);

//        for(int i = 0; i < imageptrs->ptrimageProjection->width(); i++)
//        {
//            rgbval = scanline[i];
//            if(qAlpha(rgbval) == 100)
//                count100++;
//            else if(qAlpha(rgbval) == 0)
//                count0++;
//            else if(qAlpha(rgbval) == 200)
//                count200++;
//            else if(qAlpha(rgbval) == 250)
//                count250++;
//        }
//    }

//    qDebug() << QString("Count100 = %1  count0 = %2 count200 = %3 count250 = %4").arg(count100).arg(count0).arg(count200).arg(count250);

}

void SegmentListVIIRSM::printData(SegmentVIIRSM *segm, int linesfrom, int viewsfrom)
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

inline bool SegmentListVIIRSM::PixelOK(int pix)
{
    return (pix > 0 && pix < 65528 ? true : false);
}

