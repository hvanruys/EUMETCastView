//
// globals.cpp
//
#include "stdafx.h"
#include "globals.h"
#include "qsgp4globals.h"

//////////////////////////////////////////////////////////////////////////////
double sqr(const double x) 
{
    return (x * x);
}

//////////////////////////////////////////////////////////////////////////////
double Fmod2p(const double arg)
{
    double modu = fmod(arg, TWOPI);

    if (modu < 0.0)
        modu += TWOPI;

    return modu;
}

//////////////////////////////////////////////////////////////////////////////
// AcTan()
// ArcTangent of sin(x) / cos(x). The advantage of this function over arctan()
// is that it returns the correct quadrant of the angle.
double AcTan(const double sinx, const double cosx)
{
    double ret;

    if (cosx == 0.0)
    {
        if (sinx > 0.0)
            ret = PI / 2.0;
        else
            ret = 3.0 * PI / 2.0;
    }
    else
    {
        if (cosx > 0.0)
            ret = atan(sinx / cosx);
        else
            ret = PI + atan(sinx / cosx);
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////////
double rad2deg(const double r)
{
    const double DEG_PER_RAD = 180.0 / PI;
    return r * DEG_PER_RAD;
}

//////////////////////////////////////////////////////////////////////////////
double deg2rad(const double d)
{
    const double RAD_PER_DEG = PI / 180.0;
    return d * RAD_PER_DEG;
}

/* Returns orccosine of rgument */
double
ArcCos(double arg)
{
    return( (PI/2) - ArcSin(arg) );
} /*Function ArcCos*/

/* Returns sign of a double */
int
Sign(double arg)
{
    if( arg > 0 )
        return( 1 );
    else if( arg < 0 )
        return( -1 );
    else
        return( 0 );
} /* Function Sign*/

/* Returns the arcsine of the argument */
double
ArcSin(double arg)
{
    if( fabs(arg) >= 1 )
        return( Sign(arg)*(PI/2) );
    else
        return( atan(arg/sqrt(1-arg*arg)) );
} /*Function ArcSin*/

float
ArcSinf(float arg)
{
    if( fabs(arg) >= 1 )
        return( Sign(arg)*(PI/2) );
    else
        return( atan(arg/sqrt(1-arg*arg)) );
} /*Function ArcSinf*/

/*------------------------------------------------------------------*/

/* Functions for testing and setting/clearing flags */

/* An int variable holding the single-bit flags */
static int Flags = 0;

int
isFlagSet(int flag)
{
    return (Flags & flag);
}

int
isFlagClear(int flag)
{
    return (~Flags & flag);
}

void
SetFlag(int flag)
{
    Flags |= flag;
}

void
ClearFlag(int flag)
{
    Flags &= ~flag;
}

void
LonLat2Point(float lat, float lon, QVector3D *pos, float radius)
{
    // lon -90..90
    // lat -180..180

    float	angX, angY;

    angX = lon * PI / 180.f;
    angY = lat * PI / 180.f;

    /*        pos->x = fabsf(cosf(angY)) * sinf(angX) * radius;
        pos->y = sinf(angY) * radius;
        pos->z = fabsf(cosf(angY)) * cosf(angX) * radius;
  */
    pos->setX(cosf(angY) * sinf(angX) * radius);
    pos->setY(sinf(angY) * radius);
    pos->setZ(cosf(angY) * cosf(angX) * radius);

}

void
LonLat2PointRad(float lat, float lon, QVector3D *pos, float radius)
{
    // lon -pi/2..pi/2
    // lat -pi..pi

    pos->setX(cosf(lat) * sinf(lon) * radius);
    pos->setY(sinf(lat) * radius);
    pos->setZ(cosf(lat) * cosf(lon) * radius);

}

void
Point2LonLat(double *lat_rad, double *lon_rad, double *radius, QVector3D pos )
{
    *radius = pos.length();

    *lat_rad = ArcSin(pos.y() / *radius);
    *lon_rad = atan2(pos.y(), pos.x());

}

void
Pos2LatLonAlt(double *lat_rad, double *lon_rad, double *radius, QVector3D pos)
{
    /* Reference:  The 1992 Astronomical Almanac, page K12. */

    double r,e2,phi,c;
    double sinphi;

    *lon_rad = AcTan(pos.y(),pos.x());/*radians*/
    r = sqrt(pos.x()*pos.x() + pos.y()*pos.y());
    e2 = F*(2 - F);
    *lat_rad = AcTan(pos.z(),r);/*radians*/

    do
    {
        phi = *lat_rad;
        sinphi = sin(phi);
        c = 1/sqrt(1 - e2*sinphi*sinphi);
        *lat_rad = AcTan(pos.z() + XKMPER_WGS72*c*e2*sin(phi),r);
    }
    while(fabs(*lat_rad - phi) >= 1E-10);

    *radius = XKMPER_WGS72 + r/cos(*lat_rad) - XKMPER_WGS72*c;/*kilometers*/

    if( *lat_rad > PI/2 ) *lat_rad -= TWOPI;

} /*Procedure Calculate_LatLonAlt*/

void
LonLat2Vector(float lat, float lon, QVector3D *pos, float radius)
{
    // lon -90..90
    // lat -180..180

    float	angX, angY;

    angX = lon * PI / 180.f;
    angY = lat * PI / 180.f;

    pos->setX(fabsf(cosf(angY)) * sinf(angX) * radius);
    pos->setY(sinf(angY) * radius);
    pos->setZ(fabsf(cosf(angY)) * cosf(angX) * radius);
}

void
sphericalToPixel(double lon, double lat, int &x, int &y, int devwidth, int devheight)
{

    if (lon > PI) lon -= TWOPI;
    else if (lon < -PI) lon += TWOPI;

    x = (int) ((lon - (-PI)) * devwidth/TWOPI + 0.5);

    if (x >= devwidth) x -= devwidth;
    else if (x < 0) x += devwidth;

    y = (int) ((PI/2 - lat) * devheight/PI + 0.5);
    if (y >= devheight) y = devheight - 1;

}

int
ccw(struct point p0, struct point p1, struct point p2 )
{
    int dx1, dx2, dy1, dy2;
    dx1 = p1.x - p0.x; dy1 = p1.y - p0.y;
    dx2 = p2.x - p0.x; dy2 = p2.y - p0.y;
    if (dx1*dy2 > dy1*dx2) return +1;
    if (dx1*dy2 < dy1*dx2) return -1;
    if ((dx1*dx2 < 0) || (dy1*dy2 < 0)) return -1;
    if ((dx1*dx1+dy1*dy1) < (dx2*dx2+dy2*dy2)) return +1;
    return 0;

}

int
intersect(struct line l1, struct line l2)
{
    return ((ccw(l1.p1, l1.p2, l2.p1)
             *ccw(l1.p1, l1.p2, l2.p2)) <= 0)
            && ((ccw(l2.p1, l2.p2, l1.p1)
                 *ccw(l2.p1, l2.p2, l1.p2)) <= 0);
}

int
pnpoly(int nvert, const QPoint *points, int testx, int testy)   //
{
    int i, j, c = 0;

    for (i = 0, j = nvert-1; i < nvert; j = i++) {
        if ( ((points[i].y() > testy) != (points[j].y() > testy)) &&
             (testx < (points[j].x() - points[i].x() ) * (testy-points[i].y()) / (points[j].y() - points[i].y()) + points[i].x()) )
            c = !c;
    }

    return c;
}

void sortSphericalVectorLon(SphericalVector arr[], int size)
{
    SphericalVector temp;

    for(int i = 0; i < size; i++)
    {
        for(int j = 0;j < size - 1; j++)
        {
            if(arr[j].lon > arr[j+1].lon)
            {
                //we need to swap
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

double adjust_lon_rad(double x)
{
    if ( fabs(x) < PI )
        return x;
    else
    {
        if (x < 0)
            return (x + TWOPI);
        else
            return (x - TWOPI);
    }
}

double adjust_lon_deg(double x)
{
    if ( fabs(x) < 180.0 )
        return x;
    else
    {
        if (x < 0)
            return (x + 360.0);
        else
            return (x - 360.0);
    }
}

double longitudediffdeg( double lon1, double lon2)
{
    double deltalon;
    if(lon1 == 0.0)
        return fabs(lon2);

    if(lon2 == 0.0)
        return fabs(lon1);

    if (lon1 > 0.0 && lon2 > 0.0)
    {
        deltalon = fabs(lon1 - lon2);
    }
    else if( lon1 < 0.0 && lon2 < 0.0)
    {
        deltalon = fabs(lon1 - lon2);
    }
    else
    {
        deltalon = lon1 + fabs(lon2);
        if(deltalon > 180.0)
            deltalon = 360 - deltalon;
    }
    return deltalon;

}

QDateTime julianDoubleToDateTime(double julian)
{
    // The day number is the integer part of the date
    int julianDays = qFloor(julian);
    QDate d = QDate::fromJulianDay(julianDays);

    // The fraction is the time of day
    double julianMSecs = (julian - static_cast<double>(julianDays)) * 86400.0 * 1000;

    // Julian days start at noon (12:00 UTC)
    QTime t = QTime(12, 0, 0, 0).addMSecs(qRound(julianMSecs));

    return QDateTime(d, t, Qt::UTC);
}

int Min(const int *Numbers, const int Count)
{
    int Minimum = Numbers[0];

    for(int i = 0; i < Count; i++)
        if( Minimum > Numbers[i] )
            Minimum = Numbers[i];
    return Minimum;
}

int Max(const int *Numbers, const int Count)
{
    int Maximum = Numbers[0];

    for(int i = 0; i < Count; i++)
        if( Maximum < Numbers[i] )
            Maximum = Numbers[i];
    return Maximum;
}

/* Function to eliminate roundoff errors in asin
----------------------------------------------*/
double asinz (double con)
{
    if (fabs(con) > 1.0)
    {
        if (con > 1.0)
            con = 1.0;
        else
            con = -1.0;
    }
    return(asin(con));
}

float Minf(const float v11, const float v12, const float v21, const float v22)
{
    float Minimum = v11;

    if( Minimum > v12 )
            Minimum = v12;
    if( Minimum > v21 )
            Minimum = v21;
    if( Minimum > v22 )
            Minimum = v22;

    return Minimum;
}

float Maxf(const float v11, const float v12, const float v21, const float v22)
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

double Mind(const double v11, const double v12, const double v21, const double v22)
{
    double Minimum = v11;

    if( Minimum > v12 )
            Minimum = v12;
    if( Minimum > v21 )
            Minimum = v21;
    if( Minimum > v22 )
            Minimum = v22;

    return Minimum;
}

double Maxd(const double v11, const double v12, const double v21, const double v22)
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
/*------------------------------------------------------------------*/
