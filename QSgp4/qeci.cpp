#include "qeci.h"
#include "qsgp4utilities.h"

//////////////////////////////////////////////////////////////////////
// QEci Class
//////////////////////////////////////////////////////////////////////
QEci::QEci(const Vector3 &pos,
           const Vector3 &vel,
           const QSgp4Date &date,
           QTle::eGravconsttype gravcnst)
   : m_pos (pos),
     m_vel (vel),
     m_date(date),
     whichconst(gravcnst)
{
    m_pos_normalized = m_pos;
    m_pos_normalized = m_pos_normalized.normalize();

    QTle::getgravconst( whichconst, tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2, radiusearthkmminor, flattening );
}

QEci::QEci(const QVector3D &pos,
           const QVector3D &vel,
           const QSgp4Date &date,
           QTle::eGravconsttype gravcnst)
{
    m_pos.set(pos.x(), pos.y(), pos.z());
    m_vel.set(vel.x(), vel.y(), vel.z());
    m_date = date;
    whichconst = gravcnst;

    m_pos_normalized = m_pos;
    m_pos_normalized = m_pos_normalized.normalize();

    QTle::getgravconst( whichconst, tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2, radiusearthkmminor, flattening );
}

QEci::QEci(const QGeodetic &geo, const QSgp4Date &date, QTle::eGravconsttype gravcnst)
{
    ToEci(geo, date, gravcnst);
}

void QEci::ToEci(const QGeodetic &geo, const QSgp4Date &date, QTle::eGravconsttype gravcnst)
{

    double mfactor = TWOPI * (OMEGA_E / SECONDS_PER_DAY);
    double lat = geo.latitude;
    double lon = geo.longitude;
    double alt = geo.altitude;
    whichconst = gravcnst;

    QTle::getgravconst( whichconst, tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2, radiusearthkmminor, flattening );

    ////////////////////////
    const double theta = date.ToLocalMeanSiderealTime(geo.longitude);

    /*
     * take into account earth flattening
     */
    const double c = 1.0 / sqrt(1.0 + flattening * (flattening - 2.0) * pow(sin(geo.latitude), 2.0));
    const double s = pow(1.0 - flattening, 2.0) * c;
    const double achcp = (radiusearthkm * c + geo.altitude) * cos(geo.latitude);

    m_date = date;

    m_pos.x = achcp * cos(theta);                     // km
    m_pos.y = achcp * sin(theta);                     // km
    m_pos.z = (radiusearthkm * s + alt) * sin(lat);   // km

    m_pos_normalized = m_pos;
    m_pos_normalized.normalize();


    m_vel.x = -mfactor * m_pos.y;                     // km / sec
    m_vel.y =  mfactor * m_pos.x;
    m_vel.z = 0.0;

 }

QGeodetic QEci::ToGeo()
{

   const double theta = Util::AcTan(m_pos.y, m_pos.x);
   double lon   = fmod(theta - m_date.ToGreenwichSiderealTime(), TWOPI);
   
   if (lon < 0.0) 
      lon += TWOPI;  // "wrap" negative modulo

   double r   = sqrt(m_pos.x * m_pos.x + m_pos.y * m_pos.y);
   double e2  = F * (2.0 - flattening);
   double lat = Util::AcTan(m_pos.z, r);

   const double delta = 1.0e-10;
   double phi = 0.0;
   double c = 0.0;

   do   
   {
      phi = lat;
      const double sinphi = sin(phi);
      c   = 1.0 / sqrt(1.0 - e2 * sinphi * sinphi);
      lat = Util::AcTan(m_pos.z + radiusearthkm * c * e2 * sin(phi), r);
   }
   while (fabs(lat - phi) > delta);
   
   double alt = r / cos(lat) - radiusearthkm * c;

   return QGeodetic(lat, lon, alt); // radians, radians, kilometers
}

QGeocentric QEci::ToGeocentric()
{

   const double theta = Util::AcTan(m_pos.y, m_pos.x);
   double lon   = fmod(theta - m_date.ToGreenwichSiderealTime(), TWOPI);

   if (lon < 0.0)
      lon += TWOPI;  // "wrap" negative modulo

   double r   = sqrt(m_pos.x * m_pos.x + m_pos.y * m_pos.y);
   double lat = Util::AcTan(m_pos.z, r);


   double alt = r / cos(lat) - radiusearthkm;

   return QGeocentric(lat, lon, alt); // radians, radians, kilometers
}

void QEci::Update(const QGeodetic& geo, const QSgp4Date& dt, QTle::eGravconsttype gravcnst )
{
    ToEci(geo, dt, gravcnst);
}


QVector3D QEci::GetPos_f()
{
    QVector3D pos(m_pos.x, m_pos.y, m_pos.z);
    return pos;
}

QVector3D QEci::GetVel_f()
{
    QVector3D vel(m_vel.x, m_vel.y, m_vel.z);
    return vel;
}


QEci QEci::CircularApprox(double deltat, double angular_velocity)
{

    double angdeltat = angular_velocity * deltat;
    Vector3 pos = m_pos * cos(angdeltat) + m_vel * (sin(angdeltat) / angular_velocity);

    Vector3 vel = - m_pos_normalized * (m_vel.length() / m_pos.length()) * sin(angdeltat) + m_vel * cos(angdeltat);
    double jd = m_date.Julian() + deltat / (24 * 60 * 60);

    QSgp4Date epo;
    epo.Set(jd, true);
    QEci eci1(pos, vel, epo);
    return eci1;

}
