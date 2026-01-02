// ============================================================================
// rayleighcorrector.h
// ============================================================================
#ifndef RAYLEIGHCORRECTOR_H
#define RAYLEIGHCORRECTOR_H

#include <QImage>
#include <QVector>
#include <QSize>
#include <QFuture>

// Constants for Rayleigh scattering
constexpr double EARTH_SUN_DISTANCE = 1.0; // AU, adjust seasonally if needed
constexpr double RAYLEIGH_PHASE_FUNCTION = 0.9596; // (1 + cos²θ) normalized

/**
 * Rayleigh correction for FCI MTG-I1 imagery using Qt 6.9 API
 */
class RayleighCorrector
{
public:
    struct CorrectionParams {
        double rayleighOpticalDepth;
        double surfacePressure = 1013.25; // hPa
        int bandNumber = 1;
    };

    /**
     * Get Rayleigh optical depth for FCI bands
     * @param bandNumber FCI band number (1-6 for VIS/NIR)
     * @return Rayleigh optical depth
     */
    static double getRayleighOpticalDepth(int bandNumber);

    /**
     * Perform Rayleigh correction on reflectance data
     *
     * @param reflectance Input TOA reflectance values
     * @param solarZenith Solar zenith angles in degrees
     * @param viewZenith Viewing zenith angles in degrees
     * @param relativeAzimuth Relative azimuth angles in degrees
     * @param params Correction parameters
     * @return Rayleigh-corrected surface reflectance
     */
    static QVector<float> correct(
        const QVector<float>& reflectance,
        const QVector<float>& solarZenith,
        const QVector<float>& viewZenith,
        const QVector<float>& relativeAzimuth,
        const CorrectionParams& params
        );

    /**
     * Parallel version using QtConcurrent for large datasets
     *
     * @param reflectance Input TOA reflectance values
     * @param solarZenith Solar zenith angles in degrees
     * @param viewZenith Viewing zenith angles in degrees
     * @param relativeAzimuth Relative azimuth angles in degrees
     * @param params Correction parameters
     * @return QFuture containing corrected reflectance
     */
    static QFuture<QVector<float>> correctParallel(
        const QVector<float>& reflectance,
        const QVector<float>& solarZenith,
        const QVector<float>& viewZenith,
        const QVector<float>& relativeAzimuth,
        const CorrectionParams& params
        );

    /**
     * Correct a QImage directly (assumes grayscale float data stored as RGB)
     *
     * @param inputImage Input image
     * @param solarZenith Solar zenith angles in degrees
     * @param viewZenith Viewing zenith angles in degrees
     * @param relativeAzimuth Relative azimuth angles in degrees
     * @param params Correction parameters
     * @return Corrected QImage
     */
    static QImage correctImage(
        const QImage& inputImage,
        const QVector<float>& solarZenith,
        const QVector<float>& viewZenith,
        const QVector<float>& relativeAzimuth,
        const CorrectionParams& params
        );

private:
    /**
     * Correct a single pixel
     *
     * @param reflectance Input reflectance value
     * @param solarZenithDeg Solar zenith angle in degrees
     * @param viewZenithDeg Viewing zenith angle in degrees
     * @param relativeAzimuthDeg Relative azimuth angle in degrees
     * @param tau_r Adjusted Rayleigh optical depth
     * @return Corrected reflectance value
     */
    static float correctPixel(
        float reflectance,
        float solarZenithDeg,
        float viewZenithDeg,
        float relativeAzimuthDeg,
        double tau_r
        );
};

#endif // RAYLEIGHCORRECTOR_H

