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

int main()
{
    testSolarParams2Unchanged();
    testEpochSplitMatches();
    testEpochReuseAcrossPixels();

    if (g_failures) {
        std::printf("\n%d check(s) FAILED\n", g_failures);
        return 1;
    }
    std::printf("\nall checks passed\n");
    return 0;
}
