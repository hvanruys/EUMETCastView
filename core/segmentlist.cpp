#include <QtConcurrent/QtConcurrent>
#include <QMatrix4x4>

//#include "segmentmetop.h"
//#include "segmentimage.h"
#include "segmentlist.h"
#include "avhrrsatellite.h"
#include "options.h"
#include <iomanip>

extern Options opts;
extern SegmentImage *imageptrs;
extern bool ptrimagebusy;

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

int SegmentList::NbrOfSegmentsSelectedinMemory()
{
    int nbr = 0;

    QList<Segment*>::iterator segit = segsselected.begin();

    while ( segit != segsselected.end() )
    {
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
//    int novs = this->GetNbrOfVisibleSegments();
    int novs = opts.nbrofvisiblesegments;

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
    qDebug() << QString("SetNbrOfVisibleSegments(int val = %1)").arg(val);
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

    qDebug() << QString("Nbr of segments = %1").arg(segmentlist.count());

    QList<Segment*>::iterator segit = segmentlist.begin();
    QVector2D winvec1, winvec2, winvecend1, winvecend2, winvecend3, winvecend4;

    QVector3D vecZ = m.row(2).toVector3D();

    segmentname = "";

    while ( segit != segmentlist.end() )
    {
        if(showallsegments ? true : (*segit)->segmentshow)
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

bool SegmentList::TestForSegmentGLextended(int x, int realy, float distance, const QMatrix4x4 &m, bool showallsegments, QString &segmentname)
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

void SegmentList::ShowWinvec(QPainter *painter, float distance, const QMatrix4x4 modelview)
{

    QList<Segment*>::iterator segit = segmentlist.begin();
    QVector2D winvec1, winvec2, winvecend1, winvecend2, winvecend3, winvecend4;

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


            qreal angle = ArcCos(QVector3D::dotProduct( vecZ, (*segit)->vec1));

            if (angle < PI/2 + (asin(1/distance)))
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

bool SegmentList::ComposeImage()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );  //restore in finished()

    emit progressCounter(0);

    progressresultready = 0;

    qDebug() << " SegmentList::ComposeImage";

    for (int i=0; i < 5; i++)
    {
        for (int j=0; j < 1024; j++)
        {
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

//#if 0 // necessary for incomplete segments !!
    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);
        segm->NbrOfLines = segm->ReadNbrOfLines();
        qDebug() << QString("SegmentList::ReadNbrOfLines NbrOfLines = %1").arg(segm->NbrOfLines);
        ++segsel;
    }
//#endif

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

    watcherread = new QFutureWatcher<void>(this);
    connect(watcherread, SIGNAL(progressValueChanged(int)), SLOT(progressvaluechanged(int)));
    connect(watcherread, SIGNAL(finished()), SLOT(readfinished()));

    watcherread->setFuture(QtConcurrent::map( segsselected.begin(), segsselected.end(), &SegmentList::doReadSegmentInMemory));

    return true;
}

void SegmentList::readfinished()
{

    int count3a = 0;
    int count3b = 0;

    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);
        for(int i = 0; i < 1080; i++)
        {
            if(segm->channel_3a_3b[i])  // 0 = channel 3b, 1 = channel 3a
                count3a++;
            else
                count3b++;
        }

        ++segsel;
    }

    qDebug() << QString("SegmentList::readfinished() count3a = %1 count3b = %2").arg(count3a).arg(count3b);

    if(count3a > count3b)
        channel_3_select = true;  // true = channel 3a - visible channel
    else
        channel_3_select = false; // false = channel 3b


    const float MAX_VALUE = 1023.0f;         // max value in 10-bit image
    int TotalLines = this->NbrOfSegmentLinesSelected();
    int nbrearthviews = NbrOfEartviewsPerScanline();
    qDebug() << QString("SegmentList::readfinished() TotalLines = %1 nbrearthviews = %2").arg(QString::number(TotalLines)).arg(nbrearthviews);

    scale = MAX_VALUE / (TotalLines * nbrearthviews);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    imageptrs->InitializeAVHRRImages( nbrearthviews, TotalLines );


    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);

        for( int k = 0; k < 5; k++)
        {

            if (segm->stat_min_ch[k] < stat_min_ch[k] )
                stat_min_ch[k] = segm->stat_min_ch[k];
            if (segm->stat_max_ch[k] > stat_max_ch[k] )
                stat_max_ch[k] = segm->stat_max_ch[k];
            if( segm->stat_max_norm_ch[k] < stat_min_norm_ch[k])
                stat_min_norm_ch[k] = segm->stat_min_norm_ch[k];
            if (segm->stat_max_norm_ch[k] > stat_max_norm_ch[k] )
                stat_max_norm_ch[k] = segm->stat_max_norm_ch[k];


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
//            if(k==3)
//            {
//                segm->list_stat_3_0_min_ch = stat_3_0_min_ch;
//                segm->list_stat_3_0_max_ch = stat_3_0_max_ch;
//                segm->list_stat_3_1_min_ch = stat_3_1_min_ch;
//                segm->list_stat_3_0_max_ch = stat_3_1_max_ch;
//            }
        }
        segm->NormalizeSegment(channel_3_select);
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

void SegmentList::progressvaluechanged(int segmentnbr)
{
    int totalcount = segsselected.count();
    this->progressresultready += 100 / totalcount * 2;

    qDebug() << QString("progressvaluechanged %1").arg(segmentnbr);
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
    ptrimagebusy = true;

    watchercompose = new QFutureWatcher<void>(this);
    connect(watchercompose, SIGNAL(resultReadyAt(int)), SLOT(resultcomposeisready(int)));
    connect(watchercompose, SIGNAL(finished()), SLOT(composefinished()));
    watchercompose->setFuture(QtConcurrent::map(segsselected.begin(), segsselected.end(), &SegmentList::doComposeSegmentImage));
    qDebug() << "na SegmentList::ComposeImage1()";

}


void SegmentList::composefinished()
{

    opts.texture_changed = true;
    ptrimagebusy = false;

    qDebug() << "composefinished";
    qDebug() << QString("compose stat_min_ch1 = %1  stat_max_ch1 = %2").arg(stat_min_ch[0]).arg(stat_max_ch[0]);
    qDebug() << QString("compose stat_min_ch2 = %1  stat_max_ch2 = %2").arg(stat_min_ch[1]).arg(stat_max_ch[1]);
    qDebug() << QString("compose stat_min_ch3 = %1  stat_max_ch3 = %2").arg(stat_min_ch[2]).arg(stat_max_ch[2]);
    qDebug() << QString("compose stat_min_ch4 = %1  stat_max_ch4 = %2").arg(stat_min_ch[3]).arg(stat_max_ch[3]);
    qDebug() << QString("compose stat_min_ch5 = %1  stat_max_ch5 = %2").arg(stat_min_ch[4]).arg(stat_max_ch[4]);


    emit progressCounter(100);

    QApplication::restoreOverrideCursor();

/*
    qDebug() << QString("lon[0,0] = %1 lat[0,0] = %2").arg(segsselected.at(0)->earthloc_lon[0]).arg(segsselected.at(0)->earthloc_lat[0]);
    qDebug() << QString("lon[0,102] = %1 lat[0,102] = %2").arg(segsselected.at(0)->earthloc_lon[0 + 102]).arg(segsselected.at(0)->earthloc_lat[0 + 102]);
    qDebug() << QString("lon[1079,0] = %1 lat[1079,0] = %2").arg(segsselected.at(0)->earthloc_lon[1079*103 + 0]).arg(segsselected.at(0)->earthloc_lat[1079*103 + 0]);
    qDebug() << QString("lon[1079,102] = %1 lat[1079,102] = %2").arg(segsselected.at(0)->earthloc_lon[1079*103 + 102]).arg(segsselected.at(0)->earthloc_lat[1079*103 + 102]);
*/

    emit segmentlistfinished(true);

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
        (*segit)->ComposeSegmentGVProjection(inputchannel, 0, false);
         emit segmentprojectionfinished(false);
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
        (*segit)->ComposeSegmentLCCProjection(inputchannel, 0, false);
        emit segmentprojectionfinished(false);
        ++segit;
    }

}

void SegmentList::ComposeSGProjection(int inputchannel)
{
    qDebug() << "SegmentList::ComposeSGProjection()";
    QList<Segment*>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentSGProjection(inputchannel, 0, false);
        emit segmentprojectionfinished(false);
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

    emit segmentprojectionfinished(false);

}

void SegmentList::ClearSegments()
{
    segsselected.clear();

    while (!segmentlist.isEmpty())
    {
        delete segmentlist.takeFirst();
    }
}

void SegmentList::SmoothProjectionImageBilinear()
{

    qDebug() << "start SegmentList::SmoothProjectionImageBilinear()";

    int lineimage = 0;

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();


    Segment *segmsave;

    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments(segmsave, segm, false);
        segmsave = segm;
        BilinearInterpolation(segm, false);
        ++segsel;
        lineimage += segm->NbrOfLines;
    }
}

void SegmentList::SmoothProjectionImageBicubic()
{

    qDebug() << "start SegmentList::SmoothProjectionImageBicubic()";

    int lineimage = 0;

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    Segment *segmsave;

    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments(segmsave, segm, false);
        segmsave = segm;
        BilinearInterpolation(segm, false);
        ++segsel;
        lineimage += segm->NbrOfLines;
    }
}

void SegmentList::BilinearInterpolation(Segment *segm, bool combine)
{
    qint32 x11;
    qint32 y11;
    QRgb rgb11;

    qint32 x12;
    qint32 y12;
    QRgb rgb12;

    qint32 x21;
    qint32 y21;
    QRgb rgb21;

    qint32 x22;
    qint32 y22;
    QRgb rgb22;

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

    QRgb *canvas;

    int earthviews = this->NbrOfEartviewsPerScanline();

    qDebug() << "===> start SegmentList::BilinearInterpolation(Segment *segm) for " << segm->segment_type;

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
                if( (dimx == 1 && dimy == 1) || (dimx > 50 && dimy > 50))
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

//                    if(line == 1 && pixelx == 1)
//                    {
//                        qDebug() << QString("rgb11 = %1 rgb12 = %2 rgb21 = %3 rgb22 = %4").arg(qRed(rgb11)).arg(qRed(rgb12)).arg(qRed(rgb21)).arg(qRed(rgb22));
//                        for ( int i = 0; i < dimy; i++ )
//                        {
//                            for ( int j = 0; j < dimx; j++ )
//                            {
//                                std::cout << std::setw(3) << qRed(canvas[i * dimx + j]) << " ";
//                            }
//                            std::cout << std::endl;
//                        }
//                        std::cout << "before ....................................... line " << line << " pixelx = " << pixelx << std::endl;
//                    }

//                    std::flush(cout);

                    bhm_line(xc11, yc11, xc12, yc12, rgb11, rgb12, canvas, dimx);
                    bhm_line(xc12, yc12, xc22, yc22, rgb12, rgb22, canvas, dimx);
                    bhm_line(xc22, yc22, xc21, yc21, rgb22, rgb21, canvas, dimx);
                    bhm_line(xc21, yc21, xc11, yc11, rgb21, rgb11, canvas, dimx);

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
//                        std::cout << "after -------------------------------------- line " << line << " pixelx = " << pixelx << std::endl;
//                    }

                    MapInterpolation(canvas, dimx, dimy);
                    MapCanvas(canvas, anchorX, anchorY, dimx, dimy, combine);

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

                    delete [] canvas;
                    counterb++;
                }
            }
        }
    }

    qDebug() << QString("====> end SegmentList::BilinearInterpolation(Segment *segm) counter = %1 countern = %2").arg(counter).arg(counterb);

}


void SegmentList::BilinearInterpolation12bits(Segment *segm)
{
    qint32 x11;
    qint32 y11;
    quint16 col11[3];

    qint32 x12;
    qint32 y12;
    quint16 col12[3];

    qint32 x21;
    qint32 y21;
    quint16 col21[3];

    qint32 x22;
    qint32 y22;
    quint16 col22[3];

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

    quint16 *canvasred;
    quint16 *canvasgreen;
    quint16 *canvasblue;
    quint8 *canvasalpha;

    int earthviews = this->NbrOfEartviewsPerScanline();

    qDebug() << "===> start SegmentList::BilinearInterpolation12bits(Segment *segm) for " << segm->segment_type;

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
                if( (dimx == 1 && dimy == 1) || (dimx > 50 && dimy > 50))
                {
                    counter++;
                }
                else
                {
                    col11[0] = segm->getProjectionValueRed(line, pixelx);
                    col12[0] = segm->getProjectionValueRed(line, pixelx+1);
                    col21[0] = segm->getProjectionValueRed(line+1, pixelx);
                    col22[0] = segm->getProjectionValueRed(line+1, pixelx+1);

                    col11[1] = segm->getProjectionValueGreen(line, pixelx);
                    col12[1] = segm->getProjectionValueGreen(line, pixelx+1);
                    col21[1] = segm->getProjectionValueGreen(line+1, pixelx);
                    col22[1] = segm->getProjectionValueGreen(line+1, pixelx+1);

                    col11[2] = segm->getProjectionValueBlue(line, pixelx);
                    col12[2] = segm->getProjectionValueBlue(line, pixelx+1);
                    col21[2] = segm->getProjectionValueBlue(line+1, pixelx);
                    col22[2] = segm->getProjectionValueBlue(line+1, pixelx+1);

                    xc11 = x11 - minx;
                    xc12 = x12 - minx;
                    xc21 = x21 - minx;
                    xc22 = x22 - minx;
                    yc11 = y11 - miny;
                    yc12 = y12 - miny;
                    yc21 = y21 - miny;
                    yc22 = y22 - miny;

                    canvasred = new quint16[dimx * dimy];
                    canvasgreen = new quint16[dimx * dimy];
                    canvasblue = new quint16[dimx * dimy];
                    canvasalpha = new quint8[dimx * dimy];

                    for(int i = 0 ; i < dimx * dimy ; i++)
                    {
                        canvasred[i] = 0;
                        canvasgreen[i] = 0;
                        canvasblue[i] = 0;
                        canvasalpha[i] = 0;
                    }

                    canvasred[yc11 * dimx + xc11] = col11[0];
                    canvasred[yc12 * dimx + xc12] = col12[0];
                    canvasred[yc21 * dimx + xc21] = col21[0];
                    canvasred[yc22 * dimx + xc22] = col22[0];

                    canvasgreen[yc11 * dimx + xc11] = col11[1];
                    canvasgreen[yc12 * dimx + xc12] = col12[1];
                    canvasgreen[yc21 * dimx + xc21] = col21[1];
                    canvasgreen[yc22 * dimx + xc22] = col22[1];

                    canvasblue[yc11 * dimx + xc11] = col11[2];
                    canvasblue[yc12 * dimx + xc12] = col12[2];
                    canvasblue[yc21 * dimx + xc21] = col21[2];
                    canvasblue[yc22 * dimx + xc22] = col22[2];

                    canvasalpha[yc11 * dimx + xc11] = 255;
                    canvasalpha[yc12 * dimx + xc12] = 255;
                    canvasalpha[yc21 * dimx + xc21] = 255;
                    canvasalpha[yc22 * dimx + xc22] = 255;

                    bhm_line12bits(xc11, yc11, xc12, yc12, col11, col12, canvasred, canvasgreen, canvasblue, canvasalpha, dimx);
                    bhm_line12bits(xc12, yc12, xc22, yc22, col12, col22, canvasred, canvasgreen, canvasblue, canvasalpha, dimx);
                    bhm_line12bits(xc22, yc22, xc21, yc21, col22, col21, canvasred, canvasgreen, canvasblue, canvasalpha, dimx);
                    bhm_line12bits(xc21, yc21, xc11, yc11, col21, col11, canvasred, canvasgreen, canvasblue, canvasalpha, dimx);

                    MapInterpolation12bits(canvasred, canvasgreen, canvasblue, canvasalpha, dimx, dimy);
                    MapCanvas12bits(canvasred, canvasgreen, canvasblue, canvasalpha, anchorX, anchorY, dimx, dimy);

                    delete [] canvasred;
                    delete [] canvasgreen;
                    delete [] canvasblue;
                    delete [] canvasalpha;
                }
            }
        }
    }


}

void SegmentList::BilinearBetweenSegments(Segment *segmfirst, Segment *segmnext, bool combine)
{
    qint32 x11;
    qint32 y11;
    QRgb rgb11;

    qint32 x12;
    qint32 y12;
    QRgb rgb12;

    qint32 x21;
    qint32 y21;
    QRgb rgb21;

    qint32 x22;
    qint32 y22;
    QRgb rgb22;

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

    QRgb *canvas;

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
                rgb11 = segmfirst->getProjectionValue(segmfirst->NbrOfLines-1, pixelx);
                rgb12 = segmfirst->getProjectionValue(segmfirst->NbrOfLines-1, pixelx+1);
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
                MapCanvas(canvas, anchorX, anchorY, dimx, dimy, combine);

                delete [] canvas;
                counterb++;
            }
        }
    }


    qDebug() << QString("====> end SegmentList::BilinearInbetween(Segment *segmfirst, Segment *segmnext) counter = %1 counterb = %2").arg(counter).arg(counterb);

}

void SegmentList::BilinearBetweenSegments12bits(Segment *segmfirst, Segment *segmnext)
{
    qint32 x11;
    qint32 y11;
    quint16 col11[3];

    qint32 x12;
    qint32 y12;
    quint16 col12[3];

    qint32 x21;
    qint32 y21;
    quint16 col21[3];

    qint32 x22;
    qint32 y22;
    quint16 col22[3];

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

    quint16 *canvasred;
    quint16 *canvasgreen;
    quint16 *canvasblue;
    quint8 *canvasalpha;

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
                col11[0] = segmfirst->getProjectionValueRed(segmfirst->NbrOfLines-1, pixelx);
                col12[0] = segmfirst->getProjectionValueRed(segmfirst->NbrOfLines-1, pixelx+1);
                col21[0] = segmnext->getProjectionValueRed(0, pixelx);
                col22[0] = segmnext->getProjectionValueRed(0, pixelx+1);

                col11[1] = segmfirst->getProjectionValueGreen(segmfirst->NbrOfLines-1, pixelx);
                col12[1] = segmfirst->getProjectionValueGreen(segmfirst->NbrOfLines-1, pixelx+1);
                col21[1] = segmnext->getProjectionValueGreen(0, pixelx);
                col22[1] = segmnext->getProjectionValueGreen(0, pixelx+1);

                col11[2] = segmfirst->getProjectionValueBlue(segmfirst->NbrOfLines-1, pixelx);
                col12[2] = segmfirst->getProjectionValueBlue(segmfirst->NbrOfLines-1, pixelx+1);
                col21[2] = segmnext->getProjectionValueBlue(0, pixelx);
                col22[2] = segmnext->getProjectionValueBlue(0, pixelx+1);

                xc11 = x11 - minx;
                xc12 = x12 - minx;
                xc21 = x21 - minx;
                xc22 = x22 - minx;
                yc11 = y11 - miny;
                yc12 = y12 - miny;
                yc21 = y21 - miny;
                yc22 = y22 - miny;

                canvasred = new quint16[dimx * dimy];
                canvasgreen = new quint16[dimx * dimy];
                canvasblue = new quint16[dimx * dimy];
                canvasalpha = new quint8[dimx * dimy];

                for(int i = 0 ; i < dimx * dimy ; i++)
                {
                    canvasred[i] = 0;
                    canvasgreen[i] = 0;
                    canvasblue[i] = 0;
                    canvasalpha[i] = 0;
                }


                canvasred[yc11 * dimx + xc11] = col11[0];
                canvasred[yc12 * dimx + xc12] = col12[0];
                canvasred[yc21 * dimx + xc21] = col21[0];
                canvasred[yc22 * dimx + xc22] = col22[0];

                canvasgreen[yc11 * dimx + xc11] = col11[1];
                canvasgreen[yc12 * dimx + xc12] = col12[1];
                canvasgreen[yc21 * dimx + xc21] = col21[1];
                canvasgreen[yc22 * dimx + xc22] = col22[1];

                canvasblue[yc11 * dimx + xc11] = col11[2];
                canvasblue[yc12 * dimx + xc12] = col12[2];
                canvasblue[yc21 * dimx + xc21] = col21[2];
                canvasblue[yc22 * dimx + xc22] = col22[2];

                canvasalpha[yc11 * dimx + xc11] = 255;
                canvasalpha[yc12 * dimx + xc12] = 255;
                canvasalpha[yc21 * dimx + xc21] = 255;
                canvasalpha[yc22 * dimx + xc22] = 255;


                bhm_line12bits(xc11, yc11, xc12, yc12, col11, col12, canvasred, canvasgreen, canvasblue, canvasalpha, dimx);
                bhm_line12bits(xc12, yc12, xc22, yc22, col12, col22, canvasred, canvasgreen, canvasblue, canvasalpha, dimx);
                bhm_line12bits(xc22, yc22, xc21, yc21, col22, col21, canvasred, canvasgreen, canvasblue, canvasalpha, dimx);
                bhm_line12bits(xc21, yc21, xc11, yc11, col21, col11, canvasred, canvasgreen, canvasblue, canvasalpha, dimx);

                MapInterpolation12bits(canvasred, canvasgreen, canvasblue, canvasalpha, dimx, dimy);
                MapCanvas12bits(canvasred, canvasgreen, canvasblue, canvasalpha, anchorX, anchorY, dimx, dimy);

                delete [] canvasred;
                delete [] canvasgreen;
                delete [] canvasblue;
                delete [] canvasalpha;

            }
        }
    }


    qDebug() << QString("====> end SegmentList::BilinearBetweenSegments12bit(Segment *segmfirst, Segment *segmnext) counter = %1 counterb = %2").arg(counter).arg(counterb);

}


bool SegmentList::bhm_line(int x1, int y1, int x2, int y2, QRgb rgb1, QRgb rgb2, QRgb *canvas, int dimx)
{
    int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;
    float deltared, deltagreen, deltablue;
    float red1, red2, green1, green2, blue1, blue2;

    dx=x2-x1;
    dy=y2-y1;
    dx1=abs(dx);
    dy1=abs(dy);
    px=2*dy1-dx1;
    py=2*dx1-dy1;

    red1 = qRed(rgb1);
    red2 = qRed(rgb2);
    green1 = qGreen(rgb1);
    green2 = qGreen(rgb2);
    blue1 = qBlue(rgb1);
    blue2 = qBlue(rgb2);

    if(dy1<=dx1)
    {
        if(dx1==0)
            return false;

        if(dx>=0)
        {
            x=x1;
            y=y1;
            xe=x2;
            deltared = (float)(qRed(rgb2) - qRed(rgb1))/ (float)dx1 ;
            deltagreen = (float)(qGreen(rgb2) - qGreen(rgb1))/ (float)dx1 ;
            deltablue = (float)(qBlue(rgb2) - qBlue(rgb1))/ (float)dx1 ;
//            canvas[y * yy + x] = val1;

        }
        else
        {
            x=x2;
            y=y2;
            xe=x1;
            deltared = (float)(qRed(rgb1) - qRed(rgb2))/ (float)dx1 ;
            deltagreen = (float)(qGreen(rgb1) - qGreen(rgb2))/ (float)dx1 ;
            deltablue = (float)(qBlue(rgb1) - qBlue(rgb2))/ (float)dx1 ;
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
                red1 += deltared;
                green1 += deltagreen;
                blue1 += deltablue;

                rgb1 = qRgb((int)red1, (int)green1, (int)blue1 );
                if( x != xe)
                    canvas[y * dimx + x] = rgb1;
            }
            else
            {
                red2 += deltared;
                green2 += deltagreen;
                blue2 += deltablue;

                rgb2 = qRgb((int)red2, (int)green2, (int)blue2 );
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
            deltared = (float)(qRed(rgb2) - qRed(rgb1))/ (float)dy1 ;
            deltagreen = (float)(qGreen(rgb2) - qGreen(rgb1))/ (float)dy1 ;
            deltablue = (float)(qBlue(rgb2) - qBlue(rgb1))/ (float)dy1 ;

//            canvas[y * yy + x] = val1;
        }
        else
        {
            x=x2;
            y=y2;
            ye=y1;
            deltared = (float)(qRed(rgb1) - qRed(rgb2))/ (float)dy1 ;
            deltagreen = (float)(qGreen(rgb1) - qGreen(rgb2))/ (float)dy1 ;
            deltablue = (float)(qBlue(rgb1) - qBlue(rgb2))/ (float)dy1 ;

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
                red1 += deltared;
                green1 += deltagreen;
                blue1 += deltablue;

                rgb1 = qRgb((int)red1, (int)green1, (int)blue1 );
                if( y != ye)
                    canvas[y * dimx + x] = rgb1;
            }
            else
            {
                red2 += deltared;
                green2 += deltagreen;
                blue2 += deltablue;

                rgb2 = qRgb((int)red2, (int)green2, (int)blue2 );
                if( y != ye)
                    canvas[y * dimx + x] = rgb2;
            }
        }
    }

    return true;
}


bool SegmentList::bhm_line12bits(int x1, int y1, int x2, int y2, quint16 col1[3], quint16 col2[], quint16 *canvasred, quint16 *canvasgreen, quint16 *canvasblue, quint8 *canvasalpha, int dimx)
{
    int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;
    float deltared, deltagreen, deltablue;
    float red1, red2, green1, green2, blue1, blue2;

    dx=x2-x1;
    dy=y2-y1;
    dx1=abs(dx);
    dy1=abs(dy);
    px=2*dy1-dx1;
    py=2*dx1-dy1;

    red1 = col1[0];
    red2 = col2[0];
    green1 = col1[1];
    green2 = col2[1];
    blue1 = col1[2];
    blue2 = col2[2];

    if(dy1<=dx1)
    {
        if(dx1==0)
            return false;

        if(dx>=0)
        {
            x=x1;
            y=y1;
            xe=x2;
            deltared = (float)(col2[0] - col1[0])/ (float)dx1 ;
            deltagreen = (float)(col2[1] - col1[1])/ (float)dx1 ;
            deltablue = (float)(col2[2] - col1[2])/ (float)dx1 ;
//            canvas[y * yy + x] = val1;

        }
        else
        {
            x=x2;
            y=y2;
            xe=x1;
            deltared = (float)(col1[0] - col2[0])/ (float)dx1 ;
            deltagreen = (float)(col1[1] - col2[1])/ (float)dx1 ;
            deltablue = (float)(col1[2] - col2[2])/ (float)dx1 ;
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
                red1 += deltared;
                green1 += deltagreen;
                blue1 += deltablue;

                if( x != xe)
                {
                    canvasred[y * dimx + x] = red1;
                    canvasgreen[y * dimx + x] = green1;
                    canvasblue[y * dimx + x] = blue1;
                    canvasalpha[y * dimx + x] = 255;
                }
            }
            else
            {
                red2 += deltared;
                green2 += deltagreen;
                blue2 += deltablue;

                if( x != xe)
                {
                    canvasred[y * dimx + x] = red2;
                    canvasgreen[y * dimx + x] = green2;
                    canvasblue[y * dimx + x] = blue2;
                    canvasalpha[y * dimx + x] = 255;
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
            deltared = (float)(col2[0] - col1[0])/ (float)dy1 ;
            deltagreen = (float)(col2[1] - col1[1])/ (float)dy1 ;
            deltablue = (float)(col2[2] - col1[2])/ (float)dy1 ;

//            canvas[y * yy + x] = val1;
        }
        else
        {
            x=x2;
            y=y2;
            ye=y1;
            deltared = (float)(col1[0] - col2[0])/ (float)dy1 ;
            deltagreen = (float)(col1[1] - col2[1])/ (float)dy1 ;
            deltablue = (float)(col1[2] - col2[2])/ (float)dy1 ;

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
                red1 += deltared;
                green1 += deltagreen;
                blue1 += deltablue;

                if( y != ye)
                {
                    canvasred[y * dimx + x] = red1;
                    canvasgreen[y * dimx + x] = green1;
                    canvasblue[y * dimx + x] = blue1;
                    canvasalpha[y * dimx + x] = 255;
                }
            }
            else
            {
                red2 += deltared;
                green2 += deltagreen;
                blue2 += deltablue;

                if( y != ye)
                {
                    canvasred[y * dimx + x] = red2;
                    canvasgreen[y * dimx + x] = green2;
                    canvasblue[y * dimx + x] = blue2;
                    canvasalpha[y * dimx + x] = 255;
                }
            }
        }
    }

    return true;
}



void SegmentList::MapInterpolation(QRgb *canvas, quint16 dimx, quint16 dimy)
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



        float deltared = (float)(qRed(end) - qRed(start)) / (float)(holecount+1);
        float deltagreen = (float)(qGreen(end) - qGreen(start)) / (float)(holecount+1);
        float deltablue = (float)(qBlue(end) - qBlue(start)) / (float)(holecount+1);

        float red = (float)qRed(start);
        float green = (float)qGreen(start);
        float blue = (float)qBlue(start);

        for(int w = 0; w < dimx; w++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 100)
            {
                red += deltared;
                green += deltagreen;
                blue += deltablue;
                canvas[h * dimx + w] = qRgba((int)red, (int)green, (int)blue, 100);
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

        float redstart = (float)qRed(start);
        float greenstart = (float)qGreen(start);
        float bluestart = (float)qBlue(start);

        float deltared = (float)(qRed(end) - qRed(start)) / (float)(hcount+1);
        float deltagreen = (float)(qGreen(end) - qGreen(start)) / (float)(hcount+1);
        float deltablue = (float)(qBlue(end) - qBlue(start)) / (float)(hcount+1);


        for(int h = 0; h < dimy; h++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 100)
            {
                redstart += deltared;
                greenstart += deltagreen;
                bluestart += deltablue;
                float redtotal = (qRed(canvas[h * dimx + w]) + redstart)/2;
                float greentotal = (qGreen(canvas[h * dimx + w]) + greenstart)/2;
                float bluetotal = (qBlue(canvas[h * dimx + w]) + bluestart)/2;

                canvas[h * dimx + w] = qRgba((int)redtotal, (int)greentotal, (int)bluetotal, 255);
            }
        }
    }


}

void SegmentList::MapInterpolation12bits(quint16 *canvasred, quint16 *canvasgreen, quint16 *canvasblue, quint8 *canvasalpha, quint16 dimx, quint16 dimy)
{

    for(int h = 0; h < dimy; h++ )
    {
        quint16 startred = 0;
        quint16 startgreen = 0;
        quint16 startblue = 0;

        quint16 endred = 0;
        quint16 endgreen = 0;
        quint16 endblue = 0;

        bool hole = false;
        bool first = false;
        bool last = false;
        int holecount = 0;

        for(int w = 0; w < dimx; w++)
        {
            quint16 r = canvasred[h * dimx + w];
            quint16 g = canvasgreen[h * dimx + w];
            quint16 b = canvasblue[h * dimx + w];
            quint16 a = canvasalpha[h * dimx + w];
            if(a == 255 && hole == false)
            {
                startred = r;
                startgreen = g;
                startblue = b;
                first = true;
            }
            else if(a == 255 && hole == true)
            {
                endred = r;
                endgreen = g;
                endblue = b;
                last = true;
                break;
            }
            else if(a == 0 && first == true)
            {
                hole = true;
                holecount++;
                canvasred[h * dimx + w] = 0;
                canvasgreen[h * dimx + w] = 0;
                canvasblue[h * dimx + w] = 0;
                canvasalpha[h * dimx + w] = 100;
            }
        }

        if(holecount == 0)
            continue;
        if(first == false || last == false)
        {
            for(int w = 0; w < dimx; w++)
            {
                quint16 r = canvasred[h * dimx + w];
                quint16 g = canvasgreen[h * dimx + w];
                quint16 b = canvasblue[h * dimx + w];
                quint8 a = canvasalpha[h * dimx + w];

                if(a == 100)
                {
                    canvasred[h * dimx + w] = 0;
                    canvasgreen[h * dimx + w] = 0;
                    canvasblue[h * dimx + w] = 0;
                    canvasalpha[h * dimx + w] = 0;
                }
            }
            continue;
        }



        float deltared = (float)(endred - startred) / (float)(holecount+1);
        float deltagreen = (float)(endgreen - startgreen) / (float)(holecount+1);
        float deltablue = (float)(endblue - startblue) / (float)(holecount+1);

        float red = (float)startred;
        float green = (float)startgreen;
        float blue = (float)startblue;

        for(int w = 0; w < dimx; w++)
        {
            quint16 r = canvasred[h * dimx + w];
            quint16 g = canvasgreen[h * dimx + w];
            quint16 b = canvasblue[h * dimx + w];
            quint8 a = canvasalpha[h * dimx + w];

            if(a == 100)
            {
                red += deltared;
                green += deltagreen;
                blue += deltablue;
                canvasred[h * dimx + w] = (quint16)red;
                canvasgreen[h * dimx + w] = (quint16)green;
                canvasblue[h * dimx + w] = (quint16)blue;
                canvasalpha[h * dimx + w] = 100;

            }
        }
    }


    for(int w = 0; w < dimx; w++)
    {
        quint16 startred = 0;
        quint16 startgreen = 0;
        quint16 startblue = 0;

        quint16 endred = 0;
        quint16 endgreen = 0;
        quint16 endblue = 0;

        int hcount = 0;

        bool startok = false;

        for(int h = 0; h < dimy; h++)
        {
            quint16 r = canvasred[h * dimx + w];
            quint16 g = canvasgreen[h * dimx + w];
            quint16 b = canvasblue[h * dimx + w];
            quint8 a = canvasalpha[h * dimx + w];
            if(a == 255 && !startok)
            {
                startred = r;
                startgreen = g;
                startblue = b;
            }
            else
            {
                if(a == 255)
                {
                    endred = r;
                    endgreen = g;
                    endblue = b;
                    break;
                }
                else if(a == 100)
                {
                    startok = true;
                    hcount++;
                }

            }
        }

        if(hcount == 0)
            continue;

        float fredstart = (float)startred;
        float fgreenstart = (float)startgreen;
        float fbluestart = (float)startblue;

        float deltared = (float)(endred - startred) / (float)(hcount+1);
        float deltagreen = (float)(endgreen - startgreen) / (float)(hcount+1);
        float deltablue = (float)(endblue - startblue) / (float)(hcount+1);


        for(int h = 0; h < dimy; h++)
        {
            quint16 r = canvasred[h * dimx + w];
            quint16 g = canvasgreen[h * dimx + w];
            quint16 b = canvasblue[h * dimx + w];
            quint8 a = canvasalpha[h * dimx + w];
            if(a == 100)
            {
                fredstart += deltared;
                fgreenstart += deltagreen;
                fbluestart += deltablue;
                float redtotal = (canvasred[h * dimx + w] + fredstart)/2;
                float greentotal = (canvasgreen[h * dimx + w] + fgreenstart)/2;
                float bluetotal = (canvasblue[h * dimx + w] + fbluestart)/2;

                canvasred[h * dimx + w] = (quint16)redtotal;
                canvasgreen[h * dimx + w] = (quint16)greentotal;
                canvasblue[h * dimx + w] = (quint16)bluetotal;
                canvasalpha[h * dimx + w] = 255;
            }
        }
    }

}

void SegmentList::MapInterpolation1(QRgb *canvas, quint16 dimx, quint16 dimy)
{

    for(int h = 0; h < dimy; h++ )
    {
        QRgb start = qRgba(0,0,0,0);
        QRgb end = qRgba(0,0,0,0);
        bool first = false;
        bool last = false;
        int holecount = 0;
        int firstindex;
        int lastindex;

        for(int w = 0; w < dimx; w++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 255)
            {
                firstindex = w;
                first = true;
                start = rgb;
            }
            else if(rgbalpha == 0 && first == true)
            {
                break;
            }
        }


        for(int w = dimx - 1; w >= 0; w--)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 255 && firstindex <= w)
            {
                lastindex = w;
                last = true;
                end = rgb;
            }
            else if(rgbalpha == 0 && last == true)
            {
                break;
            }
        }


        for(int w = firstindex; w < lastindex; w++)
        {
            if(qAlpha(canvas[h * dimx + w]) == 0)
            {
                canvas[h * dimx + w] = qRgba(0,0,0,100);
                holecount++;
            }
        }

        if(holecount == 0)
            continue;

        float deltared = (float)(qRed(end) - qRed(start)) / (float)(holecount+1);
        float deltagreen = (float)(qGreen(end) - qGreen(start)) / (float)(holecount+1);
        float deltablue = (float)(qBlue(end) - qBlue(start)) / (float)(holecount+1);

        float red = (float)qRed(start);
        float green = (float)qGreen(start);
        float blue = (float)qBlue(start);

        for(int w = 0; w < dimx; w++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 100)
            {
                red += deltared;
                green += deltagreen;
                blue += deltablue;
                canvas[h * dimx + w] = qRgba((int)red, (int)green, (int)blue, 100);
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

        float redstart = (float)qRed(start);
        float greenstart = (float)qGreen(start);
        float bluestart = (float)qBlue(start);

        float deltared = (float)(qRed(end) - qRed(start)) / (float)(hcount+1);
        float deltagreen = (float)(qGreen(end) - qGreen(start)) / (float)(hcount+1);
        float deltablue = (float)(qBlue(end) - qBlue(start)) / (float)(hcount+1);


        for(int h = 0; h < dimy; h++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 100)
            {
                redstart += deltared;
                greenstart += deltagreen;
                bluestart += deltablue;
                float redtotal = (qRed(canvas[h * dimx + w]) + redstart)/2;
                float greentotal = (qGreen(canvas[h * dimx + w]) + greenstart)/2;
                float bluetotal = (qBlue(canvas[h * dimx + w]) + bluestart)/2;

                canvas[h * dimx + w] = qRgba((int)redtotal, (int)greentotal, (int)bluetotal, 255);
            }
        }
    }
}

void SegmentList::MapCanvas(QRgb *canvas, qint32 anchorX, qint32 anchorY, quint16 dimx, quint16 dimy, bool combine)
{
    for(int h = 0; h < dimy; h++ )
    {
        for(int w = 0; w < dimx; w++)
        {
            QRgb rgb = canvas[h * dimx + w];
            if(qAlpha(rgb) == 255)
            {
                if (anchorX + w >= 0 && anchorX + w < imageptrs->ptrimageProjection->width() &&
                        anchorY + h >= 0 && anchorY + h < imageptrs->ptrimageProjection->height())
                {
                    if(combine)
                    {
                        QRgb rgbproj = imageptrs->ptrimageProjectionCopy->pixel(anchorX + w, anchorY + h);
                        int rproj = qRed(rgbproj);
                        int gproj = qGreen(rgbproj);
                        int bproj = qBlue(rgbproj);
                        int dnbval  = qRed(rgb);

                        float rfact = (float)((255 - rproj) * dnbval)/255.0;
                        float gfact = (float)((255 - gproj) * dnbval)/255.0;
                        float bfact = (float)((255 - bproj) * dnbval)/255.0;
                        int redout = (int)rfact + rproj > 255 ? 255 : (int)rfact + rproj;
                        int greenout = (int)gfact + gproj > 255 ? 255 : (int)gfact + gproj;
                        int blueout = (int)bfact + bproj > 255 ? 255 : (int)bfact + bproj;

                        QRgb rgbout = qRgb(redout, greenout, blueout);
                        imageptrs->ptrimageProjection->setPixel(anchorX + w, anchorY + h, rgbout);

                    }
                    else
                        imageptrs->ptrimageProjection->setPixel(anchorX + w, anchorY + h, rgb);
                }
            }
        }
    }
}

void SegmentList::MapCanvas12bits(quint16 *canvasred, quint16 *canvasgreen, quint16 *canvasblue, quint8 *canvasalpha, qint32 anchorX, qint32 anchorY, quint16 dimx, quint16 dimy)
{
    int height = imageptrs->ptrimageProjection->height();
    int width = imageptrs->ptrimageProjection->width();
    for(int h = 0; h < dimy; h++ )
    {
        for(int w = 0; w < dimx; w++)
        {
            quint16 r = canvasred[h * dimx + w];
            quint16 g = canvasgreen[h * dimx + w];
            quint16 b = canvasblue[h * dimx + w];
            quint8 a = canvasalpha[h * dimx + w];

            if(a == 255)
            {
                if (anchorX + w >= 0 && anchorX + w < width && anchorY + h >= 0 && anchorY + h < height)
                {
                     imageptrs->ptrimageProjectionRed[(anchorY + h) * width + anchorX + w] = r;
                     imageptrs->ptrimageProjectionGreen[(anchorY + h) * width + anchorX + w] = g;
                     imageptrs->ptrimageProjectionBlue[(anchorY + h) * width + anchorX + w] = b;
                     imageptrs->ptrimageProjectionAlpha[(anchorY + h) * width + anchorX + w] = 65535;
                }
            }
        }
    }
}

double SegmentList::cubicInterpolate (double p[4], double x) {
    return p[1] + 0.5 * x*(p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0])));
}

double SegmentList::bicubicInterpolate (double p[4][4], double x, double y) {
    double arr[4];
    arr[0] = cubicInterpolate(p[0], y);
    arr[1] = cubicInterpolate(p[1], y);
    arr[2] = cubicInterpolate(p[2], y);
    arr[3] = cubicInterpolate(p[3], y);
    return cubicInterpolate(arr, x);
}

qint32 SegmentList::Min(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22)
{
    qint32 Minimum = v11;

    if( Minimum > v12 )
            Minimum = v12;
    if( Minimum > v21 )
            Minimum = v21;
    if( Minimum > v22 )
            Minimum = v22;

    return Minimum;
}

qint32 SegmentList::Max(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22)
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

void SegmentList::Compose48bitProjectionPNG(QString fileName, bool mapto65535)
{
    quint16 pixval, pixvalout;
    bool iscolor = bandlist.at(0);

    int height = imageptrs->ptrimageProjection->height();
    int width = imageptrs->ptrimageProjection->width();

    // initialize the FreeImage library
    FreeImage_Initialise();

    FIBITMAP *bitmap = FreeImage_AllocateT(FIT_RGBA16, width, height); // 64 bit RGBA image

    for (int line = 0; line < height; line++)
    {
        FIRGBA16 *bits = (FIRGBA16 *)FreeImage_GetScanLine(bitmap, height - line - 1);
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            pixval = imageptrs->ptrimageProjectionRed[line * width + pixelx];
            if(mapto65535)
                pixvalout = (quint16)qMin(qMax(16 * pixval, 0), 65535);
            else
                pixvalout = pixval;

            bits[pixelx].red = pixvalout;
            if(iscolor)
            {
                pixval = imageptrs->ptrimageProjectionGreen[line * width + pixelx];
                if(mapto65535)
                    pixvalout = (quint16)qMin(qMax(16 * pixval, 0), 65535);
                else
                    pixvalout = pixval;
                bits[pixelx].green = pixvalout;

                pixval = imageptrs->ptrimageProjectionBlue[line * width + pixelx];
                if(mapto65535)
                    pixvalout = (quint16)qMin(qMax(16 * pixval, 0), 65535);
                else
                    pixvalout = pixval;
                bits[pixelx].blue = pixvalout;
                bits[pixelx].alpha = imageptrs->ptrimageProjectionAlpha[line * width + pixelx];
            }
            else
            {
                bits[pixelx].green = bits[pixelx].red;
                bits[pixelx].blue = bits[pixelx].red;
                bits[pixelx].alpha = imageptrs->ptrimageProjectionAlpha[line * width + pixelx];
            }
        }
    }

    QByteArray array = fileName.toLocal8Bit();
    char* pfileName = array.data();

    if(FreeImage_Save(FIF_PNG,bitmap, pfileName,0))
    {
        qDebug() << "bitmap successfully saved!";
    }

    FreeImage_Unload(bitmap);

    FreeImage_DeInitialise();


}
