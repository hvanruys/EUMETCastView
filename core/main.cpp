#include <QDebug>
#include <QApplication>
#include <QStyleFactory>
#include <QSurfaceFormat>

#include "mainwindow.h"
#include "segmentimage.h"
#include "options.h"
#include "poi.h"
#include "gshhsdata.h"
#include "satellite.h"

//#include <stdexcept>

#include <QMutex>

#define APPVERSION "2.1.2"


using namespace std;

QMutex g_mutex;

Options opts;
Poi poi;
SegmentImage *imageptrs;
gshhsData *gshhsdata;
QFile loggingFile;
QTextStream outlogging(&loggingFile);
QNetworkAccessManager networkaccessmanager;
SatelliteList satellitelist;

bool ptrimagebusy;

// Every now and then a masterpiece like this comes out, and the world is gifted with a few hours of hope for the human race,
// before dropping back into it's usual chaos.

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stdout, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }

    fflush(stderr);
    fflush(stdout);
}

// void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
// {

//     QString strout = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
//     switch (type) {
//     case QtDebugMsg:
//         strout += "Debug: " + msg + "\n";
//         break;
//     case QtInfoMsg:
//         strout += "Info: " + msg + "\n";
//         break;
//     case QtWarningMsg:
//         strout += "Warning: " + msg + "\n";
//         break;
//     case QtCriticalMsg:
//         strout += "Critical: " + msg + "\n";
//         break;
//     case QtFatalMsg:
//         strout += "Fatal: " + msg + "\n";
//         outlogging << strout;
//         fprintf(stderr, "%s", strout.toStdString().c_str());
//         abort();
//     }

//     if(opts.doLogging)
//     {
//         outlogging << strout;
//         outlogging.flush();
//     }

//     fprintf(stderr, "%s", strout.toStdString().c_str());

// }


int main(int argc, char *argv[])
{
    ptrimagebusy = false;

    QByteArray val("1");
    qputenv("HDF5_DISABLE_VERSION_CHECK", val);

    QCoreApplication::addLibraryPath(".");

    QApplication app(argc, argv);

    QStringList styles = QStyleFactory::keys();

    for (int i = 0; i < styles.size(); ++i)
             qInfo() << styles.at(i);

    opts.Initialize();
    poi.Initialize();

    if (QCoreApplication::arguments().contains(QStringLiteral("--logging")) ||
        QCoreApplication::arguments().contains(QStringLiteral("-l")) )
        opts.doLogging = true;

    //qInstallMessageHandler(myMessageOutput);

    if(opts.doLogging)
    {
        loggingFile.setFileName("logging.txt");
        if (!loggingFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return 0;
        //qInstallMessageHandler(myMessageOutput);
    }

    if (QCoreApplication::arguments().contains(QStringLiteral("--noopengl")) ||
        QCoreApplication::arguments().contains(QStringLiteral("-nogl")) )
        opts.doOpenGL = false;
    else
        opts.doOpenGL = true;

    satellitelist.Initialize();

    imageptrs = new SegmentImage();
    gshhsdata = new gshhsData();

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    //if (QCoreApplication::arguments().contains(QStringLiteral("--multisample")))
        format.setSamples(4);

    // Ask for the context the shaders were written against. Every shader under
    // core/shader is #version 330, which is OpenGL 3.3, and without this the
    // driver hands out whatever it likes by default - on Linux a compatibility
    // profile, which Mesa capped at OpenGL 3.0 and GLSL 1.30 for years. The
    // globe then failed on 20.04 with "GLSL 3.30 is not supported" while
    // working on newer distributions whose Mesa happens to offer a higher
    // compatibility profile. It was never a driver limit: the same machine
    // reported core profile 4.5.
    //
    // Core rather than compatibility because that is the profile drivers have
    // offered 3.3 on the longest. Nothing here needs the fixed-function
    // pipeline - the drawing classes are all VAO, VBO and shader already.
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);

    QSurfaceFormat::setDefaultFormat(format);

    app.setApplicationName("EUMETCastView");
    app.setApplicationVersion(APPVERSION);

    app.setStyle(QStyleFactory::create("Fusion"));

    QFont new_font = app.font();
    new_font.setPointSize(opts.fontsize);
    new_font.setWeight(QFont::Medium);
    app.setFont( new_font );

    opts.setDarkMode(opts.darkmode);


#ifndef QT_NO_OPENGL

    MainWindow mw;
    mw.setWindowIcon(QIcon(":/icons/300px-Orthographic_projection_SW.png"));
    mw.setContentsMargins(0,0,0,0);

    mw.show();

#else
    QLabel note("OpenGL Support required");
    note.show();
#endif
    return app.exec();
}

