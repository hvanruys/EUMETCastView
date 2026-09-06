#include "videolog.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

#include <cstdlib>

#include <hdf5.h>
#include <netcdf.h>

namespace
{
    QFile logfile;
    QTextStream logstream;
    QMutex logmutex;
    QtMessageHandler previoushandler = nullptr;

    QString typeName(QtMsgType type)
    {
        switch (type)
        {
        case QtDebugMsg:    return QStringLiteral("D");
        case QtInfoMsg:     return QStringLiteral("I");
        case QtWarningMsg:  return QStringLiteral("W");
        case QtCriticalMsg: return QStringLiteral("C");
        case QtFatalMsg:    return QStringLiteral("F");
        }
        return QStringLiteral("?");
    }

    void handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        {
            QMutexLocker locker(&logmutex);

            if (logfile.isOpen())
            {
                logstream << QDateTime::currentDateTimeUtc().toString("yyyy-MM-ddTHH:mm:ss.zzz")
                          << " " << typeName(type) << " " << msg;

                // Where it came from is only filled in for warnings and worse,
                // and only in a build that keeps it.
                if (type != QtDebugMsg && context.file != nullptr)
                    logstream << "   (" << context.file << ":" << context.line << ")";

                logstream << "\n";

                // Line by line, not when the buffer happens to fill : a process
                // that is killed off mid image has to leave its last line behind.
                logstream.flush();
                logfile.flush();
            }
        }

        // Whatever the message did before - stderr, and the abort at the end of
        // a qFatal - it keeps doing.
        if (previoushandler != nullptr)
            previoushandler(type, context, msg);
    }
}

void VideoLog::install(const QString &tag)
{
    QString name = tag.trimmed();
    bool isnumber = false;
    int imagenbr = name.toInt(&isnumber);

    // The image number, in the shape the png it writes carries, so a log and a
    // frame are easy to lay next to each other. Without one, the process id,
    // which at least keeps two runs apart.
    if (isnumber)
        name = QString("%1").arg(imagenbr, 4, 10, QChar('0'));
    else if (name.isEmpty())
        name = QString("pid%1").arg(QCoreApplication::applicationPid());

    QDir dir = QDir::current();
    if (dir.mkpath("templogs"))
        dir.cd("templogs");

    logfile.setFileName(dir.absoluteFilePath(QString("EUMETCastVideo_%1.log").arg(name)));

    if (!logfile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        // No handler is installed, so this still goes to stderr on its own.
        qWarning() << "Could not open the logfile" << logfile.fileName() << ":" << logfile.errorString();
        return;
    }

    logstream.setDevice(&logfile);
    previoushandler = qInstallMessageHandler(handler);
}

QString VideoLog::fileName()
{
    return logfile.isOpen() ? logfile.fileName() : QString();
}

void VideoLog::writeEnvironment(const QStringList &arglist)
{
    unsigned h5major = 0, h5minor = 0, h5release = 0;
    H5get_libversion(&h5major, &h5minor, &h5release);

    const char *pluginpath = getenv("HDF5_PLUGIN_PATH");
    const char *path = getenv("PATH");

    qDebug() << "=== EUMETCastVideo ===";
    qDebug() << "arguments         :" << arglist;
    qDebug() << "process id        :" << QCoreApplication::applicationPid();
    qDebug() << "working directory :" << QDir::currentPath();
    qDebug() << "application dir   :" << QCoreApplication::applicationDirPath();
    qDebug() << "logfile           :" << VideoLog::fileName();

    // The versions of what was loaded, not of what it was built against. A run
    // that works from one directory and not from another is usually a different
    // set of dll's on PATH, and this is where that shows.
    qDebug() << "Qt                : built" << QT_VERSION_STR << " running" << qVersion();
    qDebug() << "netCDF            :" << nc_inq_libvers();
    qDebug() << "HDF5              :" << QString("%1.%2.%3").arg(h5major).arg(h5minor).arg(h5release);
    qDebug() << "FCIDECOMP (32018) :" << (H5Zfilter_avail(32018) > 0 ? "available" : "NOT available");
    qDebug() << "HDF5_PLUGIN_PATH  :" << (pluginpath == nullptr ? QString("not set") : QString::fromLocal8Bit(pluginpath));
    qDebug() << "PATH              :" << (path == nullptr ? QString("not set") : QString::fromLocal8Bit(path));
    qDebug() << "======================";
}
