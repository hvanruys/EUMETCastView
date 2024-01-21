#include "qobserver.h"
#include "qtopocentric.h"

QTopocentric QObserver::GetLookAngle(QEci &eci)
{
    /*
     * update the observers Eci to match the time of the Eci passed in
     * if necessary
     */
    Update(eci.GetDate());

    /*
     * calculate differences
     */

    Vector3 range_rate = eci.GetVel() - m_eci.GetVel();
    Vector3 range = eci.GetPos() - m_eci.GetPos();


    /*
     * Calculate Local Mean Sidereal Time for observers longitude
     */
    double theta = eci.GetDate().ToLocalMeanSiderealTime(m_geo.longitude);

    double sin_lat = sin(m_geo.latitude);
    double cos_lat = cos(m_geo.latitude);
    double sin_theta = sin(theta);
    double cos_theta = cos(theta);

    double top_s = sin_lat * cos_theta * range.x
        + sin_lat * sin_theta * range.y - cos_lat * range.z;
    double top_e = -sin_theta * range.x
        + cos_theta * range.y;
    double top_z = cos_lat * cos_theta * range.x
        + cos_lat * sin_theta * range.y + sin_lat * range.z;
    double az = atan(-top_e / top_s);

    if (top_s > 0.0)
    {
        az += PIE;
    }

    if (az < 0.0)
    {
        az += 2.0 * PIE;
    }

    double el = asin(top_z / range.length());
    double rate = range.dot(range_rate) / range.length();
    /*
     * azimuth in radians
     * elevation in radians
     * range in km
     * range rate in km/s
     */
    return QTopocentric(az, el, range.length(), rate);
}
