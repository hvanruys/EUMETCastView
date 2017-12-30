#include "segment.h"
#include "segmentimage.h"

#include "options.h"
#include <QDebug>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>

extern Options opts;
extern SegmentImage *imageptrs;

#include <QMutex>
extern QMutex g_mutex;

Segment::Segment(QObject *parent) :
    QObject(parent)
{
    cnt_geadr = 0;
    cnt_giadr = 0;
    cnt_ipr = 0;
    cnt_mdr = 0;
    cnt_mphr = 0;
    cnt_sphr = 0;
    cnt_unknown = 0;
    cnt_veadr = 0;
    cnt_viadr = 0;
    //image_ready = false;
    segmentok = true;

    for(int k = 0; k < 5; k++)
    {
        stat_max_ch[k] = 0;
        stat_min_ch[k] = 999999999;
        stat_max_norm_ch[k] = 0;
        stat_min_norm_ch[k] = 999999999;
        list_stat_max_ch[k] = 0;
        list_stat_min_ch[k] = 999999999;
        active_pixels[k] = 0;
    }

//    list_stat_3_0_max_ch = 0;
//    list_stat_3_1_max_ch = 0;
//    list_stat_3_0_min_ch = 9999999;
//    list_stat_3_1_min_ch = 9999999;

//    stat_3_0_max_ch = 0;
//    stat_3_1_max_ch = 0;
//    stat_3_0_min_ch = 9999999;
//    stat_3_1_min_ch = 9999999;

    for (int i=0; i < 5; i++)
    {
        for (int j=0; j < 1024; j++)
        {
            this->segment_stats_ch[i][j] = 0;
            this->lut_ch[i][j] = 0;
        }
    }

    for (int j=0; j < 1080; j++)
    {
        this->channel_3a_3b[j] = false;
    }

    segmentselected = false;
    segmentshow = false;
    histogrammethod = 0;
}


void Segment::CalculateCornerPoints()
{

    // first line ///////////////////////////////////////

    QEci qeci;
    if(qsgp4.isNull())
        qDebug() << "qsgp4 is NULL !!!";
    qsgp4->getPosition(minutes_since_state_vector, qeci);
    QGeodetic qgeo = qeci.ToGeo();
    cornerpointcenter1 = qgeo;



    QVector3D pos;
    QVector3D d3pos = qeci.GetPos_f();
    QVector3D d3vel = qeci.GetVel_f();

    LonLat2PointRad(qgeo.latitude, qgeo.longitude, &pos, 1.001f);
    vec1.setX(pos.x());
    vec1.setY(pos.y());
    vec1.setZ(pos.z());
    vec1.normalize();

    QVector3D d3posnorm = d3pos.normalized();
    QMatrix4x4 mat;
    QVector3D d3scan;

    double e = qtle->Eccentricity();
    double epow2 = e * e;
    double epow3 = e * e * e;

    double span = qeci.GetDate().spanSec(qtle->Epoch());
    double M = fmod(qtle->MeanAnomaly() + (TWOPI * (span/qtle->Period())), TWOPI);
    double C = (2*e - epow3/4)*sin(M) + (5*epow2/4)*sin(2*M) + (13*epow3/12)*sin(3*M);
    double trueAnomaly = M + C;
    double PSO = fmod(qtle->ArgumentPerigee() + trueAnomaly, TWOPI);

    //qDebug() << QString("minutes_since_state_vector = %1 in CalculateCornerPoints").arg(minutes_since_state_vector);

    if (segtype == SEG_HRP || segtype == SEG_METOP || segtype == SEG_OLCIEFR || segtype == SEG_OLCIERR || segtype == SEG_SLSTR ||
            segtype == SEG_HRPT_METOPA || segtype == SEG_HRPT_METOPB || segtype == SEG_HRPT_M01 || segtype == SEG_HRPT_M02 || segtype == SEG_HRPT_NOAA19 ||
            segtype == SEG_DATAHUB_OLCIEFR || segtype == SEG_DATAHUB_OLCIERR || segtype == SEG_DATAHUB_SLSTR )
    {
        double pitch_steering_angle = - 0.002899 * sin( 2 * PSO);
        double roll_steering_angle = 0.00089 * sin(PSO);
        double yaw_factor = 0.068766 * cos(PSO);
        double yaw_steering_angle = 0.068766 * cos(PSO) * (1 - yaw_factor * yaw_factor/3);

        //qDebug() << QString("minutes_since_state_vector = %1 yaw_steering_angle = %2").arg(minutes_since_state_vector).arg(yaw_steering_angle*180.0/PI);

        mat.setToIdentity();
        mat.rotate(yaw_steering_angle * 180/PI, d3pos);  // yaw
        mat.rotate(roll_steering_angle * 180/PI, d3vel); // roll
        mat.rotate(pitch_steering_angle * 180/PI, QVector3D::crossProduct(d3pos,d3vel)); // pitch
        d3scan = mat * QVector3D::crossProduct(d3pos,d3vel);
    }
    else
        d3scan = QVector3D::crossProduct(d3pos,d3vel);


    QVector3D d3scannorm = d3scan.normalized();

    // VIIRS swath = 112.56°
    //
    //double delta = 0.0009439882 * 1023.5;// (in rad) for AVHRR
    double delta1;
    double delta2;

    if(segtype == SEG_VIIRSM || segtype == SEG_VIIRSDNB)
    {
        delta1 = 56.28 * PI / 180.0;  // (in rad) for VIIRS
        delta2 = delta1;
    }
    else if(segtype == SEG_OLCIEFR || segtype == SEG_OLCIERR || segtype == SEG_SLSTR || segtype == SEG_DATAHUB_OLCIEFR || segtype == SEG_DATAHUB_OLCIERR || segtype == SEG_DATAHUB_SLSTR)
    {
        delta2 = 22.1 * PI / 180.0;  // see page 97 of Sentinel-3 User Handbook
        delta1 = 46.5 * PI / 180.0;
    }
    else
    {
        delta1 = 0.0009439882 * 1023.5;
        delta2 = delta1;
    }

    double r = d3pos.length();
    double sindelta = sin(-delta1);
    double dd = r * cos(-delta1) - sqrt(XKMPER * XKMPER - r * r * sindelta * sindelta);
    QVector3D d3d = - d3posnorm * cos(-delta1) * dd + d3scannorm * sin(-delta1) * dd;
    QVector3D d3earthposfirst = d3pos + d3d;

    QEci qecifirst1(d3earthposfirst, d3vel, qsensingstart);
    cornerpointfirst1 = qecifirst1.ToGeo();

    sindelta = sin(delta2);
    dd = r * cos(delta2) - sqrt(XKMPER * XKMPER - r * r * sindelta * sindelta);
    d3d = - d3posnorm * cos(delta2) * dd + d3scannorm * sin(delta2) * dd;

    QVector3D d3earthposlast = d3pos + d3d;

    QEci qecilast1(d3earthposlast, d3vel, qsensingstart);
    cornerpointlast1 = qecilast1.ToGeo();

    // last line ///////////////////////////////////////////////////////////

    qsgp4->getPosition(minutes_since_state_vector + minutes_sensing, qeci);
    qgeo = qeci.ToGeo();
    cornerpointcenter2 = qgeo;

    d3pos = qeci.GetPos_f();
    d3vel = qeci.GetVel_f();

    LonLat2PointRad(qgeo.latitude, qgeo.longitude, &pos, 1.001f);
    vec2.setX(pos.x());
    vec2.setY(pos.y());
    vec2.setZ(pos.z());
    vec2.normalize();

    d3posnorm = d3pos.normalized();

    span = qeci.GetDate().spanSec(qtle->Epoch());
    M = fmod(qtle->MeanAnomaly() + (TWOPI * (span/qtle->Period())), TWOPI);
    C = (2*e - epow3/4)*sin(M) + (5*epow2/4)*sin(2*M) + (13*epow3/12)*sin(3*M);
    trueAnomaly = M + C;
    PSO = fmod(qtle->ArgumentPerigee() + trueAnomaly, TWOPI);

    if (segtype == SEG_HRP || segtype == SEG_METOP || segtype == SEG_OLCIEFR || segtype == SEG_OLCIERR || segtype == SEG_SLSTR ||
                segtype == SEG_HRPT_METOPA || segtype == SEG_HRPT_METOPB || segtype == SEG_HRPT_M01 || segtype == SEG_HRPT_M02 || segtype == SEG_HRPT_NOAA19 ||
            segtype == SEG_DATAHUB_OLCIEFR || segtype == SEG_DATAHUB_OLCIERR || segtype == SEG_DATAHUB_SLSTR )
    {
        double pitch_steering_angle = - 0.002899 * sin( 2 * PSO);
        double roll_steering_angle = 0.00089 * sin(PSO);
        double yaw_factor = 0.068766 * cos(PSO);
        double yaw_steering_angle = 0.068766 * cos(PSO) * (1 - yaw_factor * yaw_factor/3);

        mat.setToIdentity();
        mat.rotate(yaw_steering_angle * 180/PI, d3pos);  // yaw
        mat.rotate(roll_steering_angle * 180/PI, d3vel); // roll
        mat.rotate(pitch_steering_angle * 180/PI, QVector3D::crossProduct(d3pos,d3vel)); // pitch
        d3scan = mat * QVector3D::crossProduct(d3pos,d3vel);
    }
    else
    d3scan = QVector3D::crossProduct(d3pos,d3vel);


    d3scannorm = d3scan.normalized();

    r = d3pos.length();
    sindelta = sin(-delta1);
    dd = r * cos(-delta1) - sqrt(XKMPER * XKMPER - r * r * sindelta * sindelta);
    d3d = - d3posnorm * cos(-delta1) * dd + d3scannorm * sin(-delta1) * dd;
    d3earthposfirst = d3pos + d3d;

    QEci qecifirst2(d3earthposfirst, d3vel, qsensingend);
    cornerpointfirst2 = qecifirst2.ToGeo();

    sindelta = sin(delta2);
    dd = r * cos(delta2) - sqrt(XKMPER * XKMPER - r * r * sindelta * sindelta);
    d3d = - d3posnorm * cos(delta2) * dd + d3scannorm * sin(delta2) * dd;

    d3earthposlast = d3pos + d3d;

    QEci qecilast2(d3earthposlast, d3vel, qsensingend);
    cornerpointlast2 = qecilast2.ToGeo();
}

void Segment::CalculateDetailCornerPoints()
{


    double statevec = minutes_since_state_vector;
    QSgp4Date sensing = qsensingstart;

    while(statevec <= minutes_since_state_vector + minutes_sensing)
    {
        setupVector(statevec, sensing);
        statevec = statevec + 1.0;
        sensing.AddMin(1.0);
    }

}


void Segment::setupVector(double statevec, QSgp4Date sensing)
{

    QEci qeci;

    if(qsgp4.isNull())
        qDebug() << "qsgp4 is NULL !!!";
    qsgp4->getPosition(statevec, qeci);
    QGeodetic qgeo = qeci.ToGeo();

    QVector3D pos;
    QVector3D d3pos = qeci.GetPos_f();
    QVector3D d3vel = qeci.GetVel_f();

    QVector3D d3posnorm = d3pos.normalized();
    QMatrix4x4 mat;
    QVector3D d3scan;

    LonLat2PointRad(qgeo.latitude, qgeo.longitude, &pos, 1.001f);
    QVector3D vec;
    vec.setX(pos.x());
    vec.setY(pos.y());
    vec.setZ(pos.z());
    vec.normalize();

    vecvector.append(vec);

    double e = qtle->Eccentricity();
    double epow2 = e * e;
    double epow3 = e * e * e;

    double span = qeci.GetDate().spanSec(qtle->Epoch());
    double M = fmod(qtle->MeanAnomaly() + (TWOPI * (span/qtle->Period())), TWOPI);
    double C = (2*e - epow3/4)*sin(M) + (5*epow2/4)*sin(2*M) + (13*epow3/12)*sin(3*M);
    double trueAnomaly = M + C;
    double PSO = fmod(qtle->ArgumentPerigee() + trueAnomaly, TWOPI);

    double pitch_steering_angle = - 0.002899 * sin( 2 * PSO);
    double roll_steering_angle = 0.00089 * sin(PSO);
    double yaw_factor = 0.068766 * cos(PSO);
    double yaw_steering_angle = 0.068766 * cos(PSO) * (1 - yaw_factor * yaw_factor/3);

    mat.setToIdentity();
    mat.rotate(yaw_steering_angle * 180/PI, d3pos);  // yaw
    mat.rotate(roll_steering_angle * 180/PI, d3vel); // roll
    mat.rotate(pitch_steering_angle * 180/PI, QVector3D::crossProduct(d3pos,d3vel)); // pitch
    d3scan = mat * QVector3D::crossProduct(d3pos,d3vel);

    //d3scan = QVector3D::crossProduct(d3pos,d3vel);

    QVector3D d3scannorm = d3scan.normalized();

    double delta1, delta2;
    if(segtype == SEG_OLCIEFR || segtype == SEG_OLCIERR || segtype == SEG_SLSTR ||
            segtype == SEG_DATAHUB_OLCIEFR || segtype == SEG_DATAHUB_OLCIERR || segtype == SEG_DATAHUB_SLSTR)
    {
        delta2 = 23.0 * PI / 180.0;
        delta1 = 47.0 * PI / 180.0;
    }
    else
    {
        delta2 = 56.0 * PI / 180.0;
        delta1 = 56.0 * PI / 180.0;

    }

    double r = d3pos.length();
    double sindelta = sin(-delta1);
    double cosdelta = cos(-delta1);
    double dd = r * cosdelta - sqrt(XKMPER * XKMPER - r * r * sindelta * sindelta);
    QVector3D d3d = - d3posnorm * cosdelta * dd + d3scannorm * sindelta * dd;
    QVector3D d3earthposfirst = d3pos + d3d;

    QEci qecifirst(d3earthposfirst, d3vel, sensing);
    vectorfirst.append(qecifirst.ToGeo());

    sindelta = sin(delta2);
    cosdelta = cos(delta2);
    dd = r * cosdelta - sqrt(XKMPER * XKMPER - r * r * sindelta * sindelta);
    d3d = - d3posnorm * cosdelta * dd + d3scannorm * sindelta * dd;

    QVector3D d3earthposlast = d3pos + d3d;

    QEci qecilast(d3earthposlast, d3vel, sensing);
    vectorlast.append(qecilast.ToGeo());
}


Segment::~Segment()
{
    resetMemory();
}

void Segment::initializeMemory()
{
    qDebug() << QString("Allocating memory in InitializeMemory = %1!").arg(earth_views_per_scanline * NbrOfLines);
    qDebug() << QString("earth_views_per_scanline = %1 NbrOfLines = %2").arg(earth_views_per_scanline).arg(NbrOfLines);

    for(int k = 0; k < 5; k++)
    {
        ptrbaChannel[k].reset(new unsigned short[earth_views_per_scanline * NbrOfLines]);
        ptrbaChannelNormalized[k].reset(new unsigned short[earth_views_per_scanline * NbrOfLines]);
    }
}

void Segment::resetMemory()
{
    for(int k = 0; k < 5; k++)
    {
        ptrbaChannel[k].reset();
        ptrbaChannelNormalized[k].reset();
    }

    for(int k = 0; k < 3; k++)
    {
        ptrbaVIIRS[k].reset();
        ptrbaOLCI[k].reset();
        ptrbaOLCInormalized[k].reset();
        ptrbaSLSTR[k].reset();
        ptrbaSLSTRnormalized[k].reset();
    }
    ptrbaVIIRSDNB.reset();

    projectionCoordX.reset();
    projectionCoordY.reset();
    projectionCoordValue.reset();
    projectionCoordValueRed.reset();
    projectionCoordValueGreen.reset();
    projectionCoordValueBlue.reset();


    earthloc_lon.reset();
    earthloc_lat.reset();
    solar_zenith_angle.reset();
}

bool Segment::composeColorImage()
{
    return(bandlist.at(0));
}

void Segment::RenderSatPath(QPainter *painter, QColor color)
{

    int devwidth = (painter->device())->width();
    int devheight = (painter->device())->height();
    int posx, posy;
    int posx1, posy1;
    int save_posx;

    painter->setFont( QFont( "helvetica", 12) );

    double tsince;

    tsince = 0;
    double id;

    QEci qeci;

/*    qsgp4->getPosition(0, qeci);
    QGeodetic qgeo = qeci.ToGeo();

    sphericalToPixel( qgeo.longitude, qgeo.latitude, posx, posy, devwidth, devheight );
    painter->setPen( Qt::red );
    painter->setBrush( Qt::NoBrush );
    painter->drawEllipse( posx -  5 , posy - 5, 10, 10 );
    //painter->drawLine(posx, 0, posx, devheight);

    qsgp4->getPosition(qtle->Period(), qeci);
    qgeo = qeci.ToGeo();

    sphericalToPixel( qgeo.longitude, qgeo.latitude, posx, posy, devwidth, devheight );
    painter->setPen( Qt::yellow );
    painter->setBrush( Qt::NoBrush );
    painter->drawEllipse( posx -  5 , posy - 5, 10, 10 );
    //painter->drawLine(posx, 0, posx, devheight);
*/
    qsgp4->getPosition(minutes_since_state_vector, qeci);
    QGeodetic qgeo = qeci.ToGeo();

    sphericalToPixel( qgeo.longitude, qgeo.latitude, posx, posy, devwidth, devheight );
    posx1=posx;
    posy1=posy;
    save_posx = posx;
    //painter->setPen( Qt::blue );
    //painter->setBrush( Qt::NoBrush );
    //painter->drawEllipse( posx -  10 , posy - 10, 20, 20 );

    for( id = minutes_since_state_vector; id <= minutes_since_state_vector + minutes_sensing; id+=0.2 )
    {

        if (segmentselected)
        {
            QPen pen(Qt::cyan, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(pen);
        }
        else
        {
            QPen pen( color, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(pen);
        }

        qsgp4->getPosition(id, qeci);
        QGeodetic qgeo = qeci.ToGeo();

        sphericalToPixel( qgeo.longitude, qgeo.latitude, posx, posy, devwidth, devheight );

        if (((save_posx >= devwidth*0.9) && (posx < devwidth*0.1)) ||
        ((save_posx <= devwidth*0.1) && (posx > devwidth*0.9)) ||
        (posy <= devheight*0.02) || (posy >= devheight*0.98) )
        {
            //painter->drawEllipse( posx -  2 , posy - 2, 4, 4 );
            //painter->moveTo( posx, posy );
            posx1=posx;
            posy1=posy;
        }
       else
       {
           //painter->drawEllipse( posx -  2 , posy - 2, 4, 4 );
           //painter->lineTo( posx, posy );
           painter->drawLine(posx1, posy1, posx, posy);
           posx1=posx;
           posy1=posy;
       }

       save_posx = posx;

    }
}




void Segment::sphericalToPixel(double lon, double lat, int &x, int &y, int devwidth, int devheight)
{

  if (lon > PI) lon -= TWOPI;
  else if (lon < -PI) lon += TWOPI;

  x = (int) ((lon - (-PI)) * devwidth/TWOPI + 0.5);

  if (x >= devwidth) x -= devwidth;
  else if (x < 0) x += devwidth;

  y = (int) ((PI/2 - lat) * devheight/PI + 0.5);
  if (y >= devheight) y = devheight - 1;

}

void Segment::drawLineCyl(double lon1, double lat1, double lon2, double lat2, QPainter *painter)
{
    int x1, y1, x2, y2;

    int devwidth = (painter->device())->width();
    int devheight = (painter->device())->height();

    if (lon1 > PI) lon1 -= TWOPI;
    else if (lon1 < -PI) lon1 += TWOPI;
    if (lon2 > PI) lon2 -= TWOPI;
    else if (lon2 < -PI) lon2 += TWOPI;

    x1 = (int) ((lon1 - (-PI)) * devwidth/TWOPI + 0.5);
    y1 = (int) ((PI/2 - lat1) * devheight/PI + 0.5);
    if (y1 >= devheight) y1 = devheight - 1;

    x2 = (int) ((lon2 - (-PI)) * devwidth/TWOPI + 0.5);
    y2 = (int) ((PI/2 - lat2) * devheight/PI + 0.5);
    if (y2 >= devheight) y2 = devheight - 1;

    double a, b, dy0;
    int y0;

   if (abs(x1 - x2 ) <= devwidth*0.4)
        painter->drawLine( x1, y1, x2, y2);
   else
   {
       //qDebug() << QString("x1 = %1, y1 = %2, x2 = %3, y2 = %4").arg( x1, 0, 10).arg( y1, 0, 10).arg( x2, 0, 10).arg( y2, 0, 10);
       a = (double)(y2-y1)/(double)(devwidth + x2 - x1);
       b = y1 - a*x1;
       dy0 =  a*devwidth + b;
       //qDebug() << QString("a = %1, b = %2, devwidth = %3").arg( a, 11, 'f', 7).arg( b, 11, 'f', 7).arg( devwidth, 0, 10);
       //qDebug() << QString("y2 - y1 = %1, devwidth + x2 -x1 = %2").arg( y2 - y1, 0, 10).arg( devwidth + x2 -x1, 0, 10);
       y0 = (int)( dy0 + 0.5);

       if (x1 > devwidth/2)
       {
           painter->drawLine( x1, y1, devwidth, y0);
           painter->drawLine( 0, y0, x2, y2);
       }
       else
       {
           painter->drawLine( x2, y2, devwidth, y0);
           painter->drawLine( 0, y0, x1, y1);
       }
   }
}

void Segment::RenderPosition(QPainter *painter)
{
    int posx1, posy1;
    int posx2, posy2;

    QEci qeci;
    qsgp4->getPosition(minutes_since_state_vector, qeci);
    QGeodetic qgeo = qeci.ToGeo();

    double lon_1 = qgeo.longitude;
    double lat_1 = qgeo.latitude;
    double alt_1 = qgeo.altitude;
    painter->setPen( Qt::yellow );
    painter->setBrush( Qt::yellow );

    int devwidth = (painter->device())->width();
    int devheight = (painter->device())->height();

    //if (lon_start_rad == 0 && lat_start_rad == 0)
        sphericalToPixel( lon_1, lat_1, posx1, posy1, devwidth, devheight );
    //else
        sphericalToPixel( lon_start_rad, lat_start_rad, posx2, posy2, devwidth, devheight );
    if (this->segmentselected)
    {
        painter->setBrush( Qt::blue );
        painter->setPen( Qt::blue );
        painter->drawEllipse( posx1-3, posy1-3, 6, 6 );
    }
    else
    {
        painter->setPen(Qt::yellow);
        painter->setBrush( Qt::yellow );
        painter->drawEllipse( posx1-2, posy1-2, 4, 4 );

        //painter->setPen(Qt::red);
        //painter->setBrush( Qt::red );
        //painter->drawEllipse( posx2-2, posy2-2, 4, 4 );

    }

}

//void Segment::DrawArcGL(float cx, float cy, float r, float start_angle, float arc_angle, int num_segments)
//{
//        float theta = arc_angle / float(num_segments - 1);//theta is now calculated from the arc angle instead, the - 1 bit comes from the fact that the arc is open

//        float tangetial_factor = tanf(theta);

//        float radial_factor = cosf(theta);


//        float x = r * cosf(start_angle);//we now start at the start angle
//        float y = r * sinf(start_angle);

//        glBegin(GL_LINE_STRIP);//since the arc is not a closed curve, this is a strip now
//        for(int ii = 0; ii < num_segments; ii++)
//        {
//                glVertex2f(x + cx, y + cy);

//                float tx = -y;
//                float ty = x;

//                x += tx * tangetial_factor;
//                y += ty * tangetial_factor;

//                x *= radial_factor;
//                y *= radial_factor;
//        }
//        glEnd();
//}

int Segment::pnpoly(int nvert, QPoint points[], int testx, int testy)
{
    int i, j, c = 0;
    for (i = 0, j = nvert-1; i < nvert; j = i++) {
        if ( ((points[i].y() > testy) != (points[j].y() > testy)) &&
             (testx < (points[j].x() - points[i].x() ) * (testy-points[i].y()) / (points[j].y() - points[i].y()) + points[i].x()) )
            c = !c;
    }
    return c;
}

bool Segment::ToggleSelected()
{
    if (this->segmentselected)
        this->segmentselected = false;
    else
        this->segmentselected = true;
    return this->segmentselected;
}

void Segment::RenderEarthLocationsGL()
{

}

int Segment::ReadNbrOfLines()
{
    return 0;
}

void Segment::NormalizeSegment()
{
    for(int k = 0; k < 5; k++)
    {
        for (int j = 0; j < 1024; j++)
        {
            segment_stats_ch[k][j] = 0;
        }
    }

    for(int k = 0; k < 5; k++)
    {
        for (int line = 0; line < this->NbrOfLines; line++)
        {
            for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
            {
                int pixel = *(this->ptrbaChannel[k].data() + line * earth_views_per_scanline + pixelx);
                int pixcalc = (pixel - list_stat_min_ch[k]) * 1023 / (list_stat_max_ch[k] - list_stat_min_ch[k]);
                pixcalc = pixcalc > 1023 ? 1023 : pixcalc;
                pixcalc = pixcalc < 0 ? 0 : pixcalc;
                this->ptrbaChannel[k][line * earth_views_per_scanline + pixelx] = pixcalc;
                segment_stats_ch[k][pixcalc]++;

            }
        }
    }

    //int ret1 = imageptrs->CLAHE(this->ptrbaChannel[0], 2048, this->NbrOfLines, 0, 1023, 16, 10, 256, 2);
    //int ret2 = imageptrs->CLAHE(this->ptrbaChannel[1], 2048, this->NbrOfLines, 0, 1023, 16, 10, 256, 2);
    //int ret3 = imageptrs->CLAHE(this->ptrbaChannel[2], 2048, this->NbrOfLines, 0, 1023, 16, 10, 256, 2);


    float scale = 1023.0 / (this->NbrOfLines * earth_views_per_scanline);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE

    unsigned long sum_ch[5];

    for (int i=0; i < 5; i++)
    {
        sum_ch[i] = 0;
    }


    for( int i = 0; i < 1024; i++)
    {
        for(int k = 0; k < 5; k++)
        {
            sum_ch[k] += segment_stats_ch[k][i];
            lut_ch[k][i] = (quint16)(sum_ch[k] * scale);
            if (lut_ch[k][i] > 1023)
                lut_ch[k][i] = 1023;
        }
    }
}

void Segment::RenderSegmentlineInTexture( int channel, int nbrLine, int nbrTotalLine )
{
    // nbrLine = nbr of line in segment
    // nbrTotalLine = nbr of lines in complete picture


    QEci qeci;
    qsgp4->getPosition(minutes_since_state_vector + (double)(nbrLine) / (6.0 * 60.0), qeci);
    QGeodetic qgeo = qeci.ToGeo();

    double julian_sensing_time = julian_sensing_start + (double)(nbrLine) / ( 6.0 * 60.0 * 60.0 * 24.0);
    //qDebug() << QString("julian sensing time = %1").arg(julian_sensing_time);
    QSgp4Date juldate1(julian_sensing_time, true);


    QVector3D d3vel = qeci.GetVel_f();
    QVector3D d3pos = qeci.GetPos_f();

    QVector3D d3posnorm = d3pos.normalized();

    QMatrix4x4 mat;

    //METOP-A
    //1 29499U 06044A   11289.45577767  .00000056  00000-0  45622-4 0  5536
    //2 29499  98.7382 346.7056 0001985  53.8175 306.3175 14.21469318258958


    double e = qtle->Eccentricity();
    double epow2 = e * e;
    double epow3 = e * e * e;

    double span = qeci.GetDate().spanSec(qtle->Epoch());
    double M = fmod(qtle->MeanAnomaly() + (TWOPI * (span/qtle->Period())), TWOPI);
    double C = (2*e - epow3/4)*sin(M) + (5*epow2/4)*sin(2*M) + (13*epow3/12)*sin(3*M);
    double trueAnomaly = M + C;
    double PSO = fmod(qtle->ArgumentPerigee() + trueAnomaly, TWOPI);


    QVector3D d3scan;
    if (segment_type == "HRP" || segment_type == "HRPTMETOPA" || segment_type == "HRPTMETOPB" || segment_type == "HRPTM01" || segment_type == "HRPTM02" || segment_type == "HRPTNOAA19")
    {
        double pitch_steering_angle = + 0.002899 * sin( 2 * PSO);
        double roll_steering_angle = - 0.00089 * sin(PSO);
        double yaw_factor = 0.068766 * cos(PSO);
        double yaw_steering_angle = 0.068766 * cos(PSO) * (1 - yaw_factor * yaw_factor/3);
        //qDebug() << QString("yaw steering angle (degrees) = %1").arg(yaw_steering_angle * 180/PI);

        mat.setToIdentity();
        mat.rotate(yaw_steering_angle * 180/PI, d3pos);  // yaw
        mat.rotate(roll_steering_angle * 180/PI, d3vel); // roll
        mat.rotate(pitch_steering_angle * 180/PI, QVector3D::crossProduct(d3pos,d3vel)); // pitch
        d3scan = mat * QVector3D::crossProduct(d3pos,d3vel);
    }
    else
        d3scan = QVector3D::crossProduct(d3pos,d3vel);

    QVector3D d3scannorm = d3scan.normalized();

    double delta = 0.0009457 * 1023.5; //0.0009439882 * 1023.5;

    double r = d3pos.length();
    double sindelta = sin(-delta);
    double dd = r * cos(-delta) - sqrt(XKMPER * XKMPER - r * r * sindelta * sindelta);
    QVector3D d3d = - d3posnorm * cos(-delta) * dd + d3scannorm * sin(-delta) * dd;

    //qDebug() << QString("a = %1 b = %2").arg(XKMPER * XKMPER).arg( r * r * sindelta * sindelta);
    QVector3D d3earthposfirst = d3pos + d3d;

    QEci qecifirst(d3earthposfirst, d3vel, juldate1);
    QGeodetic qgeofirst = qecifirst.ToGeo();

    sindelta = sin(delta);
    dd = r * cos(delta) - sqrt(XKMPER * XKMPER - r * r * sindelta * sindelta);
    d3d = - d3posnorm * cos(delta) * dd + d3scannorm * sin(delta) * dd;

    QVector3D d3earthposlast = d3pos + d3d;

    QEci qecilast(d3earthposlast, d3vel, juldate1);
    QGeodetic qgeolast = qecilast.ToGeo();

    RenderSegmentlineInTextureRad(channel, qgeofirst.latitude, qgeofirst.longitude, qgeolast.latitude, qgeolast.longitude, qgeo.altitude, nbrTotalLine);

}


void Segment::RenderSegmentlineInTextureRad(int channel, double lat_first, double lon_first, double lat_last, double lon_last, double altitude, int heightintotalimage)
{
    QRgb *row_col;

    double latdiff = sin((lat_first-lat_last)/2);
    double londiff = sin((lon_first-lon_last)/2);

    double sinpsi = sqrt(latdiff * latdiff + cos(lat_first)*cos(lat_last)*londiff * londiff);
    double psi = asin(sinpsi);
    double delta = atan(sinpsi/((( XKMPER + altitude)/ XKMPER) - cos(psi)));

    //qDebug() << QString("earth_views_per_scanline = %1").arg(earth_views_per_scanline);

    //double deltax = delta / 204;
    double deltax = delta / (earth_views_per_scanline / 2);
    double psix;
    double psix1, psix2;
    double dx;
    double r = XKMPER + altitude;  // earth_location_altitude[0]/10;
    double sindeltax;
    double lonpos, latpos, dlon, tc;
    double lonpos1, latpos1, lonpos2, latpos2, dlon1, dlon2;

    tc = fmod(atan2(sin(lon_first-lon_last)*cos(lat_last), cos(lat_first)*sin(lat_last)-sin(lat_first)*cos(lat_last)*cos(lon_first-lon_last)) , 2 * PI);

    sindeltax = sin(delta);
    dx = r * cos(delta) - sqrt( XKMPER * XKMPER - r * r * sindeltax * sindeltax );
    psix = asin( dx * sindeltax / XKMPER );



    if (channel == 1)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[0]->scanLine(heightintotalimage);
    else if (channel == 2)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[1]->scanLine(heightintotalimage);
    else if (channel == 3)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[2]->scanLine(heightintotalimage);
    else if (channel == 4)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[3]->scanLine(heightintotalimage);
    else if (channel == 5)
        row_col = (QRgb*)imageptrs->ptrimagecomp_ch[4]->scanLine(heightintotalimage);
    else if (channel == 6)
        row_col = (QRgb*)imageptrs->ptrimagecomp_col->scanLine(heightintotalimage);
    else if (channel == 10)
        row_col = (QRgb*)imageptrs->ptrimageViirsM->scanLine(heightintotalimage);
    else if (channel == 11)
        row_col = (QRgb*)imageptrs->ptrimageViirsDNB->scanLine(heightintotalimage);


    QColor rgb;
    QColor rgb1, rgb2;
    int posx, posy;
    int posx1, posy1, posx2, posy2;

    QMutexLocker locker(&g_mutex);

    QPainter fb_painter(imageptrs->pmOut);

    int devwidth = (fb_painter.device())->width();
    int devheight = (fb_painter.device())->height();

    fb_painter.setPen( Qt::black );
    fb_painter.setBrush( Qt::NoBrush );

    int earthviews = earth_views_per_scanline/2;

    for (int pix = 0 ; pix < earthviews; pix+=(channel == 10 ? 2 : 1))
    {

        sindeltax = sin(deltax * pix);
        dx = r * cos(deltax * pix) - sqrt( XKMPER * XKMPER - r * r * sindeltax * sindeltax );
        psix1 = psi + asin( dx * sindeltax / XKMPER );
        psix2 = psi - asin( dx * sindeltax / XKMPER );

        latpos1 = asin(sin(lat_first)*cos(psix1)+cos(lat_first)*sin(psix1)*cos(tc));
        dlon1=atan2(sin(tc)*sin(psix1)*cos(lat_first),cos(psix1)-sin(lat_first)*sin(latpos1));
        lonpos1=fmod( lon_first-dlon1 + PI,2*PI )-PI;

        latpos2 = asin(sin(lat_first)*cos(psix2)+cos(lat_first)*sin(psix2)*cos(tc));
        dlon2=atan2(sin(tc)*sin(psix2)*cos(lat_first),cos(psix2)-sin(lat_first)*sin(latpos2));
        lonpos2=fmod( lon_first-dlon2 + PI,2*PI )-PI;

        sphericalToPixel( lonpos1, latpos1, posx1, posy1, devwidth, devheight );
        rgb1.setRgb(qRed(row_col[earthviews+pix]), qGreen(row_col[earthviews+pix]), qBlue(row_col[earthviews+pix]));
        fb_painter.setPen(rgb1);
        fb_painter.drawPoint( posx1 , posy1 );

        sphericalToPixel( lonpos2, latpos2, posx2, posy2, devwidth, devheight );
        rgb2.setRgb(qRed(row_col[earthviews-pix]), qGreen(row_col[earthviews-pix]), qBlue(row_col[earthviews-pix]));
        fb_painter.setPen(rgb2);
        fb_painter.drawPoint( posx2 , posy2 );

    }

    fb_painter.end();

    //qDebug() << QString("lon_first = %1 ; lat_first = %2 ; posx = %3 ; posy = %4").arg(lon_first).arg(lat_first).arg(posx).arg(posy);
}

void Segment::ComposeSegmentImage()
{

    QRgb *row_ch[5];
    QRgb *row_col;
    quint16 pixel[5];
    quint16 R_value, G_value, B_value;
    bool ok;
    quint16 indexout[3];

    // see FormImage::displayImage(eImageType channel) for the mutex
    // also in SegmentOLCI::ComposeSegmentImage , SegmentVIIRSM::ComposeSegmentImage and SegmentVIIRSDNB::ComposeSegmentImageWindow

    qDebug() << QString("start ComposeSegmentImage startLineNbr = %1 nbr of lines = %2").arg(this->startLineNbr).arg(this->NbrOfLines);
    int startheight = this->startLineNbr;

    QStringList channellist;
    QStringList inverse;
    if (opts.buttonMetop || opts.buttonMetopAhrpt || opts.buttonMetopBhrpt || opts.buttonM01hrpt || opts.buttonM02hrpt)
    {
        channellist = opts.channellistmetop;
        inverse = opts.metop_invlist;
    }
    else
    if (opts.buttonNoaa || opts.buttonNoaa19hrpt)
    {
        channellist = opts.channellistnoaa;
        inverse = opts.noaa_invlist;
    }
    else
    if (opts.buttonGAC)
    {
        channellist = opts.channellistgac;
        inverse = opts.gac_invlist;
    }
    else
    if (opts.buttonHRP)
    {
        channellist = opts.channellisthrp;
        inverse = opts.hrp_invlist;
    }

    int half_earth_views = earth_views_per_scanline / 2;

    for (int line = 0; line < this->NbrOfLines; line++)
    {
        row_ch[0] = (QRgb*)imageptrs->ptrimagecomp_ch[0]->scanLine(startheight + line);
        row_ch[1] = (QRgb*)imageptrs->ptrimagecomp_ch[1]->scanLine(startheight + line);
        row_ch[2] = (QRgb*)imageptrs->ptrimagecomp_ch[2]->scanLine(startheight + line);
        row_ch[3] = (QRgb*)imageptrs->ptrimagecomp_ch[3]->scanLine(startheight + line);
        row_ch[4] = (QRgb*)imageptrs->ptrimagecomp_ch[4]->scanLine(startheight + line);
        row_col = (QRgb*)imageptrs->ptrimagecomp_col->scanLine(startheight + line);


        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
        {
            for( int k = 0; k < 5; k++)
            {

                pixel[k] = *(this->ptrbaChannel[k].data() + line * earth_views_per_scanline + pixelx);
                if(histogrammethod == CMB_HISTO_NONE_95) // 95%
                    indexout[k] =  (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel[k] - imageptrs->minRadianceIndex[k] ) / (float)(imageptrs->maxRadianceIndex[k] - imageptrs->minRadianceIndex[k])), 0), 1023);
                else if(histogrammethod == CMB_HISTO_NONE_100)
                    indexout[k] = pixel[k];
                else if(histogrammethod == CMB_HISTO_EQUALIZE)
                    indexout[k] = imageptrs->lut_ch[k][pixel[k]];

                R_value = indexout[k]/4;
                if (inverse.at(k) == "1")
                {
                    row_ch[k][pixelx] = qRgb(255 - R_value, 255 - R_value, 255 - R_value);
                }
                else
                {
                    row_ch[k][pixelx] = qRgb(R_value, R_value, R_value);
                }
            }

            for( int k = 0; k < 5; k++ )
            {
                switch (channellist.at(k).toInt(&ok))
                {
                case 0:
                    break;
                case 1:
                    if (inverse.at(k) == "1")
                        R_value = 255 - (indexout[k]/4);
                    else
                        R_value = indexout[k]/4;
                     break;
                case 2:
                    if (inverse.at(k) == "1")
                        G_value = 255 - (indexout[k]/4);
                    else
                        G_value = indexout[k]/4;
                    break;
                case 3:
                    if (inverse.at(k) == "1")
                        B_value = 255 - (indexout[k]/4);
                    else
                        B_value = indexout[k]/4;
                    break;
                }
            }


            if(opts.sattrackinimage)
            {
               if (pixelx == half_earth_views - 1 || pixelx == half_earth_views)
               {
                   row_col[pixelx] = qRgb(255, 0, 0);
               }
                else
               {
                   row_col[pixelx] = qRgb(R_value, G_value, B_value);
               }
            }
            else
            {
                row_col[pixelx] = qRgb(R_value, G_value, B_value);
            }
        }

        if(opts.imageontextureOnAVHRR)
        {
            this->RenderSegmentlineInTexture( opts.channelontexture, line, startheight + line );
            opts.texture_changed = true;
        }
    }

    qDebug() << QString("--> na ComposeSegmentImage startLineNbr = %1 nbr of lines = %2 segshow = %3").arg(this->startLineNbr).arg(this->NbrOfLines).arg(this->segmentshow);

}

qint32 Segment::getProjectionX(int line, int pixelx)
{
    return projectionCoordX[line * earth_views_per_scanline + pixelx];

//    switch(segtype)
//    {
//    case eSegmentType::SEG_METOP:
//    case eSegmentType::SEG_NOAA:
//    case eSegmentType::SEG_HRP:
//        return projectionCoordX[line * 2048 + pixelx];
//        break;
//    case eSegmentType::SEG_GAC:
//        return projectionCoordX[line * 409 + pixelx];
//        break;
//    case eSegmentType::SEG_VIIRSM:
//        return projectionCoordX[line * 3200 + pixelx];
//        break;
//    case eSegmentType::SEG_VIIRSDNB:
//        return projectionCoordX[line * 4064 + pixelx];
//        break;
//    case eSegmentType::SEG_OLCIEFR:
//    case eSegmentType::SEG_OLCIERR:
//        return projectionCoordX[line * earth_views_per_scanline + pixelx];
//        break;
//    }
}

qint32 Segment::getProjectionY(int line, int pixelx)
{
    return projectionCoordY[line * earth_views_per_scanline + pixelx];

//    switch(segtype)
//    {
//    case eSegmentType::SEG_METOP:
//    case eSegmentType::SEG_NOAA:
//    case eSegmentType::SEG_HRP:
//        return projectionCoordY[line * 2048 + pixelx];
//        break;
//    case eSegmentType::SEG_GAC:
//        return projectionCoordY[line * 409 + pixelx];
//        break;
//    case eSegmentType::SEG_VIIRSM:
//        return projectionCoordY[line * 3200 + pixelx];
//        break;
//    case eSegmentType::SEG_VIIRSDNB:
//        return projectionCoordY[line * 4064 + pixelx];
//        break;
//    case eSegmentType::SEG_OLCIEFR:
//    case eSegmentType::SEG_OLCIERR:
//        return projectionCoordY[line * earth_views_per_scanline + pixelx];
//        break;
//    }
}

QRgb Segment::getProjectionValue(int line, int pixelx)
{
    return projectionCoordValue[line * earth_views_per_scanline + pixelx];
}

quint16 Segment::getProjectionValueRed(int line, int pixelx)
{
    return projectionCoordValueRed[line * earth_views_per_scanline + pixelx];
}

quint16 Segment::getProjectionValueGreen(int line, int pixelx)
{
    return projectionCoordValueGreen[line * earth_views_per_scanline + pixelx];
}

quint16 Segment::getProjectionValueBlue(int line, int pixelx)
{
    return projectionCoordValueBlue[line * earth_views_per_scanline + pixelx];
}

void Segment::setBandandColor(QList<bool> band, QList<int> color, QList<bool> invert)
{
    bandlist = band;
    colorlist = color;
    invertlist = invert;
}

void Segment::initializeProjectionCoord()
{
    projectionCoordX.reset(new int[this->NbrOfLines * this->earth_views_per_scanline]);
    projectionCoordY.reset(new int[this->NbrOfLines * this->earth_views_per_scanline]);
    projectionCoordValue.reset(new QRgb[this->NbrOfLines * this->earth_views_per_scanline]);

    for( int i = 0; i < this->NbrOfLines; i++)
    {
        for( int j = 0; j < this->earth_views_per_scanline ; j++ )
        {
            projectionCoordX[i * this->earth_views_per_scanline + j] = 65535;
            projectionCoordY[i * this->earth_views_per_scanline + j] = 65535;
            projectionCoordValue[i * this->earth_views_per_scanline + j] = qRgba(0, 0, 0, 0);
        }
    }
}

void Segment::ComposeSegmentGVProjection(int inputchannel, int histogrammethod, bool normalized)
{

}

void Segment::ComposeSegmentLCCProjection(int inputchannel, int histogrammethod, bool normalized)
{

}

void Segment::ComposeSegmentSGProjection(int inputchannel, int histogrammethod, bool normalized)
{

}

void Segment::RecalculateProjection()
{

}

Segment *Segment::ReadDatasetsInMemory()
{
    return this;
}

Segment *Segment::ReadSegmentInMemory()
{
    return this;
}



/*
Distance between points

The great circle distance d between two points with coordinates {lat1,lon1} and {lat2,lon2} is given by:

d=acos(sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(lon1-lon2))

A mathematically equivalent formula, which is less subject to rounding error for short distances is:

d=2*asin(sqrt((sin((lat1-lat2)/2))^2 + cos(lat1)*cos(lat2)*(sin((lon1-lon2)/2))^2))

*/

/*
Course between points

We obtain the initial course, tc1, (at point 1) from point 1 to point 2 by the following. The formula fails if the initial point is a pole. We can special case this with:

IF (cos(lat1) < EPS)   // EPS a small number ~ machine precision
  IF (lat1 > 0)
     tc1= pi        //  starting from N pole
  ELSE
     tc1= 2*pi         //  starting from S pole
  ENDIF
ENDIF

For starting points other than the poles:

IF sin(lon2-lon1)<0
   tc1=acos((sin(lat2)-sin(lat1)*cos(d))/(sin(d)*cos(lat1)))
ELSE
   tc1=2*pi-acos((sin(lat2)-sin(lat1)*cos(d))/(sin(d)*cos(lat1)))
ENDIF

An alternative formula, not requiring the pre-computation of d, the distance between the points, is:

   tc1=mod(atan2(sin(lon1-lon2)*cos(lat2),
           cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon1-lon2)), 2*pi)
*/

/*
Latitude of point on GC

Intermediate points {lat,lon} lie on the great circle connecting points 1 and 2 when:

lat=atan((sin(lat1)*cos(lat2)*sin(lon-lon2)
     -sin(lat2)*cos(lat1)*sin(lon-lon1))/(cos(lat1)*cos(lat2)*sin(lon1-lon2)))

(not applicable for meridians. i.e if sin(lon1-lon2)=0)
*/

/*
Lat/lon given radial and distance

A point {lat,lon} is a distance d out on the tc radial from point 1 if:

     lat=asin(sin(lat1)*cos(d)+cos(lat1)*sin(d)*cos(tc))
     IF (cos(lat)=0)
        lon=lon1      // endpoint a pole
     ELSE
        lon=mod(lon1-asin(sin(tc)*sin(d)/cos(lat))+pi,2*pi)-pi
     ENDIF

This algorithm is limited to distances such that dlon <pi/2, i.e those that extend around less
than one quarter of the circumference of the earth in longitude. A completely general, but more
complicated algorithm is necessary if greater distances are allowed:

     lat =asin(sin(lat1)*cos(d)+cos(lat1)*sin(d)*cos(tc))
     dlon=atan2(sin(tc)*sin(d)*cos(lat1),cos(d)-sin(lat1)*sin(lat))
     lon=mod( lon1-dlon +pi,2*pi )-pi
*/

int Segment::DecompressSegmentToTemp()
{

    int flags = ARCHIVE_EXTRACT_TIME;
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;

    QString intarfile = this->fileInfo.absoluteFilePath();

    qDebug() << "Start UntarSegmentToTemp 1 for absolutefilepath " + intarfile;
    qDebug() << "fileInfo.completeBaseName() = " << fileInfo.completeBaseName();

    if(this->fileInfo.isDir())
        return 0;

    QString basename = fileInfo.completeBaseName();
    if(!basename.endsWith(".SEN3"))
        basename += ".SEN3";

    QDir curdir(basename);


    qDebug() << "curdir().dirName = " << curdir.dirName();

    if (curdir.exists())
    {
        qDebug() << "Directory " << basename << " exist !";
        return 0;
    }
    else
        qDebug() << "Directory " << basename << " does not exist !";

    QByteArray array = intarfile.toUtf8();
    const char* p = array.constData();

    a = archive_read_new();
    ext = archive_write_disk_new();
    //archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    archive_write_disk_set_options(ext, flags);

    r = archive_read_open_filename(a, p, 20480);
    if (r != ARCHIVE_OK)
    {
        qDebug() << "Tar file " << intarfile << " not found ....";
        return(1);
    }

//    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
//    {
//      qDebug() << QString("%1").arg(archive_entry_pathname(entry));
//      archive_read_data_skip(a);  // Note 2
//    }

    int nbrblocks = 1;

    for (;;)
    {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r != ARCHIVE_OK)
            qDebug() << "archive_read_next_header() " << QString(archive_error_string(a));
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK)
            qDebug() << "archive_write_header() " << QString(archive_error_string(ext));
        else
        {
            qDebug() << QString("Start copy_data ....%1").arg(nbrblocks);

            copy_data(a, ext);
            r = archive_write_finish_entry(ext);
            if (r != ARCHIVE_OK)
                qDebug() << "archive_write_finish_entry() " << QString(archive_error_string(ext));
            nbrblocks++;
        }
    }

    archive_read_close(a);
    archive_read_free(a);

    archive_write_close(ext);
    archive_write_free(ext);

    return(0);
}

int Segment::copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif


    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r != ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK) {
            qDebug() << "archive_write_data_block() " << QString(archive_error_string(aw));
            return (r);
        }
    }
}
