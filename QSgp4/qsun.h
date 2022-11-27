#ifndef QSUN_H
#define QSUN_H
#include "sgp4sdp4.h"
#include "globals.h"
#include "qsgp4globals.h"
#include "Vectors.h"

class QSun
{
public:
    QSun();
    static void Calculate_Solar_Position(double time, Vector3 *solar_vector);

};

#endif // QSUN_H
