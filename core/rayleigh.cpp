// zie https://pyspectral.readthedocs.io/en/master/rayleigh_correction.html
// zie https://www.mdpi.com/2072-4292/10/4/560
// ============================================================================
// rayleighcorrector.cpp
// ============================================================================
#include "rayleigh.h"
#include <QtMath>
#include <QDebug>
#include <QtConcurrent>

#include <QVector3D>
#include <QDateTime>
#include <QtMath>

double RayleighCorrector::getRayleighOpticalDepth(int bandNumber)
{
    switch(bandNumber) {
    case 1:  return 0.2174; // VIS 0.4 (444 nm)
    case 2:  return 0.1121; // VIS 0.5 (510 nm)
    case 3:  return 0.0492; // VIS 0.6 (640 nm)
    case 4:  return 0.0318; // VIS 0.8 (865 nm)
    case 5:  return 0.0087; // NIR 1.6 (1640 nm)
    case 6:  return 0.0028; // NIR 2.2 (2250 nm)
    default: return 0.0;    // No Rayleigh correction for IR bands
    }
}

QVector<float> RayleighCorrector::correct(
    const QVector<float>& reflectance,
    const QVector<float>& solarZenith,
    const QVector<float>& viewZenith,
    const QVector<float>& relativeAzimuth,
    const CorrectionParams& params
    )
{
    const qsizetype n = reflectance.size();

    if (solarZenith.size() != n || viewZenith.size() != n ||
        relativeAzimuth.size() != n) {
        qWarning() << "RayleighCorrector: Input array size mismatch";
        return QVector<float>();
    }

    QVector<float> corrected(n);

    // Adjust optical depth for actual surface pressure
    double tau_r = params.rayleighOpticalDepth *
                   (params.surfacePressure / 1013.25);

    for (qsizetype i = 0; i < n; ++i) {
        corrected[i] = correctPixel(
            reflectance[i],
            solarZenith[i],
            viewZenith[i],
            relativeAzimuth[i],
            tau_r
            );
    }

    return corrected;
}

QFuture<QVector<float>> RayleighCorrector::correctParallel(
    const QVector<float>& reflectance,
    const QVector<float>& solarZenith,
    const QVector<float>& viewZenith,
    const QVector<float>& relativeAzimuth,
    const CorrectionParams& params
    )
{
    return QtConcurrent::run([=]() {
        return correct(reflectance, solarZenith, viewZenith,
                       relativeAzimuth, params);
    });
}

QImage RayleighCorrector::correctImage(
    const QImage& inputImage,
    const QVector<float>& solarZenith,
    const QVector<float>& viewZenith,
    const QVector<float>& relativeAzimuth,
    const CorrectionParams& params
    )
{
    QSize size = inputImage.size();
    qsizetype pixelCount = size.width() * size.height();

    // Extract reflectance from image
    QVector<float> reflectance(pixelCount);

    for (int y = 0; y < size.height(); ++y) {
        for (int x = 0; x < size.width(); ++x) {
            qsizetype idx = y * size.width() + x;
            QRgb pixel = inputImage.pixel(x, y);
            // Assuming grayscale stored in red channel, normalized 0-1
            reflectance[idx] = qRed(pixel) / 255.0f;
        }
    }

    // Perform correction
    QVector<float> corrected = correct(
        reflectance, solarZenith, viewZenith, relativeAzimuth, params
        );

    // Create output image
    QImage outputImage(size, QImage::Format_RGB32);

    for (int y = 0; y < size.height(); ++y) {
        for (int x = 0; x < size.width(); ++x) {
            qsizetype idx = y * size.width() + x;
            float val = qBound(0.0f, corrected[idx], 1.0f);
            int grayValue = qRound(val * 255.0f);
            outputImage.setPixel(x, y, qRgb(grayValue, grayValue, grayValue));
        }
    }

    return outputImage;
}

float RayleighCorrector::correctPixel(
    float reflectance,
    float solarZenithDeg,
    float viewZenithDeg,
    float relativeAzimuthDeg,
    double tau_r
    )
{
    // Convert angles to radians
    double theta_s = qDegreesToRadians(static_cast<double>(solarZenithDeg));
    double theta_v = qDegreesToRadians(static_cast<double>(viewZenithDeg));
    double phi = qDegreesToRadians(static_cast<double>(relativeAzimuthDeg));

    // Cosines
    double mu_s = qCos(theta_s);
    double mu_v = qCos(theta_v);

    // Skip invalid data
    if (mu_s <= 0 || mu_v <= 0 || reflectance < 0) {
        return -999.0f; // Fill value for invalid data
    }

    // Scattering angle
    double cos_scatter = -mu_s * mu_v +
                         qSin(theta_s) * qSin(theta_v) * qCos(phi);

    // Rayleigh phase function: P(Θ) = 3/4 * (1 + cos²Θ)
    double phase = 0.75 * (1.0 + cos_scatter * cos_scatter);

    // Atmospheric transmission
    double T_s = qExp(-tau_r / mu_s);  // Sun to surface
    double T_v = qExp(-tau_r / mu_v);  // Surface to sensor
    double T_total = T_s * T_v;

    // Rayleigh reflectance (path radiance contribution)
    double rho_ray = (tau_r * phase) / (4.0 * M_PI * mu_s * mu_v) *
                     (1.0 - qExp(-tau_r * (1.0/mu_s + 1.0/mu_v)));

    // Correct for multiple scattering (simple approximation)
    double rho_multiple = rho_ray / (1.0 - 0.5 * tau_r);

    // Remove Rayleigh contribution and account for transmission
    double rho_surface = (reflectance - rho_multiple) / T_total;

    // Ensure non-negative values
    return static_cast<float>(qMax(0.0, rho_surface));
}

// ============================================================================
// Example usage in main.cpp or another file
// ============================================================================
/*
#include "rayleighcorrector.h"
#include <QDebug>

void exampleUsage()
{
    // Prepare input data
    QVector<float> reflectance;    // TOA reflectance values
    QVector<float> solarZenith;    // Solar zenith angles
    QVector<float> viewZenith;     // Viewing zenith angles
    QVector<float> relativeAzimuth; // Relative azimuth angles

    // Set correction parameters
    RayleighCorrector::CorrectionParams params;
    params.bandNumber = 3; // VIS 0.6 band
    params.rayleighOpticalDepth = RayleighCorrector::getRayleighOpticalDepth(3);
    params.surfacePressure = 1013.25;

    // Perform correction (blocking)
    QVector<float> corrected = RayleighCorrector::correct(
        reflectance, solarZenith, viewZenith, relativeAzimuth, params
    );

    // Or use parallel version for large datasets
    QFuture<QVector<float>> future = RayleighCorrector::correctParallel(
        reflectance, solarZenith, viewZenith, relativeAzimuth, params
    );

    // Wait for result
    QVector<float> correctedParallel = future.result();

    qDebug() << "Correction complete. Processed" << corrected.size() << "pixels";
}
*/


// SGP4 constants
const double PI = 3.14159265358979323846;
const double TWO_PI = 2.0 * PI;
const double MINUTES_PER_DAY = 1440.0;
const double EARTH_RADIUS_KM = 6378.137;
const double XKE = 0.07436691613317; // (G*M_earth)^(1/2)*(er/min)^(3/2)
const double J2 = 0.00108262998905; // Second zonal harmonic of earth

struct TLE {
    QString line1;
    QString line2;
    double epoch;           // Julian date
    double meanMotion;      // revs per day
    double eccentricity;
    double inclination;     // radians
    double raan;           // right ascension of ascending node (radians)
    double argPerigee;     // argument of perigee (radians)
    double meanAnomaly;    // radians
    double bstar;          // drag term
};



#include <QVector3D>
#include <QDateTime>
#include <QtMath>

// Parse TLE format
TLE parseTLE(const QString& line1, const QString& line2) {
    TLE tle;
    tle.line1 = line1;
    tle.line2 = line2;

    // Parse epoch from line 1 (columns 19-32)
    QString epochStr = line1.mid(18, 14);
    double epochYear = epochStr.left(2).toDouble();
    epochYear += (epochYear < 57) ? 2000 : 1900;
    double epochDay = epochStr.mid(2).toDouble();

    // Convert to Julian date (simplified)
    tle.epoch = 367.0 * epochYear - floor(7.0 * (epochYear + floor(10.0/12.0)) / 4.0) +
                floor(275.0 * 1.0 / 9.0) + epochDay + 1721013.5;

    // Parse BSTAR drag term (columns 54-61 in line 1)
    QString bstarStr = line1.mid(53, 8);
    tle.bstar = bstarStr.toDouble() / 100000.0;

    // Parse line 2
    tle.inclination = line2.mid(8, 8).toDouble() * PI / 180.0;
    tle.raan = line2.mid(17, 8).toDouble() * PI / 180.0;
    QString eccStr = "0." + line2.mid(26, 7);
    tle.eccentricity = eccStr.toDouble();
    tle.argPerigee = line2.mid(34, 8).toDouble() * PI / 180.0;
    tle.meanAnomaly = line2.mid(43, 8).toDouble() * PI / 180.0;
    tle.meanMotion = line2.mid(52, 11).toDouble();

    return tle;
}

// Solve Kepler's equation for eccentric anomaly
double solveKepler(double meanAnomaly, double eccentricity, int maxIter = 10) {
    double E = meanAnomaly;
    for (int i = 0; i < maxIter; i++) {
        double dE = (meanAnomaly - E + eccentricity * sin(E)) / (1.0 - eccentricity * cos(E));
        E += dE;
        if (fabs(dE) < 1e-10) break;
    }
    return E;
}

// Calculate satellite position vector in TEME (True Equator Mean Equinox) coordinates
QVector3D calculateSatellitePosition(const TLE& tle, const QDateTime& targetTime) {
    // Calculate time since epoch in minutes
    double jdTarget = targetTime.toMSecsSinceEpoch() / 86400000.0 + 2440587.5;
    double tsince = (jdTarget - tle.epoch) * MINUTES_PER_DAY;

    // Simplified SGP4 (for near-circular orbits like GEO)
    double n0 = tle.meanMotion * TWO_PI / MINUTES_PER_DAY; // mean motion in rad/min
    double a0 = pow(XKE / n0, 2.0/3.0); // semi-major axis in earth radii

    // Secular effects of J2
    double delta1 = 1.5 * J2 * pow(EARTH_RADIUS_KM / (a0 * EARTH_RADIUS_KM), 2) *
                    (3.0 * pow(cos(tle.inclination), 2) - 1.0) /
                    pow(1.0 - pow(tle.eccentricity, 2), 1.5);

    double a1 = a0 * pow(1.0 - delta1/3.0 - delta1*delta1 - 134.0*pow(delta1, 3)/81.0, 2);
    double delta0 = 1.5 * J2 * pow(EARTH_RADIUS_KM / (a1 * EARTH_RADIUS_KM), 2) *
                    (3.0 * pow(cos(tle.inclination), 2) - 1.0) /
                    pow(1.0 - pow(tle.eccentricity, 2), 1.5);

    double n0pp = n0 / (1.0 + delta0); // corrected mean motion
    double a = a1 / (1.0 - delta0); // semi-major axis

    // Update for secular perturbations
    double M = tle.meanAnomaly + n0pp * tsince; // mean anomaly at target time
    double omega = tle.argPerigee; // argument of perigee
    double Omega = tle.raan; // RAAN

    // Solve Kepler's equation
    double E = solveKepler(M, tle.eccentricity);

    // True anomaly
    double nu = 2.0 * atan2(sqrt(1.0 + tle.eccentricity) * sin(E/2.0),
                            sqrt(1.0 - tle.eccentricity) * cos(E/2.0));

    // Radius
    double r = a * EARTH_RADIUS_KM * (1.0 - tle.eccentricity * cos(E));

    // Position in orbital plane
    double x_orb = r * cos(nu);
    double y_orb = r * sin(nu);

    // Rotate to TEME frame
    double cosO = cos(Omega);
    double sinO = sin(Omega);
    double coso = cos(omega);
    double sino = sin(omega);
    double cosi = cos(tle.inclination);
    double sini = sin(tle.inclination);

    double x = x_orb * (cosO * coso - sinO * sino * cosi) - y_orb * (cosO * sino + sinO * coso * cosi);
    double y = x_orb * (sinO * coso + cosO * sino * cosi) + y_orb * (cosO * coso * cosi - sinO * sino);
    double z = x_orb * sino * sini + y_orb * coso * sini;

    return QVector3D(x, y, z);
}

// Calculate Greenwich Mean Sidereal Time (GMST) in radians
double calculateGMST(const QDateTime& utcTime) {
    // Convert to Julian Date
    double jd = utcTime.toMSecsSinceEpoch() / 86400000.0 + 2440587.5;

    // Days since J2000.0
    double T = (jd - 2451545.0) / 36525.0;

    // GMST in seconds (IAU 1982 formula)
    double gmst_sec = 67310.54841 +
                      (876600.0 * 3600.0 + 8640184.812866) * T +
                      0.093104 * T * T -
                      6.2e-6 * T * T * T;

    // Convert to radians and normalize to [0, 2π]
    double gmst_rad = fmod(gmst_sec * PI / 43200.0, TWO_PI);
    if (gmst_rad < 0) gmst_rad += TWO_PI;

    return gmst_rad;
}

// Convert TEME to ECEF coordinates
QVector3D temeToECEF(const QVector3D& teme, const QDateTime& utcTime) {
    // Calculate GMST (Greenwich Mean Sidereal Time)
    double gmst = calculateGMST(utcTime);

    // Rotation matrix from TEME to ECEF (rotation about Z-axis by -GMST)
    double cosGMST = cos(gmst);
    double sinGMST = sin(gmst);

    // Apply rotation
    double x_ecef = cosGMST * teme.x() + sinGMST * teme.y();
    double y_ecef = -sinGMST * teme.x() + cosGMST * teme.y();
    double z_ecef = teme.z();

    return QVector3D(x_ecef, y_ecef, z_ecef);
}

// Main function to get satellite position in ECEF coordinates
QVector3D getSatellitePositionECEF(const QString& line1, const QString& line2,
                                   const QDateTime& targetTime) {
    TLE tle = parseTLE(line1, line2);
    QVector3D positionTEME = calculateSatellitePosition(tle, targetTime);
    QVector3D positionECEF = temeToECEF(positionTEME, targetTime);
    return positionECEF;
}

// Example usage
// int main() {
//     // Example TLE for MTG-I1 (you need to replace with actual current TLE)
//     QString line1 = "1 49916U 21111A   24001.50000000  .00000000  00000-0  00000+0 0  9999";
//     QString line2 = "2 49916   0.0000   0.0000 0000000   0.0000   0.0000  1.00273791  9999";

//     QDateTime targetTime = QDateTime::currentDateTimeUtc();

//     // Get position in TEME coordinates
//     TLE tle = parseTLE(line1, line2);
//     QVector3D positionTEME = calculateSatellitePosition(tle, targetTime);

//     qDebug() << "Satellite position TEME (km):";
//     qDebug() << "X:" << positionTEME.x();
//     qDebug() << "Y:" << positionTEME.y();
//     qDebug() << "Z:" << positionTEME.z();

//     // Convert to ECEF coordinates
//     QVector3D positionECEF = temeToECEF(positionTEME, targetTime);

//     qDebug() << "\nSatellite position ECEF (km):";
//     qDebug() << "X:" << positionECEF.x();
//     qDebug() << "Y:" << positionECEF.y();
//     qDebug() << "Z:" << positionECEF.z();

//     // Or use the convenience function
//     QVector3D position = getSatellitePositionECEF(line1, line2, targetTime);

//     return 0;
// }
