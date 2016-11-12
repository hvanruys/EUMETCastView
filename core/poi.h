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

    QStringList strlConfigNameOLCI;
    QStringList strlColorBandOLCI; // 0 = color , 1 = Oa01 , 2 = Oa02 , ...

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

    QStringList strlComboOLCI01;
    QStringList strlComboOLCI02;
    QStringList strlComboOLCI03;
    QStringList strlComboOLCI04;
    QStringList strlComboOLCI05;
    QStringList strlComboOLCI06;
    QStringList strlComboOLCI07;
    QStringList strlComboOLCI08;
    QStringList strlComboOLCI09;
    QStringList strlComboOLCI10;
    QStringList strlComboOLCI11;
    QStringList strlComboOLCI12;
    QStringList strlComboOLCI13;
    QStringList strlComboOLCI14;
    QStringList strlComboOLCI15;
    QStringList strlComboOLCI16;
    QStringList strlComboOLCI17;
    QStringList strlComboOLCI18;
    QStringList strlComboOLCI19;
    QStringList strlComboOLCI20;
    QStringList strlComboOLCI21;

    QStringList strlInverseOLCI01;
    QStringList strlInverseOLCI02;
    QStringList strlInverseOLCI03;
    QStringList strlInverseOLCI04;
    QStringList strlInverseOLCI05;
    QStringList strlInverseOLCI06;
    QStringList strlInverseOLCI07;
    QStringList strlInverseOLCI08;
    QStringList strlInverseOLCI09;
    QStringList strlInverseOLCI10;
    QStringList strlInverseOLCI11;
    QStringList strlInverseOLCI12;
    QStringList strlInverseOLCI13;
    QStringList strlInverseOLCI14;
    QStringList strlInverseOLCI15;
    QStringList strlInverseOLCI16;
    QStringList strlInverseOLCI17;
    QStringList strlInverseOLCI18;
    QStringList strlInverseOLCI19;
    QStringList strlInverseOLCI20;
    QStringList strlInverseOLCI21;

}; 


#endif
