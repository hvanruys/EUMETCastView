#include "qsun.h"

QSun::QSun()
{

}

/* Calculates solar position vector */
void QSun::Calculate_Solar_Position(double time, Vector3 *solar_vector)
{
  double mjd,year,T,M,L,e,C,O,Lsa,nu,R,eps;

  mjd = time - 2415020.0;
  year = 1900 + mjd/365.25;
  T = (mjd + Delta_ET(year)/SEC_PER_DAY)/36525.0;
  M = Radians(Modulus(358.47583 + Modulus(35999.04975*T,360.0)
              - (0.000150 + 0.0000033*T)*Sqr(T),360.0));
  L = Radians(Modulus(279.69668 + Modulus(36000.76892*T,360.0)
              + 0.0003025*Sqr(T),360.0));
  e = 0.01675104 - (0.0000418 + 0.000000126*T)*T;
  C = Radians((1.919460 - (0.004789 + 0.000014*T)*T)*sin(M)
          + (0.020094 - 0.000100*T)*sin(2*M) + 0.000293*sin(3*M));
  O = Radians(Modulus(259.18 - 1934.142*T,360.0));
  Lsa = Modulus(L + C - Radians(0.00569 - 0.00479*sin(O)),TWOPI);
  nu = Modulus(M + C,TWOPI);
  R = 1.0000002*(1 - Sqr(e))/(1 + e*cos(nu));
  eps = Radians(23.452294 - (0.0130125 + (0.00000164 -
        0.000000503*T)*T)*T + 0.00256*cos(O));
  R = AU*R;
  solar_vector->set(R*cos(Lsa), R*sin(Lsa)*cos(eps), R*sin(Lsa)*sin(eps));

} /*Procedure Calculate_Solar_Position*/

/*------------------------------------------------------------------*/

/* Calculates stellite's eclipse status and depth */
//int QSun::Sat_Eclipsed(QVector4D pos, QVector4D sol, double *depth)
//{
//  double sd_sun, sd_earth, delta;
//  vector_t Rho, earth;

//  /* Determine partial eclipse */
//  sd_earth = ArcSin(XKMPER_WGS72/pos->w);
//  Vec_Sub(sol,pos,&Rho);
//  sd_sun = ArcSin(SR/Rho.w);
//  Scalar_Multiply(-1,pos,&earth);
//  delta = Angle(sol,&earth);
//  *depth = sd_earth - sd_sun - delta;
//  if( sd_earth < sd_sun )
//    return( 0 );
//  else
//    if( *depth >= 0 )
//      return( 1 );
//    else
//      return( 0 );

//} /*Function Sat_Eclipsed*/
