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

using namespace std;

QMutex g_mutex;

Options opts;
SegmentImage *imageptrs;
gshhsData *gshhsdata;


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QStringList styles = QStyleFactory::keys();

    for (int i = 0; i < styles.size(); ++i)
             qDebug() << styles.at(i);

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
    app.setApplicationVersion("1.0");

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
