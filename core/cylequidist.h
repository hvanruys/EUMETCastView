#ifndef CYLEQUIDIST_H
#define CYLEQUIDIST_H

#include <qpixmap.h>
#include <qimage.h>

class CylEquiDist
{

public:
  CylEquiDist( const QString filemapname , double startlon=0, double startlat=0 );
  CylEquiDist( const int width, const int height, double startlon=0, double startlat=0 );
  ~CylEquiDist();
  bool sphericalToPixel(double lon, double lat, int &x, int &y);
  bool pixelToSpherical(const int x, const int y, double &lon, double &lat);
  // QRgb pixel( int x, int y ) { return (im.pixel(x, y)); };
  void DrawGridsRes(QPixmap *pm);
  QPixmap * RetPixMap();

  int width();
  int height();

protected:


private:

  void SetupMap();

  double del_lon;
  double del_lat;

  QScopedArrayPointer<double> lon_array;
  QScopedArrayPointer<double> lat_array;

  double start_lon;
  double start_lat;
  double map_width;
  double map_height;
  int pm_width, pm_height;
  QPixmap pm;  // the original pixmap

};

#endif
