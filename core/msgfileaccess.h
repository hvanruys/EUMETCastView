#ifndef MSGFILEACCESS_H
#define MSGFILEACCESS_H
#include <QString>

class MsgFileAccess
{
public:

    MsgFileAccess() : directory(".") {}
    MsgFileAccess(const QString filename);
    MsgFileAccess(const MsgFileAccess fa, const QString chan);
    MsgFileAccess(const QString directory, const QString resolution, const QString productid1, const QString productid2, const QString timing);

    QString directory;
    QString resolution;
    QString productid1;
    QString productid2;
    QString timing;
    QString satname;


    /// Initialize parsing a file name
    void parse(const QString filename);

    /// Initialize as a different channel of an existing FileAccess
    void parse(const MsgFileAccess fa, const QString chan);
    void parse(const QString directory, const QString resolution, const QString productid1, const QString productid2, const QString timing);
    //void ensureComplete() const;

    QString prologueFile() const;
    QString epilogueFile() const;
    QStringList segmentFiles() const;
    QStringList globit(QString filepath, QString filepattern) const;
};

#endif
