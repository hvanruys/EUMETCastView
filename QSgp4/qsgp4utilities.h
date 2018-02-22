#ifndef QSGP4UTILITIES_H
#define QSGP4UTILITIES_H

#include "qsgp4globals.h"


namespace Util
{

    /*
     * always positive result
     * Mod(-3,4)= 1   fmod(-3,4)= -3
     */
    inline double Mod(const double x, const double y)
    {
        if (y == 0)
        {
            return x;
        }

        return x - y * floor(x / y);
    }

    inline double Mod1(const double x, const double y, double *result)
    {
        if (y == 0)
        {
            return x;
        }

        *result = floor(x/y);

        return x - y * *result;
    }

    inline double WrapNegPosPI(const double a)
    {
        return Mod(a + PI, TWOPI) - PI;
    }

    inline double WrapTwoPI(const double a)
    {
        return Mod(a, TWOPI);
    }

    inline double WrapNegPos180(const double a)
    {
        return Mod(a + 180.0, 360.0) - 180.0;
    }

    inline double Wrap360(const double a)
    {
        return Mod(a, 360.0);
    }

    inline double DegreesToRadians(const double degrees)
    {
        return degrees * PI / 180.0;
    }

    inline double RadiansToDegrees(const double radians)
    {
        return radians * 180.0 / PI;
    }

    inline double AcTan(const double sinx, const double cosx)
    {
        if (cosx == 0.0)
        {
            if (sinx > 0.0)
            {
                return PI / 2.0;
            }
            else
            {
                return 3.0 * PI / 2.0;
            }
        }
        else
        {
            if (cosx > 0.0)
            {
                return atan(sinx / cosx);
            }
            else
            {
                return PI + atan(sinx / cosx);
            }
        }
    }

}

#endif // QSGP4UTILITIES_H
