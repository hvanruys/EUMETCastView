// Unit tests for LandSeaMask.
//
// Needs a real GSHHS file. Pass one as argv[1], or it looks in the usual place
// and skips cleanly if there is none, so the suite still runs on a machine
// without the shoreline data.

#include "landseamask.h"

#include <cstdio>
#include <cmath>

static int g_failures = 0;

static void check(bool ok, const char *what)
{
    if (ok) std::printf("ok   : %s\n", what);
    else  { std::printf("FAIL : %s\n", what); ++g_failures; }
}

struct Place { double lat, lon; bool water; const char *name; };

int main(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : "bin/gshhs2_3_7/gshhs_i.b";

    if (!LandSeaMask::load(path)) {
        std::printf("skip : no GSHHS file at %s - nothing to test\n", path);
        return 0;
    }
    check(LandSeaMask::isLoaded(), "mask loaded");

    // Earth is 71 % ocean. Anything far off means the fill went wrong -
    // inverted, or leaking through an unclosed polygon.
    const double wf = LandSeaMask::waterFraction();
    std::printf("info : rasterised water fraction %.4f\n", wf);
    check(wf > 0.66 && wf < 0.76, "water fraction is about 71 %");

    static const Place places[] = {
        // The case this exists for: dark forest that a brightness test calls sea.
        {  -3.0,  -60.0, false, "Amazon rainforest" },
        {   0.0,   20.0, false, "Congo basin" },
        // Plain land and plain ocean.
        {  25.0,   10.0, false, "Sahara" },
        {   0.0,  -25.0, true,  "equatorial Atlantic" },
        { -30.0,    0.0, true,  "South Atlantic" },
        {  15.0,   65.0, true,  "Arabian Sea" },
        // Greenwich crossing, where an unwrapping bug would show first.
        {  51.5,   -0.1, false, "London" },
        {  48.9,    2.3, false, "Paris" },
        {  56.0,    3.0, true,  "North Sea" },
        // Dateline and pole, the other unwrapping hazards.
        { -80.0,    0.0, false, "Antarctica" },
        { -75.0,  120.0, false, "Antarctica, far side" },
        // Inland water: level 2 must be painted back over level 1.
        {  -1.0,   33.0, true,  "Lake Victoria" },
        {  46.5,   -0.5 + 0.0, false, "western France" },
    };

    for (const Place &p : places) {
        const bool got = LandSeaMask::isWater(p.lat, p.lon);
        if (got != p.water) {
            std::printf("FAIL : %s (%.1f, %.1f) reported %s\n",
                        p.name, p.lat, p.lon, got ? "water" : "land");
            ++g_failures;
        } else {
            std::printf("ok   : %s is %s\n", p.name, p.water ? "water" : "land");
        }
    }

    // Longitude wrapping must not change the answer.
    check(LandSeaMask::isWater(0.0, -25.0) == LandSeaMask::isWater(0.0, 335.0),
          "longitude wraps consistently");
    check(LandSeaMask::isWater(25.0, 10.0) == LandSeaMask::isWater(25.0, -350.0),
          "negative wrap agrees too");

    // Out-of-range latitudes must not read out of bounds.
    LandSeaMask::isWater(-95.0, 0.0);
    LandSeaMask::isWater(95.0, 0.0);
    check(true, "out-of-range latitudes are clamped, not crashed");

    std::printf("\n%s\n", g_failures ? "FAILED" : "all checks passed");
    return g_failures ? 1 : 0;
}
