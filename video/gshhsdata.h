#ifndef GSHHSDATA_H
#define GSHHSDATA_H
#include <QVector>
#include <QString>
#include <QVector2D>
#include "gshhs.h"
#include "pixgeoconversion.h"

struct LonLatPair {
        int lonmicro;
        int latmicro;
};

// vector-file
struct VxpFeature {
        int	 nVerts;
        QVector3D *pVerts;
        LonLatPair *pLonLat;
};

struct Vxp {
        int         nFeatures;
        VxpFeature  *pFeatures;
};

class gshhsData
{
public:
    gshhsData(QVector<QString> overlayfiles);
    ~gshhsData();

    Vxp	*vxp_data[3];
    QVector<QVector2D> geooverlay[3];
    void setupGeoOverlay(double longitude, qlonglong coff, qlonglong loff, double cfac, double lfac );

private:

    void Initialize(QVector<QString> overlayfiles);
    void load_gshhs(char *pFileName, int nTotFeatures, Vxp *vxp);
    int check_gshhs(char *pFileName);
    void LonLat2Point(float lat, float lon, QVector3D *pos, float radius);

};

#endif // GSHHSDATA_H
