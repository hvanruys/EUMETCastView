#ifndef MSGFILEACCESS_H
#define MSGFILEACCESS_H
#include <QString>

class MsgFileAccess
{
public:

    MsgFileAccess() : directory(".") {}
    MsgFileAccess(const QString filename);
    MsgFileAccess(const MsgFileAccess fa, const QString chan);

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

    //void ensureComplete() const;

    const QString prologueFile();
    QString epilogueFile();
    QStringList segmentFiles();
    QStringList globit(QString filepath, QString filepattern);



};

#endif
