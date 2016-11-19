#include <QDebug>
#include <QApplication>
#include <QStyleFactory>
#include <QSurfaceFormat>

#include "mainwindow.h"
#include "segmentimage.h"
#include "options.h"
#include "poi.h"
#include "gshhsdata.h"
#include <stdexcept>

#include <QMutex>

#define APPVERSION "1.1.7"
using namespace std;

QMutex g_mutex;

Options opts;
Poi poi;
SegmentImage *imageptrs;
gshhsData *gshhsdata;
QFile loggingFile;
QTextStream out(&loggingFile);
bool doLogging;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{


#ifdef NDEBUG
   // release mode code
    QString strout;
    switch (type) {
    case QtDebugMsg:
        strout = "Release Debug: " + msg + "\n";
        break;
    case QtInfoMsg:
        strout = "Release Info: " + msg + "\n";
        break;
    case QtWarningMsg:
        strout = "Release Warning: " + msg + "\n";
        break;
    case QtCriticalMsg:
        strout = "Release Critical: " + msg + "\n";
        break;
    case QtFatalMsg:
        strout = "Release Fatal: " + msg + "\n";
        abort();
    }

    if(doLogging)
    {
        out << strout;
        out.flush();
    }
    else
        fprintf(stderr, strout.toStdString().c_str());

#else
  // debug mode code
    QString strout;
    switch (type) {
    case QtDebugMsg:
        strout = "Debug Debug: " + msg + "\n";
        break;
    case QtInfoMsg:
        strout = "Debug Info: " + msg + "\n";
        break;
    case QtWarningMsg:
        strout = "Debug Warning: " + msg + "\n";
        break;
    case QtCriticalMsg:
        strout = "Debug Critical: " + msg + "\n";
        break;
    case QtFatalMsg:
        strout = "Debug Fatal: " + msg + "\n";
        abort();
    }

    if(doLogging)
    {
        out << strout;
        out.flush();
    }
    else
        fprintf(stderr, strout.toStdString().c_str());


#endif

}

int main(int argc, char *argv[])
{
    doLogging = false;
    loggingFile.setFileName("logging.txt");
    if (!loggingFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return 0;

    qInstallMessageHandler(myMessageOutput);

    QApplication app(argc, argv);

    QStringList styles = QStyleFactory::keys();

    for (int i = 0; i < styles.size(); ++i)
             qInfo() << styles.at(i);

    app.setStyle(QStyleFactory::create("Fusion"));

    if (QCoreApplication::arguments().contains(QStringLiteral("--logging")) ||
        QCoreApplication::arguments().contains(QStringLiteral("-l")) )
        doLogging = true;

    opts.Initialize();
    poi.Initialize();

    imageptrs = new SegmentImage();
    gshhsdata = new gshhsData();

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    if (QCoreApplication::arguments().contains(QStringLiteral("--multisample")))
        format.setSamples(4);

    QSurfaceFormat::setDefaultFormat(format);

    app.setApplicationName("EUMETCastView");
    app.setApplicationVersion(APPVERSION);

//    "QTabWidget::tab:disabled { width: 0; height: 0; margin: 0; padding: 0; border: none; }"

    app.setStyleSheet(
    "QTabWidget::tab:default {border-color: navy;}"
    "QPushButton {"
        "border: 2px solid #8f8f91;"
        "border-radius: 6px;"
        "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
        "                           stop: 0 #f6f7fa, stop: 1 #dadbde);"
        "min-width: 80px;"
    "}"
    "QPushButton:pressed {"
        "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                          "stop: 0 #dadbde, stop: 1 #f6f7fa);"
    "}"
    "QPushButton:checked {"
        "background-color: rgb(100, 220, 100);"
    "}"

    "QPushButton:flat {"
        "border: none; /* no border for a flat push button */"
    "}"
    "QPushButton:default {"
        "border-color: navy; /* make the default button prominent */"
    "}");


#ifndef QT_NO_OPENGL

    MainWindow mw;
    mw.setWindowIcon(QIcon(":/icons/icons/300px-Orthographic_projection_SW.png"));
    mw.setContentsMargins(0,0,0,0);

    mw.show();

#else
    QLabel note("OpenGL Support required");
    note.show();
#endif
    return app.exec();
}
