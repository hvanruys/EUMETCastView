#ifndef QTOPOCENTRIC_H
#define QTOPOCENTRIC_H

#include <QString>
#include "qsgp4utilities.h"

struct QTopocentric
{
public:

    QTopocentric() : azimuth(0.0), elevation(0.0), range(0.0), range_rate(0.0)
    {
    }

    QTopocentric(
            double az,
            double el,
            double rnge,
            double rnge_rate)
        : azimuth(az), elevation(el), range(rnge), range_rate(rnge_rate)
    {
    }

    QTopocentric(const QTopocentric& topo)
    {
        azimuth = topo.azimuth;
        elevation = topo.elevation;
        range = topo.range;
        range_rate = topo.range_rate;
    }

    virtual ~QTopocentric()
    {
    }

    QTopocentric& operator=(const QTopocentric& topo)
    {
        if (this != &topo)
        {
            azimuth = topo.azimuth;
            elevation = topo.elevation;
            range = topo.range;
            range_rate = topo.range_rate;
        }
        return *this;
    }

    bool operator==(const QTopocentric& topo) const
    {
        return IsEqual(topo);
    }

    bool operator !=(const QTopocentric& topo) const
    {
        return !IsEqual(topo);
    }

    QString ToString() const
    {
        double az = Util::RadiansToDegrees(azimuth);
        double el = Util::RadiansToDegrees(elevation);
        QString retstr = QString("Az = %1°, El = %2°, Rng = %3 km, Rng Rt = %4 km/sec").arg(az, 6, 'f', 2).arg(el, 5, 'f', 2).arg(range, 9, 'f', 2).arg(range_rate, 6, 'f', 1);
        return retstr;

    }

    /** azimuth in radians */
    double azimuth;
    /** elevations in radians */
    double elevation;
    /** range in kilometers */
    double range;
    /** range rate in kilometers per second */
    double range_rate;

private:
    bool IsEqual(const QTopocentric& topo) const
    {
        bool equal = false;
        if (azimuth == topo.azimuth &&
                elevation == topo.elevation &&
                range == topo.range &&
                range_rate == topo.range_rate)
        {
            equal = true;
        }
        return equal;
    }
};


inline QDataStream & operator<<(QDataStream & strm, const QTopocentric& t)
{
    return strm << t.ToString();
}

#endif // QTOPOCENTRIC_H
