#ifndef SEGMENTLISTVII_H
#define SEGMENTLISTVII_H

#include <QObject>
#include "segmentlist.h"
#include "segmentvii.h"

#include "FreeImage.h"

class SatelliteList;

class SegmentListVII : public SegmentList
{
    Q_OBJECT

public:
    SegmentListVII(eSegmentType type = SEG_METOPSGA1, QObject *parent = 0);
    bool ComposeVIIImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);
    bool ComposeVIIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, int histogrammethod, bool normalized);
    static void doComposeVIIImageInThread(SegmentListVII *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist);

    /**
     * Compose one of the VII RGB recipes over the selected segments.
     *
     * The recipe names its own channels and carries its own stretch, so this
     * path replaces the band radio buttons, the colour combos and the histogram
     * combo alike - none of them has anything left to decide. What each segment
     * lays down is already the displayed brightness, on the radiance scale, so
     * everything downstream of the read is the ordinary VII path at a 100 %
     * stretch.
     */
    bool ComposeVIIRecipeImage(int recipe);
    bool ComposeVIIRecipeImageInThread(int recipe);
    static void doComposeVIIRecipeImageInThread(SegmentListVII *t, int recipe);

    /** Which recipe the image in memory was composed from, or -1 for none. */
    int getRecipe() const { return recipenbr; }

    void ComposeGVProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeLCCProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeSGProjection(int inputchannel, int histogrammethod, bool normalized);
    void ComposeOMProjection(int inputchannel, int histogrammethod, bool normalized);
    void GetCentralCoords(double *startcentrallon, double *startcentrallat, double *endcentrallon, double *endcentrallat);

    void SmoothVIIImage(bool combine);
    void SmoothVIIImage12bits();

    /* The bow-tie overlap removed by the duplication mask leaves each scan an
       hourglass, so across a seam the two ground-adjacent lines are up to 13
       apart in the array and BilinearInterpolation, which only ever pairs a
       line with the next one, cannot reach over it. These close the seam. */
    void BilinearBetweenScans(SegmentVII *segm, bool combine);
    void BilinearBetweenScansSegments(SegmentVII *segmfirst, SegmentVII *segmnext, bool combine);
    void setHistogramMethod(int histo, bool normal) { histogrammethod = histo; normalized = normal; }
    bool ChangeHistogramMethod();

    void ComposeSegments();
    void Compose48bitPNG(QString fileName, bool mapto65535);
    void Compose48bitPNGSegment(SegmentVII *segm, FIBITMAP *bitmap, int heightinsegment, bool mapto65535);

    void RecalculateCLAHEVII();
    long NbrOfSaturatedPixels();
    bool searchLatLon(int mapx, int mapy, float &lon, float &lat);

private:
    /* One seam quad: column pixelx between line linefirst of segmfirst and line
       linenext of segmnext. Same canvas the other two passes draw on. */
    void BridgeSeam(SegmentVII *segmfirst, int linefirst, SegmentVII *segmnext, int linenext,
                    int pixelx, bool combine);
    /* Last, resp. first, line of the scan starting at scanline that carries a
       projection coordinate for both pixelx and pixelx+1, or -1 when the
       duplication mask left the column empty in that scan. */
    static int lastUsableLine(SegmentVII *segm, int scanline, int nd, int pixelx);
    static int firstUsableLine(SegmentVII *segm, int scanline, int nd, int pixelx);

    void CalculateLUT();
    void CalculateLUTAlt();
    void CalculateLUTFull();
    void CalculateProjectionLUT();

    QFutureWatcher<void> *watchervii;

    int histogrammethod;
    bool normalized;

    /* Set while the image in memory came from an RGB recipe. The histogram
       methods do not apply to one - the recipe already decided the stretch - and
       neither does the CLAHE pass that finishedvii would otherwise run. */
    int recipenbr = -1;

protected slots:
    void finishedvii();
    void progressreadvalue(int progress);

};

#endif // SEGMENTLISTVII_H
