#include "videogapfiller.h"

#include <QDir>
#include <QFile>

#include <algorithm>

namespace {

QString cycleName(int cycle) { return QString("%1").arg(cycle, 3, 10, QChar('0')); }
QString chunkName(int chunk) { return QString("%1").arg(chunk, 2, 10, QChar('0')); }
QString frameNumber(int nbr)  { return QString("%1").arg(nbr, 4, 10, QChar('0')); }

QString frameName(const QString &prefix, int nbr) { return prefix + frameNumber(nbr) + ".png"; }

// The nearest cycle holding chunk. Below the hole first - a substitute from
// before it is one the viewer has already been shown, so the image goes stale
// rather than ahead of itself - and above it only for a hole in the first cycle
// of the selection, where there is nothing before.
//
// cycles is the selection as it was received. A cycle that is not in it holds
// nothing, so an absent cycle is never a source and the search reaches over it.
int nearestCycleWithChunk(const QMap<int, QMap<int, QFileInfo> > &cycles, int cycle, int chunk)
{
    for (int c = cycle - 1; c >= cycles.firstKey(); --c)
        if (cycles.value(c).contains(chunk))
            return c;

    for (int c = cycle + 1; c <= cycles.lastKey(); ++c)
        if (cycles.value(c).contains(chunk))
            return c;

    return -1;
}

}

QMap<int, QMap<int, QFileInfo> > fillMTGChunkGaps(const QMap<int, QMap<int, QFileInfo> > &cycles,
                                                  const QSet<int> &allowedchunks,
                                                  QStringList &report)
{
    QMap<int, QMap<int, QFileInfo> > filled;

    if (cycles.isEmpty() || allowedchunks.isEmpty())
        return filled;

    QList<int> chunklist = allowedchunks.values();
    std::sort(chunklist.begin(), chunklist.end());

    // Named once at the end rather than once a cycle : a chunk that was never
    // received is one line about the selection, not 144 lines about the cycles.
    QList<int> nowhere;

    for (int cycle = cycles.firstKey(); cycle <= cycles.lastKey(); ++cycle)
    {
        if (!cycles.contains(cycle))
        {
            // The entry is empty on purpose. It is here for the frame number it
            // takes - the frames keep step with the clock - and EUMETCastVideo
            // composes nothing for it : readVideoPathsMTG hands compileImageMTG
            // an empty list and it returns on the spot.
            filled.insert(cycle, QMap<int, QFileInfo>());
            report << QString("Cycle %1 : nothing was received, the frame is copied after the render")
                          .arg(cycleName(cycle));
            continue;
        }

        QMap<int, QFileInfo> chunks = cycles.value(cycle);
        QStringList taken;

        for (int chunk : std::as_const(chunklist))
        {
            if (chunks.contains(chunk))
                continue;

            const int from = nearestCycleWithChunk(cycles, cycle, chunk);

            if (from < 0)
            {
                if (!nowhere.contains(chunk))
                    nowhere << chunk;
                continue;
            }

            chunks.insert(chunk, cycles.value(from).value(chunk));
            taken << QString("chunk %1 from cycle %2").arg(chunkName(chunk), cycleName(from));
        }

        if (!taken.isEmpty())
            report << QString("Cycle %1 : %2").arg(cycleName(cycle), taken.join(", "));

        filled.insert(cycle, chunks);
    }

    for (int chunk : std::as_const(nowhere))
        report << QString("Chunk %1 is in no cycle of the selection : every image is composed without it")
                      .arg(chunkName(chunk));

    return filled;
}

int fillMissingFrames(const QString &dirpath, const QString &prefix, QStringList &report)
{
    QDir dir(dirpath);

    if (!dir.exists())
        return 0;

    // The prefix, four digits and ".png", and nothing else. Whatever else is in
    // the directory - the frames of an HRV run under their own prefix, an
    // overlay saved by hand - is not a number in this sequence and may not be
    // read as one.
    QMap<int, QString> frames;

    const QStringList names = dir.entryList(QStringList() << prefix + "????.png",
                                            QDir::Files | QDir::NoSymLinks);

    for (const QString &name : names)
    {
        bool ok = false;
        const int nbr = name.mid(prefix.length(), 4).toInt(&ok);

        if (ok)
            frames.insert(nbr, name);
    }

    if (frames.size() < 2)
        return 0;

    int written = 0;

    // The last frame that was really composed, which is what a run of holes is
    // filled with : copying the copy would come to the same picture, but only
    // for as long as nothing in between fails halfway through being written.
    int previous = frames.firstKey();

    for (int nbr = frames.firstKey() + 1; nbr <= frames.lastKey(); ++nbr)
    {
        if (frames.contains(nbr))
        {
            previous = nbr;
            continue;
        }

        if (QFile::copy(dir.absoluteFilePath(frames.value(previous)),
                        dir.absoluteFilePath(frameName(prefix, nbr))))
        {
            report << QString("Frame %1 is a copy of %2 : nothing was composed for it")
                          .arg(frameNumber(nbr), frameNumber(previous));
            ++written;
        }
        else
            report << QString("Frame %1 could not be written as a copy of %2 : the video will stop there")
                          .arg(frameNumber(nbr), frameNumber(previous));
    }

    return written;
}
