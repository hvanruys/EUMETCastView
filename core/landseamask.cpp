// Land/sea mask rasterised from GSHHS. See landseamask.h.
//
// Format: a GSHHS header per polygon, big-endian, followed by n lon/lat pairs
// in micro-degrees. Level 1 is land, 2 lake, 3 island in lake, 4 pond in that
// island, so painting in level order gets islands inside lakes right. GSHHG 2.2
// added 5 for the Antarctic ice front and 6 for the grounding line - both land,
// and both must be accepted or the file stops parsing partway through.

#include "landseamask.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
#include <mutex>
#include <string>

namespace {

struct GshhsHeader {
    int id, n, flag, west, east, south, north, area, area_full, container, ancestor;
};

inline int swab4(int v)
{
    const unsigned u = (unsigned)v;
    return (int)(((u >> 24) & 0x000000FFu) | ((u >>  8) & 0x0000FF00u)
               | ((u <<  8) & 0x00FF0000u) | ((u << 24) & 0xFF000000u));
}

constexpr int kMaxLevel = 6;

struct Poly {
    int level;
    std::vector<float> lon, lat;      // degrees, longitudes unwrapped
};

std::vector<unsigned char> g_bits;    // one bit per cell, 1 = water
bool      g_loaded = false;
double    g_waterFraction = 0.0;
std::once_flag g_once;
std::string    g_path;

inline void setCell(int x, int y, bool water)
{
    x %= LandSeaMask::GridW;
    if (x < 0) x += LandSeaMask::GridW;
    if (y < 0 || y >= LandSeaMask::GridH) return;

    const size_t i = (size_t)y * LandSeaMask::GridW + x;
    if (water) g_bits[i >> 3] |=  (unsigned char)(1u << (i & 7));
    else       g_bits[i >> 3] &= (unsigned char)~(1u << (i & 7));
}

/**
 * Even-odd scanline fill. Longitudes arrive unwrapped, so a polygon crossing
 * Greenwich or the dateline is a continuous run here and only wraps when the
 * column index is taken modulo the grid width.
 */
void fillPolygon(const Poly &p, bool water)
{
    if (p.lat.size() < 3)
        return;

    float latMin = p.lat[0], latMax = p.lat[0];
    for (size_t i = 1; i < p.lat.size(); ++i) {
        latMin = std::min(latMin, p.lat[i]);
        latMax = std::max(latMax, p.lat[i]);
    }

    int y0 = (int)std::floor((latMin + 90.0) / LandSeaMask::GridStep);
    int y1 = (int)std::ceil ((latMax + 90.0) / LandSeaMask::GridStep);
    y0 = std::max(0, y0);
    y1 = std::min(LandSeaMask::GridH - 1, y1);

    std::vector<float> xs;
    const size_t n = p.lat.size();

    for (int y = y0; y <= y1; ++y) {
        const double yLat = -90.0 + (y + 0.5) * LandSeaMask::GridStep;

        xs.clear();
        for (size_t i = 0, j = n - 1; i < n; j = i++) {
            const double la = p.lat[i], lb = p.lat[j];
            if ((la > yLat) == (lb > yLat))
                continue;                       // edge does not cross this row
            const double t = (yLat - la) / (lb - la);
            xs.push_back((float)(p.lon[i] + t * (p.lon[j] - p.lon[i])));
        }
        if (xs.size() < 2)
            continue;

        std::sort(xs.begin(), xs.end());
        for (size_t k = 0; k + 1 < xs.size(); k += 2) {
            int xa = (int)std::floor((xs[k]     + 180.0) / LandSeaMask::GridStep);
            int xb = (int)std::ceil ((xs[k + 1] + 180.0) / LandSeaMask::GridStep);
            if (xb - xa > LandSeaMask::GridW) xb = xa + LandSeaMask::GridW;
            for (int x = xa; x < xb; ++x)
                setCell(x, y, water);
        }
    }
}

bool readPolygons(const char *path, std::vector<Poly> byLevel[kMaxLevel])
{
    FILE *fp = std::fopen(path, "rb");
    if (!fp)
        return false;

    GshhsHeader h;
    bool flip = false, first = true;
    long count = 0;

    while (std::fread(&h, sizeof(GshhsHeader), 1, fp) == 1) {
        if (first) {
            // GSHHS is written big-endian. Decide by trying both readings and
            // taking whichever yields a usable level and point count - the
            // version byte alone is not enough, since the wrong byte order can
            // still leave a plausible-looking value there.
            auto sane = [](int n, int flag) {
                const int lvl = flag & 255;
                return lvl >= 1 && lvl <= kMaxLevel && n > 0 && n < 50000000;
            };
            const bool nativeOk = sane(h.n, h.flag);
            const bool swapOk   = sane(swab4(h.n), swab4(h.flag));
            if (!nativeOk && !swapOk) {
                std::fclose(fp);
                return false;                    // not a GSHHS file
            }
            flip = !nativeOk;
            first = false;
        }
        if (flip) {
            h.n    = swab4(h.n);
            h.flag = swab4(h.flag);
        }

        const int level = h.flag & 255;
        if (h.n <= 0 || h.n > 50000000 || level < 1 || level > kMaxLevel) {
            std::fclose(fp);
            return false;                        // not a GSHHS file we understand
        }

        std::vector<int> raw((size_t)h.n * 2);
        if (std::fread(raw.data(), sizeof(int), raw.size(), fp) != raw.size()) {
            std::fclose(fp);
            return false;
        }

        Poly p;
        p.level = level;
        p.lon.reserve(h.n);
        p.lat.reserve(h.n);

        double prev = 0.0;
        for (int i = 0; i < h.n; ++i) {
            int xi = raw[(size_t)i * 2], yi = raw[(size_t)i * 2 + 1];
            if (flip) { xi = swab4(xi); yi = swab4(yi); }

            double lon = xi * 1e-6;
            const double lat = yi * 1e-6;
            if (lon > 180.0) lon -= 360.0;

            // Unwrap: keep the run continuous across Greenwich and the dateline
            // so the scanline fill sees one span instead of two half-spans.
            if (i > 0) {
                while (lon - prev >  180.0) lon -= 360.0;
                while (lon - prev < -180.0) lon += 360.0;
            }
            prev = lon;

            p.lon.push_back((float)lon);
            p.lat.push_back((float)lat);
        }

        byLevel[level - 1].push_back(std::move(p));
        ++count;
    }

    std::fclose(fp);
    return count > 0;
}

void build(const char *path)
{
    std::vector<Poly> byLevel[kMaxLevel];
    if (!readPolygons(path, byLevel))
        return;

    // Start as all water, then land, lakes, islands in lakes, ponds in those,
    // and finally Antarctica. Order is what makes an island in a lake come out
    // as land again.
    g_bits.assign(((size_t)LandSeaMask::GridW * LandSeaMask::GridH + 7) / 8, 0xFF);

    for (int lvl = 0; lvl < kMaxLevel; ++lvl) {
        const bool water = (lvl == 1 || lvl == 3);   // lake, pond; 5 and 6 are ice
        for (const Poly &p : byLevel[lvl])
            fillPolygon(p, water);
    }

    size_t wet = 0;
    const size_t total = (size_t)LandSeaMask::GridW * LandSeaMask::GridH;
    for (size_t i = 0; i < total; ++i)
        if (g_bits[i >> 3] & (1u << (i & 7))) ++wet;
    g_waterFraction = (double)wet / (double)total;

    g_loaded = true;
}

} // namespace

bool LandSeaMask::load(const char *gshhsPath)
{
    if (!gshhsPath || !*gshhsPath)
        return false;

    g_path = gshhsPath;
    std::call_once(g_once, [] { build(g_path.c_str()); });
    return g_loaded;
}

bool LandSeaMask::isLoaded()
{
    return g_loaded;
}

double LandSeaMask::waterFraction()
{
    return g_waterFraction;
}

bool LandSeaMask::isWater(double latDeg, double lonDeg)
{
    if (!g_loaded)
        return false;

    while (lonDeg >= 180.0) lonDeg -= 360.0;
    while (lonDeg < -180.0) lonDeg += 360.0;

    int x = (int)((lonDeg + 180.0) / GridStep);
    int y = (int)((latDeg +  90.0) / GridStep);
    x = std::max(0, std::min(GridW - 1, x));
    y = std::max(0, std::min(GridH - 1, y));

    const size_t i = (size_t)y * GridW + x;
    return (g_bits[i >> 3] & (1u << (i & 7))) != 0;
}
