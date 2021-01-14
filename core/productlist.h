#ifndef PRODUCTLIST_H
#define PRODUCTLIST_H
#include <QString>
#include <segment.h>

struct ProductList
{
    QString status;
    QString completebasename;
    QString uuid;
    QString size;
    QString band_or_quicklook;
    QString absoluteproductpath;
    QString absolutepath;
    QString filename;
};

#endif // PRODUCTLIST_H
