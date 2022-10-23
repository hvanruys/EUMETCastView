#include "segmentlistolci.h"
#include "segmentimage.h"
#include "options.h"
#include <QtConcurrent>

extern Options opts;
extern SegmentImage *imageptrs;
extern bool ptrimagebusy;

void doComposeOLCIImageInThread(SegmentListOLCI *t, QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles)
{
    t->ComposeOLCIImageInThread(bandlist, colorlist, invertlist, decompressfiles);
}


SegmentListOLCI::SegmentListOLCI(eSegmentType type, SatelliteList *satl, QObject *parent) :
    SegmentList(parent)
{
    nbrofvisiblesegments = opts.nbrofvisiblesegments;
    qDebug() << QString("in constructor SegmentListOLCI");

    satlist = satl;
    seglisttype = type;
    histogrammethod = 0; // 0 none , 1 equalize
    normalized = false;
}

bool SegmentListOLCI::CheckForOLCIFiles(QList<bool> bandlist, QList<int> colorlist, QStringList &missing)
{
    QString filemono;
    QString filered, filegreen, fileblue;
    bool bmono = true;
    bool bred = true;
    bool bgreen = true;
    bool bblue = true;
    bool bgeocoord = true;
    bool btiegeo = true;
    bool bquality = true;



    QList<Segment*>::iterator segit = segmentlist.begin();
    while ( segit != segmentlist.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segit);
        if (segm->segmentselected)
        {
            qDebug() << "====> fileinfo.filepath = " << segm->fileInfo.filePath() << " isdirectory = " << segm->fileInfo.isDir();
            qDebug() << "====> fileinfo.filename = " << segm->fileInfo.fileName();

            if(segm->fileInfo.isDir())
            {
                bgeocoord = QFile::exists(segm->fileInfo.filePath() + "/geo_coordinates.nc");
                btiegeo = QFile::exists(segm->fileInfo.filePath() + "/tie_geometries.nc");
                bquality = QFile::exists(segm->fileInfo.filePath() + "/qualityFlags.nc");
                if(bandlist.at(0) == false) // no color
                {
                    getDatasetNameFromBand(bandlist, &filemono);
                    bmono = QFile::exists(segm->fileInfo.filePath() + "/" + filemono);
                    if(!bmono)
                        break;
                }
                else
                {
                    getDatasetNameFromColor(colorlist, 0, &filered);
                    getDatasetNameFromColor(colorlist, 1, &filegreen);
                    getDatasetNameFromColor(colorlist, 2, &fileblue);
                    bred = QFile::exists(segm->fileInfo.filePath() + "/" + filered);
                    bgreen = QFile::exists(segm->fileInfo.filePath() + "/" + filegreen);
                    bblue = QFile::exists(segm->fileInfo.filePath() + "/" + fileblue);
                    if(!bred || !bgreen || !bblue)
                        break;
                }

            }
        }
        ++segit;
    }

    if(!bgeocoord)
        missing.append("geo_coordinates.nc");
    if(!btiegeo)
        missing.append("tie_geometries.nc");
    if(!bquality)
        missing.append("qualityFlags.nc");

    if(bandlist.at(0) == false) // no color
    {
        if(!bmono)
            missing.append(filemono);
    }
    else
    {
        if(!bred)
            missing.append(filered);
        if(!bgreen)
            missing.append(filegreen);
        if(!bblue)
            missing.append(fileblue);
    }

    if(bandlist.at(0) == false)
    {
        if(!bgeocoord || !btiegeo || !bquality || !bmono)
            return false;
        else
            return  true;
    }
    else
    {
        if(!bgeocoord || !btiegeo || !bquality || !bred || !bgreen || !bblue)
            return false;
        else
            return  true;
    }

}

bool SegmentListOLCI::ComposeOLCIImage(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles, int histogrammethod, bool normalized)
{
    qDebug() << QString("SegmentListOLCI::ComposeOLCIImage");

    this->bandlist = bandlist;
    this->colorlist = colorlist;
    this->invertlist = invertlist;
    this->histogrammethod = histogrammethod;
    this->normalized = normalized;


    ptrimagebusy = true;
    QApplication::setOverrideCursor(( Qt::WaitCursor));
    watcherolci = new QFutureWatcher<void>(this);
    connect(watcherolci, SIGNAL(finished()), this, SLOT(finishedolci()));

    QFuture<void> future;
    future = QtConcurrent::run(doComposeOLCIImageInThread, this, bandlist, colorlist, invertlist, decompressfiles);
    watcherolci->setFuture(future);

    return true;

}

bool SegmentListOLCI::OLCIFileExist(QString completebasename, QString band_or_quicklook)
{
    // S3A_OL_1_EFR____20201205T102330_20201205T102630_20201205T121305_0179_066_008_2340_LN1_O_NR_002.SEN3
    // S3A_OL_1_ERR____20201210T094858_20201210T103307_20201210T115918_2649_066_079______LN1_O_NR_002.SEN3
    // 01234567890123456789012345678901234567890123456789012345678901234567890123456789

    QDir dir(opts.productdirectory);
    QString returndirstr;

    QString fileyear = completebasename.mid(16, 4);
    QString filemonth = completebasename.mid(20, 2);
    QString fileday = completebasename.mid(22,2);

    QString filestr(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday + "/" + completebasename);

    filestr.append("/" + band_or_quicklook);

    QFile filefile(filestr);
    if(filefile.exists())
        return true;
    else
        return false;

}

void SegmentListOLCI::getDatasetNameFromBand(QList<bool> bandlist, QString *filename)
{
    if(bandlist.at(1))
    {
        *filename = "Oa01_radiance.nc";
    }
    else if(bandlist.at(2))
    {
        *filename = "Oa02_radiance.nc";
    }
    else if(bandlist.at(3))
    {
        *filename = "Oa03_radiance.nc";
    }
    else if(bandlist.at(4))
    {
        *filename = "Oa04_radiance.nc";
    }
    else if(bandlist.at(5))
    {
        *filename = "Oa05_radiance.nc";
    }
    else if(bandlist.at(6))
    {
        *filename = "Oa06_radiance.nc";
    }
    else if(bandlist.at(7))
    {
        *filename = "Oa07_radiance.nc";
    }
    else if(bandlist.at(8))
    {
        *filename = "Oa08_radiance.nc";
    }
    else if(bandlist.at(9))
    {
        *filename = "Oa09_radiance.nc";
    }
    else if(bandlist.at(10))
    {
        *filename = "Oa10_radiance.nc";
    }
    else if(bandlist.at(11))
    {
        *filename = "Oa11_radiance.nc";
    }
    else if(bandlist.at(12))
    {
        *filename = "Oa12_radiance.nc";
    }
    else if(bandlist.at(13))
    {
        *filename = "Oa13_radiance.nc";
    }
    else if(bandlist.at(14))
    {
        *filename = "Oa14_radiance.nc";
    }
    else if(bandlist.at(15))
    {
        *filename = "Oa15_radiance.nc";
    }
    else if(bandlist.at(16))
    {
        *filename = "Oa16_radiance.nc";
    }
    else if(bandlist.at(17))
    {
        *filename = "Oa17_radiance.nc";
    }
    else if(bandlist.at(18))
    {
        *filename = "Oa18_radiance.nc";
    }
    else if(bandlist.at(19))
    {
        *filename = "Oa19_radiance.nc";
    }
    else if(bandlist.at(20))
    {
        *filename = "Oa20_radiance.nc";
    }
    else if(bandlist.at(21))
    {
        *filename = "Oa21_radiance.nc";
    }

}

void SegmentListOLCI::getDatasetNameFromColor(QList<int> colorlist, int colorindex, QString *filename)
{
    qDebug() << "getDatasetNameFromColor colorindex = " << colorindex;

    Q_ASSERT(colorindex >=0 && colorindex < 3);
    colorindex++; // 1, 2 or 3

    if(colorlist.at(0) == colorindex)
    {
        *filename = "Oa01_radiance.nc";
    }
    else if(colorlist.at(1) == colorindex)
    {
        *filename = "Oa02_radiance.nc";
    }
    else if(colorlist.at(2) == colorindex)
    {
        *filename = "Oa03_radiance.nc";
    }
    else if(colorlist.at(3) == colorindex)
    {
        *filename = "Oa04_radiance.nc";
    }
    else if(colorlist.at(4) == colorindex)
    {
        *filename = "Oa05_radiance.nc";
    }
    else if(colorlist.at(5) == colorindex)
    {
        *filename = "Oa06_radiance.nc";
    }
    else if(colorlist.at(6) == colorindex)
    {
        *filename = "Oa07_radiance.nc";
    }
    else if(colorlist.at(7) == colorindex)
    {
        *filename = "Oa08_radiance.nc";
    }
    else if(colorlist.at(8) == colorindex)
    {
        *filename = "Oa09_radiance.nc";
    }
    else if(colorlist.at(9) == colorindex)
    {
        *filename = "Oa10_radiance.nc";
    }
    else if(colorlist.at(10) == colorindex)
    {
        *filename = "Oa11_radiance.nc";
    }
    else if(colorlist.at(11) == colorindex)
    {
        *filename = "Oa12_radiance.nc";
    }
    else if(colorlist.at(12) == colorindex)
    {
        *filename = "Oa13_radiance.nc";
    }
    else if(colorlist.at(13) == colorindex)
    {
        *filename = "Oa14_radiance.nc";
    }
    else if(colorlist.at(14) == colorindex)
    {
        *filename = "Oa15_radiance.nc";
    }
    else if(colorlist.at(15) == colorindex)
    {
        *filename = "Oa16_radiance.nc";
    }
    else if(colorlist.at(16) == colorindex)
    {
        *filename = "Oa17_radiance.nc";
    }
    else if(colorlist.at(17) == colorindex)
    {
        *filename = "Oa18_radiance.nc";
    }
    else if(colorlist.at(18) == colorindex)
    {
        *filename = "Oa19_radiance.nc";
    }
    else if(colorlist.at(19) == colorindex)
    {
        *filename = "Oa20_radiance.nc";
    }
    else if(colorlist.at(20) == colorindex)
    {
        *filename = "Oa21_radiance.nc";
    }
}

bool SegmentListOLCI::ComposeOLCIImageInThread(QList<bool> bandlist, QList<int> colorlist, QList<bool> invertlist, bool decompressfiles)
{

    qDebug() << "bool SegmentListOLCIefr::ComposeOLCIImageInThread() started decompressfiles = " << decompressfiles;

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    this->totalnbroflines = 0;


    emit progressCounter(10);

    for (int i=0; i < 3; i++)
    {
        for (int j=0; j < 1024; j++)
        {
            imageptrs->lut_ch[i][j] = 0;
            imageptrs->lut_norm_ch[i][j] = 0;
        }
    }

    for(int k = 0; k < 3; k++)
    {
        imageptrs->stat_min = 99999999;
        imageptrs->stat_max = 0;
        imageptrs->stat_max_ch[k] = 0;
        imageptrs->stat_min_ch[k] = 9999999;
        this->stat_max_ch[k] = 0;
        this->stat_min_ch[k] = 9999999;
        imageptrs->stat_max_norm_ch[k] = 0;
        imageptrs->stat_min_norm_ch[k] = 9999999;
        this->stat_max_norm_ch[k] = 0;
        this->stat_min_norm_ch[k] = 9999999;
        imageptrs->minRadianceIndex[k] = 999999;
        imageptrs->maxRadianceIndex[k] = 0;
        imageptrs->minRadianceIndexNormalized[k] = 999999;
        imageptrs->maxRadianceIndexNormalized[k] = 0;
    }

    // Reset memory
    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        Segment *segm = (Segment *)(*segsel);
        segm->resetMemory();
        ++segsel;
    }
    segsselected.clear();

    int startlinenbr = 0;
    int totalnbrofsegments = 0;

    QList<Segment*>::iterator segit = segmentlist.begin();

    // create QList of selected segments
    while ( segit != segmentlist.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segit);
        if (segm->segmentselected)
        {
            segsselected.append(segm);
            totalnbrofsegments++;
        }
        ++segit;
    }

    int deltaprogress = 99 / (totalnbrofsegments*3);
    int totalprogress = 0;

    if(decompressfiles)
    {
        segsel = segsselected.begin();
        while ( segsel != segsselected.end() )
        {
            SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
            segm->DecompressSegmentToTemp();
            totalprogress += deltaprogress;
            emit progressCounter(totalprogress);
            ++segsel;
        }
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        segm->setBandandColor(bandlist, colorlist, invertlist);
        segm->ReadSegmentInMemory();

        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        if(segsel == segsselected.begin())
            this->earth_views_per_scanline = segm->getEarthViewsPerScanline();
        ++segsel;
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        segm->setStartLineNbr(startlinenbr);
        startlinenbr += segm->NbrOfLines;
        totalnbroflines += segm->NbrOfLines;
        ++segsel;
    }

    // image pointers always = new QImage()
    if(imageptrs->ptrimageOLCI != NULL)
    {
        delete imageptrs->ptrimageOLCI;
        imageptrs->ptrimageOLCI = NULL;
    }

    imageptrs->ptrimageOLCI = new QImage(this->earth_views_per_scanline, totalnbroflines, QImage::Format_ARGB32);
    qDebug() << QString("ptrimageOLCI created %1 x %2").arg(this->earth_views_per_scanline).arg(totalnbroflines);


    bool composecolor = false;

    long cnt_active_pixels = 0;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        composecolor = segm->composeColorImage();


        for(int i = 0; i < (composecolor ? 3 : 1); i++)
        {
            if( segm->stat_max_ch[i] > this->stat_max_ch[i])
                this->stat_max_ch[i] = segm->stat_max_ch[i];
            if( segm->stat_min_ch[i] < this->stat_min_ch[i])
                this->stat_min_ch[i] = segm->stat_min_ch[i];
            if( segm->stat_max_norm_ch[i] > this->stat_max_norm_ch[i])
                this->stat_max_norm_ch[i] = segm->stat_max_norm_ch[i];
            if( segm->stat_min_norm_ch[i] < this->stat_min_norm_ch[i])
                this->stat_min_norm_ch[i] = segm->stat_min_norm_ch[i];
        }
        cnt_active_pixels += segm->active_pixels[0];
        ++segsel;
    }


    for(int i = 0; i < (composecolor ? 3 : 1); i++)
    {
        imageptrs->stat_max_ch[i] = this->stat_max_ch[i];
        imageptrs->stat_min_ch[i] = this->stat_min_ch[i];
        imageptrs->stat_max_norm_ch[i] = this->stat_max_norm_ch[i];
        imageptrs->stat_min_norm_ch[i] = this->stat_min_norm_ch[i];
    }

    for(int k = 0; k < (composecolor ? 3 : 1); k++)
    {
        if(imageptrs->stat_max_ch[k] > imageptrs->stat_max)
            imageptrs->stat_max = imageptrs->stat_max_ch[k];
        if(imageptrs->stat_min_ch[k] < imageptrs->stat_min)
            imageptrs->stat_min = imageptrs->stat_min_ch[k];
    }

    imageptrs->active_pixels = cnt_active_pixels;

    qDebug() << QString("imageptrs stat_min_ch[0] = %1 stat_max_ch[0] = %2").arg(imageptrs->stat_min_ch[0]).arg(imageptrs->stat_max_ch[0]);
    if(composecolor)
    {
        qDebug() << QString("imageptrs stat_min_ch[1] = %1 stat_max_ch[1] = %2").arg(imageptrs->stat_min_ch[1]).arg(imageptrs->stat_max_ch[1]);
        qDebug() << QString("imageptrs stat_min_ch[2] = %1 stat_max_ch[2] = %2").arg(imageptrs->stat_min_ch[2]).arg(imageptrs->stat_max_ch[2]);
    }
    qDebug() << QString("imageptrs stat_min_norm_ch[0] = %1 stat_max_norm_ch[0] = %2").arg(imageptrs->stat_min_norm_ch[0]).arg(imageptrs->stat_max_norm_ch[0]);
    if(composecolor)
    {
        qDebug() << QString("imageptrs stat_min_norm_ch[1] = %1 stat_max_norm_ch[1] = %2").arg(imageptrs->stat_min_norm_ch[1]).arg(imageptrs->stat_max_norm_ch[1]);
        qDebug() << QString("imageotrs stat_min_norm_ch[2] = %1 stat_max_norm_ch[2] = %2").arg(imageptrs->stat_min_norm_ch[2]).arg(imageptrs->stat_max_norm_ch[2]);
    }
    qDebug() << QString("imageptrs stat_min = %1 stat_max = %2").arg(imageptrs->stat_min).arg(imageptrs->stat_max);


    //CalculateLUTAlt();
    CalculateLUTFull();

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        segm->ComposeSegmentImage(this->histogrammethod, this->normalized);
        totalprogress += deltaprogress;
        emit progressCounter(totalprogress);
        QApplication::processEvents();
        ++segsel;
    }

    qDebug() << " SegmentListOLCI::ComposeOLCIImageInThread Finished !!";

    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
    emit progressCounter(100);
    return true;
}


bool SegmentListOLCI::searchLatLon(int mapx, int mapy, float &lon, float &lat)
{
    QList<Segment*>::iterator segsel = segsselected.begin();

    if(segsselected.count() > 0)
    {
        while ( segsel != segsselected.end() )
        {
            SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
            if( mapy >= segm->startLineNbr && mapy < segm->endLineNbr)
            {
                lat = (float)segm->latitude[(mapy - segm->startLineNbr) * segm->earth_views_per_scanline + mapx]/1000000.0;
                lon = (float)segm->longitude[(mapy - segm->startLineNbr) * segm->earth_views_per_scanline + mapx]/1000000.0;
                return(true);
            }

            ++segsel;
        }
    }
    return(false);
}

void SegmentListOLCI::CalculateProjectionLUT()
{
    qDebug() << "start SegmentListOLCI::CalculateProjectionLUT()";
    int earth_views;
    long stats_ch[3][1024];
    long cnt_active_pixels = 0;

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 1024; j++)
        {
            stats_ch[k][j] = 0;
        }
    }

    for (int i=0; i < 3; i++)
    {
        for (int j=0; j < 1024; j++)
        {
            imageptrs->lut_proj_ch[i][j] = 0;
        }
    }

    for(int k = 0; k < 3; k++)
    {
        imageptrs->stat_max_proj_ch[k] = 0;
        imageptrs->stat_min_proj_ch[k] = 9999999;
        this->stat_max_proj_ch[k] = 0;
        this->stat_min_proj_ch[k] = 9999999;
    }

    bool composecolor;

    int x, y;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        segm->recalculateStatsInProjection(this->normalized);
        ++segsel;
    }

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int i = 0; i < (composecolor ? 3 : 1); i++)
        {
            if( segm->stat_max_projection[i] > this->stat_max_proj_ch[i])
                this->stat_max_proj_ch[i] = segm->stat_max_projection[i];
            if( segm->stat_min_projection[i] < this->stat_min_proj_ch[i])
                this->stat_min_proj_ch[i] = segm->stat_min_projection[i];
        }
        cnt_active_pixels += segm->active_pixels_projection;
        ++segsel;
    }


    for(int i = 0; i < (composecolor ? 3 : 1); i++)
    {
        imageptrs->stat_max_proj_ch[i] = this->stat_max_proj_ch[i];
        imageptrs->stat_min_proj_ch[i] = this->stat_min_proj_ch[i];
    }

    imageptrs->active_pixels_proj = cnt_active_pixels;

    quint16 pixel;

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        composecolor = segm->composeColorImage();
        earth_views = segm->earth_views_per_scanline;

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < segm->earth_views_per_scanline; pixelx++)
                {
                    x = segm->getProjectionX(line, pixelx);
                    y = segm->getProjectionY(line, pixelx);
                    if(x >= 0 && x < imageptrs->ptrimageProjection->width() && y >= 0 && y < imageptrs->ptrimageProjection->height())
                    {
                        if(normalized) pixel = segm->ptrbaOLCInormalized[k][line * segm->earth_views_per_scanline + pixelx];
                        else pixel = segm->ptrbaOLCI[k][line * segm->earth_views_per_scanline + pixelx];

                        int pixcalc = 1023 * (pixel - imageptrs->stat_min_proj_ch[k]) / (imageptrs->stat_max_proj_ch[k] - imageptrs->stat_min_proj_ch[k]);
                        pixcalc = ( pixcalc < 0 ? 0 : pixcalc);
                        pixcalc = ( pixcalc > 1023 ? 1023 : pixcalc );
                        stats_ch[k][pixcalc]++;
                    }
                }
            }
        }

        ++segsel;
    }

    float scale = 1024.0 / (float)imageptrs->active_pixels_proj;

    unsigned long long sum_ch[3];

    for (int i=0; i < 3; i++)
    {
        sum_ch[i] = 0;
    }


    for( int i = 0; i < 1024; i++)
    {
        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            sum_ch[k] += stats_ch[k][i];
            imageptrs->lut_proj_ch[k][i] = (quint16)(sum_ch[k] * scale);
            imageptrs->lut_proj_ch[k][i] = ( imageptrs->lut_proj_ch[k][i] > 1023 ? 1023 : imageptrs->lut_proj_ch[k][i]);
        }
    }

    //    for( int i = 0; i < 1024; i++)
    //    {
    //        qDebug() << QString("CalculateProjectionLUT i = %1 lut_proj_ch[0][i] = %2").arg(i).arg(imageptrs->lut_proj_ch[0][i]);
    //    }
}

void SegmentListOLCI::finishedolci()
{

    qDebug() << "=============>SegmentListOLCIefr::finishedolci()";
    emit progressCounter(100);
    opts.texture_changed = true;
    ptrimagebusy = false;
    delete watcherolci;
    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
}

void SegmentListOLCI::progressreadvalue(int progress)
{
    int totalcount = segsselected.count();
    this->progressresultready += 100 / totalcount;

    emit progressCounter(this->progressresultready);

    qDebug() << QString("SegmentListOLCI::progressreadvalue( %1 )").arg(progress);
}

bool SegmentListOLCI::ChangeHistogramMethod()
{

    qDebug() << "bool SegmentListOLCIefr::ChangeHistogramMethod() started";

    progressresultready = 0;
    QApplication::setOverrideCursor( Qt::WaitCursor );

    emit progressCounter(10);

    // image pointers always = new QImage()
    if(imageptrs->ptrimageOLCI != NULL)
    {
        delete imageptrs->ptrimageOLCI;
        imageptrs->ptrimageOLCI = NULL;
    }

    imageptrs->ptrimageOLCI = new QImage(this->earth_views_per_scanline, this->totalnbroflines, QImage::Format_ARGB32);
    qDebug() << QString("ptrimageOLCI created %1 x %2").arg(this->earth_views_per_scanline).arg(totalnbroflines);

    if (this->histogrammethod == CMB_HISTO_NONE_95 || this->histogrammethod == CMB_HISTO_NONE_100 || this->histogrammethod == CMB_HISTO_EQUALIZE)
        ComposeSegments();
    else if (this->histogrammethod == CMB_HISTO_CLAHE)
        RecalculateCLAHEOLCI();


    QApplication::restoreOverrideCursor();

    emit segmentlistfinished(true);
    emit progressCounter(100);
    return true;
}

void SegmentListOLCI::ComposeSegments()
{
    QList<Segment*>::iterator segsel = segsselected.begin();

    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        segm->ComposeSegmentImage(this->histogrammethod, this->normalized);
        QApplication::processEvents();
        ++segsel;
    }

}

void SegmentListOLCI::Compose48bitPNG(QString fileName, bool mapto65535)
{
    int height = NbrOfSegmentLinesSelected();
    int width = earth_views_per_scanline;

    // initialize the FreeImage library
    FreeImage_Initialise();

    FIBITMAP *bitmap = FreeImage_AllocateT(FIT_RGB16, width, height);

    QList<Segment*>::iterator segsel = segsselected.begin();
    int heightinsegment = 0;
    segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        Compose48bitPNGSegment(segm, bitmap, heightinsegment, mapto65535);
        heightinsegment += segm->GetNbrOfLines();
        ++segsel;
    }


    QByteArray array = fileName.toLocal8Bit();
    char* pfileName = array.data();

    if(FreeImage_Save(FIF_PNG,bitmap, pfileName,0))
    {
        qDebug() << "bitmap successfully saved!";
    }



    FreeImage_Unload(bitmap);

    FreeImage_DeInitialise();


}


void SegmentListOLCI::Compose48bitPNGSegment(SegmentOLCI *segm, FIBITMAP *bitmap, int heightinsegment, bool mapto65535)
{

    quint16 pixval[3], pixval65535[3];
    bool iscolor = bandlist.at(0);

    for (int line = 0; line < segm->GetNbrOfLines(); line++)
    {
        FIRGB16 *bits = (FIRGB16 *)FreeImage_GetScanLine(bitmap, totalnbroflines - line - heightinsegment - 1);
        for (int pixelx = 0; pixelx < earth_views_per_scanline; pixelx++)
        {
            for(int k = 0; k < (iscolor ? 3 : 1); k++)
            {
                pixval[k] = segm->ptrbaOLCI[k][line * earth_views_per_scanline + pixelx];
                if(pixval[k] < 65535)
                {
                    if(mapto65535)
                    {
                        //                        pixval65535[k] =  (quint16)qMin(qMax(qRound(65535.0 * (float)(pixval[k] - imageptrs->stat_min_ch[k] ) / (float)(imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k])), 0), 65535);
                        pixval65535[k] =  (quint16)qMin(qMax(qRound(16.0 * (float)(pixval[k])), 0), 65535);
                        //pixval65535[k] =  (quint16)qMin(qMax(qRound(65535.0 * (float)(pixval1024[k] - imageptrs->minRadianceIndex[k] ) / (float)(imageptrs->maxRadianceIndex[k] - imageptrs->minRadianceIndex[k])), 0), 65535);
                        pixval[k] = pixval65535[k];
                    }
                }
                else
                    pixval[k] = 0;
            }


            bits[pixelx].red = pixval[0];
            bits[pixelx].green = pixval[1];
            bits[pixelx].blue = pixval[2];

        }
    }
}

//void SegmentListOLCI::Compose48bitProjectionPNG(QString fileName, bool mapto65535)
//{
//    quint16 pixval, pixvalout;
//    bool iscolor = bandlist.at(0);

//    int height = imageptrs->ptrimageProjection->height();
//    int width = imageptrs->ptrimageProjection->width();

//    // initialize the FreeImage library
//    FreeImage_Initialise();

//    FIBITMAP *bitmap = FreeImage_AllocateT(FIT_RGB16, width, height);

//    for (int line = 0; line < height; line++)
//    {
//        FIRGB16 *bits = (FIRGB16 *)FreeImage_GetScanLine(bitmap, height - line - 1);
//        for (int pixelx = 0; pixelx < width; pixelx++)
//        {
//            pixval = imageptrs->ptrimageProjectionRed[line * width + pixelx];
//            if(mapto65535)
//                pixvalout = (quint16)qMin(qMax(16 * pixval, 0), 65535);
//            else
//                pixvalout = pixval;

//            bits[pixelx].red = pixvalout;
//            if(iscolor)
//            {
//                pixval = imageptrs->ptrimageProjectionGreen[line * width + pixelx];
//                if(mapto65535)
//                    pixvalout = (quint16)qMin(qMax(16 * pixval, 0), 65535);
//                else
//                    pixvalout = pixval;
//                bits[pixelx].green = pixvalout;

//                pixval = imageptrs->ptrimageProjectionBlue[line * width + pixelx];
//                if(mapto65535)
//                    pixvalout = (quint16)qMin(qMax(16 * pixval, 0), 65535);
//                else
//                    pixvalout = pixval;
//                bits[pixelx].blue = pixvalout;
//            }
//            else
//            {
//                bits[pixelx].green = bits[pixelx].red;
//                bits[pixelx].blue = bits[pixelx].red;
//            }
//        }
//    }

//    QByteArray array = fileName.toLocal8Bit();
//    char* pfileName = array.data();

//    if(FreeImage_Save(FIF_PNG,bitmap, pfileName,0))
//    {
//        qDebug() << "bitmap successfully saved!";
//    }

//    FreeImage_Unload(bitmap);

//    FreeImage_DeInitialise();


//}

long SegmentListOLCI::NbrOfSaturatedPixels()
{
    QList<Segment*>::iterator segsel = segsselected.begin();
    long nbrofpixels = 0;
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        nbrofpixels += segm->nbrsaturatedpixels;
        ++segsel;
    }
    return nbrofpixels;
}


void SegmentListOLCI::RecalculateCLAHEOLCI()
{
    quint16 *pixelsRed;
    quint16 *pixelsGreen;
    quint16 *pixelsBlue;
    size_t npix;
    QRgb *row;

    int pixval[3];

    bool iscolor = bandlist.at(0);

    qDebug() << " earth_views_per_scanline = " << earth_views_per_scanline << " totalnbroflines = " << totalnbroflines;

    int a = floor(totalnbroflines / 16);
    int nbroflinesreduced = a * 16;
    npix = nbroflinesreduced * 4688;
    int difflines = totalnbroflines - nbroflinesreduced;

    // image pointers always = new QImage()
    if(imageptrs->ptrimageOLCI != NULL)
    {
        delete imageptrs->ptrimageOLCI;
        imageptrs->ptrimageOLCI = NULL;
    }

    imageptrs->ptrimageOLCI = new QImage(4688, nbroflinesreduced, QImage::Format_ARGB32);
    qDebug() << QString("ptrimageOLCI created 4688 x %1").arg(nbroflinesreduced);

    pixelsRed = new quint16[npix];
    pixelsGreen = new quint16[npix];
    pixelsBlue = new quint16[npix];

    int segnbr = 0;
    //    QList<Segment*>::iterator segsel = segsselected.begin();
    //    while ( segsel != segsselected.end() )
    //    {
    //        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
    //        memcpy(pixelsRed + segnbr * segm->NbrOfLines * earth_views_per_scanline, segm->ptrbaOLCI[0].data(), segm->NbrOfLines * earth_views_per_scanline * sizeof(quint16));
    //        memcpy(pixelsGreen + segnbr * segm->NbrOfLines * earth_views_per_scanline, segm->ptrbaOLCI[1].data(), segm->NbrOfLines * earth_views_per_scanline * sizeof(quint16));
    //        memcpy(pixelsBlue + segnbr * segm->NbrOfLines * earth_views_per_scanline, segm->ptrbaOLCI[2].data(), segm->NbrOfLines * earth_views_per_scanline * sizeof(quint16));
    //        segnbr++;
    //        ++segsel;
    //    }


    int restnbrofpixels = 0;
    QList<Segment*>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        for(int i = 0; i < (segm == segsselected.last() ? segm->GetNbrOfLines() - difflines : segm->GetNbrOfLines()); i++)
        {
            //            memcpy(pixelsRed + segnbr * segm->GetNbrOfLines() * 4688 + i * 4688, segm->ptrbaOLCI[0].data() + i * 4865 + 139, 4688 * sizeof(quint16));
            //            memcpy(pixelsGreen + segnbr * nbroflinesreduced * 4688 + i * 4688, segm->ptrbaOLCI[1].data() + i * 4865 + 139, 4688 * sizeof(quint16));
            //            memcpy(pixelsBlue + segnbr * nbroflinesreduced * 4688 + i * 4688, segm->ptrbaOLCI[2].data() + i * 4865 + 139, 4688 * sizeof(quint16));
            memcpy(pixelsRed + restnbrofpixels + i * 4688, segm->ptrbaOLCI[0].data() + i * 4865 + 139, 4688 * sizeof(quint16));
            memcpy(pixelsGreen + restnbrofpixels + i * 4688, segm->ptrbaOLCI[1].data() + i * 4865 + 139, 4688 * sizeof(quint16));
            memcpy(pixelsBlue + restnbrofpixels + i * 4688, segm->ptrbaOLCI[2].data() + i * 4865 + 139, 4688 * sizeof(quint16));
        }
        //restnbrofpixels += segm->GetNbrOfLines() * 4688;

        segnbr++;
        ++segsel;
    }

    for(int i = 0; i < npix; i++)
    {
        if(*(pixelsRed + i) > 1023)
            *(pixelsRed + i) = 1023;
        if(*(pixelsGreen + i) > 1023)
            *(pixelsGreen + i) = 1023;
        if(*(pixelsBlue + i) > 1023)
            *(pixelsBlue + i) = 1023;
    }


    int ret = imageptrs->CLAHE(pixelsRed, 4688, nbroflinesreduced, 0, 1024, 16, 16, 256, 6.9);
    ret = imageptrs->CLAHE(pixelsGreen, 4688, nbroflinesreduced, 0, 1024, 16, 16, 256, 6.9 );
    ret = imageptrs->CLAHE(pixelsBlue, 4688, nbroflinesreduced, 0, 1024, 16, 16, 256, 6.9);

    for (int line = 0; line < nbroflinesreduced; line++)
    {
        row = (QRgb*)imageptrs->ptrimageOLCI->scanLine(line);
        for (int pixelx = 0; pixelx < 4688; pixelx++)
        {
            pixval[0] = *(pixelsRed + line * 4688 + pixelx);
            pixval[0] = pixval[0] > 255 ? 255 : pixval[0];
            if(iscolor)
            {
                pixval[1] = *(pixelsGreen + line * 4688 + pixelx);
                pixval[2] = *(pixelsBlue + line * 4688 + pixelx);
                pixval[1] = pixval[1] > 255 ? 255 : pixval[1];
                pixval[2] = pixval[2] > 255 ? 255 : pixval[2];

            }
            row[pixelx] = qRgb(pixval[0], iscolor ? pixval[1] : pixval[0], iscolor ? pixval[2] : pixval[0] );
        }
    }

    delete [] pixelsRed;
    delete [] pixelsGreen;
    delete [] pixelsBlue;
}

void SegmentListOLCI::CalculateLUT()
{
    qDebug() << "start SegmentListOLCI::CalculateLUT()";
    int earth_views = this->earth_views_per_scanline;
    long stats_ch[3][256];

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 256; j++)
        {
            stats_ch[k][j] = 0;
        }
    }

    bool composecolor;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
                    quint16 pixel = *(segm->ptrbaOLCI[k].data() + line * earth_views + pixelx);
                    if(pixel < 65535)
                    {
                        int pixcalc = 255 * (pixel - imageptrs->stat_min_ch[k]) / (imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k]);
                        pixcalc = ( pixcalc < 0 ? 0 : pixcalc);
                        pixcalc = ( pixcalc > 255 ? 255 : pixcalc );
                        stats_ch[k][pixcalc]++;
                    }
                }
            }
        }
        ++segsel;
    }


    //    for(int i = 0; i < 256; i++)
    //    {
    //        qDebug() << QString("stats_ch[0][%1] = %2").arg(i).arg(stats_ch[0][i]);
    //    }


    // float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    float newscale = 256.0 / imageptrs->active_pixels;

    unsigned long long sum_ch[3];

    for (int i=0; i < 3; i++)
    {
        sum_ch[i] = 0;
    }


    for( int i = 0; i < 256; i++)
    {
        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            sum_ch[k] += stats_ch[k][i];
            imageptrs->lut_ch[k][i] = (quint16)(sum_ch[k] * newscale);
            imageptrs->lut_ch[k][i] = ( imageptrs->lut_ch[k][i] > 255 ? 255 : imageptrs->lut_ch[k][i]);
        }
    }
}

void SegmentListOLCI::CalculateLUTFull()
{
    qDebug() << "start SegmentListOLCI::CalculateLUTFull()";
    int earth_views = this->earth_views_per_scanline;
    long stats_ch[3][1024];
    long stats_norm_ch[3][1024];

    for(int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 1024; j++)
        {
            stats_ch[k][j] = 0;
            stats_norm_ch[k][j] = 0;
        }
    }

    bool composecolor;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        composecolor = segm->composeColorImage();

        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
                    quint16 pixel = *(segm->ptrbaOLCI[k].data() + line * earth_views + pixelx) ;
                    quint16 indexout = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixel - imageptrs->stat_min_ch[k])/(float)(imageptrs->stat_max_ch[k] - imageptrs->stat_min_ch[k])), 0), 1023);
                    stats_ch[k][indexout]++;
                    quint16 pixelnorm = *(segm->ptrbaOLCInormalized[k].data() + line * earth_views + pixelx) ;
                    quint16 indexoutnorm = (quint16)qMin(qMax(qRound(1023.0 * (float)(pixelnorm - imageptrs->stat_min_norm_ch[k])/(float)(imageptrs->stat_max_norm_ch[k] - imageptrs->stat_min_norm_ch[k])), 0), 1023);
                    stats_norm_ch[k][indexoutnorm]++;
                }
            }
        }
        ++segsel;
    }


    //    for(int i = 0; i < 1024; i++)
    //    {
    //        qDebug() << QString("stats_ch[0][%1] = %2 ; stats_norm_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(stats_norm_ch[0][i]);
    //    }


    // float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    double newscale = (double)(1024.0 / imageptrs->active_pixels);

    qDebug() << QString("newscale = %1 active pixels = %2").arg(newscale).arg(imageptrs->active_pixels);

    unsigned long long sum_ch[3];
    unsigned long long sum_norm_ch[3];

    for (int i=0; i < 3; i++)
    {
        sum_ch[i] = 0;
        sum_norm_ch[i] = 0;
    }


    bool okmin[3], okmax[3];

    for(int k = 0; k < (composecolor ? 3 : 1); k++)
    {
        okmin[k] = false;
        okmax[k] = false;
    }

    // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
    for( int i = 0; i < 1024; i++)
    {
        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            sum_ch[k] += stats_ch[k][i];
            imageptrs->lut_ch[k][i] = (quint16)((double)sum_ch[k] * newscale);
            imageptrs->lut_ch[k][i] = ( imageptrs->lut_ch[k][i] > 1023 ? 1023 : imageptrs->lut_ch[k][i]);
            if(imageptrs->lut_ch[k][i] > 25 && okmin[k] == false)
            {
                okmin[k] = true;
                imageptrs->minRadianceIndex[k] = i;
            }
            if(imageptrs->lut_ch[k][i] > 997 && okmax[k] == false)
            {
                okmax[k] = true;
                imageptrs->maxRadianceIndex[k] = i;
            }
        }
    }

    for(int k = 0; k < (composecolor ? 3 : 1); k++)
    {
        okmin[k] = false;
        okmax[k] = false;
    }

    // min/maxRadianceIndex = index of 95% ( 2.5% of 1024 = 25, 97.5% of 1024 = 997 )
    for( int i = 0; i < 1024; i++)
    {
        for(int k = 0; k < (composecolor ? 3 : 1); k++)
        {
            sum_norm_ch[k] += stats_norm_ch[k][i];
            imageptrs->lut_norm_ch[k][i] = (quint16)((double)sum_norm_ch[k] * newscale);
            imageptrs->lut_norm_ch[k][i] = ( imageptrs->lut_norm_ch[k][i] > 1023 ? 1023 : imageptrs->lut_norm_ch[k][i]);
            if(imageptrs->lut_norm_ch[k][i] > 25 && okmin[k] == false)
            {
                okmin[k] = true;
                imageptrs->minRadianceIndexNormalized[k] = i;
            }
            if(imageptrs->lut_norm_ch[k][i] > 997 && okmax[k] == false)
            {
                okmax[k] = true;
                imageptrs->maxRadianceIndexNormalized[k] = i;
            }
        }
    }

    //    for(int i = 0; i < 1024; i++)
    //    {
    //        qDebug() << QString("stats_ch[0][%1] = %2 lut_ch[0][%3] = %4").arg(i).arg(stats_ch[0][i]).arg(i).arg(imageptrs->lut_ch[0][i]);
    //    }


    for(int k = 0; k < (composecolor ? 3 : 1); k++)
    {
        qDebug() << QString("minRadianceIndex [%1] = %2 maxRadianceIndex [%3] = %4").arg(k).arg(imageptrs->minRadianceIndex[k]).arg(k).arg(imageptrs->maxRadianceIndex[k]);
        qDebug() << QString("minRadianceIndexNormalized [%1] = %2 maxRadianceIndexNormalized [%3] = %4").arg(k).arg(imageptrs->minRadianceIndexNormalized[k]).arg(k).arg(imageptrs->maxRadianceIndexNormalized[k]);
    }

}

void SegmentListOLCI::CalculateLUTAlt()
{
    qDebug() << "start SegmentListOLCI::CalculateLUTAlt()";
    int earth_views = this->earth_views_per_scanline;
    long stats[256];

    for (int j = 0; j < 256; j++)
    {
        stats[j] = 0;
    }


    bool composecolor;

    QList<Segment *>::iterator segsel = segsselected.begin();
    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        composecolor = segm->composeColorImage();

        if(composecolor)
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
                    quint16 pixel0 = *(segm->ptrbaOLCI[0].data() + line * earth_views + pixelx);
                    quint16 pixel1 = *(segm->ptrbaOLCI[1].data() + line * earth_views + pixelx);
                    quint16 pixel2 = *(segm->ptrbaOLCI[2].data() + line * earth_views + pixelx);
                    quint16 pixcalc0 = 256 * (pixel0 - imageptrs->stat_min_ch[0]) / (imageptrs->stat_max_ch[0] - imageptrs->stat_min_ch[0]);
                    quint16 pixcalc1 = 256 * (pixel1 - imageptrs->stat_min_ch[1]) / (imageptrs->stat_max_ch[1] - imageptrs->stat_min_ch[1]);
                    quint16 pixcalc2 = 256 * (pixel2 - imageptrs->stat_min_ch[2]) / (imageptrs->stat_max_ch[2] - imageptrs->stat_min_ch[2]);

                    int pixel = (int)((float)pixcalc0 * 0.299 + (float)pixcalc1 * 0.587 + (float)pixcalc2 * 0.114);

                    pixel = ( pixel < 0 ? 0 : pixel);
                    pixel = ( pixel > 255 ? 255 : pixel );
                    stats[pixel]++;
                }
            }
        }
        else
        {
            for (int line = 0; line < segm->NbrOfLines; line++)
            {
                for (int pixelx = 0; pixelx < earth_views; pixelx++)
                {
                    int pixel0 = (int)*(segm->ptrbaOLCI[0].data() + line * earth_views + pixelx);
                    int pixcalc0 = 256 * (pixel0 - imageptrs->stat_min_ch[0]) / (imageptrs->stat_max_ch[0] - imageptrs->stat_min_ch[0]);

                    int pixel = pixcalc0;

                    pixel = ( pixel < 0 ? 0 : pixel);
                    pixel = ( pixel > 255 ? 255 : pixel );
                    stats[pixel]++;
                }
            }

        }
        ++segsel;
    }

    // float scale = 256.0 / (NbrOfSegmentLinesSelected() * earth_views);    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    float newscale = 256.0 / imageptrs->active_pixels;

    unsigned long long sum_ch = 0;

    for( int i = 0; i < 256; i++)
    {
        sum_ch += stats[i];
        imageptrs->lut_sentinel[i] = (quint16)(sum_ch * newscale);
        imageptrs->lut_sentinel[i] = ( imageptrs->lut_sentinel[i] > 255 ? 255 : imageptrs->lut_sentinel[i]);
    }
}

void SegmentListOLCI::ComposeGVProjection(int inputchannel, int histogrammethod, bool normalized)
{

    this->histogrammethod = histogrammethod;
    this->normalized = normalized;

    qDebug() << "SegmentListOLCIefr::ComposeGVProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentGVProjection(inputchannel, histogrammethod, normalized);
        emit segmentprojectionfinished(false);
        ++segit;
    }

    //the following code calculates a new LUT that only takes
    //the pixels in the projection into account and not the complete segment(s).
    if(histogrammethod == CMB_HISTO_EQUALIZE_PROJ)
    {
        CalculateProjectionLUT();
        segit = segsselected.begin();
        while ( segit != segsselected.end() )
        {
            (*segit)->RecalculateProjection();
            emit segmentprojectionfinished(false);
            ++segit;
        }
    }

}

void SegmentListOLCI::ComposeLCCProjection(int inputchannel, int histogrammethod, bool normalized)
{
    this->histogrammethod = histogrammethod;
    this->normalized = normalized;

    qDebug() << "SegmentListOLCIefr::ComposeLCCProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentLCCProjection(inputchannel, histogrammethod, normalized);
        emit segmentprojectionfinished(false);
        ++segit;
    }

    //the following code calculates a new LUT that only takes
    //the pixels in the projection into account and not the complete segment(s).
    if(histogrammethod == CMB_HISTO_EQUALIZE_PROJ)
    {
        CalculateProjectionLUT();
        segit = segsselected.begin();
        while ( segit != segsselected.end() )
        {
            (*segit)->RecalculateProjection();
            emit segmentprojectionfinished(false);
            ++segit;
        }
    }

}

void SegmentListOLCI::ComposeSGProjection(int inputchannel, int histogrammethod, bool normalized)
{
    this->histogrammethod = histogrammethod;
    this->normalized = normalized;

    qDebug() << "SegmentListOLCIefr::ComposeSGProjection()";
    QList<Segment *>::iterator segit = segsselected.begin();
    while ( segit != segsselected.end() )
    {
        (*segit)->ComposeSegmentSGProjection(inputchannel, histogrammethod, normalized);
        emit segmentprojectionfinished(false);
        ++segit;
    }

    //the following code calculates a new LUT that only takes
    //the pixels in the projection into account and not the complete segment(s).
    if(histogrammethod == CMB_HISTO_EQUALIZE_PROJ)
    {
        CalculateProjectionLUT();
        segit = segsselected.begin();
        while ( segit != segsselected.end() )
        {
            (*segit)->RecalculateProjection();
            emit segmentprojectionfinished(false);
            ++segit;
        }
    }

}

void SegmentListOLCI::SmoothOLCIImage(bool combine)
{

    qDebug() << "start SegmentListOLCI::SmoothOLCIImage()";
    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentOLCI *segmsave;

    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments(segmsave, segm, combine);
        segmsave = segm;
        BilinearInterpolation(segm, combine);
        ++segsel;
    }
}

void SegmentListOLCI::SmoothOLCIImage12bits()
{

    qDebug() << "start SegmentListOLCI::SmoothOLCIImage12bits()";

    QList<Segment *>::iterator segsel;
    segsel = segsselected.begin();

    SegmentOLCI *segmsave;

    while ( segsel != segsselected.end() )
    {
        SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
        if(segsel != segsselected.begin())
            BilinearBetweenSegments12bits(segmsave, segm);
        segmsave = segm;
        BilinearInterpolation12bits(segm);
        ++segsel;
    }
}

void SegmentListOLCI::ShowWinvec(QPainter *painter, float distance, const QMatrix4x4 modelview)
{

    QList<Segment*>::iterator segit = segmentlist.begin();
    QVector2D winvecend1, winvecend2, winvecend3, winvecend4;

    QVector3D vecZ = modelview.row(2).toVector3D();

    //    static GLfloat mat[16];
    //    const float *data = modelview.constData();
    //    for (int index = 0; index < 16; ++index)
    //         mat[index] = data[index];

    //modelview.inverted( &ok );

    while ( segit != segmentlist.end() )
    {
        if( (*segit)->segmentshow)
        {
            for(int i = 0; i < (*segit)->winvectorfirst.length()-1; i++)
            {
                winvecend1.setX((*segit)->winvectorfirst.at(i).x()); winvecend1.setY((*segit)->winvectorfirst.at(i).y());
                winvecend2.setX((*segit)->winvectorfirst.at(i+1).x()); winvecend2.setY((*segit)->winvectorfirst.at(i+1).y());
                winvecend3.setX((*segit)->winvectorlast.at(i).x()); winvecend3.setY((*segit)->winvectorlast.at(i).y());
                winvecend4.setX((*segit)->winvectorlast.at(i+1).x()); winvecend4.setY((*segit)->winvectorlast.at(i+1).y());

                //qDebug() << "winvec1 x = " << winvec1.x() << "  y = " << winvec1.y();


                //     first                                          last
                //  winvecend1 ------------------------------------ winvecend3
                //      | p01                                           | p03
                //      |                                               |
                //      |                                               |
                //      |                                               |
                //      |                                               |
                //      |                                               |
                //      | p02                                           | p04
                //  winvecend2 ------------------------------------ winvecend4
                //


                qreal angle = ArcCos(QVector3D::dotProduct( vecZ, (*segit)->vecvector.at(i)));

                if (angle < PI/2  + (asin(1/distance)))
                {

                    painter->drawLine((int)winvecend1.x(), (painter->device())->height() - (int)winvecend1.y(), (int)winvecend3.x(), (painter->device())->height() - (int)winvecend3.y() );

                    painter->drawLine((int)winvecend1.x(), (painter->device())->height() - (int)winvecend1.y(), (int)winvecend2.x(), (painter->device())->height() - (int)winvecend2.y() );
                    painter->drawLine((int)winvecend3.x(), (painter->device())->height() - (int)winvecend3.y(), (int)winvecend4.x(), (painter->device())->height() - (int)winvecend4.y() );

                    //                painter->drawLine((int)winvec1.x(), (painter->device())->height() - (int)winvec1.y(), (int)winvecend1.x(), (painter->device())->height() - (int)winvecend1.y() );
                    //                painter->drawLine((int)winvec2.x(), (painter->device())->height() - (int)winvec2.y(), (int)winvecend2.x(), (painter->device())->height() - (int)winvecend2.y() );
                    //                painter->drawLine((int)winvec1.x(), (painter->device())->height() - (int)winvec1.y(), (int)winvecend3.x(), (painter->device())->height() - (int)winvecend3.y() );
                    //                painter->drawLine((int)winvec2.x(), (painter->device())->height() - (int)winvec2.y(), (int)winvecend4.x(), (painter->device())->height() - (int)winvecend4.y() );

                }
            }
        }
        ++segit;
    }
}

