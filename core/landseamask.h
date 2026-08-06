#ifndef LANDSEAMASK_H
#define LANDSEAMASK_H

/**
 * Global land/sea mask, rasterised once from a GSHHS shoreline file.
 *
 * The Rayleigh correction needs to know where the sea surface is, because the
 * sea reflects skylight into the view in a way land does not. It had been
 * deciding that from band brightness alone - water is dark toward the red - and
 * that misreads dark vegetation at high view angle as ocean.
 *
 * This does not replace the brightness test, it joins it. The two answer
 * different questions and each covers the other's blind spot: geography knows
 * the Amazon is not the Atlantic, and brightness knows when cloud is sitting on
 * top of the Atlantic, where the sea surface is no longer what the satellite
 * sees.
 *
 * No OpenGL and no Qt, so it can be tested without a display or a data stream.
 */
class LandSeaMask
{
public:
    /**
     * Grid step in degrees. 0.01 is about 1.1 km, matching FCI's 1 km pixels at
     * nadir; a coastline then lands within half a cell, some 550 m.
     *
     * Costs 77 MB at one bit per cell, and about two seconds to rasterise the
     * high-resolution shoreline into it - once per run, against the half
     * gigabyte each band buffer of a single FCI compose already takes. Halving
     * the step again would quadruple both for a quarter of the error, which is
     * well under what a 1 km instrument can see.
     */
    static constexpr double GridStep = 0.01;
    static constexpr int    GridW    = (int)(360.0 / GridStep);   // 36000
    static constexpr int    GridH    = (int)(180.0 / GridStep);   // 18000

    /**
     * Load and rasterise. Safe to call repeatedly; only the first call for a
     * given path does the work, and a failed load is remembered so a missing
     * file does not cost a retry per image.
     *
     * @return false if the file could not be read, in which case isWater()
     *         reports everything as land and callers should fall back
     */
    static bool load(const char *gshhsPath);

    /** True once a mask has been rasterised and can be trusted. */
    static bool isLoaded();

    /**
     * Is this position open water? False for land, and false everywhere if no
     * mask is loaded - the conservative answer, since it disables the sea
     * surface rather than applying it where it does not belong.
     */
    static bool isWater(double latDeg, double lonDeg);

    /** Fraction of the globe rasterised as water. Sanity check; expect ~0.71. */
    static double waterFraction();
};

#endif // LANDSEAMASK_H
