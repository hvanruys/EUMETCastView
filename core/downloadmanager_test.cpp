// Unit tests for DownloadManager::saveFileName.
//
// The point of these is where a download lands, not what it contains. Nothing
// here touches the network: saveFileName only reads the URL and the working
// directory.

#include "downloadmanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QNetworkAccessManager>
#include <QTemporaryDir>

#include <cstdio>

// downloadmanager.cpp expects the application to own one of these; main.cpp
// declares it at file scope in exactly the same way. Being a global, it is
// built before QCoreApplication, and Qt says so on stderr - "QObject::connect
// (QObject, Unknown): invalid nullptr parameter". The real application prints
// it too. Nothing here does any I/O, so it is noise, not a failure.
QNetworkAccessManager networkaccessmanager;

static int g_failures = 0;

static void check(bool ok, const char *what)
{
    if (ok) std::printf("ok   : %s\n", what);
    else  { std::printf("FAIL : %s\n", what); ++g_failures; }
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    DownloadManager dm;

    QTemporaryDir tmp;
    if (!tmp.isValid()) { std::printf("FAIL : no temporary directory\n"); return 1; }

    const QString origCwd = QDir::currentPath();
    if (!QDir::setCurrent(tmp.path())) {
        std::printf("FAIL : cannot enter %s\n", qPrintable(tmp.path()));
        return 1;
    }
    // QTemporaryDir hands back the path as given; the shell may have reached it
    // through a symlink, and saveFileName returns the resolved one.
    const QString cwd = QDir::currentPath();

    // The default sources, and the whole reason this is here: Options seeds
    // tlelist with exactly these two names, and Satellite::ReReadTLE opens them
    // relative to the working directory. A download that lands anywhere else is
    // one nothing reads back.
    const QString weather = dm.saveFileName(QUrl(
        "https://celestrak.org/NORAD/elements/gp.php?GROUP=weather&FORMAT=tle"));
    const QString resource = dm.saveFileName(QUrl(
        "https://celestrak.org/NORAD/elements/gp.php?GROUP=resource&FORMAT=tle"));

    check(weather  == cwd + "/weather.tle",  "weather source saves as ./weather.tle");
    check(resource == cwd + "/resource.tle", "resource source saves as ./resource.tle");

    // Absolute, so the error message names somewhere real when the save fails.
    check(QDir::isAbsolutePath(weather), "path is absolute");

    // Case in the query must not reach the file name; tlelist is lower case.
    check(dm.saveFileName(QUrl(
              "https://celestrak.org/NORAD/elements/gp.php?GROUP=Weather&FORMAT=TLE"))
          == cwd + "/weather.tle", "GROUP and FORMAT are case-folded");

    // A source that is not gp.php has neither query item. Joining two empty
    // strings around a dot used to name the file ".", which is the directory.
    const QString plain = dm.saveFileName(
        QUrl("https://celestrak.org/NORAD/elements/weather.txt"));
    check(plain == cwd + "/weather.txt", "a plain URL keeps its own file name");
    check(!plain.endsWith("/.") && !plain.endsWith("/"),
          "no URL produces a directory as its save name");

    // Nothing usable in the URL at all still has to name a file.
    const QString bare = dm.saveFileName(QUrl("https://celestrak.org/"));
    check(bare == cwd + "/download.tle", "an empty path falls back to a real name");

    // The save location follows the working directory, because that is what
    // QSettings and the TLE reader follow.
    QDir().mkpath(cwd + "/elsewhere");
    QDir::setCurrent(cwd + "/elsewhere");
    check(dm.saveFileName(QUrl(
              "https://celestrak.org/NORAD/elements/gp.php?GROUP=weather&FORMAT=tle"))
          == QDir::currentPath() + "/weather.tle",
          "save location tracks the working directory");

    // Not the directory the executable sits in - the case that broke under an
    // AppImage, where that directory is a read-only mount.
    check(!dm.saveFileName(QUrl(
              "https://celestrak.org/NORAD/elements/gp.php?GROUP=weather&FORMAT=tle"))
           .startsWith(QCoreApplication::applicationDirPath() + "/"),
          "save location is not the executable's directory");

    QDir::setCurrent(origCwd);

    if (g_failures == 0) std::printf("all checks passed\n");
    else std::printf("%d check(s) FAILED\n", g_failures);
    return g_failures == 0 ? 0 : 1;
}
