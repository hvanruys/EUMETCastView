#ifndef POI_H
#define POI_H

#include <QStringList>


class Poi
{
public:
    Poi();
	void Initialize();
	void Save();

    QStringList strlGVPName;
    QStringList strlGVPLat;
    QStringList strlGVPLon;
    QStringList strlGVPScale;
    QStringList strlGVPHeight;
    QStringList strlGVPGridOnProj;
    QStringList strlGVPMapHeight;
    QStringList strlGVPMapWidth;

    QStringList strlLCCName;
    QStringList strlLCCParallel1;
    QStringList strlLCCParallel2;
    QStringList strlLCCCentral;
    QStringList strlLCCLatOrigin;
    QStringList strlLCCNorth;
    QStringList strlLCCSouth;
    QStringList strlLCCEast;
    QStringList strlLCCWest;
    QStringList strlLCCScaleX;
    QStringList strlLCCScaleY;
    QStringList strlLCCGridOnProj;
    QStringList strlLCCMapHeight;
    QStringList strlLCCMapWidth;

    QStringList strlSGName;
    QStringList strlSGLat;
    QStringList strlSGLon;
    QStringList strlSGRadius;
    QStringList strlSGScale;
    QStringList strlSGPanH;
    QStringList strlSGPanV;
    QStringList strlSGGridOnProj;
    QStringList strlSGMapHeight;
    QStringList strlSGMapWidth;

}; 


#endif
