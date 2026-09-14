#include "viewlog.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>
#include <QTextStream>

#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <hdf5.h>
#else
#include <hdf5/serial/hdf5.h>
#endif

#include <netcdf.h>

namespace
{
    QFile logfile;
    QTextStream logstream;
    QMutex logmutex;
    QtMessageHandler previoushandler = nullptr;
    bool installed = false;
    bool everopened = false;

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

    bool wanted(int argc, char *argv[])
    {
        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--logging") == 0)
                return true;
        }
        return false;
    }

    // What the box in the preferences was left at : /debugging/dologging in
    // EUMETCastView.ini, the same key Options reads and writes back at exit.
    // Read here rather than through Options because this runs before the
    // QApplication, and Options::Initialize() long after it. The ini sits in
    // the working directory, next to where the logfile goes.
    bool preferred()
    {
        QSettings settings(QStringLiteral("EUMETCastView.ini"), QSettings::IniFormat);
        return settings.value(QStringLiteral("/debugging/dologging"), false).toBool();
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

                // Line by line, not when the buffer happens to fill : the image
                // composition runs in threads and does not always come back, so
                // the last line in the file has to be where it stopped. Nothing
                // is written at all unless logging was asked for, which is what
                // pays for the flushing.
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

namespace
{
    bool startLogging()
    {
        if (installed)
            return true;

        // The working directory, which is where the application was started
        // from, and under the name it has always had.
        logfile.setFileName(QDir::current().absoluteFilePath("logging.txt"));

        // Emptied once a run, appended to when it is switched back on from the
        // preferences : what was logged before the switch is worth as much as
        // what comes after it.
        QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text |
                (everopened ? QIODevice::Append : QIODevice::Truncate);

        if (!logfile.open(mode))
        {
            // No handler is installed, so this still goes to stderr on its own.
            qWarning() << "Could not open the logfile" << logfile.fileName() << ":" << logfile.errorString();
            return false;
        }

        logstream.setDevice(&logfile);
        previoushandler = qInstallMessageHandler(handler);
        installed = true;
        everopened = true;
        return true;
    }

    void stopLogging()
    {
        if (!installed)
            return;

        // Out of the way first : a message from another thread while this runs
        // then goes straight to where it went before, instead of into a file
        // that is being closed underneath it.
        qInstallMessageHandler(previoushandler);
        previoushandler = nullptr;
        installed = false;

        QMutexLocker locker(&logmutex);
        logstream.flush();
        logfile.flush();
        logfile.close();
    }
}

void ViewLog::install(int argc, char *argv[])
{
    // The command line is for this run, the preference for every run : -l
    // switches it on without touching what is saved, and a run started
    // without it does what the box was left at.
    if (wanted(argc, argv) || preferred())
        startLogging();
}

void ViewLog::setActive(bool on)
{
    if (on == installed)
        return;

    if (on)
    {
        // The line goes in after the handler is in place, so it is the first
        // thing in the part of the file that was asked for.
        if (startLogging())
            qDebug() << "=== logging switched on ===";
    }
    else
    {
        // And this one before it comes out again, so the file says why it
        // stops instead of just ending.
        qDebug() << "=== logging switched off ===";
        stopLogging();
    }
}

bool ViewLog::isActive()
{
    return installed;
}

QString ViewLog::fileName()
{
    return logfile.isOpen() ? logfile.fileName() : QString();
}

void ViewLog::writeEnvironment(const QStringList &arglist)
{
    if (!installed)
        return;

    unsigned h5major = 0, h5minor = 0, h5release = 0;
    H5get_libversion(&h5major, &h5minor, &h5release);

    const char *pluginpath = getenv("HDF5_PLUGIN_PATH");
    const char *path = getenv("PATH");

    qDebug() << "=== EUMETCastView ===";
    qDebug() << "arguments         :" << arglist;
    qDebug() << "process id        :" << QCoreApplication::applicationPid();
    qDebug() << "working directory :" << QDir::currentPath();
    qDebug() << "application dir   :" << QCoreApplication::applicationDirPath();
    qDebug() << "logfile           :" << ViewLog::fileName();

    // The versions of what was loaded, not of what it was built against. A run
    // that works from one directory and not from another is usually a different
    // set of dll's on PATH, and this is where that shows. MainWindow prints the
    // HDF5 plugin path list it ends up with as it comes up, further down.
    qDebug() << "Qt                : built" << QT_VERSION_STR << " running" << qVersion();
    qDebug() << "netCDF            :" << nc_inq_libvers();
    qDebug() << "HDF5              :" << QString("%1.%2.%3").arg(h5major).arg(h5minor).arg(h5release);
    qDebug() << "FCIDECOMP (32018) :" << (H5Zfilter_avail(32018) > 0 ? "available" : "NOT available");
    qDebug() << "HDF5_PLUGIN_PATH  :" << (pluginpath == nullptr ? QString("not set") : QString::fromLocal8Bit(pluginpath));
    qDebug() << "PATH              :" << (path == nullptr ? QString("not set") : QString::fromLocal8Bit(path));
    qDebug() << "=====================";
}

void ViewLog::shutdown()
{
    stopLogging();
}
