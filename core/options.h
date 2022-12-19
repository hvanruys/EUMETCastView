#ifndef OPTIONS_H
#define OPTIONS_H

#include <list>
#include <string>
#include <time.h>
#include <qstringlist.h>
#include <QColor>
#include <QRect>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#define TAB_AVHRR 0
#define TAB_VIIRS 1
#define TAB_SENTINEL 2
#define TAB_MERSI 3
#define TAB_GEOSTATIONARY 4
#define TAB_PROJECTION 5
#define TAB_HISTOGRAM 6

#define TAB_LLC 0
#define TAB_GVP 1
#define TAB_GS  2

#define CMB_HISTO_NONE_95 0
#define CMB_HISTO_NONE_100 1
#define CMB_HISTO_EQUALIZE 2
#define CMB_HISTO_EQUALIZE_PROJ 3
#define CMB_HISTO_CLAHE 4

struct GeoSatellites
{
    QString fullname;
    QString shortname;
    double longitude;
    double longitudelimit1;
    double longitudelimit2;
    QString protocol;
    bool rss;
    QString searchstring;
    int indexsearchstring;
    QString filepattern;
    int imagewidth;
    int imageheight;
    int imagewidthhrv0;
    int imageheighthrv0;
    int imagewidthhrv1;
    int imageheighthrv1;
    QStringList spectrumlist;
    QStringList spectrumvalueslist;
    int indexspectrum;
    int indexfilenbr;
    int lengthfilenbr;
    int indexdate;
    int lengthdate;
    QString spectrumhrv;
    QString spectrumvaluehrv;
    int indexspectrumhrv;
    int indexfilenbrhrv;
    int lengthfilenbrhrv;
    int indexdatehrv;
    int lengthdatehrv;
    bool color;
    bool colorhrv;
    int maxsegments;
    int maxsegmentshrv;
    int segmentlength;
    int segmentlengthhrv;
    int startsegmentnbrtype0;
    int startsegmentnbrhrvtype0;
    int startsegmentnbrtype1;
    int startsegmentnbrhrvtype1;
    int clahecontextregionx;
    int clahecontextregiony;
    bool prologfile;
    bool epilogfile;
    qlonglong coff;
    qlonglong loff;
    double cfac;
    double lfac;
    qlonglong coffhrv;
    qlonglong loffhrv;
    double cfachrv;
    double lfachrv;


};

class Options 
{
public:
	Options();
    void Initialize();
    void InitializeGeo();
    void checkStringListValues();
	void Save();
    void SaveGeoIni();
    void CreateGeoSatelliteIni();
    void CreateGeoSatelliteJson();


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
    int zoomfactormersi;
    QStringList segmentdirectorylist;
    QStringList segmentdirectorylistinc;
    bool buttonMetop;
    bool buttonNoaa;
    bool buttonGAC;
    bool buttonHRP;
    bool buttonVIIRSM;
    bool buttonVIIRSDNB;
    bool buttonVIIRSMNOAA20;
    bool buttonVIIRSDNBNOAA20;
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
    bool buttonMERSI;


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
    bool imageontextureOnMERSI;
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
    double mapgvpeasting;
    double mapgvpnorthing;
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
    QByteArray mainwindowgeometry;
    QByteArray mainwindowstate;
    int toolboxwidth;

    QString localdirremote;
    QString dirremote;
	double obslat, obslon, obsalt;

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

    QString datahubuser;
    QString datahubpassword;
    QString productdirectory;
    bool provideresaoreumetsat; // 0 = ESA , 1 = eumetsat
    bool downloadxmlolciefr;
    bool downloadxmlolcierr;
    bool downloadxmlslstr;
    bool xmllogging;
    bool bellipsoid; // for OM projection
    QList<GeoSatellites> geosatellites;
    QString appdir_env;

    //  Video form
    int threadcount;
    QStringList pathlist;
    QString pattern;
    QString singleimage;
    QString satname;
    double videogamma;
    bool rss;
    QString videogshhsoverlayfile1;
    QString videogshhsoverlayfile2;
    QString videogshhsoverlayfile3;
    bool videogshhsglobe1On;
    bool videogshhsglobe2On;
    bool videogshhsglobe3On;
    int videoresolutionheight;
    int videoresolutionwidth;
    QString dayred;
    QString daygreen;
    QString dayblue;
    bool dayredinverse;
    bool daygreeninverse;
    bool dayblueinverse;
    bool dayhrv;
    QString nightred;
    QString nightgreen;
    QString nightblue;
    bool nightredinverse;
    bool nightgreeninverse;
    bool nightblueinverse;
    int videocoff;
    int videoloff;
    double videocfac;
    double videolfac;
    int videocoffhrv;
    int videoloffhrv;
    double videocfachrv;
    double videolfachrv;
    double videosatlon;
//    double videohomelon;
//    double videohomelat;
    QString videooverlaycolor1;
    QString videooverlaycolor2;
    QString videooverlaycolor3;
    QString videooverlaygridcolor;
    bool videooverlayborder;
    bool videooverlaydate;
    int videooverlaydatefontsize;
    QString videoprojectiontype;
    double gvplatitude;
    double gvplongitude;
    double gvpscale;
    int gvpheight;
    bool gvpgridonprojection;
    double gvpfalseeasting;
    double gvpfalsenorthing;
    QString videooutputname;
    QStringList ffmpeg_options;


private:
    QJsonObject getJsonDataMet_11();
    QJsonObject getJsonDataMet_10();

};

//Met-10, 0, XRIT, H-???-??????-?????????___-?????????-0?????___-, 12, HRV, 0.6, 0.8, 1.6, 3.9, 6.2, 7.3, 8.7, 9.7, 10.8, 12.8, 13.4
//Met-9, 9.2, XRIT, H-???-??????-?????????___-?????????-0?????___-, 12, HRV, 0.6, 0.8, 1.6, 3.9, 6.2, 7.3, 8.7, 9.7, 10.8, 12.8, 13.4
//Met-8, 41, XRIT, H-???-??????-?????????___-?????????-0?????___-, 12, HRV, 0.6, 0.8, 1.6, 3.9, 6.2, 7.3, 8.7, 9.7, 10.8, 12.8, 13.4
//Electro-L2, 76.1, XRIT, H-000-GOMS2_-GOMS2_4_____-?????????-??????___-XXXXXXXXXXXX-C_, 6, 0.9, 3.8, 8.0, 9.7, 10.7, 11.9
//FY2E, 86.5, HDF5, Z_SATE_C_BABJ_ + filetiming + _O_FY2E_FDI_???_001_NOM.HDF.gz, IR1, IR2, IR3, IR4, VIS, VIS1KM
//FY2G, 104.5, HDF5, Z_SATE_C_BABJ_ + filetiming + _O_FY2G_FDI_???_001_NOM.HDF.gz, IR1, IR2, IR3, IR4, VIS, VIS1KM
//Himawari-8, 140.6, XRIT, IMG_DK01???_ + filetiming.mid(0, 11) + ?*, 14, IR1, IR2, IR3, IR4, B04, B05, B06, B09, B10, B11, B12, B14, B16, VIS
//GOES-15, 224.6, XRIT, L-???-??????-GOES15*, 4, 0.7, 3.9, 6.6, 10.7
//GOES-16, 270.5, netCDF,
//GOES-13, 284.9, XRIT, L-???-??????-GOES13*, 4, 0.7, 3.9, 6.6, 10.7



#endif
