#include <QtConcurrent/QtConcurrent>
#include <QMatrix4x4>

//#include "segmentmetop.h"
//#include "segmentimage.h"
#include "segmentlist.h"
#include "avhrrsatellite.h"
#include "options.h"

extern Options opts;
extern SegmentImage *imageptrs;


void SegmentList::doComposeGVProjection(Segment *t)
{
    t->ComposeSegmentGVProjection(0);
}

void SegmentList::doReadSegmentInMemory(Segment *t)
{
    t->ReadSegmentInMemory();
}

void SegmentList::doComposeSegmentImage(Segment *t)
{
    t->ComposeSegmentImage();
}

SegmentList::SegmentList(QObject *parent) :
    QObject(parent)
{
    indexfirstvisible = 0;
    indexlastvisible = 0;

    for(int i = 0 ; i < 5; i++)
    {
        stat_max_ch[i] = 0;
        stat_min_ch[i] = 9999999;
    }

    TotalSegmentsInDirectory = 0;
}

int SegmentList::NbrOfSegments()
{
    int nbr = 0;

    QList<Segment*>::iterator segit = segmentlist.begin();

    while ( segit != segmentlist.end() )
    {
       nbr++;
       ++segit;
    }

    return nbr;

}

bool SegmentList::imageMemory()
{
    bool bMemory = false;

    QList<Segment*>::iterator segit = segmentlist.begin();

    while ( segit != segmentlist.end() )
    {
        bMemory = (*segit)->bImageMemory;
        if(bMemory)
            return true;
        ++segit;
    }

    return false;

}

int SegmentList::NbrOfSegmentsSelected()
{
    int nbr = 0;

    QList<Segment*>::iterator segit = segmentlist.begin();

    while ( segit != segmentlist.end() )
    {
        if( (*segit)->segmentselected)
            nbr++;
        ++segit;
    }

    return nbr;

}


int SegmentList::NbrOfSegmentsShown()
{
    int nbr = 0;

    QList<Segment*>::iterator segit = segmentlist.begin();

    while ( segit != segmentlist.end() )
    {
        if( (*segit)->segmentshow)
            nbr++;
        ++segit;
    }

    return nbr;

}


int SegmentList::NbrOfSegmentLinesSelected()
{
    int nbr = 0;

    QList<Segment*>::iterator segit = segsselected.begin();

    while ( segit != segsselected.end() )
    {
        nbr += (*segit)->GetNbrOfLines();
        ++segit;
    }

    return nbr;

}

int SegmentList::NbrOfEartviewsPerScanline()
{
    int nbr_earth_views = 0;

    QList<Segment*>::iterator segit = segsselected.begin();

    while ( segit != segsselected.end() )
    {
        nbr_earth_views = (*segit)->earth_views_per_scanline;
        break;
    }

    return nbr_earth_views;
}

void SegmentList::GetFirstLastVisible( double *first_julian,  double *last_julian)
{
    if( segmentlist.count() > 0)
    {
        *first_julian = segmentlist.at(indexfirstvisible)->julian_sensing_start;
        *last_julian = segmentlist.at(indexlastvisible)->julian_sensing_start;
    }
    else
    {
        *first_julian = 0;
        *last_julian = 0;
    }
}

void SegmentList::GetFirstLastVisibleFilename( QString *first_filename,  QString *last_filename)
{

    if( segmentlist.count() > 0)
    {
        *first_filename = segmentlist.at(indexfirstvisible)->fileInfo.fileName();
        *last_filename = segmentlist.at(indexlastvisible)->fileInfo.fileName();
    }
    else
    {
        *first_filename = QString("");
        *last_filename = QString("");
    }
}



void SegmentList::GetFirstLastVisible( QDateTime *first_date,  QDateTime *last_date)
{
    if( segmentlist.count() > 0)
    {
        *first_date = segmentlist.at(indexfirstvisible)->qdatetime_start;
        *last_date = segmentlist.at(indexlastvisible)->qdatetime_start;
    }
    else
    {
        *first_date = QDateTime();
        *last_date = QDateTime();
    }
}

void SegmentList::ShowSegment(int value)
{

    int cnt = 0;

    QList<Segment*>::iterator segit = segmentlist.begin();
    int nos = this->NbrOfSegments();
    int novs = this->GetNbrOfVisibleSegments();

    int viseg = (nos < novs ? nos : novs);

    if (value > nos - viseg)
        value = nos - viseg;

    while ( segit != segmentlist.end() )
    {
        if (cnt == value)
            this->SetIndexFirstVisible(cnt);
        if (cnt >= value && cnt < value + viseg)
        {
            (*segit)->segmentshow = true;
            this->SetIndexLastVisible(cnt);
        }
        else
        {
            (*segit)->segmentshow = false;
        }
        ++segit;
        cnt++;
    }
}

void SegmentList::CalculateSunPosition(double first_julian, double last_julian, QVector3D *sunPosition)
{
    geodetic_t sungeo;

    //qDebug() << "in showsunposition";

    /* Zero vector for initializations */
    vector_t zero_vector = {0,0,0,0};

    /* Solar ECI position vector  */
    vector_t solar_vector = zero_vector;

    /* Calculate solar position and satellite eclipse depth */
    Calculate_Solar_Position(first_julian, &solar_vector);

    /* Calculate Sun's Lat North, Lon East and Alt. */
    Calculate_LatLonAlt(first_julian, &solar_vector, &sungeo);

    LonLat2PointRad(sungeo.lat, sungeo.lon, sunPosition, 100.0f);


}

void SegmentList::SetNbrOfVisibleSegments(int val)
{
    // qDebug() << QString("nbrofvisiblesegments = %1").arg(val);
    nbrofvisiblesegments = val;
}

int SegmentList::GetNbrOfVisibleSegments()
{
    return nbrofvisiblesegments;
}

bool SegmentList::TestForSegment(double *deg_lon, double *deg_lat, bool leftbuttondown, bool showallsegments)
{
    double lon, lat, difflon, difflat;
    bool ret = false;
    QString filn;

    QList<Segment*>::iterator segit = segmentlist.begin();

    while ( segit != segmentlist.end() )
    {
        lon = (*segit)->lon_start_deg;
        lat = (*segit)->lat_start_deg;
        if (lon > 180)
            lon = lon - 360;

        difflon = fabs( lon - *deg_lon );
        difflat = fabs( lat - *deg_lat);

        //qDebug() << QString("%1 %2 %3").arg( (*segit).fileInfo.fileName()).arg( (*segit).lon_start_deg).arg( (*segit).lat_start_deg);
        //qDebug() << QString("difflon %1 difflat %2").arg( difflon).arg( difflat);

        if ( (difflon <  2.0) && (difflat < (fabs(*deg_lat) > 70.0 ? 4.0 : 2.0)) && (showallsegments ? true : (*segit)->segmentshow ))
        {
            filn = (*segit)->fileInfo.fileName();
            if (leftbuttondown)
            {
                if((*segit)->ToggleSelected())
                {
                    qDebug() << QString("file selected is = %1").arg(filn);
                    ret = true;
                }

            }
            break;
        }

        ++segit;
    }

    qDebug() << QString("ret = %1 filename = %2 deg_lon = %3 deg_lat = %4").arg(ret).arg(filn).arg(*deg_lon).arg(*deg_lat);
    qDebug() << QString("lon = %1 lat = %2 %3").arg( lon, 10 ).arg( lat, 10 ).arg( ret );
    //return QString("%1").arg( segnbr );
    return ret;
}

bool SegmentList::TestForSegmentGL(int x, int realy, float distance, const QMatrix4x4 &m, bool showallsegments, QString &segmentname)
{

    bool isselected = false;

    QList<Segment*>::iterator segit = segmentlist.begin();
    QVector2D winvec1, winvec2, winvecend1, winvecend2, winvecend3, winvecend4;

    QVector3D vecZ = m.row(2).toVector3D();

    segmentname = "";

    while ( segit != segmentlist.end() )
    {
        if(showallsegments ? true : (*segit)->segmentshow )
        {
            winvec1.setX((*segit)->winvec1.x()); winvec1.setY((*segit)->winvec1.y());
            winvec2.setX((*segit)->winvec2.x()); winvec2.setY((*segit)->winvec2.y());
            winvecend1.setX((*segit)->winvecend1.x()); winvecend1.setY((*segit)->winvecend1.y());
            winvecend2.setX((*segit)->winvecend2.x()); winvecend2.setY((*segit)->winvecend2.y());
            winvecend3.setX((*segit)->winvecend3.x()); winvecend3.setY((*segit)->winvecend3.y());
            winvecend4.setX((*segit)->winvecend4.x()); winvecend4.setY((*segit)->winvecend4.y());

            //                           winvec1
            //  winvecend1 ------------------------------------ winvecend3
            //      | p03                   | p00               | p05
            //      |                       |                   |
            //      |                       |                   |
            //      |                       |                   |
            //      |                       |                   |
            //      |                       |                   |
            //      | p02                   | p01               | p04
            //  winvecend2 ------------------------------------ winvecend4
            //                           winvec2


            qreal angle = ArcCos(QVector3D::dotProduct( vecZ, (*segit)->vec1));
            //qDebug() << QString("angle = %1").arg(angle * 180.0 / PI);

            if (angle < PI/2 + (asin(1/distance)))
            {

                struct point p00;
                p00.x = (int)winvec1.x();
                p00.y = (int)winvec1.y();

                struct point p01;
                p01.x = (int)winvec2.x();
                p01.y = (int)winvec2.y();

                struct point p02;
                p02.x = (int)winvecend2.x();
                p02.y = (int)winvecend2.y();

                struct point p03;
                p03.x = (int)winvecend1.x();
                p03.y = (int)winvecend1.y();


                struct point p04;
                p04.x = (int)winvecend4.x();
                p04.y = (int)winvecend4.y();

                struct point p05;
                p05.x = (int)winvecend3.x();
                p05.y = (int)winvecend3.y();


                QPoint points1[4] = {
                    QPoint(p00.x, p00.y),
                    QPoint(p01.x, p01.y),
                    QPoint(p02.x, p02.y),
                    QPoint(p03.x, p03.y)
                 };

                QPoint points2[4] = {
                    QPoint(p00.x, p00.y),
                    QPoint(p01.x, p01.y),
                    QPoint(p04.x, p04.y),
                    QPoint(p05.x, p05.y)
                 };

               int result1 = pnpoly( 4, points1, x, realy);
               int result2 = pnpoly( 4, points2, x, realy);

                if (result1 || result2)
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
        ++segit;
    }
    return isselected;
}

void SegmentList::ShowWinvec(QPainter *painter, float distance, const QMatrix4x4 modelview)
{

    QList<Segment*>::iterator segit = segmentlist.begin();
    QVector2D winvec1, winvec2, winvecend1, winvecend2, winvecend3, winvecend4;

    QVector3D vecZ = modelview.row(2).toVector3D();


//    GLint viewport[4];
//    glGetIntegerv (GL_VIEWPORT, viewport);


//    static GLfloat mat[16];
//    const float *data = modelview.constData();
//    for (int index = 0; index < 16; ++index)
//         mat[index] = data[index];

    //modelview.inverted( &ok );

    while ( segit != segmentlist.end() )
    {
        if( (*segit)->segmentshow)
        {
            winvec1.setX((*segit)->winvec1.x()); winvec1.setY((*segit)->winvec1.y());
            winvec2.setX((*segit)->winvec2.x()); winvec2.setY((*segit)->winvec2.y());
            winvecend1.setX((*segit)->winvecend1.x()); winvecend1.setY((*segit)->winvecend1.y());
            winvecend2.setX((*segit)->winvecend2.x()); winvecend2.setY((*segit)->winvecend2.y());
            winvecend3.setX((*segit)->winvecend3.x()); winvecend3.setY((*segit)->winvecend3.y());
            winvecend4.setX((*segit)->winvecend4.x()); winvecend4.setY((*segit)->winvecend4.y());

            //qDebug() << "winvec1 x = " << winvec1.x() << "  y = " << winvec1.y();


            //                           winvec1
            //  winvecend1 ------------------------------------ winvecend3
            //      | p03                   | p00               | p05
            //      |                       |                   |
            //      |                       |                   |
            //      |                       |                   |
            //      |                       |                   |
            //      |                       |                   |
            //      | p02                   | p01               | p04
            //  winvecend2 ------------------------------------ winvecend4
            //                           winvec2


            //qreal angle = ArcCos(QVector3D::dotProduct( vecZ, (*segit)->vec1));

            //if (angle < PI/2 + (asin(1/distance)))
            {

                painter->drawLine((int)winvec1.x(), (painter->device())->height() - (int)winvec1.y(), (int)winvec2.x(), (painter->device())->height() - (int)winvec2.y() );
                painter->drawLine((int)winvecend1.x(), (painter->device())->height() - (int)winvecend1.y(), (int)winvecend2.x(), (painter->device())->height() - (int)winvecend2.y() );
                painter->drawLine((int)winvecend3.x(), (painter->device())->height() - (int)winvecend3.y(), (int)winvecend4.x(), (painter->device())->height() - (int)winvecend4.y() );

                painter->drawLine((int)winvec1.x(), (painter->device())->height() - (int)winvec1.y(), (int)winvecend1.x(), (painter->device())->height() - (int)winvecend1.y() );
                painter->drawLine((int)winvec2.x(), (painter->device())->height() - (int)winvec2.y(), (int)winvecend2.x(), (painter->device())->height() - (int)winvecend2.y() );
                painter->drawLine((int)winvec1.x(), (painter->device())->height() - (int)winvec1.y(), (int)winvecend3.x(), (painter->device())->height() - (int)winvecend3.y() );
                painter->drawLine((int)winvec2.x(), (painter->device())->height() - (int)winvec2.y(), (int)winvecend4.x(), (painter->device())->height() - (int)winvecend4.y() );

            }
        }
        ++segit;
    }
}

bool SegmentList::ComposeImage(double gamma_ch[])
{
    QApplication::setOverrideCursor( Qt::WaitCursor );  //restore in finished()

    emit progressCounter(0);

    progressresultready = 0;

    for(int i = 0; i < 5; i++)
        factor_ch[i] = 1023 / pow(1023, gamma_ch[i]);
    qDebug() << " SegmentList::ComposeImage";


    for (int i=0; i < 5; i++)
    {
        for (int j=0; j < 1024; j++)
        {
            imageptrs->segment_stats_ch[i][j] = 0;
            imageptrs->lut_ch[i][j] = 0;
        }
    }

    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);
        qDebug() << "resetting memory for " << segm->fileInfo.baseName();
        segm->resetMemory();
        ++segsel;
    }

    segsselected.clear();

    int startlinenbr = 0;

    QList<Segment*>::iterator segit = segmentlist.begin();
    while ( segit != segmentlist.end() )
    {
        if ((*segit)->segmentselected)
        {
            qDebug() << "in composeimage append " << (*segit)->fileInfo.baseName();
            segsselected.append(*segit);
        }
        ++segit;
    }

#if 0
    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);
        segm->NbrOfLines = segm->ReadNbrOfLines();
        qDebug() << QString("SegmentList::ReadNbrOfLines NbrOfLines = %1").arg(segm->NbrOfLines);
        ++segsel;
    }
#endif

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);
        qDebug() << QString("SegmentList::ComposeImage NbrOfLines = %1 startline = %2").arg(segm->NbrOfLines).arg(startlinenbr);
        segm->setStartLineNbr(startlinenbr);
        segm->initializeMemory();
        startlinenbr += segm->NbrOfLines;
        ++segsel;
    }

    emit progressCounter(10);


//    watcherread = new QFutureWatcher<Segment*>(this);
//    connect(watcherread, SIGNAL(resultReadyAt(int)), SLOT(resultisready(int)));
//    connect(watcherread, SIGNAL(finished()), SLOT(readfinished()));

//    watcherread->setFuture(QtConcurrent::mapped( segsselected.begin(), segsselected.end(), &SegmentList::doReadSegmentInMemory));

    watcherread = new QFutureWatcher<void>(this);
    connect(watcherread, SIGNAL(resultReadyAt(int)), SLOT(resultisready(int)));
    connect(watcherread, SIGNAL(finished()), SLOT(readfinished()));

    watcherread->setFuture(QtConcurrent::map( segsselected.begin(), segsselected.end(), &SegmentList::doReadSegmentInMemory));

    return true;
}

void SegmentList::readfinished()
{

    const float MAX_VALUE = 1023.0f;         // max value in 10-bit image
    int TotalLines = this->NbrOfSegmentLinesSelected();
    int nbrearthviews = NbrOfEartviewsPerScanline();
    qDebug() << QString("SegmentList::readfinished() TotalLines = %1 nbrearthviews = %2").arg(QString::number(TotalLines)).arg(nbrearthviews);

    scale = MAX_VALUE / (TotalLines * nbrearthviews);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    imageptrs->InitializeAVHRRImages( nbrearthviews, TotalLines );


    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);

        for( int k = 0; k < 5; k++)
        {

            if (segm->stat_min_ch[k] < stat_min_ch[k] )
                stat_min_ch[k] = segm->stat_min_ch[k];
            if (segm->stat_max_ch[k] > stat_max_ch[k] )
                stat_max_ch[k] = segm->stat_max_ch[k];
        }
        ++segsel;
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);

        for( int k = 0; k < 5; k++)
        {
            segm->list_stat_min_ch[k] = stat_min_ch[k];
            segm->list_stat_max_ch[k] = stat_max_ch[k];
        }
        segm->NormalizeSegment();
        ++segsel;
    }

    for( int i = 0; i < 1024; i++)
    {
        for(int k = 0; k < 5; k++)
        {
            imageptrs->lut_ch[k][i] = 0;
        }
    }


    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);

        for( int i = 0; i < 1024; i++)
        {
            for(int k = 0; k < 5; k++)
            {
                imageptrs->lut_ch[k][i] += segm->lut_ch[k][i];
            }
        }
        ++segsel;
    }

    for( int i = 0; i < 1024; i++)
    {
        for(int k = 0; k < 5; k++)
        {
            imageptrs->lut_ch[k][i] /= segsselected.count();
        }
    }

    ComposeImage1();
}

void SegmentList::resultisready(int segmentnbr)
{
    int totalcount = segsselected.count();
    this->progressresultready += 100 / totalcount * 2;

    qDebug() << QString("result ready %1  %2 NbrOfLines = %3").arg(segmentnbr).arg(segsselected.at(segmentnbr)->fileInfo.absoluteFilePath()).
                arg(segsselected.at(segmentnbr)->NbrOfLines);
    emit progressCounter(this->progressresultready);

}

void SegmentList::resultcomposeisready(int segmentnbr)
{
    int totalcount = segsselected.count();
    this->progressresultready += 100 / totalcount * 2;

    qDebug() << QString("result ready %1  %2 NbrOfLines = %3").arg(segmentnbr).arg(segsselected.at(segmentnbr)->fileInfo.absoluteFilePath()).
                arg(segsselected.at(segmentnbr)->NbrOfLines);
    emit progressCounter(this->progressresultready);
    if(opts.imageontextureOnAVHRR)
        opts.texture_changed = true;

}

void SegmentList::ComposeImage1()
{
    qDebug() << "SegmentList::ComposeImage1()";

    watchercompose = new QFutureWatcher<void>(this);
    connect(watchercompose, SIGNAL(resultReadyAt(int)), SLOT(resultcomposeisready(int)));
    connect(watchercompose, SIGNAL(finished()), SLOT(composefinished()));
    watchercompose->setFuture(QtConcurrent::map(segsselected.begin(), segsselected.end(), &SegmentList::doComposeSegmentImage));
    qDebug() << "na SegmentList::ComposeImage1()";

}


void SegmentList::composefinished()
{

    opts.texture_changed = true;

    qDebug() << "composefinished";
    qDebug() << QString("compose stat_min_ch1 = %1  stat_max_ch1 = %2").arg(stat_min_ch[0]).arg(stat_max_ch[0]);
    qDebug() << QString("compose stat_min_ch2 = %1  stat_max_ch2 = %2").arg(stat_min_ch[1]).arg(stat_max_ch[1]);
    qDebug() << QString("compose stat_min_ch3 = %1  stat_max_ch3 = %2").arg(stat_min_ch[2]).arg(stat_max_ch[2]);
    qDebug() << QString("compose stat_min_ch4 = %1  stat_max_ch4 = %2").arg(stat_min_ch[3]).arg(stat_max_ch[3]);
    qDebug() << QString("compose stat_min_ch5 = %1  stat_max_ch5 = %2").arg(stat_min_ch[4]).arg(stat_max_ch[4]);

    QApplication::restoreOverrideCursor();

/*
    qDebug() << QString("lon[0,0] = %1 lat[0,0] = %2").arg(segsselected.at(0)->earthloc_lon[0]).arg(segsselected.at(0)->earthloc_lat[0]);
    qDebug() << QString("lon[0,102] = %1 lat[0,102] = %2").arg(segsselected.at(0)->earthloc_lon[0 + 102]).arg(segsselected.at(0)->earthloc_lat[0 + 102]);
    qDebug() << QString("lon[1079,0] = %1 lat[1079,0] = %2").arg(segsselected.at(0)->earthloc_lon[1079*103 + 0]).arg(segsselected.at(0)->earthloc_lat[1079*103 + 0]);
    qDebug() << QString("lon[1079,102] = %1 lat[1079,102] = %2").arg(segsselected.at(0)->earthloc_lon[1079*103 + 102]).arg(segsselected.at(0)->earthloc_lat[1079*103 + 102]);
*/

    emit segmentlistfinished();
    emit progressCounter(100);

}

void SegmentList::RenderSegments(QPainter *painter, QColor col, bool renderall)
{

    QList<Segment*>::iterator segit = segmentlist.begin();
    while ( segit != segmentlist.end() )
    {
        if(renderall)
        {
            (*segit)->RenderSatPath(painter, col);
            (*segit)->RenderPosition(painter );
        }
        else
        {
            if ((*segit)->segmentshow)
            {
                (*segit)->RenderSatPath(painter, col);
                (*segit)->RenderPosition(painter );
            }
        }
        ++segit;
    }

}

void SegmentList::ComposeGVProjection(int inputchannel)
{

    qDebug() << "SegmentList::ComposeGVProjection()";
    QList<Segment*>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentGVProjection(inputchannel);
         emit segmentprojectionfinished();
        ++segit;
    }

}

//void SegmentList::ComposeGVProjection(int inputchannel)
//{
//    projectioninputchannel = inputchannel;
//    watchercomposeprojection = new QFutureWatcher<void>(this);
//    connect(watchercomposeprojection, SIGNAL(resultReadyAt(int)), SLOT(composeprojectionreadyat(int)));
//    connect(watchercomposeprojection, SIGNAL(finished()), SLOT(composeprojectionfinished()));

//    watchercomposeprojection->setFuture(QtConcurrent::map(segsselected.begin(), segsselected.end(), &SegmentList::doComposeGVProjection));
//}

void SegmentList::ComposeLCCProjection(int inputchannel)
{

    qDebug() << "SegmentList::ComposeLCCProjection()";
    QList<Segment*>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentLCCProjection(inputchannel);
        emit segmentprojectionfinished();
        ++segit;
    }

}

void SegmentList::ComposeSGProjection(int inputchannel)
{
    qDebug() << "SegmentList::ComposeSGProjection()";
    QList<Segment*>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentSGProjection(inputchannel);
        emit segmentprojectionfinished();
        ++segit;
    }

}


//void SegmentList::composeprojectionfinished()
//{

//    qDebug() << "composeprojectionfinished";

//    QRgb val;
//    QRgb *row;

//    if(opts.smoothprojectionimage)
//        imageptrs->SmoothProjectionImage();

//    QApplication::restoreOverrideCursor();

//    emit segmentprojectionfinished();
//}

void SegmentList::composeprojectionreadyat(int segmentnbr)
{

    qDebug() << QString("composeprojectionreadyat %1").arg(segmentnbr);

    emit segmentprojectionfinished();

}

void SegmentList::ClearSegments()
{

    segsselected.clear();

    while (!segmentlist.isEmpty())
    {
        delete segmentlist.takeFirst();
    }

}

bool SegmentList::lookupLonLat(double lon_rad, double lat_rad, int &col, int &row)
{


    QList<Segment*>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        if((*segit)->lookupLonLat(lon_rad, lat_rad, col, row))
        {
            return true;
        }
        ++segit;
    }

    return false;
}
