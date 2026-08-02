// Regression test: the snu_solar_params2 refactor must be behaviour-preserving.
// Golden values were captured from the pre-refactor implementation.
//
// Build with: cmake -DBUILD_TESTS=ON ..    Run: bin/nav_util_test

#include "internal.h"
#include "nav_util.h"

#include <cmath>
#include <cstdio>

static int g_failures = 0;

static void checkClose(double got, double want, double absTol, const char *what)
{
    if (std::fabs(got - want) <= absTol) {
        std::printf("ok   : %s\n", what);
    } else {
        std::printf("FAIL : %s (got %.12f, want %.12f)\n", what, got, want);
        ++g_failures;
    }
}

struct GoldenCase {
    double jtime;
    double latDeg;
    double lonDeg;
    double mu0;
    double theta0;
    double phi0;
};

// Captured from snu_solar_params2 before the epoch split.
static const GoldenCase kGolden[] = {
    { 2460520.937500,   0.0000,    0.0000, 0.865147990000, 0.525350945282, 0.882808586559 },
    { 2460520.937500,  50.0000,    4.0000, 0.816074475102, 0.616210439899, 2.541995970279 },
    { 2460520.937500, -30.0000,  -45.0000, 0.133311171055, 1.437087099294, 1.105494731001 },
    { 2460521.250000,  60.0000,   30.0000, 0.049332356718, 1.521443938258, 5.294321500286 },
    { 2460704.500000, -70.0000,  170.0000, 0.605833089730, 0.919983744167, 0.278031756823 },
};

static const int kGoldenCount = sizeof(kGolden) / sizeof(kGolden[0]);

static void testSolarParams2Unchanged()
{
    for (int i = 0; i < kGoldenCount; ++i) {
        const GoldenCase &c = kGolden[i];
        double mu0, theta0, phi0;
        snu_solar_params2(c.jtime, c.latDeg * D2R, c.lonDeg * D2R,
                          &mu0, &theta0, &phi0, NULL);
        checkClose(mu0,    c.mu0,    1e-11, "snu_solar_params2 mu0 unchanged");
        checkClose(theta0, c.theta0, 1e-11, "snu_solar_params2 theta0 unchanged");
        checkClose(phi0,   c.phi0,   1e-11, "snu_solar_params2 phi0 unchanged");
    }
}

static void testEpochSplitMatches()
{
    // The split path must agree with the monolithic one to the last bit that
    // matters, for every golden case.
    for (int i = 0; i < kGoldenCount; ++i) {
        const GoldenCase &c = kGolden[i];

        double mu0a, theta0a, phi0a;
        snu_solar_params2(c.jtime, c.latDeg * D2R, c.lonDeg * D2R,
                          &mu0a, &theta0a, &phi0a, NULL);

        struct snu_solar_epoch e;
        snu_solar_epoch_init(c.jtime, &e);

        double mu0b, theta0b, phi0b;
        snu_solar_params_at(&e, c.jtime, c.latDeg * D2R, c.lonDeg * D2R,
                            &mu0b, &theta0b, &phi0b);

        checkClose(mu0b,    mu0a,    1e-12, "epoch split mu0 matches");
        checkClose(theta0b, theta0a, 1e-12, "epoch split theta0 matches");
        checkClose(phi0b,   phi0a,   1e-12, "epoch split phi0 matches");
    }
}

static void testEpochReuseAcrossPixels()
{
    // One epoch reused across many longitudes must still match per-call results.
    const double jtime = 2460520.937500;
    struct snu_solar_epoch e;
    snu_solar_epoch_init(jtime, &e);

    for (double lonDeg = -180.0; lonDeg <= 180.0; lonDeg += 15.0) {
        for (double latDeg = -75.0; latDeg <= 75.0; latDeg += 15.0) {
            double mu0a, theta0a, phi0a;
            snu_solar_params2(jtime, latDeg * D2R, lonDeg * D2R,
                              &mu0a, &theta0a, &phi0a, NULL);

            double mu0b, theta0b, phi0b;
            snu_solar_params_at(&e, jtime, latDeg * D2R, lonDeg * D2R,
                                &mu0b, &theta0b, &phi0b);

            checkClose(mu0b,    mu0a,    1e-12, "reused epoch mu0 matches");
            checkClose(theta0b, theta0a, 1e-12, "reused epoch theta0 matches");
            checkClose(phi0b,   phi0a,   1e-12, "reused epoch phi0 matches");
        }
    }
}

// The viewing geometry. This went untested, and carried a sign error in the
// local vertical for years: the up axis is (cos_lat*cos_lon, cos_lat*sin_lon,
// sin_lat), and the middle term was negated. Wherever sin_lon is small the
// error nearly vanishes, so a satellite over the Greenwich meridian looked
// roughly right while Meteosat-9 at 45.5 E reported a viewing zenith angle of
// 180 degrees at its own sub-satellite point, and 114 to 123 degrees across the
// part of the disc a Rayleigh correction cares most about.
static void testViewingGeometry()
{
    const double d2r = M_PI / 180.0;
    const double Rgeo = 42164.0;          // geostationary radius, km

    // Independent implementation, written from the definition of the local
    // east-north-up frame rather than from the code under test.
    auto reference = [&](double lat, double lon, double X, double Y, double Z,
                         double *vza, double *vaa) {
        const double a = 6378.1690, b = 6356.5838;
        const double e2 = 1.0 - b * b / (a * a);
        const double cl = std::cos(lat * d2r), sl = std::sin(lat * d2r);
        const double co = std::cos(lon * d2r), so = std::sin(lon * d2r);
        const double N  = a / std::sqrt(1.0 - e2 * sl * sl);
        const double qx = X - N * cl * co;
        const double qy = Y - N * cl * so;
        const double qz = Z - (b * b) / (a * a) * N * sl;
        const double u = qx * ( cl * co) + qy * ( cl * so) + qz * sl;
        const double n = qx * (-sl * co) + qy * (-sl * so) + qz * cl;
        const double e = qx * (-so)      + qy * ( co);
        *vza = std::acos(u / std::sqrt(u * u + n * n + e * e)) / d2r;
        *vaa = std::atan2(e, n) / d2r;
        if (*vaa < 0.0) *vaa += 360.0;
    };

    // Every geostationary longitude this application supports, not just zero.
    const double sublon[] = { -75.0, -37.5, 0.0, 9.5, 45.5, 86.5, 140.7 };

    for (double sl : sublon) {
        const double X = Rgeo * std::cos(sl * d2r);
        const double Y = Rgeo * std::sin(sl * d2r);
        const double Z = 0.0;

        // Looking straight down the boresight: the satellite is at the zenith.
        float vza, vaa;
        snu_vza_and_vaa(0.0, sl, 0.0, X, Y, Z, &vza, &vaa);
        checkClose(vza, 0.0, 1e-3, "sub-satellite point sees the satellite overhead");

        for (double plat = -60.0; plat <= 60.0; plat += 30.0) {
            for (double dlon = -60.0; dlon <= 60.0; dlon += 30.0) {
                double rvza, rvaa;
                snu_vza_and_vaa(plat, sl + dlon, 0.0, X, Y, Z, &vza, &vaa);
                reference(plat, sl + dlon, X, Y, Z, &rvza, &rvaa);
                checkClose(vza, rvza, 1e-3, "viewing zenith matches the reference");
                // Azimuth is degenerate at the sub-satellite point, where the
                // satellite is overhead and there is no direction to it.
                if (rvza > 0.5)
                    checkClose(vaa, rvaa, 1e-3, "viewing azimuth matches the reference");
            }
        }
    }

    // Nothing on a geostationary disc is ever below the horizon: a viewing
    // zenith angle at or past 90 degrees means the geometry is wrong, and it is
    // what the path reflectance clamp turned into a subtracted haze so large it
    // erased the blue channel.
    for (double sl : sublon) {
        const double X = Rgeo * std::cos(sl * d2r);
        const double Y = Rgeo * std::sin(sl * d2r);
        for (double plat = -70.0; plat <= 70.0; plat += 10.0) {
            for (double dlon = -70.0; dlon <= 70.0; dlon += 10.0) {
                // Only points the satellite can actually see. The geometric
                // horizon is at about 81 degrees of great-circle distance;
                // stay inside it, since past that a vza over 90 is the truth.
                const double gc = std::acos(std::cos(plat * d2r) * std::cos(dlon * d2r)) / d2r;
                if (gc > 75.0)
                    continue;
                float vza, vaa;
                snu_vza_and_vaa(plat, sl + dlon, 0.0, X, Y, 0.0, &vza, &vaa);
                if (vza >= 90.0f) {
                    std::printf("FAIL : vza %.2f at lat %.0f lon %.0f, satellite at %.1f\n",
                                vza, plat, sl + dlon, sl);
                    ++g_failures;
                    return;
                }
            }
        }
    }
    std::printf("ok   : nothing on the disc is ever below the horizon\n");
}

int main()
{
    testSolarParams2Unchanged();
    testEpochSplitMatches();
    testEpochReuseAcrossPixels();
    testViewingGeometry();

    if (g_failures) {
        std::printf("\n%d check(s) FAILED\n", g_failures);
        return 1;
    }
    std::printf("\nall checks passed\n");
    return 0;
}
