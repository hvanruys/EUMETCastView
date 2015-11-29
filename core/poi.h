#ifndef POI_H
#define POI_H

#include <QStringList>


class Poi
{
public:
    Poi();
	void Initialize();
	void Save();
//    void setBandM(int index, int val);
//    void setBandMToZero(int index);

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

    QStringList strlConfigNameM;

    QStringList strlColorBandM; // 0 = color , 1 = M1 , 2 = M2 , ...

//    QStringList strlColorsM;
//    QStringList strlBandM1;
//    QStringList strlBandM2;
//    QStringList strlBandM3;
//    QStringList strlBandM4;
//    QStringList strlBandM5;
//    QStringList strlBandM6;
//    QStringList strlBandM7;
//    QStringList strlBandM8;
//    QStringList strlBandM9;
//    QStringList strlBandM10;
//    QStringList strlBandM11;
//    QStringList strlBandM12;
//    QStringList strlBandM13;
//    QStringList strlBandM14;
//    QStringList strlBandM15;
//    QStringList strlBandM16;

    QStringList strlComboM1;
    QStringList strlComboM2;
    QStringList strlComboM3;
    QStringList strlComboM4;
    QStringList strlComboM5;
    QStringList strlComboM6;
    QStringList strlComboM7;
    QStringList strlComboM8;
    QStringList strlComboM9;
    QStringList strlComboM10;
    QStringList strlComboM11;
    QStringList strlComboM12;
    QStringList strlComboM13;
    QStringList strlComboM14;
    QStringList strlComboM15;
    QStringList strlComboM16;

    QStringList strlInverseM1;
    QStringList strlInverseM2;
    QStringList strlInverseM3;
    QStringList strlInverseM4;
    QStringList strlInverseM5;
    QStringList strlInverseM6;
    QStringList strlInverseM7;
    QStringList strlInverseM8;
    QStringList strlInverseM9;
    QStringList strlInverseM10;
    QStringList strlInverseM11;
    QStringList strlInverseM12;
    QStringList strlInverseM13;
    QStringList strlInverseM14;
    QStringList strlInverseM15;
    QStringList strlInverseM16;


}; 


#endif
