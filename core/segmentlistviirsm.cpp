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


SegmentListVIIRSM::SegmentListVIIRSM(SatelliteList *satl, QObject *parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListVIIRSM");

    satlist = satl;
    seglisttype = eSegmentType::SEG_VIIRSM;

    earthviews = 3200;
}

bool SegmentListVIIRSM::ComposeVIIRSImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{
    qDebug() << QString("SegmentListVIIRSM::ComposeVIIRSImage");

    QApplication::setOverrideCursor(( Qt::WaitCursor));
    watcherviirs = new QFutureWatcher<void>(this);
    connect(watcherviirs, SIGNAL(finished()), this, SLOT(finishedviirs()));

    QFuture<void> future;
    future = QtConcurrent::run(doComposeVIIRSMImageInThread, this, bandlist, colorlist, invertlist);
    watcherviirs->setFuture(future);

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
    if(imageptrs->ptrimageViirsM != NULL)
    {
        delete imageptrs->ptrimageViirsM;
        imageptrs->ptrimageViirsM = NULL;
    }

    imageptrs->ptrimageViirsM = new QImage(earthviews, totalnbroflines, QImage::Format_ARGB32);

    int deltaprogress = 99 / (totalnbrofsegments*2);
    int totalprogress = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        segm->ReadSegmentInMemory();
        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
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

    emit segmentlistfinished(true);
    emit progressCounter(100);


    return true;

}

void SegmentListVIIRSM::ComposeGVProjection(int inputchannel)
{

    qDebug() << "SegmentListVIIRSM::ComposeGVProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentGVProjection(inputchannel);
        emit segmentprojectionfinished(false);
        ++segit;
    }



    // the following code calculates a new LUT that only takes
    // the pixels in the projection into account and not the complete segment(s).
//    CalculateProjectionLUT();
//    segit = segsselected.begin();
//    while ( segit != segsselected.end() )
//    {
//        (*segit)->RecalculateProjection();
//         emit segmentprojectionfinished(false);
//        ++segit;
//    }



}

void SegmentListVIIRSM::CalculateProjectionLUT()
{
    qDebug() << "start SegmentListVIIRSM::CalculateProjectionLUT()";
    int earth_views;
    long stats_ch[3][256];
    long cnt_active_pixels = 0;

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 256; j++)
        {
            stats_ch[k][j] = 0;
        }
    }

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

    bool composecolor;

    int x, y;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        segm->recalculateStatsInProjection();
        ++segsel;
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int i = 0; i < (composecolor ? 3 : 1); i++)
        {
            if( segm->stat_max_projection[i] > this->stat_max_ch[i])
                this->stat_max_ch[i] = segm->stat_max_projection[i];
            if( segm->stat_min_projection[i] < this->stat_min_ch[i])
                this->stat_min_ch[i] = segm->stat_min_projection[i];
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


    segsel = segsselected.begin();
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
                    x = segm->getProjectionX(line, pixelx);
                    y = segm->getProjectionY(line, pixelx);
                    if(x >= 0 && x < imageptrs->ptrimageProjection->width() && y >= 0 && y < imageptrs->ptrimageProjection->height())
                    {
                        int pixel = *(segm->ptrbaVIIRS[k].data() + line * segm->earth_views_per_scanline + pixelx);
                        int pixcalc = 256 * (pixel - imageptrs->stat_min_ch[k]) / (imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k]);
                        pixcalc = ( pixcalc < 0 ? 0 : pixcalc);
                        pixcalc = ( pixcalc > 255 ? 255 : pixcalc );
                        stats_ch[k][pixcalc]++;
                    }

                }
            }
        }

        ++segsel;
    }

    float scale = 256.0 / (float)imageptrs->active_pixels;

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
            imageptrs->lut_ch[k][i] = (quint16)(sum_ch[k] * scale);
            imageptrs->lut_ch[k][i] = ( imageptrs->lut_ch[k][i] > 255 ? 255 : imageptrs->lut_ch[k][i]);
        }
    }
}

void SegmentListVIIRSM::finishedviirs()
{

    qDebug() << "=============>SegmentListVIIRSM::readfinishedviirs()";
    emit progressCounter(100);
    opts.texture_changed = true;
    delete watcherviirs;
    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
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


    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        segm->setBandandColor(bandlist, colorlist, invertlist);
        segm->initializeMemory();
        ++segsel;
    }

    int totalnbrofsegments = this->NbrOfSegmentsSelected();

    int deltaprogress = 99 / (totalnbrofsegments*2);
    int totalprogress = 0;


    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        segm->ReadDatasetsInMemory();
        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
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
    qDebug() << " SegmentListVIIRS::ShowImageSerialM Finished !!";

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
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



void SegmentListVIIRSM::SmoothVIIRSImage(bool combine)
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
            BilinearBetweenSegments(segmsave, segm, combine);
        segmsave = segm;
        BilinearInterpolation(segm, combine);
        //printData(segm);
        ++segsel;
        lineimage += segm->NbrOfLines;
    }

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

