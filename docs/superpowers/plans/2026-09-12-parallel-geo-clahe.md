# Parallel CLAHE on the geostationary button path — implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the geostationary CLAHE button run on every core and use an order of magnitude less memory, with output identical to today's for every image whose region size is even.

**Architecture:** `SegmentImage::CLAHE` keeps its interface and becomes two `QtConcurrent::blockingMap` phases (one task per contextual region, then one per interpolation block). The Lab round trip moves out of `FormImage::recalculateCLAHEMeteosat1` into a new `SegmentImage::CLAHELab(QImage*, nrX, nrY, clip)` that keeps only the 8-bit L plane and recomputes a/b in its write-back pass, one scanline per task. `recalculateCLAHEMeteosat1` shrinks to the GUI wrapper. A no-GUI probe holding verbatim copies of the old code asserts byte equality.

**Tech Stack:** C++20, Qt 6.9 (`QtConcurrent`, `QImage`), the vendored `ColorSpace` converters in `core/`, CMake + Ninja debug tree at `build/Desktop_Qt_6_9_2-Debug`, probe harness at `/home/hugo/EUMETCastTools/viiprobes/`.

Spec: `docs/superpowers/specs/2026-09-12-parallel-geo-clahe-design.md`.

---

## File structure

| File | Responsibility |
|---|---|
| `core/segmentimage.h` | declare `CLAHELab`; `CLAHE` and the five private helpers are already declared and stay as they are |
| `core/segmentimage.cpp` | parallel `CLAHE` (lines 1373–1466 today); new `CLAHELab` right after it; two new includes |
| `core/formimage.cpp` | `recalculateCLAHEMeteosat1` (lines 3618–3765 today) becomes the GUI wrapper; one new include |
| `/home/hugo/EUMETCastTools/viiprobes/claheprobe.cpp` | new: reference copies of the old serial kernel and the old Lab loops, synthetic images, equality checks, timings |
| `/home/hugo/EUMETCastTools/viiprobes/build.sh` | add `claheprobe` to the loop |
| `/home/hugo/EUMETCastTools/viiprobes/README.md` | one section for `claheprobe` |

The probe directory is **not** a git repository; nothing there is committed. The user asked for the probes to be kept.

## Ground rules for every task

- The application build tree is the Ninja debug one: `cmake --build /home/hugo/EUMETCastTools/EUMETCastView/build/Desktop_Qt_6_9_2-Debug`. It builds both `EUMETCastView` and `EUMETCastVideo` and links into the shared `bin/`.
- **Rebuild the application before rebuilding the probe**, every time. The probe links the application's `.o` files; stale ones link fine and then test code that no longer exists.
- The probe runs from anywhere (`SegmentImage`'s constructor reads no INI), but it needs `QT_QPA_PLATFORM=offscreen` because it is a `QApplication`.
- The "debug" tree compiles the application sources with a trailing `-O2` (an interface compile option from `PublicDecompWT-2.8.1/CMakeLists.txt`; last `-O` wins), and `build.sh` now reads the same `FLAGS` from `build.ninja`, so probe and application code are optimised alike and the probe's ref/new timings compare like with like.

---

### Task 1: Probe with the reference kernel, asserting the kernel it will replace is what it copies

This pins today's behaviour before anything changes. Both sides are the serial kernel now, so the assertion passes trivially; it is the regression guard for Task 2.

**Files:**
- Create: `/home/hugo/EUMETCastTools/viiprobes/claheprobe.cpp`
- Modify: `/home/hugo/EUMETCastTools/viiprobes/build.sh:44`

- [ ] **Step 1: Write the probe**

`/home/hugo/EUMETCastTools/viiprobes/claheprobe.cpp`:

```cpp
// Probe: SegmentImage::CLAHE and SegmentImage::CLAHELab against verbatim
// copies of the serial code they replaced (core/segmentimage.cpp and
// FormImage::recalculateCLAHEMeteosat1 as of commit a9a19e4). Links the
// application's own objects, so the code under test is exactly what the
// geostationary CLAHE button runs.
//
//   QT_QPA_PLATFORM=offscreen ./claheprobe
//
// Exit status 0 when every asserted case is byte-identical.

#include <QApplication>
#include <QImage>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QMutex>

#include "options.h"
#include "segmentimage.h"
#include "gshhsdata.h"
#include "satellite.h"
#include "poi.h"
#include "ColorSpace.h"
#include "Conversion.h"

#include <cstring>
#include <cstdlib>

QMutex g_mutex;
Options opts;
Poi poi;
SegmentImage *imageptrs;
gshhsData *gshhsdata;
QFile loggingFile;
QTextStream outlogging(&loggingFile);
QNetworkAccessManager networkaccessmanager;
SatelliteList satellitelist;
bool ptrimagebusy;

// ---------------------------------------------------------------------------
// Reference: the serial kernel, verbatim from core/segmentimage.cpp before
// it went parallel. Member functions became statics; nothing else changed.
// ---------------------------------------------------------------------------
namespace ref {

#define uiNR_OF_GREY (4096)
const unsigned int uiMAX_REG_X = 16;
const unsigned int uiMAX_REG_Y = 16;

static void ClipHistogram (unsigned long* pulHistogram, unsigned int
             uiNrGreylevels, unsigned long ulClipLimit)
{
    unsigned long* pulBinPointer, *pulEndPointer, *pulHisto;
    unsigned long ulNrExcess, ulUpper, ulBinIncr, ulStepSize, i;
    long lBinExcess;

    ulNrExcess = 0;  pulBinPointer = pulHistogram;
    for (i = 0; i < uiNrGreylevels; i++) {
    lBinExcess = (long) pulBinPointer[i] - (long) ulClipLimit;
    if (lBinExcess > 0) ulNrExcess += lBinExcess;
    };

    ulBinIncr = ulNrExcess / uiNrGreylevels;
    ulUpper =  ulClipLimit - ulBinIncr;

    for (i = 0; i < uiNrGreylevels; i++)
    {
        if (pulHistogram[i] > ulClipLimit) pulHistogram[i] = ulClipLimit;
        else
        {
            if (pulHistogram[i] > ulUpper)
            {
                ulNrExcess -= pulHistogram[i] - ulUpper; pulHistogram[i]=ulClipLimit;
            }
            else
            {
                ulNrExcess -= ulBinIncr; pulHistogram[i] += ulBinIncr;
            }
        }
    }

    while (ulNrExcess)
    {
        pulEndPointer = &pulHistogram[uiNrGreylevels]; pulHisto = pulHistogram;

        while (ulNrExcess && pulHisto < pulEndPointer)
        {
            ulStepSize = uiNrGreylevels / ulNrExcess;
            if (ulStepSize < 1) ulStepSize = 1;
            for (pulBinPointer=pulHisto; pulBinPointer < pulEndPointer && ulNrExcess; pulBinPointer += ulStepSize)
            {
                if (*pulBinPointer < ulClipLimit)
                {
                    (*pulBinPointer)++;	 ulNrExcess--;
                }
            }
            pulHisto++;
        }
    }
}

static void MakeHistogram (unsigned short* pImage, unsigned int uiXRes,
        unsigned int uiSizeX, unsigned int uiSizeY,
        unsigned long* pulHistogram,
        unsigned int uiNrGreylevels, unsigned short* pLookupTable)
{
    unsigned short* pImagePointer;
    unsigned int i;

    for (i = 0; i < uiNrGreylevels; i++) pulHistogram[i] = 0L;

    for (i = 0; i < uiSizeY; i++)
    {
        pImagePointer = &pImage[uiSizeX];
        while (pImage < pImagePointer) pulHistogram[pLookupTable[*pImage++]]++;
        pImagePointer += uiXRes;
        pImage = pImagePointer-uiSizeX;
    }
}

static void MapHistogram (unsigned long* pulHistogram, unsigned short Min, unsigned short Max,
           unsigned int uiNrGreylevels, unsigned long ulNrOfPixels)
{
    unsigned int i;  unsigned long ulSum = 0;
    const float fScale = ((float)(Max - Min)) / ulNrOfPixels;
    const unsigned long ulMin = (unsigned long) Min;

    for (i = 0; i < uiNrGreylevels; i++) {
    ulSum += pulHistogram[i]; pulHistogram[i]=(unsigned long)(ulMin+ulSum*fScale);
    if (pulHistogram[i] > Max) pulHistogram[i] = Max;
    }
}

static void MakeLut (unsigned short * pLUT, unsigned short Min, unsigned short Max, unsigned int uiNrBins)
{
    int i;
    const unsigned short BinSize = (unsigned short) (1 + (Max - Min) / uiNrBins);

    for (i = Min; i <= Max; i++)  pLUT[i] = (i - Min) / BinSize;
}

static void Interpolate (unsigned short *pImage, int uiXRes, unsigned long * pulMapLU,
     unsigned long * pulMapRU, unsigned long * pulMapLB,  unsigned long * pulMapRB,
     unsigned int uiXSize, unsigned int uiYSize, unsigned short *pLUT)
{
    const unsigned int uiIncr = uiXRes-uiXSize;
    unsigned short GreyValue; unsigned int uiNum = uiXSize*uiYSize;

    unsigned int uiXCoef, uiYCoef, uiXInvCoef, uiYInvCoef, uiShift = 0;

    if (uiNum & (uiNum - 1))
        for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize;  uiYCoef++, uiYInvCoef--,pImage+=uiIncr)
        {
            for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--)
            {
                GreyValue = pLUT[*pImage];
                *pImage++ = (unsigned short ) ((uiYInvCoef * (uiXInvCoef*pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue])
                    + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) / uiNum);
            }
        }
    else
    {
        while (uiNum >>= 1) uiShift++;
        for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize; uiYCoef++, uiYInvCoef--,pImage+=uiIncr)
        {
            for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--)
            {
                GreyValue = pLUT[*pImage];
                *pImage++ = (unsigned short)((uiYInvCoef* (uiXInvCoef * pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue])
                    + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) >> uiShift);
            }
        }
    }
}

static int CLAHE (unsigned short* pImage, unsigned int uiXRes, unsigned int uiYRes,
     unsigned short Min, unsigned short Max, unsigned int uiNrX, unsigned int uiNrY,
          unsigned int uiNrBins, float fCliplimit)
{
    unsigned int uiX, uiY;
    unsigned int uiXSize, uiYSize, uiSubX, uiSubY;
    unsigned int uiXL, uiXR, uiYU, uiYB;
    unsigned long ulClipLimit, ulNrPixels;
    unsigned short* pImPointer;
    unsigned short aLUT[uiNR_OF_GREY];
    unsigned long* pulHist, *pulMapArray;
    unsigned long* pulLU, *pulLB, *pulRU, *pulRB;

    if (uiNrX > uiMAX_REG_X) return -1;
    if (uiNrY > uiMAX_REG_Y) return -2;
    if (uiXRes % uiNrX) return -3;
    if (uiYRes % uiNrY) return -4;
    if (Max >= uiNR_OF_GREY) return -5;
    if (Min >= Max) return -6;
    if (uiNrX < 2 || uiNrY < 2) return -7;
    if (fCliplimit == 1.0) return 0;
    if (uiNrBins == 0) uiNrBins = 128;

    pulMapArray=(unsigned long *)malloc(sizeof(unsigned long)*uiNrX*uiNrY*uiNrBins);
    if (pulMapArray == 0) return -8;

    uiXSize = uiXRes/uiNrX; uiYSize = uiYRes/uiNrY;
    ulNrPixels = (unsigned long)uiXSize * (unsigned long)uiYSize;

    if(fCliplimit > 0.0) {
       ulClipLimit = (unsigned long) (fCliplimit * (uiXSize * uiYSize) / uiNrBins);
       ulClipLimit = (ulClipLimit < 1UL) ? 1UL : ulClipLimit;
    }
    else ulClipLimit = 1UL<<14;
    MakeLut(aLUT, Min, Max, uiNrBins);
    for (uiY = 0, pImPointer = pImage; uiY < uiNrY; uiY++)
    {
        for (uiX = 0; uiX < uiNrX; uiX++, pImPointer += uiXSize)
        {
            pulHist = &pulMapArray[uiNrBins * (uiY * uiNrX + uiX)];
            MakeHistogram(pImPointer,uiXRes,uiXSize,uiYSize,pulHist,uiNrBins,aLUT);
            ClipHistogram(pulHist, uiNrBins, ulClipLimit);
            MapHistogram(pulHist, Min, Max, uiNrBins, ulNrPixels);
        }
        pImPointer += (uiYSize - 1) * uiXRes;
    }

    for (pImPointer = pImage, uiY = 0; uiY <= uiNrY; uiY++)
    {
        if (uiY == 0)
        {
            uiSubY = uiYSize >> 1;  uiYU = 0; uiYB = 0;
        }
        else
        {
            if (uiY == uiNrY)
            {
                uiSubY = uiYSize >> 1;	uiYU = uiNrY-1;	 uiYB = uiYU;
            }
            else
                {
                    uiSubY = uiYSize; uiYU = uiY - 1; uiYB = uiYU + 1;
                }
        }

        for (uiX = 0; uiX <= uiNrX; uiX++)
        {
            if (uiX == 0)
            {
                uiSubX = uiXSize >> 1; uiXL = 0; uiXR = 0;
            }
            else
                {
                    if (uiX == uiNrX)
                    {
                        uiSubX = uiXSize >> 1;  uiXL = uiNrX - 1; uiXR = uiXL;
                    }
                    else
                        {
                            uiSubX = uiXSize; uiXL = uiX - 1; uiXR = uiXL + 1;
                        }
                }

            pulLU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXL)];
            pulRU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXR)];
            pulLB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXL)];
            pulRB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXR)];
            Interpolate(pImPointer,uiXRes,pulLU,pulRU,pulLB,pulRB,uiSubX,uiSubY,aLUT);
            pImPointer += uiSubX;
        }
        pImPointer += (uiSubY - 1) * uiXRes;
    }

    free(pulMapArray);
    return 0;
}

} // namespace ref

// ---------------------------------------------------------------------------
// Synthetic inputs. Deterministic seeds, so a failure reproduces.
// ---------------------------------------------------------------------------

// A diagonal ramp over [0, maxval] with +-8% noise: every histogram bin gets
// populated, every region's mapping differs from its neighbours'.
static unsigned short *syntheticGrey(int w, int h, int maxval)
{
    unsigned short *p = new unsigned short[(size_t)w * h];
    QRandomGenerator rng(4242);
    const int noise = maxval / 12;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            int v = (int)(((long long)(x + y) * maxval) / (w + h - 2));
            v += (int)rng.bounded(2 * noise + 1) - noise;
            p[(size_t)y * w + x] = (unsigned short)qBound(0, v, maxval);
        }
    return p;
}

struct KernelCase {
    const char *name;
    int w, h, maxval, nrx, nry;
    float clip;
    bool assertEqual;   // false: an odd region size, where the serial kernel drifts
};

// Runs the reference and the application kernel on identical copies of one
// synthetic image and compares every pixel. Returns true when the case holds
// (equal when asserted; always for a reported-only case).
static bool kernelCase(const KernelCase &c)
{
    const size_t npix = (size_t)c.w * c.h;
    unsigned short *a = syntheticGrey(c.w, c.h, c.maxval);
    unsigned short *b = new unsigned short[npix];
    memcpy(b, a, npix * sizeof(unsigned short));

    QElapsedTimer t;
    t.start();
    int rr = ref::CLAHE(a, c.w, c.h, 0, c.maxval, c.nrx, c.nry, 256, c.clip);
    qint64 tref = t.elapsed();
    t.restart();
    int rn = imageptrs->CLAHE(b, c.w, c.h, 0, c.maxval, c.nrx, c.nry, 256, c.clip);
    qint64 tnew = t.elapsed();

    size_t diff = 0;
    int firstx = -1, firsty = -1, lastx = -1, lasty = -1;
    for (int y = 0; y < c.h; y++)
        for (int x = 0; x < c.w; x++)
            if (a[(size_t)y * c.w + x] != b[(size_t)y * c.w + x])
            {
                if (diff == 0) { firstx = x; firsty = y; }
                lastx = x; lasty = y;
                diff++;
            }

    const bool ok = (rr == rn) && (!c.assertEqual || diff == 0);
    qInfo().noquote() << QString("kernel %1: %2x%3 max %4 regions %5x%6 clip %7  ret %8/%9  ref %10 ms  new %11 ms  differing %12%13")
                         .arg(c.name).arg(c.w).arg(c.h).arg(c.maxval).arg(c.nrx).arg(c.nry).arg(c.clip)
                         .arg(rr).arg(rn).arg(tref).arg(tnew).arg(diff)
                         .arg(diff ? QString("  first (%1,%2) last (%3,%4)").arg(firstx).arg(firsty).arg(lastx).arg(lasty) : QString())
                      << (c.assertEqual ? (ok ? "  OK" : "  FAIL") : "  (reported only)");

    delete [] a;
    delete [] b;
    return ok;
}

int main(int argc, char *argv[])
{
    ptrimagebusy = false;
    QApplication app(argc, argv);

    imageptrs = new SegmentImage();

    bool ok = true;

    const KernelCase kernelCases[] = {
        { "fdhsi 8-bit",       5568, 5568,  255, 16, 16, 3.0f, true  },
        { "fdhsi 10-bit",      5568, 5568, 1023, 16, 16, 3.0f, true  },
        { "fdhsi olci clip",   5568, 5568,  255, 16, 16, 6.9f, true  },
        { "fdhsi clip 1.0",    5568, 5568,  255, 16, 16, 1.0f, true  },
        { "himawari 10x10",    5500, 5500,  255, 10, 10, 3.0f, true  },
        { "goes odd 339",      5424, 5424, 1023, 16, 16, 3.0f, false },
    };
    for (const KernelCase &c : kernelCases)
        ok = kernelCase(c) && ok;

    qInfo() << (ok ? "ALL ASSERTED CASES OK" : "SOME CASE FAILED");
    return ok ? 0 : 1;
}
```

- [ ] **Step 2: Add the probe to `build.sh`**

In `/home/hugo/EUMETCastTools/viiprobes/build.sh`, change line 44

```bash
for probe in viitex_probe viitex_probe2; do
```

to

```bash
for probe in viitex_probe viitex_probe2 claheprobe; do
```

and the final echo (line 53) to

```bash
echo "done: $HERE/viitex_probe $HERE/viitex_probe2 $HERE/claheprobe"
```

- [ ] **Step 3: Build the application (so the objects are current), then the probe**

Run:
```bash
cmake --build /home/hugo/EUMETCastTools/EUMETCastView/build/Desktop_Qt_6_9_2-Debug 2>&1 | tail -3
/home/hugo/EUMETCastTools/viiprobes/build.sh 2>&1 | tail -3
```
Expected: `ninja: no work to do.` (or a short rebuild), then `done: .../viitex_probe .../viitex_probe2 .../claheprobe`.

- [ ] **Step 4: Run it — the baseline must be identical to itself**

Run:
```bash
QT_QPA_PLATFORM=offscreen /home/hugo/EUMETCastTools/viiprobes/claheprobe 2>&1 | grep -v "^Calculate\|^Interpolate"
```
Expected: six `kernel …` lines; the five asserted ones end in `OK` with `differing 0`; the `goes odd 339` line reports `differing 0` too (both sides are still the same serial code); last line `ALL ASSERTED CASES OK`; exit status 0. Note the `ref` and `new` timings — they should be about equal.

No commit: the probe directory is not versioned.

---

### Task 2: Parallel `SegmentImage::CLAHE`

**Files:**
- Modify: `core/segmentimage.cpp:1-4` (includes) and `core/segmentimage.cpp:1373-1466` (`CLAHE`)

- [ ] **Step 1: Add the includes**

At the top of `core/segmentimage.cpp`, change

```cpp
#include "segmentimage.h"
#include "viil1breader.h"

#include <QDebug>
```

to

```cpp
#include "segmentimage.h"
#include "viil1breader.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <numeric>
```

- [ ] **Step 2: Replace the body of `CLAHE`**

Replace everything from the line `unsigned int uiX, uiY;		  /* counters */` (the first line inside the function body, after the doc comment) up to and including the `return 0;						  /* return status OK */` and its closing `}` with:

```cpp
{
    unsigned int uiXSize, uiYSize;	  /* size of contextual regions */
    unsigned long ulClipLimit, ulNrPixels;/* clip limit and region pixel count */
    unsigned short aLUT[uiNR_OF_GREY];	    /* lookup table used for scaling of input image */
    unsigned long* pulMapArray;		   /* pointer to mappings */

    if (uiNrX > uiMAX_REG_X) return -1;	   /* # of regions x-direction too large */
    if (uiNrY > uiMAX_REG_Y) return -2;	   /* # of regions y-direction too large */
    if (uiXRes % uiNrX) return -3;	  /* x-resolution no multiple of uiNrX */
    if (uiYRes % uiNrY) return -4;	  /* y-resolution no multiple of uiNrY */
    if (Max >= uiNR_OF_GREY) return -5;	   /* maximum too large */
    if (Min >= Max) return -6;		  /* minimum equal or larger than maximum */
    if (uiNrX < 2 || uiNrY < 2) return -7;/* at least 4 contextual regions required */
    if (fCliplimit == 1.0) return 0;	  /* is OK, immediately returns original image. */
    if (uiNrBins == 0) uiNrBins = 128;	  /* default value when not specified */

    pulMapArray=(unsigned long *)malloc(sizeof(unsigned long)*uiNrX*uiNrY*uiNrBins);
    if (pulMapArray == 0) return -8;	  /* Not enough memory! (try reducing uiNrBins) */

    uiXSize = uiXRes/uiNrX; uiYSize = uiYRes/uiNrY;  /* Actual size of contextual regions */
    ulNrPixels = (unsigned long)uiXSize * (unsigned long)uiYSize;

    if(fCliplimit > 0.0) {		  /* Calculate actual cliplimit	 */
       ulClipLimit = (unsigned long) (fCliplimit * (uiXSize * uiYSize) / uiNrBins);
       ulClipLimit = (ulClipLimit < 1UL) ? 1UL : ulClipLimit;
    }
    else ulClipLimit = 1UL<<14;		  /* Large value, do not clip (AHE) */
    MakeLut(aLUT, Min, Max, uiNrBins);	  /* Make lookup table for mapping of greyvalues */

    qDebug() << "Calculate greylevel mappings for each contextual region";
    /* One task per contextual region. Each reads only its own tile and writes
       only its own uiNrBins slot of pulMapArray, so they are independent. */
    QVector<int> regions(uiNrX * uiNrY);
    std::iota(regions.begin(), regions.end(), 0);
    QtConcurrent::blockingMap(regions, [&](int region)
    {
        const unsigned int uiX = region % uiNrX;
        const unsigned int uiY = region / uiNrX;
        unsigned short *pImPointer = pImage + (size_t)uiY * uiYSize * uiXRes + (size_t)uiX * uiXSize;
        unsigned long *pulHist = &pulMapArray[uiNrBins * region];
        MakeHistogram(pImPointer,uiXRes,uiXSize,uiYSize,pulHist,uiNrBins,aLUT);
        ClipHistogram(pulHist, uiNrBins, ulClipLimit);
        MapHistogram(pulHist, Min, Max, uiNrBins, ulNrPixels);
    });

    qDebug() << "Interpolate greylevel mappings to get CLAHE image";
    /* One task per block of the (uiNrX+1) x (uiNrY+1) interpolation grid. The
       border blocks are half a region wide, so a block's origin follows from
       its index; the serial code walked a running pointer instead, which for
       an odd region size came up one pixel short per block row. Every block
       writes only its own pixels and the mappings are read-only by now. */
    QVector<int> blocks((uiNrX + 1) * (uiNrY + 1));
    std::iota(blocks.begin(), blocks.end(), 0);
    QtConcurrent::blockingMap(blocks, [&](int block)
    {
        const unsigned int uiX = block % (uiNrX + 1);
        const unsigned int uiY = block / (uiNrX + 1);
        unsigned int uiSubX, uiSubY;	  /* size of the block */
        unsigned int uiXL, uiXR, uiYU, uiYB;  /* the four regions it interpolates between */
        unsigned int uiX0, uiY0;	  /* its origin in the image */

        if (uiY == 0)       /* special case: top row */
        {
            uiSubY = uiYSize >> 1;  uiYU = 0; uiYB = 0; uiY0 = 0;
        }
        else if (uiY == uiNrY)				  /* special case: bottom row */
        {
            uiSubY = uiYSize >> 1;	uiYU = uiNrY-1;	 uiYB = uiYU; uiY0 = (uiYSize >> 1) + (uiY - 1) * uiYSize;
        }
        else						  /* default values */
        {
            uiSubY = uiYSize; uiYU = uiY - 1; uiYB = uiYU + 1; uiY0 = (uiYSize >> 1) + (uiY - 1) * uiYSize;
        }

        if (uiX == 0)				  /* special case: left column */
        {
            uiSubX = uiXSize >> 1; uiXL = 0; uiXR = 0; uiX0 = 0;
        }
        else if (uiX == uiNrX)			  /* special case: right column */
        {
            uiSubX = uiXSize >> 1;  uiXL = uiNrX - 1; uiXR = uiXL; uiX0 = (uiXSize >> 1) + (uiX - 1) * uiXSize;
        }
        else					  /* default values */
        {
            uiSubX = uiXSize; uiXL = uiX - 1; uiXR = uiXL + 1; uiX0 = (uiXSize >> 1) + (uiX - 1) * uiXSize;
        }

        unsigned long *pulLU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXL)];
        unsigned long *pulRU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXR)];
        unsigned long *pulLB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXL)];
        unsigned long *pulRB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXR)];
        Interpolate(pImage + (size_t)uiY0 * uiXRes + uiX0, uiXRes, pulLU, pulRU, pulLB, pulRB, uiSubX, uiSubY, aLUT);
    });

    free(pulMapArray);					  /* free space for histograms */
    return 0;						  /* return status OK */
}
```

The doc comment block above the body (`/*   pImage - Pointer to the input/output image … */`) stays exactly as it is.

- [ ] **Step 3: Build the application, then the probe**

Run:
```bash
cmake --build /home/hugo/EUMETCastTools/EUMETCastView/build/Desktop_Qt_6_9_2-Debug 2>&1 | grep -i "error\|warning: unused\|Linking" | head
/home/hugo/EUMETCastTools/viiprobes/build.sh 2>&1 | tail -1
```
Expected: no `error` lines; `done: …`.

- [ ] **Step 4: Run the probe**

Run:
```bash
QT_QPA_PLATFORM=offscreen /home/hugo/EUMETCastTools/viiprobes/claheprobe 2>&1 | grep -v "^Calculate\|^Interpolate"
```
Expected:
- the five asserted cases end in `OK` with `differing 0`, and `new` is well under `ref` (on the 32-core machine phase timing should drop by roughly an order of magnitude; at `-O0` the absolute numbers are not meaningful);
- `goes odd 339` reports a **non-zero** `differing` count, with `first (…, y)` where `y` is at the first block-row boundary (`339/2 = 169`) or below, and `last` near the bottom-right — that is the serial drift the spec describes, and it is expected;
- `ALL ASSERTED CASES OK`, exit status 0.

If an asserted case differs, the block-origin arithmetic is wrong; compare `uiX0`/`uiY0` against the running pointer sums in `ref::CLAHE` before touching anything else.

- [ ] **Step 5: Commit**

```bash
cd /home/hugo/EUMETCastTools/EUMETCastView
git add core/segmentimage.cpp
git commit -m "$(cat <<'EOF'
Run the CLAHE kernel's regions and blocks on every core

Both phases of SegmentImage::CLAHE are independent per contextual region
and per interpolation block, so each becomes a QtConcurrent::blockingMap.
A block's origin is now computed from its index; the running pointer the
serial code walked came up one pixel short per block row whenever the
region size was odd, which skewed GOES, MSG RSS and OLCI by up to sixteen
pixels at the bottom of the image.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01GqDgDRLPZLRcQJDETQ1pQz
EOF
)"
```

---

### Task 3: Probe: the reference Lab pipeline and the failing `CLAHELab` case

**Files:**
- Modify: `/home/hugo/EUMETCastTools/viiprobes/claheprobe.cpp`

- [ ] **Step 1: Add the reference pipeline after `namespace ref { … }`**

Insert directly after the line `} // namespace ref` (and before the `// Synthetic inputs` banner):

```cpp
// ---------------------------------------------------------------------------
// Reference: the pixel loops of FormImage::recalculateCLAHEMeteosat1 before
// they moved into SegmentImage::CLAHELab, verbatim except that the image is
// a parameter and the kernel is the reference one above.
// ---------------------------------------------------------------------------
namespace ref {

static void CLAHELab(QImage *image, unsigned int nrx, unsigned int nry, float cliplimit)
{
    QRgb *row_col;
    QRgb c;

    double *L;
    double *a;
    double *b;

    double Lu;
    double a_lab;
    double b_lab;
    size_t npix;

    ushort *pixelsL;

    int width = image->width();
    int height = image->height();
    npix = width*height;

    L = new double[width*height];
    a = new double[width*height];
    b = new double[width*height];

    ColorSpace::Rgb srcColor;
    ColorSpace::Lab dstColor;

    for (int line = height - 1; line >= 0; line--)
    {
        row_col = (QRgb*)image->scanLine(line);
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            c = row_col[pixelx];
            srcColor.r = qRed(c);
            srcColor.g = qGreen(c);
            srcColor.b = qBlue(c);

            srcColor.To<ColorSpace::Lab>(&dstColor);

            dstColor.l = (dstColor.l < 0.0 ? 0.0 : dstColor.l);
            dstColor.l = (dstColor.l > 100.0 ? 100.0 : dstColor.l);
            dstColor.a = (dstColor.a < -128.0 ? -128.0 : dstColor.a);
            dstColor.a = (dstColor.a > 128.0 ? 128.0 : dstColor.a);
            dstColor.b = (dstColor.b < -128.0 ? -128.0 : dstColor.b);
            dstColor.b = (dstColor.b > 128.0 ? 128.0 : dstColor.b);
            L[line * width + pixelx] = dstColor.l;
            a[line * width + pixelx] = dstColor.a;
            b[line * width + pixelx] = dstColor.b;
        }
    }

    pixelsL = new ushort[npix];

    for (int line = height - 1; line >= 0; line--)
    {
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            pixelsL[line * width + pixelx] = (ushort)qRound(L[line * width + pixelx] * 255.0 / 100.0);
            pixelsL[line * width + pixelx] = (pixelsL[line * width + pixelx] > 255 ? 255 : pixelsL[line * width + pixelx]);
        }
    }

    ref::CLAHE(pixelsL, width, height, 0, 255, nrx, nry, 256, cliplimit);

    for (int line = height - 1; line >= 0; line--)
    {
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            L[line * width + pixelx] = (double)(pixelsL[line * width + pixelx] * 100.0 / 255.0);
            L[line * width + pixelx] = (L[line * width + pixelx] > 100.0 ? 100.0 : L[line * width + pixelx]);
            L[line * width + pixelx] = (L[line * width + pixelx] < 0.0 ? 0.0 : L[line * width + pixelx]);
        }
    }

    ColorSpace::Lab srcColor1;
    ColorSpace::Rgb dstColor1;

    for (int line = height - 1; line >= 0; line--)
    {
        row_col = (QRgb*)image->scanLine(line);
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            Lu = L[line * width + pixelx];
            a_lab = a[line * width + pixelx];
            b_lab = b[line * width + pixelx];
            srcColor1.l = Lu;
            srcColor1.a = a_lab;
            srcColor1.b = b_lab;

            srcColor1.To<ColorSpace::Rgb>(&dstColor1);

            dstColor1.r = (dstColor1.r > 255.0 ? 255.0 : dstColor1.r);
            dstColor1.g = (dstColor1.g > 255.0 ? 255.0 : dstColor1.g);
            dstColor1.b = (dstColor1.b > 255.0 ? 255.0 : dstColor1.b);

            dstColor1.r = (dstColor1.r < 0.0 ? 0.0 : dstColor1.r);
            dstColor1.g = (dstColor1.g < 0.0 ? 0.0 : dstColor1.g);
            dstColor1.b = (dstColor1.b < 0.0 ? 0.0 : dstColor1.b);

            row_col[pixelx] = qRgb((int)dstColor1.r, (int)dstColor1.g, (int)dstColor1.b);
        }
    }

    delete [] pixelsL;
    delete [] L;
    delete [] a;
    delete [] b;
}

} // namespace ref
```

- [ ] **Step 2: Add the synthetic colour image and the pipeline case, after `kernelCase`**

Insert directly after the closing `}` of `kernelCase` (before `int main`):

```cpp
// A disc of smooth colour ramps with +-20 noise per channel, and the alpha-0
// black outside the disc that a space-filled geostationary image has. Both
// pipelines write qRgb(), i.e. alpha 255, everywhere; the case checks that
// the new one does exactly what the old one did there too.
static QImage syntheticRgb(int size)
{
    QImage img(size, size, QImage::Format_ARGB32);
    QRandomGenerator rng(12345);
    const double c = size / 2.0;
    const double r2 = c * c * 0.96;
    for (int y = 0; y < size; y++)
    {
        QRgb *row = (QRgb *)img.scanLine(y);
        for (int x = 0; x < size; x++)
        {
            if ((x - c) * (x - c) + (y - c) * (y - c) > r2)
            {
                row[x] = qRgba(0, 0, 0, 0);
                continue;
            }
            int r = (x * 255) / (size - 1);
            int g = (y * 255) / (size - 1);
            int b = ((x + y) * 255) / (2 * (size - 1));
            r = qBound(0, r + (int)rng.bounded(41) - 20, 255);
            g = qBound(0, g + (int)rng.bounded(41) - 20, 255);
            b = qBound(0, b + (int)rng.bounded(41) - 20, 255);
            row[x] = qRgb(r, g, b);
        }
    }
    return img;
}

// Runs the reference loops and SegmentImage::CLAHELab on identical copies of
// one synthetic colour image and compares every byte of every scanline.
static bool pipelineCase(const char *name, int size, unsigned int regions, float clip)
{
    QImage a = syntheticRgb(size);
    QImage b = a.copy();

    QElapsedTimer t;
    t.start();
    ref::CLAHELab(&a, regions, regions, clip);
    qint64 tref = t.elapsed();
    t.restart();
    int rn = imageptrs->CLAHELab(&b, regions, regions, clip);
    qint64 tnew = t.elapsed();

    long differingLines = 0;
    int firstLine = -1;
    for (int y = 0; y < size; y++)
        if (memcmp(a.constScanLine(y), b.constScanLine(y), a.bytesPerLine()) != 0)
        {
            if (firstLine < 0) firstLine = y;
            differingLines++;
        }

    const bool ok = (rn == 0) && differingLines == 0;
    qInfo().noquote() << QString("pipeline %1: %2x%3 regions %4x%4 clip %5  ret %6  ref %7 ms  new %8 ms  differing lines %9%10")
                         .arg(name).arg(size).arg(size).arg(regions).arg(clip)
                         .arg(rn).arg(tref).arg(tnew).arg(differingLines)
                         .arg(firstLine >= 0 ? QString("  first line %1").arg(firstLine) : QString())
                      << (ok ? "  OK" : "  FAIL");
    return ok;
}
```

- [ ] **Step 3: Call it from `main`**

In `main`, change

```cpp
    for (const KernelCase &c : kernelCases)
        ok = kernelCase(c) && ok;

    qInfo() << (ok ? "ALL ASSERTED CASES OK" : "SOME CASE FAILED");
```

to

```cpp
    for (const KernelCase &c : kernelCases)
        ok = kernelCase(c) && ok;

    ok = pipelineCase("fdhsi", 5568, 16, 3.0f) && ok;
    ok = pipelineCase("fdhsi clip 1.0", 5568, 16, 1.0f) && ok;
    ok = pipelineCase("himawari", 5500, 10, 3.0f) && ok;

    qInfo() << (ok ? "ALL ASSERTED CASES OK" : "SOME CASE FAILED");
```

- [ ] **Step 4: Build the probe and verify it fails to compile**

Run:
```bash
/home/hugo/EUMETCastTools/viiprobes/build.sh 2>&1 | grep -m1 "error"
```
Expected: a compile error saying `class SegmentImage` has no member named `CLAHELab`.

---

### Task 4: `SegmentImage::CLAHELab`

**Files:**
- Modify: `core/segmentimage.h:185-187` (after the `CLAHE` declaration)
- Modify: `core/segmentimage.cpp:1-6` (includes) and after the end of `CLAHE` (before `void  SegmentImage::ClipHistogram`)

- [ ] **Step 1: Declare it**

In `core/segmentimage.h`, change

```cpp
    int CLAHE (unsigned short *pImage, unsigned int uiXRes, unsigned int uiYRes,
         unsigned short Min, unsigned short Max, unsigned int uiNrX, unsigned int uiNrY,
              unsigned int uiNrBins, float fCliplimit);
```

to

```cpp
    int CLAHE (unsigned short *pImage, unsigned int uiXRes, unsigned int uiYRes,
         unsigned short Min, unsigned short Max, unsigned int uiNrX, unsigned int uiNrY,
              unsigned int uiNrBins, float fCliplimit);
    int CLAHELab (QImage *image, unsigned int uiNrX, unsigned int uiNrY, float fCliplimit);
```

- [ ] **Step 2: Add the ColorSpace includes**

In `core/segmentimage.cpp`, change

```cpp
#include "segmentimage.h"
#include "viil1breader.h"
```

to

```cpp
#include "segmentimage.h"
#include "viil1breader.h"
#include "ColorSpace.h"
#include "Conversion.h"
```

- [ ] **Step 3: Implement it**

In `core/segmentimage.cpp`, insert directly after the closing `}` of `CLAHE` (before `void  SegmentImage::ClipHistogram`):

```cpp
// CLAHE on the lightness of an RGB image: every pixel goes to CIE Lab, the
// 8-bit L plane is equalised with CLAHE, and the pixel is rebuilt from the new
// L and its own a and b. Only the L plane is kept between the two passes;
// a and b are recomputed from the untouched pixel on the way back, which is
// one extra conversion per pixel in exchange for 2 instead of 26 bytes of
// temporaries per pixel. Both passes run one scanline per task on the
// kernel's own pool (see clahePool).
// Returns what CLAHE returns; on an error the image is left as it was.
int SegmentImage::CLAHELab (QImage *image, unsigned int uiNrX, unsigned int uiNrY, float fCliplimit)
{
    const int width = image->width();
    const int height = image->height();
    const size_t npix = (size_t)width * height;

    // One detach here, on the calling thread; the workers index from the
    // base pointer. scanLine() from several threads races on the detach
    // bookkeeping.
    uchar *base = image->bits();
    const qsizetype bpl = image->bytesPerLine();

    qDebug() << Q_FUNC_INFO << "image width = " << width << " height = " << height << " npix = " << npix;

    ushort *pixelsL = new ushort[npix];

    QVector<int> lines(height);
    std::iota(lines.begin(), lines.end(), 0);

    QtConcurrent::blockingMap(clahePool(), lines, [&](int line)
    {
        const QRgb *row_col = (const QRgb *)(base + line * bpl);
        ushort *rowL = pixelsL + (size_t)line * width;
        ColorSpace::Rgb srcColor;
        ColorSpace::Lab dstColor;
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            const QRgb c = row_col[pixelx];
            srcColor.r = qRed(c);
            srcColor.g = qGreen(c);
            srcColor.b = qBlue(c);

            srcColor.To<ColorSpace::Lab>(&dstColor);

            double l = dstColor.l;
            l = (l < 0.0 ? 0.0 : l);
            l = (l > 100.0 ? 100.0 : l);
            ushort L = (ushort)qRound(l * 255.0 / 100.0);
            rowL[pixelx] = (L > 255 ? 255 : L);
        }
    });

    int ret = CLAHE(pixelsL, width, height, 0, 255, uiNrX, uiNrY, 256, fCliplimit);
    if (ret != 0)
    {
        qDebug() << Q_FUNC_INFO << "CLAHE returned" << ret << "; image left unchanged";
        delete [] pixelsL;
        return ret;
    }

    QtConcurrent::blockingMap(clahePool(), lines, [&](int line)
    {
        QRgb *row_col = (QRgb *)(base + line * bpl);
        const ushort *rowL = pixelsL + (size_t)line * width;
        ColorSpace::Rgb srcColor;
        ColorSpace::Lab lab;
        ColorSpace::Rgb dstColor;
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            const QRgb c = row_col[pixelx];
            srcColor.r = qRed(c);
            srcColor.g = qGreen(c);
            srcColor.b = qBlue(c);

            srcColor.To<ColorSpace::Lab>(&lab);

            lab.a = (lab.a < -128.0 ? -128.0 : lab.a);
            lab.a = (lab.a > 128.0 ? 128.0 : lab.a);
            lab.b = (lab.b < -128.0 ? -128.0 : lab.b);
            lab.b = (lab.b > 128.0 ? 128.0 : lab.b);

            double l = (double)(rowL[pixelx] * 100.0 / 255.0);
            l = (l > 100.0 ? 100.0 : l);
            l = (l < 0.0 ? 0.0 : l);
            lab.l = l;

            lab.To<ColorSpace::Rgb>(&dstColor);

            dstColor.r = (dstColor.r > 255.0 ? 255.0 : dstColor.r);
            dstColor.g = (dstColor.g > 255.0 ? 255.0 : dstColor.g);
            dstColor.b = (dstColor.b > 255.0 ? 255.0 : dstColor.b);

            dstColor.r = (dstColor.r < 0.0 ? 0.0 : dstColor.r);
            dstColor.g = (dstColor.g < 0.0 ? 0.0 : dstColor.g);
            dstColor.b = (dstColor.b < 0.0 ? 0.0 : dstColor.b);

            row_col[pixelx] = qRgb((int)dstColor.r, (int)dstColor.g, (int)dstColor.b);
        }
    });

    delete [] pixelsL;
    return 0;
}
```

- [ ] **Step 4: Build the application, then the probe**

Run:
```bash
cmake --build /home/hugo/EUMETCastTools/EUMETCastView/build/Desktop_Qt_6_9_2-Debug 2>&1 | grep -i "error" | head
/home/hugo/EUMETCastTools/viiprobes/build.sh 2>&1 | tail -1
```
Expected: no `error` lines; `done: …`.

- [ ] **Step 5: Run the probe**

Run:
```bash
QT_QPA_PLATFORM=offscreen /home/hugo/EUMETCastTools/viiprobes/claheprobe 2>&1 | grep -v "^Calculate\|^Interpolate\|CLAHELab"
```
Expected: the kernel lines as in Task 2, then three `pipeline …` lines ending in `OK` with `differing lines 0` and `ret 0`, `new` far below `ref`; `ALL ASSERTED CASES OK`; exit status 0.

If a pipeline case differs on **every** line, suspect the clamps or the order of `qRound`/clamp in pass 1 against the reference. If it differs on a **few** lines only, suspect the scanline addressing (`base + line * bpl`).

- [ ] **Step 6: Commit**

```bash
cd /home/hugo/EUMETCastTools/EUMETCastView
git add core/segmentimage.h core/segmentimage.cpp
git commit -m "$(cat <<'EOF'
Add SegmentImage::CLAHELab, the Lab round trip of the geostationary CLAHE

The RGB -> L -> CLAHE -> RGB work that recalculateCLAHEMeteosat1 does
inline moves next to the kernel, one scanline per task, keeping only the
8-bit L plane between the passes: a and b are recomputed from the
untouched pixel on the way back. Output is byte-identical; temporaries
drop from 26 to 2 bytes per pixel, which is what makes the 0.5 km HRFI
composite feasible.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01GqDgDRLPZLRcQJDETQ1pQz
EOF
)"
```

---

### Task 5: `FormImage::recalculateCLAHEMeteosat1` becomes the GUI wrapper

**Files:**
- Modify: `core/formimage.cpp:9-15` (includes) and `core/formimage.cpp:3618-3765` (`recalculateCLAHEMeteosat1`)

- [ ] **Step 1: Add the timer include**

In `core/formimage.cpp`, change

```cpp
#include <QDebug>
#include <QMessageBox>
```

to

```cpp
#include <QDebug>
#include <QElapsedTimer>
#include <QMessageBox>
```

- [ ] **Step 2: Replace the function**

Replace the whole of `FormImage::recalculateCLAHEMeteosat1` — from `void FormImage::recalculateCLAHEMeteosat1()` through its closing `}` just before `void FormImage::recalculateCLAHEAVHRR()` — with:

```cpp
void FormImage::recalculateCLAHEMeteosat1()
{
    SegmentListGeostationary *sl;

    sl = segs->getActiveSegmentList();

    QApplication::setOverrideCursor( Qt::WaitCursor ); // this might take time

    formtoolbox->setProgressValue(10);

    // Himawari-9's 5500 px disc is not a multiple of 16; the kernel wants
    // whole regions.
    const unsigned int regions = (sl->getGeoSatellite() == eGeoSatellite::H9 ? 10 : 16);

    QElapsedTimer timer;
    timer.start();
    int ret = imageptrs->CLAHELab(imageptrs->ptrimageGeostationary, regions, regions, opts.clahecliplimit);
    qDebug() << Q_FUNC_INFO << "CLAHELab returned" << ret << "in" << timer.elapsed() << "ms";

    formtoolbox->setProgressValue(100);

    if(sl->getKindofImage() != "HRV" && sl->getKindofImage() != "HRV Color")
        if(opts.imageontextureOnMet)
            emit render3dgeo(sl->getGeoSatelliteIndex());

    QApplication::restoreOverrideCursor();
}
```

- [ ] **Step 3: Leave the `ColorSpace.h` / `Conversion.h` includes in `formimage.cpp` alone**

`recalculateCLAHEAVHRR` (`core/formimage.cpp:3861` and on) still does its own Lab round trip with them. Confirm with:
```bash
cd /home/hugo/EUMETCastTools/EUMETCastView && grep -c "ColorSpace::" core/formimage.cpp
```
Expected: a count greater than 0.

- [ ] **Step 4: Build both targets**

Run:
```bash
cmake --build /home/hugo/EUMETCastTools/EUMETCastView/build/Desktop_Qt_6_9_2-Debug 2>&1 | grep -i "error\|Linking CXX executable" 
ls -la /home/hugo/EUMETCastTools/EUMETCastView/bin/EUMETCastView /home/hugo/EUMETCastTools/EUMETCastView/bin/EUMETCastVideo
```
Expected: no `error` lines; both executables carry today's timestamp.

- [ ] **Step 5: Commit**

```bash
cd /home/hugo/EUMETCastTools/EUMETCastView
git add core/formimage.cpp
git commit -m "$(cat <<'EOF'
Let the geostationary CLAHE button call SegmentImage::CLAHELab

recalculateCLAHEMeteosat1 is now the GUI wrapper it was around the pixel
work: cursor, progress, the 10x10 choice for Himawari-9, the globe
texture. The intermediate progress values go; the GUI thread is inside
the work and never painted them.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01GqDgDRLPZLRcQJDETQ1pQz
EOF
)"
```

---

### Task 6: In-application check and the probe README

**Files:**
- Modify: `/home/hugo/EUMETCastTools/viiprobes/README.md` (append a section)

- [ ] **Step 1: Document the probe**

Append to `/home/hugo/EUMETCastTools/viiprobes/README.md`:

```markdown
### `claheprobe` - the geostationary CLAHE against the code it replaced

    QT_QPA_PLATFORM=offscreen ./claheprobe

Runs `SegmentImage::CLAHE` and `SegmentImage::CLAHELab` (the parallel kernel
and the Lab round trip behind the geostationary CLAHE button, 2026-09-12)
against verbatim copies of the serial code they replaced, on synthetic
images, and asserts byte equality. Prints old-vs-new timings; the build is
`-O0`, so read them as ratios.

Unlike the VII probes it needs no INI, no data and no particular working
directory: `SegmentImage`'s constructor reads nothing.

One case, `goes odd 339` (5424 px at 16 regions), is reported but not
asserted: the serial kernel's running pointer came up one pixel short per
block row whenever the region size was odd, so on GOES, MSG RSS and OLCI
images the old and new results differ by design. Exit status 0 means every
asserted case held.
```

- [ ] **Step 2: Run the application on an HRFI image**

This step is the user's: it needs the GUI and the live data.

1. Start `bin/EUMETCastView` from `bin/` (the INI there is the live one).
2. Compose a Meteosat-12 HRFI image — e.g. `vis_06_hr` from
   `/mnt/nfs_Vol4T/received/hvs-3/E3H-MTG-3/<yyyy>/<mm>/<dd>` — and press
   **CLAHE** on the geostationary tab.
3. In the log, find the line `… recalculateCLAHEMeteosat1 CLAHELab returned 0 in N ms`.
4. Look at the image: contrast enhanced the way it was before, no tiling,
   no seams at region boundaries, disc edge intact.
5. Repeat with an FDHSI composite and, if the machine has the memory for
   it, the 0.5 km sharpened composite — the case the old code could not do.

Expected: `returned 0`, an elapsed time of seconds rather than minutes for
HRFI, and a result that looks like the old CLAHE did.

- [ ] **Step 3: Tell the user the numbers**

Report the probe's `ref`/`new` timings for the 5568² kernel and pipeline
cases and the in-app HRFI elapsed time, and remind them that the probe's
`goes odd 339` difference is the drift fix, not a regression.
