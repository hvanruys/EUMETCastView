#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QtConcurrent/QtConcurrent>
#include <QUdpSocket>
#include "rssvideo.h"
//#include "qsun.h"
//#include "qobserver.h"

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

int main(int argc, char *argv[])
{

    QStringList datelist;
    QStringList pathlist;

    QApplication a(argc, argv); //use QApplication for font
    //QCoreApplication app(argc, argv);
    QStringList arglist = a.arguments();
    QString argsingleimage = "";
    if(arglist.count() > 1)
        argsingleimage = arglist.at(1);

    RSSVideo video("EUMETCastVideo.xml", argsingleimage);

    QDir temp;
    temp.mkdir("tempvideo");
    temp.mkdir("tempimages");

    QDir tempimages("./tempimages");
    QDir tempvideo("./tempvideo");

    QStringList filters;
    filters << "*.png";

    tempvideo.setNameFilters(filters);
    tempvideo.setFilter(QDir::Files | QDir::NoSymLinks);
    tempimages.setNameFilters(filters);
    tempimages.setFilter(QDir::Files | QDir::NoSymLinks);

    // delete the images in 'tempvideo'
    foreach(QFileInfo item, tempvideo.entryInfoList() )
    {
        QFile file(item.absoluteFilePath());
        if(file.exists())
            file.remove();
    }


    //    if(video.reader->daykindofimage.length() == 0)
    //    {
    //        video.sendMessages("Wrong parameters !");
    //        return 1;
    //    }

    if(video.reader->singleimage.length() == 0)
    {
        video.sendMessages("Start creating the video frames ...");
    }
    else
    {
        video.sendMessages("Start creating the image ...");
    }

    video.getDatePathVectorFromDir(&datelist, &pathlist);

    ////        for(int i = 0; i < datelist.count(); i++)
    ////        {
    ////            qDebug() << datelist.at(i) << pathlist.at(i);
    ////        }


    int threadcount = QThread::idealThreadCount();

    video.sendMessages(QString("ideal threadcount = %1").arg(threadcount));
    video.sendMessages(QString("datelist count = %1").arg(datelist.count()));

    int xmlthreadcount = video.reader->threadcount;
    if(xmlthreadcount == 0)
        threadcount = QThread::idealThreadCount();
    else
        threadcount = xmlthreadcount;

    if(datelist.count() == 0)
    {
        video.sendMessages("Warning : No segments found !");
        printf("Warning : No segments found !\n");
        return 0;
    }

    bool videofound = false;

    if(video.reader->singleimage.length() > 0)
    {
        for(int i = 0; i < datelist.count(); i++)
        {
            if(video.reader->singleimage == datelist.at(i))
            {
                video.compileImage(datelist.at(i), pathlist.at(i), i);
                videofound = true;
                break;
            }
        }
        if(!videofound)
            video.sendMessages(QString("--- singleimage %1 not found ! ---").arg(video.reader->singleimage));
        else
            video.sendMessages(QString("=== Image %1 %2 created ===").arg(video.reader->satname).arg(video.reader->singleimage));

    }
    else
    {
        QThreadPool::globalInstance()->setMaxThreadCount(threadcount);
        QFuture<void> future[datelist.count()];
        for(int i = 0; i < datelist.count(); i++)
        {
            future[i] = QtConcurrent::run(&RSSVideo::doCompileImage, &video, datelist.at(i), pathlist.at(i), i);
        }
        for(int i = 0; i < datelist.count(); i++)
        {
            future[i].waitForFinished();
            video.sendMessages(QString("image %1 of %2 is finished!").arg(i).arg(datelist.count()));
        }

        QString datestr = datelist.at(0).mid(0, 8);

        QStringList list = video.reader->ffmpegparameters.split(QLatin1Char(',')); //, Qt::SkipEmptyParts);

        video.sendMessages(QString("start ffmpeg with parameters %1").arg(video.reader->ffmpegparameters));


        QProcess process;
        process.setProgram("ffmpeg");
        process.setArguments(list);
        process.setStandardOutputFile("ffmpegouput.txt");
        process.setStandardErrorFile("ffmpegoutputerror.txt"); //QProcess::nullDevice());
        process.start();
        process.waitForFinished(-1);
        video.sendMessages(QString("=== The video is created ! ==="));
    }


    video.sendMessages("========== Pocessing ended ============== ");
    return 0; //a.exec();
}



