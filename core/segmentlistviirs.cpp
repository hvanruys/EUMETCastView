#include <QtConcurrent>
#include "segmentlistviirs.h"
#include "segmentviirs.h"
#include "segmentimage.h"
#include "options.h"

#include <QDir>
#include <QDebug>
#include <QPainter>
#include <math.h>

extern Options opts;
extern SegmentImage *imageptrs;

void SegmentListVIIRS::doReadSegmentInMemoryVIIRS(Segment *t)
{
    t->ReadSegmentInMemory();
}

void SegmentListVIIRS::doComposeSegmentImageVIIRS(Segment *t)
{
    t->ComposeSegmentImage();
}

void SegmentListVIIRS::doComposeProjection(Segment *t)
{
    t->ComposeProjectionConcurrent();
}

SegmentListVIIRS::SegmentListVIIRS(SatelliteList *satl, QObject *parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListVIIRS");

    earthviews = 3200;

    satlist = satl;

}

void SegmentListVIIRS::GetFirstLastVisibleSegmentData( QString *satnamefirst, QString *segdatefirst, QString *segtimefirst,  QString *satnamelast, QString *segdatelast, QString *segtimelast)
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
        if(last_filename.mid(0,8) == "SVMC_npp")
        {
            *satnamelast = last_filename.mid(5, 3);
            *segdatelast = QString("%1-%2-%3").arg(last_filename.mid(10,4)).arg(last_filename.mid(14,2)).arg(last_filename.mid(16,2));
            *segtimelast = QString("%1:%2:%3").arg(last_filename.mid(20,2)).arg(last_filename.mid(22,2)).arg(last_filename.mid(24,2));
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

bool SegmentListVIIRS::ComposeVIIRSImageConcurrent(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{

    qDebug() << "bool SegmentListVIIRS::ComposeNewImage(QList<bool> bandlist, QList<int> colorlist) started";

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


void SegmentListVIIRS::viirsFinished()
{
    QApplication::restoreOverrideCursor();

    qDebug() << "viirsFinished()";

    emit segmentlistfinished();
    emit progressCounter(100);

}


bool SegmentListVIIRS::ComposeVIIRSImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
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


// Problems with reopen the hdf5 files in a concurrent environment .... does not work.
// Serial solution works.
//bool SegmentListVIIRS::ShowImage(QList<bool> bandlist, QList<int> colorlist)
//{

//    progressresultready = 0;
//    QApplication::setOverrideCursor( Qt::WaitCursor ); // reset in composefinishedviirs

//    for (int i=0; i < 3; i++)
//    {
//        for (int j=0; j < 1024; j++)
//        {
//            imageptrs->segment_stats_ch[i][j] = 0;
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


//    QList<Segment*>::iterator segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
//        segm->setBandandColor(bandlist, colorlist);
//        segm->initializeMemory();
//        ++segsel;
//    }


//    watcherreadviirs = new QFutureWatcher<void>(this);
//    connect(watcherreadviirs, SIGNAL(resultReadyAt(int)), SLOT(resultisreadyviirs(int)));
//    connect(watcherreadviirs, SIGNAL(finished()), SLOT(readfinishedviirs()));

//    watcherreadviirs->setFuture(QtConcurrent::map( segsselected.begin(), segsselected.end(), &SegmentListVIIRS::doReadDatasetsInMemoryVIIRS));

//    return true;

//}

void SegmentListVIIRS::readfinishedviirs()
{
    qDebug() << "SegmentListVIIRS::readfinishedviirs()";

    delete watcherreadviirs;

    int cntsegments = 0;

    bool composecolor;

    QList<Segment *>::iterator segsel = segsselected.begin();
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
        cntsegments++;
        ++segsel;
    }


    for(int i = 0; i < (composecolor ? 3 : 1); i++)
    {
        imageptrs->stat_max_ch[i] = this->stat_max_ch[i];
        imageptrs->stat_min_ch[i] = this->stat_min_ch[i];
    }

    CalculateLUT();

    watchercomposeviirs = new QFutureWatcher<void>(this);
    connect(watchercomposeviirs, SIGNAL(finished()), SLOT(composefinishedviirs()));
    watchercomposeviirs->setFuture(QtConcurrent::map(segsselected.begin(), segsselected.end(), &SegmentListVIIRS::doComposeSegmentImageVIIRS));
}


void SegmentListVIIRS::composefinishedviirs()
{

    qDebug() << "SegmentListVIIRS::composefinishedviirs()";

    //SmoothVIIRSImage();

    emit progressCounter(100);

    delete watchercomposeviirs;
    opts.texture_changed = true;
    QApplication::restoreOverrideCursor();

    emit segmentlistfinished();

}

void SegmentListVIIRS::progressreadvalue(int progress)
{
    int totalcount = segsselected.count();
    this->progressresultready += 100 / totalcount;

    emit progressCounter(this->progressresultready);

    qDebug() << QString("SegmentListVIIRS::progressreadvalue( %1 )").arg(progress);
}

void SegmentListVIIRS::ShowImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
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
        segm->setBandandColor(bandlist, colorlist, invertlist);
        segm->initializeMemory();
        ++segsel;
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
        segm->ReadDatasetsInMemory();
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
        ++segsel;
    }
    qDebug() << " SegmentListVIIRS::ShowImageSerial Finished !!";

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished();
    emit progressCounter(100);
}

void SegmentListVIIRS::CalculateLUT()
{
    qDebug() << "start SegmentListVIIRS::CalculateLUT()";
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
        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
        composecolor = segm->composeColorImage();
        earth_views = segm->earth_views_per_scanline;

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < segm->earth_views_per_scanline; pixelx++)
                {
                    int pixel = *(segm->ptrbaVIIRS[k] + line * segm->earth_views_per_scanline + pixelx);
                    int pixcalc = 256 * (pixel - imageptrs->stat_min_ch[k]) / (imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k]);
                    pixcalc = ( pixcalc < 0 ? 0 : pixcalc);
                    pixcalc = ( pixcalc > 255 ? 255 : pixcalc );
                    stats_ch[k][pixcalc]++;

                }
            }
        }


        ++segsel;
    }

    float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE

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

void SegmentListVIIRS::SmoothVIIRSImage()
{

    qDebug() << "start SegmentListVIIRS::SmoothVIIRSImage()";
    int earth_views;
    bool composecolor;

    int pixval[3];
    bool valok[3];

    int lineimage;
    int saveindex1 = -1;
    int saveindex2 = -1;
    int savepixval = -1;

    QList<Segment *>::iterator segsel;
    for (int pixelx = 0; pixelx < earthviews; pixelx++)
    {
        lineimage = 0;
        segsel = segsselected.begin();
        while ( segsel != segsselected.end() )
        {
            SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
            composecolor = segm->composeColorImage();

            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for(int k = 0; k < (composecolor ? 3 : 1); k++)
                    pixval[k] = *(segm->ptrbaVIIRS[k] + line * segm->earth_views_per_scanline + pixelx);


                valok[0] = PixelOK(pixval[0]);
                if(composecolor)
                {
                    valok[1] = PixelOK(pixval[1]);
                    valok[2] = PixelOK(pixval[2]);
                }

                if( valok[0] && (composecolor ? valok[1] && valok[2] : true))
                {
                    if(saveindex2 > 0)
                    {
                        InterpolateVIIRS(saveindex1, saveindex2+1, pixelx);
                        saveindex2 = -1;
                    }
                    saveindex1 = lineimage + line;
                    savepixval = pixval[0];
                }
                else
                {
                    if(savepixval != -1)
                    {
                        saveindex2 = lineimage + line;
                        savepixval = pixval[0];

                    }
                 }

            }
            ++segsel;
            lineimage += segm->NbrOfLines;
        }

    }

    qDebug() << "end SegmentListVIIRS::SmoothVIIRSImage()";



}

void SegmentListVIIRS::InterpolateVIIRS(int index1, int index2, int pixelx)
{
    //qDebug() << QString("index1 = %1 index2 = %2 pixelx = %4").arg(index1).arg(index2).arg(pixelx);

    QRgb *row_1, *row_2;

    row_1 = (QRgb*)imageptrs->ptrimageViirs->scanLine(index1);
    row_2 = (QRgb*)imageptrs->ptrimageViirs->scanLine(index2);

    QRgb pix1, pix2;

    pix1 = row_1[pixelx];
    pix2 = row_2[pixelx];

    int red1 = qRed(pix1);
    int red2 = qRed(pix2);
    int green1 = qGreen(pix1);
    int green2 = qGreen(pix2);
    int blue1 = qBlue(pix1);
    int blue2 = qBlue(pix2);

    int parts = index2 - index1;

    int deltared = (red2 - red1)/parts;
    int deltagreen = (green2 - green1)/parts;
    int deltablue = (blue2 - blue1)/parts;

    int red, green, blue;
    red = ( red1 + deltared > 255 ? 255 : red1 + deltared);
    green = ( green1 + deltagreen > 255 ? 255 : green1 + deltagreen);
    blue = ( blue1 + deltablue > 255 ? 255 : blue1 + deltablue);

    for (int line = index1 + 1, delta = 1; line < index2; line++)
    {
        row_1 = (QRgb*)imageptrs->ptrimageViirs->scanLine(line);

        red = ( red1 + deltared * delta > 255 ? 255 : red1 + deltared * delta);
        green = ( green1 + deltagreen * delta > 255 ? 255 : green1 + deltagreen * delta);
        blue = ( blue1 + deltablue * delta > 255 ? 255 : blue1 + deltablue * delta);

        row_1[pixelx] = qRgb(red, green, blue );
        delta++;
    }

}

bool SegmentListVIIRS::PixelOK(int pix)
{
    if(pix > 0 && pix < 65528)
        return true;
    else
        return false;
}

