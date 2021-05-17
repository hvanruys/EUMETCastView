#include "poi.h"
#include "options.h"
#include <QSettings>
#include <QDebug>
extern Options opts;

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
    strlConfigNameOLCI = POIsettings.value("/OLCIConfig/strlName").value<QStringList>();
    strlConfigNameSLSTR = POIsettings.value("/SLSTRConfig/strlName").value<QStringList>();
    strlConfigNameMERSI = POIsettings.value("/MERSIConfig/strlName").value<QStringList>();

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

    if(strlConfigNameOLCI.count() == 0)
    {
        strlConfigNameOLCI << "User defined" << "Natural Colors" << "True Colors";
        strlColorBandOLCI << "0" << "0" << "0"; // "0" = color , "1" = Oa01, ..

        strlComboOLCI01 << "0" << "0" << "0";
        strlComboOLCI02 << "0" << "0" << "0";
        strlComboOLCI03 << "3" << "0" << "3";
        strlComboOLCI04 << "0" << "0" << "0";
        strlComboOLCI05 << "0" << "0" << "0";
        strlComboOLCI06 << "2" << "0" << "2";
        strlComboOLCI07 << "0" << "0" << "0";
        strlComboOLCI08 << "0" << "0" << "0";
        strlComboOLCI09 << "0" << "3" << "0";
        strlComboOLCI10 << "1" << "0" << "1";
        strlComboOLCI11 << "0" << "0" << "0";
        strlComboOLCI12 << "0" << "0" << "0";
        strlComboOLCI13 << "0" << "0" << "0";
        strlComboOLCI14 << "0" << "0" << "0";
        strlComboOLCI15 << "0" << "0" << "0";
        strlComboOLCI16 << "0" << "0" << "0";
        strlComboOLCI17 << "0" << "2" << "0";
        strlComboOLCI18 << "0" << "0" << "0";
        strlComboOLCI19 << "0" << "0" << "0";
        strlComboOLCI20 << "0" << "0" << "0";
        strlComboOLCI21 << "0" << "1" << "0";

        strlInverseOLCI01 << "0" << "0" << "0";
        strlInverseOLCI02 << "0" << "0" << "0";
        strlInverseOLCI03 << "0" << "0" << "0";
        strlInverseOLCI04 << "0" << "0" << "0";
        strlInverseOLCI05 << "0" << "0" << "0";
        strlInverseOLCI06 << "0" << "0" << "0";
        strlInverseOLCI07 << "0" << "0" << "0";
        strlInverseOLCI08 << "0" << "0" << "0";
        strlInverseOLCI09 << "0" << "0" << "0";
        strlInverseOLCI10 << "0" << "0" << "0";
        strlInverseOLCI11 << "0" << "0" << "0";
        strlInverseOLCI12 << "0" << "0" << "0";
        strlInverseOLCI13 << "0" << "0" << "0";
        strlInverseOLCI14 << "0" << "0" << "0";
        strlInverseOLCI15 << "0" << "0" << "0";
        strlInverseOLCI16 << "0" << "0" << "0";
        strlInverseOLCI17 << "0" << "0" << "0";
        strlInverseOLCI18 << "0" << "0" << "0";
        strlInverseOLCI19 << "0" << "0" << "0";
        strlInverseOLCI20 << "0" << "0" << "0";
        strlInverseOLCI21 << "0" << "0" << "0";


    }
    else
    {
        strlColorBandOLCI = POIsettings.value("/OLCIConfig/strlColorBandOLCI").value<QStringList>();
        strlComboOLCI01 = POIsettings.value("/OLCIConfig/strlComboOLCI01").value<QStringList>();
        strlComboOLCI02 = POIsettings.value("/OLCIConfig/strlComboOLCI02").value<QStringList>();
        strlComboOLCI03 = POIsettings.value("/OLCIConfig/strlComboOLCI03").value<QStringList>();
        strlComboOLCI04 = POIsettings.value("/OLCIConfig/strlComboOLCI04").value<QStringList>();
        strlComboOLCI05 = POIsettings.value("/OLCIConfig/strlComboOLCI05").value<QStringList>();
        strlComboOLCI06 = POIsettings.value("/OLCIConfig/strlComboOLCI06").value<QStringList>();
        strlComboOLCI07 = POIsettings.value("/OLCIConfig/strlComboOLCI07").value<QStringList>();
        strlComboOLCI08 = POIsettings.value("/OLCIConfig/strlComboOLCI08").value<QStringList>();
        strlComboOLCI09 = POIsettings.value("/OLCIConfig/strlComboOLCI09").value<QStringList>();
        strlComboOLCI10 = POIsettings.value("/OLCIConfig/strlComboOLCI10").value<QStringList>();
        strlComboOLCI11 = POIsettings.value("/OLCIConfig/strlComboOLCI11").value<QStringList>();
        strlComboOLCI12 = POIsettings.value("/OLCIConfig/strlComboOLCI12").value<QStringList>();
        strlComboOLCI13 = POIsettings.value("/OLCIConfig/strlComboOLCI13").value<QStringList>();
        strlComboOLCI14 = POIsettings.value("/OLCIConfig/strlComboOLCI14").value<QStringList>();
        strlComboOLCI15 = POIsettings.value("/OLCIConfig/strlComboOLCI15").value<QStringList>();
        strlComboOLCI16 = POIsettings.value("/OLCIConfig/strlComboOLCI16").value<QStringList>();
        strlComboOLCI17 = POIsettings.value("/OLCIConfig/strlComboOLCI17").value<QStringList>();
        strlComboOLCI18 = POIsettings.value("/OLCIConfig/strlComboOLCI18").value<QStringList>();
        strlComboOLCI19 = POIsettings.value("/OLCIConfig/strlComboOLCI19").value<QStringList>();
        strlComboOLCI20 = POIsettings.value("/OLCIConfig/strlComboOLCI20").value<QStringList>();
        strlComboOLCI21 = POIsettings.value("/OLCIConfig/strlComboOLCI21").value<QStringList>();

        strlInverseOLCI01 = POIsettings.value("/OLCIConfig/strlInverseOLCI01").value<QStringList>();
        strlInverseOLCI02 = POIsettings.value("/OLCIConfig/strlInverseOLCI02").value<QStringList>();
        strlInverseOLCI03 = POIsettings.value("/OLCIConfig/strlInverseOLCI03").value<QStringList>();
        strlInverseOLCI04 = POIsettings.value("/OLCIConfig/strlInverseOLCI04").value<QStringList>();
        strlInverseOLCI05 = POIsettings.value("/OLCIConfig/strlInverseOLCI05").value<QStringList>();
        strlInverseOLCI06 = POIsettings.value("/OLCIConfig/strlInverseOLCI06").value<QStringList>();
        strlInverseOLCI07 = POIsettings.value("/OLCIConfig/strlInverseOLCI07").value<QStringList>();
        strlInverseOLCI08 = POIsettings.value("/OLCIConfig/strlInverseOLCI08").value<QStringList>();
        strlInverseOLCI09 = POIsettings.value("/OLCIConfig/strlInverseOLCI09").value<QStringList>();
        strlInverseOLCI10 = POIsettings.value("/OLCIConfig/strlInverseOLCI10").value<QStringList>();
        strlInverseOLCI11 = POIsettings.value("/OLCIConfig/strlInverseOLCI11").value<QStringList>();
        strlInverseOLCI12 = POIsettings.value("/OLCIConfig/strlInverseOLCI12").value<QStringList>();
        strlInverseOLCI13 = POIsettings.value("/OLCIConfig/strlInverseOLCI13").value<QStringList>();
        strlInverseOLCI14 = POIsettings.value("/OLCIConfig/strlInverseOLCI14").value<QStringList>();
        strlInverseOLCI15 = POIsettings.value("/OLCIConfig/strlInverseOLCI15").value<QStringList>();
        strlInverseOLCI16 = POIsettings.value("/OLCIConfig/strlInverseOLCI16").value<QStringList>();
        strlInverseOLCI17 = POIsettings.value("/OLCIConfig/strlInverseOLCI17").value<QStringList>();
        strlInverseOLCI18 = POIsettings.value("/OLCIConfig/strlInverseOLCI18").value<QStringList>();
        strlInverseOLCI19 = POIsettings.value("/OLCIConfig/strlInverseOLCI19").value<QStringList>();
        strlInverseOLCI20 = POIsettings.value("/OLCIConfig/strlInverseOLCI20").value<QStringList>();
        strlInverseOLCI21 = POIsettings.value("/OLCIConfig/strlInverseOLCI21").value<QStringList>();
    }

    if(strlConfigNameSLSTR.count() == 0)
    {
        strlConfigNameSLSTR << "User defined" << "Natural Colors" << "False Colors";
        strlColorBandSLSTR << "0" << "0" << "0"; // "0" = color , "1" = S1, ..

        strlComboSLSTRS1 << "3" << "3" << "0";
        strlComboSLSTRS2 << "2" << "2" << "3";
        strlComboSLSTRS3 << "1" << "1" << "2";
        strlComboSLSTRS4 << "0" << "0" << "0";
        strlComboSLSTRS5 << "0" << "0" << "1";
        strlComboSLSTRS6 << "0" << "0" << "0";
        strlComboSLSTRS7 << "0" << "0" << "0";
        strlComboSLSTRS8 << "0" << "0" << "0";
        strlComboSLSTRS9 << "0" << "0" << "0";
        strlComboSLSTRF1 << "0" << "0" << "0";
        strlComboSLSTRF2 << "0" << "0" << "0";

        strlInverseSLSTRS1 << "0" << "0" << "0";
        strlInverseSLSTRS2 << "0" << "0" << "0";
        strlInverseSLSTRS3 << "0" << "0" << "0";
        strlInverseSLSTRS4 << "0" << "0" << "0";
        strlInverseSLSTRS5 << "0" << "0" << "0";
        strlInverseSLSTRS6 << "0" << "0" << "0";
        strlInverseSLSTRS7 << "0" << "0" << "0";
        strlInverseSLSTRS8 << "0" << "0" << "0";
        strlInverseSLSTRS9 << "0" << "0" << "0";
        strlInverseSLSTRF1 << "0" << "0" << "0";
        strlInverseSLSTRF2 << "0" << "0" << "0";
    }
    else
    {
        strlColorBandSLSTR = POIsettings.value("/SLSTRConfig/strlColorBandSLSTR").value<QStringList>();
        strlComboSLSTRS1 = POIsettings.value("/SLSTRConfig/strlComboSLSTRS1").value<QStringList>();
        strlComboSLSTRS2 = POIsettings.value("/SLSTRConfig/strlComboSLSTRS2").value<QStringList>();
        strlComboSLSTRS3 = POIsettings.value("/SLSTRConfig/strlComboSLSTRS3").value<QStringList>();
        strlComboSLSTRS4 = POIsettings.value("/SLSTRConfig/strlComboSLSTRS4").value<QStringList>();
        strlComboSLSTRS5 = POIsettings.value("/SLSTRConfig/strlComboSLSTRS5").value<QStringList>();
        strlComboSLSTRS6 = POIsettings.value("/SLSTRConfig/strlComboSLSTRS6").value<QStringList>();
        strlComboSLSTRS7 = POIsettings.value("/SLSTRConfig/strlComboSLSTRS7").value<QStringList>();
        strlComboSLSTRS8 = POIsettings.value("/SLSTRConfig/strlComboSLSTRS8").value<QStringList>();
        strlComboSLSTRS9 = POIsettings.value("/SLSTRConfig/strlComboSLSTRS9").value<QStringList>();
        strlComboSLSTRF1 = POIsettings.value("/SLSTRConfig/strlComboSLSTRF1").value<QStringList>();
        strlComboSLSTRF2 = POIsettings.value("/SLSTRConfig/strlComboSLSTRF2").value<QStringList>();

        strlInverseSLSTRS1 = POIsettings.value("/SLSTRConfig/strlInverseSLSTRS1").value<QStringList>();
        strlInverseSLSTRS2 = POIsettings.value("/SLSTRConfig/strlInverseSLSTRS2").value<QStringList>();
        strlInverseSLSTRS3 = POIsettings.value("/SLSTRConfig/strlInverseSLSTRS3").value<QStringList>();
        strlInverseSLSTRS4 = POIsettings.value("/SLSTRConfig/strlInverseSLSTRS4").value<QStringList>();
        strlInverseSLSTRS5 = POIsettings.value("/SLSTRConfig/strlInverseSLSTRS5").value<QStringList>();
        strlInverseSLSTRS6 = POIsettings.value("/SLSTRConfig/strlInverseSLSTRS6").value<QStringList>();
        strlInverseSLSTRS7 = POIsettings.value("/SLSTRConfig/strlInverseSLSTRS7").value<QStringList>();
        strlInverseSLSTRS8 = POIsettings.value("/SLSTRConfig/strlInverseSLSTRS8").value<QStringList>();
        strlInverseSLSTRS9 = POIsettings.value("/SLSTRConfig/strlInverseSLSTRS9").value<QStringList>();
        strlInverseSLSTRF1 = POIsettings.value("/SLSTRConfig/strlInverseSLSTRF1").value<QStringList>();
        strlInverseSLSTRF2 = POIsettings.value("/SLSTRConfig/strlInverseSLSTRF2").value<QStringList>();

    }

    strlComboGeo1 = POIsettings.value("/GeoConfig/strlComboGeo1").value<QStringList>();

    // MET_11, MET_10, MET_9, MET_8, GOMS2, FY2H, FY2G, GOES_15, GOES_16, GOES_17, H8
    if(strlComboGeo1.count() != opts.geosatellites.count())
    {
        strlComboGeo1.clear();
        strlComboGeo2.clear();
        strlComboGeo3.clear();
        strlComboGeo4.clear();
        strlComboGeo5.clear();
        strlComboGeo6.clear();
        strlComboGeo7.clear();
        strlComboGeo8.clear();
        strlComboGeo9.clear();
        strlComboGeo10.clear();
        strlComboGeo11.clear();
        strlComboGeo12.clear();
        strlComboGeo13.clear();
        strlComboGeo14.clear();
        strlComboGeo15.clear();
        strlComboGeo16.clear();

        strlInverseGeo1.clear();
        strlInverseGeo2.clear();
        strlInverseGeo3.clear();
        strlInverseGeo4.clear();
        strlInverseGeo5.clear();
        strlInverseGeo6.clear();
        strlInverseGeo7.clear();
        strlInverseGeo8.clear();
        strlInverseGeo9.clear();
        strlInverseGeo10.clear();
        strlInverseGeo11.clear();
        strlInverseGeo12.clear();
        strlInverseGeo13.clear();
        strlInverseGeo14.clear();
        strlInverseGeo15.clear();
        strlInverseGeo16.clear();

        strlComboGeo1 << "3" << "3" << "3" << "3" << "3" << "3" << "3" << "3" << "3" << "3" << "0";
        strlComboGeo2 << "2" << "2" << "2" << "2" << "2" << "2" << "2" << "2" << "1" << "1" << "3";
        strlComboGeo3 << "1" << "1" << "1" << "1" << "1" << "1" << "1" << "1" << "2" << "2" << "2";
        strlComboGeo4 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlComboGeo5 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "1" << "0" << "1";
        strlComboGeo6 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlComboGeo7 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlComboGeo8 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlComboGeo9 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlComboGeo10 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlComboGeo11 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlComboGeo12 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlComboGeo13 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlComboGeo14 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlComboGeo15 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlComboGeo16 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";

        strlInverseGeo1 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo2 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo3 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo4 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo5 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo6 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo7 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo8 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo9 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo10 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo11 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo12 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo13 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo14 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo15 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";
        strlInverseGeo16 << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0";

    }
    else
    {

        strlComboGeo2 = POIsettings.value("/GeoConfig/strlComboGeo2").value<QStringList>();
        strlComboGeo3 = POIsettings.value("/GeoConfig/strlComboGeo3").value<QStringList>();
        strlComboGeo4 = POIsettings.value("/GeoConfig/strlComboGeo4").value<QStringList>();
        strlComboGeo5 = POIsettings.value("/GeoConfig/strlComboGeo5").value<QStringList>();
        strlComboGeo6 = POIsettings.value("/GeoConfig/strlComboGeo6").value<QStringList>();
        strlComboGeo7 = POIsettings.value("/GeoConfig/strlComboGeo7").value<QStringList>();
        strlComboGeo8 = POIsettings.value("/GeoConfig/strlComboGeo8").value<QStringList>();
        strlComboGeo9 = POIsettings.value("/GeoConfig/strlComboGeo9").value<QStringList>();
        strlComboGeo10 = POIsettings.value("/GeoConfig/strlComboGeo10").value<QStringList>();
        strlComboGeo11 = POIsettings.value("/GeoConfig/strlComboGeo11").value<QStringList>();
        strlComboGeo12 = POIsettings.value("/GeoConfig/strlComboGeo12").value<QStringList>();
        strlComboGeo13 = POIsettings.value("/GeoConfig/strlComboGeo13").value<QStringList>();
        strlComboGeo14 = POIsettings.value("/GeoConfig/strlComboGeo14").value<QStringList>();
        strlComboGeo15 = POIsettings.value("/GeoConfig/strlComboGeo15").value<QStringList>();
        strlComboGeo16 = POIsettings.value("/GeoConfig/strlComboGeo16").value<QStringList>();

        strlInverseGeo1 = POIsettings.value("/GeoConfig/strlInverseGeo1").value<QStringList>();
        strlInverseGeo2 = POIsettings.value("/GeoConfig/strlInverseGeo2").value<QStringList>();
        strlInverseGeo3 = POIsettings.value("/GeoConfig/strlInverseGeo3").value<QStringList>();
        strlInverseGeo4 = POIsettings.value("/GeoConfig/strlInverseGeo4").value<QStringList>();
        strlInverseGeo5 = POIsettings.value("/GeoConfig/strlInverseGeo5").value<QStringList>();
        strlInverseGeo6 = POIsettings.value("/GeoConfig/strlInverseGeo6").value<QStringList>();
        strlInverseGeo7 = POIsettings.value("/GeoConfig/strlInverseGeo7").value<QStringList>();
        strlInverseGeo8 = POIsettings.value("/GeoConfig/strlInverseGeo8").value<QStringList>();
        strlInverseGeo9 = POIsettings.value("/GeoConfig/strlInverseGeo9").value<QStringList>();
        strlInverseGeo10 = POIsettings.value("/GeoConfig/strlInverseGeo10").value<QStringList>();
        strlInverseGeo11 = POIsettings.value("/GeoConfig/strlInverseGeo11").value<QStringList>();
        strlInverseGeo12 = POIsettings.value("/GeoConfig/strlInverseGeo12").value<QStringList>();
        strlInverseGeo13 = POIsettings.value("/GeoConfig/strlInverseGeo13").value<QStringList>();
        strlInverseGeo14 = POIsettings.value("/GeoConfig/strlInverseGeo14").value<QStringList>();
        strlInverseGeo15 = POIsettings.value("/GeoConfig/strlInverseGeo15").value<QStringList>();
        strlInverseGeo16 = POIsettings.value("/GeoConfig/strlInverseGeo16").value<QStringList>();

    }

    if(strlConfigNameMERSI.count() == 0)
    {
        strlConfigNameMERSI << "User defined" << "Natural Colors" << "True Colors";
        strlColorBandMERSI << "0" << "0" << "0"; // "0" = color , "1" = Oa01, ..

        strlComboMERSI5 << "0" << "0" << "0";
        strlComboMERSI6 << "2" << "1" << "0";
        strlComboMERSI7 << "0" << "0" << "0";
        strlComboMERSI8 << "0" << "0" << "0";
        strlComboMERSI9 << "0" << "0" << "0";
        strlComboMERSI10 << "1" << "0" << "3";
        strlComboMERSI11 << "0" << "0" << "2";
        strlComboMERSI12 << "0" << "3" << "1";
        strlComboMERSI13 << "0" << "0" << "0";
        strlComboMERSI14 << "0" << "0" << "0";
        strlComboMERSI15 << "0" << "2" << "0";
        strlComboMERSI16 << "0" << "0" << "0";
        strlComboMERSI17 << "0" << "0" << "0";
        strlComboMERSI18 << "0" << "0" << "0";
        strlComboMERSI19 << "0" << "0" << "0";

        strlInverseMERSI5 << "0" << "0" << "0";
        strlInverseMERSI6 << "0" << "0" << "0";
        strlInverseMERSI7 << "0" << "0" << "0";
        strlInverseMERSI8 << "0" << "0" << "0";
        strlInverseMERSI9 << "0" << "0" << "0";
        strlInverseMERSI10 << "0" << "0" << "0";
        strlInverseMERSI11 << "0" << "0" << "0";
        strlInverseMERSI12 << "0" << "0" << "0";
        strlInverseMERSI13 << "0" << "0" << "0";
        strlInverseMERSI14 << "0" << "0" << "0";
        strlInverseMERSI15 << "0" << "0" << "0";
        strlInverseMERSI16 << "0" << "0" << "0";
        strlInverseMERSI17 << "0" << "0" << "0";
        strlInverseMERSI18 << "0" << "0" << "0";
        strlInverseMERSI19 << "0" << "0" << "0";

    }
    else
    {
        strlColorBandMERSI = POIsettings.value("/MERSIConfig/strlColorBandMERSI").value<QStringList>();
        strlComboMERSI5 = POIsettings.value("/MERSIConfig/strlComboMERSI5").value<QStringList>();
        strlComboMERSI6 = POIsettings.value("/MERSIConfig/strlComboMERSI6").value<QStringList>();
        strlComboMERSI7 = POIsettings.value("/MERSIConfig/strlComboMERSI7").value<QStringList>();
        strlComboMERSI8 = POIsettings.value("/MERSIConfig/strlComboMERSI8").value<QStringList>();
        strlComboMERSI9 = POIsettings.value("/MERSIConfig/strlComboMERSI9").value<QStringList>();
        strlComboMERSI10 = POIsettings.value("/MERSIConfig/strlComboMERSI10").value<QStringList>();
        strlComboMERSI11 = POIsettings.value("/MERSIConfig/strlComboMERSI11").value<QStringList>();
        strlComboMERSI12 = POIsettings.value("/MERSIConfig/strlComboMERSI12").value<QStringList>();
        strlComboMERSI13 = POIsettings.value("/MERSIConfig/strlComboMERSI13").value<QStringList>();
        strlComboMERSI14 = POIsettings.value("/MERSIConfig/strlComboMERSI14").value<QStringList>();
        strlComboMERSI15 = POIsettings.value("/MERSIConfig/strlComboMERSI15").value<QStringList>();
        strlComboMERSI16 = POIsettings.value("/MERSIConfig/strlComboMERSI16").value<QStringList>();
        strlComboMERSI17 = POIsettings.value("/MERSIConfig/strlComboMERSI17").value<QStringList>();
        strlComboMERSI18 = POIsettings.value("/MERSIConfig/strlComboMERSI18").value<QStringList>();
        strlComboMERSI19 = POIsettings.value("/MERSIConfig/strlComboMERSI19").value<QStringList>();

        strlInverseMERSI5 = POIsettings.value("/MERSIConfig/strlInverseMERSI5").value<QStringList>();
        strlInverseMERSI6 = POIsettings.value("/MERSIConfig/strlInverseMERSI6").value<QStringList>();
        strlInverseMERSI7 = POIsettings.value("/MERSIConfig/strlInverseMERSI7").value<QStringList>();
        strlInverseMERSI8 = POIsettings.value("/MERSIConfig/strlInverseMERSI8").value<QStringList>();
        strlInverseMERSI9 = POIsettings.value("/MERSIConfig/strlInverseMERSI9").value<QStringList>();
        strlInverseMERSI10 = POIsettings.value("/MERSIConfig/strlInverseMERSI10").value<QStringList>();
        strlInverseMERSI11 = POIsettings.value("/MERSIConfig/strlInverseMERSI11").value<QStringList>();
        strlInverseMERSI12 = POIsettings.value("/MERSIConfig/strlInverseMERSI12").value<QStringList>();
        strlInverseMERSI13 = POIsettings.value("/MERSIConfig/strlInverseMERSI13").value<QStringList>();
        strlInverseMERSI14 = POIsettings.value("/MERSIConfig/strlInverseMERSI14").value<QStringList>();
        strlInverseMERSI15 = POIsettings.value("/MERSIConfig/strlInverseMERSI15").value<QStringList>();
        strlInverseMERSI16 = POIsettings.value("/MERSIConfig/strlInverseMERSI16").value<QStringList>();
        strlInverseMERSI17 = POIsettings.value("/MERSIConfig/strlInverseMERSI17").value<QStringList>();
        strlInverseMERSI18 = POIsettings.value("/MERSIConfig/strlInverseMERSI18").value<QStringList>();
        strlInverseMERSI19 = POIsettings.value("/MERSIConfig/strlInverseMERSI19").value<QStringList>();
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

    POIsettings.setValue("/OLCIConfig/strlName", strlConfigNameOLCI);
    POIsettings.setValue("/OLCIConfig/strlColorBandOLCI", strlColorBandOLCI);

    POIsettings.setValue("/OLCIConfig/strlComboOLCI01", strlComboOLCI01);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI02", strlComboOLCI02);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI03", strlComboOLCI03);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI04", strlComboOLCI04);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI05", strlComboOLCI05);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI06", strlComboOLCI06);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI07", strlComboOLCI07);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI08", strlComboOLCI08);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI09", strlComboOLCI09);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI10", strlComboOLCI10);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI11", strlComboOLCI11);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI12", strlComboOLCI12);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI13", strlComboOLCI13);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI14", strlComboOLCI14);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI15", strlComboOLCI15);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI16", strlComboOLCI16);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI17", strlComboOLCI17);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI18", strlComboOLCI18);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI19", strlComboOLCI19);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI20", strlComboOLCI20);
    POIsettings.setValue("/OLCIConfig/strlComboOLCI21", strlComboOLCI21);

    POIsettings.setValue("/OLCIConfig/strlInverseOLCI01", strlInverseOLCI01);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI02", strlInverseOLCI02);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI03", strlInverseOLCI03);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI04", strlInverseOLCI04);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI05", strlInverseOLCI05);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI06", strlInverseOLCI06);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI07", strlInverseOLCI07);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI08", strlInverseOLCI08);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI09", strlInverseOLCI09);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI10", strlInverseOLCI10);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI11", strlInverseOLCI11);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI12", strlInverseOLCI12);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI13", strlInverseOLCI13);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI14", strlInverseOLCI14);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI15", strlInverseOLCI15);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI16", strlInverseOLCI16);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI17", strlInverseOLCI17);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI18", strlInverseOLCI18);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI19", strlInverseOLCI19);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI20", strlInverseOLCI20);
    POIsettings.setValue("/OLCIConfig/strlInverseOLCI21", strlInverseOLCI21);

    POIsettings.setValue("/SLSTRConfig/strlName", strlConfigNameSLSTR);
    POIsettings.setValue("/SLSTRConfig/strlColorBandSLSTR", strlColorBandSLSTR);

    POIsettings.setValue("/SLSTRConfig/strlComboSLSTRS1", strlComboSLSTRS1);
    POIsettings.setValue("/SLSTRConfig/strlComboSLSTRS2", strlComboSLSTRS2);
    POIsettings.setValue("/SLSTRConfig/strlComboSLSTRS3", strlComboSLSTRS3);
    POIsettings.setValue("/SLSTRConfig/strlComboSLSTRS4", strlComboSLSTRS4);
    POIsettings.setValue("/SLSTRConfig/strlComboSLSTRS5", strlComboSLSTRS5);
    POIsettings.setValue("/SLSTRConfig/strlComboSLSTRS6", strlComboSLSTRS6);
    POIsettings.setValue("/SLSTRConfig/strlComboSLSTRS7", strlComboSLSTRS7);
    POIsettings.setValue("/SLSTRConfig/strlComboSLSTRS8", strlComboSLSTRS8);
    POIsettings.setValue("/SLSTRConfig/strlComboSLSTRS9", strlComboSLSTRS9);
    POIsettings.setValue("/SLSTRConfig/strlComboSLSTRF1", strlComboSLSTRF1);
    POIsettings.setValue("/SLSTRConfig/strlComboSLSTRF2", strlComboSLSTRF2);

    POIsettings.setValue("/SLSTRConfig/strlInverseSLSTRS1", strlInverseSLSTRS1);
    POIsettings.setValue("/SLSTRConfig/strlInverseSLSTRS2", strlInverseSLSTRS2);
    POIsettings.setValue("/SLSTRConfig/strlInverseSLSTRS3", strlInverseSLSTRS3);
    POIsettings.setValue("/SLSTRConfig/strlInverseSLSTRS4", strlInverseSLSTRS4);
    POIsettings.setValue("/SLSTRConfig/strlInverseSLSTRS5", strlInverseSLSTRS5);
    POIsettings.setValue("/SLSTRConfig/strlInverseSLSTRS6", strlInverseSLSTRS6);
    POIsettings.setValue("/SLSTRConfig/strlInverseSLSTRS7", strlInverseSLSTRS7);
    POIsettings.setValue("/SLSTRConfig/strlInverseSLSTRS8", strlInverseSLSTRS8);
    POIsettings.setValue("/SLSTRConfig/strlInverseSLSTRS9", strlInverseSLSTRS9);
    POIsettings.setValue("/SLSTRConfig/strlInverseSLSTRF1", strlInverseSLSTRF1);
    POIsettings.setValue("/SLSTRConfig/strlInverseSLSTRF2", strlInverseSLSTRF2);

    POIsettings.setValue("/GeoConfig/strlComboGeo1", strlComboGeo1);
    POIsettings.setValue("/GeoConfig/strlComboGeo2", strlComboGeo2);
    POIsettings.setValue("/GeoConfig/strlComboGeo3", strlComboGeo3);
    POIsettings.setValue("/GeoConfig/strlComboGeo4", strlComboGeo4);
    POIsettings.setValue("/GeoConfig/strlComboGeo5", strlComboGeo5);
    POIsettings.setValue("/GeoConfig/strlComboGeo6", strlComboGeo6);
    POIsettings.setValue("/GeoConfig/strlComboGeo7", strlComboGeo7);
    POIsettings.setValue("/GeoConfig/strlComboGeo8", strlComboGeo8);
    POIsettings.setValue("/GeoConfig/strlComboGeo9", strlComboGeo9);
    POIsettings.setValue("/GeoConfig/strlComboGeo10", strlComboGeo10);
    POIsettings.setValue("/GeoConfig/strlComboGeo11", strlComboGeo11);
    POIsettings.setValue("/GeoConfig/strlComboGeo12", strlComboGeo12);
    POIsettings.setValue("/GeoConfig/strlComboGeo13", strlComboGeo13);
    POIsettings.setValue("/GeoConfig/strlComboGeo14", strlComboGeo14);
    POIsettings.setValue("/GeoConfig/strlComboGeo15", strlComboGeo15);
    POIsettings.setValue("/GeoConfig/strlComboGeo16", strlComboGeo16);

    POIsettings.setValue("/GeoConfig/strlInverseGeo1", strlInverseGeo1);
    POIsettings.setValue("/GeoConfig/strlInverseGeo2", strlInverseGeo2);
    POIsettings.setValue("/GeoConfig/strlInverseGeo3", strlInverseGeo3);
    POIsettings.setValue("/GeoConfig/strlInverseGeo4", strlInverseGeo4);
    POIsettings.setValue("/GeoConfig/strlInverseGeo5", strlInverseGeo5);
    POIsettings.setValue("/GeoConfig/strlInverseGeo6", strlInverseGeo6);
    POIsettings.setValue("/GeoConfig/strlInverseGeo7", strlInverseGeo7);
    POIsettings.setValue("/GeoConfig/strlInverseGeo8", strlInverseGeo8);
    POIsettings.setValue("/GeoConfig/strlInverseGeo9", strlInverseGeo9);
    POIsettings.setValue("/GeoConfig/strlInverseGeo10", strlInverseGeo10);
    POIsettings.setValue("/GeoConfig/strlInverseGeo11", strlInverseGeo11);
    POIsettings.setValue("/GeoConfig/strlInverseGeo12", strlInverseGeo12);
    POIsettings.setValue("/GeoConfig/strlInverseGeo13", strlInverseGeo13);
    POIsettings.setValue("/GeoConfig/strlInverseGeo14", strlInverseGeo14);
    POIsettings.setValue("/GeoConfig/strlInverseGeo15", strlInverseGeo15);
    POIsettings.setValue("/GeoConfig/strlInverseGeo16", strlInverseGeo16);

    POIsettings.setValue("/MERSIConfig/strlName", strlConfigNameMERSI);
    POIsettings.setValue("/MERSIConfig/strlColorBandMERSI", strlColorBandMERSI);

    POIsettings.setValue("/MERSIConfig/strlComboMERSI5", strlComboMERSI5);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI6", strlComboMERSI6);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI7", strlComboMERSI7);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI8", strlComboMERSI8);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI9", strlComboMERSI9);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI10", strlComboMERSI10);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI11", strlComboMERSI11);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI12", strlComboMERSI12);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI13", strlComboMERSI13);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI14", strlComboMERSI14);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI15", strlComboMERSI15);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI16", strlComboMERSI16);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI17", strlComboMERSI17);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI18", strlComboMERSI18);
    POIsettings.setValue("/MERSIConfig/strlComboMERSI19", strlComboMERSI19);

    POIsettings.setValue("/MERSIConfig/strlInverseMERSI5", strlInverseMERSI5);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI6", strlInverseMERSI6);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI7", strlInverseMERSI7);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI8", strlInverseMERSI8);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI9", strlInverseMERSI9);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI10", strlInverseMERSI10);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI11", strlInverseMERSI11);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI12", strlInverseMERSI12);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI13", strlInverseMERSI13);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI14", strlInverseMERSI14);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI15", strlInverseMERSI15);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI16", strlInverseMERSI16);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI17", strlInverseMERSI17);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI18", strlInverseMERSI18);
    POIsettings.setValue("/MERSIConfig/strlInverseMERSI19", strlInverseMERSI19);


}

