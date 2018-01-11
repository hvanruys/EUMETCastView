#ifndef MAPCYL_H
#define MAPCYL_H

#include "cylequidist.h"

#include "satellite.h"
#include "avhrrsatellite.h"

class MapFieldCyl : public QWidget
{
  Q_OBJECT

public:
  MapFieldCyl( QWidget *parent=0, CylEquiDist *meqdis=0 , SatelliteList *satlist=0, AVHRRSatellite *seglist=0 );
  ~MapFieldCyl();
  void setGrid(bool grid);
  QRect tip( const QPoint & );


protected:
  void paintEvent( QPaintEvent * );
  void mousePressEvent( QMouseEvent *);
  void mouseReleaseEvent( QMouseEvent *);
  void mouseMoveEvent( QMouseEvent *);
  void resizeEvent( QResizeEvent * );
  void wheelEvent(QWheelEvent *event);
  void timerEvent(QTimerEvent *event);

  
private:
  void scale();
  void showTwilight(double sun_lon, double sun_lat, QPainter *paint);
  void showSunPosition(QPainter *paint);
  void showSunPosition(QPainter *paint, double , double );

  int conversion_flags;
  const char* filename;

  QPixmap pmScaled;      // the scaled pixmap
  //QPixmap pmScaled_n;      // the scaled pixmap at night
  QPixmap pmScaled_res;  //  the end result
  //QPixmap pmBackground;

  bool gridset;
  double geo_alt;
  bool down;
  double map_lon, map_lat;
  int map_x, map_y;
  SatelliteList *sats;
  CylEquiDist *med;
  AVHRRSatellite *segs;


public slots:


signals:
  void timeChanged( const QString &text );
  void mapClicked();
  void wheelChange(int numSteps);

};


#endif // MAP_H
