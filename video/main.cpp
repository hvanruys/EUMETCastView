#include <QGuiApplication>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QUdpSocket>
//#include "rssvideo.h"
#include "videomaker.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


void RenamePNGFiles()
{
    QDir workingdir(".");
    QStringList filters;
    filters << "*.png";
    workingdir.setNameFilters(filters);
    QFileInfoList fileinfolist;

    workingdir.setFilter(QDir::Files | QDir::NoSymLinks);
    workingdir.setSorting(QDir::Name);
    fileinfolist = workingdir.entryInfoList();
    for (int i = 0; i < fileinfolist.size(); ++i)
    {
        QFile pngfile(fileinfolist.at(i).fileName());
        QString newName = fileinfolist.at(i).fileName().mid(0, 4) + fileinfolist.at(i).fileName().mid(17);
        if(pngfile.rename(newName))
            qDebug() << QString("renaming png files : %1").arg(fileinfolist.at(i).fileName());
    }

}

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QStringList arglist = app.arguments();
    QString timestamp = "";
    QString imagenbrstr = "";
    if(arglist.count() == 3)
    {
        imagenbrstr = arglist.at(1);
        timestamp = arglist.at(2);
    }
    qDebug() << arglist;

    // date = "001" "002" ..."144" for MET_12
    // date = "YYYYMMDDhhmm" for MET_11/10/9
    // arglist => "89" "090" for MET_12
    // arglist => "50" "202511081320"
    VideoMaker video("EUMETCastVideo.json", timestamp);

    if(video.reader->shortname == "MET_12")
        video.compileImageMTG(timestamp, imagenbrstr.toInt());
    else
        video.compileImage(timestamp, imagenbrstr.toInt());

    //         QString datestr = datelist.at(0).mid(0, 8);

    //         QStringList list = video.reader->ffmpegparameters.split(QLatin1Char(',')); //, Qt::SkipEmptyParts);

    //         video.sendMessages(QString("start ffmpeg with parameters %1").arg(video.reader->ffmpegparameters));


    //         QProcess process;
    //         process.setProgram("ffmpeg");
    //         process.setArguments(list);
    //         process.setStandardOutputFile("ffmpegouput.txt");
    //         process.setStandardErrorFile("ffmpegoutputerror.txt"); //QProcess::nullDevice());
    //         process.start();
    //         process.waitForFinished(-1);
    //         video.sendMessages(QString("=== The video is created ! ==="));

    return 0;
}
