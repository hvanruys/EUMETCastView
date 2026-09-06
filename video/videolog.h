#ifndef VIDEOLOG_H
#define VIDEOLOG_H

#include <QString>
#include <QStringList>

// Everything EUMETCastVideo says through qDebug(), qWarning(), qCritical() and
// qFatal() - which is everything it says, since sendMessages() logs what it
// puts on the udp socket as well - written to a file of its own.
//
// The process is started by EUMETCastView and has no console of its own on
// Windows, so its output went nowhere. It also dies from time to time with an
// access violation, which is why every line is flushed as it is written : the
// last line in the file is where the process was when it stopped.
namespace VideoLog
{
    // Opens templogs/EUMETCastVideo_<tag>.log under the working directory and
    // sends the message stream there. tag is the image number, so the processes
    // that run next to each other do not write over one another. Call it before
    // the QGuiApplication, so that what Qt itself says on the way up is kept.
    void install(const QString &tag);

    // What this process is and what it is running with : the arguments, the
    // directories, and the versions of Qt, netCDF and HDF5 it actually loaded,
    // with PATH and HDF5_PLUGIN_PATH behind them. Needs a QCoreApplication.
    void writeEnvironment(const QStringList &arglist);

    // Where install() put the file, empty if it could not be opened.
    QString fileName();
}

#endif // VIDEOLOG_H
