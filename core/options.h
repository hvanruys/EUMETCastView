#ifndef OPTIONS_H
#define OPTIONS_H

#include <list>
#include <string>
#include <time.h>
#include <qstringlist.h>
#include <QColor>
#include <QRect>

#define TAB_AVHRR 0
#define TAB_VIIRS 1
#define TAB_SENTINEL 2
#define TAB_GEOSTATIONARY 3
#define TAB_PROJECTION 4
#define TAB_HISTOGRAM 5

#define TAB_LLC 0
#define TAB_GVP 1
#define TAB_GS  2

#define CMB_HISTO_NONE_95 0
#define CMB_HISTO_NONE_100 1
#define CMB_HISTO_EQUALIZE 2
#define CMB_HISTO_EQUALIZE_PROJ 3
#define CMB_HISTO_CLAHE 4

class Options 
{
public:
	Options();
	void Initialize();
    void checkStringListValues();
	void Save();

/* Calendar date and time (UTC) */
	struct tm utc;
	QStringList tlelist;
	QStringList catnbrlist;
    int zoomfactoravhrr;
    int zoomfactormeteosat;
    int zoomfactorprojection;
    int zoomfactorviirs;
    int zoomfactorolci;
    int zoomfactorslstr;
    QStringList segmentdirectorylist;
    QStringList segmentdirectorylistinc;
    bool buttonMetop;
    bool buttonNoaa;
    bool buttonGAC;
    bool buttonHRP;
    bool buttonVIIRSM;
    bool buttonVIIRSDNB;
    bool buttonOLCIefr;
    bool buttonOLCIerr;
    bool buttonSLSTR;
    bool buttonDatahubOLCIefr;
    bool buttonDatahubOLCIerr;
    bool buttonDatahubSLSTR;
    bool buttonMetopAhrpt;
    bool buttonMetopBhrpt;
    bool buttonNoaa19hrpt;
    bool buttonM01hrpt;
    bool buttonM02hrpt;


    bool buttonRealTime;
    bool buttonShowAllSegments;
    bool buttonPhong;
    int nbrofvisiblesegments;
    int realminutesshown;
    int nbrofhours;


    QStringList metop_invlist, noaa_invlist, gac_invlist, hrp_invlist;
    bool sattrackinimage;

    double getObsLat() { return(obslat); }
	void setObsLat(const double lat) { obslat = lat; }
	double getObsLon() { return(obslon); }
	void setObsLon(const double lon) { obslon = lon; }
	double getObsAlt() { return(obsalt); }
	void setObsAlt(const double alt) { obsalt = alt; }
	void deleteTleFile( QString sel );
    bool addTleFile( QString sel );

    void deleteSegmentDirectory( QString sel );
    QStringList channellistmetop;
    QStringList channellistnoaa;
    QStringList channellistgac;
    QStringList channellisthrp;

    QStringList stationlistname;
    QStringList stationlistlon;
    QStringList stationlistlat;

    QStringList geostationarylistlon;
    QStringList geostationarylistname;

    QString segmenttype;
    QString backgroundimage2D;
    QString backgroundimage3D;
    QStringList tlesources;
    QString gshhsglobe1;
    QString gshhsglobe2;
    QString gshhsglobe3;
    QString gshhsoverlay1;
    QString gshhsoverlay2;
    QString gshhsoverlay3;
    QString sathorizoncolor;
    QString sattrackcolor;
    QString satsegmentcolor;
    QString satsegmentcolorsel;
    QString globeoverlaycolor1;
    QString globeoverlaycolor2;
    QString globeoverlaycolor3;
    QString geoimageoverlaycolor1;
    QString geoimageoverlaycolor2;
    QString geoimageoverlaycolor3;
    QString olciimageoverlaycolor;
    QString globelonlatcolor;
    QString maplccextentcolor;
    QString mapgvpextentcolor;
    QString projectionoverlaycolor1;
    QString projectionoverlaycolor2;
    QString projectionoverlaycolor3;
    QString projectionoverlaylonlatcolor;
    QString equirectangulardirectory;

    QString skyboxup;
    QString skyboxdown;
    QString skyboxleft;
    QString skyboxright;
    QString skyboxfront;
    QString skyboxback;


    bool textureOn;
    bool stationnameOn;
    bool lightingOn;
    bool imageontextureOnMet;
    bool imageontextureOnAVHRR;
    bool imageontextureOnVIIRS;
    bool imageontextureOnOLCI;
    bool imageontextureOnSLSTR;
    bool windowvectors;
    bool udpmessages;
    bool gshhsglobe1On;
    bool gshhsglobe2On;
    bool gshhsglobe3On;
    bool graytextureOn;

    int currenttoolbox; // LLC - GVP - Stereographic
    int currenttabwidget; // AVHRR - VIIRS - OLCI - Geostationary - Projections
    int parallel1;
    int parallel2;
    int centralmeridian;
    int latitudeoforigin;
    int mapextentnorth;
    int mapextentsouth;
    int mapextentwest;
    int mapextenteast;
    bool mapextentlamberton;
    bool mapextentperspectiveon;
    int mapheight;
    int mapwidth;
    double mapgvplon;
    double mapgvplat;
    int mapgvpheight;
    double mapgvpscale;
    double maplccscalex;
    double maplccscaley;
    double mapsglat;
    double mapsglon;
    double mapsgscale;
    int mapsgpanvert;
    int mapsgpanhorizon;
    double mapsgradius;
    double meteosatgamma;
    double yawcorrection;
    int smoothprojectiontype;
    bool gridonprojection;
    float clahecliplimit;

    int dnbsblowerlimit;
    int dnbsbupperlimit;
    int dnbsbvalue;
    int dnbspbwindowsvalue;

    QByteArray ephemsplittersizes;

    //QByteArray mainwindowgeometry;
    //QByteArray windowstate;

    QString localdirremote;
    QString dirremote;
	double obslat, obslon, obsalt;

    int lastcomboMet006;
    int lastcomboMet008;
    int lastcomboMet016;
    int lastcomboMet039;
    int lastcomboMet062;
    int lastcomboMet073;
    int lastcomboMet087;
    int lastcomboMet097;
    int lastcomboMet108;
    int lastcomboMet120;
    int lastcomboMet134;

    bool lastinverseMet006;
    bool lastinverseMet008;
    bool lastinverseMet016;
    bool lastinverseMet039;
    bool lastinverseMet062;
    bool lastinverseMet073;
    bool lastinverseMet087;
    bool lastinverseMet097;
    bool lastinverseMet108;
    bool lastinverseMet120;
    bool lastinverseMet134;

    int lastinputprojection;
    int lastVIIRSband;

    bool colormapMagma;
    bool colormapInferno;
    bool colormapPlasma;
    bool colormapViridis;

    bool remove_OLCI_dirs;
    bool remove_SLSTR_dirs;
    bool usesaturationmask;

    // Global variable not saved
    int channelontexture;
    bool fbo_changed;
    bool texture_changed;
    bool gridonolciimage;

    QString esauser;
    QString esapassword;
    QString eumetsatuser;
    QString eumetsatpassword;
    QString productdirectory;
    bool provideresaoreumetsat; // 0 = ESA , 1 = eumetsat
    bool downloadxmlolciefr;
    bool downloadxmlolcierr;
    bool downloadxmlslstr;

}; 


#endif
