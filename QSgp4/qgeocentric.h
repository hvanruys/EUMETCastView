#ifndef QGEOCENTRIC_H
#define QGEOCENTRIC_H

#include <QString>

class QGeocentric
{
public:
    QGeocentric() : latitude(0.0), longitude(0.0), altitude(0.0)
    {
    }

    QGeocentric( double lat, double lon, double alt)
    {
        latitude = lat;
        longitude = lon;
        altitude = alt;
    }

    QGeocentric(const QGeocentric& geo)
    {
        latitude = geo.latitude;
        longitude = geo.longitude;
        altitude = geo.altitude;
    }


    virtual ~QGeocentric()
    {
    }

    QGeocentric& operator=(const QGeocentric& geo)
    {
        if (this != &geo)
        {
            latitude = geo.latitude;
            longitude = geo.longitude;
            altitude = geo.altitude;
        }
        return *this;
    }

    bool operator==(const QGeocentric& geo) const
    {
        return IsEqual(geo);
    }

    bool operator!=(const QGeocentric& geo) const
    {
        return !IsEqual(geo);
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
    bool IsEqual(const QGeocentric& geo) const
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
};

#endif // QGEOCENTRIC_H
