/*
 * Unit SGP_Obs
 *           Author:  Dr TS Kelso 
 * Original Version:  1992 Jun 02 
 * Current Revision:  1992 Sep 28 
 *          Version:  1.40 
 *        Copyright:  1992, All Rights Reserved 
 *
 *   Ported to C by:  Neoklis Kyriazis  April 9 2001
 */

#include "sgp4sdp4.h"
#include "globals.h"
#include "qsgp4globals.h"

/* Procedure Calculate_User_PosVel passes the user's geodetic position */
/* and the time of interest and returns the ECI position and velocity  */
/* of the observer. The velocity calculation assumes the geodetic      */
/* position is stationary relative to the earth's surface.             */
void
Calculate_User_PosVel(double time,
                      geodetic_t *geodetic,
                      vector_t *obs_pos,
                      vector_t *obs_vel)
{
/* Reference:  The 1992 Astronomical Almanac, page K11. */

  double c,sq,achcp;

  geodetic->theta = FMod2p(ThetaG_JD(time) + geodetic->lon);/*LMST*/
  c = 1/sqrt(1 + F*(F - 2)*Sqr(sin(geodetic->lat)));
  sq = Sqr(1 - F)*c;
  achcp = (XKMPER_WGS72*c + geodetic->alt)*cos(geodetic->lat);
  obs_pos->x = achcp*cos(geodetic->theta);/*kilometers*/
  obs_pos->y = achcp*sin(geodetic->theta);
  obs_pos->z = (XKMPER_WGS72*sq + geodetic->alt)*sin(geodetic->lat);
  obs_vel->x = -mfactor*obs_pos->y;/*kilometers/second*/
  obs_vel->y =  mfactor*obs_pos->x;
  obs_vel->z =  0;
  Magnitude(obs_pos);
  Magnitude(obs_vel);
} /*Procedure Calculate_User_PosVel*/

/*------------------------------------------------------------------*/

/* Procedure Calculate_LatLonAlt will calculate the geodetic  */
/* position of an object given its ECI position pos and time. */
/* It is intended to be used to determine the ground track of */
/* a satellite.  The calculations  assume the earth to be an  */
/* oblate spheroid as defined in WGS '72.                     */
void
Calculate_LatLonAlt(double time, vector_t *pos,  geodetic_t *geodetic)
{
  /* Reference:  The 1992 Astronomical Almanac, page K12. */

  double r,e2,phi,c;

  geodetic->theta = AcTan(pos->y,pos->x);/*radians*/
  geodetic->lon = FMod2p(geodetic->theta - ThetaG_JD(time));/*radians*/
  r = sqrt(Sqr(pos->x) + Sqr(pos->y));
  e2 = F*(2 - F);
  geodetic->lat = AcTan(pos->z,r);/*radians*/

  do
    {
      phi = geodetic->lat;
      c = 1/sqrt(1 - e2*Sqr(sin(phi)));
      geodetic->lat = AcTan(pos->z + XKMPER_WGS72*c*e2*sin(phi),r);
    }
  while(fabs(geodetic->lat - phi) >= 1E-10);

  geodetic->alt = r/cos(geodetic->lat) - XKMPER_WGS72*c;/*kilometers*/

  if( geodetic->lat > PIE/2 ) geodetic->lat -= TWOPI;
  
} /*Procedure Calculate_LatLonAlt*/

/*------------------------------------------------------------------*/

/* The procedures Calculate_Obs and Calculate_RADec calculate         */
/* the *topocentric* coordinates of the object with ECI position,     */
/* {pos}, and velocity, {vel}, from location {geodetic} at {time}.    */
/* The {obs_set} returned for Calculate_Obs consists of azimuth,      */
/* elevation, range, and range rate (in that order) with units of     */
/* radians, radians, kilometers, and kilometers/second, respectively. */
/* The WGS '72 geoid is used and the effect of atmospheric refraction */
/* (under standard temperature and pressure) is incorporated into the */
/* elevation calculation; the effect of atmospheric refraction on     */
/* range and range rate has not yet been quantified.                  */

/* The {obs_set} for Calculate_RADec consists of right ascension and  */
/* declination (in that order) in radians.  Again, calculations are   */
/* based on *topocentric* position using the WGS '72 geoid and        */
/* incorporating atmospheric refraction.                              */

void
Calculate_Obs(double time,
                vector_t *pos,
                vector_t *vel,
                geodetic_t *geodetic,
                vector_t *obs_set)
  {
   double
     sin_lat,cos_lat,
     sin_theta,cos_theta,
     el,azim,
     top_s,top_e,top_z;

   vector_t
     obs_pos,obs_vel,range,rgvel;

  Calculate_User_PosVel(time, geodetic, &obs_pos, &obs_vel);

    range.x = pos->x - obs_pos.x;
    range.y = pos->y - obs_pos.y;
    range.z = pos->z - obs_pos.z;

    rgvel.x = vel->x - obs_vel.x;
    rgvel.y = vel->y - obs_vel.y;
    rgvel.z = vel->z - obs_vel.z;

  Magnitude(&range);

  sin_lat = sin(geodetic->lat);
  cos_lat = cos(geodetic->lat);
  sin_theta = sin(geodetic->theta);
  cos_theta = cos(geodetic->theta);
  top_s = sin_lat*cos_theta*range.x
         + sin_lat*sin_theta*range.y
         - cos_lat*range.z;
  top_e = -sin_theta*range.x
         + cos_theta*range.y;
  top_z = cos_lat*cos_theta*range.x
         + cos_lat*sin_theta*range.y
         + sin_lat*range.z;
  azim = atan(-top_e/top_s); /*Azimuth*/
  if( top_s > 0 ) 
    azim = azim + PIE;
  if( azim < 0 )
    azim = azim + TWOPI;
  el = ArcSin(top_z/range.w);
  obs_set->x = azim;      /* Azimuth (radians)  */
  obs_set->y = el;        /* Elevation (radians)*/
  obs_set->z = range.w; /* Range (kilometers) */

 /*Range Rate (kilometers/second)*/
  obs_set->w = Dot(&range, &rgvel)/range.w;

/* Corrections for atmospheric refraction */
/* Reference:  Astronomical Algorithms by Jean Meeus, pp. 101-104    */
/* Correction is meaningless when apparent elevation is below horizon */

//  obs_set->y = obs_set->y + Radians((1.02/tan(Radians(Degrees(el)+
//               10.3/(Degrees(el)+5.11))))/60);

//  if( obs_set->y >= 0 )
//    SetFlag(VISIBLE_FLAG);
//  else
//    {
//    obs_set->y = el;  /*Reset to true elevation*/
//    ClearFlag(VISIBLE_FLAG);
//    } /*else*/
  } /*Procedure Calculate_Obs*/

/*------------------------------------------------------------------*/

void
Calculate_RADec( double time,
                 vector_t *pos,
                 vector_t *vel,
                 geodetic_t *geodetic,
                 vector_t *obs_set)
{
/* Reference:  Methods of Orbit Determination by  */
/*                Pedro Ramon Escobal, pp. 401-402 */

double
    phi,theta,sin_theta,cos_theta,sin_phi,cos_phi,
    az,el,Lxh,Lyh,Lzh,Sx,Ex,Zx,Sy,Ey,Zy,Sz,Ez,Zz,
    Lx,Ly,Lz,cos_delta,sin_alpha,cos_alpha;

  Calculate_Obs(time,pos,vel,geodetic,obs_set);

//  if( isFlagSet(VISIBLE_FLAG) )
//    {
//    az = obs_set->x;
//    el = obs_set->y;
//    phi   = geodetic->lat;
//    theta = FMod2p(ThetaG_JD(time) + geodetic->lon);
//    sin_theta = sin(theta);
//    cos_theta = cos(theta);
//    sin_phi = sin(phi);
//    cos_phi = cos(phi);
//    Lxh = -cos(az)*cos(el);
//    Lyh =  sin(az)*cos(el);
//    Lzh =  sin(el);
//    Sx = sin_phi*cos_theta;
//    Ex = -sin_theta;
//    Zx = cos_theta*cos_phi;
//    Sy = sin_phi*sin_theta;
//    Ey = cos_theta;
//    Zy = sin_theta*cos_phi;
//    Sz = -cos_phi;
//    Ez = 0;
//    Zz = sin_phi;
//    Lx = Sx*Lxh + Ex*Lyh + Zx*Lzh;
//    Ly = Sy*Lxh + Ey*Lyh + Zy*Lzh;
//    Lz = Sz*Lxh + Ez*Lyh + Zz*Lzh;
//    obs_set->y = ArcSin(Lz);  /*Declination (radians)*/
//    cos_delta = sqrt(1 - Sqr(Lz));
//    sin_alpha = Ly/cos_delta;
//    cos_alpha = Lx/cos_delta;
//    obs_set->x = AcTan(sin_alpha,cos_alpha); /*Right Ascension (radians)*/
//    obs_set->x = FMod2p(obs_set->x);
//    }  /*if*/
  } /* Procedure Calculate_RADec */


/// @brief The usual PI/180 constant
//static const double DEG_TO_RAD = 0.017453292519943295769236907684886;
/// @brief Earth's quatratic mean radius for WGS-84
//static const double EARTH_RADIUS_IN_METERS = 6372797.560856;

/** @brief Computes the arc, in radian, between two WGS-84 positions.
  *
  * The result is equal to <code>Distance(from,to)/EARTH_RADIUS_IN_METERS</code>
  *    <code>= 2*asin(sqrt(h(d/EARTH_RADIUS_IN_METERS )))</code>
  *
  * where:<ul>
  *    <li>d is the distance in meters between 'from' and 'to' positions.</li>
  *    <li>h is the haversine function: <code>h(x)=sin²(x/2)</code></li>
  * </ul>
  *
  * The haversine formula gives:
  *    <code>h(d/R) = h(from.lat-to.lat)+h(from.lon-to.lon)+cos(from.lat)*cos(to.lat)</code>
  *
  * @sa http://en.wikipedia.org/wiki/Law_of_haversines
  */
/*
double ArcInRadians(const Position& from, const Position& to) {
    double latitudeArc  = (from.lat - to.lat) * DEG_TO_RAD;
    double longitudeArc = (from.lon - to.lon) * DEG_TO_RAD;
    double latitudeH = sin(latitudeArc * 0.5);
    latitudeH *= latitudeH;
    double lontitudeH = sin(longitudeArc * 0.5);
    lontitudeH *= lontitudeH;
    double tmp = cos(from.lat*DEG_TO_RAD) * cos(to.lat*DEG_TO_RAD);
    return 2.0 * asin(sqrt(latitudeH + tmp*lontitudeH));
}
*/
/** @brief Computes the distance, in meters, between two WGS-84 positions.
  *
  * The result is equal to <code>EARTH_RADIUS_IN_METERS*ArcInRadians(from,to)</code>
  *
  * @sa ArcInRadians
  */
/*
double DistanceInMeters(const Position& from, const Position& to) {
    return EARTH_RADIUS_IN_METERS*ArcInRadians(from, to);
}
*/
/*------------------------------------------------------------------*/
