#include <QtConcurrent>
#include "segmentlistviirs.h"
#include "segmentviirs.h"
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

quint16 Min(const quint16 v11, const quint16 v12, const quint16 v21, const quint16 v22)
{
    quint16 Minimum = v11;

    if( Minimum > v12 )
            Minimum = v12;
    if( Minimum > v21 )
            Minimum = v21;
    if( Minimum > v22 )
            Minimum = v22;

    return Minimum;
}

quint16 Min2(const quint16 v11, const quint16 v12)
{
    quint16 Minimum = v11;

    if( Minimum > v12 )
            Minimum = v12;

    return Minimum;
}

quint16 Max(const quint16 v11, const quint16 v12, const quint16 v21, const quint16 v22)
{
    int Maximum = v11;

    if( Maximum < v12 )
            Maximum = v12;
    if( Maximum < v21 )
            Maximum = v21;
    if( Maximum < v22 )
            Maximum = v22;

    return Maximum;
}

quint16 Max2(const quint16 v11, const quint16 v12)
{
    int Maximum = v11;

    if( Maximum < v12 )
            Maximum = v12;

    return Maximum;
}

void doComposeVIIRSImageInThread(SegmentListVIIRS *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{
    t->ComposeVIIRSImageInThread(bandlist, colorlist, invertlist);
}

void SegmentListVIIRS::doReadSegmentInMemoryVIIRS(Segment *t)
{
    t->ReadSegmentInMemory();
}

void SegmentListVIIRS::doComposeSegmentImageVIIRS(Segment *t)
{
    t->ComposeSegmentImage();
}

//void SegmentListVIIRS::doComposeProjection(Segment *t)
//{
//    t->ComposeProjectionConcurrent();
//}

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

bool SegmentListVIIRS::ComposeVIIRSImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{
    qDebug() << QString("SegmentListVIIRS::ComposeVIIRSImage");

    QApplication::setOverrideCursor(( Qt::WaitCursor));
    watcherreadviirs = new QFutureWatcher<void>(this);
    connect(watcherreadviirs, SIGNAL(finished()), this, SLOT(readfinishedviirs()));
    QFuture<void> future = QtConcurrent::run(doComposeVIIRSImageInThread, this, bandlist, colorlist, invertlist);
    watcherreadviirs->setFuture(future);

    return true;

}

//bool SegmentListVIIRS::ComposeVIIRSImageConcurrent(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
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

//    // Reset memory
//    QList<Segment*>::iterator segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        Segment *segm = (Segment *)(*segsel);
//        segm->resetMemory();
//        ++segsel;
//    }
//    segsselected.clear();


//    int startlinenbr = 0;
//    int totalnbroflines = 0;

//    QList<Segment*>::iterator segit = segmentlist.begin();

//    // create QList of selected segments
//    while ( segit != segmentlist.end() )
//    {
//        SegmentVIIRS *segm = (SegmentVIIRS *)(*segit);

//        if (segm->segmentselected)
//        {
//             segsselected.append(segm);
//             totalnbroflines += 768;
//        }
//        ++segit;
//    }


//    segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
//        segm->setBandandColor(bandlist, colorlist, invertlist);
//        segm->setStartLineNbr(startlinenbr);
//        segm->initializeMemory();
//        startlinenbr += segm->NbrOfLines;
//        ++segsel;
//    }

//    emit progressCounter(10);

//    // image pointers always = new QImage()
//    if(imageptrs->ptrimageViirs != NULL)
//    {
//        delete imageptrs->ptrimageViirs;
//        imageptrs->ptrimageViirs = NULL;
//    }

//    imageptrs->ptrimageViirs = new QImage(earthviews, totalnbroflines, QImage::Format_ARGB32);


//    watcherreadviirs = new QFutureWatcher<void>(this);
//    connect(watcherreadviirs, SIGNAL(finished()), this, SLOT(readfinishedviirs()));
//    connect(watcherreadviirs, SIGNAL(progressValueChanged(int)), this, SLOT(progressreadvalue(int)));

//    watcherreadviirs->setFuture(QtConcurrent::map( segsselected.begin(), segsselected.end(), &SegmentListVIIRS::doReadSegmentInMemoryVIIRS));

//    return true;
//}

bool SegmentListVIIRS::ComposeVIIRSImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
{

    qDebug() << "bool SegmentListVIIRS::ComposeVIIRSImageInThread(QList<bool> bandlist, QList<int> colorlist) started";

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

        //QApplication::processEvents();
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

//void SegmentListVIIRS::viirsFinished()
//{
//    QApplication::restoreOverrideCursor();

//    qDebug() << "viirsFinished()";

//    emit segmentlistfinished();
//    emit progressCounter(100);

//}


//bool SegmentListVIIRS::ComposeVIIRSImageSerial(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist)
//{

//    qDebug() << "bool SegmentListVIIRS::ComposeVIIRSImageSerial(QList<bool> bandlist, QList<int> colorlist) started";

//    progressresultready = 0;
//    QApplication::setOverrideCursor( Qt::WaitCursor );

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

//    // Reset memory
//    QList<Segment*>::iterator segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        Segment *segm = (Segment *)(*segsel);
//        segm->resetMemory();
//        ++segsel;
//    }
//    segsselected.clear();


//    int startlinenbr = 0;
//    int totalnbroflines = 0;
//    int totalnbrofsegments = 0;

//    QList<Segment*>::iterator segit = segmentlist.begin();

//    // create QList of selected segments
//    while ( segit != segmentlist.end() )
//    {
//        SegmentVIIRS *segm = (SegmentVIIRS *)(*segit);
//        if (segm->segmentselected)
//        {
//             segsselected.append(segm);
//             totalnbroflines += 768;
//             totalnbrofsegments++;
//        }
//        ++segit;
//    }


//    segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
//        segm->setBandandColor(bandlist, colorlist, invertlist);
//        segm->setStartLineNbr(startlinenbr);
//        segm->initializeMemory();
//        startlinenbr += segm->NbrOfLines;
//        ++segsel;
//    }


//    // image pointers always = new QImage()
//    if(imageptrs->ptrimageViirs != NULL)
//    {
//        delete imageptrs->ptrimageViirs;
//        imageptrs->ptrimageViirs = NULL;
//    }

//    imageptrs->ptrimageViirs = new QImage(earthviews, totalnbroflines, QImage::Format_ARGB32);

//    int deltaprogress = 99 / (totalnbrofsegments*2);
//    int totalprogress = 0;

//    segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
//        segm->ReadSegmentInMemory();

//        totalprogress += deltaprogress;
//        emit progressCounter(totalprogress);

//        QApplication::processEvents();
//        ++segsel;
//    }

//    bool composecolor;

//    segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
//        composecolor = segm->composeColorImage();

//        for(int i = 0; i < (composecolor ? 3 : 1); i++)
//        {
//            if( segm->stat_max_ch[i] > this->stat_max_ch[i])
//                this->stat_max_ch[i] = segm->stat_max_ch[i];
//            if( segm->stat_min_ch[i] < this->stat_min_ch[i])
//                this->stat_min_ch[i] = segm->stat_min_ch[i];
//        }
//        ++segsel;
//    }


//    for(int i = 0; i < (composecolor ? 3 : 1); i++)
//    {
//        imageptrs->stat_max_ch[i] = this->stat_max_ch[i];
//        imageptrs->stat_min_ch[i] = this->stat_min_ch[i];
//    }

//    CalculateLUT();

//    segsel = segsselected.begin();
//    while ( segsel != segsselected.end() )
//    {
//        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
//        segm->ComposeSegmentImage();

//        totalprogress += deltaprogress;
//        emit progressCounter(totalprogress);

//        QApplication::processEvents();
//        ++segsel;
//    }
//    qDebug() << " SegmentListVIIRS::ComposeVIIRSImageSerial Finished !!";

//    QApplication::restoreOverrideCursor();

//    emit segmentlistfinished();
//    emit progressCounter(100);


//    return true;
//}


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

    int lineimage = 0;

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentVIIRS *segmsave;

    while ( segsel != segsselected.end() )
    {
        SegmentVIIRS *segm = (SegmentVIIRS *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearInbetween(segmsave, segm);
        segmsave = segm;
        BilinearInterpolation(segm);
        //printData(segm);
        ++segsel;
        lineimage += segm->NbrOfLines;
    }

    QRgb *scanline;
    QRgb rgbval;
    long count100 = 0;
    long count0 = 0;
    long count250 = 0;
    long count200 = 0;

    for(int j = 0; j < imageptrs->ptrimageProjection->height(); j++)
    {
        scanline = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(j);

        for(int i = 0; i < imageptrs->ptrimageProjection->width(); i++)
        {
            rgbval = scanline[i];
            if(qAlpha(rgbval) == 100)
                count100++;
            else if(qAlpha(rgbval) == 0)
                count0++;
            else if(qAlpha(rgbval) == 200)
                count200++;
            else if(qAlpha(rgbval) == 250)
                count250++;
        }
    }

    qDebug() << QString("Count100 = %1  count0 = %2 count200 = %3 count250 = %4").arg(count100).arg(count0).arg(count200).arg(count250);

}

void SegmentListVIIRS::printData(SegmentVIIRS *segm, int linesfrom, int viewsfrom)
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

inline bool SegmentListVIIRS::PixelOK(int pix)
{
    return (pix > 0 && pix < 65528 ? true : false);
}


void SegmentListVIIRS::BilinearInbetween(SegmentVIIRS *segmfirst, SegmentVIIRS *segmnext)
{
    quint16 x11;
    quint16 y11;
    QRgb rgb11;

    quint16 x12;
    quint16 y12;
    QRgb rgb12;

    quint16 x21;
    quint16 y21;
    QRgb rgb21;

    quint16 x22;
    quint16 y22;
    QRgb rgb22;

    quint16 xc11;
    quint16 yc11;

    quint16 xc12;
    quint16 yc12;

    quint16 xc21;
    quint16 yc21;

    quint16 xc22;
    quint16 yc22;

    quint16 minx;
    quint16 miny;
    quint16 maxx;
    quint16 maxy;

    quint16 anchorX;
    quint16 anchorY;

    int dimx, dimy;

    long counter = 0;
    long counterb = 0;

    QRgb *canvas;

    for (int pixelx = 0; pixelx < earthviews-1; pixelx++)
    {
        x11 = segmfirst->getProjectionX(767, pixelx);
        y11 = segmfirst->getProjectionY(767, pixelx);

        x12 = segmfirst->getProjectionX(767, pixelx+1);
        y12 = segmfirst->getProjectionY(767, pixelx+1);

        x21 = segmnext->getProjectionX(0, pixelx);
        y21 = segmnext->getProjectionY(0, pixelx);

        x22 = segmnext->getProjectionX(0, pixelx+1);
        y22 = segmnext->getProjectionY(0, pixelx+1);

        if(x11 < 65528 && x12 < 65528 && x21 < 65528 && x22 < 65528)
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
                rgb11 = segmfirst->getProjectionValue(767, pixelx);
                rgb12 = segmfirst->getProjectionValue(767, pixelx+1);
                rgb21 = segmnext->getProjectionValue(0, pixelx);
                rgb22 = segmnext->getProjectionValue(0, pixelx+1);

                xc11 = x11 - minx;
                xc12 = x12 - minx;
                xc21 = x21 - minx;
                xc22 = x22 - minx;
                yc11 = y11 - miny;
                yc12 = y12 - miny;
                yc21 = y21 - miny;
                yc22 = y22 - miny;

                canvas = new QRgb[dimx * dimy];
                for(int i = 0 ; i < dimx * dimy ; i++)
                    canvas[i] = qRgba(0,0,0,0);

                canvas[yc11 * dimx + xc11] = rgb11;
                canvas[yc12 * dimx + xc12] = rgb12;
                canvas[yc21 * dimx + xc21] = rgb21;
                canvas[yc22 * dimx + xc22] = rgb22;


                bhm_line(xc11, yc11, xc12, yc12, rgb11, rgb12, canvas, dimx);
                bhm_line(xc12, yc12, xc22, yc22, rgb12, rgb22, canvas, dimx);
                bhm_line(xc22, yc22, xc21, yc21, rgb22, rgb21, canvas, dimx);
                bhm_line(xc21, yc21, xc11, yc11, rgb21, rgb11, canvas, dimx);

                MapInterpolation(canvas, dimx, dimy);
                MapCanvas(canvas, anchorX, anchorY, dimx, dimy);

                delete [] canvas;
                counterb++;
            }
        }
    }


    qDebug() << QString("====> end SegmentListVIIRS::BilinearInbetween(SegmentVIIRS *segmfirst, SegmentVIIRS *segmnext) counter = %1 counterb = %2").arg(counter).arg(counterb);

}

void SegmentListVIIRS::BilinearInterpolation(SegmentVIIRS *segm)
{
    quint16 x11;
    quint16 y11;
    QRgb rgb11;

    quint16 x12;
    quint16 y12;
    QRgb rgb12;

    quint16 x21;
    quint16 y21;
    QRgb rgb21;

    quint16 x22;
    quint16 y22;
    QRgb rgb22;

    quint16 xc11;
    quint16 yc11;

    quint16 xc12;
    quint16 yc12;

    quint16 xc21;
    quint16 yc21;

    quint16 xc22;
    quint16 yc22;

    quint16 minx;
    quint16 miny;
    quint16 maxx;
    quint16 maxy;

    quint16 anchorX;
    quint16 anchorY;

    int dimx, dimy;

    long counter = 0;
    long counterb = 0;

    QRgb *canvas;

    qDebug() << "===> start SegmentListVIIRS::BilinearInterpolation(SegmentVIIRS *segm)";
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

            if(x11 < 65528 && x12 < 65528 && x21 < 65528 && x22 < 65528)
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
                    rgb11 = segm->getProjectionValue(line, pixelx);
                    rgb12 = segm->getProjectionValue(line, pixelx+1);
                    rgb21 = segm->getProjectionValue(line+1, pixelx);
                    rgb22 = segm->getProjectionValue(line+1, pixelx+1);

                    xc11 = x11 - minx;
                    xc12 = x12 - minx;
                    xc21 = x21 - minx;
                    xc22 = x22 - minx;
                    yc11 = y11 - miny;
                    yc12 = y12 - miny;
                    yc21 = y21 - miny;
                    yc22 = y22 - miny;

                    canvas = new QRgb[dimx * dimy];
                    for(int i = 0 ; i < dimx * dimy ; i++)
                        canvas[i] = qRgba(0,0,0,0);

                    canvas[yc11 * dimx + xc11] = rgb11;
                    canvas[yc12 * dimx + xc12] = rgb12;
                    canvas[yc21 * dimx + xc21] = rgb21;
                    canvas[yc22 * dimx + xc22] = rgb22;


//                    for ( int i = 0; i < dimy; i++ )
//                    {
//                       for ( int j = 0; j < dimx; j++ )
//                       {
//                          std::cout << std::setw(6) << qRed(canvas[i * dimx + j]) << " ";
//                       }
//                       std::cout << std::endl;
//                    }

//                    std::cout << "....................................... line " << line << " pixelx = " << pixelx << std::endl;

                    bhm_line(xc11, yc11, xc12, yc12, rgb11, rgb12, canvas, dimx);
                    bhm_line(xc12, yc12, xc22, yc22, rgb12, rgb22, canvas, dimx);
                    bhm_line(xc22, yc22, xc21, yc21, rgb22, rgb21, canvas, dimx);
                    bhm_line(xc21, yc21, xc11, yc11, rgb21, rgb11, canvas, dimx);

//                    for ( int i = 0; i < dimy; i++ )
//                    {
//                       for ( int j = 0; j < dimx; j++ )
//                       {
//                          std::cout << std::setw(6) << qRed(canvas[i * dimx + j]) << " ";
//                       }
//                       std::cout << std::endl;
//                    }

//                    std::cout << "-------------------------------------- line " << line << " pixelx = " << pixelx << std::endl;

                    MapInterpolation(canvas, dimx, dimy);
                    MapCanvas(canvas, anchorX, anchorY, dimx, dimy);

//                    for ( int i = 0; i < dimy; i++ )
//                    {
//                       for ( int j = 0; j < dimx; j++ )
//                       {
//                          std::cout << std::setw(6) << qRed(canvas[i * dimx + j]) << " ";
//                       }
//                       std::cout << std::endl;
//                    }

//                    std::cout << "================================= line " << line << " pixelx = " << pixelx << std::endl;

                    delete [] canvas;
                    counterb++;
                }
            }
        }
    }

    qDebug() << QString("====> end SegmentListVIIRS::BilinearInterpolation(SegmentVIIRS *segm) counter = %1 countern = %2").arg(counter).arg(counterb);

}


bool SegmentListVIIRS::bhm_line(int x1, int y1, int x2, int y2, QRgb rgb1, QRgb rgb2, QRgb *canvas, int dimx)
{
    int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;
    int deltared, deltagreen, deltablue;
    dx=x2-x1;
    dy=y2-y1;
    dx1=fabs(dx);
    dy1=fabs(dy);
    px=2*dy1-dx1;
    py=2*dx1-dy1;


    if(dy1<=dx1)
    {
        if(dx1==0)
            return false;

        if(dx>=0)
        {
            x=x1;
            y=y1;
            xe=x2;
            deltared = (qRed(rgb2) - qRed(rgb1))/ dx1 ;
            deltagreen = (qGreen(rgb2) - qGreen(rgb1))/ dx1 ;
            deltablue = (qBlue(rgb2) - qBlue(rgb1))/ dx1 ;
//            canvas[y * yy + x] = val1;

        }
        else
        {
            x=x2;
            y=y2;
            xe=x1;
            deltared = (qRed(rgb1) - qRed(rgb2))/ dx1 ;
            deltagreen = (qGreen(rgb1) - qGreen(rgb2))/ dx1 ;
            deltablue = (qBlue(rgb1) - qBlue(rgb2))/ dx1 ;
//            canvas[y * yy + x] = val2;

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
                rgb1 = qRgb(qRed(rgb1) + deltared, qGreen(rgb1) + deltagreen, qBlue(rgb1) + deltablue );
                if( x != xe)
                    canvas[y * dimx + x] = rgb1;
            }
            else
            {
                rgb2 = qRgb(qRed(rgb2) + deltared, qGreen(rgb2) + deltagreen, qBlue(rgb2) + deltablue );
                if( x != xe)
                    canvas[y * dimx + x] = rgb2;
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
            deltared = (qRed(rgb2) - qRed(rgb1))/ dy1 ;
            deltagreen = (qGreen(rgb2) - qGreen(rgb1))/ dy1 ;
            deltablue = (qBlue(rgb2) - qBlue(rgb1))/ dy1 ;

//            canvas[y * yy + x] = val1;
        }
        else
        {
            x=x2;
            y=y2;
            ye=y1;
            deltared = (qRed(rgb1) - qRed(rgb2))/ dy1 ;
            deltagreen = (qGreen(rgb1) - qGreen(rgb2))/ dy1 ;
            deltablue = (qBlue(rgb1) - qBlue(rgb2))/ dy1 ;

//            canvas[y * yy + x] = val2;
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
                rgb1 = qRgb(qRed(rgb1) + deltared, qGreen(rgb1) + deltagreen, qBlue(rgb1) + deltablue );
                if( y != ye)
                    canvas[y * dimx + x] = rgb1;
            }
            else
            {
                rgb2 = qRgb(qRed(rgb2) + deltared, qGreen(rgb2) + deltagreen, qBlue(rgb2) + deltablue );
                if( y != ye)
                    canvas[y * dimx + x] = rgb2;
            }
        }
    }

    return true;
}



void SegmentListVIIRS::MapInterpolation(QRgb *canvas, quint16 dimx, quint16 dimy)
{

    for(int h = 0; h < dimy; h++ )
    {
        QRgb start = qRgba(0,0,0,0);
        QRgb end = qRgba(0,0,0,0);
        bool hole = false;
        bool first = false;
        bool last = false;
        int holecount = 0;

        for(int w = 0; w < dimx; w++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 255 && hole == false)
            {
                start = rgb;
                first = true;
            }
            else if(rgbalpha == 255 && hole == true)
            {
                end = rgb;
                last = true;
                break;
            }
            else if(rgbalpha == 0 && first == true)
            {
                hole = true;
                holecount++;
                canvas[h * dimx + w] = qRgba(0,0,0,100);
            }
        }

        if(holecount == 0)
            continue;
        if(first == false || last == false)
        {
            for(int w = 0; w < dimx; w++)
            {
                QRgb rgb = canvas[h * dimx + w];
                if(qAlpha(rgb) == 100)
                    canvas[h * dimx + w] = qRgba(0,0,0,0);
            }
            continue;
        }



        int deltared = (qRed(end) - qRed(start)) / (holecount+1);
        int deltagreen = (qGreen(end) - qGreen(start)) / (holecount+1);
        int deltablue = (qBlue(end) - qBlue(start)) / (holecount+1);

        int red = qRed(start);
        int green = qGreen(start);
        int blue = qBlue(start);

        for(int w = 0; w < dimx; w++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 100)
            {
                red += deltared;
                green += deltagreen;
                blue += deltablue;
                canvas[h * dimx + w] = qRgba(red, green, blue, 100);
            }
        }
    }


    for(int w = 0; w < dimx; w++)
    {
        QRgb start = qRgba(0,0,0,0);
        QRgb end = qRgba(0,0,0,0);

        int hcount = 0;

        bool startok = false;

        for(int h = 0; h < dimy; h++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 255 && !startok)
            {
                start = rgb;
            }
            else
            {
                if(rgbalpha == 255)
                {
                    end = rgb;
                    break;
                }
                else if(rgbalpha == 100)
                {
                    startok = true;
                    hcount++;
                }

            }
        }

        if(hcount == 0)
            continue;

        int redstart = qRed(start);
        int greenstart = qGreen(start);
        int bluestart = qBlue(start);

        int deltared = (qRed(end) - qRed(start)) / (hcount+1);
        int deltagreen = (qGreen(end) - qGreen(start)) / (hcount+1);
        int deltablue = (qBlue(end) - qBlue(start)) / (hcount+1);


        for(int h = 0; h < dimy; h++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 100)
            {
                redstart += deltared;
                greenstart += deltagreen;
                bluestart += deltablue;
                int redtotal = (qRed(canvas[h * dimx + w]) + redstart)/2;
                int greentotal = (qGreen(canvas[h * dimx + w]) + greenstart)/2;
                int bluetotal = (qBlue(canvas[h * dimx + w]) + bluestart)/2;

                canvas[h * dimx + w] = qRgba(redtotal, greentotal, bluetotal, 255);
            }
        }
    }


}

void SegmentListVIIRS::MapCanvas(QRgb *canvas, quint16 anchorX, quint16 anchorY, quint16 dimx, quint16 dimy)
{

    for(int h = 0; h < dimy; h++ )
    {
        for(int w = 0; w < dimx; w++)
        {
            QRgb rgb = canvas[h * dimx + w];
            if(qAlpha(rgb) == 255)
                imageptrs->ptrimageProjection->setPixel(anchorX + w, anchorY + h, rgb);
        }
    }


}

