#ifndef QGEODETIC_H
#define QGEODETIC_H

#include <QString>
#include "qsgp4utilities.h"
#include "qgeocentric.h"
#include "qtle.h"

class QGeodetic
{
public:
    QGeodetic() : latitude(0.0), longitude(0.0), altitude(0.0)
    {
    }

    QGeodetic( double lat, double lon, double alt)
    {
        latitude = lat;
        longitude = lon;
        altitude = alt;
    }

    QGeodetic(const QGeodetic& geo)
    {
        latitude = geo.latitude;
        longitude = geo.longitude;
        altitude = geo.altitude;
    }

    virtual ~QGeodetic()
    {
    }

    QGeodetic& operator=(const QGeodetic& geo)
    {
        if (this != &geo)
        {
            latitude = geo.latitude;
            longitude = geo.longitude;
            altitude = geo.altitude;
        }
        return *this;
    }

    bool operator==(const QGeodetic& geo) const
    {
        return IsEqual(geo);
    }

    bool operator!=(const QGeodetic& geo) const
    {
        return !IsEqual(geo);
    }

    QGeocentric toGeocentric(QTle::eGravconsttype which)
    {
        double lon = longitude;
        QTle::getgravconst( which, tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2, radiusearthkmminor, flattening );

        double lat = Util::AcTan( tan(latitude) * radiusearthkmminor * radiusearthkmminor, radiusearthkm * radiusearthkm );
        QGeocentric gc(lat, lon, altitude );
        return gc;
    }

    QString ToString() const
    {
        double lat = Util::RadiansToDegrees(latitude);
        double lon = Util::RadiansToDegrees(longitude);
        QString retstr = QString("Lat: %1°, Lon = %2°, Alt : %3 km").arg(lat, 9, 'f', 3 ).arg(lon, 9, 'f', 3).arg(altitude, 9, 'f', 2);
        return retstr;
    }

    /** latitude in radians (-PI >= latitude < PI) */
    double latitude;
    /** latitude in radians (-PI/2 >= latitude <= PI/2) */
    double longitude;
    /** altitude in kilometers */
    double altitude;

private:
    bool IsEqual(const QGeodetic& geo) const
    {
        bool equal = false;
        if (latitude == geo.latitude &&
                longitude == geo.longitude &&
                altitude == geo.altitude)
        {
            equal = false;
        }
        return equal;
    }

protected:
   double tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2, radiusearthkmminor, flattening;

};

#endif // QGEODODETIC_H
