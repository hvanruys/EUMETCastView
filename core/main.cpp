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

#define APPVERSION "2.0.8"


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

    qInstallMessageHandler(myMessageOutput);

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

    QSurfaceFormat::setDefaultFormat(format);

    app.setApplicationName("EUMETCastView");
    app.setApplicationVersion(APPVERSION);

    //"QTabWidget::tab:disabled { width: 0; height: 0; margin: 0; padding: 0; border: none; }"

    // app.setStyleSheet(
    // "QTabWidget::tab:default {border-color: navy;}"
    // "QPushButton {"
    //     "border: 2px solid #8f8f91;"
    //     "border-radius: 6px;"
    //     "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
    //     "                           stop: 0 #f6f7fa, stop: 1 #dadbde);"
    //     "min-width: 80px;"
    // "}"
    // "QPushButton:pressed {"
    //     "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
    //                                       "stop: 0 #dadbde, stop: 1 #f6f7fa);"
    // "}"
    // "QPushButton:checked {"
    //     "background-color: rgb(100, 220, 100);"
    // "}"

    // "QPushButton:flat {"
    //     "border: none; /* no border for a flat push button */"
    // "}"
    // "QPushButton:default {"
    //     "border-color: navy; /* make the default button prominent */"
    // "}");

    app.setStyle(QStyleFactory::create("Fusion"));

    QFont new_font = app.font();
    new_font.setPointSize(opts.fontsize);
    new_font.setWeight(QFont::Medium);
    app.setFont( new_font );

    opts.setDarkMode(opts.darkmode);


    // QPalette newPalette;
    // newPalette.setColor(QPalette::Window,          QColor( 37,  37,  37));
    // newPalette.setColor(QPalette::WindowText,      Qt::white); //QColor(212, 212, 212));
    // newPalette.setColor(QPalette::Base,            QColor( 60,  60,  60));
    // newPalette.setColor(QPalette::AlternateBase,   QColor( 45,  45,  45));
    // newPalette.setColor(QPalette::PlaceholderText, QColor(127, 127, 127));
    // newPalette.setColor(QPalette::Text,           Qt::white); // QColor(212, 212, 212));
    // newPalette.setColor(QPalette::Button,          QColor( 45,  45,  45));
    // newPalette.setColor(QPalette::ButtonText,      QColor(212, 212, 212));
    // newPalette.setColor(QPalette::BrightText,      QColor(240, 240, 240));
    // newPalette.setColor(QPalette::Highlight,       QColor( 38,  79, 120));
    // newPalette.setColor(QPalette::HighlightedText, QColor(240, 240, 240));

    // newPalette.setColor(QPalette::Light,           QColor( 60,  60,  60));
    // newPalette.setColor(QPalette::Midlight,        QColor( 52,  52,  52));
    // newPalette.setColor(QPalette::Dark,            QColor( 30,  30,  30) );
    // newPalette.setColor(QPalette::Mid,             QColor( 37,  37,  37));
    // newPalette.setColor(QPalette::Shadow,          QColor( 0,    0,   0));
    // newPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    // app.setPalette(newPalette);

    // QPalette darkPalette;
    // darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    // darkPalette.setColor(QPalette::WindowText, Qt::white);
    // darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
    // darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    // darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    // darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    // darkPalette.setColor(QPalette::Text, Qt::white);
    // darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    // darkPalette.setColor(QPalette::ButtonText, Qt::white);
    // darkPalette.setColor(QPalette::BrightText, Qt::red);

    // darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218).lighter());
    // darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    // app.setPalette(darkPalette);

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

