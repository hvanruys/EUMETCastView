#include "cylequidist.h"
#include "options.h"
#include "globals.h"
#include "qsgp4globals.h"

#include <qpixmap.h>
// #include <qimage.h>
#include <qpainter.h>
#include <QMessageBox>
#include <QtDebug>

extern Options opts;

CylEquiDist::CylEquiDist( const QString filemapname, double startlon, double startlat )
{
  bool ok = false;
  qDebug() << QString("filemapname = %1").arg(filemapname);
  ok = pm.load(filemapname, 0, Qt::AutoColor);
  if ( ok )
  {
     qDebug() << "map is loaded !";
  }
  else
  {
    qDebug() << " could not load map";
    pm = QPixmap(800, 600);
    pm.fill(Qt::red);
  }
  double obslon;
  obslon = opts.getObsLon(); // 0 --> 360

  SetupMap();


}

CylEquiDist::CylEquiDist(const int width , const int height, double startlon, double startlat )
{
    pm_width = width;
    pm_height = height;

    del_lon = map_width/pm_width;
    del_lat = map_height/pm_height;

    int i;

    lon_array.reset(new double[pm_width]);
    for (i = 0; i < pm_width; i++)
        lon_array[i] = (i + 0.5) * del_lon + start_lon;

    lat_array.reset(new double[pm_height]);
    for (i = 0; i < pm_height; i++)
        lat_array[i] = start_lat - (i + 0.5) * del_lat;

}

CylEquiDist::~CylEquiDist()
{
    qDebug() << "closing CylEquiDist";
}

void
CylEquiDist::SetupMap()
{
    if( pm.width()==0)
    {
        pm_width = 600;
        qDebug() << "pm.width = 0";
    }
    else
    {
        qDebug() << "pm.width = " << pm.width();
        pm_width = pm.width();
    }

    if(pm.height()==0)
    {
        pm_height = 400;
        qDebug() << "pm.height = 0";
    }
    else
    {
        qDebug() << "pm.height = " << pm.height();
        pm_height = pm.height();
    }

    del_lon = map_width/pm_width;
    del_lat = map_height/pm_height;

    int i;

    lon_array.reset(new double[pm_width]);
    for (i = 0; i < pm_width; i++)
        lon_array[i] = (i + 0.5) * del_lon + start_lon;

    lat_array.reset(new double[pm_height]);
    for (i = 0; i < pm_height; i++)
        lat_array[i] = start_lat - (i + 0.5) * del_lat;

}

int
CylEquiDist::width()
{
  return(pm_width);
}

int
CylEquiDist::height()
{
  return(pm_height);
}

bool CylEquiDist::sphericalToPixel(double lon, double lat, int &x, int &y)
{

  if (lon > PIE) lon -= TWOPI;
  else if (lon < -PIE) lon += TWOPI;

  x = (int) ((lon - start_lon)/del_lon + 0.5);

  if (x >= pm_width) x -= pm_width;
  else if (x < 0) x += pm_width;

  y = (int) ((start_lat - lat)/del_lat + 0.5);

  if (y >= pm_height) y = pm_height - 1;

  return(true);
}

bool CylEquiDist::pixelToSpherical(const int x, const int y, double &lon, double &lat)
{
    lon = lon_array[x];
    lat = lat_array[y];
    return(true);
}


QPixmap *
CylEquiDist::RetPixMap()
{
    return(&pm);
}



void
CylEquiDist::DrawGridsRes(QPixmap *pm)
{
  QPainter painter(pm);

  QPen pen( QColor(100,100,100), 1);
  QString line1;

  painter.setPen( pen );

  painter.setFont( QFont( "helvetica", 8) );

  int totx = pm->width();
  int toty = pm->height();

  painter.drawLine(20, toty/2, totx-20, toty/2  );
  painter.drawLine(30, toty/6, totx-30, toty/6  );
  painter.drawLine(30, toty/3, totx-30, toty/3  );
  painter.drawLine(30, 2*toty/3, totx-30, 2*toty/3  );
  painter.drawLine(30, 5*toty/6, totx-30, 5*toty/6  );

  QPen pen1(QColor(0,150,0), 1, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
  painter.setPen(pen1);
  painter.drawLine(0, toty*(0.5 - 23.439/180.0), totx, toty*(0.5 - 23.439/180.0));
  painter.drawLine(0, toty*(0.5 + 23.439/180.0), totx, toty*(0.5 + 23.439/180.0));

  painter.setPen( pen );

  painter.drawLine(totx/12, 15, totx/12, toty-15);
  painter.drawLine(totx/6, 15, totx/6, toty-15);
  painter.drawLine(totx/4, 15,  totx/4, toty-15);
  painter.drawLine(totx/3, 15, totx/3, toty-15);
  painter.drawLine(5*totx/12, 15, 5*totx/12, toty-15);
  painter.drawLine(totx/2, 15, totx/2, toty-15);
  painter.drawLine(7*totx/12, 15, 7*totx/12, toty-15);
  painter.drawLine(2*totx/3, 15, 2*totx/3, toty-15);
  painter.drawLine(9*totx/12, 15, 9*totx/12, toty-15);
  painter.drawLine(5*totx/6, 15, 5*totx/6, toty-15);
  painter.drawLine(11*totx/12, 15, 11*totx/12, toty-15);

  pen.setColor( QColor(255,255,255) );
  painter.setPen( pen );

  line1.asprintf( "0" );
  painter.drawText( 5, toty/2+3, line1 );
  painter.drawText( totx-15, toty/2+3,line1 );

  line1.asprintf( "+60" );
  painter.drawText( 5, toty/6+3, line1 );
  painter.drawText( totx-25, toty/6+3,line1 );

  line1.asprintf( "+30" );
  painter.drawText( 5, toty/3+3, line1 );
  painter.drawText( totx-25, toty/3+3,line1 );

  line1.asprintf( "-30" );
  painter.drawText( 5, 2*toty/3+3, line1 );
  painter.drawText( totx-25, 2*toty/3+3,line1 );

  line1.asprintf( "-60" );
  painter.drawText( 5, 5*toty/6+3, line1 );
  painter.drawText( totx-25, 5*toty/6+3,line1 );


  int intd, i, gridx, mx;


  for( i=1, gridx=210; i<12; i++, gridx += 30 )
  {
    intd = gridx;
    if (intd >= 360)
      intd = intd - 360;
    if (intd >= 360)
      intd = intd - 360;
    if (intd == 0)
      mx = 3;
    else if (intd < 100)
      mx = 6;
    else
      mx = 9;

    line1.asprintf( "%d", intd  );
    painter.drawText( i*totx/12-mx, 15,line1 );
    painter.drawText( i*totx/12-mx, toty-5,line1 );
  }


}

