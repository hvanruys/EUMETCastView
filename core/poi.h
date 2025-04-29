#ifndef POI_H
#define POI_H

#include <QStringList>


class Poi
{
public:
    Poi();
	void Initialize();
	void Save();
    void FillStrlGeo(QStringList strl, QString val, int nbrgeosats);

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
    QStringList strlGVPFalseEasting;
    QStringList strlGVPFalseNorthing;

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

    QStringList strlConfigNameMERSI;
    QStringList strlColorBandMERSI; // 0 = color , 1 = Band 5 , 2 = Band 6 , ...

    QStringList strlConfigNameSLSTR;
    QStringList strlColorBandSLSTR; // 0 = color , 1 = S1 , 2 = S2 , ...

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

    QStringList strlComboSLSTRS1;
    QStringList strlComboSLSTRS2;
    QStringList strlComboSLSTRS3;
    QStringList strlComboSLSTRS4;
    QStringList strlComboSLSTRS5;
    QStringList strlComboSLSTRS6;
    QStringList strlComboSLSTRS7;
    QStringList strlComboSLSTRS8;
    QStringList strlComboSLSTRS9;
    QStringList strlComboSLSTRF1;
    QStringList strlComboSLSTRF2;

    QStringList strlInverseSLSTRS1;
    QStringList strlInverseSLSTRS2;
    QStringList strlInverseSLSTRS3;
    QStringList strlInverseSLSTRS4;
    QStringList strlInverseSLSTRS5;
    QStringList strlInverseSLSTRS6;
    QStringList strlInverseSLSTRS7;
    QStringList strlInverseSLSTRS8;
    QStringList strlInverseSLSTRS9;
    QStringList strlInverseSLSTRF1;
    QStringList strlInverseSLSTRF2;

    QStringList strlComboGeo1;
    QStringList strlComboGeo2;
    QStringList strlComboGeo3;
    QStringList strlComboGeo4;
    QStringList strlComboGeo5;
    QStringList strlComboGeo6;
    QStringList strlComboGeo7;
    QStringList strlComboGeo8;
    QStringList strlComboGeo9;
    QStringList strlComboGeo10;
    QStringList strlComboGeo11;
    QStringList strlComboGeo12;
    QStringList strlComboGeo13;
    QStringList strlComboGeo14;
    QStringList strlComboGeo15;
    QStringList strlComboGeo16;

    QStringList strlInverseGeo1;
    QStringList strlInverseGeo2;
    QStringList strlInverseGeo3;
    QStringList strlInverseGeo4;
    QStringList strlInverseGeo5;
    QStringList strlInverseGeo6;
    QStringList strlInverseGeo7;
    QStringList strlInverseGeo8;
    QStringList strlInverseGeo9;
    QStringList strlInverseGeo10;
    QStringList strlInverseGeo11;
    QStringList strlInverseGeo12;
    QStringList strlInverseGeo13;
    QStringList strlInverseGeo14;
    QStringList strlInverseGeo15;
    QStringList strlInverseGeo16;

    QStringList strlComboMERSI5;
    QStringList strlComboMERSI6;
    QStringList strlComboMERSI7;
    QStringList strlComboMERSI8;
    QStringList strlComboMERSI9;
    QStringList strlComboMERSI10;
    QStringList strlComboMERSI11;
    QStringList strlComboMERSI12;
    QStringList strlComboMERSI13;
    QStringList strlComboMERSI14;
    QStringList strlComboMERSI15;
    QStringList strlComboMERSI16;
    QStringList strlComboMERSI17;
    QStringList strlComboMERSI18;
    QStringList strlComboMERSI19;

    QStringList strlInverseMERSI5;
    QStringList strlInverseMERSI6;
    QStringList strlInverseMERSI7;
    QStringList strlInverseMERSI8;
    QStringList strlInverseMERSI9;
    QStringList strlInverseMERSI10;
    QStringList strlInverseMERSI11;
    QStringList strlInverseMERSI12;
    QStringList strlInverseMERSI13;
    QStringList strlInverseMERSI14;
    QStringList strlInverseMERSI15;
    QStringList strlInverseMERSI16;
    QStringList strlInverseMERSI17;
    QStringList strlInverseMERSI18;
    QStringList strlInverseMERSI19;

}; 


#endif
