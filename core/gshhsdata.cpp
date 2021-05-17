#include <QDebug>
#include "gshhsdata.h"
#include "options.h"

extern Options opts;

// c = crude
// l = low
// i = intermediate
// h = high
// f = full


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

    if(opts.appdir_env == "")
        Initialize(opts.gshhsglobe1, opts.gshhsglobe2, opts.gshhsglobe3, opts.gshhsoverlay1, opts.gshhsoverlay2, opts.gshhsoverlay3);
    else
        Initialize(opts.appdir_env + "/" + opts.gshhsglobe1, opts.appdir_env + "/" + opts.gshhsglobe2, opts.appdir_env + "/" + opts.gshhsglobe3,
                   opts.appdir_env + "/" + opts.gshhsoverlay1, opts.appdir_env + "/" + opts.gshhsoverlay2, opts.appdir_env + "/" + opts.gshhsoverlay3);


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
    version = (h.flag >> 8) & 255;
    flip = (version != GSHHS_DATA_RELEASE);	/* Take as sign that byte-swabbing is needed */


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
    int k, line, max_east = 270000000, info, single, error, ID, flip;
    int  OK, level, version, greenwich, river, src, msformat = 0, first = 1;
    size_t n_read;
    struct POINT_GSHHS p;
    struct GSHHS h;
    //Vxp *vxp = new Vxp;

    int nFeatures = 0;

    info = single = error = ID = 0;


    if ((fp = fopen (pFileName, "rb")) == NULL ) {
            qDebug() << QString( "gshhs:  Could not find file %1.").arg(pFileName);
            return; // exit (EXIT_FAILURE);
    }

    n_read = fread ((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
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
