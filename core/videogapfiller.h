#ifndef VIDEOGAPFILLER_H
#define VIDEOGAPFILLER_H

#include <QFileInfo>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

// Filling the holes a video render would otherwise be left with. Both of these
// are data and files only - no widgets, no options, nothing of the form they
// are called from - so that they can be run outside the application.

// MET_12. The selection is cycles - the repeat cycle of the day, 1 to 144 -
// each holding the chunks of that cycle, 1 to 40, that the projection asks for.
//
// A chunk that is not there is not a band missing out of the image :
// compileImageMTG stacks the chunks that did come in and adds up their rows, so
// everything under the hole moves up and is navigated as the wrong latitude.
// The chunk covers a fixed band of the disc, so the same chunk of a neighbouring
// cycle puts those rows back, ten minutes stale. It is taken from the nearest
// cycle that has it : the one before by preference, the one after when the hole
// is in the first cycle of the selection. Cycles are searched in the selection
// as it was found, never in what this function filled in, so a run of holes all
// name the same real file rather than a copy of a copy.
//
// A cycle that is not there at all becomes an entry holding nothing. It takes a
// frame number - that is the whole point of it, the frames stay in step with the
// clock - and EUMETCastVideo composes nothing for it. fillMissingFrames() puts
// the frame in afterwards.
//
// Every cycle that was touched is named in report, one line each.
QMap<int, QMap<int, QFileInfo> > fillMTGChunkGaps(const QMap<int, QMap<int, QFileInfo> > &cycles,
                                                  const QSet<int> &allowedchunks,
                                                  QStringList &report);

// The frames of a render, dirpath/prefix + a four digit number + ".png", have to
// run consecutively : ffmpeg's image2 demuxer stops at the first number it does
// not find. A frame is missing whenever nothing was composed for it - a cycle
// that was not there, a chunk that could not be read, a process that died - and
// one of them is enough to cut the video short.
//
// Every number missing between the lowest and the highest frame present is
// written as a copy of the last frame before it that is real, so a run of holes
// repeats one frame rather than nothing. Holes at the two ends are left alone :
// image2 looks for its own start index, and a run that stopped early is a
// shorter video, not a broken one.
//
// Returns the number of frames written. Every one of them is named in report.
int fillMissingFrames(const QString &dirpath, const QString &prefix, QStringList &report);

#endif // VIDEOGAPFILLER_H
