#include "avhrrsatellite.h"

#include <QDebug>
#include <QDate>
#include <QApplication>

template <typename T>
struct PtrLess // public std::binary_function<bool, const T*, const T*>
{
  bool operator()(const T* a, const T* b) const
  {
    // may want to check that the pointers aren't zero...
    return *a < *b;
  }
};

extern Options opts;
extern SegmentImage *imageptrs;

bool LessThan(const QFileInfo &s1, const QFileInfo &s2)
{
    return s1.fileName().mid(46, 12) + s1.fileName().mid(26, 6) + s1.fileName().mid(36, 6) < s2.fileName().mid(46, 12) + s2.fileName().mid(26, 6) + s2.fileName().mid(36, 6);
}

AVHRRSatellite::AVHRRSatellite(QObject *parent, SatelliteList *satl) :
  QObject(parent)
{
    qDebug() << QString("constructor AVHRRSatellite");

    satlist = satl;

    seglmetop = new SegmentListMetop();
    seglnoaa = new SegmentListNoaa(satlist);
    seglhrp = new SegmentListHRP();
    seglgac = new SegmentListGAC();
    seglviirsm = new SegmentListVIIRSM();
    seglviirsdnb = new SegmentListVIIRSDNB();
    seglolciefr = new SegmentListOLCI(SEG_OLCIEFR);
    seglolcierr = new SegmentListOLCI(SEG_OLCIERR);

    seglmeteosat = new SegmentListGeostationary();
    seglmeteosat->bisRSS = false;
    seglmeteosat->bActiveSegmentList = true;
    seglmeteosat->setGeoSatellite(SegmentListGeostationary::MET_10);
    seglmeteosat->geosatlon = opts.geostationarylistlon[SegmentListGeostationary::MET_10].toDouble();
    seglmeteosat->geosatname = opts.geostationarylistname[SegmentListGeostationary::MET_10];

    seglmeteosatrss = new SegmentListGeostationary();
    seglmeteosatrss->bisRSS = true;
    seglmeteosatrss->setGeoSatellite(SegmentListGeostationary::MET_9);
    seglmeteosatrss->geosatlon = opts.geostationarylistlon[SegmentListGeostationary::MET_9].toDouble();
    seglmeteosatrss->geosatname = opts.geostationarylistname[SegmentListGeostationary::MET_9];

    seglmet7 = new SegmentListGeostationary();
    seglmet7->bisRSS = false;
    seglmet7->setGeoSatellite(SegmentListGeostationary::MET_7);
    seglmet7->geosatlon = opts.geostationarylistlon[SegmentListGeostationary::MET_7].toDouble();
    seglmet7->geosatname = opts.geostationarylistname[SegmentListGeostationary::MET_7];

    seglmet8 = new SegmentListGeostationary();
    seglmet8->bisRSS = false;
    seglmet8->setGeoSatellite(SegmentListGeostationary::MET_8);
    seglmet8->geosatlon = opts.geostationarylistlon[SegmentListGeostationary::MET_8].toDouble();
    seglmet8->geosatname = opts.geostationarylistname[SegmentListGeostationary::MET_8];

    seglgoes13dc3 = new SegmentListGeostationary();
    seglgoes13dc3->bisRSS = false;
    seglgoes13dc3->setGeoSatellite(SegmentListGeostationary::GOES_13);
    seglgoes13dc3->geosatlon = opts.geostationarylistlon[SegmentListGeostationary::GOES_13].toDouble();
    seglgoes13dc3->geosatname = opts.geostationarylistname[SegmentListGeostationary::GOES_13];

    seglgoes15dc3 = new SegmentListGeostationary();
    seglgoes15dc3->bisRSS = false;
    seglgoes15dc3->setGeoSatellite(SegmentListGeostationary::GOES_15);
    seglgoes15dc3->geosatlon = opts.geostationarylistlon[SegmentListGeostationary::GOES_15].toDouble();
    seglgoes15dc3->geosatname = opts.geostationarylistname[SegmentListGeostationary::GOES_15];

    seglgoes13dc4 = new SegmentListGeostationary();
    seglgoes13dc4->bisRSS = false;
    seglgoes13dc4->setGeoSatellite(SegmentListGeostationary::GOES_13);
    seglgoes13dc4->geosatlon = opts.geostationarylistlon[SegmentListGeostationary::GOES_13].toDouble();
    seglgoes13dc4->geosatname = opts.geostationarylistname[SegmentListGeostationary::GOES_13];

    seglgoes15dc4 = new SegmentListGeostationary();
    seglgoes15dc4->bisRSS = false;
    seglgoes15dc4->setGeoSatellite(SegmentListGeostationary::GOES_15);
    seglgoes15dc4->geosatlon = opts.geostationarylistlon[SegmentListGeostationary::GOES_15].toDouble();
    seglgoes15dc4->geosatname = opts.geostationarylistname[SegmentListGeostationary::GOES_15];

    seglfy2e = new SegmentListGeostationary();
    seglfy2e->bisRSS = false;
    seglfy2e->setGeoSatellite(SegmentListGeostationary::FY2E);
    seglfy2e->geosatlon = opts.geostationarylistlon[SegmentListGeostationary::FY2E].toDouble();
    seglfy2e->geosatname = opts.geostationarylistname[SegmentListGeostationary::FY2E];

    seglfy2g = new SegmentListGeostationary();
    seglfy2g->bisRSS = false;
    seglfy2g->setGeoSatellite(SegmentListGeostationary::FY2G);
    seglfy2g->geosatlon = opts.geostationarylistlon[SegmentListGeostationary::FY2G].toDouble();
    seglfy2g->geosatname = opts.geostationarylistname[SegmentListGeostationary::FY2G];

    seglh8 = new SegmentListGeostationary();
    seglh8->bisRSS = false;
    seglh8->setGeoSatellite(SegmentListGeostationary::H8);
    seglh8->geosatlon = opts.geostationarylistlon[SegmentListGeostationary::H8].toDouble();
    seglh8->geosatname = opts.geostationarylistname[SegmentListGeostationary::H8];


    countmetop = 0;
    countnoaa = 0;
    counthrp = 0;
    countgac = 0;
    countviirsm = 0;
    countviirsdnb = 0;
    countolciefr = 0;
    countolcierr = 0;

    showallsegments = false;

}

void AVHRRSatellite::emitProgressCounter(int counter)
{
    emit progressCounter(counter);
}

/**
 * @brief AVHRRSatellite::AddSegmentsToList
 * @param fileinfolist
 *
 */
void AVHRRSatellite::AddSegmentsToList(QFileInfoList fileinfolist)
{
    QFileInfo fileInfo;
    QDir segmentdir;

    SegmentMetop *segmetop;
    SegmentNoaa *segnoaa;
    SegmentHRP *seghrp;
    SegmentGAC *seggac;
    SegmentVIIRSM *segviirsm;
    SegmentVIIRSDNB *segviirsdnb;
    SegmentOLCI *segolciefr;
    SegmentOLCI *segolcierr;


    QList<Segment*> *slmetop = seglmetop->GetSegmentlistptr();
    QList<Segment*> *slnoaa = seglnoaa->GetSegmentlistptr();
    QList<Segment*> *slhrp = seglhrp->GetSegmentlistptr();
    QList<Segment*> *slgac = seglgac->GetSegmentlistptr();
    QList<Segment*> *slviirsm = seglviirsm->GetSegmentlistptr();
    QList<Segment*> *slviirsdnb = seglviirsdnb->GetSegmentlistptr();
    QList<Segment*> *slolciefr = seglolciefr->GetSegmentlistptr();
    QList<Segment*> *slolcierr = seglolcierr->GetSegmentlistptr();

    int counter = 0;

    for (int i = 0; i < fileinfolist.size(); ++i)
    {
        fileInfo = fileinfolist.at(i);
        counter++;

        if (fileInfo.size() == 0)
            continue;

        if (fileInfo.fileName().mid( 0, 8) == "AVHR_xxx" && fileInfo.fileName().mid( 67, 4) == ".bz2")   // EPS-10
        {
            seglmetop->SetDirectoryName(fileInfo.absolutePath());
            QFile file( segmentdir.filePath(fileInfo.absoluteFilePath()));
            segmetop = new SegmentMetop(&file,satlist);
            if(segmetop->segmentok == true)
            {
                slmetop->append(segmetop);
                countmetop++;
            }
            else
                delete segmetop;
        } else if (fileInfo.fileName().mid( 0, 6) == "avhrr_" && fileInfo.fileName().mid( 22, 6) == "noaa19")  // Data Channel 1
        {
            seglnoaa->SetDirectoryName(fileInfo.absolutePath());
            if (satlist->SatExistInList(33591) )
            {
                QFile file( segmentdir.filePath(fileInfo.absoluteFilePath()));
                segnoaa = new SegmentNoaa(&file, satlist);
                if(segnoaa->segmentok == true)
                {
                    slnoaa->append(segnoaa);
                    countnoaa++;
                }
                else
                    delete segnoaa;
            }
        } else if (fileInfo.fileName().mid( 0, 8) == "AVHR_HRP" && fileInfo.fileName().mid( 67, 4) == ".bz2")   // Data Channel 1
        {
            seglhrp->SetDirectoryName(fileInfo.absolutePath());
            QFile file( segmentdir.filePath(fileInfo.absoluteFilePath()));
            seghrp = new SegmentHRP(&file,satlist);
            if(seghrp->segmentok == true)
            {
                slhrp->append(seghrp);
                counthrp++;
            }
            else
                delete seghrp;

        } else if (fileInfo.fileName().mid( 0, 8) == "AVHR_GAC") // EPS-15
        {
            seglgac->SetDirectoryName(fileInfo.absolutePath());
            QFile file( segmentdir.filePath(fileInfo.absoluteFilePath()));
            seggac = new SegmentGAC(&file, satlist);
            if(seggac->segmentok == true)
            {
                slgac->append(seggac);
                countgac++;
            }
            else
                delete seggac;
        } else if (fileInfo.fileName().mid( 0, 8) == "SVMC_npp" && fileInfo.fileName().mid( 77, 3) == "bz2") // NPP-2
        {
            seglviirsm->SetDirectoryName(fileInfo.absolutePath());
            QFile file( segmentdir.filePath(fileInfo.absoluteFilePath()));
            segviirsm = new SegmentVIIRSM(&file, satlist);
            if(segviirsm->segmentok == true)
            {
                slviirsm->append(segviirsm);
                countviirsm++;
            }
            else
                delete segviirsm;
        } else if (fileInfo.fileName().mid( 0, 10) == "SVDNBC_npp" && fileInfo.fileName().mid( 79, 3) == "bz2") // NPP-2
        {
            //SVDNBC_npp_d20150810_t0033443_e0035085_b19602_c20150824113128000166_eum_ops.h5.bz2
            //0123456789012345678901234567890123456789012345678901234567890123456789012345678901
            seglviirsdnb->SetDirectoryName(fileInfo.absolutePath());
            QFile file( segmentdir.filePath(fileInfo.absoluteFilePath()));
            segviirsdnb = new SegmentVIIRSDNB(&file, satlist);
            if(segviirsdnb->segmentok == true)
            {
                slviirsdnb->append(segviirsdnb);
                countviirsdnb++;
            }
            else
                delete segviirsdnb;
        } else if (fileInfo.fileName().mid( 0, 12) == "S3A_OL_1_EFR" && fileInfo.fileName().mid( 100, 3) == "tar") // S3A EFR
        {
            //S3A_OL_1_EFR____20161026T121318_20161026T121318_20161026T163853_0000_010_166______MAR_O_NR_002.SEN3.tar
            //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            //0         1         2         3         4         5         6         7         8         9         10
            seglolciefr->SetDirectoryName(fileInfo.absolutePath());
            QFile file( segmentdir.filePath(fileInfo.absoluteFilePath()));
            segolciefr = new SegmentOLCI(SEG_OLCIEFR, &file, satlist);
            if(segolciefr->segmentok == true)
            {
                slolciefr->append(segolciefr);
                countolciefr++;
            }
            else
                delete segolciefr;
        } else if (fileInfo.fileName().mid( 0, 12) == "S3A_OL_1_ERR" && fileInfo.fileName().mid( 100, 3) == "tar") // S3A ERR
        {
            //S3A_OL_1_ERR____20161026T121318_20161026T121318_20161026T163853_0000_010_166______MAR_O_NR_002.SEN3.tar
            //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
            //0         1         2         3         4         5         6         7         8         9         10
            seglolcierr->SetDirectoryName(fileInfo.absolutePath());
            QFile file( segmentdir.filePath(fileInfo.absoluteFilePath()));
            segolcierr = new SegmentOLCI(SEG_OLCIERR, &file, satlist);
            if(segolcierr->segmentok == true)
            {
                slolcierr->append(segolcierr);
                countolcierr++;
            }
            else
                delete segolcierr;
        } else if (fileInfo.fileName().mid( 0, 9) == "H-000-MSG" && fileInfo.fileName().mid( 13, 3) == "MSG" &&
                   fileInfo.fileName().mid( 18, 3) == "___" && fileInfo.fileName().mid( 59, 2) == "C_")
        // Data Channel 2
        {
            //012345678901234567890123456789012345678901234567890123456789012
            //H-000-MSG3__-MSG3________-HRV______-000001___-201310270845-C_
            //H-000-MSG1__-MSG1________-IR_016___-000007___-201307011145-C_
            //H-000-MSG1__-MSG1_IODC____-HRV______-000005___-201605091215-C_

            int filenbr = fileInfo.fileName().mid(36, 6).toInt();
            QString strspectrum = fileInfo.fileName().mid(26, 6);
            QString strdate = fileInfo.fileName().mid(46, 12);
            //qDebug() << strdate << strspectrum << QString("%1").arg(filenbr);

            if (strspectrum != "______")
            {
                seglmeteosat->setImagePath(fileInfo.absolutePath());

                QMap<int, QFileInfo> hashfile;
                QMap<QString, QMap<int, QFileInfo> > hashspectrum;

                if (segmentlistmapmeteosat.contains(strdate))
                {
                    hashspectrum = segmentlistmapmeteosat.value(strdate);
                    if (hashspectrum.contains(strspectrum))
                        hashfile = hashspectrum.value(strspectrum);
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert( strspectrum, hashfile);
                    segmentlistmapmeteosat.insert(strdate, hashspectrum);
                }
                else
                {
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert(strspectrum, hashfile);
                    segmentlistmapmeteosat.insert( strdate, hashspectrum );
                }

            }
        } else if (fileInfo.fileName().mid( 0, 9) == "H-000-MSG" && fileInfo.fileName().mid( 13, 3) == "MSG" &&
                   fileInfo.fileName().mid( 18, 3) == "RSS" && fileInfo.fileName().mid( 59, 2) == "C_")
            //   // Data Channel 5
        {
            //H-000-MSG1__-MSG1_RSS____-HRV______-000016___-201211210610-C_

            int filenbr = fileInfo.fileName().mid(36, 6).toInt();
            QString strspectrum = fileInfo.fileName().mid(26, 6);
            QString strdate = fileInfo.fileName().mid(46, 12);
            //qDebug() << strdate << strspectrum << QString("%1").arg(filenbr);

            if (strspectrum != "______")
            {
                seglmeteosatrss->setImagePath(fileInfo.absolutePath());

                QMap<int, QFileInfo> hashfile;
                QMap<QString, QMap<int, QFileInfo> > hashspectrum;

                if (segmentlistmapmeteosatrss.contains(strdate))
                {
                    hashspectrum = segmentlistmapmeteosatrss.value(strdate);
                    if (hashspectrum.contains(strspectrum))
                        hashfile = hashspectrum.value(strspectrum);
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert( strspectrum, hashfile);
                    segmentlistmapmeteosatrss.insert(strdate, hashspectrum);
                }
                else
                {
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert(strspectrum, hashfile);
                    segmentlistmapmeteosatrss.insert( strdate, hashspectrum );
                }

            }
        } else if (fileInfo.fileName().mid( 0, 9) == "H-000-MSG" && fileInfo.fileName().mid( 13, 3) == "MSG" &&
                   fileInfo.fileName().mid( 18, 4) == "IODC" && fileInfo.fileName().mid( 59, 2) == "C_")
            //   // E1B-GEO-1
        {
            //012345678901234567890123456789012345678901234567890123456789012
            //H-000-MSG1__-MSG1_IODC___-HRV______-000001___-201610060845-C_
            int filenbr = fileInfo.fileName().mid(36, 6).toInt();
            QString strspectrum = fileInfo.fileName().mid(26, 6);
            QString strdate = fileInfo.fileName().mid(46, 12);
            //qDebug() << strdate << strspectrum << QString("%1").arg(filenbr);

            if (strspectrum != "______")
            {
                seglmet8->setImagePath(fileInfo.absolutePath());

                QMap<int, QFileInfo> hashfile;
                QMap<QString, QMap<int, QFileInfo> > hashspectrum;

                if (segmentlistmapmet8.contains(strdate))
                {
                    hashspectrum = segmentlistmapmet8.value(strdate);
                    if (hashspectrum.contains(strspectrum))
                        hashfile = hashspectrum.value(strspectrum);
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert( strspectrum, hashfile);
                    segmentlistmapmet8.insert(strdate, hashspectrum);
                }
                else
                {
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert(strspectrum, hashfile);
                    segmentlistmapmet8.insert( strdate, hashspectrum );
                }

            }
        } else if (fileInfo.fileName().mid( 0, 17) == "L-000-MTP___-MET7" && fileInfo.fileName().mid( 59, 2) == "C_") // Data Channel 3
        {
            //L-000-MTP___-MET7________-06_4_057E-000004___-201403300930-C_
            //L-000-MTP___-MET7________-06_4_057E-PRO______-201404011600-__

            int filenbr = fileInfo.fileName().mid(36, 6).toInt();
            QString strspectrum = fileInfo.fileName().mid(26, 6);
            QString strdate = fileInfo.fileName().mid(46, 12);
            //qDebug() << strdate << strspectrum << QString("%1").arg(filenbr);

            if (strspectrum != "______")
            {
                seglmet7->setImagePath(fileInfo.absolutePath());

                QMap<int, QFileInfo> hashfile;
                QMap<QString, QMap<int, QFileInfo> > hashspectrum;

                if (segmentlistmapmet7.contains(strdate))
                {
                    hashspectrum = segmentlistmapmet7.value(strdate);
                    if (hashspectrum.contains(strspectrum))
                        hashfile = hashspectrum.value(strspectrum);
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert( strspectrum, hashfile);
                    segmentlistmapmet7.insert(strdate, hashspectrum);
                }
                else
                {
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert(strspectrum, hashfile);
                    segmentlistmapmet7.insert( strdate, hashspectrum );
                }

            }
        } else if (fileInfo.fileName().mid( 0, 19) == "L-000-MSG3__-GOES13" && fileInfo.fileName().mid( 59, 2) == "C_" &&
           //        && fileInfo.absolutePath().contains("Data Channel 3", Qt::CaseInsensitive))  // Data Channel 3
                   (fileInfo.fileName().mid(54, 4) == "0000" ||
                    fileInfo.fileName().mid(54, 4) == "0300" ||
                    fileInfo.fileName().mid(54, 4) == "0600" ||
                    fileInfo.fileName().mid(54, 4) == "0900" ||
                    fileInfo.fileName().mid(54, 4) == "1200" ||
                    fileInfo.fileName().mid(54, 4) == "1500" ||
                    fileInfo.fileName().mid(54, 4) == "1800" ||
                    fileInfo.fileName().mid(54, 4) == "2100"))
        {
            //L-000-MSG3__-GOES13______-00_7_075W-000001___-201404031200-C_
            //L-000-MSG3__-GOES13______-00_7_075W-PRO______-201404031200-__

            int filenbr = fileInfo.fileName().mid(36, 6).toInt();
            QString strspectrum = fileInfo.fileName().mid(26, 6);
            QString strdate = fileInfo.fileName().mid(46, 12);
            // qDebug() << "Data Channel 3 - GOES13 " << strdate << " " << strspectrum << " " << QString("%1").arg(filenbr);

            if (strspectrum != "______")
            {
                seglgoes13dc3->setImagePath(fileInfo.absolutePath());

                QMap<int, QFileInfo> hashfile;
                QMap<QString, QMap<int, QFileInfo> > hashspectrum;

                if (segmentlistmapgoes13dc3.contains(strdate))
                {
                    hashspectrum = segmentlistmapgoes13dc3.value(strdate);
                    if (hashspectrum.contains(strspectrum))
                        hashfile = hashspectrum.value(strspectrum);
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert( strspectrum, hashfile);
                    segmentlistmapgoes13dc3.insert(strdate, hashspectrum);
                }
                else
                {
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert(strspectrum, hashfile);
                    segmentlistmapgoes13dc3.insert( strdate, hashspectrum );
                }

            }
        }
        else if (fileInfo.fileName().mid( 0, 19) == "L-000-MSG3__-GOES13" && fileInfo.fileName().mid( 59, 2) == "C_" &&
              //     && fileInfo.absolutePath().contains("Data Channel 4", Qt::CaseInsensitive))  // Data Channel 4
                 (fileInfo.fileName().mid(54, 4) == "0100" ||
                  fileInfo.fileName().mid(54, 4) == "0200" ||
                  fileInfo.fileName().mid(54, 4) == "0400" ||
                  fileInfo.fileName().mid(54, 4) == "0500" ||
                  fileInfo.fileName().mid(54, 4) == "0700" ||
                  fileInfo.fileName().mid(54, 4) == "0800" ||
                  fileInfo.fileName().mid(54, 4) == "1000" ||
                  fileInfo.fileName().mid(54, 4) == "1100" ||
                  fileInfo.fileName().mid(54, 4) == "1300" ||
                  fileInfo.fileName().mid(54, 4) == "1400" ||
                  fileInfo.fileName().mid(54, 4) == "1600" ||
                  fileInfo.fileName().mid(54, 4) == "1700" ||
                  fileInfo.fileName().mid(54, 4) == "1900" ||
                  fileInfo.fileName().mid(54, 4) == "2000" ||
                  fileInfo.fileName().mid(54, 4) == "2200" ||
                  fileInfo.fileName().mid(54, 4) == "2300"))

        {
            //L-000-MSG3__-GOES13______-00_7_075W-000001___-201404031200-C_
            //L-000-MSG3__-GOES13______-00_7_075W-PRO______-201404031200-__

            int filenbr = fileInfo.fileName().mid(36, 6).toInt();
            QString strspectrum = fileInfo.fileName().mid(26, 6);
            QString strdate = fileInfo.fileName().mid(46, 12);

            if (strspectrum != "______")
            {
                seglgoes13dc4->setImagePath(fileInfo.absolutePath());

                QMap<int, QFileInfo> hashfile;
                QMap<QString, QMap<int, QFileInfo> > hashspectrum;

                if (segmentlistmapgoes13dc4.contains(strdate))
                {
                    hashspectrum = segmentlistmapgoes13dc4.value(strdate);
                    if (hashspectrum.contains(strspectrum))
                        hashfile = hashspectrum.value(strspectrum);
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert( strspectrum, hashfile);
                    segmentlistmapgoes13dc4.insert(strdate, hashspectrum);
                }
                else
                {
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert(strspectrum, hashfile);
                    segmentlistmapgoes13dc4.insert( strdate, hashspectrum );
                }

            }
        }
        else if (fileInfo.fileName().mid( 0, 19) == "L-000-MSG3__-GOES15" && fileInfo.fileName().mid( 59, 2) == "C_" &&
              //     fileInfo.absolutePath().contains("Data Channel 3", Qt::CaseInsensitive))   // Data Channel 3
                 (fileInfo.fileName().mid(54, 4) == "0000" ||
                  fileInfo.fileName().mid(54, 4) == "0300" ||
                  fileInfo.fileName().mid(54, 4) == "0600" ||
                  fileInfo.fileName().mid(54, 4) == "0900" ||
                  fileInfo.fileName().mid(54, 4) == "1200" ||
                  fileInfo.fileName().mid(54, 4) == "1500" ||
                  fileInfo.fileName().mid(54, 4) == "1800" ||
                  fileInfo.fileName().mid(54, 4) == "2100"))

        {
            //L-000-MSG3__-GOES15______-00_7_135W-000001___-201404031500-C_
            //L-000-MSG3__-GOES15______-00_7_135W-PRO______-201404031500-__

            int filenbr = fileInfo.fileName().mid(36, 6).toInt();
            QString strspectrum = fileInfo.fileName().mid(26, 6);
            QString strdate = fileInfo.fileName().mid(46, 12);
            //qDebug() << "GOES15 " << strdate << " " << strspectrum << " " << QString("%1").arg(filenbr);

            if (strspectrum != "______")
            {
                seglgoes15dc3->setImagePath(fileInfo.absolutePath());

                QMap<int, QFileInfo> hashfile;
                QMap<QString, QMap<int, QFileInfo> > hashspectrum;

                if (segmentlistmapgoes15dc3.contains(strdate))
                {
                    hashspectrum = segmentlistmapgoes15dc3.value(strdate);
                    if (hashspectrum.contains(strspectrum))
                        hashfile = hashspectrum.value(strspectrum);
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert( strspectrum, hashfile);
                    segmentlistmapgoes15dc3.insert(strdate, hashspectrum);
                }
                else
                {
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert(strspectrum, hashfile);
                    segmentlistmapgoes15dc3.insert( strdate, hashspectrum );
                }

            }
        } else if (fileInfo.fileName().mid( 0, 19) == "L-000-MSG3__-GOES15" && fileInfo.fileName().mid( 59, 2) == "C_" &&
                //   fileInfo.absolutePath().contains("Data Channel 4", Qt::CaseInsensitive))   // Data Channel 4
                   (fileInfo.fileName().mid(54, 4) == "0100" ||
                    fileInfo.fileName().mid(54, 4) == "0200" ||
                    fileInfo.fileName().mid(54, 4) == "0400" ||
                    fileInfo.fileName().mid(54, 4) == "0500" ||
                    fileInfo.fileName().mid(54, 4) == "0700" ||
                    fileInfo.fileName().mid(54, 4) == "0800" ||
                    fileInfo.fileName().mid(54, 4) == "1000" ||
                    fileInfo.fileName().mid(54, 4) == "1100" ||
                    fileInfo.fileName().mid(54, 4) == "1300" ||
                    fileInfo.fileName().mid(54, 4) == "1400" ||
                    fileInfo.fileName().mid(54, 4) == "1600" ||
                    fileInfo.fileName().mid(54, 4) == "1700" ||
                    fileInfo.fileName().mid(54, 4) == "1900" ||
                    fileInfo.fileName().mid(54, 4) == "2000" ||
                    fileInfo.fileName().mid(54, 4) == "2200" ||
                    fileInfo.fileName().mid(54, 4) == "2300"))

        {
            //L-000-MSG3__-GOES15______-00_7_135W-000001___-201404031500-C_
            //L-000-MSG3__-GOES15______-00_7_135W-PRO______-201404031500-__

            int filenbr = fileInfo.fileName().mid(36, 6).toInt();
            QString strspectrum = fileInfo.fileName().mid(26, 6);
            QString strdate = fileInfo.fileName().mid(46, 12);
            //qDebug() << "GOES15 " << strdate << " " << strspectrum << " " << QString("%1").arg(filenbr);

            if (strspectrum != "______")
            {
                seglgoes15dc4->setImagePath(fileInfo.absolutePath());

                QMap<int, QFileInfo> hashfile;
                QMap<QString, QMap<int, QFileInfo> > hashspectrum;

                if (segmentlistmapgoes15dc4.contains(strdate))
                {
                    hashspectrum = segmentlistmapgoes15dc4.value(strdate);
                    if (hashspectrum.contains(strspectrum))
                        hashfile = hashspectrum.value(strspectrum);
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert( strspectrum, hashfile);
                    segmentlistmapgoes15dc4.insert(strdate, hashspectrum);
                }
                else
                {
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert(strspectrum, hashfile);
                    segmentlistmapgoes15dc4.insert( strdate, hashspectrum );
                }

            }

        } else if (fileInfo.fileName().mid( 0, 6) == "IMG_DK")
            //IMG_DK01B04_201510090000_001.bz2
            //0123456789012345678901234567890
            //YYYYMMDDhhmm : YYYY=year, MM=month, DD=day of month, hh=hour, mm=minute
            //nnnn : ‘_001’-‘_010’ for segmented full earth’s disk image data files
            //Sequence number is set only for dissemination of the segment files.
        {
            int filenbr = fileInfo.fileName().mid(25, 3).toInt();
            QString strspectrum = fileInfo.fileName().mid(8, 3);
            QString strdate = fileInfo.fileName().mid(12, 11);
            // qDebug() << "Himawari-8 " << strdate << " " << strspectrum << " " << QString("%1").arg(filenbr);

            seglh8->setImagePath(fileInfo.absolutePath());

            QMap<int, QFileInfo> hashfile;
            QMap<QString, QMap<int, QFileInfo> > hashspectrum;

            if (segmentlistmaph8.contains(strdate))
            {
                hashspectrum = segmentlistmaph8.value(strdate);
                if (hashspectrum.contains(strspectrum))
                    hashfile = hashspectrum.value(strspectrum);
                hashfile.insert( filenbr, fileInfo );
                hashspectrum.insert( strspectrum, hashfile);
                segmentlistmaph8.insert(strdate, hashspectrum);
            }
            else
            {
                hashfile.insert( filenbr, fileInfo );
                hashspectrum.insert(strspectrum, hashfile);
                segmentlistmaph8.insert( strdate, hashspectrum );
            }
        } else if (fileInfo.fileName().mid( 0, 14) == "Z_SATE_C_BABJ_" && fileInfo.fileName().mid( 31, 8) == "FY2E_FDI") // Data Channel 12
        {
            //0123456789012345678901234567890123456789012345678901234567890
            //Z_SATE_C_BABJ_20150623131500_O_FY2D_FDI_IR1_001_NOM.HDF.gz
            //Z_SATE_C_BABJ_20150717080000_O_FY2G_FDI_VIS1KM_001_NOM.HDF.gz
            int filenbr = fileInfo.fileName().mid(44, 3).toInt();
            QString strspectrum = fileInfo.fileName().mid(40, 3);
            if(fileInfo.fileName().mid(40, 6) == "VIS1KM")
                strspectrum = "VIS1KM";

            QString strdate = fileInfo.fileName().mid(14, 12);
            //qDebug() << strdate << strspectrum << QString("%1").arg(filenbr);

            if (strspectrum != "______")
            {
                seglfy2e->setImagePath(fileInfo.absolutePath());

                QMap<int, QFileInfo> hashfile;
                QMap<QString, QMap<int, QFileInfo> > hashspectrum;

                if (segmentlistmapfy2e.contains(strdate))
                {
                    hashspectrum = segmentlistmapfy2e.value(strdate);
                    if (hashspectrum.contains(strspectrum))
                        hashfile = hashspectrum.value(strspectrum);
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert( strspectrum, hashfile);
                    segmentlistmapfy2e.insert(strdate, hashspectrum);
                }
                else
                {
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert(strspectrum, hashfile);
                    segmentlistmapfy2e.insert( strdate, hashspectrum );
                }
            }
        }  else if (fileInfo.fileName().mid( 0, 14) == "Z_SATE_C_BABJ_" && fileInfo.fileName().mid( 31, 8) == "FY2G_FDI") // Data Channel 12
        {
            //0123456789012345678901234567890123456789012345678901234567890
            //Z_SATE_C_BABJ_20150623131500_O_FY2D_FDI_IR1_001_NOM.HDF.gz
            int filenbr = fileInfo.fileName().mid(44, 3).toInt();
            QString strspectrum = fileInfo.fileName().mid(40, 3);
            if(fileInfo.fileName().mid(40, 6) == "VIS1KM")
                strspectrum = "VIS1KM";

            QString strdate = fileInfo.fileName().mid(14, 12);
            //qDebug() << strdate << strspectrum << QString("%1").arg(filenbr);

            if (strspectrum != "______")
            {
                seglfy2g->setImagePath(fileInfo.absolutePath());

                QMap<int, QFileInfo> hashfile;
                QMap<QString, QMap<int, QFileInfo> > hashspectrum;

                if (segmentlistmapfy2g.contains(strdate))
                {
                    hashspectrum = segmentlistmapfy2g.value(strdate);
                    if (hashspectrum.contains(strspectrum))
                        hashfile = hashspectrum.value(strspectrum);
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert( strspectrum, hashfile);
                    segmentlistmapfy2g.insert(strdate, hashspectrum);
                }
                else
                {
                    hashfile.insert( filenbr, fileInfo );
                    hashspectrum.insert(strspectrum, hashfile);
                    segmentlistmapfy2g.insert( strdate, hashspectrum );
                }

            }
        }

        emit signalProgress(i);
        QApplication::processEvents();
    }
}


void AVHRRSatellite::ReadDirectories(QDate seldate, int hoursbefore)
{
    QFileInfoList fileinfolist; // QList<QFileInfo>


    qDebug() << QString("in AVHRRSatellite:ReadDirectories(QDate, int) hoursbefore = %1").arg(hoursbefore);

    QApplication::setOverrideCursor( Qt::WaitCursor ); // this might take time

    QList<Segment*> *slnoaa = seglnoaa->GetSegmentlistptr();
    QList<Segment*> *slhrp = seglhrp->GetSegmentlistptr();
    QList<Segment*> *slgac = seglgac->GetSegmentlistptr();
    QList<Segment*> *slmetop = seglmetop->GetSegmentlistptr();
    QList<Segment*> *slviirsm = seglviirsm->GetSegmentlistptr();
    QList<Segment*> *slviirsdnb = seglviirsdnb->GetSegmentlistptr();
    QList<Segment*> *slolciefr = seglolciefr->GetSegmentlistptr();
    QList<Segment*> *slolcierr = seglolcierr->GetSegmentlistptr();

    seglnoaa->ClearSegments();
    seglhrp->ClearSegments();
    seglgac->ClearSegments();
    seglmetop->ClearSegments();
    seglviirsm->ClearSegments();
    seglviirsdnb->ClearSegments();
    seglolciefr->ClearSegments();
    seglolcierr->ClearSegments();

    segmentlistmapmeteosat.clear();
    segmentlistmapmeteosatrss.clear();
    segmentlistmapmet7.clear();
    segmentlistmapmet8.clear();
    segmentlistmapgoes13dc3.clear();
    segmentlistmapgoes15dc3.clear();
    segmentlistmapgoes13dc4.clear();
    segmentlistmapgoes15dc4.clear();
    segmentlistmapfy2e.clear();
    segmentlistmapfy2g.clear();
    segmentlistmaph8.clear();

    this->countmetop = 0;
    this->countgac = 0;
    this->counthrp = 0;
    this->countnoaa = 0;
    this->countviirsm = 0;
    this->countviirsdnb = 0;
    this->countviirsmdnb = 0;
    this->countolciefr = 0;
    this->countolcierr = 0;

    imageptrs->ptrProjectionBrightnessTemp.reset();
    imageptrs->ptrProjectionInfra.reset();

    QDir segmentdir;
    QDateTime datebefore;
    QString pathbefore;

    bool noaaTle = false;
    bool metopTle = false;
    bool nppTle = false;
    bool sentinel3Tle = false;

    if (opts.segmentdirectorylist.count() > 0)
    {
        QStringList::Iterator its = opts.segmentdirectorylist.begin();
        QStringList::Iterator itc = opts.segmentdirectorylistinc.begin(); //segmentdirectory checked

        while( its != opts.segmentdirectorylist.end() )
        {
            if (*itc == "1")  //include checked
            {
                segmentdir.cd( *its );
                segmentdir.setFilter(QDir::Files | QDir::NoSymLinks);
                segmentdir.setSorting(QDir::Name);
                fileinfolist = segmentdir.entryInfoList();

                if (fileinfolist.size() > 0)
                {
                    emit signalResetProgressbar(fileinfolist.size(), (*its));
                    QApplication::processEvents();
                }

                qDebug() << QString("fileinfolist.size = %1 in subdir %2").arg(fileinfolist.size()).arg(*its);

                QString yeardir = seldate.toString("yyyyMMdd").mid(0, 4);
                QString monthdir = seldate.toString("yyyyMMdd").mid(4, 2);
                QString daydir = seldate.toString("yyyyMMdd").mid(6, 2);

                QString thepath = (*its) + "/" + yeardir + monthdir + daydir;
                QString thepathYYYYMMDD = (*its) + "/" + yeardir + "/" + monthdir + "/" + daydir;

                if(segmentdir.cd( thepath ) || segmentdir.cd( thepathYYYYMMDD ))
                {
                    segmentdir.setFilter(QDir::Files | QDir::NoSymLinks);
                    segmentdir.setSorting(QDir::Name); //::Time);
                    fileinfolist.append(segmentdir.entryInfoList());
                    qDebug() << QString("fileinfolist.size = %1 in subdir %2").arg(fileinfolist.size()).arg(thepath);
                }

                QMap<QString, QFileInfo> map;

                InsertToMap(fileinfolist, &map, &noaaTle, &metopTle, &nppTle, &sentinel3Tle, seldate, 0);

                if(metopTle)
                {
                    bool ok1 = false, ok2 = false;
                    Satellite metop_sat;
                    ok1 = satlist->GetSatellite(29499, &metop_sat);
                    ok2 = satlist->GetSatellite(38771, &metop_sat);
                    if (ok1 == false || ok2 == false)
                    {
                        QApplication::restoreOverrideCursor();
                        QMessageBox msgBox;
                        msgBox.setText("Need the Metop TLE's.");
                        msgBox.exec();

                        return;
                    }
                }

                if(nppTle)
                {
                    bool ok = false;
                    Satellite nppsat;
                    ok = satlist->GetSatellite(37849, &nppsat);
                    if (ok == false)
                    {
                        QApplication::restoreOverrideCursor();
                        QMessageBox msgBox;
                        msgBox.setText("Need the Suomi TLE's.");
                        msgBox.exec();

                        return;
                    }
                }

                if(noaaTle)
                {
                    bool ok = false;
                    Satellite noaasat;
                    ok = satlist->GetSatellite(33591, &noaasat);
                    if (ok == false)
                    {
                        QApplication::restoreOverrideCursor();
                        QMessageBox msgBox;
                        msgBox.setText("Need the Noaa TLE's.");
                        msgBox.exec();

                        return;
                    }
                }

                if(sentinel3Tle)
                {
                    bool ok = false;
                    Satellite sentinelsat;
                    ok = satlist->GetSatellite(41335, &sentinelsat);
                    if (ok == false)
                    {
                        QApplication::restoreOverrideCursor();
                        QMessageBox msgBox;
                        msgBox.setText("Need the Sentinel 3 TLE's.");
                        msgBox.exec();

                        return;
                    }
                }

                if(hoursbefore > 0)
                {
                    datebefore.setDate(seldate);
                    datebefore.setDate(datebefore.date().addDays(-1));

                    QString pathbefore = (*its) + "/" + datebefore.toString( "yyyyMMdd").mid(0, 4) +
                            "/" + datebefore.toString( "yyyyMMdd").mid(4, 2) + "/" + datebefore.toString( "yyyyMMdd").mid(6, 2);
                    qDebug() << QString("pathbefore = %1").arg(pathbefore);

                    if(segmentdir.cd( pathbefore ))
                    {
                        segmentdir.setFilter(QDir::Files | QDir::NoSymLinks);
                        segmentdir.setSorting(QDir::Name); //::Time);
                        fileinfolist = segmentdir.entryInfoList();
                        qDebug() << QString("fileinfolist.size = %1 in subdir %2").arg(fileinfolist.size()).arg(pathbefore);
                        InsertToMap(fileinfolist, &map, &noaaTle, &metopTle, &nppTle, &sentinel3Tle, seldate, hoursbefore);

                    }
                }


                fileinfolist = map.values();

                emit signalResetProgressbar(fileinfolist.size(), (*its));

                if( fileinfolist.count() > 0)
                    AddSegmentsToList(fileinfolist);
                qDebug() << QString("ReadDirectories count = %1").arg(fileinfolist.count());
            }
            ++its;
            ++itc;

        }
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("There are no segment directories set !");
        msgBox.exec();
    }


    QApplication::restoreOverrideCursor();

    qDebug() << QString("Count segmentlistmetop = %1").arg(slmetop->count());
    qDebug() << QString("Count segmentlistnoaa = %1").arg(slnoaa->count());
    qDebug() << QString("Count segmentlisthrp = %1").arg(slhrp->count());
    qDebug() << QString("Count segmentlistgac = %1").arg(slgac->count());
    qDebug() << QString("Count segmentlistviirsm = %1").arg(slviirsm->count());
    qDebug() << QString("Count segmentlistviirsdnb = %1").arg(slviirsdnb->count());
    qDebug() << QString("Count segmentlistolciefr = %1").arg(slolciefr->count());
    qDebug() << QString("Count segmentlistolcierr = %1").arg(slolcierr->count());

    qDebug() << QString( "Nbr of items in segmentlist MET-10     = %1").arg(segmentlistmapmeteosat.size());
    qDebug() << QString( "Nbr of items in segmentlist MET-9      = %1").arg(segmentlistmapmeteosatrss.size());
    qDebug() << QString( "Nbr of items in segmentlist MET-8      = %1").arg(segmentlistmapmet8.size());
    qDebug() << QString( "Nbr of items in segmentlist MET-7      = %1").arg(segmentlistmapmet7.size());
    qDebug() << QString( "Nbr of items in segmentlist GOES-13    = %1").arg(segmentlistmapgoes13dc3.size() + segmentlistmapgoes13dc4.size());
    qDebug() << QString( "Nbr of items in segmentlist GOES-15    = %1").arg(segmentlistmapgoes15dc3.size() + segmentlistmapgoes15dc4.size());
    qDebug() << QString( "Nbr of items in segmentlist FY2E       = %1").arg(segmentlistmapfy2e.size());
    qDebug() << QString( "Nbr of items in segmentlist FY2G       = %1").arg(segmentlistmapfy2g.size());
    qDebug() << QString( "Nbr of items in segmentlist Himawari-8 = %1").arg(segmentlistmaph8.size());

    QString strtot = QString("Total segments = %1").arg(slmetop->count()+slnoaa->count()+slgac->count()+slhrp->count()+slviirsm->count()+slolciefr->count()+slolcierr->count()+
                                                        segmentlistmapmeteosat.size()+segmentlistmapmeteosatrss.size() +
                                                        segmentlistmapmet7.size() + segmentlistmapmet8.size() +
                                                        segmentlistmapfy2e.size() + segmentlistmapfy2g.size() +
                                                        segmentlistmapgoes13dc3.size() + segmentlistmapgoes13dc4.size() +
                                                        segmentlistmapgoes15dc3.size() + segmentlistmapgoes15dc4.size() +
                                                        segmentlistmaph8.size());
    emit signalResetProgressbar(1, strtot);
    emit signalNothingSelected();
}

void AVHRRSatellite::InsertToMap(QFileInfoList fileinfolist, QMap<QString, QFileInfo> *map, bool *noaaTle, bool *metopTle, bool *nppTle, bool *sentinel3Tle, QDate seldate, int hoursbefore)
{

    QDateTime filedate;
    bool fileok = false;

    foreach (const QFileInfo &fileinfo, fileinfolist)
    {
        fileok = false;

        //avhrr_20130701_151100_noaa19
        if (fileinfo.fileName().mid( 0, 6) == "avhrr_" && fileinfo.fileName().mid( 22, 6) == "noaa19")
        {
            *noaaTle = true;
            QDate d(fileinfo.fileName().mid( 6, 4).toInt(), fileinfo.fileName().mid( 10, 2).toInt(), fileinfo.fileName().mid( 12, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 15, 2).toInt(), fileinfo.fileName().mid( 17, 2).toInt(), fileinfo.fileName().mid( 19, 2).toInt());
            filedate.setTime(t);
            if(hoursbefore == 0)
                fileok = true;
            else if(t.hour() >= 24 - hoursbefore)
                fileok = true;
        }
        //AVHR_xxx_1B_M01_20130701051903Z_20130701052203Z_N_O_20130701054640Z
        //AVHR_GAC_1B_N19_20130701041003Z_20130701041303Z_N_O_20130701054958Z
        //AVHR_HRP_00_M02_20130701060200Z_20130701060300Z_N_O_20130701061314Z
        else if (fileinfo.fileName().mid( 0, 11) == "AVHR_GAC_1B" ||
                 fileinfo.fileName().mid( 0, 11) == "AVHR_HRP_00" ||
                 fileinfo.fileName().mid( 0, 11) == "AVHR_xxx_1B")
        {
            *metopTle = true;
            QDate d(fileinfo.fileName().mid( 16, 4).toInt(), fileinfo.fileName().mid( 20, 2).toInt(), fileinfo.fileName().mid( 22, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 24, 2).toInt(), fileinfo.fileName().mid( 26, 2).toInt(), fileinfo.fileName().mid( 28, 2).toInt());
            filedate.setTime(t);
            if(hoursbefore == 0)
                fileok = true;
            else if(t.hour() >= 24 - hoursbefore)
                fileok = true;
//            if(fileok == true)
//                qDebug() << fileinfo.fileName().mid( 12, 3) << " " << fileinfo.fileName().mid( 16, 8) << " " << fileinfo.fileName().mid( 24, 6);

        }
        //SVMC_npp_d20141117_t0837599_e0839241_b15833_c20141117084501709131_eum_ops
        //SVDNBC_npp_d20151019_t0013359_e0015001_b20595_c20151019002104000944_eum_ops.h5
        else if (fileinfo.fileName().mid( 0, 8) == "SVMC_npp")
        {
            *nppTle = true;
            QDate d(fileinfo.fileName().mid( 10, 4).toInt(), fileinfo.fileName().mid( 14, 2).toInt(), fileinfo.fileName().mid( 16, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 20, 2).toInt(), fileinfo.fileName().mid( 22, 2).toInt(), fileinfo.fileName().mid( 24, 2).toInt());
            filedate.setTime(t);
            if(hoursbefore == 0)
                fileok = true;
            else if(t.hour() >= 24 - hoursbefore)
                fileok = true;
        }
        else if (fileinfo.fileName().mid( 0, 10) == "SVDNBC_npp")
        {
            *nppTle = true;
            QDate d(fileinfo.fileName().mid( 12, 4).toInt(), fileinfo.fileName().mid( 16, 2).toInt(), fileinfo.fileName().mid( 18, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 22, 2).toInt(), fileinfo.fileName().mid( 24, 2).toInt(), fileinfo.fileName().mid( 26, 2).toInt());
            filedate.setTime(t);
            if(hoursbefore == 0)
                fileok = true;
            else if(t.hour() >= 24 - hoursbefore)
                fileok = true;
        }
        //0123456789012345678901234567890123456789
        //S3A_OL_1_EFR____20161026T121318_20161026T121318_20161026T163853_0000_010_166______MAR_O_NR_002.SEN3
        else if (fileinfo.fileName().mid( 0, 12) == "S3A_OL_1_EFR")
        {
            *sentinel3Tle = true;
            QDate d(fileinfo.fileName().mid( 16, 4).toInt(), fileinfo.fileName().mid( 20, 2).toInt(), fileinfo.fileName().mid( 22, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 25, 2).toInt(), fileinfo.fileName().mid( 27, 2).toInt(), fileinfo.fileName().mid( 29, 2).toInt());
            filedate.setTime(t);
            if(hoursbefore == 0)
            {
                if(d == seldate)
                    fileok = true;
            }
            else if(t.hour() >= 24 - hoursbefore)
                fileok = true;
        }
        //0123456789012345678901234567890123456789
        //S3A_OL_1_ERR____20161026T121318_20161026T121318_20161026T163853_0000_010_166______MAR_O_NR_002.SEN3
        else if (fileinfo.fileName().mid( 0, 12) == "S3A_OL_1_ERR")
        {
            *sentinel3Tle = true;
            QDate d(fileinfo.fileName().mid( 16, 4).toInt(), fileinfo.fileName().mid( 20, 2).toInt(), fileinfo.fileName().mid( 22, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 25, 2).toInt(), fileinfo.fileName().mid( 27, 2).toInt(), fileinfo.fileName().mid( 29, 2).toInt());
            filedate.setTime(t);
            if(hoursbefore == 0)
            {
                if(d == seldate)
                    fileok = true;
            }
            else if(t.hour() >= 24 - hoursbefore)
                fileok = true;
        }

        //H-000-MSG3__-MSG3________-HRV______-000001___-201310270845-C_
        //H-000-MSG1__-MSG1_RSS____-HRV______-000016___-201211210610-C_
        //H-000-GOMS1_-GOMS1_4_____-00_9_076E-000001___-201312231100-C_
        //L-000-MTP___-MET7________-06_4_057E-000004___-201403300930-C_
        //L-000-MSG3__-GOES13______-00_7_075W-000001___-201404031200-C_
        //L-000-MSG3__-GOES15______-00_7_135W-000001___-201404031500-C_
        //L-000-MSG3__-MTSAT2______-00_7_145E-000006___-201404032100-C_
        else if (fileinfo.fileName().mid( 0, 9) == "H-000-MSG" && fileinfo.fileName().mid( 59, 2) == "C_")
        {
            QDate d(fileinfo.fileName().mid( 46, 4).toInt(), fileinfo.fileName().mid( 50, 2).toInt(), fileinfo.fileName().mid( 52, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 54, 2).toInt(), fileinfo.fileName().mid( 56, 2).toInt(), 0);
            filedate.setTime(t);
            if(hoursbefore == 0 )
                fileok = true;
        }
        else if (fileinfo.fileName().mid( 0, 11) == "H-000-GOMS1" && fileinfo.fileName().mid( 59, 2) == "C_")
        {
            QDate d(fileinfo.fileName().mid( 46, 4).toInt(), fileinfo.fileName().mid( 50, 2).toInt(), fileinfo.fileName().mid( 52, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 54, 2).toInt(), fileinfo.fileName().mid( 56, 2).toInt(), 0);
            filedate.setTime(t);
            if(hoursbefore == 0)
                fileok = true;
        }
        else if (fileinfo.fileName().mid( 0, 17) == "L-000-MTP___-MET7" && fileinfo.fileName().mid( 59, 2) == "C_")
        {
            QDate d(fileinfo.fileName().mid( 46, 4).toInt(), fileinfo.fileName().mid( 50, 2).toInt(), fileinfo.fileName().mid( 52, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 54, 2).toInt(), fileinfo.fileName().mid( 56, 2).toInt(), 0);
            filedate.setTime(t);
            if(hoursbefore == 0)
                fileok = true;
        }
        else if (fileinfo.fileName().mid( 0, 19) == "L-000-MSG3__-GOES13" && fileinfo.fileName().mid( 59, 2) == "C_")
        {
            QDate d(fileinfo.fileName().mid( 46, 4).toInt(), fileinfo.fileName().mid( 50, 2).toInt(), fileinfo.fileName().mid( 52, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 54, 2).toInt(), fileinfo.fileName().mid( 56, 2).toInt(), 0);
            filedate.setTime(t);
            if(hoursbefore == 0)
                fileok = true;
        }
        else if (fileinfo.fileName().mid( 0, 19) == "L-000-MSG3__-GOES15" && fileinfo.fileName().mid( 59, 2) == "C_")
        {
            QDate d(fileinfo.fileName().mid( 46, 4).toInt(), fileinfo.fileName().mid( 50, 2).toInt(), fileinfo.fileName().mid( 52, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 54, 2).toInt(), fileinfo.fileName().mid( 56, 2).toInt(), 0);
            filedate.setTime(t);
            if(hoursbefore == 0)
                fileok = true;
        }
        //0123456789012345678901234567890123456789012345678901234567890
        //Z_SATE_C_BABJ_20150624130000_O_FY2G_FDI_IR1_001_NOM.HDF.gz
        else if (fileinfo.fileName().mid( 0, 14) == "Z_SATE_C_BABJ_" && fileinfo.fileName().mid( 31, 8) == "FY2E_FDI") // Data Channel 12
        {
            QDate d(fileinfo.fileName().mid( 14, 4).toInt(), fileinfo.fileName().mid( 18, 2).toInt(), fileinfo.fileName().mid( 20, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 22, 2).toInt(), fileinfo.fileName().mid( 24, 2).toInt(), 0);
            filedate.setTime(t);
            if(hoursbefore == 0)
                fileok = true;
        }
        else if (fileinfo.fileName().mid( 0, 14) == "Z_SATE_C_BABJ_" && fileinfo.fileName().mid( 31, 8) == "FY2G_FDI") // Data Channel 12
        {
            QDate d(fileinfo.fileName().mid( 14, 4).toInt(), fileinfo.fileName().mid( 18, 2).toInt(), fileinfo.fileName().mid( 20, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 22, 2).toInt(), fileinfo.fileName().mid( 24, 2).toInt(), 0);
            filedate.setTime(t);
            if(hoursbefore == 0)
                fileok = true;
        }
        //IMG_DK01B04_201510090000_001.bz2
        //0123456789012345678901234567890
        //YYYYMMDDhhmm : YYYY=year, MM=month, DD=day of month, hh=hour, mm=minute
        //nnnn : ‘_001’-‘_010’ for segmented full earth’s disk image data files
        //Sequence number is set only for dissemination of the segment files.
        else if (fileinfo.fileName().mid( 0, 6) == "IMG_DK") //Data Channel 4
        {
            QDate d(fileinfo.fileName().mid( 12, 4).toInt(), fileinfo.fileName().mid( 16, 2).toInt(), fileinfo.fileName().mid( 18, 2).toInt());
            filedate.setDate(d);
            QTime t(fileinfo.fileName().mid( 20, 2).toInt(), fileinfo.fileName().mid( 22, 2).toInt(), 0);
            filedate.setTime(t);
            if(hoursbefore == 0)
                fileok = true;
        }

        if(fileok)
        {
            *map->insert(fileinfo.fileName(), fileinfo);
        }

    }
}

void AVHRRSatellite::AddSegmentsToListFromUdp(QByteArray thefilepath)
{

    SegmentMetop *segmetop;
    SegmentNoaa *segnoaa;
    SegmentHRP *seghrp;
    SegmentGAC *seggac;
    SegmentVIIRSM *segviirsm;
    SegmentVIIRSDNB *segviirsdnb;


    QList<Segment*> *slmetop = seglmetop->GetSegmentlistptr();
    QList<Segment*> *slnoaa = seglnoaa->GetSegmentlistptr();
    QList<Segment*> *slhrp = seglhrp->GetSegmentlistptr();
    QList<Segment*> *slgac = seglgac->GetSegmentlistptr();
    QList<Segment*> *slviirsm = seglviirsm->GetSegmentlistptr();
    QList<Segment*> *slviirsdnb = seglviirsdnb->GetSegmentlistptr();

    thefilepath.replace( opts.dirremote.toLatin1(), opts.localdirremote.toLatin1()); // "/media/sdc1/", "/home/hugo/Vol2T/");
    qDebug() << "AddSegmentsToListFromUdp : " + QString(thefilepath);


    if (opts.segmentdirectorylist.count() > 0)
    {
        QStringList::Iterator its = opts.segmentdirectorylist.begin();
        QStringList::Iterator itc = opts.segmentdirectorylistinc.begin(); //segmentdirectory checked

        while( its != opts.segmentdirectorylist.end() )
        {
            if (*itc == "1")  //include checked
            {
                if (QString(thefilepath).contains(QString(*its)))
                {
                    QFileInfo fileinfo(thefilepath);
                    if ( fileinfo.fileName().mid( 0, 8) == "AVHR_xxx" && fileinfo.fileName().mid( 67, 4) == ".bz2")
                    {

                        qDebug() << "fileinfo filename  = " << fileinfo.fileName();
                        QFile file(thefilepath);
                        segmetop = new SegmentMetop(&file,satlist);
                        segmetop->segmentshow = true;
                        slmetop->append(segmetop);
                        emit signalAddedSegmentlist();

                    }
                    if (fileinfo.fileName().mid( 0, 6) == "avhrr_" && fileinfo.fileName().mid( 22, 6) == "noaa19")
                    {
                        qDebug() << "fileinfo filename  = " << fileinfo.fileName();
                        QFile file(thefilepath);
                        segnoaa = new SegmentNoaa(&file,satlist);
                        segnoaa->segmentshow = true;
                        slnoaa->append(segnoaa);
                        emit signalAddedSegmentlist();

                    }
                    if (fileinfo.fileName().mid( 0, 8) == "AVHR_HRP" && fileinfo.fileName().mid( 67, 4) == ".bz2")
                    {
                        qDebug() << "fileinfo filename  = " << fileinfo.fileName();
                        QFile file(thefilepath);
                        seghrp = new SegmentHRP(&file,satlist);
                        seghrp->segmentshow = true;
                        slhrp->append(seghrp);
                        emit signalAddedSegmentlist();

                    }
                    if (fileinfo.fileName().mid( 0, 8) == "AVHR_GAC")
                    {
                        qDebug() << "fileinfo filename  = " << fileinfo.fileName();
                        QFile file(thefilepath);
                        seggac = new SegmentGAC(&file,satlist);
                        seggac->segmentshow = true;
                        slgac->append(seggac);
                        emit signalAddedSegmentlist();

                    }
                    if (fileinfo.fileName().mid( 0, 8) == "SVMC_npp")
                    {
                        qDebug() << "fileinfo filename  = " << fileinfo.fileName();
                        QFile file(thefilepath);
                        segviirsm = new SegmentVIIRSM(&file,satlist);
                        segviirsm->segmentshow = true;
                        slviirsm->append(segviirsm);
                        emit signalAddedSegmentlist();

                    }
                    if (fileinfo.fileName().mid( 0, 10) == "SVDNBC_npp")
                    {
                        qDebug() << "fileinfo filename  = " << fileinfo.fileName();
                        QFile file(thefilepath);
                        segviirsdnb = new SegmentVIIRSDNB(&file,satlist);
                        segviirsdnb->segmentshow = true;
                        slviirsdnb->append(segviirsdnb);
                        emit signalAddedSegmentlist();

                    }
                    if (fileinfo.fileName().mid( 0, 9) == "H-000-MSG" && fileinfo.fileName().mid( 13, 3) == "MSG")
                    {
                        QFile file(thefilepath);

                        int filesequence = fileinfo.fileName().mid(36, 6).toInt()-1;
                        QString strspectrum = fileinfo.fileName().mid(26, 6);
                        QString strdate = fileinfo.fileName().mid(46, 12);

                        //Q_ASSERT( filesequence > 7);
                    }
                }
            }

            its++;
            itc++;
        }
    }

    seglmetop->SetTotalSegmentsInDirectory(slmetop->count());
    seglnoaa->SetTotalSegmentsInDirectory(slnoaa->count());
    seglhrp->SetTotalSegmentsInDirectory(slhrp->count());
    seglgac->SetTotalSegmentsInDirectory(slgac->count());
    seglviirsm->SetTotalSegmentsInDirectory(slviirsm->count());
    seglviirsdnb->SetTotalSegmentsInDirectory(slviirsdnb->count());

}

void AVHRRSatellite::RemoveAllSelectedAVHRR()
{
    int countsel = 0;

    qDebug() << "AVHRRSatellite::RemoveAllSelectedAVHRR()";

    QList<Segment*> *slmetop = seglmetop->GetSegmentlistptr();
    QList<Segment*> *slnoaa = seglnoaa->GetSegmentlistptr();
    QList<Segment*> *slhrp = seglhrp->GetSegmentlistptr();
    QList<Segment*> *slgac = seglgac->GetSegmentlistptr();

    countsel = 0;
    QList<Segment*>::iterator segitmetop = slmetop->begin();
    while ( segitmetop != slmetop->end() )
    {
        if((*segitmetop)->IsSelected())
        {
            countsel++;
            (*segitmetop)->ToggleSelected();
        }
        //(*segitmetop)->resetImageReady();
        (*segitmetop)->resetMemory();

        ++segitmetop;
    }

    qDebug() << QString("nbr of segments in metop = %1 countsel = %2").arg(slmetop->count()).arg(countsel);
    QList<Segment*> *slmetopsel = seglmetop->GetSegsSelectedptr();
    slmetopsel->clear();

    countsel = 0;
    QList<Segment*>::iterator segitnoaa = slnoaa->begin();
    while ( segitnoaa != slnoaa->end() )
    {
        if((*segitnoaa)->IsSelected())
        {
            countsel++;
            (*segitnoaa)->ToggleSelected();
        }
        //(*segitnoaa)->resetImageReady();
        (*segitnoaa)->resetMemory();

        ++segitnoaa;
    }

    qDebug() << QString("nbr of segments in noaa = %1 countsel = %2").arg(slnoaa->count()).arg(countsel);
    QList<Segment*> *slnoaasel = seglnoaa->GetSegsSelectedptr();
    slnoaasel->clear();

    countsel = 0;
    QList<Segment*>::iterator segithrp = slhrp->begin();
    while ( segithrp != slhrp->end() )
    {
        if((*segithrp)->IsSelected())
        {
            countsel++;
            (*segithrp)->ToggleSelected();
        }
        //(*segithrp)->resetImageReady();
        (*segithrp)->resetMemory();

        ++segithrp;
    }

    qDebug() << QString("nbr of segments in hrp = %1 countsel = %2").arg(slhrp->count()).arg(countsel);
    QList<Segment*> *slhrpsel = seglhrp->GetSegsSelectedptr();
    slhrpsel->clear();

    countsel = 0;
    QList<Segment*>::iterator segitgac = slgac->begin();
    while ( segitgac != slgac->end() )
    {
        if((*segitgac)->IsSelected())
        {
            countsel++;
            (*segitgac)->ToggleSelected();
        }
        //(*segitgac)->resetImageReady();
        (*segitgac)->resetMemory();

        ++segitgac;
    }

    qDebug() << QString("nbr of segments in gac = %1 countsel = %2").arg(slgac->count()).arg(countsel);
    QList<Segment*> *slgacsel = seglgac->GetSegsSelectedptr();
    slgacsel->clear();

}

void AVHRRSatellite::RemoveAllSelectedVIIRSM()
{
    qDebug() << "AVHRRSatellite::RemoveAllSelectedVIIRSM()";

    QList<Segment*> *slviirs = seglviirsm->GetSegmentlistptr();

    QList<Segment*>::iterator segitviirs = slviirs->begin();
    while ( segitviirs != slviirs->end() )
    {
        if((*segitviirs)->IsSelected())
            (*segitviirs)->ToggleSelected();
        (*segitviirs)->resetMemory();

        ++segitviirs;
    }

    QList<Segment*> *slviirssel = seglviirsm->GetSegsSelectedptr();
    slviirssel->clear();


}

void AVHRRSatellite::RemoveAllSelectedVIIRSDNB()
{
    qDebug() << "AVHRRSatellite::RemoveAllSelectedVIIRSDNB()";

    QList<Segment*> *slviirs = seglviirsdnb->GetSegmentlistptr();

    QList<Segment*>::iterator segitviirs = slviirs->begin();
    while ( segitviirs != slviirs->end() )
    {
        if((*segitviirs)->IsSelected())
            (*segitviirs)->ToggleSelected();
        (*segitviirs)->resetMemory();

        ++segitviirs;
    }

    QList<Segment*> *slviirssel = seglviirsdnb->GetSegsSelectedptr();
    slviirssel->clear();
}

void AVHRRSatellite::RemoveAllSelectedOLCIefr()
{
    qDebug() << "AVHRRSatellite::RemoveAllSelectedOLCIefr()";

    QList<Segment*> *slolciefr = seglolciefr->GetSegmentlistptr();

    QList<Segment*>::iterator segitolci = slolciefr->begin();
    while ( segitolci != slolciefr->end() )
    {
        if((*segitolci)->IsSelected())
            (*segitolci)->ToggleSelected();
        (*segitolci)->resetMemory();

        ++segitolci;
    }

    QList<Segment*> *slolcisel = seglolciefr->GetSegsSelectedptr();
    slolcisel->clear();
}

void AVHRRSatellite::RemoveAllSelectedOLCIerr()
{
    qDebug() << "AVHRRSatellite::RemoveAllSelectedOLCIerr()";

    QList<Segment*> *slolcierr = seglolcierr->GetSegmentlistptr();

    QList<Segment*>::iterator segitolci = slolcierr->begin();
    while ( segitolci != slolcierr->end() )
    {
        if((*segitolci)->IsSelected())
            (*segitolci)->ToggleSelected();
        (*segitolci)->resetMemory();

        ++segitolci;
    }

    QList<Segment*> *slolcisel = seglolcierr->GetSegsSelectedptr();
    slolcisel->clear();
}

bool AVHRRSatellite::SelectedAVHRRSegments()
{
    qDebug() << "AVHRRSatellite::SelectedAVHRRSegments()";

    bool selsegs = false;

    QList<Segment*> *slmetop = seglmetop->GetSegmentlistptr();
    QList<Segment*> *slnoaa = seglnoaa->GetSegmentlistptr();
    QList<Segment*> *slhrp = seglhrp->GetSegmentlistptr();
    QList<Segment*> *slgac = seglgac->GetSegmentlistptr();

    QList<Segment*>::iterator segitmetop = slmetop->begin();
    while ( segitmetop != slmetop->end() )
    {
        if((*segitmetop)->IsSelected())
            return true;
        ++segitmetop;
    }

    QList<Segment*>::iterator segitnoaa = slnoaa->begin();
    while ( segitnoaa != slnoaa->end() )
    {
        if((*segitnoaa)->IsSelected())
            return true;

        ++segitnoaa;
    }

    QList<Segment*>::iterator segithrp = slhrp->begin();
    while ( segithrp != slhrp->end() )
    {
        if((*segithrp)->IsSelected())
            return true;

        ++segithrp;
    }

    QList<Segment*>::iterator segitgac = slgac->begin();
    while ( segitgac != slgac->end() )
    {
        if((*segitgac)->IsSelected())
            return true;

        ++segitgac;
    }


    return false;

}

bool AVHRRSatellite::SelectedVIIRSMSegments()
{
    qDebug() << "AVHRRSatellite::SelectedVIIRSSegments()";

//    bool selsegs = false;

    QList<Segment*> *slviirs = seglviirsm->GetSegmentlistptr();
    if(seglviirsm->NbrOfSegmentsSelected() == 0)
        return false;
    else
        return true;


//    QList<Segment*>::iterator segitviirs = slviirs->begin();
//    while ( segitviirs != slviirs->end() )
//    {
//        if((*segitviirs)->IsSelected())
//            return true;

//        ++segitviirs;
//    }

//    return false;

}

bool AVHRRSatellite::SelectedVIIRSDNBSegments()
{
    qDebug() << "AVHRRSatellite::SelectedVIIRSDNBSegments()";

//    bool selsegs = false;

    QList<Segment*> *slviirs = seglviirsdnb->GetSegmentlistptr();

    if(seglviirsdnb->NbrOfSegmentsSelected() == 0)
        return false;
    else
        return true;

//    QList<Segment*>::iterator segitviirs = slviirs->begin();
//    while ( segitviirs != slviirs->end() )
//    {
//        if((*segitviirs)->IsSelected())
//            return true;

//        ++segitviirs;
//    }

//    return false;

}

bool AVHRRSatellite::SelectedOLCIefrSegments()
{
    qDebug() << "AVHRRSatellite::SelectedOLCIefrSegments()";

    bool selsegs = false;

    QList<Segment*> *slolci = seglolciefr->GetSegmentlistptr();

    if(seglolciefr->NbrOfSegmentsSelected() == 0)
        return false;
    else
        return true;

//    QList<Segment*>::iterator segitolci = slolci->begin();
//    while ( segitolci != slolci->end() )
//    {
//        if((*segitolci)->IsSelected())
//            return true;

//        ++segitolci;
//    }

//    return false;

}

bool AVHRRSatellite::SelectedOLCIerrSegments()
{
    qDebug() << "AVHRRSatellite::SelectedOLCIerrSegments()";

    bool selsegs = false;

    QList<Segment*> *slolci = seglolcierr->GetSegmentlistptr();

    if(seglolcierr->NbrOfSegmentsSelected() == 0)
        return false;
    else
        return true;

//    QList<Segment*>::iterator segitolci = slolci->begin();
//    while ( segitolci != slolci->end() )
//    {
//        if((*segitolci)->IsSelected())
//            return true;

//        ++segitolci;
//    }

//    return false;

}

QStringList AVHRRSatellite::GetOverviewSegmentsMetop()
{

    QStringList strlist;
    strlist << seglmetop->GetDirectoryName() << QString("Metop") << QString("%1").arg(this->countmetop);

    return strlist;

}

QStringList AVHRRSatellite::GetOverviewSegmentsNoaa()
{

    QStringList strlist;
    strlist << seglnoaa->GetDirectoryName() << QString("Noaa") << QString("%1").arg(countnoaa);

    return strlist;

}

QStringList AVHRRSatellite::GetOverviewSegmentsHRP()
{

    QStringList strlist;
    strlist << seglhrp->GetDirectoryName() << QString("HRP") << QString("%1").arg(counthrp);

    return strlist;

}

QStringList AVHRRSatellite::GetOverviewSegmentsGAC()
{


    QStringList strlist;
    strlist << seglgac->GetDirectoryName() << QString("GAC") <<  QString("%1").arg(countgac);

    return strlist;

}

QStringList AVHRRSatellite::GetOverviewSegmentsVIIRSM()
{

    QStringList strlist;
    strlist << seglviirsm->GetDirectoryName() << QString("VIIRSM") <<  QString("%1").arg(countviirsm);

    return strlist;

}

QStringList AVHRRSatellite::GetOverviewSegmentsVIIRSDNB()
{

    QStringList strlist;
    strlist << seglviirsdnb->GetDirectoryName() << QString("VIIRSDNB") <<  QString("%1").arg(countviirsdnb);

    return strlist;

}

QStringList AVHRRSatellite::GetOverviewSegmentsOLCIefr()
{

    QStringList strlist;
    strlist << seglolciefr->GetDirectoryName() << QString("OLCI EFR") <<  QString("%1").arg(countolciefr);

    return strlist;

}

QStringList AVHRRSatellite::GetOverviewSegmentsOLCIerr()
{

    QStringList strlist;
    strlist << seglolcierr->GetDirectoryName() << QString("OLCI ERR") <<  QString("%1").arg(countolcierr);

    return strlist;

}

QStringList AVHRRSatellite::GetOverviewSegmentsMeteosat()
{
    QStringList strlist;
    strlist << " " << QString("Meteosat-10") << QString("%1").arg(this->segmentlistmapmeteosat.count());

    return strlist;
}

QStringList AVHRRSatellite::GetOverviewSegmentsMeteosatRss()
{
    QStringList strlist;
    strlist << " " << QString("Meteosat-9") << QString("%1").arg(this->segmentlistmapmeteosatrss.count());

    return strlist;
}

QStringList AVHRRSatellite::GetOverviewSegmentsMeteosat7()
{
    QStringList strlist;
    strlist << " " << QString("Meteosat-7") << QString("%1").arg(this->segmentlistmapmet7.count());

    return strlist;
}

QStringList AVHRRSatellite::GetOverviewSegmentsMeteosat8()
{
    QStringList strlist;
    strlist << " " << QString("Meteosat-8") << QString("%1").arg(this->segmentlistmapmet8.count());

    return strlist;
}

QStringList AVHRRSatellite::GetOverviewSegmentsGOES13()
{
    QStringList strlist;
    strlist << " " << QString("GOES-13") << QString("%1").arg(this->segmentlistmapgoes13dc3.count() + this->segmentlistmapgoes13dc4.count());

    return strlist;
}

QStringList AVHRRSatellite::GetOverviewSegmentsGOES15()
{
    QStringList strlist;
    strlist << " " << QString("GOES-15") << QString("%1").arg(this->segmentlistmapgoes15dc3.count() + this->segmentlistmapgoes15dc4.count());

    return strlist;
}


QStringList AVHRRSatellite::GetOverviewSegmentsFY2E()
{
    QStringList strlist;
    strlist << " " << QString("FY2E") << QString("%1").arg(this->segmentlistmapfy2e.count());

    return strlist;
}

QStringList AVHRRSatellite::GetOverviewSegmentsFY2G()
{
    QStringList strlist;
    strlist << " " << QString("FY2G") << QString("%1").arg(this->segmentlistmapfy2g.count());

    return strlist;
}

QStringList AVHRRSatellite::GetOverviewSegmentsH8()
{
    QStringList strlist;
    strlist << " " << QString("H8") << QString("%1").arg(this->segmentlistmaph8.count());

    return strlist;
}

QString AVHRRSatellite::GetOverviewSegments()
{

    int nbrsegmmetop = 0, nbrsegmmetopsel = 0;
    int nbrsegmnoaa = 0, nbrsegmnoaasel = 0;
    int nbrsegmgac = 0, nbrsegmgacsel = 0;
    int nbrsegmhrp = 0, nbrsegmhrpsel = 0;
    int nbrsegmviirsm = 0, nbrsegmviirsmsel = 0;
    int nbrsegmviirsdnb = 0, nbrsegmviirsdnbsel = 0;
    int nbrsegmolciefr = 0, nbrsegmolciefrsel = 0;
    int nbrsegmolcierr = 0, nbrsegmolcierrsel = 0;

    QList<Segment*> *slmetop = seglmetop->GetSegmentlistptr();
    QList<Segment*> *slnoaa = seglnoaa->GetSegmentlistptr();
    QList<Segment*> *slhrp = seglhrp->GetSegmentlistptr();
    QList<Segment*> *slgac = seglgac->GetSegmentlistptr();
    QList<Segment*> *slviirsm = seglviirsm->GetSegmentlistptr();
    QList<Segment*> *slviirsdnb = seglviirsdnb->GetSegmentlistptr();
    QList<Segment*> *slolciefr = seglolciefr->GetSegmentlistptr();
    QList<Segment*> *slolcierr = seglolcierr->GetSegmentlistptr();

    QList<Segment*>::iterator segitmetop = slmetop->begin();
    while ( segitmetop != slmetop->end() )
    {
        if((*segitmetop)->IsSelected())
        {
            ++nbrsegmmetopsel;
        }
        ++nbrsegmmetop;
        ++segitmetop;
    }

    QList<Segment*>::iterator segitnoaa = slnoaa->begin();
    while ( segitnoaa != slnoaa->end() )
    {
        if((*segitnoaa)->IsSelected())
        {
            ++nbrsegmnoaasel;

        }
        ++nbrsegmnoaa;
        ++segitnoaa;
    }

    QList<Segment*>::iterator segithrp = slhrp->begin();
    while ( segithrp != slhrp->end() )
    {
        if((*segithrp)->IsSelected())
        {
            ++nbrsegmhrpsel;

        }
        ++nbrsegmhrp;
        ++segithrp;
    }


    QList<Segment*>::iterator segitgac = slgac->begin();
    while ( segitgac != slgac->end() )
    {
        if((*segitgac)->IsSelected())
        {
            ++nbrsegmgacsel;

        }
        ++nbrsegmgac;
        ++segitgac;
    }

    QList<Segment*>::iterator segitviirsm = slviirsm->begin();
    while ( segitviirsm != slviirsm->end() )
    {
        if((*segitviirsm)->IsSelected())
        {
            ++nbrsegmviirsmsel;

        }
        ++nbrsegmviirsm;
        ++segitviirsm;
    }

    QList<Segment*>::iterator segitviirsdnb = slviirsdnb->begin();
    while ( segitviirsdnb != slviirsdnb->end() )
    {
        if((*segitviirsdnb)->IsSelected())
        {
            ++nbrsegmviirsdnbsel;

        }
        ++nbrsegmviirsdnb;
        ++segitviirsdnb;
    }

    QList<Segment*>::iterator segitolciefr = slolciefr->begin();
    while ( segitolciefr != slolciefr->end() )
    {
        if((*segitolciefr)->IsSelected())
        {
            ++nbrsegmolciefrsel;

        }
        ++nbrsegmolciefr;
        ++segitolciefr;
    }

    QList<Segment*>::iterator segitolcierr = slolcierr->begin();
    while ( segitolcierr != slolcierr->end() )
    {
        if((*segitolcierr)->IsSelected())
        {
            ++nbrsegmolcierrsel;

        }
        ++nbrsegmolcierr;
        ++segitolcierr;
    }

    return QString("For %1 \n"
                   "\rSegments in directory = %2\n\rTotal Segments Metop = %3\n\rselected = %4 \n"
                   "For %5 \n"
                   "\rSegments in directory = %6\n\rTotal Segments Noaa = %7\n\rselected = %8 \n"
                   "For %9 \n"
                   "\rSegments in directory = %10\n\rTotal Segments GAC = %11\n\rselected = %12 \n"
                   "For %13 \n"
                   "\rSegments in directory = %14\n\rTotal Segments HRP = %15\n\rselected = %16 \n"
                   "For %17 \n"
                   "\rSegments in directory = %18\n\rTotal Segments VIIRSM = %19\n\rselected = %20 \n"
                   "For %21 \n"
                   "\rSegments in directory = %22\n\rTotal Segments VIIRSDNB = %23\n\rselected = %24 \n"
                   "For %25 \n"
                   "\rSegments in directory = %26\n\rTotal Segments OLCI EFR = %27\n\rselected = %28 \n"
                   "For %29 \n"
                   "\rSegments in directory = %30\n\rTotal Segments OLCI EFR = %31\n\rselected = %32 \n").
            arg(seglmetop->GetDirectoryName()).arg(seglmetop->GetTotalSegmentsInDirectory()).arg(nbrsegmmetop).arg(nbrsegmmetopsel).
            arg(seglnoaa->GetDirectoryName()).arg(seglnoaa->GetTotalSegmentsInDirectory()).arg(nbrsegmnoaa).arg(nbrsegmnoaasel).
            arg(seglgac->GetDirectoryName()).arg(seglgac->GetTotalSegmentsInDirectory()).arg(nbrsegmgac).arg(nbrsegmgacsel).
            arg(seglhrp->GetDirectoryName()).arg(seglhrp->GetTotalSegmentsInDirectory()).arg(nbrsegmhrp).arg(nbrsegmhrpsel).
            arg(seglviirsm->GetDirectoryName()).arg(seglviirsm->GetTotalSegmentsInDirectory()).arg(nbrsegmviirsm).arg(nbrsegmviirsmsel).
            arg(seglviirsdnb->GetDirectoryName()).arg(seglviirsdnb->GetTotalSegmentsInDirectory()).arg(nbrsegmviirsdnb).arg(nbrsegmviirsdnbsel).
            arg(seglolciefr->GetDirectoryName()).arg(seglolciefr->GetTotalSegmentsInDirectory()).arg(nbrsegmolciefr).arg(nbrsegmolciefrsel).
            arg(seglolcierr->GetDirectoryName()).arg(seglolcierr->GetTotalSegmentsInDirectory()).arg(nbrsegmolcierr).arg(nbrsegmolcierrsel);

}

void AVHRRSatellite::drawOverlay(char *pFileName )
{

    double w, e, s, n, area, f_area, lon, lat;
    char source, kind[2] = {'P', 'L'}, c = '>';
    FILE *fp = 0;
    int line, max_east = 270000000, info, single, error, ID, flip;
    int  level, version, greenwich, river, src, msformat = 0, first = 1;
    size_t n_read;
    struct POINT_GSHHS p;
    struct GSHHS h;
    Vxp *vxp = new Vxp;

    int nFeatures = 0;
    long totnbrofpoints = 0;
    nbrofpointsselected = 0;

    info = single = error = ID = 0;


    if ((fp = fopen (pFileName, "rb")) == 0 ) {
            qDebug() << QString( "gshhs:  Could not find file %s.").arg(pFileName);
            return; //exit (EXIT_FAILURE);
    }

    n_read = fread ((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
    version = (h.flag >> 8) & 255;
    flip = (version != GSHHS_DATA_RELEASE);	// Take as sign that byte-swabbing is needed

    vxp->nFeatures = 1;
    vxp->pFeatures = new VxpFeature[1];

    while (n_read == 1)
    {
            if (flip)
            {
                    h.id = swabi4 ((unsigned int)h.id);
                    h.n  = swabi4 ((unsigned int)h.n);
                    h.west  = swabi4 ((unsigned int)h.west);
                    h.east  = swabi4 ((unsigned int)h.east);
                    h.south = swabi4 ((unsigned int)h.south);
                    h.north = swabi4 ((unsigned int)h.north);
                    h.area  = swabi4 ((unsigned int)h.area);
                    h.area_full  = swabi4 ((unsigned int)h.area_full);
                    h.flag  = swabi4 ((unsigned int)h.flag);
                    h.container  = swabi4 ((unsigned int)h.container);
                    h.ancestor  = swabi4 ((unsigned int)h.ancestor);
            }
            level = h.flag & 255;				// Level is 1-4
            version = (h.flag >> 8) & 255;			// Version is 1-7
            //if (first) fprintf (stderr, "gshhs %s - Found GSHHS version %d in file %s\n", GSHHS_PROG_VERSION, version, file);
            greenwich = (h.flag >> 16) & 1;			// Greenwich is 0 or 1
            src = (h.flag >> 24) & 1;			// Greenwich is 0 (WDBII) or 1 (WVS)
            river = (h.flag >> 25) & 1;			// River is 0 (not river) or 1 (is river)
            w = h.west  * GSHHS_SCL;			// Convert from microdegrees to degrees
            e = h.east  * GSHHS_SCL;
            s = h.south * GSHHS_SCL;
            n = h.north * GSHHS_SCL;
            source = (src == 1) ? 'W' : 'C';		// Either WVS or CIA (WDBII) pedigree
            if (river) source = tolower ((int)source);	// Lower case c means river-lake
            line = (h.area) ? 0 : 1;			// Either Polygon (0) or Line (1) (if no area)
            area = 0.1 * h.area;				// Now im km^2
            f_area = 0.1 * h.area_full;			// Now im km^2

            //OK = (!single || h.id == ID);
            first = 0;

            if (!msformat) c = kind[line];

            vxp->pFeatures[0].nVerts = h.n;
            vxp->pFeatures[0].pVerts = new QVector3D[ h.n ];

            for (int k = 0; k < h.n; k++)
            {
                if (fread ((void *)&p, (size_t)sizeof(struct POINT_GSHHS), (size_t)1, fp) != 1)
                {
                    //fprintf (stderr, "gshhs:  Error reading file %s for %s %d, point %d.\n", argv[1], name[line], h.id, k);
                    exit (EXIT_FAILURE);
                }
                if (flip)
                {
                    p.x = swabi4 ((unsigned int)p.x);
                    p.y = swabi4 ((unsigned int)p.y);
                }
                lon = p.x * GSHHS_SCL;
                if ((greenwich && p.x > max_east) || (h.west > 180000000)) lon -= 360.0;
                lat = p.y * GSHHS_SCL;
                // LonLat2Point(lat, lon, &vxp->pFeatures[nFeatures].pVerts[k], 1.0f);

                //if (lon > 0. && lon < 30.0 && lat > 30.0 && lat < 60.0)
                //{
                    totnbrofpoints++;
                    //drawPoint(lon * PI / 180, lat * PI/ 180);
                //}

            }

            max_east = 180000000;	// Only Eurasia needs 270
            n_read = fread((void *)&h, (size_t)sizeof (struct GSHHS), (size_t)1, fp);
            nFeatures++;
    }

    fclose (fp);


   qDebug() << QString("total nbr of points = %1 total selected = %2").arg(totnbrofpoints).arg(nbrofpointsselected);
}

SegmentListGeostationary *AVHRRSatellite::getActiveSegmentList()
{
    SegmentListGeostationary *sl = NULL;
    QString activelist;

    if(seglmeteosat->bActiveSegmentList == true)
    {
        activelist = "Meteosat-10";
        sl = seglmeteosat;
    }
    else if(seglmeteosatrss->bActiveSegmentList == true)
    {
        activelist = "Meteosat-9";
        sl = seglmeteosatrss;
    }
    else if(seglmet7->bActiveSegmentList == true)
    {
        activelist = "Meteosat-8";
        sl = seglmet8;
    }
    else if(seglmet7->bActiveSegmentList == true)
    {
        activelist = "Meteosat-7";
        sl = seglmet7;
    }
    else if(seglgoes13dc3->bActiveSegmentList == true)
    {
        activelist = "GOES-13(dc3)";
        sl = seglgoes13dc3;
    }
    else if(seglgoes15dc3->bActiveSegmentList == true)
    {
        activelist = "GOES-15(dc3)";
        sl = seglgoes15dc3;
    }
    else if(seglgoes13dc4->bActiveSegmentList == true)
    {
        activelist = "GOES-13(dc4)";
        sl = seglgoes13dc4;
    }
    else if(seglgoes15dc4->bActiveSegmentList == true)
    {
        activelist = "GOES-15(dc4)";
        sl = seglgoes15dc4;
    }
    else if(seglfy2e->bActiveSegmentList == true)
    {
        activelist = "FY2E";
        sl = seglfy2e;
    }
    else if(seglfy2g->bActiveSegmentList == true)
    {
        activelist = "FY2G";
        sl = seglfy2g;
    }
    else if(seglh8->bActiveSegmentList == true)
    {
        activelist = "H8";
        sl = seglh8;
    }
    else
        return NULL;

    qDebug() << "Activesegmentlist = " << activelist;
    return sl;


}


/*
AVHRRSatellite::~AVHRRSatellite()
{

    qDebug() << QString("in destructor AVHRRSatellite");

    delete segmentlistmetop;


    delete segmentlistnoaa;


    delete segmentlistgac;


    delete segmentlisthrp;


}
*/
