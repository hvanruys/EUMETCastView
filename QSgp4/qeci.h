#pragma once
#ifndef QECI_H
#define QECI_H
#include <QVector3D>
#include "qsgp4date.h"
#include "qgeodetic.h"
#include "qgeocentric.h"
#include "qtle.h"
#include "Vectors.h"

//////////////////////////////////////////////////////////////////////
// class QEci
// Encapsulates an Earth-Centered Inertial position, velocity, and time.
class QEci
{
public:
   QEci() {}
   QEci(const Vector3 &pos, const Vector3 &vel, const QSgp4Date &date, QTle::eGravconsttype gravcnst = QTle::wgs72);
   QEci(const QVector3D &pos, const QVector3D &vel, const QSgp4Date &date, QTle::eGravconsttype gravcnst = QTle::wgs72);
   QEci(const QGeodetic &geo, const QSgp4Date &date, QTle::eGravconsttype gravcnst = QTle::wgs72);
   QGeodetic ToGeo();
   QGeocentric ToGeocentric();
   void Update(const QGeodetic& geo, const QSgp4Date& dt, QTle::eGravconsttype gravcnst = QTle::wgs72);

   virtual ~QEci() {}


   Vector3& GetPos() { return m_pos;  }
   Vector3& GetVel() { return m_vel;  }
   QVector3D GetPos_f();
   QVector3D GetVel_f();
   QSgp4Date GetDate() const { return m_date; }

   QEci CircularApprox(double deltat, double angular_velocity);


   bool operator==(const QSgp4Date& dt) const
   {
       return m_date == dt;
   }

   bool operator!=(const QSgp4Date& dt) const
   {
       return m_date != dt;
   }


   double tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2, radiusearthkmminor, flattening;

protected:
   Vector3  m_pos;
   Vector3  m_vel;

   QSgp4Date  m_date;
   Vector3 m_pos_normalized;
   QTle::eGravconsttype whichconst;

private:
   void ToEci(const QGeodetic& geo, const QSgp4Date& dt, QTle::eGravconsttype gravcnst );


};

#endif
