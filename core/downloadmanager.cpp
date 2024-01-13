/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include "downloadmanager.h"

#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QDebug>
#include <QDir>

#include <stdio.h>

// https://celestrak.org/NORAD/elements/gp.php?GROUP=weather&FORMAT=tle

extern QNetworkAccessManager networkaccessmanager;

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent), downloadedCount(0), totalCount(0)
{
}

void DownloadManager::append(const QStringList &urlList)
{
    foreach (QString url, urlList)
        append(QUrl::fromEncoded(url.toLocal8Bit()));

    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, SIGNAL(finished()));
}

void DownloadManager::append(const QUrl &url)
{
    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, SLOT(startNextDownload()));

    downloadQueue.enqueue(url);
    ++totalCount;
}

void DownloadManager::clearqueue()
{
    qDebug() << QString("downloadqueue aantal = %1").arg(downloadQueue.count());
    downloadQueue.clear();
    totalCount = 0;
    downloadedCount = 0;
}

QString DownloadManager::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();
    if (basename.isEmpty())
        basename = "download";

    return "./" + basename;
}

void DownloadManager::startNextDownload()
{
    if (downloadQueue.isEmpty()) {
        emit finished(QString("%1/%2 files downloaded successfully").arg(downloadedCount).arg(totalCount));
        return;
    }

    QUrl url = downloadQueue.dequeue();

    QString filename = saveFileName(url);

    output.setFileName(filename);
    if (!output.open(QIODevice::WriteOnly)) {
        emit startnext( QString("Problem opening save file '%1' for download '%2': %3\n").
                        arg(filename).arg(url.toEncoded().constData()).arg(qPrintable(output.errorString())));

        startNextDownload();
        return;                 // skip this download
    }

    QNetworkRequest request(url);
    currentDownload = networkaccessmanager.get(request);
    connect(currentDownload, SIGNAL(downloadProgress(qint64,qint64)), SLOT(downloadProgress(qint64,qint64)));
    connect(currentDownload, SIGNAL(finished()), SLOT(downloadFinished()));
    connect(currentDownload, SIGNAL(readyRead()), SLOT(downloadReadyRead()));

    // prepare the output
    emit startnext(QString("Downloading %1...\n").arg(url.toEncoded().constData()));
    downloadTime.start();
}

void DownloadManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{

    // calculate the download speed
    double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    } else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024*1024;
        unit = "MB/s";
    }

    emit progress(QString::fromLatin1("%1 %2").arg(speed, 3, 'f', 1).arg(unit));

}

void DownloadManager::downloadFinished()
{
    output.close();

    if (currentDownload->error()) {
        // download failed
        qDebug() << "Failed: " << currentDownload->errorString();
    } else {
        qDebug() << "Download succeeded.";
        ++downloadedCount;
    }

    currentDownload->deleteLater();
    startNextDownload();
}

void DownloadManager::downloadReadyRead()
{
    output.write(currentDownload->readAll());
}
