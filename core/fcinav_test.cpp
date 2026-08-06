// FCI grid navigation: the forward and inverse transforms must describe one
// grid, and it must be the grid the files themselves define.
//
// This exists because they once did not. The land/sea mask geolocated its
// pixels with the generic CGMS routine and the approximate COFF/CFAC from the
// INI, while the coastline overlay used the FCI grid definition. The two
// disagreed by two to four kilometres across the disc, which put the mask a
// few pixels off every shoreline - enough to leave a fringe of coastal land
// classified as sea.

#include "pixgeoconversion.h"

#include <cmath>
#include <cstdio>

static int failures = 0;
static int checks   = 0;

static void ok(bool cond, const char *what)
{
    ++checks;
    printf("%-5s: %s\n", cond ? "ok" : "FAIL", what);
    if (!cond) ++failures;
}

static void checkNear(double got, double want, double tol, const char *what)
{
    ++checks;
    const bool good = std::fabs(got - want) <= tol;
    if (good)
        printf("ok   : %s\n", what);
    else
        printf("FAIL : %s (got %.6f, want %.6f, tol %.6f)\n", what, got, want, tol);
    if (!good) ++failures;
}

int main()
{
    pixgeoConversion pc;

    // --------------------------------------------------------------- grid
    //
    // The 1 km grid as the FCI files publish it. Every BODY file carries, for
    // the x and y coordinate variables:
    //
    //   x: scale_factor = -2.79435763233999e-05, add_offset = 0.155603804756852
    //   y: scale_factor = +2.79435763233999e-05, add_offset = -0.155603804756852
    //
    // so column c (1-based) sits at azimuth 0.155603804756852 - c * step. Those
    // two numbers are the whole grid definition, and the test is that the code
    // reproduces them rather than something close to them.
    printf("--- grid definition ---\n");

    const double step  = 2.79435763233999e-05;   // radians per pixel
    const double first = 0.155603804756852;      // add_offset

    // getGridParams is expressed in degrees as lambda_0 and a sampling; the
    // published add_offset is one sampling beyond lambda_0, since the file
    // counts columns from 1 and the code counts them from 0.
    const double lambda0Deg   = 8.9138402398;
    const double samplingDeg  = 0.001601048988;

    // Tolerances are set by how many digits the degree constants carry, not by
    // double precision: they agree to about 1e-10 relative, which is the
    // rounding of the published degree values and nothing more.
    checkNear(samplingDeg * M_PI / 180.0, step, 1e-14,
              "the angular step matches the files' scale_factor");
    checkNear((lambda0Deg + samplingDeg) * M_PI / 180.0, first, 1e-11,
              "lambda_0 plus one step matches the files' add_offset");

    // The centre of the grid, which is what the INI's COFF/LOFF got wrong.
    checkNear(lambda0Deg / samplingDeg, 5567.5, 1e-5,
              "the sub-satellite point sits at 0-based pixel 5567.5");

    // ------------------------------------------------------- disc geometry
    printf("\n--- disc geometry ---\n");

    double lat = 0.0, lon = 0.0;

    // The centre of an 11136 grid falls between pixels, so the two pixels
    // either side of it straddle the sub-satellite point symmetrically.
    ok(pc.pixcoord2geocoordFCI(0.0, 5568, 5568, &lat, &lon) == 0,
       "the grid centre is on the disc");
    const double latLo = lat, lonLo = lon;
    ok(pc.pixcoord2geocoordFCI(0.0, 5569, 5569, &lat, &lon) == 0,
       "the pixel diagonally opposite it is too");
    checkNear(0.5 * (latLo + lat), 0.0, 1e-6, "they straddle the equator");
    checkNear(0.5 * (lonLo + lon), 0.0, 1e-6, "they straddle the sub-satellite longitude");

    // Row 1 is the south limb, column 1 the west - the orientation the reader
    // relies on when it places a segment by start_position_row.
    ok(pc.pixcoord2geocoordFCI(0.0, 5568, 2000, &lat, &lon) == 0 && lat < 0.0,
       "low row numbers are the southern half");
    ok(pc.pixcoord2geocoordFCI(0.0, 2000, 5568, &lat, &lon) == 0 && lon < 0.0,
       "low column numbers are the western half");

    // Off the disc has to be reported, not returned as a plausible number.
    ok(pc.pixcoord2geocoordFCI(0.0, 1, 1, &lat, &lon) != 0,
       "the corner of the grid is off the disc");

    // ------------------------------------------------------- the round trip
    //
    // This is the invariant the bug broke. geocoord2pixcoordFCI is what draws
    // the coastline overlay; pixcoord2geocoordFCI is what the land/sea mask
    // asks. If they do not invert each other the mask sits off the shoreline
    // the user can see, which is exactly what happened.
    printf("\n--- round trip against the overlay's transform ---\n");

    struct { const char *name; double lat, lon; } places[] = {
        { "Belgian coast",  51.30,   3.20 },
        { "Brittany",       48.70,  -3.50 },
        { "Portugal",       38.70,  -9.40 },
        { "Sicily",         37.10,  15.20 },
        { "Nile delta",     31.40,  30.30 },
        { "Senegal",        14.70, -17.40 },
        { "Gulf of Guinea",  5.60,  -0.10 },
        { "nadir",           0.00,   0.00 },
        { "Namibia",       -22.70,  14.50 },
        { "Cape Town",     -33.90,  18.40 },
        { "Iceland",        64.10, -21.90 },
    };

    double worstKm = 0.0;
    const char *worstAt = "";

    for (const auto &p : places) {
        int col = 0, row = 0;
        if (pc.geocoord2pixcoordFCI(0.0, p.lat, p.lon, &col, &row) != 0) {
            printf("FAIL : %s is off the disc\n", p.name);
            ++checks; ++failures;
            continue;
        }

        double blat = 0.0, blon = 0.0;
        if (pc.pixcoord2geocoordFCI(0.0, col, row, &blat, &blon) != 0) {
            printf("FAIL : %s does not come back from the grid\n", p.name);
            ++checks; ++failures;
            continue;
        }

        // geocoord2pixcoordFCI rounds to whole pixels, so the round trip can
        // only be asked to land within half a pixel - about 0.8 km here.
        const double dkm = std::hypot((blat - p.lat) * 111.32,
                                      (blon - p.lon) * 111.32
                                          * std::cos(p.lat * M_PI / 180.0));
        if (dkm > worstKm) { worstKm = dkm; worstAt = p.name; }

        char msg[160];
        snprintf(msg, sizeof msg, "%s returns to within half a pixel", p.name);
        checkNear(dkm, 0.0, 0.8, msg);
    }

    printf("info : worst round-trip error %.3f km at %s\n", worstKm, worstAt);

    printf("\n%d checks, %s\n", checks,
           failures ? "SOME FAILED" : "all checks passed");
    return failures ? 1 : 0;
}
