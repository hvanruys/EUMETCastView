#include "segmentlistolci.h"
#include "segmentolci.h"
#include "segmentimage.h"
#include "options.h"
#include <QtConcurrent>

extern Options opts;
extern SegmentImage *imageptrs;


void doComposeOLCIImageInThread(SegmentListOLCI *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool readnew)
{
    t->ComposeOLCIImageInThread(bandlist, colorlist, invertlist, readnew);
}


SegmentListOLCI::SegmentListOLCI(eSegmentType type, SatelliteList *satl, QObject *parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListOLCI");

    satlist = satl;
    seglisttype = type;


}

bool SegmentListOLCI::ComposeOLCIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool untarfiles)
{
    qDebug() << QString("SegmentListOLCI::ComposeOLCIImage");

    this->bandlist = bandlist;
    this->colorlist = colorlist;
    this->inverselist = invertlist;

    QApplication::setOverrideCursor(( Qt::WaitCursor));
    watcherolci = new QFutureWatcher<void>(this);
    connect(watcherolci, SIGNAL(finished()), this, SLOT(finishedolci()));

    QFuture<void> future;
    future = QtConcurrent::run(doComposeOLCIImageInThread, this, bandlist, colorlist, invertlist, untarfiles);
    watcherolci->setFuture(future);

    return true;

}

bool SegmentListOLCI::ComposeOLCIImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool untarfiles)
{

    qDebug() << "bool SegmentListOLCIefr::ComposeOLCIImageInThread() started";

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

    if(untarfiles)
    {
        segsel = segsselected.begin();
        while ( segsel != segsselected.end() )
        {
            SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
            segm->UntarSegmentToTemp();
            totalprogress += deltaprogress;
            emit progressCounter(totalprogress);
            ++segsel;
        }
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        segm->setBandandColor(bandlist, colorlist, invertlist);
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
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        segm->setStartLineNbr(startlinenbr);
        startlinenbr += segm->NbrOfLines;
        totalnbroflines += segm->NbrOfLines;
        ++segsel;
    }

    // image pointers always = new QImage()
    if(imageptrs->ptrimageOLCI != NULL)
    {
        delete imageptrs->ptrimageOLCI;
        imageptrs->ptrimageOLCI = NULL;
    }

    imageptrs->ptrimageOLCI = new QImage(this->earth_views_per_scanline, totalnbroflines, QImage::Format_ARGB32);
    qDebug() << QString("ptrimageOLCI created %1 x %2").arg(this->earth_views_per_scanline).arg(totalnbroflines);


    bool composecolor;

    long cnt_active_pixels = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
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
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        segm->ComposeSegmentImage();
        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        QApplication::processEvents();
        ++segsel;
    }

    qDebug() << " SegmentListOLCI::ComposeOLCIImageInThread Finished !!";


    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
    emit progressCounter(100);
    return true;
}

void SegmentListOLCI::finishedolci()
{

    qDebug() << "=============>SegmentListOLCIefr::finishedolci()";
    emit progressCounter(100);
    opts.texture_changed = true;
    delete watcherolci;
    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
}

void SegmentListOLCI::progressreadvalue(int progress)
{
    int totalcount = segsselected.count();
    this->progressresultready += 100 / totalcount;

    emit progressCounter(this->progressresultready);

    qDebug() << QString("SegmentListOLCI::progressreadvalue( %1 )").arg(progress);
}

void SegmentListOLCI::CalculateLUT()
{
    qDebug() << "start SegmentListOLCI::CalculateLUT()";
    int earth_views = this->earth_views_per_scanline;
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
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
                    int pixel = *(segm->ptrbaOLCI[k].data() + line * earth_views + pixelx);
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

void SegmentListOLCI::ComposeGVProjection(int inputchannel)
{

    qDebug() << "SegmentListOLCIefr::ComposeGVProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentGVProjection(inputchannel);
        emit segmentprojectionfinished(false);
        ++segit;
    }
}

void SegmentListOLCI::ComposeLCCProjection(int inputchannel)
{

    qDebug() << "SegmentListOLCIefr::ComposeLCCProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentLCCProjection(inputchannel);
        emit segmentprojectionfinished(false);
        ++segit;
    }
}

void SegmentListOLCI::ComposeSGProjection(int inputchannel)
{

    qDebug() << "SegmentListOLCIefr::ComposeSGProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentSGProjection(inputchannel);
        emit segmentprojectionfinished(false);
        ++segit;
    }
}

void SegmentListOLCI::SmoothOLCIImage(bool combine)
{

    qDebug() << "start SegmentListOLCI::SmoothOLCIImage()";

    int lineimage = 0;

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentOLCI *segmsave;

    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments(segmsave, segm, combine);
        segmsave = segm;
        BilinearInterpolation(segm, combine);
        ++segsel;
        lineimage += segm->NbrOfLines;
    }
}

void SegmentListOLCI::ShowWinvec(QPainter *painter, float distance, const QMatrix4x4 modelview)
{

    QList<Segment*>::iterator segit = segmentlist.begin();
    QVector2D winvecend1, winvecend2, winvecend3, winvecend4;

    QVector3D vecZ = modelview.row(2).toVector3D();

//    static GLfloat mat[16];
//    const float *data = modelview.constData();
//    for (int index = 0; index < 16; ++index)
//         mat[index] = data[index];

    //modelview.inverted( &ok );

    while ( segit != segmentlist.end() )
    {
        if( (*segit)->segmentshow)
        {
            for(int i = 0; i < (*segit)->winvectorfirst.length()-1; i++)
            {
                winvecend1.setX((*segit)->winvectorfirst.at(i).x()); winvecend1.setY((*segit)->winvectorfirst.at(i).y());
                winvecend2.setX((*segit)->winvectorfirst.at(i+1).x()); winvecend2.setY((*segit)->winvectorfirst.at(i+1).y());
                winvecend3.setX((*segit)->winvectorlast.at(i).x()); winvecend3.setY((*segit)->winvectorlast.at(i).y());
                winvecend4.setX((*segit)->winvectorlast.at(i+1).x()); winvecend4.setY((*segit)->winvectorlast.at(i+1).y());

                //qDebug() << "winvec1 x = " << winvec1.x() << "  y = " << winvec1.y();


                //     first                                          last
                //  winvecend1 ------------------------------------ winvecend3
                //      | p01                                           | p03
                //      |                                               |
                //      |                                               |
                //      |                                               |
                //      |                                               |
                //      |                                               |
                //      | p02                                           | p04
                //  winvecend2 ------------------------------------ winvecend4
                //


                qreal angle = ArcCos(QVector3D::dotProduct( vecZ, (*segit)->vecvector.at(i)));

                if (angle < PI/2) //  + (asin(1/distance)))
                {

                    painter->drawLine((int)winvecend1.x(), (painter->device())->height() - (int)winvecend1.y(), (int)winvecend3.x(), (painter->device())->height() - (int)winvecend3.y() );

                    painter->drawLine((int)winvecend1.x(), (painter->device())->height() - (int)winvecend1.y(), (int)winvecend2.x(), (painter->device())->height() - (int)winvecend2.y() );
                    painter->drawLine((int)winvecend3.x(), (painter->device())->height() - (int)winvecend3.y(), (int)winvecend4.x(), (painter->device())->height() - (int)winvecend4.y() );

                    //                painter->drawLine((int)winvec1.x(), (painter->device())->height() - (int)winvec1.y(), (int)winvecend1.x(), (painter->device())->height() - (int)winvecend1.y() );
                    //                painter->drawLine((int)winvec2.x(), (painter->device())->height() - (int)winvec2.y(), (int)winvecend2.x(), (painter->device())->height() - (int)winvecend2.y() );
                    //                painter->drawLine((int)winvec1.x(), (painter->device())->height() - (int)winvec1.y(), (int)winvecend3.x(), (painter->device())->height() - (int)winvecend3.y() );
                    //                painter->drawLine((int)winvec2.x(), (painter->device())->height() - (int)winvec2.y(), (int)winvecend4.x(), (painter->device())->height() - (int)winvecend4.y() );

                }
            }
        }
        ++segit;
    }
}

bool SegmentListOLCI::TestForSegmentGLerr(int x, int realy, float distance, const QMatrix4x4 &m, bool showallsegments, QString &segmentname)
{

    bool isselected = false;

    qDebug() << QString("Nbr of segments = %1").arg(segmentlist.count());

    QList<Segment*>::iterator segit = segmentlist.begin();
    QVector2D winvec1, winvec2, winvecend1, winvecend2, winvecend3, winvecend4;

    QVector3D vecZ = m.row(2).toVector3D();

    segmentname = "";

    while ( segit != segmentlist.end() )
    {
        if(showallsegments ? true : (*segit)->segmentshow)
        {
            for(int i = 0; i < (*segit)->winvectorfirst.length()-1; i++)
            {
                winvecend1.setX((*segit)->winvectorfirst.at(i).x()); winvecend1.setY((*segit)->winvectorfirst.at(i).y());
                winvecend2.setX((*segit)->winvectorfirst.at(i+1).x()); winvecend2.setY((*segit)->winvectorfirst.at(i+1).y());
                winvecend3.setX((*segit)->winvectorlast.at(i).x()); winvecend3.setY((*segit)->winvectorlast.at(i).y());
                winvecend4.setX((*segit)->winvectorlast.at(i+1).x()); winvecend4.setY((*segit)->winvectorlast.at(i+1).y());

                //     first                                          last
                //  winvecend1 ------------------------------------ winvecend3
                //      | p01                                           | p03
                //      |                                               |
                //      |                                               |
                //      |                                               |
                //      |                                               |
                //      |                                               |
                //      | p02                                           | p04
                //  winvecend2 ------------------------------------ winvecend4
                //


                qreal angle = ArcCos(QVector3D::dotProduct( vecZ, (*segit)->vecvector.at(i)));
                //qDebug() << QString("angle = %1").arg(angle * 180.0 / PI);

                if (angle < PI/2 + (asin(1/distance)))
                {


                    struct point p01;
                    p01.x = (int)winvecend1.x();
                    p01.y = (int)winvecend1.y();

                    struct point p02;
                    p02.x = (int)winvecend2.x();
                    p02.y = (int)winvecend2.y();

                    struct point p03;
                    p03.x = (int)winvecend3.x();
                    p03.y = (int)winvecend3.y();

                    struct point p04;
                    p04.x = (int)winvecend4.x();
                    p04.y = (int)winvecend4.y();



                    QPoint points[4] = {
                        QPoint(p01.x, p01.y),
                        QPoint(p02.x, p02.y),
                        QPoint(p04.x, p04.y),
                        QPoint(p03.x, p03.y)
                    };


                    int result = pnpoly( 4, points, x, realy);

                    if (result)
                    {
                        if((*segit)->ToggleSelected())
                        {
                            qDebug() << QString("segment selected is = %1").arg((*segit)->fileInfo.fileName());
                            isselected = true;
                            segmentname = (*segit)->fileInfo.fileName();
                            qApp->processEvents();
                        }
                        break;
                    }
                }

            }
        }
        ++segit;
    }
    return isselected;
}

