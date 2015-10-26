#include <QDebug>
#include <QApplication>
#include <QStyleFactory>
#include <QSurfaceFormat>

#include "mainwindow.h"
#include "segmentimage.h"
#include "options.h"
#include "gshhsdata.h"
#include <stdexcept>

#include <QMutex>

#define APPVERSION "1.1.1"
using namespace std;

QMutex g_mutex;

Options opts;
SegmentImage *imageptrs;
gshhsData *gshhsdata;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();


#ifdef NDEBUG
   // release mode code
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Release Debug: %s\n", localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(stderr, "Release Info: %s\n", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "Release Warning: %s\n", localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Release Critical: %s\n", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "Release Fatal: %s\n", localMsg.constData());
        abort();
    }
#else
  // debug mode code

    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug Debug: %s\n", localMsg.constData());
        //fprintf(stderr, "           : %s:%u\n", context.file, context.line);
        //fprintf(stderr, "           : %s:%u, %s\n", context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Debug Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Debug Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Debug Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Debug Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
#endif

}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);

    QApplication app(argc, argv);

    QStringList styles = QStyleFactory::keys();

    for (int i = 0; i < styles.size(); ++i)
             qInfo() << styles.at(i);

    app.setStyle(QStyleFactory::create("Fusion"));


    opts.Initialize();
    imageptrs = new SegmentImage();
    gshhsdata = new gshhsData();

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    if (QCoreApplication::arguments().contains(QStringLiteral("--multisample")))
        format.setSamples(4);

    QSurfaceFormat::setDefaultFormat(format);

    app.setApplicationName("EUMETCastView");
    app.setApplicationVersion(APPVERSION);

    app.setStyleSheet(
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
