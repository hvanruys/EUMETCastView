#ifndef VIEWLOG_H
#define VIEWLOG_H

#include <QString>
#include <QStringList>

// Everything EUMETCastView says through qDebug(), qWarning(), qCritical() and
// qFatal() written to a file of its own - but only when -l or --logging is on
// the command line, when the box in the preferences was left ticked at the
// last exit, or when it is switched on afterwards in the preferences. Without
// any of these nothing is installed, no file is opened and every message goes
// exactly where it went before.
//
// On Windows the application is built for the GUI subsystem and so has no
// console of its own : its output was only ever visible when it was started
// from a cmd window that then had to stay open in front of it. This is what
// replaces that window.
namespace ViewLog
{
    // Opens logging.txt in the working directory and sends the message stream
    // there, if -l or --logging is among the arguments or /debugging/dologging
    // in EUMETCastView.ini - next to it, in the same directory - is true. Call
    // it before the QApplication, so that what Qt itself says on the way up is
    // kept.
    void install(int argc, char *argv[]);

    // Switches the logfile on and off while the application runs, which is what
    // the checkbox in the preferences does. Switching it on again appends to
    // the file rather than emptying it. Whether it is on at the next start is
    // Options' business : opts.doLogging goes back into the ini at exit.
    void setActive(bool on);

    // True when the messages are going to the file at this moment.
    bool isActive();

    // What this run is and what it is running with : the arguments, the
    // directories, and the versions of Qt, netCDF and HDF5 it actually loaded,
    // with PATH and HDF5_PLUGIN_PATH behind them. Needs a QCoreApplication, and
    // does nothing when the run is not being logged.
    void writeEnvironment(const QStringList &arglist);

    // Where install() put the file, empty when there is none.
    QString fileName();

    // Flushes and closes the file and puts back the handler that was there
    // before. Called on the way out.
    void shutdown();
}

#endif // VIEWLOG_H
