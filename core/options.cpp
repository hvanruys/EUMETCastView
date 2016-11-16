#include "options.h"
//#include "sgp4sdp4.h"

#include <qsettings.h>
#include <QDebug>



Options::Options() 
{
    qDebug() << "Initializing options";
}

void Options::Initialize()
{
    qDebug() << "Options::Initialize()";

    QSettings settings( "EUMETCastView.ini", QSettings::IniFormat);
    obslon = settings.value("/observer/longitude", 0.0 ).toDouble();
    obslat = settings.value("/observer/latitude", 0.0 ).toDouble();
    obsalt = settings.value( "/observer/altitude", 0.0 ).toDouble();
    buttonMetop=settings.value("/window/buttonmetop", false ).toBool();
    buttonNoaa=settings.value("/window/buttonnoaa", false ).toBool();
    buttonGAC=settings.value("/window/buttongac", false ).toBool();
    buttonHRP=settings.value("/window/buttonhrp", false ).toBool();
    buttonVIIRSM=settings.value("/window/buttonviirs", false ).toBool();
    buttonVIIRSDNB=settings.value("/window/buttonviirsdnb", false ).toBool();
    buttonOLCIefr=settings.value("/window/buttonolciefr", false ).toBool();
    buttonOLCIerr=settings.value("/window/buttonolcierr", false ).toBool();

    buttonRealTime=settings.value("/window/buttonrealtime", true ).toBool();
    buttonEqualization=settings.value("/window/buttonequalization", false ).toBool();
    nbrofvisiblesegments=settings.value("/segments/nbrofvisiblesegments", 10).toInt();
    nbrofhours=settings.value("/segments/nbrofhours", 0).toInt();

    metop_invlist=settings.value("/segments/metop_invlist").value<QStringList>();
    noaa_invlist=settings.value("/segments/noaa_invlist").value<QStringList>();
    gac_invlist=settings.value("/segments/gac_invlist").value<QStringList>();
    hrp_invlist=settings.value("/segments/hrp_invlist").value<QStringList>();

    sattrackinimage=settings.value("/segments/sattrackinimage", false ).toBool();

    tlelist = settings.value("/satellite/tlefiles").value<QStringList>();
    QStringList catnbrlistcop;
    catnbrlistcop = settings.value( "/satellite/catnrs" ).value<QStringList>();

    bool found;
    bool ok;

    for ( QStringList::Iterator itcop = catnbrlistcop.begin(); itcop != catnbrlistcop.end(); ++itcop )
    {
        if( catnbrlist.count() == 0)
            catnbrlist.append(*itcop);
        else
        {
            found = false;
            for ( QStringList::Iterator itc = catnbrlist.begin(); itc != catnbrlist.end(); ++itc )
            {
                if ( (*itcop).toInt( &ok, 10) == (*itc).toInt( &ok, 10 ) )
                {
                    found = true;
                    break;
                }
            }

            if( found == false)
            {
                catnbrlist.append(*itcop);
            }
        }
    }


    geostationarylistlon = settings.value( "/satellite/geostationarylistlon" ).value<QStringList>();
    geostationarylistname = settings.value( "/satellite/geostationarylistname" ).value<QStringList>();

    segmentdirectorylist = settings.value( "/satellite/segmentdirectories" ).value<QStringList>();
    segmentdirectorylistinc = settings.value( "/satellite/segmentdirectoriesinc" ).value<QStringList>();
    realminutesshown=settings.value("/satellite/minutesshown", 10).toInt();

    channellistmetop=settings.value("/segments/channellistmetop").value<QStringList>();
    channellistnoaa=settings.value("/segments/channellistnoaa").value<QStringList>();
    channellistgac=settings.value("/segments/channellistGAC").value<QStringList>();
    channellisthrp=settings.value("/segments/channellistHRP").value<QStringList>();

    stationlistname=settings.value("/window/stationlistname").value<QStringList>();
    stationlistlon=settings.value("/window/stationlistlon").value<QStringList>();
    stationlistlat=settings.value("/window/stationlistlat").value<QStringList>();
    backgroundimage2D=settings.value("/window/backgroundimage2D", "images/Topography.jpg").value<QString>();
    backgroundimage3D=settings.value("/window/backgroundimage3D", "images/NE2_50M_SR_W_4096.jpg").value<QString>();
    tlesources=settings.value("/satellite/tlesources").value<QStringList>();

    gshhsglobe1=settings.value("/window/gshhsglobe1", "gshhs2_3_4/gshhs_i.b").value<QString>();
    gshhsglobe2=settings.value("/window/gshhsglobe2", "gshhs2_3_4/wdb_borders_i.b").value<QString>();
    gshhsglobe3=settings.value("/window/gshhsglobe3", "gshhs2_3_4/wdb_rivers_i.b").value<QString>();
    gshhsoverlay1=settings.value("/window/gshhsoverlay1", "gshhs2_3_4/gshhs_i.b").value<QString>();
    gshhsoverlay2=settings.value("/window/gshhsoverlay2", "gshhs2_3_4/wdb_borders_i.b").value<QString>();
    gshhsoverlay3=settings.value("/window/gshhsoverlay3", "").value<QString>();

    skyboxup=settings.value("/window/skyboxup", "images/ulukai/corona_up.png").value<QString>();
    skyboxdown=settings.value("/window/skyboxdown", "images/ulukai/corona_dn.png").value<QString>();
    skyboxleft=settings.value("/window/skyboxleft", "images/ulukai/corona_lf.png").value<QString>();
    skyboxright=settings.value("/window/skyboxright", "images/ulukai/corona_rt.png").value<QString>();
    skyboxfront=settings.value("/window/skyboxfront", "images/ulukai/corona_ft.png").value<QString>();
    skyboxback=settings.value("/window/skyboxback", "images/ulukai/corona_bk.png").value<QString>();

    sathorizoncolor=settings.value("/window/sathorizoncolor", "#ffff00").value<QString>();
    sattrackcolor=settings.value("/window/sattrackcolor", "#dcdc00").value<QString>();

    satsegmentcolor=settings.value("/window/satsegmentcolor", "#00aa00").value<QString>();
    satsegmentcolorsel=settings.value("/window/satsegmentcolorsel", "#ff0e42").value<QString>();

    globeoverlaycolor1=settings.value("/window/globeoverlaycolor1", "#afaf47").value<QString>();
    globeoverlaycolor2=settings.value("/window/globeoverlaycolor2", "#898969").value<QString>();
    globeoverlaycolor3=settings.value("/window/globeoverlaycolor3", "#54686d").value<QString>();
    imageoverlaycolor1=settings.value("/window/imageoverlaycolor1", "#959559").value<QString>();
    imageoverlaycolor2=settings.value("/window/imageoverlaycolor2", "#929274").value<QString>();
    imageoverlaycolor3=settings.value("/window/imageoverlaycolor3", "#63696f").value<QString>();

    globelonlatcolor=settings.value("/window/globelonlatcolor", "#b9b9b9").value<QString>();
    maplccextentcolor=settings.value("/window/maplccextentcolor", "#152ce7").value<QString>();
    mapgvpextentcolor=settings.value("/window/mapgvpextentcolor", "#dc2cc2").value<QString>();
    projectionoverlaycolor1=settings.value("/window/projectionoverlaycolor1", "#a0a351").value<QString>();
    projectionoverlaycolor2=settings.value("/window/projectionoverlaycolor2", "#7d7b60").value<QString>();
    projectionoverlaycolor3=settings.value("/window/projectionoverlaycolor3", "#75749b").value<QString>();
    projectionoverlaylonlatcolor=settings.value("/window/projectionoverlaylonlatcolor", "#b9b9b9").value<QString>();

    smoothprojectiontype = settings.value("/window/smoothprojectiontype", 0 ).toInt();
    equirectangulardirectory=settings.value("/window/equirectangulardirectory", "").value<QString>();

    gridonprojection = settings.value("/window/gridonprojection", true ).toBool();
    textureOn = settings.value("/window/textureon", true ).toBool();
    stationnameOn = settings.value("/window/stationnameon", false ).toBool();
    lightingOn = settings.value("/window/lightingon", true ).toBool();
    imageontextureOnMet = settings.value("/window/imageontextureonmet", false ).toBool();
    imageontextureOnAVHRR = settings.value("/window/imageontextureonavhrr", true ).toBool();
    imageontextureOnVIIRS = settings.value("/window/imageontextureonviirs", true ).toBool();
    imageontextureOnOLCIefr = settings.value("/window/imageontextureonolciefr", true ).toBool();
    imageontextureOnOLCIerr = settings.value("/window/imageontextureonolcierr", true ).toBool();
    windowvectors = settings.value("/window/windowsvectors", false ).toBool();

    localdirremote = settings.value("/window/localdirremote", "").value<QString>();
    dirremote = settings.value("/window/dirremote", "").value<QString>();
    udpmessages = settings.value("/window/udpmessages", false).toBool();

    gshhsglobe1On = settings.value("/window/gshhsglobe1on", true ).toBool();
    gshhsglobe2On = settings.value("/window/gshhsglobe2on", true ).toBool();
    gshhsglobe3On = settings.value("/window/gshhsglobe3on", false ).toBool();
    graytextureOn = settings.value("/window/graytextureon", false ).toBool();
    ephemsplittersizes = settings.value("/ephemwindow/splitterSizes").toByteArray(); // .value<QByteArray>();

    //mainwindowgeometry = settings.value("/window/mainwindowgeometry").value<QByteArray>();
    //windowstate = settings.value("/window/windowstate").value<QByteArray>();

    zoomfactoravhrr = settings.value("/window/zoomfactoravhrr", 100).toInt();
    if( zoomfactoravhrr < 5 || zoomfactoravhrr > 500 )
        zoomfactoravhrr = 100;

    zoomfactormeteosat = settings.value("/window/zoomfactormeteosat", 100).toInt();
    if( zoomfactormeteosat < 5 || zoomfactormeteosat > 500 )
        zoomfactormeteosat = 100;

    zoomfactorprojection = settings.value("/window/zoomfactorprojection", 100).toInt();
    if( zoomfactorprojection < 5 || zoomfactorprojection > 500 )
        zoomfactorprojection = 100;

    zoomfactorviirs = settings.value("/window/zoomfactorviirs", 100).toInt();
    if( zoomfactorviirs < 5 || zoomfactorviirs > 500 )
        zoomfactorviirs = 100;

    currenttoolbox = settings.value("/parameters/currenttoolbox", 0).toInt();

    currenttabwidget = settings.value("/parameters/currenttabwidget", 0).toInt();
    parallel1 = settings.value("/parameters/parallel1", 20).toInt();
    parallel2 = settings.value("/parameters/parallel2", 60).toInt();
    centralmeridian = settings.value("/parameters/centralmeridian", 0).toInt();
    latitudeoforigin = settings.value("/parameters/latitudeoforigin", 0).toInt();
    mapextentnorth  = settings.value("/parameters/mapextentnorth", 10).toInt();
    mapextentsouth  = settings.value("/parameters/mapextentsouth", -10).toInt();
    mapextentwest  = settings.value("/parameters/mapextentwest", -10).toInt();
    mapextenteast  = settings.value("/parameters/mapextenteast", 10).toInt();

    mapextentlamberton  = settings.value("/parameters/mapextentlamberton", false).toBool();
    mapextentperspectiveon  = settings.value("/parameters/mapextentperspectiveon", false).toBool();
    mapheight = settings.value("/parameters/mapheight", 600).toInt();
    mapwidth = settings.value("/parameters/mapwidth", 800).toInt();

    mapgvplon = settings.value("/parameters/mapgvplon", 0.0).toDouble();
    mapgvplat = settings.value("/parameters/mapgvplat", 0.0).toDouble();
    mapgvpheight = settings.value("/parameters/mapgvpheight", 36000).toInt();
    mapgvpscale = settings.value("/parameters/mapgvpscale", 1.0).toDouble();
    maplccscalex = settings.value("/parameters/maplccscalex", 1.0).toDouble();
    maplccscaley = settings.value("/parameters/maplccscaley", 1.0).toDouble();
    meteosatgamma = settings.value("/parameters/meteosatgamma", 1.0).toDouble();
    mapsglat = settings.value("/parameters/mapsglat", 0.0).toDouble();
    mapsglon = settings.value("/parameters/mapsglon", 0.0).toDouble();
    mapsgscale = settings.value("/parameters/mapsgscale", 1.0).toDouble();
    mapsgpanvert = settings.value("/parameters/mapsgpanvert", 0).toInt();
    mapsgpanhorizon = settings.value("/parameters/mapsgpanhorizon", 0).toInt();
    mapsgradius = settings.value("/parameters/mapsgradius", 45.0).toDouble();

    yawcorrection = settings.value("/parameters/yawcorrection", 0.0).toDouble();

    clahecliplimit = settings.value("/parameters/clahecliplimit", 1.0).toFloat();

    lastinputprojection = settings.value("/window/lastinputprojection", 0 ).toInt();
    lastVIIRSband = settings.value("/window/viirsband", 0 ).toInt();

    dnbsblowerlimit = settings.value("/dnb/dnbsblowerlimit", -250 ).toInt();
    dnbsbupperlimit = settings.value("/dnb/dnbsbupperlimit", -100 ).toInt();
    dnbsbvalue = settings.value("/dnb/dnbsbvalue", -209 ).toInt();
    dnbspbwindowsvalue = settings.value("/dnb/dnbspbwindowsvalue", 3 ).toInt();

    lastcomboMet006 = settings.value("/window/comboMet006", 0 ).toInt();;
    lastcomboMet008 = settings.value("/window/comboMet008", 0 ).toInt();
    lastcomboMet016 = settings.value("/window/comboMet016", 0 ).toInt();
    lastcomboMet039 = settings.value("/window/comboMet039", 0 ).toInt();
    lastcomboMet062 = settings.value("/window/comboMet062", 0 ).toInt();
    lastcomboMet073 = settings.value("/window/comboMet073", 0 ).toInt();
    lastcomboMet087 = settings.value("/window/comboMet087", 0 ).toInt();
    lastcomboMet097 = settings.value("/window/comboMet097", 0 ).toInt();
    lastcomboMet108 = settings.value("/window/comboMet108", 0 ).toInt();
    lastcomboMet120 = settings.value("/window/comboMet120", 0 ).toInt();
    lastcomboMet134 = settings.value("/window/comboMet134", 0 ).toInt();


    lastinverseMet006 = settings.value("/window/inverseMet006", false ).toBool();;
    lastinverseMet008 = settings.value("/window/inverseMet008", false ).toBool();
    lastinverseMet016 = settings.value("/window/inverseMet016", false ).toBool();
    lastinverseMet039 = settings.value("/window/inverseMet039", false ).toBool();
    lastinverseMet062 = settings.value("/window/inverseMet062", false ).toBool();
    lastinverseMet073 = settings.value("/window/inverseMet073", false ).toBool();
    lastinverseMet087 = settings.value("/window/inverseMet087", false ).toBool();
    lastinverseMet097 = settings.value("/window/inverseMet097", false ).toBool();
    lastinverseMet108 = settings.value("/window/inverseMet108", false ).toBool();
    lastinverseMet120 = settings.value("/window/inverseMet120", false ).toBool();
    lastinverseMet134 = settings.value("/window/inverseMet134", false ).toBool();

    colormapMagma = settings.value("/projection/colormapmagma", false ).toBool();
    colormapInferno = settings.value("/projection/colormapinferno", false ).toBool();
    colormapPlasma = settings.value("/projection/colormapplasma", true ).toBool();
    colormapViridis = settings.value("/projection/colormapviridis", false ).toBool();

    remove_S3A_dirs = settings.value("/parameters/removes3adirs", false).toBool();

    checkStringListValues();
    fbo_changed = false;
    texture_changed = false;

}

void Options::checkStringListValues()
{
    if(channellistmetop.count() != 5) { channellistmetop << "3" << "2" << "1" << "0" << "0"; }
    if(channellistnoaa.count() != 5) { channellistnoaa << "3" << "2" << "1" << "0" << "0"; }
    if(channellistgac.count() != 5) { channellistgac << "3" << "2" << "1" << "0" << "0"; }
    if(channellisthrp.count() != 5) { channellisthrp << "3" << "2" << "1" << "0" << "0"; }

    if(metop_invlist.count() != 5) { metop_invlist << "0" << "0" << "0" << "0" <<"0"; }
    if(noaa_invlist.count() != 5) { noaa_invlist << "0" << "0" << "0" << "0" <<"0"; }
    if(gac_invlist.count() != 5) { gac_invlist << "0" << "0" << "0" << "0" <<"0"; }
    if(hrp_invlist.count() != 5) { hrp_invlist << "0" << "0" << "0" << "0" <<"0"; }

    if(stationlistname.count() == 0)
    {
        stationlistname << "McMur Station" << "Maspalomas" << "Svalbard" << "Edmonton" << "Gander" << "Gilmore Creek" <<
                           "Monterey" << "Kangerlussuaq" << "Lannion" << "Saint-Denis (La Reunion)" <<
                           "Moscow" << "Muscat" << "Tromso" << "Ewa Beach" << "Miami" << "Pine Island Glacier" << "Athens";
        stationlistlon << "166.666" << "-15.63" << "15.23" << "-113.5" << "-54.57" << "-147.40" << "-121.55" << "-50.67" <<
                           "-3.5" << "55.50" << "37.569" << "58.29" << "18.933" << "-158.07" << "-80.16" << "-100.0" << "23.769";
        stationlistlat << "-77.85" << "27.78" << "78.13" << "53.33" << "48.95" << "64.97" << "36.35" << "66.98" << "48.75" <<
                          "-20.91" << "55.759" << "23.59" << "69.65" << "21.33" << "25.74" << "-75.166667" << "37.815";
    }

//    enum eGeoSatellite {
//        MET_10 =0,
//        MET_9,
//        MET_7,
//        FY2E,
//        FY2G,
//        GOES_13,
//        GOES_15,
//        H8,
//        ELECTRO_N1,
//        NOGEO
//    };

    //if(geostationarylistname.count() == 0)
    {
        geostationarylistname.clear();
        geostationarylistlon.clear();
        geostationarylistname << "Meteosat-10" << "Meteosat-9" << "Meteosat-7" << "FY2E" << "FY2G" << "GOES13" << "GOES15" << "Himawari-8" << "Meteosat-8";
        geostationarylistlon << "0.0" << "9.5" << "57" << "86.5" << "104.5" << "-74.9" << "-135.2" << "140.7" << "41.5";
    }

    if(tlesources.count() == 0)
    {
        tlesources << "http://celestrak.com/NORAD/elements/weather.txt";
    }

}

void Options::Save()
{

    qDebug() << QString("Saving Options ");

    QSettings settings( "EUMETCastView.ini", QSettings::IniFormat);

    settings.setValue("/satellite/tlefiles", tlelist );

    QStringList catnbrlistcop;
    bool found;
    bool ok;

    catnbrlistcop = catnbrlist;
    catnbrlist.clear();

    for ( QStringList::Iterator itcop = catnbrlistcop.begin(); itcop != catnbrlistcop.end(); ++itcop )
    {
        if( catnbrlist.count() == 0)
            catnbrlist.append(*itcop);
        else
        {
            found = false;
            for ( QStringList::Iterator itc = catnbrlist.begin(); itc != catnbrlist.end(); ++itc )
            {
                if ( (*itcop).toInt( &ok, 10) == (*itc).toInt( &ok, 10 ) )
                {
                    found = true;
                    break;
                }
            }

            if( found == false)
            {
                catnbrlist.append(*itcop);
            }
        }
    }

    settings.setValue("/satellite/catnrs", catnbrlist );
    settings.setValue("/satellite/segmentdirectories", segmentdirectorylist );
    settings.setValue("/satellite/segmentdirectoriesinc", segmentdirectorylistinc );
    settings.setValue("/satellite/minutesshown", realminutesshown );
    settings.setValue("/segments/nbrofvisiblesegments", nbrofvisiblesegments);
    settings.setValue("/segments/nbrofhours", nbrofhours);
    settings.setValue( "/window/buttonmetop", buttonMetop );
    settings.setValue( "/window/buttonnoaa", buttonNoaa );
    settings.setValue( "/window/buttongac", buttonGAC );
    settings.setValue( "/window/buttonhrp", buttonHRP );
    settings.setValue( "/window/buttonviirs", buttonVIIRSM );
    settings.setValue( "/window/buttonviirsdnb", buttonVIIRSDNB );
    settings.setValue( "/window/buttonolciefr", buttonOLCIefr );
    settings.setValue( "/window/buttonolcierr", buttonOLCIerr );

    settings.setValue( "/window/buttonrealtime", buttonRealTime );
    settings.setValue( "/window/buttonequalization", buttonEqualization );
    settings.setValue( "/observer/longitude", obslon );
    settings.setValue( "/observer/latitude", obslat );
    settings.setValue( "/observer/altitude", obsalt );

    settings.setValue("/segments/metop_invlist", metop_invlist);
    settings.setValue("/segments/noaa_invlist", noaa_invlist);
    settings.setValue("/segments/gac_invlist", gac_invlist);
    settings.setValue("/segments/hrp_invlist", hrp_invlist);

    settings.setValue( "/segments/sattrackinimage", sattrackinimage );

    settings.setValue("/segments/channellistmetop", channellistmetop);
    settings.setValue("/segments/channellistnoaa", channellistnoaa);
    settings.setValue("/segments/channellistGAC", channellistgac);
    settings.setValue("/segments/channellistHRP", channellisthrp);

    settings.setValue("/window/stationlistname", stationlistname );
    settings.setValue("/window/stationlistlon", stationlistlon );
    settings.setValue("/window/stationlistlat", stationlistlat );

    settings.setValue("/window/backgroundimage2D", backgroundimage2D );
    settings.setValue("/window/backgroundimage3D", backgroundimage3D );
    settings.setValue("/satellite/tlesources", tlesources );

    settings.setValue("/window/gshhsglobe1", gshhsglobe1 );
    settings.setValue("/window/gshhsglobe2", gshhsglobe2 );
    settings.setValue("/window/gshhsglobe3", gshhsglobe3 );
    settings.setValue("/window/gshhsoverlay1", gshhsoverlay1 );
    settings.setValue("/window/gshhsoverlay2", gshhsoverlay2 );
    settings.setValue("/window/gshhsoverlay3", gshhsoverlay3 );

    settings.setValue("/window/skyboxup", skyboxup );
    settings.setValue("/window/skyboxdown", skyboxdown );
    settings.setValue("/window/skyboxleft", skyboxleft );
    settings.setValue("/window/skyboxright", skyboxright );
    settings.setValue("/window/skyboxfront", skyboxfront );
    settings.setValue("/window/skyboxback", skyboxback );

    settings.setValue("/window/sathorizoncolor", sathorizoncolor );
    settings.setValue("/window/sattrackcolor", sattrackcolor );

    settings.setValue("/window/satsegmentcolor", satsegmentcolor );
    settings.setValue("/window/satsegmentcolorsel", satsegmentcolorsel );

    settings.setValue("/window/globeoverlaycolor1", globeoverlaycolor1 );
    settings.setValue("/window/globeoverlaycolor2", globeoverlaycolor2 );
    settings.setValue("/window/globeoverlaycolor3", globeoverlaycolor3 );
    settings.setValue("/window/imageoverlaycolor1", imageoverlaycolor1 );
    settings.setValue("/window/imageoverlaycolor2", imageoverlaycolor2 );
    settings.setValue("/window/imageoverlaycolor3", imageoverlaycolor3 );

    settings.setValue("/window/globelonlatcolor", globelonlatcolor );
    settings.setValue("/window/maplccextentcolor", maplccextentcolor );
    settings.setValue("/window/mapgvpextentcolor", mapgvpextentcolor );
    settings.setValue("/window/projectionoverlaycolor1", projectionoverlaycolor1 );
    settings.setValue("/window/projectionoverlaycolor2", projectionoverlaycolor2 );
    settings.setValue("/window/projectionoverlaycolor3", projectionoverlaycolor3 );
    settings.setValue("/window/projectionoverlaylonlatcolor", projectionoverlaylonlatcolor );

    settings.setValue("/window/smoothprojectiontype", smoothprojectiontype );
    settings.setValue("/window/equirectangulardirectory", equirectangulardirectory );

    settings.setValue("/window/gridonprojection", gridonprojection );
    settings.setValue("/window/textureon", textureOn );
    settings.setValue("/window/stationnameon", stationnameOn );
    settings.setValue("/window/lightingon", lightingOn );
    settings.setValue("/window/imageontextureonmet", imageontextureOnMet );
    settings.setValue("/window/imageontextureonavhrr", imageontextureOnAVHRR );
    settings.setValue("/window/imageontextureonviirs", imageontextureOnVIIRS );
    settings.setValue("/window/imageontextureonolciefr", imageontextureOnOLCIefr );
    settings.setValue("/window/imageontextureonolcierr", imageontextureOnOLCIerr );
    settings.setValue("/window/windowsvectors", windowvectors );

    settings.setValue("/window/gshhsglobe1on", gshhsglobe1On );
    settings.setValue("/window/gshhsglobe2on", gshhsglobe2On );
    settings.setValue("/window/gshhsglobe3on", gshhsglobe3On );
    settings.setValue("/window/graytextureon", graytextureOn );

    settings.setValue("/window/localdirremote", localdirremote );
    settings.setValue("/window/dirremote", dirremote );
    settings.setValue("/window/udpmessages", udpmessages );


    settings.setValue("/ephemwindow/splitterSizes", ephemsplittersizes );

    //settings.setValue("/window/mainwindowgeometry", mainwindowgeometry );
    //settings.setValue("/window/windowstate", windowstate );

    settings.setValue("/window/zoomfactoravhrr", zoomfactoravhrr );
    settings.setValue("/window/zoomfactormeteosat", zoomfactormeteosat );
    settings.setValue("/window/zoomfactorprojection", zoomfactorprojection );
    settings.setValue("/window/zoomfactorviirs", zoomfactorviirs );


    qDebug() << QString("saving currenttoolbox = %1 currenttabwidget = %2").arg(currenttoolbox).arg(currenttabwidget);
    settings.setValue("/parameters/currenttoolbox", currenttoolbox);
    settings.setValue("/parameters/currenttabwidget", currenttabwidget);
    settings.setValue("/parameters/parallel1", parallel1);
    settings.setValue("/parameters/parallel2", parallel2);
    settings.setValue("/parameters/centralmeridian", centralmeridian);
    settings.setValue("/parameters/latitudeoforigin", latitudeoforigin);

    settings.setValue("/parameters/mapextentnorth", mapextentnorth);

    settings.setValue("/dnb/dnbsblowerlimit", dnbsblowerlimit );
    settings.setValue("/dnb/dnbsbupperlimit", dnbsbupperlimit );
    settings.setValue("/dnb/dnbsbvalue", dnbsbvalue );
    settings.setValue("/dnb/dnbspbwindowsvalue", dnbspbwindowsvalue );

    qDebug() << QString("save Options::mapextentnorth = %1").arg(mapextentnorth);
    qDebug() << QString("save Options::mapextentsouth = %1").arg(mapextentsouth);
    qDebug() << QString("save Options::mapextentwest = %1").arg(mapextentwest);
    qDebug() << QString("save Options::mapextenteast = %1").arg(mapextenteast);


    settings.setValue("/parameters/mapextentsouth", mapextentsouth);
    settings.setValue("/parameters/mapextentwest", mapextentwest);
    settings.setValue("/parameters/mapextenteast", mapextenteast);
    settings.setValue("/parameters/mapextentlamberton", mapextentlamberton);
    settings.setValue("/parameters/mapextentperspectiveon", mapextentperspectiveon);
    settings.setValue("/parameters/mapheight", mapheight);
    settings.setValue("/parameters/mapwidth", mapwidth);
    settings.setValue("/parameters/mapgvplon", mapgvplon);
    settings.setValue("/parameters/mapgvplat", mapgvplat);
    settings.setValue("/parameters/mapgvpheight", mapgvpheight);
    settings.setValue("/parameters/mapgvpscale", mapgvpscale);
    settings.setValue("/parameters/maplccscalex", maplccscalex);
    settings.setValue("/parameters/maplccscaley", maplccscaley);
    settings.setValue("/parameters/meteosatgamma", meteosatgamma);
    settings.setValue("/parameters/mapsglat", mapsglat);
    settings.setValue("/parameters/mapsglon", mapsglon);
    settings.setValue("/parameters/mapsgscale", mapsgscale);
    settings.setValue("/parameters/mapsgpanvert", mapsgpanvert);
    settings.setValue("/parameters/mapsgpanhorizon", mapsgpanhorizon);
    settings.setValue("/parameters/mapsgradius", mapsgradius);

    settings.setValue("/parameters/yawcorrection", yawcorrection);
    settings.setValue("/parameters/clahecliplimit", clahecliplimit);

    settings.setValue( "/satellite/geostationarylistlon", geostationarylistlon );
    settings.setValue( "/satellite/geostationarylistname", geostationarylistname );

    settings.setValue( "/window/comboMet006", lastcomboMet006);
    settings.setValue( "/window/comboMet008", lastcomboMet008);
    settings.setValue( "/window/comboMet016", lastcomboMet016);
    settings.setValue( "/window/comboMet039", lastcomboMet039);
    settings.setValue( "/window/comboMet062", lastcomboMet062);
    settings.setValue( "/window/comboMet073", lastcomboMet073);
    settings.setValue( "/window/comboMet087", lastcomboMet087);
    settings.setValue( "/window/comboMet097", lastcomboMet097);
    settings.setValue( "/window/comboMet108", lastcomboMet108);
    settings.setValue( "/window/comboMet120", lastcomboMet120);
    settings.setValue( "/window/comboMet134", lastcomboMet134);

    settings.setValue( "/window/inverseMet006", lastinverseMet006);
    settings.setValue( "/window/inverseMet008", lastinverseMet008);
    settings.setValue( "/window/inverseMet016", lastinverseMet016);
    settings.setValue( "/window/inverseMet039", lastinverseMet039);
    settings.setValue( "/window/inverseMet062", lastinverseMet062);
    settings.setValue( "/window/inverseMet073", lastinverseMet073);
    settings.setValue( "/window/inverseMet087", lastinverseMet087);
    settings.setValue( "/window/inverseMet097", lastinverseMet097);
    settings.setValue( "/window/inverseMet108", lastinverseMet108);
    settings.setValue( "/window/inverseMet120", lastinverseMet120);
    settings.setValue( "/window/inverseMet134", lastinverseMet134);

    settings.setValue( "/window/lastinputprojection", lastinputprojection );
    settings.setValue( "/window/viirsband", lastVIIRSband);

    settings.setValue( "/projection/colormapmagma", colormapMagma);
    settings.setValue( "/projection/colormapinferno", colormapInferno);
    settings.setValue( "/projection/colormapplasma", colormapPlasma);
    settings.setValue( "/projection/colormapviridis", colormapViridis);

    settings.setValue("/parameters/removes3adirs", remove_S3A_dirs);


}

void Options::deleteTleFile( QString sel )
{

  QStringList::Iterator its = tlelist.begin();
  QStringList strlistout;
  
  while( its != tlelist.end() )
  {
    if (*its != sel)
      strlistout << *its;
    ++its;
  }
  tlelist = strlistout;
  
}  


bool Options::addTleFile( QString sel )
{
  QStringList::Iterator its = tlelist.begin();
  bool thesame = false;
  while( its != tlelist.end() )
  {
    if (*its  == sel)
      thesame = true;
    ++its;
  }
  if(!thesame)
      tlelist << sel;
  return (!thesame);

}


void Options::deleteSegmentDirectory( QString sel )
{
  QStringList::Iterator its = segmentdirectorylist.begin();
  QStringList strlistout;

  while( its != segmentdirectorylist.end() )
  {
    if (*its != sel)
      strlistout << *its;
    ++its;
  }
  segmentdirectorylist = strlistout;

}

