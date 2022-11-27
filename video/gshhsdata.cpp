#include <QDebug>
#include <QVector3D>
#include "gshhsdata.h"

// c = crude
// l = low
// i = intermediate
// h = high
// f = full


gshhsData::gshhsData(QVector<QString> overlayfiles)
{


    for(int i = 0; i < 3; i++)
    {
        vxp_data[i] = new Vxp;
        vxp_data[i]->nFeatures = 0;
    }

    Initialize(overlayfiles);


}


gshhsData::~gshhsData()
{
}


void gshhsData::Initialize(QVector<QString> overlayfiles)
{

    for(int i = 0; i < overlayfiles.count(); i++)
    {
        QByteArray ba = overlayfiles.at(i).toUtf8();
        if (ba.length() > 0)
        {
            int nFeatures = check_gshhs(ba.data());
            load_gshhs(ba.data(), nFeatures, vxp_data[i]);
            qDebug() << QString("gshhsdata[%1] loaded nFeatures = %2").arg(i).arg(nFeatures);
        }
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

void gshhsData::setupGeoOverlay(double longitude, qlonglong coff, qlonglong loff, double cfac, double lfac)
{

    int col, save_col;
    int row, save_row;
    bool first = true;

    double lat_deg;
    double lon_deg;
    int ret;
    double sub_lon;

    pixgeoConversion pixconv;

    this->geooverlay[0].clear();
    this->geooverlay[1].clear();
    this->geooverlay[2].clear();

    for(int k = 0; k < 3; k++)
    {
        first = true;

        for (int i=0; i<this->vxp_data[k]->nFeatures; i++)
        {
            for (int j=0; j<this->vxp_data[k]->pFeatures[i].nVerts; j++)
            {
                lat_deg = this->vxp_data[k]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = this->vxp_data[k]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                if(lon_deg < 90.0 || lon_deg > -90.0)
                {
                    {
                        ret = pixconv.geocoord2pixcoord(longitude, lat_deg, lon_deg, coff, loff, cfac, lfac, &col, &row);

                    }
                    if(ret == 0)
                    {
                        if (first)
                        {
                            first = false;
                            save_col = col;
                            save_row = row;
                            this->geooverlay[k].append(QVector2D(-1, -1));
                        }
                        else
                        {
                            save_col = col;
                            save_row = row;
                        }

                        this->geooverlay[k].append(QVector2D(col, row));

                    }
                    else
                        first = true;
                }
            }
            first = true;
        }

    }

    qDebug() << QString("geooverlay[0] length = %1").arg(this->geooverlay[0].count());
    qDebug() << QString("geooverlay[1] length = %1").arg(this->geooverlay[1].count());
    qDebug() << QString("geooverlay[2] length = %1").arg(this->geooverlay[2].count());

}

