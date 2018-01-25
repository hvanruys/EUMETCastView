#include "msgfileaccess.h"
#include "options.h"
#include <QString>
#include <QStringList>
#include <QDir>
#include <QDebug>
#include <stdexcept>

using namespace std;

extern Options opts;


#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\\"

// Pad the string 'base' with trailing underscores to ensure it's at least
// final_len characters long

static QString deunderscore(const QString str)
{
    quint16 pos = str.size() - 1;
    while (pos > 0 && str.at(pos) == '_')
        --pos;
    return str.mid(0, pos + 1);
}

// dir/res:prodid1:prodid2:time
bool isValid(const QString filename)
{
    if (filename.size() < 3) return false;

    // Segments
    if (filename.mid(filename.size() - 3) == "-C_")
        return true;

    // check that it contains at least 3 ':' signs
    if(filename.count(":") != 3)
            return false;
    return true;
}

MsgFileAccess::MsgFileAccess(const QString filename)
{
    parse(filename);
}

MsgFileAccess::MsgFileAccess(const MsgFileAccess fa, const QString chan)
{
    parse(fa, chan);
}

void MsgFileAccess::parse(const MsgFileAccess fa, const QString chan)
{
    directory = fa.directory;
    resolution = fa.resolution;
    productid1 = fa.productid1;
    productid2 = chan;
    timing = fa.timing;
}

void MsgFileAccess::parse(const QString filename)
{
    // Split directory name
    int beg;
    int end = filename.lastIndexOf("/");
    if (end == filename.size())
    {
        directory = ".";
        beg = 0;
    }
    else
    {
        directory = filename.mid(0, end);
        if (directory.size() == 0) directory = "/";
        beg = end + 1;
    }

    if (filename.size() < 3)
        throw std::runtime_error(filename.toStdString() + " is not a valid xRIT file name");

    if (filename.mid(filename.size() - 3) == "-C_")
    {
        // Parse segment filename [directory/]H-000-MSG1__-MSG1________-HRV______-000018___-200611141200-C_
        if ((end = filename.indexOf("-", beg)) == filename.size())
            throw std::runtime_error("XRIT segment " + filename.toStdString() + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");

        resolution = filename.mid(beg, end-beg);

        // Skip first number
        beg = end + 1;
        if ((end = filename.indexOf("-", beg)) == filename.size())
            throw std::runtime_error("XRIT segment " + filename.toStdString() + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");

        // Skip string (satellite name?)
        beg = end + 1;
        if ((end = filename.indexOf("-", beg)) == filename.size())
            throw std::runtime_error("XRIT segment " + filename.toStdString() + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");
        satname = deunderscore(filename.mid(beg, end-beg));

        beg = end + 1;
        if ((end = filename.indexOf("-", beg)) == filename.size())
            throw std::runtime_error("XRIT segment " + filename.toStdString() + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");
        productid1 = deunderscore(filename.mid(beg, end-beg));

        beg = end + 1;
        if ((end = filename.indexOf("-", beg)) == filename.size())
            throw std::runtime_error("XRIT segment " + filename.toStdString() + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");
        productid2 = deunderscore(filename.mid(beg, end-beg));

        // Skip segment number
        beg = end + 1;
        if ((end = filename.indexOf("-", beg)) == filename.size())
            throw std::runtime_error("XRIT segment " + filename.toStdString() + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");

        beg = end + 1;
        if ((end = filename.indexOf("-", beg)) == filename.size())
            throw std::runtime_error("XRIT segment " + filename.toStdString() + " is not in the form [directory/]resolution-nnn-xxxxxx-productid1-productid2-datetime-C_");
        timing = deunderscore(filename.mid(beg, end-beg));
    } else {
        // Parse shortened form [directory/]resolution:productid1:productid2:datetime
        if ((end = filename.indexOf(":", beg)) == filename.size())
            throw std::runtime_error("XRIT name " + filename.toStdString() + " is not in the form [directory/]resolution:productid1:productid2:datetime");
        resolution = filename.mid(beg, end-beg);

        beg = end + 1;
        if ((end = filename.indexOf(":", beg)) == filename.size())
            throw std::runtime_error("XRIT name " + filename.toStdString() + " is not in the form [directory/]resolution:productid1:productid2:datetime");
        productid1 = filename.mid(beg, end-beg);

        beg = end + 1;
        if ((end = filename.indexOf(":", beg)) == filename.size())
            throw std::runtime_error("XRIT name " + filename.toStdString() + " is not in the form [directory/]resolution:productid1:productid2:datetime");
        productid2 = filename.mid(beg, end-beg);

        beg = end + 1;
        timing = filename.mid(beg);
    }
}


QString MsgFileAccess::prologueFile()
{
  //"H-000-MSG3__-MSG3________-_________-PRO______-201311050930-__"
  //"L-000-MTP___-MET7________-00_7_057E-PRO______-201404010900-__"
    QString filename;

    if(productid1 == "MET7")
    {
        filename = resolution
                + "-" "000" "-" + satname.leftJustified(6, '_') + "-" // Split to avoid warnings on trigraphs
                + productid1.leftJustified(12, '_') + "-" + productid2.leftJustified(9, '_') + "-"
                     + "PRO______-"
                     + timing
                     + "-__";
    }
    else
    {
        filename = resolution
                + "-" "000" "-" + satname.leftJustified(6, '_') + "-" // Split to avoid warnings on trigraphs
                + productid1.leftJustified(12, '_') + "-" + "_________" + "-"
                     + "PRO______-"
                     + timing
                     + "-__";

    }

  qDebug() << QString("de prologuefile = %1").arg(this->directory + "/" + filename);
  qDebug() << QString("de productid1 = %1").arg(productid1);
  qDebug() << QString("de productid2 = %1").arg(productid2);
  qDebug() << QString("de satname = %1").arg(satname);
  QFile prologue(this->directory + "/" + filename);
  if(prologue.exists())
      return filename;
  else
      return "";

}

QString MsgFileAccess::epilogueFile()
{
    //H-000-MSG2__-MSG2_RSS____-_________-EPI______-201311080745-__
    //H-000-MSG3__-MSG3________-_________-PRO______-201311050930-__"

    QString filename = resolution
                  + "-" "000" "-" + satname.leftJustified(6, '_') + "-" // Split to avoid warnings on trigraphs
                  + productid1.leftJustified(12, '_') + "-_________-"
                       + "EPI______-"
                       + timing
                       + "-__";

    QFile prologue(this->directory + "/" + filename);
    if(prologue.exists())
        return filename;
    else
        return "";

}

QStringList MsgFileAccess::segmentFiles()
{

  QString filename = resolution
           + "-???" "-??????" "-"	// Split to avoid warnings on trigraphs
           + productid1.leftJustified(12, '_') + "-"
          + productid2.leftJustified(9, '_') + "-"
           + "0?????___" + "-"
           + timing + "-" + "C_";


  QStringList msgfiles = this->globit(this->directory, filename);
  return msgfiles;
}

QStringList MsgFileAccess::globit(QString filepath, QString filepattern)
{

    QDir meteosatdir(filepath);
    meteosatdir.setFilter(QDir::Files | QDir::NoSymLinks);
    //meteosatdir.setSorting(QDir::Name);


    QStringList strlist = meteosatdir.entryList();
    QStringList strlistout;
    QStringList ret;

    qDebug() << QString("strlist = %1 strlist out = %2").arg(strlist.count()).arg(strlistout.count());
    QStringList::Iterator itc = strlist.begin();

    while( itc != strlist.end() )
    {
        if(meteosatdir.match(filepattern, *itc))
            strlistout.append(*itc);
        itc++;
    }
    qDebug() << QString("strlist = %1 strlist out = %2").arg(strlist.count()).arg(strlistout.count());

    return strlistout;

}
