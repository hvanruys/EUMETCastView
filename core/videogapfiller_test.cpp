// Unit tests for the two gap fillers a video render needs.
//
// Neither of them touches a widget or an option, so all of this runs without a
// display and without satellite data. fillMTGChunkGaps() is given maps built
// here; fillMissingFrames() is given a QTemporaryDir of empty files, since what
// it does with a frame is copy it, not read it.
//
// The one case that does want real data is at the end and is skipped when it is
// not there : bin/EUMETCastVideo.json as the application wrote it, which holds a
// genuine hole - cycle 111 has no chunk 15.

#include "videogapfiller.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include <cstdio>

static int g_failures = 0;

static void check(bool ok, const char *what)
{
    if (ok) std::printf("ok   : %s\n", what);
    else  { std::printf("FAIL : %s\n", what); ++g_failures; }
}

static void skip(const char *what)
{
    std::printf("skip : %s\n", what);
}

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------

// A chunk file of a cycle, named the way one is recognised again : the value
// only has to be told apart from the other values, it is never opened.
static QFileInfo chunkfile(int cycle, int chunk)
{
    return QFileInfo(QString("/data/cycle%1_chunk%2.nc")
                         .arg(cycle, 3, 10, QChar('0'))
                         .arg(chunk, 2, 10, QChar('0')));
}

// cycles first..last, every one holding every chunk of chunks.
static QMap<int, QMap<int, QFileInfo> > buildCycles(int first, int last, const QList<int> &chunks)
{
    QMap<int, QMap<int, QFileInfo> > cycles;

    for (int cycle = first; cycle <= last; ++cycle) {
        QMap<int, QFileInfo> inner;
        for (int chunk : chunks)
            inner.insert(chunk, chunkfile(cycle, chunk));
        cycles.insert(cycle, inner);
    }

    return cycles;
}

static QSet<int> chunkSet(const QList<int> &chunks)
{
    QSet<int> set;
    for (int chunk : chunks)
        set.insert(chunk);
    return set;
}

// The cycle a chunk of the filled map came from, read back off the name.
static int cameFrom(const QMap<int, QMap<int, QFileInfo> > &filled, int cycle, int chunk)
{
    if (!filled.contains(cycle) || !filled.value(cycle).contains(chunk))
        return -1;

    const QString name = filled.value(cycle).value(chunk).fileName();
    return name.mid(5, 3).toInt();          // cycleNNN_chunkNN.nc
}

static bool writeFrame(const QDir &dir, const QString &name, const QByteArray &content)
{
    QFile file(dir.absoluteFilePath(name));
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(content);
    file.close();
    return true;
}

// ---------------------------------------------------------------------------
// fillMTGChunkGaps
// ---------------------------------------------------------------------------

static void testChunkGaps()
{
    const QList<int> allchunks = {1, 2, 3, 4};
    const QSet<int> allowed = chunkSet(allchunks);

    // Nothing missing : the selection comes back as it went in, and quietly.
    {
        const QMap<int, QMap<int, QFileInfo> > cycles = buildCycles(1, 5, allchunks);
        QStringList report;
        const QMap<int, QMap<int, QFileInfo> > filled = fillMTGChunkGaps(cycles, allowed, report);

        check(filled.keys() == cycles.keys(), "complete selection : the cycles are unchanged");
        check(filled.value(3).value(2).absoluteFilePath() == cycles.value(3).value(2).absoluteFilePath(),
              "complete selection : the files are unchanged");
        check(report.isEmpty(), "complete selection : nothing is reported");
    }

    // One chunk out of a cycle in the middle : taken from the cycle before.
    {
        QMap<int, QMap<int, QFileInfo> > cycles = buildCycles(1, 5, allchunks);
        cycles[3].remove(2);

        QStringList report;
        const QMap<int, QMap<int, QFileInfo> > filled = fillMTGChunkGaps(cycles, allowed, report);

        check(filled.value(3).size() == 4, "missing chunk : the cycle holds all four again");
        check(cameFrom(filled, 3, 2) == 2, "missing chunk : it came from the cycle before");
        check(cameFrom(filled, 3, 1) == 3, "missing chunk : the others are the cycle's own");
        check(report.size() == 1 && report.first().contains("2") && report.first().contains("3"),
              "missing chunk : the substitution is reported once");
    }

    // The hole is in the first cycle, so there is nothing before it : the cycle
    // after has to do.
    {
        QMap<int, QMap<int, QFileInfo> > cycles = buildCycles(1, 5, allchunks);
        cycles[1].remove(2);

        QStringList report;
        const QMap<int, QMap<int, QFileInfo> > filled = fillMTGChunkGaps(cycles, allowed, report);

        check(cameFrom(filled, 1, 2) == 2, "hole in the first cycle : it came from the cycle after");
    }

    // Two cycles running without the same chunk : both take the last real one,
    // rather than the second taking the copy the first was given.
    {
        QMap<int, QMap<int, QFileInfo> > cycles = buildCycles(1, 5, allchunks);
        cycles[3].remove(2);
        cycles[4].remove(2);

        QStringList report;
        const QMap<int, QMap<int, QFileInfo> > filled = fillMTGChunkGaps(cycles, allowed, report);

        check(cameFrom(filled, 3, 2) == 2 && cameFrom(filled, 4, 2) == 2,
              "run of holes : both cycles name the same real file");
    }

    // A cycle that is not there at all : an entry holding nothing, so that it
    // still takes a frame number.
    {
        QMap<int, QMap<int, QFileInfo> > cycles = buildCycles(1, 5, allchunks);
        cycles.remove(3);

        QStringList report;
        const QMap<int, QMap<int, QFileInfo> > filled = fillMTGChunkGaps(cycles, allowed, report);

        const QList<int> expected = {1, 2, 3, 4, 5};
        check(filled.keys() == expected, "absent cycle : it is back in the numbering");
        check(filled.value(3).isEmpty(), "absent cycle : its entry holds nothing");
        check(filled.value(4).size() == 4, "absent cycle : the cycle after it is untouched");
        check(report.size() == 1, "absent cycle : it is reported once");
    }

    // An absent cycle is not a source : the cycle after it reaches past it.
    {
        QMap<int, QMap<int, QFileInfo> > cycles = buildCycles(1, 5, allchunks);
        cycles.remove(3);
        cycles[4].remove(2);

        QStringList report;
        const QMap<int, QMap<int, QFileInfo> > filled = fillMTGChunkGaps(cycles, allowed, report);

        check(cameFrom(filled, 4, 2) == 2, "absent cycle : the one after it reaches over it");
    }

    // A chunk that is in no cycle of the selection cannot be filled in. It is
    // left out, and said once rather than once a cycle.
    {
        QMap<int, QMap<int, QFileInfo> > cycles = buildCycles(1, 5, {1, 2, 3});

        QStringList report;
        const QMap<int, QMap<int, QFileInfo> > filled = fillMTGChunkGaps(cycles, allowed, report);

        check(filled.value(3).size() == 3, "chunk in no cycle : it stays out of every cycle");
        check(!filled.value(3).contains(4), "chunk in no cycle : nothing was invented for it");

        int named = 0;
        for (const QString &line : report)
            if (line.contains("4")) ++named;
        check(named == 1, "chunk in no cycle : reported once, not once a cycle");
    }

    // The range is the selection's own, not 1 to 144.
    {
        const QMap<int, QMap<int, QFileInfo> > cycles = buildCycles(60, 64, allchunks);

        QStringList report;
        const QMap<int, QMap<int, QFileInfo> > filled = fillMTGChunkGaps(cycles, allowed, report);

        const QList<int> expected = {60, 61, 62, 63, 64};
        check(filled.keys() == expected, "range : nothing outside the selection is added");
    }

    // Only the chunks the projection asks for are filled in.
    {
        QMap<int, QMap<int, QFileInfo> > cycles = buildCycles(1, 5, allchunks);
        cycles[3].remove(2);
        cycles[3].remove(4);

        QStringList report;
        const QMap<int, QMap<int, QFileInfo> > filled = fillMTGChunkGaps(cycles, chunkSet({1, 2, 3}), report);

        check(filled.value(3).contains(2), "allowed chunks : a chunk that is asked for is filled in");
        check(!filled.value(3).contains(4), "allowed chunks : one that is not is left alone");
    }

    // Nothing in, nothing out.
    {
        QStringList report;
        const QMap<int, QMap<int, QFileInfo> > filled =
            fillMTGChunkGaps(QMap<int, QMap<int, QFileInfo> >(), allowed, report);

        check(filled.isEmpty(), "empty selection : nothing is made up");
        check(report.isEmpty(), "empty selection : nothing is reported");
    }
}

// ---------------------------------------------------------------------------
// fillMissingFrames
// ---------------------------------------------------------------------------

static void testMissingFrames()
{
    const QString prefix = "PROJ_MET_12_";

    // A complete run : nothing to do, and nothing done.
    {
        QTemporaryDir tmp;
        if (!tmp.isValid()) { check(false, "temporary directory"); return; }
        QDir dir(tmp.path());

        for (int i = 0; i < 5; ++i)
            writeFrame(dir, QString("%1%2.png").arg(prefix).arg(i, 4, 10, QChar('0')), QByteArray("frame"));

        QStringList report;
        const int written = fillMissingFrames(dir.absolutePath(), prefix, report);

        check(written == 0, "complete run : no frame is written");
        check(dir.entryList(QDir::Files).size() == 5, "complete run : the directory is untouched");
        check(report.isEmpty(), "complete run : nothing is reported");
    }

    // A hole in the middle : the frame before it, copied.
    {
        QTemporaryDir tmp;
        if (!tmp.isValid()) { check(false, "temporary directory"); return; }
        QDir dir(tmp.path());

        for (int i : {0, 1, 2, 4, 5})
            writeFrame(dir, QString("%1%2.png").arg(prefix).arg(i, 4, 10, QChar('0')),
                       QByteArray("frame ") + QByteArray::number(i));

        QStringList report;
        const int written = fillMissingFrames(dir.absolutePath(), prefix, report);

        check(written == 1, "hole : one frame is written");

        QFile filled(dir.absoluteFilePath(prefix + "0003.png"));
        check(filled.exists(), "hole : the missing number is there now");
        if (filled.open(QIODevice::ReadOnly)) {
            check(filled.readAll() == QByteArray("frame 2"), "hole : it is the frame before it");
            filled.close();
        }
        check(report.size() == 1 && report.first().contains("0003"), "hole : it is reported");
    }

    // Several missing in a row : all of them the last real frame, not a copy of
    // a copy - which comes to the same picture, but only if nothing is chained.
    {
        QTemporaryDir tmp;
        if (!tmp.isValid()) { check(false, "temporary directory"); return; }
        QDir dir(tmp.path());

        for (int i : {0, 3})
            writeFrame(dir, QString("%1%2.png").arg(prefix).arg(i, 4, 10, QChar('0')),
                       QByteArray("frame ") + QByteArray::number(i));

        QStringList report;
        const int written = fillMissingFrames(dir.absolutePath(), prefix, report);

        check(written == 2, "run of holes : both frames are written");

        for (const char *name : {"0001", "0002"}) {
            QFile filled(dir.absoluteFilePath(prefix + name + ".png"));
            if (filled.open(QIODevice::ReadOnly)) {
                check(filled.readAll() == QByteArray("frame 0"), "run of holes : it is the last real frame");
                filled.close();
            } else
                check(false, "run of holes : the frame was written");
        }
    }

    // The run does not start at zero. ffmpeg finds its own start index, so
    // there is nothing to fill in front of the first frame.
    {
        QTemporaryDir tmp;
        if (!tmp.isValid()) { check(false, "temporary directory"); return; }
        QDir dir(tmp.path());

        for (int i : {2, 3, 4})
            writeFrame(dir, QString("%1%2.png").arg(prefix).arg(i, 4, 10, QChar('0')), QByteArray("frame"));

        QStringList report;
        const int written = fillMissingFrames(dir.absolutePath(), prefix, report);

        check(written == 0, "hole at the head : nothing is written in front of the first frame");
        check(!QFile::exists(dir.absoluteFilePath(prefix + "0000.png")), "hole at the head : 0000 stays away");
    }

    // Nothing is added past the last frame either : a run that stopped early is
    // a shorter video.
    {
        QTemporaryDir tmp;
        if (!tmp.isValid()) { check(false, "temporary directory"); return; }
        QDir dir(tmp.path());

        for (int i : {0, 1, 2})
            writeFrame(dir, QString("%1%2.png").arg(prefix).arg(i, 4, 10, QChar('0')), QByteArray("frame"));

        QStringList report;
        const int written = fillMissingFrames(dir.absolutePath(), prefix, report);

        check(written == 0, "hole at the tail : nothing is written past the last frame");
        check(dir.entryList(QDir::Files).size() == 3, "hole at the tail : the directory is untouched");
    }

    // Only this run's frames are counted. The HRV frames of another run and
    // anything else in the directory are not numbers in this sequence.
    {
        QTemporaryDir tmp;
        if (!tmp.isValid()) { check(false, "temporary directory"); return; }
        QDir dir(tmp.path());

        writeFrame(dir, prefix + "0000.png", QByteArray("frame 0"));
        writeFrame(dir, prefix + "0002.png", QByteArray("frame 2"));
        writeFrame(dir, "PROJHRV_MET_12_0001.png", QByteArray("hrv"));
        writeFrame(dir, "overlayproj.png", QByteArray("overlay"));
        writeFrame(dir, prefix + "notanumber.png", QByteArray("junk"));

        QStringList report;
        const int written = fillMissingFrames(dir.absolutePath(), prefix, report);

        check(written == 1, "foreign names : only this run's hole is filled");

        QFile filled(dir.absoluteFilePath(prefix + "0001.png"));
        check(filled.exists(), "foreign names : the hole is filled from this run");
        if (filled.open(QIODevice::ReadOnly)) {
            check(filled.readAll() == QByteArray("frame 0"), "foreign names : the HRV frame was not used");
            filled.close();
        }
    }

    // One frame, or none, or no directory at all.
    {
        QTemporaryDir tmp;
        if (!tmp.isValid()) { check(false, "temporary directory"); return; }
        QDir dir(tmp.path());

        QStringList report;
        check(fillMissingFrames(dir.absolutePath(), prefix, report) == 0, "empty directory : nothing is written");

        writeFrame(dir, prefix + "0000.png", QByteArray("frame"));
        check(fillMissingFrames(dir.absolutePath(), prefix, report) == 0, "one frame : nothing is written");

        check(fillMissingFrames(dir.absoluteFilePath("nosuchplace"), prefix, report) == 0,
              "no directory : nothing is written");
    }
}

// ---------------------------------------------------------------------------
// The real thing, when it is on disk
// ---------------------------------------------------------------------------

static void testRealJson(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { skip("bin/EUMETCastVideo.json is not there"); return; }

    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    file.close();

    if (!root.contains("files")) { skip("bin/EUMETCastVideo.json holds no files"); return; }

    const QJsonObject files = root["files"].toObject();

    QMap<int, QMap<int, QFileInfo> > cycles;
    QSet<int> allowed;

    for (const QString &cyclekey : files.keys()) {
        const QJsonObject chunks = files[cyclekey].toObject();
        QMap<int, QFileInfo> inner;
        for (const QString &chunkkey : chunks.keys()) {
            const int chunk = chunkkey.toInt();
            inner.insert(chunk, QFileInfo(chunks[chunkkey].toObject()["absoluteFilePath"].toString()));
            allowed.insert(chunk);
        }
        cycles.insert(cyclekey.toInt(), inner);
    }

    QStringList report;
    const QMap<int, QMap<int, QFileInfo> > filled = fillMTGChunkGaps(cycles, allowed, report);

    std::printf("     %d cycles read, %lld chunks asked for, %d line(s) reported\n",
                int(cycles.size()), qint64(allowed.size()), int(report.size()));
    for (const QString &line : report)
        std::printf("     %s\n", qPrintable(line));

    check(filled.size() == cycles.lastKey() - cycles.firstKey() + 1,
          "real json : every cycle of the range has an entry");

    bool complete = true;
    for (auto it = filled.constBegin(); it != filled.constEnd(); ++it)
        if (!it.value().isEmpty() && it.value().size() != allowed.size())
            complete = false;

    check(complete, "real json : every cycle that was there holds every chunk");
}

int main(int argc, char **argv)
{
    testChunkGaps();
    testMissingFrames();
    testRealJson(argc > 1 ? QString::fromLocal8Bit(argv[1]) : QString("bin/EUMETCastVideo.json"));

    std::printf("%s\n", g_failures == 0 ? "all ok" : "FAILURES");
    return g_failures == 0 ? 0 : 1;
}
