#ifndef MOON_H
#define MOON_H

#include <QDate>
#include "AA+.h"

class moonCalc
{
public:
    static int moonVisible[144];
    static int moonCoordX[144];
    static int moonCoordY[144];

    void CalcMoon(QDate selected, int geosatindex);
    int getTimeIndex(QString hours, QString minutes);
    void getTimeFromIndex(int index, int *hours, int *minutes);
    int moonIsVisible(int index) { return moonVisible[index]; };
    int getmoonCoordX(int index) { return moonCoordX[index]; };
    int getmoonCoordY(int index) { return moonCoordY[index]; };


private:
    void GetMoonIllumination(double JD, bool bHighPrecision, double& illuminated_fraction, double& position_angle, double& phase_angle);
    void GetLunarRaDecByJulian(double JD, double& RA, double& Dec);
    void GetSolarRaDecByJulian(double JD, bool bHighPrecision, double& RA, double& Dec);
    double MapToMinus180To180Range(double Degrees);

};

#endif // MOON_H
