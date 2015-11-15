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

}

