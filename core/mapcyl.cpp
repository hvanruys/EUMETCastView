#include "mapcyl.h"
#include "cylequidist.h"
#include "options.h"
#include "globals.h"
#include "sgp4sdp4.h"
//#include "avhrrSegmentListMetop.h"
//#include "avhrrSegmentListNoaa.h"

#include <QApplication>
#include <QPainter>
#include <QLabel>
#include <QTimer>
#include <QMouseEvent>
#include <QDebug>

extern Options opts;

MapFieldCyl::MapFieldCyl( QWidget *parent, CylEquiDist *meqdis, SatelliteList *satlist, AVHRRSatellite *seglist )
        : QWidget( parent )
{

  //this->setBackgroundMode( NoBackground );
  setBackgroundRole(QPalette::Dark);

  med=meqdis;
  sats=satlist;
  segs=seglist;

  //qDebug() << QString("in creator MapFieldCyl nbr seglist %1 selected %2").arg(segs->segmentlistmetop->NbrOfSegments()).arg(segs->segmentlistmetop->NbrOfSegmentsSelected());

  this->setMouseTracking(true);

  pmScaled = QPixmap();
  pmScaled_res = QPixmap();
  //pmScaled_n = QPixmap();
  //pmBackground = QPixmap();
  map_lon = 0;
  map_lat = 0;
  down = false;

  gridset = true;

  this->startTimer(1000);

  scale();


}

MapFieldCyl::~MapFieldCyl()
{
    qDebug() << "closing MapFieldCyl";
}



void MapFieldCyl::timerEvent(QTimerEvent *event)
{
    //qDebug() << "Timer ID:" << event->timerId();
    update();
}

void MapFieldCyl::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    if (event->orientation() == Qt::Horizontal) {
        // qDebug() << QString("scrollHorizontally = %1").arg(numSteps);
    } else {
        // qDebug() << QString("scrollVertically = %1").arg(numSteps);
        emit wheelChange(numSteps);
    }
    event->accept();
}

void MapFieldCyl::paintEvent( QPaintEvent * )
{

    QPainter painter(&pmScaled_res);
    painter.drawPixmap(0, 0, pmScaled);
    sats->RenderAllSatellites( &painter );
    sats->showHorizon( deg2rad(opts.getObsLon()), deg2rad(opts.getObsLat()), sats->GetSelectedSatAlt(), &painter);

    //qDebug() << QString("in paintevent MapFieldCyl nbr seglist %1 selected %2").arg(segs->segmentlistmetop->NbrOfSegments()).arg(segs->segmentlistmetop->NbrOfSegmentsSelected());

    double first_julian, last_julian;


    if (opts.buttonMetop && segs->seglmetop->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglmetop->RenderSegments( &painter, QColor(Qt::magenta), true );
        }
        else
        {
            segs->seglmetop->GetFirstLastVisible(&first_julian, &last_julian);
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglmetop->RenderSegments( &painter, QColor(Qt::magenta), false );
        }
    }

    if (opts.buttonNoaa && segs->seglnoaa->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglnoaa->RenderSegments( &painter, QColor(Qt::green), true );
        }
        else
        {
            segs->seglnoaa->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglnoaa->RenderSegments( &painter, QColor(Qt::green), false );
        }
    }

    if (opts.buttonGAC && segs->seglgac->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglgac->RenderSegments( &painter, QColor(Qt::darkYellow), true );
        }
        else
        {
            segs->seglgac->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglgac->RenderSegments( &painter, QColor(Qt::darkYellow), false );
        }
    }

    if (opts.buttonHRP && segs->seglhrp->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglhrp->RenderSegments( &painter, QColor(Qt::darkRed), true );
        }
        else
        {
            segs->seglhrp->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglhrp->RenderSegments( &painter, QColor(Qt::darkRed), false );
        }
    }

    if (opts.buttonMetopAhrpt && segs->seglmetopAhrpt->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglmetopAhrpt->RenderSegments( &painter, QColor(Qt::darkRed), true );
        }
        else
        {
            segs->seglmetopAhrpt->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglmetopAhrpt->RenderSegments( &painter, QColor(Qt::darkRed), false );
        }
    }

    if (opts.buttonMetopBhrpt && segs->seglmetopBhrpt->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglmetopBhrpt->RenderSegments( &painter, QColor(Qt::darkRed), true );
        }
        else
        {
            segs->seglmetopBhrpt->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglmetopBhrpt->RenderSegments( &painter, QColor(Qt::darkRed), false );
        }
    }

    if (opts.buttonNoaa19hrpt && segs->seglnoaa19hrpt->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglnoaa19hrpt->RenderSegments( &painter, QColor(Qt::darkRed), true );
        }
        else
        {
            segs->seglnoaa19hrpt->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglnoaa19hrpt->RenderSegments( &painter, QColor(Qt::darkRed), false );
        }
    }

    if (opts.buttonM01hrpt && segs->seglM01hrpt->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglM01hrpt->RenderSegments( &painter, QColor(Qt::darkRed), true );
        }
        else
        {
            segs->seglM01hrpt->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglM01hrpt->RenderSegments( &painter, QColor(Qt::darkRed), false );
        }
    }

    if (opts.buttonM02hrpt && segs->seglM02hrpt->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglM02hrpt->RenderSegments( &painter, QColor(Qt::darkRed), true );
        }
        else
        {
            segs->seglM02hrpt->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglM02hrpt->RenderSegments( &painter, QColor(Qt::darkRed), false );
        }
    }

    if (opts.buttonVIIRSM && segs->seglviirsm->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglviirsm->RenderSegments( &painter, QColor(Qt::cyan), true );
        }
        else
        {
            segs->seglviirsm->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglviirsm->RenderSegments( &painter, QColor(Qt::cyan), false );
        }
    }

    if (opts.buttonVIIRSDNB && segs->seglviirsdnb->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglviirsdnb->RenderSegments( &painter, QColor(Qt::cyan), true );
        }
        else
        {
            segs->seglviirsdnb->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglviirsdnb->RenderSegments( &painter, QColor(Qt::cyan), false );
        }
    }

    if (opts.buttonOLCIefr && segs->seglolciefr->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglolciefr->RenderSegments( &painter, QColor(Qt::cyan), true );
        }
        else
        {
            segs->seglolciefr->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglolciefr->RenderSegments( &painter, QColor(Qt::cyan), false );
        }
    }

    if (opts.buttonOLCIerr && segs->seglolcierr->NbrOfSegments() > 0)
    {
        if(segs->getShowAllSegments())
        {
            segs->seglolcierr->RenderSegments( &painter, QColor(Qt::cyan), true );
        }
        else
        {
            segs->seglolcierr->GetFirstLastVisible( &first_julian, &last_julian );
            showSunPosition(&painter, first_julian, last_julian);
            segs->seglolcierr->RenderSegments( &painter, QColor(Qt::cyan), false );
        }
    }

    if (opts.buttonMetop == false && opts.buttonNoaa == false && opts.buttonGAC == false &&
            opts.buttonHRP == false && opts.buttonVIIRSM == false && opts.buttonVIIRSDNB == false && opts.buttonOLCIefr == false && opts.buttonOLCIerr == false &&
            opts.buttonMetopAhrpt == false && opts.buttonMetopBhrpt == false && opts.buttonNoaa19hrpt == false && opts.buttonM01hrpt == false &&
            opts.buttonM02hrpt == false)
        showSunPosition(&painter);

    if (down)
    {
        //showSunPosition(&painter);
        //qDebug() << QString("first_utc = %1, last_utc = %2").arg(first_utc).arg(last_utc);
    }

    QPainter paint(this);
    paint.drawPixmap( 0, 0, pmScaled_res );

}

void MapFieldCyl::resizeEvent( QResizeEvent * )
{

  if ( width() != pmScaled.width() || height() != pmScaled.height())
  {
    scale();
  }

}

void MapFieldCyl::mousePressEvent( QMouseEvent *e )
{
    if(e->button() == Qt::LeftButton)
        return;

    bool isselected = false;
    down = true;
    map_x = e->x();
    map_y = e->y();
    map_lon = (double)((TWOPI*e->x()-PI*pmScaled_res.width())/pmScaled_res.width());
    map_lat = (double)(((PI/2)*pmScaled_res.height()-PI*e->y())/pmScaled_res.height());

    double lon = rad2deg(map_lon);
    double lat = rad2deg(map_lat);

    if(opts.buttonMetop)
        isselected = segs->seglmetop->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonNoaa)
        isselected = segs->seglnoaa->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonHRP)
        isselected = segs->seglhrp->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonGAC)
        isselected = segs->seglgac->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonMetopAhrpt)
        isselected = segs->seglmetopAhrpt->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonMetopBhrpt)
        isselected = segs->seglmetopBhrpt->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonNoaa19hrpt)
        isselected = segs->seglnoaa19hrpt->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonM01hrpt)
        isselected = segs->seglM01hrpt->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonM02hrpt)
        isselected = segs->seglM02hrpt->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonVIIRSM)
        isselected = segs->seglviirsm->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonVIIRSDNB)
        isselected = segs->seglviirsdnb->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonOLCIefr)
        isselected = segs->seglolciefr->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );
    else if (opts.buttonOLCIerr)
        isselected = segs->seglolcierr->TestForSegment( &lon, &lat, true, segs->getShowAllSegments() );


    if(isselected)
        emit mapClicked();  // show selected segmentlist in FormEphem
     else
        sats->TestForSat(e->x(), e->y());

    update();

}

void MapFieldCyl::mouseReleaseEvent( QMouseEvent * )
{
  down = false;
  update();
}

void MapFieldCyl::mouseMoveEvent( QMouseEvent *e )
{

  map_lon = (double)((TWOPI*e->x()-PI*pmScaled_res.width())/pmScaled_res.width());
  map_lat = (double)(((PI/2)*pmScaled_res.height()-PI*e->y())/pmScaled_res.height());

}

void MapFieldCyl::scale()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );
    pmScaled = (*med->RetPixMap()).scaled(width(),height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    //pmScaled = (*med->RetPixMap()).scaled(width(),height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    //pmScaled = *med->RetPixMap();
    pmScaled_res = pmScaled;
    if (gridset) med->DrawGridsRes(&pmScaled);
    QApplication::restoreOverrideCursor();	// restore original cursor
}


void MapFieldCyl::showSunPosition(QPainter *painter)
{
  double jul_utc;

  /* Zero vector for initializations */
  vector_t zero_vector = {0,0,0,0};

  /* Solar ECI position vector  */
  vector_t solar_vector = zero_vector;
  geodetic_t sun_geodetic;

  struct tm utc;

  UTC_Calendar_Now(&utc);  //&opts.utc);
  jul_utc = Julian_Date(&utc);  //&opts.utc);

  //qDebug() << QString("%1, %2, %3, %4, %5, %6, %7, %8, %9").arg(utc.tm_hour).arg(utc.tm_isdst).arg(utc.tm_mday).arg(utc.tm_min).arg(utc.tm_mon);

  /* Calculate solar position and satellite eclipse depth */
  /* Also set or clear the satellite eclipsed flag accordingly */
  Calculate_Solar_Position(jul_utc, &solar_vector);

  /* Calculate Sun's Lat North, Lon East and Alt. */
  Calculate_LatLonAlt(jul_utc, &solar_vector, &sun_geodetic);

  int posx, posy;

  sphericalToPixel( sun_geodetic.lon, sun_geodetic.lat, posx, posy, pmScaled_res.width(), pmScaled_res.height() );

  painter->setBrush( Qt::yellow );
  painter->drawEllipse( posx -  4 , posy - 4, 8, 8 );
  painter->setPen( Qt::green );
  showTwilight(sun_geodetic.lon, sun_geodetic.lat, painter);

}

void MapFieldCyl::showSunPosition(QPainter *painter, double first_julian, double last_julian)
{

  /* Zero vector for initializations */
  vector_t zero_vector = {0,0,0,0};

  /* Solar ECI position vector  */
  vector_t solar_vector = zero_vector;
  geodetic_t sun_geodetic;

  Calculate_Solar_Position(first_julian, &solar_vector);

  /* Calculate Sun's Lat North, Lon East and Alt. */
  Calculate_LatLonAlt(first_julian, &solar_vector, &sun_geodetic);

  int posx, posy;

  sphericalToPixel( sun_geodetic.lon, sun_geodetic.lat, posx, posy, pmScaled_res.width(), pmScaled_res.height() );

  painter->setBrush( Qt::yellow );
  painter->setPen(Qt::yellow);
  painter->drawEllipse( posx -  4 , posy - 4, 8, 8 );

  painter->setPen( Qt::green );
  showTwilight(sun_geodetic.lon, sun_geodetic.lat, painter);

  //jul_utc = Julian_Date_of_Year(last_year) + last_utc;
  Calculate_Solar_Position(last_julian, &solar_vector);

  /* Calculate Sun's Lat North, Lon East and Alt. */
  Calculate_LatLonAlt(last_julian, &solar_vector, &sun_geodetic);

  sphericalToPixel( sun_geodetic.lon, sun_geodetic.lat, posx, posy,pmScaled_res.width(),pmScaled_res.height() );

  painter->setBrush( Qt::yellow );
  painter->setPen(Qt::yellow);
  painter->drawEllipse( posx -  4 , posy - 4, 8, 8 );

  painter->setPen( Qt::darkGreen );
  showTwilight(sun_geodetic.lon, sun_geodetic.lat, painter);

}

void MapFieldCyl::showTwilight(double sun_lon, double sun_lat, QPainter *painter)
{
  double sunhor;
  double Xlon;
  int posx, posy;
  int posx1,posy1;

  Xlon=PI;

  if (sun_lat<=0)
    sunhor = AcTan( sin(sun_lon-(PI/2)-Xlon),1/tan((PI/2)-sun_lat) );
  else
    sunhor = AcTan( sin(sun_lon-(PI/2)-Xlon),1/tan(sun_lat-(PI/2)) );

  if (Xlon < PI)
    posx = (int)(pmScaled_res.width() * ( Xlon + PI) / TWOPI);
  else
    posx = (int)(pmScaled_res.width() * ( Xlon - PI) / TWOPI);
  if (sun_lat<=0)
    posy = (int)( pmScaled_res.height() * ( 3*(PI/2) - sunhor) / PI);
  else
    posy = (int)( pmScaled_res.height() * ( sunhor - (PI/2) ) / PI);

  posx1 = posx;
  posy1 = posy;

  for (Xlon=PI; Xlon < TWOPI; Xlon += (PI/2)/20)
  {
    if (sun_lat<=0)
      sunhor = AcTan( sin(sun_lon-(PI/2)-Xlon),1/tan((PI/2)-sun_lat) );
    else
      sunhor = AcTan( sin(sun_lon-(PI/2)-Xlon),1/tan(sun_lat-(PI/2)) );

    if (Xlon < PI)
      posx = (int)(pmScaled_res.width() * ( Xlon + PI) / TWOPI);
    else
      posx = (int)(pmScaled_res.width() * ( Xlon - PI) / TWOPI);
    if (sun_lat<=0)
      posy = (int)( pmScaled_res.height() * ( 3*(PI/2) - sunhor) / PI);
    else
      posy = (int)( pmScaled_res.height() * ( sunhor -(PI/2)) / PI);

    painter->drawLine( posx1, posy1, posx, posy );
    posx1 = posx;
    posy1 = posy;

  }

  for (Xlon=0; Xlon < PI; Xlon += (PI/2)/20)
  {
    if (sun_lat<=0)
      sunhor = AcTan( sin(sun_lon-(PI/2)-Xlon),1/tan((PI/2)-sun_lat) );
    else
      sunhor = AcTan( sin(sun_lon-(PI/2)-Xlon),1/tan(sun_lat-(PI/2)) );

    if (Xlon < PI)
      posx = (int)(pmScaled_res.width() * ( Xlon + PI) / TWOPI);
    else
      posx = (int)(pmScaled_res.width() * ( Xlon - PI) / TWOPI);
    if (sun_lat<=0)
      posy = (int)( pmScaled_res.height() * ( 3*(PI/2) - sunhor) / PI);
    else
      posy = (int)( pmScaled_res.height() * ( sunhor-(PI/2)) / PI);

    painter->drawLine( posx1, posy1, posx, posy );
    posx1 = posx;
    posy1 = posy;
  }

}

void MapFieldCyl::setGrid(bool grid)
{
   gridset = grid;
}

