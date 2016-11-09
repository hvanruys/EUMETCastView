#include "poi.h"
#include <QSettings>
#include <QDebug>


Poi::Poi()
{
}

void Poi::Initialize()
{
    QSettings POIsettings( "POI.ini", QSettings::IniFormat);

    strlGVPName = POIsettings.value("/GVP/strlName").value<QStringList>();
    strlLCCName = POIsettings.value("/LCC/strlName").value<QStringList>();
    strlSGName = POIsettings.value("/SG/strlName").value<QStringList>();
    strlConfigNameM = POIsettings.value("/MConfig/strlName").value<QStringList>();
    strlConfigNameOLCIefr = POIsettings.value("/OLCIefrConfig/strlName").value<QStringList>();

    if(strlGVPName.count() == 0)
    {
        strlGVPName << "User defined" << "Spain" << "Al-Wantanya";
        strlGVPLat << "0.0" << "40.0" << "22.5";
        strlGVPLon << "0.0" << "-4.0" << "28.6";
        strlGVPScale << "1.0" << "0.1" << "0.01";
        strlGVPHeight << "36000" << "999999" << "1000000";
        strlGVPMapWidth << "800" << "2000" << "800";
        strlGVPMapHeight << "600" << "2000" << "600";
        strlGVPGridOnProj << "0" << "1" << "0";
    }
    else
    {
        strlGVPLat = POIsettings.value("/GVP/strlLat").value<QStringList>();
        strlGVPLon = POIsettings.value("/GVP/strlLon").value<QStringList>();
        strlGVPScale = POIsettings.value("/GVP/strlscale").value<QStringList>();
        strlGVPHeight = POIsettings.value("/GVP/strlheight").value<QStringList>();
        strlGVPMapWidth = POIsettings.value("/GVP/strlmapwidth").value<QStringList>();
        strlGVPMapHeight = POIsettings.value("/GVP/strlmapheight").value<QStringList>();
        strlGVPGridOnProj = POIsettings.value("/GVP/strlgridonproj").value<QStringList>();
    }


    if(strlLCCName.count() == 0)
    {
        strlLCCName << "User defined" << "Europe";
        strlLCCParallel1 << "50" << "50";
        strlLCCParallel2 << "20" << "20";
        strlLCCCentral << "9" << "9";
        strlLCCLatOrigin << "0" << "0";
        strlLCCNorth << "62" << "62";
        strlLCCSouth << "35" << "35";
        strlLCCEast << "27" << "27";
        strlLCCWest << "-9" << "-9";
        strlLCCScaleX << "1.0" << "1.0";
        strlLCCScaleY << "1.0" << "1.0";
        strlLCCMapWidth << "1920" << "1920";
        strlLCCMapHeight << "1080" << "1080";
        strlLCCGridOnProj << "0" << "0";
    }
    else
    {
        strlLCCParallel1  = POIsettings.value("/LCC/strlParallel1").value<QStringList>();
        strlLCCParallel2  = POIsettings.value("/LCC/strlParallel2").value<QStringList>();
        strlLCCCentral  = POIsettings.value("/LCC/strlCentral").value<QStringList>();
        strlLCCLatOrigin = POIsettings.value("/LCC/strlLatOrigin").value<QStringList>();
        strlLCCNorth = POIsettings.value("/LCC/strlNorth").value<QStringList>();
        strlLCCSouth = POIsettings.value("/LCC/strlSouth").value<QStringList>();
        strlLCCEast = POIsettings.value("/LCC/strlEast").value<QStringList>();
        strlLCCWest = POIsettings.value("/LCC/strlWest").value<QStringList>();
        strlLCCScaleX = POIsettings.value("/LCC/strlScaleX").value<QStringList>();
        strlLCCScaleY = POIsettings.value("/LCC/strlScaleY").value<QStringList>();
        strlLCCMapHeight = POIsettings.value("/LCC/strlmapheight").value<QStringList>();
        strlLCCMapWidth = POIsettings.value("/LCC/strlmapwidth").value<QStringList>();
        strlLCCGridOnProj = POIsettings.value("/LCC/strlgridonproj").value<QStringList>();
    }

    if(strlSGName.count() == 0)
    {
        strlSGName << "User defined" << "Equatorial 0°" << "Equatorial 180°" << "North" << "South";
        strlSGLat << "0.0" << "0.0" << "0.0" << "90.0" << "-90.0";
        strlSGLon << "0.0" << "0.0" << "180.0" << "0.0" << "0.0";
        strlSGRadius << "90.01" << "90.01" << "90.01" << "90.01" << "90.01";
        strlSGScale << "13.0" << "13.0" << "13.0" << "13.0" << "13.0";
        strlSGPanH << "0" << "0" << "0" << "0" << "0";
        strlSGPanV << "0" << "0" << "0" << "0" << "0";
        strlSGMapWidth << "1000" << "1000" << "1000" << "1000" << "1000";
        strlSGMapHeight << "1000" << "1000" << "1000" << "1000" << "1000";
        strlSGGridOnProj << "1" << "1" << "1" << "1" << "1";

    }
    else
    {
        strlSGLat = POIsettings.value("/SG/strlLat").value<QStringList>();
        strlSGLon = POIsettings.value("/SG/strlLon").value<QStringList>();
        strlSGRadius = POIsettings.value("/SG/strlRadius").value<QStringList>();
        strlSGScale = POIsettings.value("/SG/strlScale").value<QStringList>();
        strlSGPanH = POIsettings.value("/SG/strlPanH").value<QStringList>();
        strlSGPanV = POIsettings.value("/SG/strlPanV").value<QStringList>();
        strlSGMapHeight = POIsettings.value("/SG/strlmapheight").value<QStringList>();
        strlSGMapWidth = POIsettings.value("/SG/strlmapwidth").value<QStringList>();
        strlSGGridOnProj = POIsettings.value("/SG/strlgridonproj").value<QStringList>();
    }

    if(strlConfigNameM.count() == 0)
    {
        strlConfigNameM << "User defined" << "Natural Colors" << "True Colors" << "M13-M14-M15";

        strlColorBandM << "0" << "0" << "0" << "0"; // "0" = color , "1" = M1, ..

        strlComboM1 << "0" << "0" << "0" << "0";
        strlComboM2 << "0" << "0" << "0" << "0";
        strlComboM3 << "0" << "0" << "3" << "0";
        strlComboM4 << "0" << "0" << "2" << "0";
        strlComboM5 << "3" << "3" << "1" << "0";
        strlComboM6 << "0" << "0" << "0" << "0";
        strlComboM7 << "2" << "2" << "0" << "0";
        strlComboM8 << "0" << "0" << "0" << "0";
        strlComboM9 << "0" << "0" << "0" << "0";
        strlComboM10 << "1" << "1" << "0" << "0";
        strlComboM11 << "0" << "0" << "0" << "0";
        strlComboM12 << "0" << "0" << "0" << "0";
        strlComboM13 << "0" << "0" << "0" << "3";
        strlComboM14 << "0" << "0" << "0" << "2";
        strlComboM15 << "0" << "0" << "0" << "1";
        strlComboM16 << "0" << "0" << "0" << "0";

        strlInverseM1 << "0" << "0" << "0" << "0";
        strlInverseM2 << "0" << "0" << "0" << "0";
        strlInverseM3 << "0" << "0" << "0" << "0";
        strlInverseM4 << "0" << "0" << "0" << "0";
        strlInverseM5 << "0" << "0" << "0" << "0";
        strlInverseM6 << "0" << "0" << "0" << "0";
        strlInverseM7 << "0" << "0" << "0" << "0";
        strlInverseM8 << "0" << "0" << "0" << "0";
        strlInverseM9 << "0" << "0" << "0" << "0";
        strlInverseM10 << "0" << "0" << "0" << "0";
        strlInverseM11 << "1" << "1" << "1" << "1";
        strlInverseM12 << "1" << "1" << "1" << "1";
        strlInverseM13 << "1" << "1" << "1" << "1";
        strlInverseM14 << "1" << "1" << "1" << "1";
        strlInverseM15 << "1" << "1" << "1" << "1";
        strlInverseM16 << "1" << "1" << "1" << "1";

    }
    else
    {
        strlColorBandM = POIsettings.value("/MConfig/strlColorBandM").value<QStringList>();

        strlComboM1 = POIsettings.value("/MConfig/strlComboM1").value<QStringList>();
        strlComboM2 = POIsettings.value("/MConfig/strlComboM2").value<QStringList>();
        strlComboM3 = POIsettings.value("/MConfig/strlComboM3").value<QStringList>();
        strlComboM4 = POIsettings.value("/MConfig/strlComboM4").value<QStringList>();
        strlComboM5 = POIsettings.value("/MConfig/strlComboM5").value<QStringList>();
        strlComboM6 = POIsettings.value("/MConfig/strlComboM6").value<QStringList>();
        strlComboM7 = POIsettings.value("/MConfig/strlComboM7").value<QStringList>();
        strlComboM8 = POIsettings.value("/MConfig/strlComboM8").value<QStringList>();
        strlComboM9 = POIsettings.value("/MConfig/strlComboM9").value<QStringList>();
        strlComboM10 = POIsettings.value("/MConfig/strlComboM10").value<QStringList>();
        strlComboM11 = POIsettings.value("/MConfig/strlComboM11").value<QStringList>();
        strlComboM12 = POIsettings.value("/MConfig/strlComboM12").value<QStringList>();
        strlComboM13 = POIsettings.value("/MConfig/strlComboM13").value<QStringList>();
        strlComboM14 = POIsettings.value("/MConfig/strlComboM14").value<QStringList>();
        strlComboM15 = POIsettings.value("/MConfig/strlComboM15").value<QStringList>();
        strlComboM16 = POIsettings.value("/MConfig/strlComboM16").value<QStringList>();

        strlInverseM1 = POIsettings.value("/MConfig/strlInverseM1").value<QStringList>();
        strlInverseM2 = POIsettings.value("/MConfig/strlInverseM2").value<QStringList>();
        strlInverseM3 = POIsettings.value("/MConfig/strlInverseM3").value<QStringList>();
        strlInverseM4 = POIsettings.value("/MConfig/strlInverseM4").value<QStringList>();
        strlInverseM5 = POIsettings.value("/MConfig/strlInverseM5").value<QStringList>();
        strlInverseM6 = POIsettings.value("/MConfig/strlInverseM6").value<QStringList>();
        strlInverseM7 = POIsettings.value("/MConfig/strlInverseM7").value<QStringList>();
        strlInverseM8 = POIsettings.value("/MConfig/strlInverseM8").value<QStringList>();
        strlInverseM9 = POIsettings.value("/MConfig/strlInverseM9").value<QStringList>();
        strlInverseM10 = POIsettings.value("/MConfig/strlInverseM10").value<QStringList>();
        strlInverseM11 = POIsettings.value("/MConfig/strlInverseM11").value<QStringList>();
        strlInverseM12 = POIsettings.value("/MConfig/strlInverseM12").value<QStringList>();
        strlInverseM13 = POIsettings.value("/MConfig/strlInverseM13").value<QStringList>();
        strlInverseM14 = POIsettings.value("/MConfig/strlInverseM14").value<QStringList>();
        strlInverseM15 = POIsettings.value("/MConfig/strlInverseM15").value<QStringList>();
        strlInverseM16 = POIsettings.value("/MConfig/strlInverseM16").value<QStringList>();

    }

    if(strlConfigNameOLCIefr.count() == 0)
    {
        strlConfigNameOLCIefr << "User defined" << "Natural Colors" << "True Colors";
        strlColorBandOLCIefr << "0" << "0" << "0"; // "0" = color , "1" = Oa01, ..

        strlComboOLCIefr01 << "0" << "0" << "0";
        strlComboOLCIefr02 << "0" << "0" << "0";
        strlComboOLCIefr03 << "0" << "0" << "0";
        strlComboOLCIefr04 << "0" << "0" << "0";
        strlComboOLCIefr05 << "0" << "0" << "0";
        strlComboOLCIefr06 << "0" << "0" << "0";
        strlComboOLCIefr07 << "0" << "0" << "0";
        strlComboOLCIefr08 << "0" << "0" << "0";
        strlComboOLCIefr09 << "0" << "0" << "0";
        strlComboOLCIefr10 << "0" << "0" << "0";
        strlComboOLCIefr11 << "0" << "0" << "0";
        strlComboOLCIefr12 << "0" << "0" << "0";
        strlComboOLCIefr13 << "0" << "0" << "0";
        strlComboOLCIefr14 << "0" << "0" << "0";
        strlComboOLCIefr15 << "0" << "0" << "0";
        strlComboOLCIefr16 << "0" << "0" << "0";
        strlComboOLCIefr17 << "0" << "0" << "0";
        strlComboOLCIefr18 << "0" << "0" << "0";
        strlComboOLCIefr19 << "0" << "0" << "0";
        strlComboOLCIefr20 << "0" << "0" << "0";
        strlComboOLCIefr21 << "0" << "0" << "0";

        strlInverseOLCIefr01 << "0" << "0" << "0";
        strlInverseOLCIefr02 << "0" << "0" << "0";
        strlInverseOLCIefr03 << "0" << "0" << "0";
        strlInverseOLCIefr04 << "0" << "0" << "0";
        strlInverseOLCIefr05 << "0" << "0" << "0";
        strlInverseOLCIefr06 << "0" << "0" << "0";
        strlInverseOLCIefr07 << "0" << "0" << "0";
        strlInverseOLCIefr08 << "0" << "0" << "0";
        strlInverseOLCIefr09 << "0" << "0" << "0";
        strlInverseOLCIefr10 << "0" << "0" << "0";
        strlInverseOLCIefr11 << "0" << "0" << "0";
        strlInverseOLCIefr12 << "0" << "0" << "0";
        strlInverseOLCIefr13 << "0" << "0" << "0";
        strlInverseOLCIefr14 << "0" << "0" << "0";
        strlInverseOLCIefr15 << "0" << "0" << "0";
        strlInverseOLCIefr16 << "0" << "0" << "0";
        strlInverseOLCIefr17 << "0" << "0" << "0";
        strlInverseOLCIefr18 << "0" << "0" << "0";
        strlInverseOLCIefr19 << "0" << "0" << "0";
        strlInverseOLCIefr20 << "0" << "0" << "0";
        strlInverseOLCIefr21 << "0" << "0" << "0";


    }
    else
    {
        strlColorBandOLCIefr = POIsettings.value("/OLCIefrConfig/strlColorBandOLCIefr").value<QStringList>();
        strlComboOLCIefr01 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr01").value<QStringList>();
        strlComboOLCIefr02 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr02").value<QStringList>();
        strlComboOLCIefr03 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr03").value<QStringList>();
        strlComboOLCIefr04 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr04").value<QStringList>();
        strlComboOLCIefr05 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr05").value<QStringList>();
        strlComboOLCIefr06 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr06").value<QStringList>();
        strlComboOLCIefr07 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr07").value<QStringList>();
        strlComboOLCIefr08 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr08").value<QStringList>();
        strlComboOLCIefr09 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr09").value<QStringList>();
        strlComboOLCIefr10 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr10").value<QStringList>();
        strlComboOLCIefr11 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr11").value<QStringList>();
        strlComboOLCIefr12 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr12").value<QStringList>();
        strlComboOLCIefr13 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr13").value<QStringList>();
        strlComboOLCIefr14 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr14").value<QStringList>();
        strlComboOLCIefr15 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr15").value<QStringList>();
        strlComboOLCIefr16 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr16").value<QStringList>();
        strlComboOLCIefr17 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr17").value<QStringList>();
        strlComboOLCIefr18 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr18").value<QStringList>();
        strlComboOLCIefr19 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr19").value<QStringList>();
        strlComboOLCIefr20 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr20").value<QStringList>();
        strlComboOLCIefr21 = POIsettings.value("/OLCIefrConfig/strlComboOLCIefr21").value<QStringList>();

        strlInverseOLCIefr01 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr01").value<QStringList>();
        strlInverseOLCIefr02 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr02").value<QStringList>();
        strlInverseOLCIefr03 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr03").value<QStringList>();
        strlInverseOLCIefr04 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr04").value<QStringList>();
        strlInverseOLCIefr05 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr05").value<QStringList>();
        strlInverseOLCIefr06 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr06").value<QStringList>();
        strlInverseOLCIefr07 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr07").value<QStringList>();
        strlInverseOLCIefr08 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr08").value<QStringList>();
        strlInverseOLCIefr09 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr09").value<QStringList>();
        strlInverseOLCIefr10 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr10").value<QStringList>();
        strlInverseOLCIefr11 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr11").value<QStringList>();
        strlInverseOLCIefr12 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr12").value<QStringList>();
        strlInverseOLCIefr13 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr13").value<QStringList>();
        strlInverseOLCIefr14 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr14").value<QStringList>();
        strlInverseOLCIefr15 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr15").value<QStringList>();
        strlInverseOLCIefr16 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr16").value<QStringList>();
        strlInverseOLCIefr17 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr17").value<QStringList>();
        strlInverseOLCIefr18 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr18").value<QStringList>();
        strlInverseOLCIefr19 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr19").value<QStringList>();
        strlInverseOLCIefr20 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr20").value<QStringList>();
        strlInverseOLCIefr21 = POIsettings.value("/OLCIefrConfig/strlInverseOLCIefr21").value<QStringList>();


    }

}


void Poi::Save()
{
    QSettings POIsettings( "POI.ini", QSettings::IniFormat);

    POIsettings.setValue("/GVP/strlName", strlGVPName );
    POIsettings.setValue("/GVP/strlLat", strlGVPLat);
    POIsettings.setValue("/GVP/strlLon", strlGVPLon);
    POIsettings.setValue("/GVP/strlscale", strlGVPScale);
    POIsettings.setValue("/GVP/strlheight", strlGVPHeight);
    POIsettings.setValue("/GVP/strlmapwidth", strlGVPMapWidth);
    POIsettings.setValue("/GVP/strlmapheight", strlGVPMapHeight);
    POIsettings.setValue("/GVP/strlgridonproj", strlGVPGridOnProj);

    POIsettings.setValue("/LCC/strlName", strlLCCName);
    POIsettings.setValue("/LCC/strlParallel1", strlLCCParallel1);
    POIsettings.setValue("/LCC/strlParallel2", strlLCCParallel2);
    POIsettings.setValue("/LCC/strlCentral", strlLCCCentral);
    POIsettings.setValue("/LCC/strlLatOrigin", strlLCCLatOrigin);
    POIsettings.setValue("/LCC/strlNorth", strlLCCNorth);
    POIsettings.setValue("/LCC/strlSouth", strlLCCSouth);
    POIsettings.setValue("/LCC/strlEast", strlLCCEast);
    POIsettings.setValue("/LCC/strlWest", strlLCCWest);
    POIsettings.setValue("/LCC/strlScaleX", strlLCCScaleX);
    POIsettings.setValue("/LCC/strlScaleY", strlLCCScaleY);
    POIsettings.setValue("/LCC/strlmapwidth", strlLCCMapWidth);
    POIsettings.setValue("/LCC/strlmapheight", strlLCCMapHeight);
    POIsettings.setValue("/LCC/strlgridonproj", strlLCCGridOnProj);

    POIsettings.setValue("/SG/strlName", strlSGName );
    POIsettings.setValue("/SG/strlLat", strlSGLat );
    POIsettings.setValue("/SG/strlLon", strlSGLon );
    POIsettings.setValue("/SG/strlRadius", strlSGRadius );
    POIsettings.setValue("/SG/strlScale", strlSGScale );
    POIsettings.setValue("/SG/strlPanH", strlSGPanH );
    POIsettings.setValue("/SG/strlPanV", strlSGPanV );
    POIsettings.setValue("/SG/strlmapwidth", strlSGMapWidth);
    POIsettings.setValue("/SG/strlmapheight", strlSGMapHeight);
    POIsettings.setValue("/SG/strlgridonproj", strlSGGridOnProj);

    POIsettings.setValue("/MConfig/strlName", strlConfigNameM);
    POIsettings.setValue("/MConfig/strlColorBandM", strlColorBandM);

    POIsettings.setValue("/MConfig/strlComboM1", strlComboM1);
    POIsettings.setValue("/MConfig/strlComboM2", strlComboM2);
    POIsettings.setValue("/MConfig/strlComboM3", strlComboM3);
    POIsettings.setValue("/MConfig/strlComboM4", strlComboM4);
    POIsettings.setValue("/MConfig/strlComboM5", strlComboM5);
    POIsettings.setValue("/MConfig/strlComboM6", strlComboM6);
    POIsettings.setValue("/MConfig/strlComboM7", strlComboM7);
    POIsettings.setValue("/MConfig/strlComboM8", strlComboM8);
    POIsettings.setValue("/MConfig/strlComboM9", strlComboM9);
    POIsettings.setValue("/MConfig/strlComboM10", strlComboM10);
    POIsettings.setValue("/MConfig/strlComboM11", strlComboM11);
    POIsettings.setValue("/MConfig/strlComboM12", strlComboM12);
    POIsettings.setValue("/MConfig/strlComboM13", strlComboM13);
    POIsettings.setValue("/MConfig/strlComboM14", strlComboM14);
    POIsettings.setValue("/MConfig/strlComboM15", strlComboM15);
    POIsettings.setValue("/MConfig/strlComboM16", strlComboM16);

    POIsettings.setValue("/MConfig/strlInverseM1", strlInverseM1);
    POIsettings.setValue("/MConfig/strlInverseM2", strlInverseM2);
    POIsettings.setValue("/MConfig/strlInverseM3", strlInverseM3);
    POIsettings.setValue("/MConfig/strlInverseM4", strlInverseM4);
    POIsettings.setValue("/MConfig/strlInverseM5", strlInverseM5);
    POIsettings.setValue("/MConfig/strlInverseM6", strlInverseM6);
    POIsettings.setValue("/MConfig/strlInverseM7", strlInverseM7);
    POIsettings.setValue("/MConfig/strlInverseM8", strlInverseM8);
    POIsettings.setValue("/MConfig/strlInverseM9", strlInverseM9);
    POIsettings.setValue("/MConfig/strlInverseM10", strlInverseM10);
    POIsettings.setValue("/MConfig/strlInverseM11", strlInverseM11);
    POIsettings.setValue("/MConfig/strlInverseM12", strlInverseM12);
    POIsettings.setValue("/MConfig/strlInverseM13", strlInverseM13);
    POIsettings.setValue("/MConfig/strlInverseM14", strlInverseM14);
    POIsettings.setValue("/MConfig/strlInverseM15", strlInverseM15);
    POIsettings.setValue("/MConfig/strlInverseM16", strlInverseM16);

    POIsettings.setValue("/OLCIefrConfig/strlName", strlConfigNameOLCIefr);
    POIsettings.setValue("/OLCIefrConfig/strlColorBandOLCIefr", strlColorBandOLCIefr);

    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr01", strlComboOLCIefr01);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr02", strlComboOLCIefr02);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr03", strlComboOLCIefr03);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr04", strlComboOLCIefr04);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr05", strlComboOLCIefr05);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr06", strlComboOLCIefr06);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr07", strlComboOLCIefr07);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr08", strlComboOLCIefr08);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr09", strlComboOLCIefr09);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr10", strlComboOLCIefr10);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr11", strlComboOLCIefr11);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr12", strlComboOLCIefr12);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr13", strlComboOLCIefr13);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr14", strlComboOLCIefr14);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr15", strlComboOLCIefr15);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr16", strlComboOLCIefr16);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr17", strlComboOLCIefr17);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr18", strlComboOLCIefr18);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr19", strlComboOLCIefr19);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr20", strlComboOLCIefr20);
    POIsettings.setValue("/OLCIefrConfig/strlComboOLCIefr21", strlComboOLCIefr21);

    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr01", strlInverseOLCIefr01);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr02", strlInverseOLCIefr02);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr03", strlInverseOLCIefr03);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr04", strlInverseOLCIefr04);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr05", strlInverseOLCIefr05);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr06", strlInverseOLCIefr06);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr07", strlInverseOLCIefr07);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr08", strlInverseOLCIefr08);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr09", strlInverseOLCIefr09);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr10", strlInverseOLCIefr10);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr11", strlInverseOLCIefr11);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr12", strlInverseOLCIefr12);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr13", strlInverseOLCIefr13);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr14", strlInverseOLCIefr14);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr15", strlInverseOLCIefr15);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr16", strlInverseOLCIefr16);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr17", strlInverseOLCIefr17);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr18", strlInverseOLCIefr18);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr19", strlInverseOLCIefr19);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr20", strlInverseOLCIefr20);
    POIsettings.setValue("/OLCIefrConfig/strlInverseOLCIefr21", strlInverseOLCIefr21);


}

