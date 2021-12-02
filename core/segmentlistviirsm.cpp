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
extern bool ptrimagebusy;



void SegmentListVIIRSM::doComposeVIIRSMImageInThread(SegmentListVIIRSM *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{
    t->ComposeVIIRSImageInThread(bandlist, colorlist, invertlist);
}


SegmentListVIIRSM::SegmentListVIIRSM(SatelliteList *satl, eSegmentType type, QObject *parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListVIIRSM");

    satlist = satl;
    seglisttype = type;
    earthviews = 3200;
}

bool SegmentListVIIRSM::ComposeVIIRSImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{
    qDebug() << QString("SegmentListVIIRSM::ComposeVIIRSImage");

    this->bandlist = bandlist;
    this->colorlist = colorlist;
    this->inverselist = invertlist;

    ptrimagebusy = true;

    QApplication::setOverrideCursor(( Qt::WaitCursor));

    connect(&watcherviirs, SIGNAL(finished()), this, SLOT(finishedviirs()));

    QFuture<void> future;
    future = QtConcurrent::run(doComposeVIIRSMImageInThread, this, bandlist, colorlist, invertlist);
    watcherviirs.setFuture(future);

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
    this->minBrightnessTemp = 9999999.9;
    this->maxBrightnessTemp = 0.0;

    // Reset memory ; segselected can be metop, noaa , hrp, gac, viirsm, viirsdnb, olciefr and olcierr
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
     qDebug() << "bool SegmentListVIIRSM::ComposeVIIRSImageInThread ptrimageViirsM QImage created " <<
                 imageptrs->ptrimageViirsM->width() << "X" << imageptrs->ptrimageViirsM->height();

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

        if(segm->maxBrightnessTemp > this->maxBrightnessTemp)
            this->maxBrightnessTemp = segm->maxBrightnessTemp;
        if(segm->minBrightnessTemp < this->minBrightnessTemp)
            this->minBrightnessTemp = segm->minBrightnessTemp;


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
    imageptrs->minBrightnessTemp = this->minBrightnessTemp;
    imageptrs->maxBrightnessTemp = this->maxBrightnessTemp;


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

    qDebug() << QString("minBrightnessTemp = %1 maxBrightnessTemp = %2").arg(minBrightnessTemp).arg(maxBrightnessTemp);
    qDebug() << " SegmentListVIIRS::ComposeVIIRSMImageInThread Finished !!";



//    segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
//        segm->ComposeSegmentImage();
//        totalprogress += deltaprogress;
//        emit progressCounter(totalprogress);
//        QApplication::processEvents();
//        ++segsel;
//    }


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
        (*segit)->ComposeSegmentGVProjection(inputchannel, 0, false);
        emit segmentprojectionfinished(false);
        ++segit;
    }

    initBrightnessTemp();

    //the following code calculates a new LUT that only takes
    //the pixels in the projection into account and not the complete segment(s).
//   CalculateProjectionLUT();
//   segit = segsselected.begin();
//   while ( segit != segsselected.end() )
//   {
//       (*segit)->RecalculateProjection();
//        emit segmentprojectionfinished(false);
//       ++segit;
//   }

}


void SegmentListVIIRSM::ComposeLCCProjection(int inputchannel)
{

    qDebug() << "SegmentListVIIRSM::ComposeLCCProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentLCCProjection(inputchannel, 0, false);
        emit segmentprojectionfinished(false);
        ++segit;
    }

    initBrightnessTemp();
}

void SegmentListVIIRSM::ComposeSGProjection(int inputchannel)
{

    qDebug() << "SegmentListVIIRSM::ComposeSGProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentSGProjection(inputchannel, 0, false);
        emit segmentprojectionfinished(false);
        ++segit;
    }

    initBrightnessTemp();
}

void SegmentListVIIRSM::ComposeOMProjection(int inputchannel)
{

    qDebug() << "SegmentListVIIRSM::ComposeOMProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentOMProjection(inputchannel, 0, false);
        emit segmentprojectionfinished(false);
        QApplication::processEvents();

        ++segit;
    }

}

void SegmentListVIIRSM::initBrightnessTemp()
{
    int height = imageptrs->ptrimageProjection->height();
    int width = imageptrs->ptrimageProjection->width() ;

    float fmin = 9999999.0;
    float fmax = 0.0;
    for( int i = 0; i < height; i++)
    {
        for( int j = 0; j < width; j++ )
        {
            float tmp = imageptrs->ptrProjectionBrightnessTemp[i * width + j];
            if(tmp > 0.0)
            {
                if(tmp>=fmax)
                    fmax = tmp;
                if(tmp<=fmin)
                    fmin = tmp;
            }
        }
    }

    this->minBrightnessTempProjection = fmin;
    this->maxBrightnessTempProjection = fmax;

    qDebug() << QString("----> min max ptrProjectionBrightnessTemp min = %1 max = %2").arg(fmin).arg(fmax);

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
    ptrimagebusy = false;
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
    this->minBrightnessTemp = 9999999.9;
    this->maxBrightnessTemp = 0.0;


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

        if(segm->maxBrightnessTemp > this->maxBrightnessTemp)
            this->maxBrightnessTemp = segm->maxBrightnessTemp;
        if(segm->minBrightnessTemp < this->minBrightnessTemp)
            this->minBrightnessTemp = segm->minBrightnessTemp;

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
    imageptrs->minBrightnessTemp = this->minBrightnessTemp;
    imageptrs->maxBrightnessTemp = this->maxBrightnessTemp;

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
        ++segsel;
    }
}

void SegmentListVIIRSM::SmoothVIIRSImage12bits()
{

    qDebug() << "start SegmentListVIIRSM::SmoothVIIRSImage12bits()";

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentVIIRSM *segmsave;

    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments12bits(segmsave, segm);
        segmsave = segm;
        BilinearInterpolation12bits(segm);
        ++segsel;
    }
}

void SegmentListVIIRSM::SmoothProjectionBrightnessTemp()
{

    qDebug() << "start SegmentListVIIRSM::SmoothProjectionBrightnessTemp()";

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentVIIRSM *segmsave;

    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegmentsFloat(segmsave, segm);
        segmsave = segm;
        BilinearInterpolationFloat(segm);
        ++segsel;
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

void SegmentListVIIRSM::BilinearInterpolationFloat(SegmentVIIRSM *segm)
{
    qint32 x11;
    qint32 y11;
    float bt11;

    qint32 x12;
    qint32 y12;
    float bt12;

    qint32 x21;
    qint32 y21;
    float bt21;

    qint32 x22;
    qint32 y22;
    float bt22;

    qint32 xc11;
    qint32 yc11;

    qint32 xc12;
    qint32 yc12;

    qint32 xc21;
    qint32 yc21;

    qint32 xc22;
    qint32 yc22;

    qint32 minx;
    qint32 miny;
    qint32 maxx;
    qint32 maxy;

    qint32 anchorX;
    qint32 anchorY;

    int dimx, dimy;

    long counter = 0;
    long counterb = 0;

    float *canvasfloat;
    int *canvasint;

    int earthviews = this->NbrOfEartviewsPerScanline();

    for (int line = 0; line < segm->NbrOfLines-1; line++)
    {
        for (int pixelx = 0; pixelx < earthviews-1; pixelx++)
        {
            x11 = segm->getProjectionX(line, pixelx);
            y11 = segm->getProjectionY(line, pixelx);

            x12 = segm->getProjectionX(line, pixelx+1);
            y12 = segm->getProjectionY(line, pixelx+1);

            x21 = segm->getProjectionX(line+1, pixelx);
            y21 = segm->getProjectionY(line+1, pixelx);

            x22 = segm->getProjectionX(line+1, pixelx+1);
            y22 = segm->getProjectionY(line+1, pixelx+1);

            if(x11 < 65528 && x12 < 65528 && x21 < 65528 && x22 < 65528
                    && y11 < 65528 && y12 < 65528 && y21 < 65528 && y22 < 65528)
                    // && x11 > -50 && x12 > -50 && x21 > -50 && x22 > -50
                    // && y11 > -50 && y12 > -50 && y21 > -50 && y22 > -50 )
            {
                minx = Min(x11, x12, x21, x22);
                miny = Min(y11, y12, y21, y22);
                maxx = Max(x11, x12, x21, x22);
                maxy = Max(y11, y12, y21, y22);

                anchorX = minx;
                anchorY = miny;
                dimx = maxx + 1 - minx;
                dimy = maxy + 1 - miny;
                if( dimx == 1 && dimy == 1 )
                {
                    counter++;
                }
                else
                {
                    bt11 = segm->getBrightnessTemp(line, pixelx);
                    bt12 = segm->getBrightnessTemp(line, pixelx+1);
                    bt21 = segm->getBrightnessTemp(line+1, pixelx);
                    bt22 = segm->getBrightnessTemp(line+1, pixelx+1);


                    xc11 = x11 - minx;
                    xc12 = x12 - minx;
                    xc21 = x21 - minx;
                    xc22 = x22 - minx;
                    yc11 = y11 - miny;
                    yc12 = y12 - miny;
                    yc21 = y21 - miny;
                    yc22 = y22 - miny;


                    canvasfloat = new float[dimx * dimy];
                    for(int i = 0 ; i < dimx * dimy ; i++)
                        canvasfloat[i] = -1.0;

                    canvasint = new int[dimx * dimy];
                    for(int i = 0 ; i < dimx * dimy ; i++)
                        canvasint[i] = -1;

                    canvasfloat[yc11 * dimx + xc11] = bt11;
                    canvasfloat[yc12 * dimx + xc12] = bt12;
                    canvasfloat[yc21 * dimx + xc21] = bt21;
                    canvasfloat[yc22 * dimx + xc22] = bt22;

                    canvasint[yc11 * dimx + xc11] = 1;
                    canvasint[yc12 * dimx + xc12] = 1;
                    canvasint[yc21 * dimx + xc21] = 1;
                    canvasint[yc22 * dimx + xc22] = 1;

//                    std::cout.precision(4);

//                    if(line == 1 && pixelx == 1)
//                    {
//                        //qDebug() << QString("rgb11 = %1 rgb12 = %2 rgb21 = %3 rgb22 = %4").arg(qRed(rgb11)).arg(qRed(rgb12)).arg(qRed(rgb21)).arg(qRed(rgb22));
//                        for ( int i = 0; i < dimy; i++ )
//                        {
//                            for ( int j = 0; j < dimx; j++ )
//                            {
//                                std::cout << std::setw(5) << canvasfloat[i * dimx + j] << " ";
//                            }
//                            std::cout << std::endl;
//                        }
//                        std::cout << "before ....................................... line " << line << " pixelx = " << pixelx << std::endl;
//                    }

//                    std::flush(cout);

                    bhm_line_float(xc11, yc11, xc12, yc12, bt11, bt12, canvasfloat, canvasint, dimx);
                    bhm_line_float(xc12, yc12, xc22, yc22, bt12, bt22, canvasfloat, canvasint, dimx);
                    bhm_line_float(xc22, yc22, xc21, yc21, bt22, bt21, canvasfloat, canvasint, dimx);
                    bhm_line_float(xc21, yc21, xc11, yc11, bt21, bt11, canvasfloat, canvasint, dimx);

//                    if(line == 1 && pixelx == 1)
//                    {
//                        for ( int i = 0; i < dimy; i++ )
//                        {
//                            for ( int j = 0; j < dimx; j++ )
//                            {
//                                std::cout << std::setw(5) << canvasfloat[i * dimx + j] << " ";
//                            }
//                            std::cout << std::endl;
//                        }
//                        std::cout << "after -------------------------------------- line " << line << " pixelx = " << pixelx << std::endl;
//                    }

                    MapInterpolationFloat(canvasfloat, canvasint, dimx, dimy);
                    MapCanvasFloat(canvasfloat, canvasint, anchorX, anchorY, dimx, dimy);

//                    if(line == 1 && pixelx == 1)
//                    {
//                        for ( int i = 0; i < dimy; i++ )
//                        {
//                            for ( int j = 0; j < dimx; j++ )
//                            {
//                                std::cout << std::setw(3) << qRed(canvas[i * dimx + j]) << " ";
//                            }
//                            std::cout << std::endl;
//                        }
//                        std::cout << "================================= line " << line << " pixelx = " << pixelx << std::endl;
//                    }

                    delete [] canvasfloat;
                    delete [] canvasint;
                    counterb++;
                }
            }
        }
    }

    qDebug() << QString("====> end SegmentList::BilinearInterpolation(Segment *segm) counter = %1 countern = %2").arg(counter).arg(counterb);

}

void SegmentListVIIRSM::BilinearBetweenSegmentsFloat(SegmentVIIRSM *segmfirst, SegmentVIIRSM *segmnext)
{
    qint32 x11;
    qint32 y11;
    float bt11;

    qint32 x12;
    qint32 y12;
    float bt12;

    qint32 x21;
    qint32 y21;
    float bt21;

    qint32 x22;
    qint32 y22;
    float bt22;

    qint32 xc11;
    qint32 yc11;

    qint32 xc12;
    qint32 yc12;

    qint32 xc21;
    qint32 yc21;

    qint32 xc22;
    qint32 yc22;

    qint32 minx;
    qint32 miny;
    qint32 maxx;
    qint32 maxy;

    qint32 anchorX;
    qint32 anchorY;

    int dimx, dimy;

    long counter = 0;
    long counterb = 0;

    float *canvasfloat;
    int *canvasint;

    int earthviews = this->NbrOfEartviewsPerScanline();

    for (int pixelx = 0; pixelx < earthviews-1; pixelx++)
    {
        x11 = segmfirst->getProjectionX(segmfirst->NbrOfLines-1, pixelx);
        y11 = segmfirst->getProjectionY(segmfirst->NbrOfLines-1, pixelx);

        x12 = segmfirst->getProjectionX(segmfirst->NbrOfLines-1, pixelx+1);
        y12 = segmfirst->getProjectionY(segmfirst->NbrOfLines-1, pixelx+1);

        x21 = segmnext->getProjectionX(0, pixelx);
        y21 = segmnext->getProjectionY(0, pixelx);

        x22 = segmnext->getProjectionX(0, pixelx+1);
        y22 = segmnext->getProjectionY(0, pixelx+1);

        if(x11 < 65528 && x12 < 65528 && x21 < 65528 && x22 < 65528
                && y11 < 65528 && y12 < 65528 && y21 < 65528 && y22 < 65528
                //&& x11 >= -20 && x12 >= -20 && x21 >= -20 && x22 >= -20
                //&& y11 >= -20 && y12 >= -20 && y21 >= -20 && y22 >= -20
                && abs(x11 - x21) < 100)
        {

            minx = Min(x11, x12, x21, x22);
            miny = Min(y11, y12, y21, y22);
            maxx = Max(x11, x12, x21, x22);
            maxy = Max(y11, y12, y21, y22);

            anchorX = minx;
            anchorY = miny;
            dimx = maxx + 1 - minx;
            dimy = maxy + 1 - miny;
            if( dimx == 1 && dimy == 1 )
            {
                counter++;
            }
            else
            {
                bt11 = segmfirst->getBrightnessTemp(segmfirst->NbrOfLines-1, pixelx);
                bt12 = segmfirst->getBrightnessTemp(segmfirst->NbrOfLines-1, pixelx+1);
                bt21 = segmnext->getBrightnessTemp(0, pixelx);
                bt22 = segmnext->getBrightnessTemp(0, pixelx+1);

                xc11 = x11 - minx;
                xc12 = x12 - minx;
                xc21 = x21 - minx;
                xc22 = x22 - minx;
                yc11 = y11 - miny;
                yc12 = y12 - miny;
                yc21 = y21 - miny;
                yc22 = y22 - miny;

                canvasfloat = new float[dimx * dimy];
                for(int i = 0 ; i < dimx * dimy ; i++)
                    canvasfloat[i] = -1.0;

                canvasint = new int[dimx * dimy];
                for(int i = 0 ; i < dimx * dimy ; i++)
                    canvasint[i] = -1;

                canvasfloat[yc11 * dimx + xc11] = bt11;
                canvasfloat[yc12 * dimx + xc12] = bt12;
                canvasfloat[yc21 * dimx + xc21] = bt21;
                canvasfloat[yc22 * dimx + xc22] = bt22;

                canvasint[yc11 * dimx + xc11] = 1;
                canvasint[yc12 * dimx + xc12] = 1;
                canvasint[yc21 * dimx + xc21] = 1;
                canvasint[yc22 * dimx + xc22] = 1;

                bhm_line_float(xc11, yc11, xc12, yc12, bt11, bt12, canvasfloat, canvasint, dimx);
                bhm_line_float(xc12, yc12, xc22, yc22, bt12, bt22, canvasfloat, canvasint, dimx);
                bhm_line_float(xc22, yc22, xc21, yc21, bt22, bt21, canvasfloat, canvasint, dimx);
                bhm_line_float(xc21, yc21, xc11, yc11, bt21, bt11, canvasfloat, canvasint, dimx);

                MapInterpolationFloat(canvasfloat, canvasint, dimx, dimy);
                MapCanvasFloat(canvasfloat, canvasint, anchorX, anchorY, dimx, dimy);

                delete [] canvasfloat;
                delete [] canvasint;
                counterb++;
            }
        }
    }


    qDebug() << QString("====> end SegmentList::BilinearBetweenSegmentsFloat(Segment *segmfirst, Segment *segmnext) counter = %1 counterb = %2").arg(counter).arg(counterb);

}


bool SegmentListVIIRSM::bhm_line_float(int x1, int y1, int x2, int y2, float bt1, float bt2, float *canvas, int *canvas1, int dimx)
{
    int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;
    float delta;
    float val1, val2;

    dx=x2-x1;
    dy=y2-y1;
    dx1=abs(dx);
    dy1=abs(dy);
    px=2*dy1-dx1;
    py=2*dx1-dy1;

    val1 = bt1;
    val2 = bt2;

    if(dy1<=dx1)
    {
        if(dx1==0)
            return false;

        if(dx>=0)
        {
            x=x1;
            y=y1;
            xe=x2;
            delta = (val2 - val1)/(float)dx1 ;
        }
        else
        {
            x=x2;
            y=y2;
            xe=x1;
            delta = (val1 - val2)/ (float)dx1 ;
        }

        for(i=0;x<xe;i++)
        {
            x=x+1;

            if(px<0)
            {
                px=px+2*dy1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    y=y+1;
                }
                else
                {
                    y=y-1;
                }
                px=px+2*(dy1-dx1);
            }
            if(dx>=0)
            {
                val1 += delta;
                if( x != xe)
                {
                    canvas[y * dimx + x] = val1;
                    canvas1[y * dimx + x] = 1;
                }
            }
            else
            {
                val2 += delta;
                if( x != xe)
                {
                    canvas[y * dimx + x] = val2;
                    canvas1[y * dimx + x] = 1;
                }
            }

        }
    }
    else
    {
        if(dy1==0)
            return false;

        if(dy>=0)
        {
            x=x1;
            y=y1;
            ye=y2;
            delta = (val2 - val1)/ (float)dy1 ;
        }
        else
        {
            x=x2;
            y=y2;
            ye=y1;
            delta = (val1 - val2)/ (float)dy1 ;
        }


        for(i=0;y<ye;i++)
        {
            y=y+1;

            if(py<=0)
            {
                py=py+2*dx1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    x=x+1;
                }
                else
                {
                    x=x-1;
                }
                py=py+2*(dx1-dy1);
            }
            if(dy>=0)
            {
                val1 += delta;
                if( y != ye)
                {
                    canvas[y * dimx + x] = val1;
                    canvas1[y * dimx + x] = 1;
                }
            }
            else
            {
                val2 += delta;
                if( y != ye)
                {
                    canvas[y * dimx + x] = val2;
                    canvas1[y * dimx + x] = 1;
                }
            }
        }
    }

    return true;
}


void SegmentListVIIRSM::MapInterpolationFloat(float *canvas, int *canvas1, quint16 dimx, quint16 dimy)
{

    for(int h = 0; h < dimy; h++ )
    {
        float start = 0.0;
        float end = 0.0;
        bool first = false;
        bool last = false;
        int holecount = 0;
        int firstindex;
        int lastindex;

        for(int w = 0; w < dimx; w++)
        {
            float val = canvas[h * dimx + w];
            int val1 = canvas1[h * dimx + w];
            if(val1 == 1)
            {
                firstindex = w;
                first = true;
                start = val;
            }
            else if(val1 == -1 && first == true)
            {
                break;
            }
        }


        for(int w = dimx - 1; w >= 0; w--)
        {
            float val = canvas[h * dimx + w];
            int val1 = canvas1[h * dimx + w];
            if(val1 == 1 && firstindex <= w)
            {
                lastindex = w;
                last = true;
                end = val;
            }
            else if(val1 == -1 && last == true)
            {
                break;
            }
        }


        for(int w = firstindex; w < lastindex; w++)
        {
            if(canvas1[h * dimx + w] == -1)
            {
                canvas1[h * dimx + w] = 0;
                holecount++;
            }
        }

        if(holecount == 0)
            continue;

        float delta = (end - start) / (float)(holecount+1);
        float temp = start;

        for(int w = 0; w < dimx; w++)
        {
            int val1 = canvas1[h * dimx + w];
            if(val1 == 0)
            {
                temp += delta;
                canvas[h * dimx + w] = temp;
                canvas1[h * dimx + w] = 2;
            }
        }
    }


    for(int w = 0; w < dimx; w++)
    {
        float start = 0.0;
        float end = 0.0;

        int hcount = 0;

        bool startok = false;

        for(int h = 0; h < dimy; h++)
        {
            float val = canvas[h * dimx + w];
            int val1 = canvas1[h * dimx + w];
            if(val > 0.0 && !startok)
            {
                start = val;
            }
            else
            {
                if(val1 == 1)
                {
                    end = val;
                    break;
                }
                else if(val1 == 2)
                {
                    startok = true;
                    hcount++;
                }

            }
        }

        if(hcount == 0)
            continue;

        float temp = start;
        float delta = (end - start) / (float)(hcount+1);

        for(int h = 0; h < dimy; h++)
        {
            float val = canvas[h * dimx + w];
            int val1 = canvas1[h * dimx + w];
            if(val1 == 2)
            {
                temp += delta;
                float total = (canvas[h * dimx + w] + temp)/2;

                canvas[h * dimx + w] = total;
            }
        }
    }
}



void SegmentListVIIRSM::MapCanvasFloat(float *canvas, int *canvas1, qint32 anchorX, qint32 anchorY, quint16 dimx, quint16 dimy)
{
    for(int h = 0; h < dimy; h++ )
    {
        for(int w = 0; w < dimx; w++)
        {
            float val = canvas[h * dimx + w];
            int val1 = canvas1[h * dimx + w];
            if(val1 > -1)
            {
                if (anchorX + w >= 0 && anchorX + w < imageptrs->ptrimageProjection->width() &&
                        anchorY + h >= 0 && anchorY + h < imageptrs->ptrimageProjection->height())
                {
                     imageptrs->ptrProjectionBrightnessTemp[(anchorY + h) * imageptrs->ptrimageProjection->width() + anchorX + w] = val;
                }
            }
        }
    }
}

void SegmentListVIIRSM::GetCentralCoords(double *startcentrallon, double *startcentrallat, double *endcentrallon, double *endcentrallat)
{
    double slon, slat, elon, elat;
    double save_slon, save_slat, save_elon, save_elat;
    int startindex, endindex;

    save_slon = 65535.0;
    save_slat = 65535.0;
    save_elon = 65535.0;
    save_elat = 65535.0;

    bool first = true;

    QList<Segment*>::iterator segsel = segsselected.begin();

    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        segm->getCentralCoords(&slon, &slat, &elon, &elat, &startindex, &endindex);

        if(abs(slon) <= 180.0 && abs(slat) <= 90.0 && abs(elon) <= 180.0 && abs(elat) <= 90.0)
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

void SegmentListVIIRSM::GetCornerCoords(double *cornerlon1, double *cornerlat1, double *cornerlon2, double *cornerlat2, double *cornerlon3, double *cornerlat3, double *cornerlon4, double *cornerlat4)
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
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        count++;
        if(count == 1)
        {
            segm->getStartCornerCoords(&save_cornerlon1, &save_cornerlat1, &save_cornerlon2, &save_cornerlat2, &Xcornerindex1, &Xcornerindex2, &Ycornerindex12 );
            if(abs(save_cornerlon1) <= 180.0 && abs(save_cornerlat1) <= 90.0 && abs(save_cornerlon2) <= 180.0 && abs(save_cornerlat2) <= 90.0)
                break;
        }
        ++segsel;
    }

    segsel = segsselected.begin();
    count = 0;

    while ( segsel != segsselected.end() )
    {
        SegmentVIIRSM *segm = (SegmentVIIRSM *)(*segsel);
        count++;

        if(count == segsselected.size())
        {
            segm->getEndCornerCoords(&save_cornerlon3, &save_cornerlat3, &save_cornerlon4, &save_cornerlat4, &Xcornerindex3, &Xcornerindex4, &Ycornerindex34 );
            if(abs(save_cornerlon3) <= 180.0 && abs(save_cornerlat3) <= 90.0 && abs(save_cornerlon4) <= 180.0 && abs(save_cornerlat4) <= 90.0)
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

