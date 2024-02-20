#include "moon.h"
#include "options.h"
#include "globals.h"
#include <QDebug>

extern Options opts;

// angular diameter of earth as seen from a geostationary orbit = 17.3°
// For Himawari-9
// image = 5500 x 5500 pixels
// QPoint pt(opts.geosatellites.at(geoindex).coff, opts.geosatellites.at(geoindex).loff);
// paint->setPen(Qt::red);
// paint->drawEllipse(pt, opts.geosatellites.at(geoindex).coff - 28, opts.geosatellites.at(geoindex).loff - 40);
// rx = opts.geosatellites.at(geoindex).coff - 28
// ry = opts.geosatellites.at(geoindex).loff - 40
// 5500 pixels --> 17.523°
// 1.0° --> 5500/17.523 = 313.873 = scale


void moonCalc::CalcMoon(QDate selected, int geosatindex)
{
    double hours, minutes;
    int day, year, month;
    int min = 10;
    int timeindex;
    year = selected.year();
    month = selected.month();
    day = selected.day();
    float scale = 313.873;

    double opposietlong = MapToMinus180To180Range(180 - opts.geosatellites.at(geosatindex).longitude);

    for( int hours = 0; hours < 24; hours++)
    {
        for( int minutes = 0; minutes < 60; minutes += min)
        {
            timeindex = (hours * 6) + (minutes/min);
//            double dday =  static_cast<double>(day) + static_cast<double>(hours)/24.0 + static_cast<double>(minutes)/(24.0*60.0);
            double dday =  static_cast<double>(day) + static_cast<double>(hours)/24.0 + static_cast<double>(minutes-3)/(24.0*60.0) +
                    static_cast<double>(50)/(24.0*60.0*60.0);
            double JD = CAADate::DateToJD(year, month, dday, true);

            double JDMoon = CAADynamicalTime::UTC2TT(JD);
            double MoonLong = CAAELP2000::EclipticLongitude(JDMoon);
            double MoonLat = CAAELP2000::EclipticLatitude(JDMoon);

            int MoonLongdeg, MoonLongmin, MoonLongsec;
            int MoonLatdeg, MoonLatmin, MoonLatsec;
            HoursTohms(MoonLong, MoonLongdeg, MoonLongmin, MoonLongsec);
            HoursTohms(MoonLat, MoonLatdeg, MoonLatmin, MoonLatsec);
            CAA2DCoordinate Equatorial = CAACoordinateTransformation::Ecliptic2Equatorial(MoonLong, MoonLat, CAANutation::TrueObliquityOfEcliptic(JDMoon));
            double MoonRad = CAAELP2000::RadiusVector(JDMoon);
            //MoonRad /= 149597870.691; //Convert KM to AU
            double AST = CAASidereal::ApparentGreenwichSiderealTime(JDMoon);
            double LongtitudeAsHourAngle = CAACoordinateTransformation::DegreesToHours( opposietlong );
            double LocalHourAngle = AST - LongtitudeAsHourAngle - Equatorial.X;

            double LHA = CAACoordinateTransformation::MapTo0To24Range(LocalHourAngle);
            int LHAhour, LHAmin, LHAsec;
            HoursTohms(LHA, LHAhour, LHAmin, LHAsec);
            int DEdeg, DEmin, DEsec;
            HoursTohms(Equatorial.Y, DEdeg, DEmin, DEsec);
            int RAhour, RAmin, RAsec;
            HoursTohms(Equatorial.X, RAhour, RAmin, RAsec);
            //printf("%d/%d/%d %2d:%02d UTC Ecliptic long = %02d°%02d'%02d\" lat = %02d°%02d'%02d\" RA = %02dh%02dm%02ds localHourAngle = %02dh%02dm%02ds DE = %02d°%02d'%02d\" MoonRad = %f\n",
            //   year, month, day, hours, minutes, MoonLongdeg, MoonLongmin, MoonLongsec, MoonLatdeg, MoonLatmin, MoonLatsec, RAhour, RAmin, RAsec, LHAhour, LHAmin, LHAsec, DEdeg, DEmin, DEsec, MoonRad);

            double diffangle = 9.0;

            double HourAngleDegrees = MapToMinus180To180Range(CAACoordinateTransformation::HoursToDegrees(LHA));
            double DecLHA = sqrt(HourAngleDegrees * HourAngleDegrees + Equatorial.Y * Equatorial.Y);
//            double parallax = 5.92 * sin(CAACoordinateTransformation::DegreesToRadians(DecLHA));
            double parallax = 5.40 * sin(CAACoordinateTransformation::DegreesToRadians(DecLHA));
            double deltaY = Equatorial.Y*parallax/DecLHA;
            double deltaX = HourAngleDegrees*parallax/DecLHA;

            double illuminated_fraction = 0;
            double position_angle = 0;
            double phase_angle = 0;

            GetMoonIllumination(JD, true, illuminated_fraction, position_angle, phase_angle);

            if(HourAngleDegrees - deltaX > -diffangle && HourAngleDegrees - deltaX < diffangle && Equatorial.Y - deltaY < diffangle && Equatorial.Y - deltaY > -diffangle)
            {

                printf("==%d/%d/%d %2d:%02d UTC Ecliptic long = %f° lat = %f° RA = %fh DE = %f° localHourAngle = %f° AST = %f MoonRad = %f parallax = %f\n",
                       year, month, day, hours, minutes, MoonLong, MoonLat, Equatorial.X, Equatorial.Y, HourAngleDegrees, AST, MoonRad, parallax);
                fflush(stdout);
                int ilfraction = static_cast<int>((illuminated_fraction * 100) + 0.5);
                moonVisible[timeindex] = ilfraction;

                moonCoordX[timeindex] = (int)((HourAngleDegrees - deltaX) * scale) + 2750;
                moonCoordY[timeindex] = 2750 - (int)((Equatorial.Y - deltaY) * scale);

//                moonlist.append(scene->addEllipse(mooncoordX - 0.25*scale, mooncoordY - 0.25*scale, 0.5*scale, 0.5*scale, blackpen, moonBrush));
//                QGraphicsTextItem *text = scene->addText(QString("%1:%2").arg(hours, 2, 'f', 0, '0').arg(minutes, 2, 'f', 0, '0'));
//                textlist.append(text);
//                text->setPos(mooncoordX-scale, mooncoordY+0.5*scale);
//                int ilfraction = static_cast<int>((illuminated_fraction * 100) + 0.5);

//                ui->listWidget->addItem(new QListWidgetItem(QString("%1:%2 illumination = %3 ").arg(hours, 2, 'f', 0, '0').arg(minutes, 2, 'f', 0, '0').arg(ilfraction), ui->listWidget));

            }
            else
            {
                moonVisible[timeindex] = 0;
                moonCoordX[timeindex] = 0;
                moonCoordY[timeindex] = 0;
            }

        }
    }

//    for(int i = 0; i < 144; i++)
//    {
//        qDebug() << QString("%1 illumated fraction = %2").arg(i).arg(moonVisible[i]);
//    }
}

void moonCalc::GetMoonIllumination(double JD, bool bHighPrecision, double& illuminated_fraction, double& position_angle, double& phase_angle)
{
  double moon_alpha{0};
  double moon_delta{0};
  GetLunarRaDecByJulian(JD, moon_alpha, moon_delta);
  double sun_alpha{0};
  double sun_delta{0};
  GetSolarRaDecByJulian(JD, bHighPrecision, sun_alpha, sun_delta);
  const double geo_elongation{CAAMoonIlluminatedFraction::GeocentricElongation(moon_alpha, moon_delta, sun_alpha, sun_delta)};

  position_angle = CAAMoonIlluminatedFraction::PositionAngle(sun_alpha, sun_delta, moon_alpha, moon_delta);
  phase_angle = CAAMoonIlluminatedFraction::PhaseAngle(geo_elongation, 368410.0, 149971520.0);
  illuminated_fraction = CAAMoonIlluminatedFraction::IlluminatedFraction(phase_angle);
}

void moonCalc::GetLunarRaDecByJulian(double JD, double& RA, double& Dec)
{
  const double JDMoon{CAADynamicalTime::UTC2TT(JD)};
  const double lambda{CAAMoon::EclipticLongitude(JDMoon)};
  const double beta{CAAMoon::EclipticLatitude(JDMoon)};
  const double epsilon{CAANutation::TrueObliquityOfEcliptic(JDMoon)};
  CAA2DCoordinate Lunarcoord{CAACoordinateTransformation::Ecliptic2Equatorial(lambda, beta, epsilon)};
  RA = Lunarcoord.X;
  Dec = Lunarcoord.Y;
}

void moonCalc::GetSolarRaDecByJulian(double JD, bool bHighPrecision, double& RA, double& Dec)
{
  const double JDSun{CAADynamicalTime::UTC2TT(JD)};
  const double lambda{CAASun::ApparentEclipticLongitude(JDSun, bHighPrecision)};
  const double beta{CAASun::ApparentEclipticLatitude(JDSun, bHighPrecision)};
  const double epsilon{CAANutation::TrueObliquityOfEcliptic(JDSun)};
  CAA2DCoordinate Solarcoord{CAACoordinateTransformation::Ecliptic2Equatorial(lambda, beta, epsilon)};
  RA = Solarcoord.X;
  Dec = Solarcoord.Y;
}


inline double moonCalc::MapToMinus180To180Range(double Degrees)
{
    double fResult = CAACoordinateTransformation::MapTo0To360Range(Degrees);

    if (fResult > 180)
      fResult = fResult - 360;

    return fResult;
}

int moonCalc::getTimeIndex(QString h, QString m)
{
    int hours = h.toInt();
    int minutes = m.toInt();

    return (hours * 6) + (minutes/10);
}

void moonCalc::getTimeFromIndex(int index, int *hours, int *minutes)
{
    int m = index % 6;
    int h = index / 6;
    *hours = h;
    *minutes = m*10;

}

