#ifndef OPTIONS_H
#define OPTIONS_H

#include <list>
#include <string>
#include <time.h>
#include <qstringlist.h>
#include <QColor>
#include <QRect>


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
    QStringList segmentdirectorylist;
    QStringList segmentdirectorylistinc;
    bool buttonMetop;
    bool buttonNoaa;
    bool buttonGAC;
    bool buttonHRP;
    bool buttonVIIRS;
    bool buttonRealTime;
    bool buttonEqualization;
    int nbrofvisiblesegments;
    int realminutesshown;
    int nbrofhours;


    QStringList metop_invlist, noaa_invlist, gac_invlist, hrp_invlist, viirs_invlist;
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
    QStringList channellistviirs;

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
    QString imageoverlaycolor1;
    QString imageoverlaycolor2;
    QString imageoverlaycolor3;
    QString globelonlatcolor;
    QString maplccextentcolor;
    QString mapgvpextentcolor;
    QString projectionoverlaycolor1;
    QString projectionoverlaycolor2;
    QString projectionoverlaycolor3;
    QString projectionoverlaylonlatcolor;

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
    bool udpmessages;
    bool gshhsglobe1On;
    bool gshhsglobe2On;
    bool gshhsglobe3On;
    bool graytextureOn;

    int currenttoolbox;
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
    bool smoothprojectionimage;
    float clahecliplimit;


    QByteArray ephemsplittersizes;
    QByteArray mainwindowgeometry;
    QByteArray windowstate;
    QString localdirremote;
    QString dirremote;
	double obslat, obslon, obsalt;

    // Global variable not saved
    int channelontexture;
    bool fbo_changed;
    bool texture_changed;
    bool bPhongModel;

}; 


#endif
