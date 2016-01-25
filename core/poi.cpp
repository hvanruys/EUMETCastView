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

}

