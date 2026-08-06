#include "segmentimage.h"

#include <QDebug>
#define uiNR_OF_GREY (4096)

const unsigned int uiMAX_REG_X = 16;	  /* max. # contextual regions in x-direction */
const unsigned int uiMAX_REG_Y = 16;	  /* max. # contextual regions in y-direction */

extern Options opts;

SegmentImage::SegmentImage()
{
    for(int k = 0; k < 5; k++)
    {
        this->ptrimagecomp_ch[k] = new QImage();
    }

    ptrimagecomp_col = new QImage();
    ptrexpand_col = new QImage();
    ptrimageGeostationary = new QImage(); //3712, 3712, QImage::Format_ARGB32);
    //ptrimageGeostationaryNight = new QImage();
    ptrimageProjection = new QImage();
    ptrimageProjectionCopy = new QImage();
    ptrimageViirsM = new QImage();
    ptrimageViirsDNB = new QImage();
    ptrimageOLCI = new QImage();
    ptrimageSLSTR = new QImage();
    ptrimageMERSI = new QImage();
    ptrProjectionBrightnessTemp.reset();

    ptrimageRGBRecipeRed.reset(new quint8[3712 * 3712]);
    ptrimageRGBRecipeGreen.reset(new quint8[3712 * 3712]);
    ptrimageRGBRecipeBlue.reset(new quint8[3712 * 3712]);

    alphazero = false;
    olcitype = SEG_NONE;

    CalcSatAngles();

    for( int i = 0; i < 10; i++)
    {
        ptrRed[i] = NULL;
        ptrGreen[i] = NULL;
        ptrBlue[i] = NULL;
        ptrNight[i] = NULL;
    }

    for( int i = 0; i < 3; i++)
    {
        for( int j = 0; j < 40; j++)
        {
            ptrMTG[i][j] = NULL;
            ptrIndex[i][j] = NULL;
        }
    }

    for( int i = 0; i < 24; i++)
    {
        ptrHRV[i] = NULL;
    }

    for( int i = 0; i < 3; i++)
    {
        ptrDQF[i] = NULL;
    }

    for( int i = 0; i < 3; i++)
    {
        fillvalue[i] = 0;
    }
    SetupSEVIRIRGBrecipes();
    SetupFCIRGBrecipes();
}

//void SegmentImage::SetupRGBrecipes()
//{
//    QList<QString> recipes;

//    recipes << "Airmass RGB"
//            << "Dust RGB"
//            << "24 hours Microphysics RGB"
//            << "Ash RGB"
//            << "Day Microphysics RGB"
//            << "Severe Storms RGB"
//            << "Snow RGB"
//            << "Natural Colors RGB"
//            << "Night Microphysics RGB";

//}

// Helper: build one RGBRecipeColor for a SEVIRI channel.
// The channel number follows from the name, so a recipe never spells it out.
// So does the dimension: a brightness temperature is stretched between two
// absolute kelvin bounds, everything else between two fractions of the scene
// min/max, which is what the compose loop reads "K" and "%" to mean.
static RGBRecipeColor makeSEVIRIColor(const QString& band, seviriunits units,
                                      float from, float to, float gamma,
                                      bool inverse = false, bool reflective = false)
{
    RGBRecipeColor c;
    c.channels.append(band);
    c.spectral_channel_nbr.append(SegmentImage::GetSpectralChannelNbr(band));
    c.subtract.append(false);   // first entry is never subtracted from itself
    c.inverse.append(inverse);
    c.reflective.append(reflective);
    c.units     = units;
    c.rangefrom = from;
    c.rangeto   = to;
    c.gamma     = gamma;
    c.dimension = (units == SEVIRI_UNIT_BT) ? "K" : "%";
    return c;
}

// Append a second channel to an existing RGBRecipeColor (for difference channels)
static void appendSEVIRIBand(RGBRecipeColor& c, const QString& band, bool subtract)
{
    c.channels.append(band);
    c.spectral_channel_nbr.append(SegmentImage::GetSpectralChannelNbr(band));
    c.subtract.append(subtract);
    c.inverse.append(false);
    c.reflective.append(false);
}

void SegmentImage::SetupSEVIRIRGBrecipes()
{
    // Channel numbers (1-12): VIS006(1) VIS008(2) IR_016(3) IR_039(4)
    //                         WV_062(5) WV_073(6) IR_087(7) IR_097(8)
    //                         IR_108(9) IR_120(10) IR_134(11) HRV(12)
    //
    // The order of this list is an interface: ComposeGeoRGBRecipeInThread
    // dispatches the recipes that need their own composer by index, so a recipe
    // may be appended but not moved.

    // 0 - Airmass RGB
    {
        RGBRecipe r;
        r.Name = "SEVIRI Airmass RGB";
        r.needsza = false;
        RGBRecipeColor R = makeSEVIRIColor("WV_062", SEVIRI_UNIT_BT, -25.0f, 0.0f, 1.0f);
        appendSEVIRIBand(R, "WV_073", true);
        RGBRecipeColor G = makeSEVIRIColor("IR_097", SEVIRI_UNIT_BT, -40.0f, 5.0f, 1.0f);
        appendSEVIRIBand(G, "IR_108", true);
        RGBRecipeColor B = makeSEVIRIColor("WV_062", SEVIRI_UNIT_BT, 208.0f, 243.0f, 1.0f, true);
        r.Colorvector << R << G << B;
        seviri_rgbrecipes.append(r);
    }

    // 1 - Dust RGB
    {
        RGBRecipe r;
        r.Name = "SEVIRI Dust RGB";
        r.needsza = false;
        RGBRecipeColor R = makeSEVIRIColor("IR_120", SEVIRI_UNIT_BT, -4.0f, 2.0f, 1.0f);
        appendSEVIRIBand(R, "IR_108", true);
        RGBRecipeColor G = makeSEVIRIColor("IR_108", SEVIRI_UNIT_BT, 0.0f, 15.0f, 2.5f);
        appendSEVIRIBand(G, "IR_087", true);
        RGBRecipeColor B = makeSEVIRIColor("IR_108", SEVIRI_UNIT_BT, 261.0f, 289.0f, 1.0f);
        r.Colorvector << R << G << B;
        seviri_rgbrecipes.append(r);
    }

    // 2 - 24 hours Microphysics RGB
    {
        RGBRecipe r;
        r.Name = "SEVIRI 24 hours Microphysics RGB";
        r.needsza = false;
        RGBRecipeColor R = makeSEVIRIColor("IR_120", SEVIRI_UNIT_BT, -4.0f, 2.0f, 1.0f);
        appendSEVIRIBand(R, "IR_108", true);
        RGBRecipeColor G = makeSEVIRIColor("IR_108", SEVIRI_UNIT_BT, 0.0f, 6.0f, 1.2f);
        appendSEVIRIBand(G, "IR_087", true);
        RGBRecipeColor B = makeSEVIRIColor("IR_108", SEVIRI_UNIT_BT, 248.0f, 303.0f, 1.0f);
        r.Colorvector << R << G << B;
        seviri_rgbrecipes.append(r);
    }

    // 3 - Ash RGB
    {
        RGBRecipe r;
        r.Name = "SEVIRI Ash RGB";
        r.needsza = false;
        RGBRecipeColor R = makeSEVIRIColor("IR_120", SEVIRI_UNIT_BT, -4.0f, 2.0f, 1.0f);
        appendSEVIRIBand(R, "IR_108", true);
        RGBRecipeColor G = makeSEVIRIColor("IR_108", SEVIRI_UNIT_BT, -4.0f, 5.0f, 1.0f);
        appendSEVIRIBand(G, "IR_087", true);
        RGBRecipeColor B = makeSEVIRIColor("IR_108", SEVIRI_UNIT_BT, 243.0f, 303.0f, 1.0f);
        r.Colorvector << R << G << B;
        seviri_rgbrecipes.append(r);
    }

    // 4 - Day Microphysics RGB Summer
    // Composed by ComposeDayMicrophysicsRGB: the green is the solar part of
    // IR_039, which has to be separated from the thermal part first.
    {
        RGBRecipe r;
        r.Name = "SEVIRI Day Microphysics RGB Summer";
        r.needsza = true;
        RGBRecipeColor R = makeSEVIRIColor("VIS008", SEVIRI_UNIT_REF,    0.0f, 1.0f, 1.0f);
        RGBRecipeColor G = makeSEVIRIColor("IR_039", SEVIRI_UNIT_REFL39, 0.0f, 0.6f, 2.5f, false, true);
        RGBRecipeColor B = makeSEVIRIColor("IR_108", SEVIRI_UNIT_BT,   203.0f, 323.0f, 1.0f);
        r.Colorvector << R << G << B;
        seviri_rgbrecipes.append(r);
    }

    // 5 - Severe Storms RGB
    {
        RGBRecipe r;
        r.Name = "SEVIRI Severe Storms RGB";
        r.needsza = false;
        RGBRecipeColor R = makeSEVIRIColor("WV_062", SEVIRI_UNIT_BT, -35.0f, 5.0f, 1.0f);
        appendSEVIRIBand(R, "WV_073", true);
        RGBRecipeColor G = makeSEVIRIColor("IR_039", SEVIRI_UNIT_BT, -5.0f, 60.0f, 0.5f);
        appendSEVIRIBand(G, "IR_108", true);
        RGBRecipeColor B = makeSEVIRIColor("IR_016", SEVIRI_UNIT_REF, -0.75f, 0.25f, 1.0f);
        appendSEVIRIBand(B, "VIS006", true);
        r.Colorvector << R << G << B;
        seviri_rgbrecipes.append(r);
    }

    // 6 - Snow RGB
    // Composed by ComposeSnowRGB, for the same reason as Day Microphysics.
    {
        RGBRecipe r;
        r.Name = "SEVIRI Snow RGB";
        r.needsza = true;
        RGBRecipeColor R = makeSEVIRIColor("VIS008", SEVIRI_UNIT_BRF,    0.0f, 1.0f, 1.7f);
        RGBRecipeColor G = makeSEVIRIColor("IR_016", SEVIRI_UNIT_BRF,    0.0f, 0.7f, 1.7f);
        RGBRecipeColor B = makeSEVIRIColor("IR_039", SEVIRI_UNIT_REFL39, 0.0f, 0.3f, 1.7f, false, true);
        r.Colorvector << R << G << B;
        seviri_rgbrecipes.append(r);
    }

    // 7 - Natural Colours RGB
    // The one SEVIRI recipe read as a picture rather than as a diagnostic, so
    // the only one worth Rayleigh-correcting. It acts almost entirely on the
    // blue, VIS006 being the only channel here with any appreciable optical
    // depth, which lifts the haze off ocean and dark land.
    {
        RGBRecipe r;
        r.Name = "SEVIRI Natural Colours RGB";
        r.needsza = false;
        r.rayleigh = true;
        RGBRecipeColor R = makeSEVIRIColor("IR_016", SEVIRI_UNIT_REF, 0.0f, 0.9f, 1.8f);
        RGBRecipeColor G = makeSEVIRIColor("VIS008", SEVIRI_UNIT_REF, 0.0f, 0.9f, 1.8f);
        RGBRecipeColor B = makeSEVIRIColor("VIS006", SEVIRI_UNIT_REF, 0.0f, 0.9f, 1.8f);
        r.Colorvector << R << G << B;
        seviri_rgbrecipes.append(r);
    }

    // 8 - Night Microphysics RGB
    {
        RGBRecipe r;
        r.Name = "SEVIRI Night Microphysics RGB";
        r.needsza = false;
        RGBRecipeColor R = makeSEVIRIColor("IR_120", SEVIRI_UNIT_BT, -4.0f, 2.0f, 1.0f);
        appendSEVIRIBand(R, "IR_108", true);
        RGBRecipeColor G = makeSEVIRIColor("IR_108", SEVIRI_UNIT_BT, 0.0f, 10.0f, 1.0f);
        appendSEVIRIBand(G, "IR_039", true);
        RGBRecipeColor B = makeSEVIRIColor("IR_108", SEVIRI_UNIT_BT, 243.0f, 293.0f, 1.0f);
        r.Colorvector << R << G << B;
        seviri_rgbrecipes.append(r);
    }

    // 9 - IR_039 sun reflected
    // Grey: the solar part of IR_039 on all three colours.
    {
        RGBRecipe r;
        r.Name = "SEVIRI IR_039 sun reflected";
        r.needsza = true;
        RGBRecipeColor C = makeSEVIRIColor("IR_039", SEVIRI_UNIT_REFL39, 0.0f, 1.0f, 1.0f);
        r.Colorvector << C << C << C;
        seviri_rgbrecipes.append(r);
    }

    // 10 - Day Microphysics RGB Winter
    // Same channels as the summer recipe; the green is stretched over a quarter
    // of the range because a low winter sun reflects that much less.
    {
        RGBRecipe r;
        r.Name = "SEVIRI Day Microphysics RGB Winter";
        r.needsza = true;
        RGBRecipeColor R = makeSEVIRIColor("VIS008", SEVIRI_UNIT_REF,    0.0f, 1.0f, 1.0f);
        RGBRecipeColor G = makeSEVIRIColor("IR_039", SEVIRI_UNIT_REFL39, 0.0f, 0.25f, 1.5f, false, true);
        RGBRecipeColor B = makeSEVIRIColor("IR_108", SEVIRI_UNIT_BT,   213.0f, 303.0f, 1.0f);
        r.Colorvector << R << G << B;
        seviri_rgbrecipes.append(r);
    }
}

int SegmentImage::GetFCIBandIndex(const QString& bandname)
{
    // Returns 0-15 for the 16 FCI bands, or -1 if not found
    static const char* names[16] = {
        "vis_04", "vis_05", "vis_06", "vis_08", "vis_09",
        "nir_13", "nir_16", "nir_22",
        "ir_38", "wv_63", "wv_73", "ir_87", "ir_97", "ir_105", "ir_123", "ir_133"
    };
    for (int i = 0; i < 16; i++)
        if (bandname == names[i]) return i;
    return -1;
}

// Helper: build one RGBRecipeColor for an FCI band
// subtract=true means this band is subtracted from the previous
static RGBRecipeColor makeFCIColor(const QString& band, int bandIdx,
                                   bool subtract, bool inverse,
                                   float from, float to, float gamma)
{
    RGBRecipeColor c;
    c.channels.append(band);
    c.spectral_channel_nbr.append(bandIdx);
    c.subtract.append(false);   // first entry is never subtracted from itself
    c.inverse.append(inverse);
    c.reflective.append(false);
    c.units = SEVIRI_UNIT_BT;
    c.rangefrom = from;
    c.rangeto   = to;
    c.gamma     = gamma;
    c.dimension = "K";
    return c;
}

// Append a second band to an existing RGBRecipeColor (for difference channels)
static void appendFCIBand(RGBRecipeColor& c, const QString& band, int bandIdx, bool subtract)
{
    c.channels.append(band);
    c.spectral_channel_nbr.append(bandIdx);
    c.subtract.append(subtract);
    c.inverse.append(false);
    c.reflective.append(false);
}

void SegmentImage::SetupFCIRGBrecipes()
{
    // Band indices (0-15): vis_04(0) vis_05(1) vis_06(2) vis_08(3) vis_09(4)
    //                      nir_13(5) nir_16(6) nir_22(7)
    //                      ir_38(8) wv_63(9) wv_73(10) ir_87(11) ir_97(12)
    //                      ir_105(13) ir_123(14) ir_133(15)

    // 0 - GeoColor
    // Layered day/night composite after Miller et al. (2020). Daytime is the
    // Rayleigh-corrected true colour with an NDVI-driven vegetation
    // enhancement; night is an infrared cloud layer over a dark earth, with
    // city lights if an image is configured. The two are joined across the
    // terminator by the same twilight fade the correction already applies.
    //
    // The colours name the three day bands and set the day stretch, matching
    // FCI True Color RGB so the lit half looks like the picture it grew out of.
    // The near infrared and the two infrared windows are dependencies rather
    // than colours, so they go in auxchannels and composeFCIGeoColor resolves
    // them by name.
    {
        RGBRecipe r;
        r.Name = "FCI GeoColor RGB";
        r.needsza = true;
        r.compose = RECIPE_GEOCOLOR;
        RGBRecipeColor R = makeFCIColor("vis_06", 2, false, false, 0.0f, 1.0f, 2.2f);
        RGBRecipeColor G = makeFCIColor("vis_05", 1, false, false, 0.0f, 1.0f, 2.2f);
        RGBRecipeColor B = makeFCIColor("vis_04", 0, false, false, 0.0f, 1.0f, 2.2f);
        r.Colorvector << R << G << B;
        r.auxchannels << "vis_08" << "ir_38" << "ir_105";
        r.auxbands    << 3       << 8       << 13;
        fci_rgbrecipes.append(r);
    }

    // 1 - True Color RGB
    // R: vis_06 (0.635 µm red), G: vis_05 (0.51 µm green), B: vis_04 (0.44 µm blue)
    {
        RGBRecipe r;
        r.Name = "FCI True Color RGB";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("vis_06", 2, false, false, 0.0f, 1.0f, 2.2f);
        RGBRecipeColor G = makeFCIColor("vis_05", 1, false, false, 0.0f, 1.0f, 2.2f);
        RGBRecipeColor B = makeFCIColor("vis_04", 0, false, false, 0.0f, 1.0f, 2.2f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 2 - True Color NDVI RGB
    // True Color RGB with the GeoColor vegetation enhancement over the top: the
    // same three bands and the same stretch, but green is pulled toward the near
    // infrared in proportion to NDVI before the display gamma.
    //
    // It exists because a strictly true-colour render puts dense forest at a
    // dark olive - chlorophyll absorbs at 0.51 um as well as in the red - so
    // vegetation reads greener here than the instrument says, and closer to how
    // it looks to a person standing in it. Desert, ocean and cloud all sit at an
    // index near zero and are left exactly as True Color RGB draws them.
    //
    // vis_08 is the near infrared NDVI needs. It is never a colour of its own,
    // so it goes in auxchannels; being a visible band it also keeps the disc at
    // 11136 rather than dropping it to the infrared grid.
    {
        RGBRecipe r;
        r.Name = "FCI True Color NDVI RGB";
        r.needsza = false;
        r.compose = RECIPE_VEGGREEN;
        RGBRecipeColor R = makeFCIColor("vis_06", 2, false, false, 0.0f, 1.0f, 2.2f);
        RGBRecipeColor G = makeFCIColor("vis_05", 1, false, false, 0.0f, 1.0f, 2.2f);
        RGBRecipeColor B = makeFCIColor("vis_04", 0, false, false, 0.0f, 1.0f, 2.2f);
        r.Colorvector << R << G << B;
        r.auxchannels << "vis_08";
        r.auxbands    << 3;
        fci_rgbrecipes.append(r);
    }

    // 3 - True Color Enhanced RGB
    // Tighter range clips the brightest 30 % (clouds still white due to gamma), spreads
    // the land portion further across the 0-255 display range.
    {
        RGBRecipe r;
        r.Name = "FCI True Color Enhanced";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("vis_06", 2, false, false, 0.0f, 0.7f, 2.2f);
        RGBRecipeColor G = makeFCIColor("vis_05", 1, false, false, 0.0f, 0.7f, 2.2f);
        RGBRecipeColor B = makeFCIColor("vis_04", 0, false, false, 0.0f, 0.7f, 2.2f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 4 - Natural Colors RGB
    // R: nir_16 (refl), G: vis_08 (refl), B: vis_06 (refl)
    // EUMETSAT standard: [0, 1.0] all channels, gamma 1.8
    {
        RGBRecipe r;
        r.Name = "FCI Natural Colors RGB";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("nir_16", 6, false, false, 0.0f, 1.0f, 1.8f);
        RGBRecipeColor G = makeFCIColor("vis_08", 3, false, false, 0.0f, 1.0f, 1.8f);
        RGBRecipeColor B = makeFCIColor("vis_06", 2, false, false, 0.0f, 1.0f, 1.8f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 5 - Natural Colors Enhanced RGB
    // Same channels as Natural Colors but tighter ranges to stretch land/sea contrast.
    // Bright clouds clip to white; land features are spread across more of the display range.
    {
        RGBRecipe r;
        r.Name = "FCI Natural Colors Enhanced";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("nir_16", 6, false, false, 0.0f, 0.7f, 1.8f);
        RGBRecipeColor G = makeFCIColor("vis_08", 3, false, false, 0.0f, 0.6f, 1.8f);
        RGBRecipeColor B = makeFCIColor("vis_06", 2, false, false, 0.0f, 0.5f, 1.8f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 6 - Airmass RGB
    {
        RGBRecipe r;
        r.Name = "FCI Airmass RGB";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("wv_63", 9, false, false, -25.0f, 0.0f, 1.0f);
        appendFCIBand(R, "wv_73", 10, true);
        RGBRecipeColor G = makeFCIColor("ir_97", 12, false, false, -40.0f, 5.0f, 1.0f);
        appendFCIBand(G, "ir_105", 13, true);
        RGBRecipeColor B = makeFCIColor("wv_63", 9, false, true, 208.0f, 243.0f, 1.0f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 7 - Dust RGB
    {
        RGBRecipe r;
        r.Name = "FCI Dust RGB";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("ir_123", 14, false, false, -4.0f, 2.0f, 1.0f);
        appendFCIBand(R, "ir_105", 13, true);
        RGBRecipeColor G = makeFCIColor("ir_105", 13, false, false, 0.0f, 15.0f, 2.5f);
        appendFCIBand(G, "ir_87", 11, true);
        RGBRecipeColor B = makeFCIColor("ir_105", 13, false, false, 261.0f, 289.0f, 1.0f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 8 - 24h Microphysics RGB
    {
        RGBRecipe r;
        r.Name = "FCI 24h Microphysics RGB";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("ir_123", 14, false, false, -4.0f, 2.0f, 1.0f);
        appendFCIBand(R, "ir_105", 13, true);
        RGBRecipeColor G = makeFCIColor("ir_105", 13, false, false, 0.0f, 6.0f, 1.2f);
        appendFCIBand(G, "ir_87", 11, true);
        RGBRecipeColor B = makeFCIColor("ir_105", 13, false, false, 248.0f, 303.0f, 1.0f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 9 - Ash RGB
    {
        RGBRecipe r;
        r.Name = "FCI Ash RGB";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("ir_123", 14, false, false, -4.0f, 2.0f, 1.0f);
        appendFCIBand(R, "ir_105", 13, true);
        RGBRecipeColor G = makeFCIColor("ir_105", 13, false, false, -4.0f, 5.0f, 1.0f);
        appendFCIBand(G, "ir_87", 11, true);
        RGBRecipeColor B = makeFCIColor("ir_105", 13, false, false, 243.0f, 303.0f, 1.0f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 10 - Day Severe Storms RGB
    // R: wv_63-wv_73 (BT diff K), G: ir_38-ir_105 (BT diff K), B: nir_16-vis_06 (reflectance diff)
    {
        RGBRecipe r;
        r.Name = "FCI Day Severe Storms RGB";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("wv_63", 9, false, false, -30.0f, 0.0f, 1.0f);
        appendFCIBand(R, "wv_73", 10, true);
        RGBRecipeColor G = makeFCIColor("ir_38", 8, false, false, 0.0f, 55.0f, 0.5f);
        appendFCIBand(G, "ir_105", 13, true);
        RGBRecipeColor B = makeFCIColor("nir_16", 6, false, false, -0.7f, 0.2f, 1.0f);
        appendFCIBand(B, "vis_06", 2, true);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 11 - Snow RGB
    // R: vis_08 (refl), G: nir_16 (refl), B: ir_38 (solar refl approx)
    {
        RGBRecipe r;
        r.Name = "FCI Snow RGB";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("vis_08", 3, false, false, 0.0f, 1.0f, 1.7f);
        RGBRecipeColor G = makeFCIColor("nir_16", 6, false, false, 0.0f, 0.7f, 1.7f);
        RGBRecipeColor B = makeFCIColor("ir_38",  8, false, false, 0.0f, 0.3f, 1.7f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 12 - Cloud Phase RGB
    // R: nir_16 (refl), G: nir_22 (refl), B: vis_06 (refl)
    {
        RGBRecipe r;
        r.Name = "FCI Cloud Phase RGB";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("nir_16", 6, false, false, 0.0f, 0.5f, 1.0f);
        RGBRecipeColor G = makeFCIColor("nir_22", 7, false, false, 0.0f, 0.5f, 1.0f);
        RGBRecipeColor B = makeFCIColor("vis_06", 2, false, false, 0.0f, 1.0f, 1.0f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 13 - Night Microphysics RGB
    {
        RGBRecipe r;
        r.Name = "FCI Night Microphysics RGB";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("ir_123", 14, false, false, -4.0f, 2.0f, 1.0f);
        appendFCIBand(R, "ir_105", 13, true);
        RGBRecipeColor G = makeFCIColor("ir_105", 13, false, false, 0.0f, 10.0f, 1.0f);
        appendFCIBand(G, "ir_38", 8, true);
        RGBRecipeColor B = makeFCIColor("ir_105", 13, false, false, 243.0f, 293.0f, 1.0f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 14 - Night Fog RGB (same channels as Night Microphysics, different G stretch)
    {
        RGBRecipe r;
        r.Name = "FCI Night Fog RGB";
        r.needsza = false;
        RGBRecipeColor R = makeFCIColor("ir_123", 14, false, false, -4.0f, 2.0f, 1.0f);
        appendFCIBand(R, "ir_105", 13, true);
        RGBRecipeColor G = makeFCIColor("ir_105", 13, false, false, 0.0f, 6.0f, 2.0f);
        appendFCIBand(G, "ir_38", 8, true);
        RGBRecipeColor B = makeFCIColor("ir_105", 13, false, false, 243.0f, 293.0f, 1.0f);
        r.Colorvector << R << G << B;
        fci_rgbrecipes.append(r);
    }

    // 15 - NDVI
    // (vis_08 - vis_06) / (vis_08 + vis_06). Vegetation is dark in the red and
    // bright in the near infrared, so the index separates it from soil, water
    // and cloud far better than either band alone.
    //
    // Grey: all three colours carry the same index, mapped linearly from -1 to
    // +1 onto 0..254. Being a ratio it is insensitive to the sun-zenith
    // normalisation, so it stays meaningful whether or not the Rayleigh
    // correction is switched on - with it on this is a surface NDVI, without it
    // a top-of-atmosphere one.
    //
    // Groundwork for the GeoColor composite, which uses NDVI to decide how land
    // should be tinted.
    {
        RGBRecipe r;
        r.Name = "FCI NDVI";
        r.needsza = false;
        r.compose = RECIPE_NORMDIFF;
        RGBRecipeColor R = makeFCIColor("vis_08", 3, false, false, -1.0f, 1.0f, 1.0f);
        appendFCIBand(R, "vis_06", 2, true);
        r.Colorvector << R << R << R;
        fci_rgbrecipes.append(r);
    }


}

int SegmentImage::GetSpectralChannelNbr(QString channel)
{
    //channel.remove(QChar('_'), Qt::CaseInsensitive);

    if(channel == "VIS006")
        return 1;
    else if(channel == "VIS008")
        return 2;
    else if(channel == "IR_016")
        return 3;
    else if(channel == "IR_039")
        return 4;
    else if(channel == "WV_062")
        return 5;
    else if(channel == "WV_073")
        return 6;
    else if(channel == "IR_087")
        return 7;
    else if(channel == "IR_097")
        return 8;
    else if(channel == "IR_108")
        return 9;
    else if(channel == "IR_120")
        return 10;
    else if(channel == "IR_134")
        return 11;
    else if(channel == "HRV")
        return 12;
    else
        return 0;
}

void SegmentImage::CalcSatAngles()
{
    double swath = 55.3*PIE/180.0;
    double deltaphi = swath/1024;
    double deltaphigac  = swath/204.5;
    double phi, beta, d;
    double satheight = 850;
    double delta, totdelta;

    for(int i = 1; i < 1024; i++)
    {
        phi= deltaphi*(double)i;
        beta=PIE-ArcSin((XKMPER_WGS84 + satheight)*sin(phi)/XKMPER_WGS84);
        Sigmadist[1024 - i] = Sigmadist[1024 + i] = PIE - beta - phi;
    }
    Sigmadist[1024] = 0.0;
    Sigmadist[0] = Sigmadist[1];

    for(int i = 1; i < 204; i++)
    {
        phi= deltaphigac*(double)i;
        beta=PIE-ArcSin((XKMPER_WGS84 + satheight)*sin(phi)/XKMPER_WGS84);
        SigmadistGAC[204 - i] = SigmadistGAC[204 + i] = PIE - beta - phi;
    }
    SigmadistGAC[204] = 0.0;
    SigmadistGAC[0] = SigmadistGAC[1] = SigmadistGAC[2] = SigmadistGAC[3] = 0.0;
    SigmadistGAC[405] = SigmadistGAC[406] = SigmadistGAC[407] = SigmadistGAC[408] = 0.0;


    for(int i = 0; i < 2048; i++)
    {
        fraction[i] = 0.0;
    }

    for(int i = 0; i < 409; i++)
    {
        fractionGAC[i] = 0.0;
    }

    for(int i = 0; i < 102; i++)
    {
        totdelta = fabs(Sigmadist[4 + i*20] - Sigmadist[4 + (i+1)*20]);
        for(int j = 0; j < 20; j++)
        {
            delta=fabs(Sigmadist[4 + i*20] - Sigmadist[4 + i*20 + j]);
            fraction[4 + i*20 + j] = delta/totdelta;
        }
    }

    for(int i = 0; i < 50; i++)
    {
        totdelta = fabs(SigmadistGAC[4 + i*8] - SigmadistGAC[4 + (i+1)*8]);
        for(int j = 0; j < 8; j++)
        {
            delta=fabs(SigmadistGAC[4 + i*8] - SigmadistGAC[4 + i*8 + j]);
            fractionGAC[4 + i*8 + j] = delta/totdelta;
        }
    }
    fractionGAC[404] = 1.0;
}


void SegmentImage::DeleteImagePtrs()
{

    qDebug() << "in DeleteImagePointers";

    for(int k = 0; k < 5; k++)
    {
        if(ptrimagecomp_ch[k] != NULL)
        {
            delete ptrimagecomp_ch[k];
            ptrimagecomp_ch[k] = NULL;
        }
    }

    if(ptrimagecomp_col != NULL)
    {
        delete ptrimagecomp_col;
        ptrimagecomp_col = NULL;
    }

    if(ptrexpand_col != NULL)
    {
        delete ptrexpand_col;
        ptrexpand_col = NULL;
    }

    if(ptrimageGeostationary != NULL)
    {
        delete ptrimageGeostationary;
        ptrimageGeostationary = NULL;
    }


    ptrimageGeoNight.reset();

    if(ptrimageProjection != NULL)
    {
        delete ptrimageProjection;
        ptrimageProjection = NULL;
    }

    ptrimageProjectionRed.reset();
    ptrimageProjectionGreen.reset();
    ptrimageProjectionBlue.reset();
    ptrimageProjectionAlpha.reset();

    ptrimageRGBRecipeRed.reset();
    ptrimageRGBRecipeGreen.reset();
    ptrimageRGBRecipeBlue.reset();

    if(ptrimageViirsM != NULL)
    {
        delete ptrimageViirsM;
        ptrimageViirsM = NULL;
    }

    if(ptrimageViirsDNB != NULL)
    {
        delete ptrimageViirsDNB;
        ptrimageViirsDNB = NULL;
    }

    if(ptrimageOLCI != NULL)
    {
        delete ptrimageOLCI;
        ptrimageOLCI = NULL;
    }

    if(ptrimageSLSTR != NULL)
    {
        delete ptrimageSLSTR;
        ptrimageSLSTR = NULL;
    }

    if(ptrimageMERSI != NULL)
    {
        delete ptrimageMERSI;
        ptrimageMERSI = NULL;
    }

    ResetPtrImage();
}

void SegmentImage::ResetPtrImage()
{

    for( int i = 0; i < 10; i++)
    {
        if (ptrRed[i] != NULL)
        {
            delete [] ptrRed[i];
            ptrRed[i] = NULL;
        }
        if (ptrGreen[i] != NULL)
        {
            delete [] ptrGreen[i];
            ptrGreen[i] = NULL;
        }
        if (ptrBlue[i] != NULL)
        {
            delete [] ptrBlue[i];
            ptrBlue[i] = NULL;
        }
        if (ptrNight[i] != NULL)
        {
            delete [] ptrNight[i];
            ptrNight[i] = NULL;
        }
    }

    for( int i = 0; i < 24; i++)
    {
        if (ptrHRV[i] != NULL)
        {
            delete [] ptrHRV[i];
            ptrHRV[i] = NULL;
        }
    }

    for( int i = 0; i < 3; i++)
    {
        if (ptrDQF[i] != NULL)
        {
            delete [] ptrDQF[i];
            ptrDQF[i] = NULL;
        }
    }
}

void SegmentImage::InitializeAVHRRImages( int imagewidth, int imageheight) // , long stat_min_ch[], long stat_max_ch[] )
{
    qDebug() << "voor initializeimages";

    for(int k = 0; k < 5; k++)
    {
        if(ptrimagecomp_ch[k] != NULL)
            delete ptrimagecomp_ch[k];
    }

    if(ptrimagecomp_col != NULL)
        delete ptrimagecomp_col;

    qDebug() << QString("Total nbr of pixels = %1").arg(imagewidth*imageheight);

    for(int k = 0; k < 5; k++)
    {
        ptrimagecomp_ch[k] = new QImage(imagewidth, imageheight, QImage::Format_ARGB32);
    }

    ptrimagecomp_col = new QImage(imagewidth, imageheight, QImage::Format_ARGB32);


}

void SegmentImage::InitializeImageGeostationary( int imagewidth, int imageheight) // , long stat_min_ch[], long stat_max_ch[] )
{

    if(ptrimageGeostationary != NULL)
        delete ptrimageGeostationary;

    qDebug() << QString("Total nbr of pixels = %1").arg(imagewidth*imageheight);
    qDebug() << QString("width %1  height %2 alphazero = %3").arg(imagewidth).arg(imageheight).arg(alphazero);

    ptrimageGeostationary = new QImage(imagewidth, imageheight, QImage::Format_ARGB32);
    QColor nuts(0,0,0, (alphazero == true ? 0 : 255 ));
    ptrimageGeostationary->fill(nuts);

}



void SegmentImage::ReverseImage()
{
    ptrimagecomp_col = ReverseImageChannel(ptrimagecomp_col);

    for(int k = 0; k < 5; k++)
    {
       ptrimagecomp_ch[k] = ReverseImageChannel(ptrimagecomp_ch[k]);
    }

}


QImage *SegmentImage::ReverseImageChannel(QImage *ptr)
{

    QRgb *row_ch;
    QRgb *row_result;

    int TotalLines = ptr->size().height();
    //qDebug() << QString("TotalLines = %1").arg(TotalLines);


    QImage *ptrimage = new QImage(ptr->size().width(), ptr->size().height(), QImage::Format_ARGB32);

    for( int j = TotalLines - 1, k = 0;  k < TotalLines ; j--, k++)
    {
        row_ch = (QRgb*)ptr->scanLine(j);
        row_result = (QRgb *)ptrimage->scanLine(k);
        for (int l=0, m=ptr->size().width()-1; l < ptr->size().width(); l++, m--)
        {
            row_result[l] = row_ch[m];
        }
    }

    delete ptr;

    return ptrimage;
}

void SegmentImage::ExpandImage(int channelshown)
{
    QRgb *row_col;

    QRgb *row_result;

    if(channelshown == 7)
        return;

    int TotalLines = ptrimagecomp_col->size().height();
    if (TotalLines == 0)
        return;

    qDebug() << QString("=============  in expand image ; totallines = %1").arg(TotalLines);
    qDebug() << QString("=============  in expand image ; width = %1").arg(ptrimagecomp_col->size().width());
    qDebug() << QString("=============  in expand image ; channelshown = %1").arg(channelshown);

    int nbrwidth = ptrimagecomp_col->size().width()/2;

    double theta_p = Radians(55.37) /nbrwidth;

    int *p = new int[nbrwidth];
    p[0] = 1;

    int totalline = 1;

    for (int j = 1; j < nbrwidth; j++)
    {
        p[j] = floor((tan(j*theta_p)-tan((j-1)*theta_p))/tan(theta_p));
        totalline += p[j];
    }

    qDebug() << QString("totalline = %1").arg(totalline);

    delete ptrexpand_col;

    ptrexpand_col = new QImage(2 * totalline, TotalLines, QImage::Format_ARGB32);

    int inp = 0, outp = 0;

    for( int j = 0;  j < TotalLines ; j++)
    {
        switch (channelshown)
        {
            case 1:
                row_col = (QRgb*)ptrimagecomp_ch[0]->scanLine(j);
                row_result = (QRgb *)ptrexpand_col->scanLine(j);
                break;
            case 2:
                row_col = (QRgb*)ptrimagecomp_ch[1]->scanLine(j);
                row_result = (QRgb *)ptrexpand_col->scanLine(j);
                break;
            case 3:
                row_col = (QRgb*)ptrimagecomp_ch[2]->scanLine(j);
                row_result = (QRgb *)ptrexpand_col->scanLine(j);
                break;
            case 4:
                row_col = (QRgb*)ptrimagecomp_ch[3]->scanLine(j);
                row_result = (QRgb *)ptrexpand_col->scanLine(j);
                break;
            case 5:
                row_col = (QRgb*)ptrimagecomp_ch[4]->scanLine(j);
                row_result = (QRgb *)ptrexpand_col->scanLine(j);
                break;
            case 6:
                row_col = (QRgb*)ptrimagecomp_col->scanLine(j);
                row_result = (QRgb *)ptrexpand_col->scanLine(j);
                break;

        }
        inp = nbrwidth-1;
        outp = totalline-1;

        while( inp >=0)
        {

            for( int rep=0; rep < p[nbrwidth-1-inp]; rep++)
            {
                row_result[outp] = row_col[inp];
                outp--;
            }
            inp--;
        }

        inp = nbrwidth;
        outp = totalline;

        while( inp < nbrwidth*2)
        {

            for( int rep=0; rep < p[inp - nbrwidth]; rep++)
            {
                row_result[outp] = row_col[inp];
                outp++;
            }
            inp++;
        }
    }

    delete [] p;
}


void SegmentImage::RotateImage()
{
    ptrimagecomp_col = ReverseImageChannel(ptrimagecomp_col);

    for(int k = 0; k < 5; k++)
    {
       ptrimagecomp_ch[k] = ReverseImageChannel(ptrimagecomp_ch[k]);
    }

}


QImage *SegmentImage::RotateImageChannel(QImage *ptr)
{

    QRgb *row_ch;
    QRgb *row_result;

    int TotalLines = ptr->size().height();
    //qDebug() << QString("TotalLines = %1").arg(TotalLines);


    QImage *ptrimage = new QImage(ptr->size().width(), ptr->size().height(), QImage::Format_ARGB32);

    for( int j = TotalLines - 1, k = 0;  k < TotalLines ; j--, k++)
    {
        row_ch = (QRgb*)ptr->scanLine(j);
        row_result = (QRgb *)ptrimage->scanLine(k);
        for (int l=0, m=ptr->size().width()-1; l < ptr->size().width(); l++, m--)
        {
            row_result[l] = row_ch[m];
        }
    }

    delete ptr;

    return ptrimage;
}

void SegmentImage::showHistogram(QImage *ptr)
{
    QRgb *row_ch;
     int TotalLines = ptr->size().height();
    //qDebug() << QString("TotalLines = %1").arg(TotalLines);

 /*   const CImg<unsigned char>


    for( int j = 0;  j < TotalLines ; j++)
    {
        row_ch = (QRgb*)ptr->scanLine(j);
        for (int m=0; m < ptr->size().width(); m++)
        {
            row_result[l] = row_ch[m];
        }
    }

*/
}

/*
 *
 *
convert from RGB color space to L*a*b* color space:

1. **Normalize RGB values:** Divide each RGB value (red, green, blue) by 255 to normalize them to a range of 0 to 1.

2. **Linear transformation of RGB values:** Apply the following linear transformation to each RGB value:

| Color | Transformation |
|---|---|
| Red | R * 0.4124 + G * 0.3576 + B * 0.1805 |
| Green | R * 0.2126 + G * 0.7152 + B * 0.0722 |
| Blue | R * 0.0193 + G * 0.1192 + B * 0.9505 |

3. **XYZ to CIE XYZ transformation:** Normalize the XYZ values using the reference white (D65):

| Color | Transformation |
|---|---|
| X | X / Xn |
| Y | Y / Yn |
| Z | Z / Zn |

where Xn, Yn, and Zn are the CIE XYZ coordinates of the reference white.
[Xn, Yn, Zn] = [95.047, 100.00, 108.883]

4. **Compute L*, a*, and b* values:**

| Color | Transformation |
|---|---|
| L* | 116 * (Y^(1/3)) - 16 |
| a* | 500 * [(X^(1/3)) - (Y^(1/3))] |
| b* | 200 * [(Y^(1/3)) - (Z^(1/3))] |

where X, Y, and Z are the normalized XYZ values.

The resulting L*, a*, and b* values represent the color in L*a*b* color space.


Converting from L*a*b* color space to RGB color space involves several steps:

1. **Inverse transformation of CIE XYZ coordinates:** Normalize the L*, a*, and b* values:

| Color | Transformation |
|---|---|
| X = (L* + 16) / 116 |
| Y = X - a* / 500 |
| Z = X - b* / 200 |

2. **XYZ to RGB transformation:** Apply the following matrix multiplication to the normalized XYZ values:

```
[[ 3.2406 - 1.5372 - 0.4985],
 [-0.9689 1.8758 0.0415],
 [0.0557 -0.2040 1.4095]] * [[X], [Y], [Z]]
```

3. **Gamma correction:** Apply gamma correction to the resulting RGB values to obtain the final RGB values:

```
R' = 255 * (R^γ)
G' = 255 * (G^γ)
B' = 255 * (B^γ)
```

where R', G', and B' are the gamma-corrected RGB values, R, G, and B are the uncorrected RGB values, and γ is the gamma value (typically 2.2).
 * */
// Contrast Limited Adaptive Histogram Equalization
int  SegmentImage::CLAHE (unsigned short* pImage, unsigned int uiXRes, unsigned int uiYRes,
     unsigned short Min, unsigned short Max, unsigned int uiNrX, unsigned int uiNrY,
          unsigned int uiNrBins, float fCliplimit)
/*   pImage - Pointer to the input/output image
 *   uiXRes - Image resolution in the X direction
 *   uiYRes - Image resolution in the Y direction
 *   Min - Minimum greyvalue of input image (also becomes minimum of output image)
 *   Max - Maximum greyvalue of input image (also becomes maximum of output image)
 *   uiNrX - Number of contextial regions in the X direction (min 2, max uiMAX_REG_X)
 *   uiNrY - Number of contextial regions in the Y direction (min 2, max uiMAX_REG_Y)
 *   uiNrBins - Number of greybins for histogram ("dynamic range")
 *   float fCliplimit - Normalized cliplimit (higher values give more contrast)
 * The number of "effective" greylevels in the output image is set by uiNrBins; selecting
 * a small value (eg. 128) speeds up processing and still produce an output image of
 * good quality. The output image will have the same minimum and maximum value as the input
 * image. A clip limit smaller than 1 results in standard (non-contrast limited) AHE.
 */
{

    unsigned int uiX, uiY;		  /* counters */
    unsigned int uiXSize, uiYSize, uiSubX, uiSubY; /* size of context. reg. and subimages */
    unsigned int uiXL, uiXR, uiYU, uiYB;  /* auxiliary variables interpolation routine */
    unsigned long ulClipLimit, ulNrPixels;/* clip limit and region pixel count */
    unsigned short* pImPointer;		   /* pointer to image */
    unsigned short aLUT[uiNR_OF_GREY];	    /* lookup table used for scaling of input image */
    unsigned long* pulHist, *pulMapArray; /* pointer to histogram and mappings*/
    unsigned long* pulLU, *pulLB, *pulRU, *pulRB; /* auxiliary pointers interpolation */

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
    for (uiY = 0, pImPointer = pImage; uiY < uiNrY; uiY++)
    {
        for (uiX = 0; uiX < uiNrX; uiX++, pImPointer += uiXSize)
        {
            pulHist = &pulMapArray[uiNrBins * (uiY * uiNrX + uiX)];
            MakeHistogram(pImPointer,uiXRes,uiXSize,uiYSize,pulHist,uiNrBins,aLUT);
            ClipHistogram(pulHist, uiNrBins, ulClipLimit);
            MapHistogram(pulHist, Min, Max, uiNrBins, ulNrPixels);
        }
        pImPointer += (uiYSize - 1) * uiXRes;		  /* skip lines, set pointer */
    }

    qDebug() << "Interpolate greylevel mappings to get CLAHE image";
    for (pImPointer = pImage, uiY = 0; uiY <= uiNrY; uiY++)
    {
        if (uiY == 0)       /* special case: top row */
        {
            uiSubY = uiYSize >> 1;  uiYU = 0; uiYB = 0;
        }
        else
        {
            if (uiY == uiNrY)				  /* special case: bottom row */
            {
                uiSubY = uiYSize >> 1;	uiYU = uiNrY-1;	 uiYB = uiYU;
            }
            else
                {					  /* default values */
                    uiSubY = uiYSize; uiYU = uiY - 1; uiYB = uiYU + 1;
                }
        }

        for (uiX = 0; uiX <= uiNrX; uiX++)
        {
            if (uiX == 0)				  /* special case: left column */
            {
                uiSubX = uiXSize >> 1; uiXL = 0; uiXR = 0;
            }
            else
                {
                    if (uiX == uiNrX)			  /* special case: right column */
                    {
                        uiSubX = uiXSize >> 1;  uiXL = uiNrX - 1; uiXR = uiXL;
                    }
                    else
                        {					  /* default values */
                            uiSubX = uiXSize; uiXL = uiX - 1; uiXR = uiXL + 1;
                        }
                }

            pulLU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXL)];
            pulRU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXR)];
            pulLB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXL)];
            pulRB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXR)];
            Interpolate(pImPointer,uiXRes,pulLU,pulRU,pulLB,pulRB,uiSubX,uiSubY,aLUT);
            pImPointer += uiSubX;			  /* set pointer on next matrix */
        }
        pImPointer += (uiSubY - 1) * uiXRes;
    }

    free(pulMapArray);					  /* free space for histograms */
    return 0;						  /* return status OK */
}

void  SegmentImage::ClipHistogram (unsigned long* pulHistogram, unsigned int
             uiNrGreylevels, unsigned long ulClipLimit)
/* This function performs clipping of the histogram and redistribution of bins.
 * The histogram is clipped and the number of excess pixels is counted. Afterwards
 * the excess pixels are equally redistributed across the whole histogram (providing
 * the bin count is smaller than the cliplimit).
 */
{
    unsigned long* pulBinPointer, *pulEndPointer, *pulHisto;
    unsigned long ulNrExcess, ulUpper, ulBinIncr, ulStepSize, i;
    long lBinExcess;

    ulNrExcess = 0;  pulBinPointer = pulHistogram;
    for (i = 0; i < uiNrGreylevels; i++) { /* calculate total number of excess pixels */
    lBinExcess = (long) pulBinPointer[i] - (long) ulClipLimit;
    if (lBinExcess > 0) ulNrExcess += lBinExcess;	  /* excess in current bin */
    };

    /* Second part: clip histogram and redistribute excess pixels in each bin */
    ulBinIncr = ulNrExcess / uiNrGreylevels;		  /* average binincrement */
    ulUpper =  ulClipLimit - ulBinIncr;	 /* Bins larger than ulUpper set to cliplimit */

    for (i = 0; i < uiNrGreylevels; i++)
    {
        if (pulHistogram[i] > ulClipLimit) pulHistogram[i] = ulClipLimit; /* clip bin */
        else
        {
            if (pulHistogram[i] > ulUpper)		/* high bin count */
            {
                ulNrExcess -= pulHistogram[i] - ulUpper; pulHistogram[i]=ulClipLimit;
            }
            else
            {					/* low bin count */
                ulNrExcess -= ulBinIncr; pulHistogram[i] += ulBinIncr;
            }
        }
    }

    while (ulNrExcess)       /* Redistribute remaining excess  */
    {
        pulEndPointer = &pulHistogram[uiNrGreylevels]; pulHisto = pulHistogram;

        while (ulNrExcess && pulHisto < pulEndPointer)
        {
            ulStepSize = uiNrGreylevels / ulNrExcess;
            if (ulStepSize < 1) ulStepSize = 1;		  /* stepsize at least 1 */
            for (pulBinPointer=pulHisto; pulBinPointer < pulEndPointer && ulNrExcess; pulBinPointer += ulStepSize)
            {
                if (*pulBinPointer < ulClipLimit)
                {
                    (*pulBinPointer)++;	 ulNrExcess--;	  /* reduce excess */
                }
            }
            pulHisto++;		  /* restart redistributing on other bin location */
        }
    }
}

void  SegmentImage::MakeHistogram (unsigned short* pImage, unsigned int uiXRes,
        unsigned int uiSizeX, unsigned int uiSizeY,
        unsigned long* pulHistogram,
        unsigned int uiNrGreylevels, unsigned short* pLookupTable)
/* This function classifies the greylevels present in the array image into
 * a greylevel histogram. The pLookupTable specifies the relationship
 * between the greyvalue of the pixel (typically between 0 and 4095) and
 * the corresponding bin in the histogram (usually containing only 128 bins).
 */
{
    unsigned short* pImagePointer;
    unsigned int i;

    for (i = 0; i < uiNrGreylevels; i++) pulHistogram[i] = 0L; /* clear histogram */

    for (i = 0; i < uiSizeY; i++)
    {
        pImagePointer = &pImage[uiSizeX];
        while (pImage < pImagePointer) pulHistogram[pLookupTable[*pImage++]]++;
        pImagePointer += uiXRes;
        pImage = pImagePointer-uiSizeX;
    }
}

void  SegmentImage::MapHistogram (unsigned long* pulHistogram, unsigned short Min, unsigned short Max,
           unsigned int uiNrGreylevels, unsigned long ulNrOfPixels)
/* This function calculates the equalized lookup table (mapping) by
 * cumulating the input histogram. Note: lookup table is rescaled in range [Min..Max].
 */
{
    unsigned int i;  unsigned long ulSum = 0;
    const float fScale = ((float)(Max - Min)) / ulNrOfPixels;
    const unsigned long ulMin = (unsigned long) Min;

    for (i = 0; i < uiNrGreylevels; i++) {
    ulSum += pulHistogram[i]; pulHistogram[i]=(unsigned long)(ulMin+ulSum*fScale);
    if (pulHistogram[i] > Max) pulHistogram[i] = Max;
    }
}

void  SegmentImage::MakeLut (unsigned short * pLUT, unsigned short Min, unsigned short Max, unsigned int uiNrBins)
/* To speed up histogram clipping, the input image [Min,Max] is scaled down to
 * [0,uiNrBins-1]. This function calculates the LUT.
 */
{
    int i;
    const unsigned short BinSize = (unsigned short) (1 + (Max - Min) / uiNrBins);

    for (i = Min; i <= Max; i++)  pLUT[i] = (i - Min) / BinSize;
}

void  SegmentImage::Interpolate (unsigned short *pImage, int uiXRes, unsigned long * pulMapLU,
     unsigned long * pulMapRU, unsigned long * pulMapLB,  unsigned long * pulMapRB,
     unsigned int uiXSize, unsigned int uiYSize, unsigned short *pLUT)
/* pImage      - pointer to input/output image
 * uiXRes      - resolution of image in x-direction
 * pulMap*     - mappings of greylevels from histograms
 * uiXSize     - uiXSize of image submatrix
 * uiYSize     - uiYSize of image submatrix
 * pLUT	       - lookup table containing mapping greyvalues to bins
 * This function calculates the new greylevel assignments of pixels within a submatrix
 * of the image with size uiXSize and uiYSize. This is done by a bilinear interpolation
 * between four different mappings in order to eliminate boundary artifacts.
 * It uses a division; since division is often an expensive operation, I added code to
 * perform a logical shift instead when feasible.
 */
{
    const unsigned int uiIncr = uiXRes-uiXSize; /* Pointer increment after processing row */
    unsigned short GreyValue; unsigned int uiNum = uiXSize*uiYSize; /* Normalization factor */

    unsigned int uiXCoef, uiYCoef, uiXInvCoef, uiYInvCoef, uiShift = 0;

    if (uiNum & (uiNum - 1))   /* If uiNum is not a power of two, use division */
        for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize;  uiYCoef++, uiYInvCoef--,pImage+=uiIncr)
        {
            for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--)
            {
                GreyValue = pLUT[*pImage];		   /* get histogram bin value */
                *pImage++ = (unsigned short ) ((uiYInvCoef * (uiXInvCoef*pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue])
                    + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) / uiNum);
            }
        }
    else
    {			   /* avoid the division and use a right shift instead */
        while (uiNum >>= 1) uiShift++;		   /* Calculate 2log of uiNum */
        for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize; uiYCoef++, uiYInvCoef--,pImage+=uiIncr)
        {
            for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--)
            {
                GreyValue = pLUT[*pImage];	  /* get histogram bin value */
                *pImage++ = (unsigned short)((uiYInvCoef* (uiXInvCoef * pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue])
                    + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) >> uiShift);
            }
        }
    }
}

void  SegmentImage::SmoothProjectionImage()
{
    QRgb *row;
    QRgb val;
    QRgb savepixelfirst;
    quint32 count = 0;
    bool first = true;
    bool hole = false;
    int firstholeindex;
    int reddiff, greendiff, bluediff;
    int diff;

    qDebug() << "SegmentImage::SmoothProjectionImage()";

    for( int h = 0; h < this->ptrimageProjection->height(); h++)
    {
        first = true;
        hole = false;
        row = (QRgb*)this->ptrimageProjection->scanLine(h);
        for( int w = 0; w < this->ptrimageProjection->width(); w++)
        {
            val = *(row + w);
            if (qAlpha(val) == 250 && first)
                continue;
            else
            {
                if (qAlpha(val) == 255 && first)
                {
                    first = false;
                    savepixelfirst = row[w];
                }
                else if (qAlpha(val) == 250 && !first && !hole)
                {
                    hole = true;
                    firstholeindex = w;
                }
                else if (qAlpha(val) == 250 && !first && hole)
                {
                    hole = true;
                }
                else if (qAlpha(val) == 255 && !first && !hole)
                {
                    savepixelfirst = row[w];
                }
                else if (qAlpha(val) == 255 && !first && hole)
                {
                    diff = w - firstholeindex;
                    if (diff < 25)
                    {
                        reddiff = (qRed(row[w]) - qRed(row[firstholeindex - 1]))/diff;
                        greendiff = (qGreen(row[w]) - qGreen(row[firstholeindex - 1]))/diff;
                        bluediff = (qBlue(row[w]) - qBlue(row[firstholeindex - 1]))/diff;

                        for(int ind = firstholeindex; ind < w; ind++)
                        {
                            row[ind] = qRgba(qRed(row[firstholeindex-1])+reddiff, qGreen(row[firstholeindex-1])+greendiff, qBlue(row[firstholeindex-1])+bluediff, 255);
                            count++;
                        }
                    }
                    hole = false;
                }
            }
        }
    }

}


void SegmentImage::boundaryFill4 (int x, int y)
{
    QRgb currentrgb;
    currentrgb = ptrimageProjection->pixel(x, y);
    if (qAlpha(currentrgb) != 255)
    {
        ptrimageProjection->setPixel(x, y, currentrgb);

        boundaryFill4 (x+1, y);
        boundaryFill4 (x-1, y);
        boundaryFill4 (x, y+1);
        boundaryFill4 (x, y-1);
    }
}

qint32 SegmentImage::Min(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22)
{
    qint32 Minimum = v11;

    if( Minimum > v12 )
            Minimum = v12;
    if( Minimum > v21 )
            Minimum = v21;
    if( Minimum > v22 )
            Minimum = v22;

    return Minimum;
}

qint32 SegmentImage::Max(const qint32 v11, const qint32 v12, const qint32 v21, const qint32 v22)
{
    int Maximum = v11;

    if( Maximum < v12 )
            Maximum = v12;
    if( Maximum < v21 )
            Maximum = v21;
    if( Maximum < v22 )
            Maximum = v22;

    return Maximum;
}

bool SegmentImage::bhm_line(int x1, int y1, int x2, int y2, QRgb rgb1, QRgb rgb2, QRgb *canvas, int dimx)
{
    int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;
    float deltared, deltagreen, deltablue;
    float red1, red2, green1, green2, blue1, blue2;

    dx=x2-x1;
    dy=y2-y1;
    dx1=abs(dx);
    dy1=abs(dy);
    px=2*dy1-dx1;
    py=2*dx1-dy1;

    red1 = qRed(rgb1);
    red2 = qRed(rgb2);
    green1 = qGreen(rgb1);
    green2 = qGreen(rgb2);
    blue1 = qBlue(rgb1);
    blue2 = qBlue(rgb2);

    if(dy1<=dx1)
    {
        if(dx1==0)
            return false;

        if(dx>=0)
        {
            x=x1;
            y=y1;
            xe=x2;
            deltared = (float)(qRed(rgb2) - qRed(rgb1))/ (float)dx1 ;
            deltagreen = (float)(qGreen(rgb2) - qGreen(rgb1))/ (float)dx1 ;
            deltablue = (float)(qBlue(rgb2) - qBlue(rgb1))/ (float)dx1 ;
//            canvas[y * yy + x] = val1;

        }
        else
        {
            x=x2;
            y=y2;
            xe=x1;
            deltared = (float)(qRed(rgb1) - qRed(rgb2))/ (float)dx1 ;
            deltagreen = (float)(qGreen(rgb1) - qGreen(rgb2))/ (float)dx1 ;
            deltablue = (float)(qBlue(rgb1) - qBlue(rgb2))/ (float)dx1 ;
//            canvas[y * yy + x] = val2;

        }

        for(i=0;x<xe;i++)
        {
            x=x+1;

            if(px<0)
            {
                px=px+2*dy1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    y=y+1;
                }
                else
                {
                    y=y-1;
                }
                px=px+2*(dy1-dx1);
            }
            if(dx>=0)
            {
                red1 += deltared;
                green1 += deltagreen;
                blue1 += deltablue;

                rgb1 = qRgb((int)red1, (int)green1, (int)blue1 );
                if( x != xe)
                    canvas[y * dimx + x] = rgb1;
            }
            else
            {
                red2 += deltared;
                green2 += deltagreen;
                blue2 += deltablue;

                rgb2 = qRgb((int)red2, (int)green2, (int)blue2 );
                if( x != xe)
                    canvas[y * dimx + x] = rgb2;
            }

        }
    }
    else
    {
        if(dy1==0)
            return false;

        if(dy>=0)
        {
            x=x1;
            y=y1;
            ye=y2;
            deltared = (float)(qRed(rgb2) - qRed(rgb1))/ (float)dy1 ;
            deltagreen = (float)(qGreen(rgb2) - qGreen(rgb1))/ (float)dy1 ;
            deltablue = (float)(qBlue(rgb2) - qBlue(rgb1))/ (float)dy1 ;

//            canvas[y * yy + x] = val1;
        }
        else
        {
            x=x2;
            y=y2;
            ye=y1;
            deltared = (float)(qRed(rgb1) - qRed(rgb2))/ (float)dy1 ;
            deltagreen = (float)(qGreen(rgb1) - qGreen(rgb2))/ (float)dy1 ;
            deltablue = (float)(qBlue(rgb1) - qBlue(rgb2))/ (float)dy1 ;

//            canvas[y * yy + x] = val2;
        }


        for(i=0;y<ye;i++)
        {
            y=y+1;

            if(py<=0)
            {
                py=py+2*dx1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    x=x+1;
                }
                else
                {
                    x=x-1;
                }
                py=py+2*(dx1-dy1);
            }
            if(dy>=0)
            {
                red1 += deltared;
                green1 += deltagreen;
                blue1 += deltablue;

                rgb1 = qRgb((int)red1, (int)green1, (int)blue1 );
                if( y != ye)
                    canvas[y * dimx + x] = rgb1;
            }
            else
            {
                red2 += deltared;
                green2 += deltagreen;
                blue2 += deltablue;

                rgb2 = qRgb((int)red2, (int)green2, (int)blue2 );
                if( y != ye)
                    canvas[y * dimx + x] = rgb2;
            }
        }
    }

    return true;
}

void SegmentImage::MapInterpolation(QRgb *canvas, quint16 dimx, quint16 dimy)
{

    for(int h = 0; h < dimy; h++ )
    {
        QRgb start = qRgba(0,0,0,0);
        QRgb end = qRgba(0,0,0,0);
        bool hole = false;
        bool first = false;
        bool last = false;
        int holecount = 0;

        for(int w = 0; w < dimx; w++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 255 && hole == false)
            {
                start = rgb;
                first = true;
            }
            else if(rgbalpha == 255 && hole == true)
            {
                end = rgb;
                last = true;
                break;
            }
            else if(rgbalpha == 0 && first == true)
            {
                hole = true;
                holecount++;
                canvas[h * dimx + w] = qRgba(0,0,0,100);
            }
        }

        if(holecount == 0)
            continue;
        if(first == false || last == false)
        {
            for(int w = 0; w < dimx; w++)
            {
                QRgb rgb = canvas[h * dimx + w];
                if(qAlpha(rgb) == 100)
                    canvas[h * dimx + w] = qRgba(0,0,0,0);
            }
            continue;
        }



        float deltared = (float)(qRed(end) - qRed(start)) / (float)(holecount+1);
        float deltagreen = (float)(qGreen(end) - qGreen(start)) / (float)(holecount+1);
        float deltablue = (float)(qBlue(end) - qBlue(start)) / (float)(holecount+1);

        float red = (float)qRed(start);
        float green = (float)qGreen(start);
        float blue = (float)qBlue(start);

        for(int w = 0; w < dimx; w++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 100)
            {
                red += deltared;
                green += deltagreen;
                blue += deltablue;
                canvas[h * dimx + w] = qRgba((int)red, (int)green, (int)blue, 100);
            }
        }
    }


    for(int w = 0; w < dimx; w++)
    {
        QRgb start = qRgba(0,0,0,0);
        QRgb end = qRgba(0,0,0,0);

        int hcount = 0;

        bool startok = false;

        for(int h = 0; h < dimy; h++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 255 && !startok)
            {
                start = rgb;
            }
            else
            {
                if(rgbalpha == 255)
                {
                    end = rgb;
                    break;
                }
                else if(rgbalpha == 100)
                {
                    startok = true;
                    hcount++;
                }

            }
        }

        if(hcount == 0)
            continue;

        float redstart = (float)qRed(start);
        float greenstart = (float)qGreen(start);
        float bluestart = (float)qBlue(start);

        float deltared = (float)(qRed(end) - qRed(start)) / (float)(hcount+1);
        float deltagreen = (float)(qGreen(end) - qGreen(start)) / (float)(hcount+1);
        float deltablue = (float)(qBlue(end) - qBlue(start)) / (float)(hcount+1);


        for(int h = 0; h < dimy; h++)
        {
            QRgb rgb = canvas[h * dimx + w];
            int rgbalpha = qAlpha(rgb);
            if(rgbalpha == 100)
            {
                redstart += deltared;
                greenstart += deltagreen;
                bluestart += deltablue;
                float redtotal = (qRed(canvas[h * dimx + w]) + redstart)/2;
                float greentotal = (qGreen(canvas[h * dimx + w]) + greenstart)/2;
                float bluetotal = (qBlue(canvas[h * dimx + w]) + bluestart)/2;

                canvas[h * dimx + w] = qRgba((int)redtotal, (int)greentotal, (int)bluetotal, 255);
            }
        }
    }


}

void SegmentImage::MapCanvas(QRgb *canvas, qint32 anchorX, qint32 anchorY, quint16 dimx, quint16 dimy, bool combine)
{
    for(int h = 0; h < dimy; h++ )
    {
        for(int w = 0; w < dimx; w++)
        {
            QRgb rgb = canvas[h * dimx + w];
            if(qAlpha(rgb) == 255)
            {
                if (anchorX + w >= 0 && anchorX + w < ptrimageProjection->width() &&
                        anchorY + h >= 0 && anchorY + h < ptrimageProjection->height())
                {
                    if(combine)
                    {
                        QRgb rgbproj = ptrimageProjectionCopy->pixel(anchorX + w, anchorY + h);
                        int rproj = qRed(rgbproj);
                        int gproj = qGreen(rgbproj);
                        int bproj = qBlue(rgbproj);
                        int dnbval  = qRed(rgb);

                        float rfact = (float)((255 - rproj) * dnbval)/255.0;
                        float gfact = (float)((255 - gproj) * dnbval)/255.0;
                        float bfact = (float)((255 - bproj) * dnbval)/255.0;
                        int redout = (int)rfact + rproj > 255 ? 255 : (int)rfact + rproj;
                        int greenout = (int)gfact + gproj > 255 ? 255 : (int)gfact + gproj;
                        int blueout = (int)bfact + bproj > 255 ? 255 : (int)bfact + bproj;

                        QRgb rgbout = qRgb(redout, greenout, blueout);
                        ptrimageProjection->setPixel(anchorX + w, anchorY + h, rgbout);

                    }
                    else
                        ptrimageProjection->setPixel(anchorX + w, anchorY + h, rgb);
                }
            }
        }
    }
}
