#include <QDebug>
#include "gshhsdata.h"
#include "options.h"

extern Options opts;

// c = crude
// l = low
// i = intermediate
// h = high
// f = full

// These changes required us to enhance the GSHHG C-structure used to
// read and write the data.  As of version 2.0 the header structure is

// struct GSHHG {  /* Global Self-consistent Hierarchical High-resolution Shorelines */
//         int id;         /* Unique polygon id number, starting at 0 */
//         int n;          /* Number of points in this polygon */
//         int flag;       /* = level + version << 8 + greenwich << 16 + source << 24 + river << 25 */
//         /* flag contains 5 items, as follows:
//          * low byte:    level = flag & 255: Values: 1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake
//          * 2nd byte:    version = (flag >> 8) & 255: Values: Should be 12 for GSHHG release 12 (i.e., version 2.2)
//          * 3rd byte:    greenwich = (flag >> 16) & 1: Values: Greenwich is 1 if Greenwich is crossed
//          * 4th byte:    source = (flag >> 24) & 1: Values: 0 = CIA WDBII, 1 = WVS
//          * 4th byte:    river = (flag >> 25) & 1: Values: 0 = not set, 1 = river-lake and level = 2
//          */
//         int west, east, south, north;   /* min/max extent in micro-degrees */
//         int area;       /* Area of polygon in 1/10 km^2 */
//         int area_full;  /* Area of original full-resolution polygon in 1/10 km^2 */
//         int container;  /* Id of container polygon that encloses this polygon (-1 if none) */
//         int ancestor;   /* Id of ancestor polygon in the full resolution set that was the source of this polygon (-1 if none) */
// };

// Following each header structure is n structures of coordinates:

// struct GSHHG_POINT {	/* Each lon, lat pair is stored in micro-degrees in 4-byte signed integer format */
// 	int32_t x;
// 	int32_t y;
// };

// Some useful information:

// A) To avoid headaches the binary files were written to be big-endian.
//    If you use the GMT supplement gshhg it will check for endian-ness and if needed will
//    byte swab the data automatically. If not then you will need to deal with this yourself.

// B) In addition to GSHHS we also distribute the files with political boundaries and
//    river lines.  These derive from the WDBII data set.

// C) As to the best of our knowledge, the GSHHG data are geodetic longitude, latitude
//    locations on the WGS-84 ellipsoid.  This is certainly true of the WVS data (the coastlines).
//    Lakes, riverlakes (and river lines and political borders) came from the WDBII data set
//    which may have been on WGS072.  The difference in ellipsoid is way less then the data
//    uncertainties.  Offsets have been noted between GSHHG and modern GPS positions.

// D) Originally, the gshhs_dp tool was used on the full resolution data to produce the lower
//    resolution versions.  However, the Douglas-Peucker algorithm often produce polygons with
//    self-intersections as well as create segments that intersect other polygons.  These problems
//    have been corrected in the GSHHG lower resolutions over the years.  If you use gshhs_dp to
//    generate your own lower-resolution data set you should expect these problems.

// E) The shapefiles release was made by formatting the GSHHG data using the extended GMT/GIS
//    metadata understood by OGR, then using ogr2ogr to build the shapefiles.  Each resolution
//    is stored in its own subdirectory (e.g., f, h, i, l, c) and each level (1-4) appears in
//    its own shapefile.  Thus, GSHHS_h_L3.shp contains islands in lakes for the high res
//    data. Because of GIS limitations some polygons that straddle the Dateline (including
//    Antarctica) have been split into two parts (east and west).

// F) The netcdf-formatted coastlines distributed with GMT derives directly from GSHHG; however
//    the polygons have been broken into segments within tiles.  These files are not meant
//    to be used by users other than via GMT tools (pscoast, grdlandmask, etc).

gshhsData::gshhsData()
{


    for(int i = 0; i < 3; i++)
    {
        vxp_data[i] = new Vxp;
        vxp_data[i]->nFeatures = 0;
    }

    for(int i = 0; i < 3; i++)
    {
        vxp_data_overlay[i] = new Vxp;
        vxp_data_overlay[i]->nFeatures = 0;
    }

    // An unset overlay must stay unset. Prefixing APPDIR onto an empty setting
    // turns "not configured" into the AppDir directory itself, which is not
    // empty, so Initialize below stops skipping it and tries to read it - and
    // fopen on a directory succeeds in read mode on Linux, so it gets all the
    // way to reporting phantom features. gshhsoverlay3 defaults to empty, which
    // made this fire on every AppImage launch.
    auto resolve = [](const QString &p) {
        return (p.isEmpty() || opts.appdir_env.isEmpty()) ? p
                                                          : opts.appdir_env + "/" + p;
    };

    Initialize(resolve(opts.gshhsglobe1),   resolve(opts.gshhsglobe2),   resolve(opts.gshhsglobe3),
               resolve(opts.gshhsoverlay1), resolve(opts.gshhsoverlay2), resolve(opts.gshhsoverlay3));


}

void gshhsData::initializegshhsData(QOpenGLShaderProgram *prog)
{
    program = prog;

    initializeOpenGLFunctions();

    QVector<GLfloat> positions[3];
    int totalverts = 0;


    for( int k = 0; k < 3; k++)
    {
        featurevertsindex[k].append(0);

        for( int i = 0; i < vxp_data[k]->nFeatures; i++)
        {
            for (int j = 0; j < vxp_data[k]->pFeatures[i].nVerts; j++)
            {
                positions[k].append((float)vxp_data[k]->pFeatures[i].pVerts[j].x());
                positions[k].append((float)vxp_data[k]->pFeatures[i].pVerts[j].y());
                positions[k].append((float)vxp_data[k]->pFeatures[i].pVerts[j].z());
            }
            totalverts += vxp_data[k]->pFeatures[i].nVerts;
            featurevertsindex[k].append(totalverts);
        }

        totalverts = 0;
    }

    // Bind shader pipeline for use
    program->bind();

    vao1.create();
    QOpenGLVertexArrayObject::Binder vaoBinder1(&vao1);

    positionsBuf1.create();
    positionsBuf1.setUsagePattern(QOpenGLBuffer::StaticDraw);
    positionsBuf1.bind();
    positionsBuf1.allocate(positions[0].data(), positions[0].size() * sizeof(GLfloat));

    vertexPosition = program->attributeLocation("VertexPosition");
    program->enableAttributeArray(vertexPosition);
    program->setAttributeBuffer(vertexPosition, GL_FLOAT, 0, 3);


    vao2.create();
    QOpenGLVertexArrayObject::Binder vaoBinder2(&vao2);

    positionsBuf2.create();
    positionsBuf2.setUsagePattern(QOpenGLBuffer::StaticDraw);
    positionsBuf2.bind();
    positionsBuf2.allocate(positions[1].data(), positions[1].size() * sizeof(GLfloat));

    vertexPosition = program->attributeLocation("VertexPosition");
    program->enableAttributeArray(vertexPosition);
    program->setAttributeBuffer(vertexPosition, GL_FLOAT, 0, 3);

    vao3.create();
    QOpenGLVertexArrayObject::Binder vaoBinder3(&vao3);

    positionsBuf3.create();
    positionsBuf3.setUsagePattern(QOpenGLBuffer::StaticDraw);
    positionsBuf3.bind();
    positionsBuf3.allocate(positions[2].data(), positions[2].size() * sizeof(GLfloat));

    vertexPosition = program->attributeLocation("VertexPosition");
    program->enableAttributeArray(vertexPosition);
    program->setAttributeBuffer(vertexPosition, GL_FLOAT, 0, 3);

    qDebug() << QString("gshhsdata positions.size() = %1").arg(positions[0].size());
    qDebug() << QString("gshhsdata totalverts = %1").arg(totalverts);


    delete [] vxp_data[0];
    delete [] vxp_data[1];
    delete [] vxp_data[2];



}

gshhsData::~gshhsData()
{
    vao1.destroy();
    positionsBuf1.destroy();
    vao2.destroy();
    positionsBuf2.destroy();
    vao3.destroy();
    positionsBuf3.destroy();
 }

void gshhsData::render(QMatrix4x4 projection, QMatrix4x4 modelview, int bBorders)
{
    if(bBorders)
    {

        program->bind();

        program->setUniformValue("MVP", projection * modelview);
        QMatrix3x3 norm = modelview.normalMatrix();
        program->setUniformValue("NormalMatrix", norm);

        if(opts.gshhsglobe1On)
        {
            QColor rendercolor(opts.globeoverlaycolor1);
            program->setUniformValue("outcolor", QVector4D(rendercolor.redF(), rendercolor.greenF(), rendercolor.blueF(), 1.0f));
            QOpenGLVertexArrayObject::Binder vaoBinder1(&vao1);
            for(int i = 0; i < featurevertsindex[0].size()-1; i++)
                glDrawArrays(GL_LINE_STRIP, featurevertsindex[0].at(i), featurevertsindex[0].at(i+1)-featurevertsindex[0].at(i));
        }

        if(opts.gshhsglobe2On)
        {
            QColor rendercolor(opts.globeoverlaycolor2);
            program->setUniformValue("outcolor", QVector4D(rendercolor.redF(), rendercolor.greenF(), rendercolor.blueF(), 1.0f));
            QOpenGLVertexArrayObject::Binder vaoBinder2(&vao2);
            for(int i = 0; i < featurevertsindex[1].size()-1; i++)
                glDrawArrays(GL_LINE_STRIP, featurevertsindex[1].at(i), featurevertsindex[1].at(i+1)-featurevertsindex[1].at(i));
        }

        if(opts.gshhsglobe3On)
        {
            QColor rendercolor(opts.globeoverlaycolor3);
            program->setUniformValue("outcolor", QVector4D(rendercolor.redF(), rendercolor.greenF(), rendercolor.blueF(), 1.0f));
            QOpenGLVertexArrayObject::Binder vaoBinder3(&vao3);
            for(int i = 0; i < featurevertsindex[2].size()-1; i++)
                glDrawArrays(GL_LINE_STRIP, featurevertsindex[2].at(i), featurevertsindex[2].at(i+1)-featurevertsindex[2].at(i));
        }

        program->release();
    }
}

void gshhsData::Initialize(QString data1, QString data2, QString data3, QString dataoverlay1, QString dataoverlay2, QString dataoverlay3)
{

    QByteArray baglobe1 = data1.toUtf8();
    QByteArray baglobe2 = data2.toUtf8();
    QByteArray baglobe3 = data3.toUtf8();
    QByteArray baoverlay1 = dataoverlay1.toUtf8();
    QByteArray baoverlay2 = dataoverlay2.toUtf8();
    QByteArray baoverlay3 = dataoverlay3.toUtf8();

    if (baglobe1.length() > 0)
    {
        int nFeatures = check_gshhs(baglobe1.data());
        load_gshhs(baglobe1.data(), nFeatures, vxp_data[0]);
        qDebug() << QString("gshhs 1 file loaded nFeatures = %1").arg(nFeatures);
    }

    if (baglobe2.length() > 0)
    {
        int nFeatures = check_gshhs(baglobe2.data());
        load_gshhs(baglobe2.data(), nFeatures, vxp_data[1]);
        qDebug() << QString("gshhs 2 file loaded nFeatures = %1").arg(nFeatures);
    }

    if (baglobe3.length() > 0)
    {
        int nFeatures = check_gshhs(baglobe3.data());
        load_gshhs(baglobe3.data(), nFeatures, vxp_data[2]);
        qDebug() << QString("gshhs 3 file loaded nFeatures = %1").arg(nFeatures);
    }

    if (baoverlay1.length() > 0)
    {
        int nFeatures = check_gshhs(baoverlay1.data());
        load_gshhs(baoverlay1.data(), nFeatures, vxp_data_overlay[0]);
        qDebug() << QString("gshhs overlay 1 file loaded nFeatures = %1").arg(nFeatures);
    }

    if (baoverlay2.length() > 0)
    {
        int nFeatures = check_gshhs(baoverlay2.data());
        load_gshhs(baoverlay2.data(), nFeatures, vxp_data_overlay[1]);
        qDebug() << QString("gshhs overlay 2 file loaded nFeatures = %1").arg(nFeatures);
    }

    if (baoverlay3.length() > 0)
    {
        int nFeatures = check_gshhs(baoverlay3.data());
        load_gshhs(baoverlay3.data(), nFeatures, vxp_data_overlay[2]);
        qDebug() << QString("gshhs overlay 3 file loaded nFeatures = %1").arg(nFeatures);
    }

}

int gshhsData::check_gshhs(char *pFileName)
{

    double w, e, s, n, area, f_area, lon, lat;
    //char source, kind[2] = {'P', 'L'}, c = '>', *file = NULL;
    //char *name[2] = {"polygon", "line"};
    FILE *fp = NULL;
    int k, line, max_east = 270000000, info, single, error, ID, flip;
    int  OK, level, version, greenwich, river, src, msformat = 0, first = 1;
    size_t n_read;
    struct POINT_GSHHS p;
    struct GSHHS h;

    int nFeatures = 0;


    if ((fp = fopen (pFileName, "rb")) == NULL ) {
            qDebug() << QString( "gshhs:  Could not find file %1.").arg(pFileName);
            return 0; //exit (EXIT_FAILURE);
    }

    n_read = fread ((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);

    // Opening is not the same as being readable. A directory opens perfectly
    // well in read mode on Linux and only fails here, and so does a truncated
    // or empty file. Without this the header below is read uninitialised and,
    // worse, the count returned is 1 rather than 0 - one feature that was never
    // loaded, which the caller then walks straight off the end of.
    if (n_read != 1) {
        qDebug() << QString("gshhs: no readable header in %1 - ignoring it.").arg(pFileName);
        fclose (fp);
        return 0;
    }

    version = (h.flag >> 8) & 255;
    flip = (version != GSHHS_DATA_RELEASE);	/* Take as sign that byte-swabbing is needed */
    qDebug() << "====> flip = " << flip;

    level = h.flag & 255;				/* Level is 1-4 */
    version = (h.flag >> 8) & 255;			/* Version is 1-7 */
    //if (first) fprintf (stderr, "gshhs %s - Found GSHHS version %d in file %s\n", GSHHS_PROG_VERSION, version, file);
    greenwich = (h.flag >> 16) & 1;			/* Greenwich is 0 or 1 */
    src = (h.flag >> 24) & 1;			/* Greenwich is 0 (WDBII) or 1 (WVS) */
    river = (h.flag >> 25) & 1;			/* River is 0 (not river) or 1 (is river) */

    qDebug() << "level = " << level;
    qDebug() << "version = " << version;
    qDebug() << "greenwich = " << greenwich;
    qDebug() << "src = " << src;
    qDebug() << "river = " << river;

    while (n_read == 1)
    {
            if (flip)
            {
                    h.id = swabi4 ((unsigned int)h.id);
                    h.n  = swabi4 ((unsigned int)h.n);
                    h.west  = swabi4 ((unsigned int)h.west);
                    h.east  = swabi4 ((unsigned int)h.east);
                    h.south = swabi4 ((unsigned int)h.south);
                    h.north = swabi4 ((unsigned int)h.north);
                    h.area  = swabi4 ((unsigned int)h.area);
                    h.area_full  = swabi4 ((unsigned int)h.area_full);
                    h.flag  = swabi4 ((unsigned int)h.flag);
                    h.container  = swabi4 ((unsigned int)h.container);
                    h.ancestor  = swabi4 ((unsigned int)h.ancestor);
            }

            fseek (fp, (long)(h.n * sizeof(struct POINT_GSHHS)), SEEK_CUR);
            max_east = 180000000;	/* Only Eurasia needs 270 */
            n_read = fread((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
            if (n_read > 0)
                nFeatures++;
    }

    fclose (fp);
    return (nFeatures + 1);
}

void gshhsData::load_gshhs(char *pFileName, int nTotFeatures, Vxp *vxp)
{

    double w, e, s, n, area, f_area, lon, lat;
    char source, kind[2] = {'P', 'L'}, c = '>'; //, *file = NULL;
    //char *name[2] = {"polygon", "line"};
    FILE *fp = NULL;
    int k, line, max_east = 270000000, flip;
    int  OK, level, version, greenwich, river, src, msformat = 0, first = 1;
    size_t n_read;
    struct POINT_GSHHS p;
    struct GSHHS h;
    //Vxp *vxp = new Vxp;

    int nFeatures = 0;

    if ((fp = fopen (pFileName, "rb")) == NULL ) {
        qDebug() << QString( "gshhs:  Could not find file %1.").arg(pFileName);
        return; // exit (EXIT_FAILURE);
    }

    n_read = fread ((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);

    // See check_gshhs. Claiming features here that the loop below will never
    // fill is what turns an unreadable file into a crash rather than an empty
    // overlay.
    if (n_read != 1) {
        qDebug() << QString("gshhs: no readable header in %1 - ignoring it.").arg(pFileName);
        fclose (fp);
        return;
    }

    version = (h.flag >> 8) & 255;
    flip = (version != GSHHS_DATA_RELEASE);	/* Take as sign that byte-swabbing is needed */

    vxp->nFeatures = nTotFeatures;
    vxp->pFeatures = new VxpFeature[nTotFeatures];

    while (n_read == 1)
    {
            if (flip)
            {
                    h.id = swabi4 ((unsigned int)h.id);
                    h.n  = swabi4 ((unsigned int)h.n);
                    h.west  = swabi4 ((unsigned int)h.west);
                    h.east  = swabi4 ((unsigned int)h.east);
                    h.south = swabi4 ((unsigned int)h.south);
                    h.north = swabi4 ((unsigned int)h.north);
                    h.area  = swabi4 ((unsigned int)h.area);
                    h.area_full  = swabi4 ((unsigned int)h.area_full);
                    h.flag  = swabi4 ((unsigned int)h.flag);
                    h.container  = swabi4 ((unsigned int)h.container);
                    h.ancestor  = swabi4 ((unsigned int)h.ancestor);
            }
            level = h.flag & 255;				/* Level is 1-4 */
            version = (h.flag >> 8) & 255;			/* Version is 1-7 */
            //if (first) fprintf (stderr, "gshhs %s - Found GSHHS version %d in file %s\n", GSHHS_PROG_VERSION, version, file);
            greenwich = (h.flag >> 16) & 1;			/* Greenwich is 0 or 1 */
            src = (h.flag >> 24) & 1;			/* Greenwich is 0 (WDBII) or 1 (WVS) */
            river = (h.flag >> 25) & 1;			/* River is 0 (not river) or 1 (is river) */
            w = h.west  * GSHHS_SCL;			/* Convert from microdegrees to degrees */
            e = h.east  * GSHHS_SCL;
            s = h.south * GSHHS_SCL;
            n = h.north * GSHHS_SCL;
            source = (src == 1) ? 'W' : 'C';		/* Either WVS or CIA (WDBII) pedigree */
            if (river) source = tolower ((int)source);	/* Lower case c means river-lake */
            line = (h.area) ? 0 : 1;			/* Either Polygon (0) or Line (1) (if no area) */
            area = 0.1 * h.area;				/* Now im km^2 */
            f_area = 0.1 * h.area_full;			/* Now im km^2 */

            //OK = (!single || h.id == ID);
            first = 0;

            if (!msformat) c = kind[line];

            vxp->pFeatures[nFeatures].nVerts = h.n;
            vxp->pFeatures[nFeatures].pVerts = new QVector3D[ h.n ];
            vxp->pFeatures[nFeatures].pLonLat = new LonLatPair[ h.n ];


            for (int k = 0; k < h.n; k++)
            {
                if (fread ((void *)&p, (size_t)sizeof(struct POINT_GSHHS), (size_t)1, fp) != 1)
                {
                    //fprintf (stderr, "gshhs:  Error reading file %s for %s %d, point %d.\n", argv[1], name[line], h.id, k);
                    exit (EXIT_FAILURE);
                }
                if (flip)
                {
                    p.x = swabi4 ((unsigned int)p.x);
                    p.y = swabi4 ((unsigned int)p.y);
                }
                lon = p.x * GSHHS_SCL;
                if ((greenwich && p.x > max_east) || (h.west > 180000000)) lon -= 360.0;
                lat = p.y * GSHHS_SCL;
                LonLat2Point(lat, lon, &vxp->pFeatures[nFeatures].pVerts[k], 1.0f);
                vxp->pFeatures[nFeatures].pLonLat[k].lonmicro = p.x;
                vxp->pFeatures[nFeatures].pLonLat[k].latmicro = p.y;

            }

            max_east = 180000000;	/* Only Eurasia needs 270 */
            n_read = fread((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
            nFeatures++;
    }

    fclose (fp);

}

void gshhsData::LonLat2Point(float lat, float lon, QVector3D *pos, float radius)
{
        // lon -90..90
        // lat -180..180

        float	angX, angY;

        angX = lon * M_PI / 180.f;
        angY = lat * M_PI / 180.f;

        pos->setX(cosf(angY) * sinf(angX) * radius);
        pos->setY(sinf(angY) * radius);
        pos->setZ(cosf(angY) * cosf(angX) * radius);

}
