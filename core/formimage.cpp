#include "formimage.h"
#include "segmentimage.h"
#include "options.h"
#include "gshhsdata.h"
#include "pixgeoconversion.h"

#include <qtconcurrentrun.h>

extern Options opts;
extern SegmentImage *imageptrs;
extern gshhsData *gshhsdata;

#include <QMutex>
#include <QDebug>

#define MAP_X 3
#define MAP_Y 3

extern QMutex g_mutex;

void doCLAHE(quint16 *t)
{
    imageptrs->CLAHE(t, 3712, 3712, 0, 1023, 16, 16, 256, opts.clahecliplimit);
}

void doCLAHEHRVFull(quint16 *t)
{
    imageptrs->CLAHE(t, 5568, 11136, 0, 1023, 16, 16, 256, opts.clahecliplimit);
}

void doCLAHEHRVEurope(quint16 *t)
{
    imageptrs->CLAHE(t, 5568, 5*464, 0, 1023, 16, 16, 256, opts.clahecliplimit);
}

FormImage::FormImage(QWidget *parent, SatelliteList *satlist, AVHRRSatellite *seglist ) :
    QWidget(parent)
{
    sats = satlist;
    segs = seglist;
    channelshown = IMAGE_GEOSTATIONARY;
    zoomIncrement = 2;
    maxZoomValue = 300;
    minZoomValue = 4;

    zoomValueavhrr = opts.zoomfactoravhrr;
    zoomValuemeteosat = opts.zoomfactormeteosat;
    zoomValueprojection = opts.zoomfactorprojection;
    zoomValueviirs = opts.zoomfactorviirs;

    scaleFactor = (double)getZoomValue()/100;
    qDebug() << QString("FormImage::FormImage scalefactor = %1").arg(scaleFactor);
    imageLabel = new MyImageLabel;

    imageLabel->setScaledContents(true);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(imageLabel);
    this->setLayout(mainLayout);

    //imageLabel->setPixmap(QPixmap::fromImage(*(imageptrs->ptrimageMeteosat)));
    //this->adjustImage();
    overlaymeteosat = true;
    overlayprojection = true;
    refreshoverlay = true;
    changeinfraprojection = false;

    this->setSegmentType(SEG_NONE);

    metopcount = 0;
    noaacount = 0;
    gaccount = 0;
    hrpcount = 0;
    viirsmcount = 0;
    viirsdnbcount = 0;
    olciefrcount = 0;
    olcierrcount = 0;
    txtInfo = "";


}

QLabel *FormImage::returnimageLabelptr()
{
    return imageLabel;
}

bool FormImage::toggleOverlayMeteosat()
{
    displayImage(channelshown);
    if(overlaymeteosat)
        overlaymeteosat = false;
    else
        overlaymeteosat = true;
    this->update();
    return overlaymeteosat;
}

bool FormImage::toggleOverlayProjection()
{
    displayImage(channelshown);
    if(overlayprojection)
        overlayprojection = false;
    else
        overlayprojection = true;
    this->update();
    return overlayprojection;
}

void FormImage::setPixmapToLabel(bool settoolboxbuttons)
{

    refreshoverlay = true;

    emit allsegmentsreceivedbuttons(settoolboxbuttons);

    switch(channelshown)
    {
    case IMAGE_AVHRR_CH1:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[0]) ));
        break;
    case IMAGE_AVHRR_CH2:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[1]) ));
        break;
    case IMAGE_AVHRR_CH3:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[2]) ));
        break;
    case IMAGE_AVHRR_CH4:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[3]) ));
        break;
    case IMAGE_AVHRR_CH5:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[4]) ));
        break;
    case IMAGE_AVHRR_COL:
        imageLabel->setPixmap(QPixmap::fromImage(*(imageptrs->ptrimagecomp_col)));
        break;
    case IMAGE_AVHRR_EXPAND:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrexpand_col)));
        break;
    case IMAGE_GEOSTATIONARY:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageGeostationary)));
        break;
    case IMAGE_PROJECTION:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageProjection)));
        break;
    case IMAGE_VIIRS_M:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageViirsM)));
        break;
    case IMAGE_VIIRS_DNB:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageViirsDNB)));
        break;
    case IMAGE_OLCI:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageOLCI)));
        break;

    }

    this->adjustImage();
    QApplication::processEvents();

    qDebug() << QString("FormImage::setPixmapToLabel() channelshown = %1").arg(this->channelshown);

}

void FormImage::setPixmapToLabelDNB(bool settoolboxbuttons)
{
    refreshoverlay = true;
    imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageViirsDNB)));
    this->update();

}

void FormImage::displayImage(eImageType channel)
{
    qDebug() << QString("FormImage::displayImage(eImageType channel) channel = %1").arg(channel);
    qDebug() << QString("FormImage ptrimagecomp[0] bytecount = %1").arg(imageptrs->ptrimagecomp_ch[0]->byteCount());
    qDebug() << QString("FormImage ptrimageviirsm bytecount = %1").arg(imageptrs->ptrimageViirsM->byteCount());
    qDebug() << QString("FormImage ptrimageviirsdnb bytecount = %1").arg(imageptrs->ptrimageViirsDNB->byteCount());
    qDebug() << QString("FormImage ptrimageolci efr bytecount = %1").arg(imageptrs->ptrimageOLCI->byteCount());

    this->channelshown = channel;

//    switch(channelshown)
//    {
//    case IMAGE_AVHRR_CH1:
//    case IMAGE_AVHRR_CH2:
//    case IMAGE_AVHRR_CH3:
//    case IMAGE_AVHRR_CH4:
//    case IMAGE_AVHRR_CH5:
//    case IMAGE_AVHRR_COL:
//         if(imageptrs->ptrimagecomp_ch[0]->byteCount() == 0)
//         {
//             return;
//         }
//        break;
//    case IMAGE_VIIRS_M:
//         if(imageptrs->ptrimageViirsM->byteCount() == 0)
//         {
//             return;
//         }
//        break;
//    case IMAGE_VIIRS_DNB:
//         if(imageptrs->ptrimageViirsDNB->byteCount() == 0)
//         {
//             return;
//         }
//        break;
//    case IMAGE_OLCI_EFR:
//         if(imageptrs->ptrimageOLCIefr->byteCount() == 0)
//         {
//             return;
//         }
//        break;
//    case IMAGE_OLCI_ERR:
//         if(imageptrs->ptrimageOLCIerr->byteCount() == 0)
//         {
//             return;
//         }
//        break;
//    }

    g_mutex.lock();

    switch(channelshown)
    {
    case IMAGE_AVHRR_CH1:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[0]) ));
        break;
    case IMAGE_AVHRR_CH2:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[1]) ));
        break;
    case IMAGE_AVHRR_CH3:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[2]) ));
        break;
    case IMAGE_AVHRR_CH4:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[3]) ));
        break;
    case IMAGE_AVHRR_CH5:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[4]) ));
        break;
    case IMAGE_AVHRR_COL:
        imageLabel->setPixmap(QPixmap::fromImage(*(imageptrs->ptrimagecomp_col)));
        break;
    case IMAGE_AVHRR_EXPAND:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrexpand_col)));
        break;
    case IMAGE_GEOSTATIONARY:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageGeostationary)));
        break;
    case IMAGE_PROJECTION:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageProjection)));
        break;
    case IMAGE_VIIRS_M:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageViirsM)));
        break;
    case IMAGE_VIIRS_DNB:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageViirsDNB)));
        break;
    case IMAGE_OLCI:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageOLCI)));
        break;

    }

    this->update();
    this->adjustImage();
    g_mutex.unlock();

    refreshoverlay = true;

    qDebug() << QString("after FormImage::displayImage(eImageType channel) channel = %1").arg(channel);


}

void FormImage::slotMakeImage()
{
    this->ComposeImage();
}

void FormImage::ComposeImage()
{

    if(opts.buttonMetop || opts.buttonNoaa || opts.buttonHRP || opts.buttonGAC)
    {
        metopcount = segs->seglmetop->NbrOfSegmentsSelected();
        noaacount = segs->seglnoaa->NbrOfSegmentsSelected();
        hrpcount = segs->seglhrp->NbrOfSegmentsSelected();
        gaccount = segs->seglgac->NbrOfSegmentsSelected();
    }
    else if(opts.buttonVIIRSM)
    {
        viirsmcount = segs->seglviirsm->NbrOfSegmentsSelected();
    }
    else if(opts.buttonVIIRSDNB)
    {
        viirsdnbcount = segs->seglviirsdnb->NbrOfSegmentsSelected();
    }
    else if(opts.buttonOLCIefr)
    {
        olciefrcount = segs->seglolciefr->NbrOfSegmentsSelected();
    }
    else if(opts.buttonOLCIerr)
    {
        olcierrcount = segs->seglolcierr->NbrOfSegmentsSelected();
    }
    else
        return;

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    qDebug() << QString("in FormImage::ComposeImage nbr of metop segments selected = %1").arg(metopcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of noaa segments selected = %1").arg(noaacount);
    qDebug() << QString("in FormImage::ComposeImage nbr of hrp segments selected = %1").arg(hrpcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of gac segments selected = %1").arg(gaccount);
    qDebug() << QString("in FormImage::ComposeImage nbr of viirsm segments selected = %1").arg(viirsmcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of viirsdnb segments selected = %1").arg(viirsdnbcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of olciefr segments selected = %1").arg(olciefrcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of olcierr segments selected = %1").arg(olcierrcount);

    if(opts.buttonMetop || opts.buttonNoaa || opts.buttonGAC || opts.buttonHRP)
    {
        if (metopcount + noaacount + hrpcount + gaccount > 0)
        {
            displayImage(IMAGE_AVHRR_COL);

            if (metopcount > 0 && opts.buttonMetop)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_METOP);
                segs->seglmetop->ComposeImage(opts.buttonEqualization ? metop_gammaEqual_ch : metop_gamma_ch);
            }
            else if (noaacount > 0 && opts.buttonNoaa)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_NOAA);
                segs->seglnoaa->ComposeImage(opts.buttonEqualization ? noaa_gammaEqual_ch : noaa_gamma_ch);
            }
            else if (hrpcount > 0 && opts.buttonHRP)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_HRP);
                segs->seglhrp->ComposeImage(opts.buttonEqualization ? hrp_gammaEqual_ch : hrp_gamma_ch);
            }
            else if (gaccount > 0 && opts.buttonGAC)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_GAC);
                segs->seglgac->ComposeImage(opts.buttonEqualization ? gac_gammaEqual_ch : gac_gamma_ch);
            }
        }
        else
            return;
    }
    else if(viirsmcount > 0 && opts.buttonVIIRSM)
    {
            formtoolbox->setToolboxButtons(false);

            this->displayImage(IMAGE_VIIRS_M);
            this->kindofimage = "VIIRSM";
            this->setSegmentType(SEG_VIIRSM);
            bandlist = formtoolbox->getVIIRSMBandList();
            colorlist = formtoolbox->getVIIRSMColorList();
            invertlist = formtoolbox->getVIIRSMInvertList();
//          in Workerthread
            segs->seglviirsm->ComposeVIIRSImage(bandlist, colorlist, invertlist);
//          in main thread
//            segs->seglviirsm->ComposeVIIRSImageSerial(bandlist, colorlist, invertlist);
    }
    else if(viirsdnbcount > 0 && opts.buttonVIIRSDNB)
    {
            formtoolbox->setToolboxButtons(false);
            segs->seglviirsdnb->graphvalues.reset(new long[150 * 180]);
            for(int i = 0; i < 150 * 180; i++)
                segs->seglviirsdnb->graphvalues[i] = 0;


            this->displayImage(IMAGE_VIIRS_DNB);
            this->kindofimage = "VIIRSDNB";
            this->setSegmentType(SEG_VIIRSDNB);

            bandlist = formtoolbox->getVIIRSMBandList();
            colorlist = formtoolbox->getVIIRSMColorList();
            invertlist = formtoolbox->getVIIRSMInvertList();
            //          in Workerthread
            segs->seglviirsdnb->ComposeVIIRSImage(bandlist, colorlist, invertlist);
    }
    else if(olciefrcount > 0 && opts.buttonOLCIefr)
    {
            formtoolbox->setToolboxButtons(false);

            this->displayImage(IMAGE_OLCI);
            this->kindofimage = "OLCIEFR";
            this->setSegmentType(SEG_OLCIEFR);

            bandlist = formtoolbox->getOLCIBandList();
            colorlist = formtoolbox->getOLCIColorList();
            invertlist = formtoolbox->getOLCIInvertList();


            //          in Workerthread
            segs->seglolciefr->ComposeOLCIImage(bandlist, colorlist, invertlist, true);
    }
    else if(olcierrcount > 0 && opts.buttonOLCIerr)
    {
            formtoolbox->setToolboxButtons(false);

            this->displayImage(IMAGE_OLCI);
            this->kindofimage = "OLCIERR";
            this->setSegmentType(SEG_OLCIERR);

            bandlist = formtoolbox->getOLCIBandList();
            colorlist = formtoolbox->getOLCIColorList();
            invertlist = formtoolbox->getOLCIInvertList();


            //          in Workerthread
            segs->seglolcierr->ComposeOLCIImage(bandlist, colorlist, invertlist, true);
    }
    else
        return;
}

void FormImage::slotShowVIIRSMImage()
{
    this->ShowVIIRSMImage();
}

bool FormImage::ShowVIIRSMImage()
{
    bool ret = false;

    viirsmcount = segs->seglviirsm->NbrOfSegmentsSelected();

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    qDebug() << QString("in FormImage::ShowVIIRSImage nbr of viirs segments selected = %1").arg(viirsmcount);

    if (viirsmcount > 0)
    {

        ret = true;
        displayImage(IMAGE_VIIRS_M);

        emit allsegmentsreceivedbuttons(false);

        this->kindofimage = "VIIRSM";

        bandlist = formtoolbox->getVIIRSMBandList();
        colorlist = formtoolbox->getVIIRSMColorList();
        invertlist = formtoolbox->getVIIRSMInvertList();

        segs->seglviirsm->ShowImageSerial(bandlist, colorlist, invertlist);
    }
    else
        ret = false;

    return ret;

}

void FormImage::slotShowOLCIefrImage()
{
    this->ShowOLCIefrImage();
}

bool FormImage::ShowOLCIefrImage()
{
    bool ret = false;

    olciefrcount = segs->seglolciefr->NbrOfSegmentsSelected();

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    qDebug() << QString("in FormImage::ShowOLCIefrImage nbr of olci efr segments selected = %1").arg(olciefrcount);

    if (olciefrcount > 0)
    {

        ret = true;
        displayImage(IMAGE_OLCI);

        emit allsegmentsreceivedbuttons(false);

        this->kindofimage = "OLCIEFR";

        bandlist = formtoolbox->getOLCIBandList();
        colorlist = formtoolbox->getOLCIColorList();
        invertlist = formtoolbox->getOLCIInvertList();

        segs->seglolciefr->ComposeOLCIImage(bandlist, colorlist, invertlist, false);
    }
    else
        ret = false;

    return ret;

}

void FormImage::slotShowOLCIerrImage()
{
    this->ShowOLCIerrImage();
}

bool FormImage::ShowOLCIerrImage()
{
    bool ret = false;

    olcierrcount = segs->seglolcierr->NbrOfSegmentsSelected();

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    qDebug() << QString("in FormImage::ShowOLCIerrImage nbr of olci err segments selected = %1").arg(olcierrcount);

    if (olcierrcount > 0)
    {

        ret = true;
        displayImage(IMAGE_OLCI);

        emit allsegmentsreceivedbuttons(false);

        this->kindofimage = "OLCIERR";

        bandlist = formtoolbox->getOLCIBandList();
        colorlist = formtoolbox->getOLCIColorList();
        invertlist = formtoolbox->getOLCIInvertList();

        segs->seglolcierr->ComposeOLCIImage(bandlist, colorlist, invertlist, false);
    }
    else
        ret = false;

    return ret;

}

bool FormImage::ShowVIIRSDNBImage()
{
    bool ret = false;

    viirsdnbcount = segs->seglviirsdnb->NbrOfSegmentsSelected();

    qDebug() << QString("in FormImage::ShowVIIRSDNBImage nbr of viirs segments selected = %1").arg(viirsdnbcount);
    if (viirsdnbcount > 0)
    {
        ret = true;
        displayImage(IMAGE_VIIRS_DNB);

        emit allsegmentsreceivedbuttons(false);

        this->kindofimage = "VIIRSDNB";

        segs->seglviirsdnb->ShowImageSerial();
    }
    else
        ret = false;

    return ret;

}

void FormImage::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        mousepoint = e->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void FormImage::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() == Qt::LeftButton)
    {
        emit moveImage(e->pos(), mousepoint);
    }
    //qDebug() << QString("mousemoveevent pos.x = %1 pos.y = %2").arg(e->pos().x()).arg(e->pos().y());
}

void FormImage::mouseReleaseEvent(QMouseEvent *)
{
    setCursor(Qt::ArrowCursor);
}

void FormImage::wheelEvent(QWheelEvent *event)
{

    //QTimer::singleShot(5100, this, SLOT(zoomOverlaySwitch()));
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    if(numSteps > 0)
        formwheelZoom(1);
    if(numSteps < 0)
        formwheelZoom(-1);

}

void FormImage::formwheelZoom(int d)
{
    if(d > 0){
        this->makeZoom(getZoomValue() + zoomIncrement);
    }

    if(d < 0){
        this->makeZoom(getZoomValue() - zoomIncrement);
    }

}

void FormImage::makeZoom(double f)
{
    if(f <= maxZoomValue && f >= minZoomValue)
    {
        setZoomValue(f);
        scaleFactor = f/100;
        this->adjustImage();

        QString windowTitleFormat = QString("EUMETCastView zoomLevel");
        windowTitleFormat.replace("zoomLevel", QString("%1%").arg((int)(f)));
        this->topLevelWidget()->setWindowTitle(windowTitleFormat);
    }
}

void FormImage::setZoomValue(int z)
{
    switch(channelshown)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        zoomValueavhrr = z;
        opts.zoomfactoravhrr = z;
        break;
    case 8:
        zoomValuemeteosat = z;
        opts.zoomfactormeteosat = z;
        break;
    case 9:
        zoomValueprojection = z;
        opts.zoomfactorprojection = z;
        break;
    case 10:
    case 11:
        zoomValueviirs = z;
        opts.zoomfactorviirs = z;
        break;
    case 12:
        zoomValueprojection = z;
        opts.zoomfactorprojection = z;
        break;

    }
}

int FormImage::getZoomValue()
{
    int zoomValue;
    switch(channelshown)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        zoomValue = zoomValueavhrr;
        break;
    case 8:
        zoomValue = zoomValuemeteosat;
        break;
    case 9:
        zoomValue = zoomValueprojection;
        break;
    case 10:
    case 11:
        zoomValue = zoomValueviirs;
        break;
    case 12:
        zoomValue = zoomValueprojection;
        break;

    }
    return zoomValue;
}

/*
void FormImage::zoomOverlaySwitch()
{
    overlayzoommove = true;
    this->update();
}
*/

void FormImage::paintEvent( QPaintEvent * )
{
    if (imageLabel->pixmap() == 0)
        return;

    QPixmap *pix;
    pix=(QPixmap *)imageLabel->pixmap();

    if(pix->height() == 0)
        return;

    //pix=imageLabel->pixmap();
    //pix=QPixmap::fromImage(*(imageptrs->ptrimageMeteosat));
    QPainter painter(pix);

    if(channelshown >= 1 && channelshown <= 8)
    {
        QFont f("Courier", 40, QFont::Bold);
        painter.setFont(f);
        painter.setPen(Qt::yellow);
        painter.drawText(10, 50, kindofimage);
    }


    SegmentListGeostationary *sl = NULL;

    if(channelshown >= 1 && channelshown <= 6)
        displayAVHRRImageInfo();
    if(channelshown == IMAGE_VIIRS_M || channelshown == IMAGE_VIIRS_DNB)
        displayVIIRSImageInfo();
    if(channelshown == IMAGE_OLCI)
        displayOLCIImageInfo();

    if(segs->seglmeteosat->bActiveSegmentList == true)
    {
        sl = segs->seglmeteosat;
    }
    else if(segs->seglmeteosatrss->bActiveSegmentList == true)
    {
        sl = segs->seglmeteosatrss;
    }
    else if(segs->seglmet8->bActiveSegmentList == true)
    {
        sl = segs->seglmet8;
    }
    else if(segs->seglmet7->bActiveSegmentList == true)
    {
        sl = segs->seglmet7;
    }
    else if(segs->seglgoes13dc3->bActiveSegmentList == true)
    {
        sl = segs->seglgoes13dc3;
    }
    else if(segs->seglgoes15dc3->bActiveSegmentList == true)
    {
        sl = segs->seglgoes15dc3;
    }
    else if(segs->seglgoes13dc4->bActiveSegmentList == true)
    {
        sl = segs->seglgoes13dc4;
    }
    else if(segs->seglgoes15dc4->bActiveSegmentList == true)
    {
        sl = segs->seglgoes15dc4;
    }
    else if(segs->seglfy2e->bActiveSegmentList == true)
    {
        sl = segs->seglfy2e;
    }
    else if(segs->seglfy2g->bActiveSegmentList == true)
    {
        sl = segs->seglfy2g;
    }
    else if(segs->seglh8->bActiveSegmentList == true)
    {
        sl = segs->seglh8;
    }
    else
        return;


    if(channelshown == IMAGE_GEOSTATIONARY)
        displayGeoImageInfo();


    if (channelshown == IMAGE_GEOSTATIONARY && overlaymeteosat && refreshoverlay)
    {
        this->OverlayGeostationary(&painter, sl);
        refreshoverlay = false;
    }
    if(channelshown == IMAGE_PROJECTION && overlayprojection && refreshoverlay)
    {
        this->OverlayProjection(&painter, sl);
        refreshoverlay = false;
    }

    if(channelshown == IMAGE_PROJECTION && changeinfraprojection)
    {
        changeinfraprojection = false;
    }

    this->adjustImage();

}

void FormImage::displayAVHRRImageInfo()
{
    QString segtype;
    eSegmentType type;
    int nbrselected;

    type = getSegmentType();
    switch(type)
    {
    case SEG_NONE:
        segtype = "None";
        break;
    case SEG_METOP:
        segtype = "Metop";
        break;
    case SEG_NOAA:
        segtype = "Noaa";
        break;
    case SEG_HRP:
        segtype = "HRP";
        break;
    case SEG_GAC:
        segtype = "GAC";
        break;
    default:
        segtype = "NA";
        break;
    }

    if (type == SEG_NOAA)
        nbrselected = segs->seglnoaa->NbrOfSegmentsSelected();
    else if( type == SEG_METOP)
        nbrselected = segs->seglmetop->NbrOfSegmentsSelected();
    else if( type == SEG_GAC)
        nbrselected = segs->seglgac->NbrOfSegmentsSelected();
    else if( type == SEG_HRP)
        nbrselected = segs->seglhrp->NbrOfSegmentsSelected();

    txtInfo = QString("<!DOCTYPE html>"
                      "<html><head><title>Info</title></head>"
                      "<body>"
                      "<h2 style='color:blue'>Image Information</h1>"
                      "<h3>Segment type = %1</h3>"
                      "<h3>Nbr of segments = %2</h3>"
                      "<h3>Image width = %3 height = %4</h3>"
                      "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimagecomp_col->width()).arg(imageptrs->ptrimagecomp_col->height());


    formtoolbox->writeInfoToAVHRR(txtInfo);

}

void FormImage::displayVIIRSImageInfo()
{
    QString segtype;
    eSegmentType type;
    int nbrselected;
    float moonillum;

    type = getSegmentType();
    switch(type)
    {
    case SEG_NONE:
        segtype = "None";
        break;
    case SEG_VIIRSM:
        segtype = "VIIRSM";
        nbrselected = segs->seglviirsm->NbrOfSegmentsSelected();

        break;
    case SEG_VIIRSDNB:
        segtype = "VIIRSDNB";
        nbrselected = segs->seglviirsdnb->NbrOfSegmentsSelected();
        moonillum = segs->seglviirsdnb->getMoonIllumination();

        break;
    default:
        segtype = "NA";
        break;
    }


    if(type == SEG_VIIRSM)
    {
        txtInfo = QString("<!DOCTYPE html>"
                          "<html><head><title>Info</title></head>"
                          "<body>"
                          "<h3 style='color:blue'>Image Information</h3>"
                          "<p>Segment type = %1<br>"
                          "Nbr of segments = %2<br>"
                          "Image width = %3 height = %4<br>"
                          "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimageViirsM->width()).arg(imageptrs->ptrimageViirsM->height());
        formtoolbox->writeInfoToVIIRSM(txtInfo);

    }
    if(type == SEG_VIIRSDNB)
    {
        txtInfo = QString("<!DOCTYPE html>"
                          "<html><head><title>Info</title></head>"
                          "<body>"
                          "<h3 style='color:blue'>Image Information</h3>"
                          "<p>Segment type = %1<br>"
                          "Nbr of segments = %2<br>"
                          "Image width = %3 height = %4<br>"
                          "Moon illumination = %5 %</p>"
                          "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimageViirsDNB->width())
                .arg(imageptrs->ptrimageViirsDNB->height()).arg(moonillum, 4, 'f', 2);
        formtoolbox->writeInfoToVIIRSDNB(txtInfo);

    }


}

void FormImage::displayGeoImageInfo()
{

    if(segs->seglmeteosat->bActiveSegmentList == true)
    {
        displayGeoImageInformation("Meteosat 10");
    } else if(segs->seglmeteosatrss->bActiveSegmentList == true)
    {
        displayGeoImageInformation("Meteosat 9");
    } else if(segs->seglmet8->bActiveSegmentList == true)
    {
        displayGeoImageInformation("Meteosat 8");
    } else if(segs->seglmet7->bActiveSegmentList == true)
    {
        displayGeoImageInformation("Meteosat 7");
    } else if(segs->seglgoes13dc3->bActiveSegmentList == true)
    {
        displayGeoImageInformation("GOES 13");
    } else if(segs->seglgoes13dc4->bActiveSegmentList == true)
    {
        displayGeoImageInformation("GOES 13");
    } else if(segs->seglgoes15dc3->bActiveSegmentList == true)
    {
        displayGeoImageInformation("GOES 15");
    } else if(segs->seglgoes15dc4->bActiveSegmentList == true)
    {
        displayGeoImageInformation("GOES 15");
    } else if(segs->seglfy2e->bActiveSegmentList == true)
    {
        displayGeoImageInformation("Feng Yun 2E");
    } else if(segs->seglfy2g->bActiveSegmentList == true)
    {
        displayGeoImageInformation("Feng Yun 2G");
    } else if(segs->seglh8->bActiveSegmentList == true)
    {
        displayGeoImageInformation("Himawari-8");
    }


}

void FormImage::displayGeoImageInformation(QString satname)
{
    txtInfo = QString("<!DOCTYPE html>"
                      "<html><head><title>Info</title></head>"
                      "<body>"
                      "<h2 style='color:blue'>Image Information</h1>"
                      "<h3>Segment type = %1</h3>"
                      "<h3>Image width = %3 height = %4</h3>"
                      "</body></html>").arg(satname).arg(imageptrs->ptrimageGeostationary->width()).arg(imageptrs->ptrimageGeostationary->height());


    formtoolbox->writeInfoToGeo(txtInfo);
}

void FormImage::displayOLCIImageInfo()
{
    QString segtype;
    eSegmentType type;
    int nbrselected;


    type = getSegmentType();
    switch(type)
    {
    case SEG_NONE:
        segtype = "None";
        break;
    case SEG_OLCIEFR:
        segtype = "OLCI efr";
        nbrselected = segs->seglolciefr->NbrOfSegmentsSelected();

        break;
    case SEG_OLCIERR:
        segtype = "OLCI err";
        nbrselected = segs->seglolcierr->NbrOfSegmentsSelected();

        break;
    default:
        segtype = "NA";
        break;
    }


    if(type == SEG_OLCIEFR || type == SEG_OLCIERR)
    {
        txtInfo = QString("<!DOCTYPE html>"
                          "<html><head><title>Info</title></head>"
                          "<body>"
                          "<h3 style='color:blue'>Image Information</h3>"
                          "<p>Segment type = %1<br>"
                          "Nbr of segments = %2<br>"
                          "Image width = %3 height = %4<br>"
                          "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimageOLCI->width()).arg(imageptrs->ptrimageOLCI->height());
        formtoolbox->writeInfoToOLCI(txtInfo);

    }

}


void FormImage::adjustPicSize(bool setwidth)
{

//    IMAGE_NONE = 0,
//    IMAGE_AVHRR_CH1,
//    IMAGE_AVHRR_CH2,
//    IMAGE_AVHRR_CH3,
//    IMAGE_AVHRR_CH4,
//    IMAGE_AVHRR_CH5,
//    IMAGE_AVHRR_COL,
//    IMAGE_AVHRR_EXPAND,
//    IMAGE_GEOSTATIONARY,
//    IMAGE_PROJECTION,
//    IMAGE_VIIRS_M,
//    IMAGE_VIIRS_DNB,
//    IMAGE_OLCI_EFR,
//    IMAGE_OLCI_ERR,
//    IMAGE_EQUIRECTANGLE

    QSize met;

    met.setWidth(imageptrs->ptrimageGeostationary->width());
    met.setHeight(imageptrs->ptrimageGeostationary->width());

    double w,h,mw,mh,rw,rh,g=1;
    if(channelshown >= 1 && channelshown <= 6)
    {
        w=imageptrs->ptrimagecomp_ch[0]->width();
        h=imageptrs->ptrimagecomp_ch[0]->height();
    }
    else if(channelshown == IMAGE_AVHRR_EXPAND)
    {
       w=imageptrs->ptrexpand_col->width();
       h=imageptrs->ptrexpand_col->height();
    }
    else if(channelshown == IMAGE_GEOSTATIONARY)
    {
       w=imageptrs->ptrimageGeostationary->width();
       h=imageptrs->ptrimageGeostationary->height();
    }
    else if(channelshown == IMAGE_PROJECTION)
    {
       w=imageptrs->ptrimageProjection->width();
       h=imageptrs->ptrimageProjection->height();
    }
    else if(channelshown == IMAGE_VIIRS_M)
    {
       w=imageptrs->ptrimageViirsM->width();
       h=imageptrs->ptrimageViirsM->height();
    }
    else if(channelshown == IMAGE_VIIRS_DNB)
    {
       w=imageptrs->ptrimageViirsDNB->width();
       h=imageptrs->ptrimageViirsDNB->height();
    }
    else if(channelshown == IMAGE_OLCI)
    {
       w=imageptrs->ptrimageOLCI->width();
       h=imageptrs->ptrimageOLCI->height();
    }

    mw=this->parentWidget()->width();
    mh=this->parentWidget()->height();

    rw=mw/w;
    rh=mh/h;

    if(rw<1 && rh>1)
    {
        g=rw;
    }
    if(rw>1 && rh<1)
    {
        g=rh;
    }
    if(rw<1 && rh<1)
    {
        if(setwidth == false)
        {
            if(rw<rh)
            {
                g=rw;
            }
            if(rh<rw)
            {
                g=rh;
            }
        }
        else
            g=rw;
    }


    scaleFactor = g;
    setZoomValue( g*100 );
    this->adjustImage();


}

void FormImage::adjustImage()
{

    scaleFactor = (double)getZoomValue()/100;
    if(scaleFactor==1)
    {
        imageLabel->adjustSize();
        this->adjustSize();
    }
    else
    {
        imageLabel->resize(imageLabel->pixmap()->size() * scaleFactor);
        this->resize(imageLabel->pixmap()->size() * scaleFactor);
    }

    QString windowTitleFormat = QString("EUMETCastView zoomLevel");
    windowTitleFormat.replace("zoomLevel", QString("%1%").arg((int)(getZoomValue())));
    this->topLevelWidget()->setWindowTitle(windowTitleFormat);

}

void FormImage::slotUpdateMeteosat()
{

    qDebug() << "start FormImage::slotUpdateMeteosat()";

    refreshoverlay = true;

    imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageGeostationary)));
    this->adjustImage();

    SegmentListGeostationary *sl;
    if(segs->seglmeteosat->bActiveSegmentList == true)
    {
        sl = segs->seglmeteosat;
    }
    else if(segs->seglmeteosatrss->bActiveSegmentList == true)
    {
        sl = segs->seglmeteosatrss;
    }
    else if(segs->seglmet8->bActiveSegmentList == true)
    {
        sl = segs->seglmet8;
    }
    else if(segs->seglmet7->bActiveSegmentList == true)
    {
        sl = segs->seglmet7;
    }
    else if(segs->seglgoes13dc3->bActiveSegmentList == true)
    {
        sl = segs->seglgoes13dc3;
    }
    else if(segs->seglgoes15dc3->bActiveSegmentList == true)
    {
        sl = segs->seglgoes15dc3;
    }
    else if(segs->seglgoes13dc4->bActiveSegmentList == true)
    {
        sl = segs->seglgoes13dc4;
    }
    else if(segs->seglgoes15dc4->bActiveSegmentList == true)
    {
        sl = segs->seglgoes15dc4;
    }
    else if(segs->seglfy2e->bActiveSegmentList == true)
    {
        sl = segs->seglfy2e;
    }
    else if(segs->seglfy2g->bActiveSegmentList == true)
    {
        sl = segs->seglfy2g;
    }
    else if(segs->seglh8->bActiveSegmentList == true)
    {
        sl = segs->seglh8;
    }
    else
        return;


    if(sl->allSegmentsReceived())
    {
        QApplication::restoreOverrideCursor();

        if(opts.imageontextureOnMet)
        {
            if(sl->getKindofImage() == "HRV" || sl->getKindofImage() == "HRV Color")
            {
                qDebug() << "all HRV received !!!!!!!!!!!!!!";
                emit allsegmentsreceivedbuttons(true);
            }
            else
            {
                qDebug() << "all VIS_IR received !!!!!!!!!!!!!!";
                emit render3dgeo(sl->getGeoSatellite());
            }
        }
        else
            emit allsegmentsreceivedbuttons(true);
    }

    qDebug() << "FormImage::slotUpdateMeteosat()";
    this->update();

}

//void FormImage::slotUpdateHimawari()
//{

//    qDebug() << "start FormImage::slotUpdateHimawari()";

//    refreshoverlay = true;

//    imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageGeostationary)));

//    SegmentListGeostationary *sl;

//    if(segs->seglh8->bActiveSegmentList == true)
//    {
//        sl = segs->seglh8;
//    }
//    else
//        return;


//    if(sl->allSegmentsReceived())
//    {

//        QApplication::restoreOverrideCursor();

//        if(opts.imageontextureOnMet)
//        {
//            qDebug() << "all VIS_IR received !!!!!!!!!!!!!!";
//            emit render3dgeo(sl->getGeoSatellite());
//        }
//        else
//            emit allsegmentsreceivedbuttons(true);
//    }

//    //imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageGeostationary)));

//    qDebug() << "FormImage::slotUpdateHimawari()";

//    this->update();

//}



void FormImage::slotUpdateProjection()
{
    this->displayImage(IMAGE_PROJECTION);
    this->refreshoverlay = true;
    this->update();
}


QSize FormImage::getPictureSize() const
{
    QSize g;
    if(!imageLabel->pixmap()->isNull())
        g = imageLabel->pixmap()->size();
    else
        g = QSize(-1,-1);

    return g;
}

void FormImage::recalculateCLAHEAvhrr(QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    quint16 *pixelsRed;
    quint16 *pixelsGreen;
    quint16 *pixelsBlue;

    size_t npix;

    SegmentListMetop *sl = segs->seglmetop;
    if(sl->GetSegsSelectedptr()->count() > 0)
    {
        qDebug() << "segs->seglmetop count selected = " << sl->GetSegsSelectedptr()->count();
        qDebug() << "height = " << imageptrs->ptrimagecomp_col->height();
        qDebug() << "width = " << imageptrs->ptrimagecomp_col->width();
        qDebug() << this->kindofimage;
        qDebug() << spectrumvector.at(0) << " " << spectrumvector.at(1) << " " << spectrumvector.at(2);
        //npix = imageptrs->ptrimagecomp_col->height() * imageptrs->ptrimagecomp_col->width();
        //memcpy(pixelsRed, imageptrs->ptrimagecomp_ch[0], npix * sizeof(quint32));

    }

}

void FormImage::recalculateCLAHE(QVector<QString> spectrumvector, QVector<bool> inversevector)
{

    QRgb *row_col;
    quint16 cred, cgreen, cblue, c;
    quint16 r,g, b;

    SegmentListGeostationary *sl;

    if(segs->seglmeteosat->bActiveSegmentList == true)
    {
        sl = segs->seglmeteosat;
    }
    else if(segs->seglmeteosatrss->bActiveSegmentList == true)
    {
        sl = segs->seglmeteosatrss;
    }
    else if(segs->seglmet8->bActiveSegmentList == true)
    {
        sl = segs->seglmet8;
    }
    else if(segs->seglmet7->bActiveSegmentList == true)
    {
        sl = segs->seglmet7;
    }
    else if(segs->seglgoes13dc3->bActiveSegmentList == true)
    {
        sl = segs->seglgoes13dc3;
    }
    else if(segs->seglgoes15dc3->bActiveSegmentList == true)
    {
        sl = segs->seglgoes15dc3;
    }
    else if(segs->seglgoes13dc4->bActiveSegmentList == true)
    {
        sl = segs->seglgoes13dc4;
    }
    else if(segs->seglgoes15dc4->bActiveSegmentList == true)
    {
        sl = segs->seglgoes15dc4;
    }
    else if(segs->seglfy2e->bActiveSegmentList == true)
    {
        sl = segs->seglfy2e;
    }
    else if(segs->seglfy2g->bActiveSegmentList == true)
    {
        sl = segs->seglfy2g;
    }
    else if(segs->seglh8->bActiveSegmentList == true)
    {
        sl = segs->seglh8;
    }
    else
        return;

    if (sl->getKindofImage() == "HRV Color")
        return;

    size_t npix;
    size_t npixHRV;
    if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8)
    {
        npix = 3712*3712;
        if (sl->areatype == 1)
            npixHRV = 5568*11136;
        else
            npixHRV = 5568*5*464;
    }
    else if(sl->getGeoSatellite() == SegmentListGeostationary::MET_9)
    {
        npix = 3712*3*464;
        npixHRV = 5568*5*464;
    }
    else if(sl->getGeoSatellite() == SegmentListGeostationary::MET_7)
    {
        if(spectrumvector.at(0) == "00_7_0")
        {
            npix = 5032*10*500;
            npixHRV = 0;
        }
        else
        {
            npix = 2532*5*500;
            npixHRV = 0;
        }
    }
    else if(sl->getGeoSatellite() == SegmentListGeostationary::GOES_13 || sl->getGeoSatellite() == SegmentListGeostationary::GOES_15)
    {
        npix = 2816*7*464;
        npixHRV = 0;
    }
    else if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
    {
        npix = 2288*2288;
        npixHRV = 9152*9152;
    }
    else if(sl->getGeoSatellite() == SegmentListGeostationary::H8)
    {
        npix = 5500*10*550;
        npixHRV = 0;
    }


    QApplication::setOverrideCursor( Qt::WaitCursor ); // this might take time

    quint16 *pixelsRed;
    quint16 *pixelsGreen;
    quint16 *pixelsBlue;
    quint16 *pixelsHRV;

    qDebug() << QString("recalculateCLAHE() ; kind of image = %1").arg(sl->getKindofImage());

    if(sl->getKindofImage() == "VIS_IR Color" && (sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8 ))
    {
        pixelsRed = new quint16[npix];
        pixelsGreen = new quint16[npix];
        pixelsBlue = new quint16[npix];

        for( int i = (sl->bisRSS ? 5 : 0); i < 8; i++)
        {
            if(sl->isPresentRed[i])
                memcpy(pixelsRed + (sl->bisRSS ? i - 5 : i) * 464 * 3712, imageptrs->ptrRed[i], 464 * 3712 * sizeof(quint16));
        }
        for( int i = (sl ->bisRSS ? 5 : 0); i < 8; i++)
        {
            if(sl->isPresentGreen[i])
                memcpy(pixelsGreen + (sl->bisRSS ? i - 5 : i) * 464 * 3712, imageptrs->ptrGreen[i], 464 * 3712 * sizeof(quint16));
        }
        for( int i = (sl ->bisRSS ? 5 : 0); i < 8; i++)
        {
            if(sl->isPresentBlue[i])
                memcpy(pixelsBlue + (sl->bisRSS ? i - 5 : i) * 464 * 3712, imageptrs->ptrBlue[i], 464 * 3712 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == SegmentListGeostationary::H8)
    {
        qDebug() << QString("memcpy(pixelsRed .....");

        pixelsRed = new quint16[npix];
        pixelsGreen = new quint16[npix];
        pixelsBlue = new quint16[npix];

        for( int i = 0; i < 10; i++)
        {
            if(sl->isPresentRed[i])
                memcpy(pixelsRed + i * 550 * 5500, imageptrs->ptrRed[i], 550 * 5500 * sizeof(quint16));
        }
        for( int i = 0; i < 10; i++)
        {
            if(sl->isPresentGreen[i])
                memcpy(pixelsGreen + i * 550 * 5500, imageptrs->ptrGreen[i], 550 * 5500 * sizeof(quint16));
        }
        for( int i = 0; i < 10; i++)
        {
            if(sl->isPresentBlue[i])
                memcpy(pixelsBlue + i * 550 * 5500, imageptrs->ptrBlue[i], 550 * 5500 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "HRV" && (sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8 ))
    {
        pixelsHRV = new quint16[npixHRV];
        for( int i = 0, k = 0; i < (sl->bisRSS ? 5 : ( sl->areatype == 1 ? 24 : 5)); i++)
        {
            k = (sl->bisRSS ? 19 + i : (sl->areatype == 1 ? i : 19 + i));
            if(sl->isPresentHRV[k])
            {
                qDebug() << QString("is present %1").arg(k);
                memcpy(pixelsHRV + i * 464 * 5568, imageptrs->ptrHRV[k], 464 * 5568 * sizeof(quint16));
            }
        }
    }
    else if(sl->getKindofImage() == "HRV" && (sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G ))
    {
        pixelsHRV = new quint16[npixHRV];
        memcpy(pixelsHRV, imageptrs->ptrHRV[0], 9152 * 9152 * sizeof(quint16));
    }
    else if(sl->getKindofImage() == "VIS_IR" && (sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8 ))
    {
        pixelsRed = new quint16[npix];
        for( int i = (sl->bisRSS ? 5 : 0); i < 8 ; i++)
        {
            if(sl->isPresentRed[i])
                memcpy(pixelsRed + (sl->bisRSS ? i - 5 : i) * 464 * 3712, imageptrs->ptrRed[i], 464 * 3712 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR" && sl->getGeoSatellite() == SegmentListGeostationary::MET_7 )
    {
        pixelsRed = new quint16[npix];
        if(spectrumvector.at(0) == "00_7_0")
        {
            for( int i = 0; i < 10 ; i++)
            {
                if(sl->isPresentMono[i])
                    memcpy(pixelsRed + i * 500 * 5032, imageptrs->ptrRed[i], 500 * 5032 * sizeof(quint16));
            }
        }
        else
        {
            for( int i = 0; i < 5 ; i++)
            {
                if(sl->isPresentMono[i])
                    memcpy(pixelsRed + i * 500 * 2532, imageptrs->ptrRed[i], 500 * 2532 * sizeof(quint16));
            }
        }
    }
    else if(sl->getKindofImage() == "VIS_IR" && (sl->getGeoSatellite() == SegmentListGeostationary::GOES_13 || sl->getGeoSatellite() == SegmentListGeostationary::GOES_15))
    {
        pixelsRed = new quint16[npix];
        for( int i = 0; i < 7 ; i++)
        {
            if(sl->isPresentMono[i])
                memcpy(pixelsRed + i * 464 * 2816, imageptrs->ptrRed[i], 464 * 2816 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR" && (sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G ))
    {
        pixelsRed = new quint16[npix];
        if(sl->isPresentMono[0])
        {
            memcpy(pixelsRed, imageptrs->ptrRed[0], 2288 * 2288 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR" && sl->getGeoSatellite() == SegmentListGeostationary::H8)
    {
        pixelsRed = new quint16[npix];
        for( int i = 0; i < 10 ; i++)
        {
            if(sl->isPresentRed[i])
                memcpy(pixelsRed + i * 550 * 5500, imageptrs->ptrRed[i], 550 * 5500 * sizeof(quint16));
        }
    }

    int ret = 0;


    if(sl->getKindofImage() == "VIS_IR Color" && (sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8 ))
    {
        imageptrs->CLAHE(pixelsRed, 3712, (sl->bisRSS ? 3*464 : 3712), 0, 1023, 16, 16, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsGreen, 3712, (sl->bisRSS ? 3*464 : 3712), 0, 1023, 16, 16, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsBlue, 3712, (sl->bisRSS ? 3*464 : 3712), 0, 1023, 16, 16, 256, opts.clahecliplimit);
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == SegmentListGeostationary::H8 )
    {
        ret = imageptrs->CLAHE(pixelsRed, 5500, 5500, 0, 1023, 10, 10, 256, opts.clahecliplimit);
        qDebug() << QString("CLAHE return code = %1").arg(ret);
        imageptrs->CLAHE(pixelsGreen, 5500, 5500, 0, 1023, 10, 10, 256, opts.clahecliplimit);
        qDebug() << QString("CLAHE return code = %1").arg(ret);
        imageptrs->CLAHE(pixelsBlue, 5500, 5500, 0, 1023, 10, 10, 256, opts.clahecliplimit);
        qDebug() << QString("CLAHE return code = %1").arg(ret);
    }
    else if(sl->getKindofImage() == "HRV" && (sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8))
    {
        if(sl->bisRSS)
        {
            qDebug() << "recalculateCLAHE() ; isRSS = true";
            imageptrs->CLAHE(pixelsHRV, 5568, 5*464, 0, 1023, 16, 16, 256, opts.clahecliplimit);

        }
        else
        {
            if(sl->areatype == 1)
            {
                qDebug() << "recalculateCLAHE() ; areatype == 1";
                imageptrs->CLAHE(pixelsHRV, 5568, 11136, 0, 1023, 16, 16, 256, opts.clahecliplimit);
            }
            else
            {
                qDebug() << "recalculateCLAHE() ; areatype == 0";
                imageptrs->CLAHE(pixelsHRV, 5568, 5*464, 0, 1023, 16, 16, 256, opts.clahecliplimit);
            }
        }
    }
    else if(sl->getKindofImage() == "HRV" && (sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G ))
    {
        imageptrs->CLAHE(pixelsHRV, 9152, 9152, 0, 255, 16, 16, 256, opts.clahecliplimit);
    }
    else if(sl->getKindofImage() == "VIS_IR")
    {
        if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8)
            imageptrs->CLAHE(pixelsRed, 3712, 3712, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::MET_9)
            imageptrs->CLAHE(pixelsRed, 3712, 3*464, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::MET_7)
        {
            qDebug() << "SegmentListMeteosat::MET_7";

            if(spectrumvector.at(0) == "00_7_0")
                imageptrs->CLAHE(pixelsRed, 5032, 5000, 0, 255, 8, 10, 256, opts.clahecliplimit);
            else
                imageptrs->CLAHE(pixelsRed, 2532, 5*500, 0, 255, 12, 10, 256, opts.clahecliplimit);
        }
        else if(sl->getGeoSatellite() == SegmentListGeostationary::GOES_13 || sl->getGeoSatellite() == SegmentListGeostationary::GOES_15)
            imageptrs->CLAHE(pixelsRed, 2816, 464*7, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
            imageptrs->CLAHE(pixelsRed, 2288, 2288, 0, 255, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == SegmentListGeostationary::H8)
            imageptrs->CLAHE(pixelsRed, 5500, 5500, 0, 1023, 10, 10, 256, opts.clahecliplimit);
    }

    //g_mutex.lock();

    if(sl->getKindofImage() == "VIS_IR Color" && (sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8 ))
    {
        for(int i = 0; i < (sl->bisRSS ? 3 : 8); i++)
        {
            for (int line = 463; line >= 0; line--)
            {
                row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine((sl->bisRSS ? 3*464 : 3712) - i * 464 - line - 1);

                for (int pixelx = 3711; pixelx >= 0; pixelx--)
                {
                    cred = *(pixelsRed + i * 464 * 3712 + line * 3712  + pixelx);
                    cgreen = *(pixelsGreen + i * 464 * 3712 + line * 3712  + pixelx);
                    cblue = *(pixelsBlue + i * 464 * 3712 + line * 3712  + pixelx);


                    r = quint8(inversevector[0] ? 255 - cred/4 : cred/4);
                    g = quint8(inversevector[1] ? 255 - cgreen/4 : cgreen/4);
                    b = quint8(inversevector[2] ? 255 - cblue/4 : cblue/4);

                    row_col[3711 - pixelx] = qRgb(r,g,b);
                }
            }
        }
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == SegmentListGeostationary::H8)
    {

        for(int i = 0; i < 10; i++)
        {
            for (int line = 0; line < 550; line++)
            {
                row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(i * 550 + line);
                for (int pixelx = 0; pixelx < 5500; pixelx++)
                {
                    cred = *(pixelsRed + i * 550 * 5500 + line * 5500  + pixelx);
                    cgreen = *(pixelsGreen + i * 550 * 5500 + line * 5500  + pixelx);
                    cblue = *(pixelsBlue + i * 550 * 5500 + line * 5500  + pixelx);

                    r = quint8(inversevector[0] ? 255 - cred/4 : cred/4);
                    g = quint8(inversevector[1] ? 255 - cgreen/4 : cgreen/4);
                    b = quint8(inversevector[2] ? 255 - cblue/4 : cblue/4);

                    row_col[pixelx] = qRgb(r,g,b);
                }
            }
        }
    }
    else if(sl->getKindofImage() == "HRV")
    {
        if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8)
        {
            for(int i = 0; i < (sl->bisRSS ? 5 : (sl->areatype == 1 ? 24 : 5)); i++)
            {
                for (int line = 463; line >= 0; line--)
                {
                    row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine((sl->bisRSS ? 5 : (sl->areatype == 1 ? 24 : 5))*464 - i * 464 - line - 1);
                    for (int pixelx = 5567; pixelx >= 0; pixelx--)
                    {
                        c = *(pixelsHRV + i * 464 * 5568 + line * 5568  + pixelx);
                        r = quint8(inversevector[0] ? 255 - c/4 : c/4);
                        g = quint8(inversevector[0] ? 255 - c/4 : c/4);
                        b = quint8(inversevector[0] ? 255 - c/4 : c/4);

                        row_col[5567-pixelx] = qRgb(r,g,b);
                    }
                }
            }
        }
        else if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
        {
            for (int line = 0; line < 9152; line++)
            {
                row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
                for (int pixelx = 0; pixelx < 9152; pixelx++)
                {
                    c = *(pixelsHRV + line * 9152  + pixelx);

                    r = quint8(inversevector[0] ? 255 - c : c);
                    g = quint8(inversevector[0] ? 255 - c : c);
                    b = quint8(inversevector[0] ? 255 - c : c);
                    row_col[pixelx] = qRgb(r,g,b);
                }
            }
        }
    }
    else if(sl->getKindofImage() == "VIS_IR")
    {
        if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8 )
        {
            for(int i = 0 ; i < (sl->bisRSS ? 3 : 8); i++)
            {
                for (int line = 463; line >= 0; line--)
                {
                    row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine((sl->bisRSS ? 3*464 : 3712) - i * 464 - line - 1);
                    for (int pixelx = 3711; pixelx >= 0; pixelx--)
                    {
                        c = *(pixelsRed + i * 464 * 3712 + line * 3712  + pixelx);

                        r = quint8(inversevector[0] ? 255 - c/4 : c/4);
                        g = quint8(inversevector[0] ? 255 - c/4 : c/4);
                        b = quint8(inversevector[0] ? 255 - c/4 : c/4);

                        row_col[3711 - pixelx] = qRgb(r,g,b);
                    }
                }
            }
        }
        else if(sl->getGeoSatellite() == SegmentListGeostationary::MET_7 )
        {
            if(spectrumvector.at(0) == "00_7_0")
            {
                for(int i = 0 ; i < 10; i++)
                {
                    for (int line = 0; line < 500; line++)
                    {
                        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(5000 - i * 500 - line - 1);
                        for (int pixelx = 5031; pixelx >= 0; pixelx--)
                        {
                            c = *(pixelsRed + i * 500 * 5032 + line * 5032  + pixelx);

                            r = quint8(inversevector[0] ? 255 - c : c);
                            g = quint8(inversevector[0] ? 255 - c : c);
                            b = quint8(inversevector[0] ? 255 - c : c);

                            row_col[5031 - pixelx] = qRgb(r,g,b);
                        }
                    }
                }
            }
            else
            {
                for(int i = 0 ; i < 5; i++)
                {
                    for (int line = 0; line < 500; line++)
                    {
                        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(5*500 - i * 500 - line - 1);
                        for (int pixelx = 2531; pixelx >= 0; pixelx--)
                        {
                            c = *(pixelsRed + i * 500 * 2532 + line * 2532  + pixelx);

                            r = quint8(inversevector[0] ? 255 - c : c);
                            g = quint8(inversevector[0] ? 255 - c : c);
                            b = quint8(inversevector[0] ? 255 - c : c);

                            row_col[2531 - pixelx] = qRgb(r,g,b);
                        }
                    }
                }
            }
        }
        else if(sl->getGeoSatellite() == SegmentListGeostationary::GOES_13 || sl->getGeoSatellite() == SegmentListGeostationary::GOES_15)
        {
            for(int i = 0 ; i < 7; i++)
            {
                //for (int line = 463; line >= 0; line--)
                for (int line = 0; line < 464; line++)
                {
                    row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(i * 464 + line);
                    for (int pixelx = 0; pixelx < 2816; pixelx++)
                    {
                        c = *(pixelsRed + i * 464 * 2816 + line * 2816  + pixelx);

                        r = quint8(inversevector[0] ? 255 - c/4 : c/4);
                        g = quint8(inversevector[0] ? 255 - c/4 : c/4);
                        b = quint8(inversevector[0] ? 255 - c/4 : c/4);

                        row_col[pixelx] = qRgb(r,g,b);
                    }
                }
            }
        }
        else if(sl->getGeoSatellite() == SegmentListGeostationary::H8 )
        {
            qDebug() << "in if(sl->getGeoSatellite() == SegmentListGeostationary::H8 )";

            for(int i = 0 ; i < 10; i++)
            {
                //for (int line = 549; line >= 0; line--)
                for (int line = 0; line < 550; line++)
                {
                    row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(i * 550 + line);
                    for (int pixelx = 0; pixelx < 5500; pixelx++)
                    {
                        c = *(pixelsRed + i * 550 * 5500 + line * 5500  + pixelx);

                        r = quint8(inversevector[0] ? 255 - c/4 : c/4);
                        g = quint8(inversevector[0] ? 255 - c/4 : c/4);
                        b = quint8(inversevector[0] ? 255 - c/4 : c/4);

                        row_col[pixelx] = qRgb(r,g,b);
                    }
                }
            }
        }
        else if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
        {
            qDebug() << "recalculate CLAHE ; VIS_IR and FY2E/G move to ptrImageGeostationary";

            for (int line = 0; line < 2288; line++)
            {
                row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
                for (int pixelx = 0; pixelx < 2288; pixelx++)
                {
                    c = *(pixelsRed + line * 2288  + pixelx);

                    r = quint8(inversevector[0] ? 255 - c : c);
                    g = quint8(inversevector[0] ? 255 - c : c);
                    b = quint8(inversevector[0] ? 255 - c : c);

                    row_col[pixelx] = qRgb(r,g,b);
                }
            }
        }
    }

    //g_mutex.unlock();

    if(sl->getKindofImage() == "VIS_IR Color" )
    {
        delete [] pixelsRed;
        delete [] pixelsGreen;
        delete [] pixelsBlue;
    }
    else if(sl->getKindofImage() == "HRV")
    {
        delete [] pixelsHRV;
    }
    else
    {
        delete [] pixelsRed;
    }

    if(sl->getKindofImage() != "HRV" && sl->getKindofImage() != "HRV Color")
        if(opts.imageontextureOnMet)
            emit render3dgeo(sl->getGeoSatellite());

    QApplication::restoreOverrideCursor();
}

void FormImage::CLAHEprojection()
{
    quint16 *pixelsRed;
    quint16 *pixelsGreen;
    quint16 *pixelsBlue;
    QRgb *scan;
    QRgb rgb;
    int projwidth = imageptrs->ptrimageProjection->width();
    int projheight = imageptrs->ptrimageProjection->height();

    qDebug() << "FormImage::CLAHEprojection()";

    QApplication::setOverrideCursor( Qt::WaitCursor ); // this might take time

    pixelsRed = new quint16[projwidth * projheight];
    pixelsGreen = new quint16[projwidth * projheight];
    pixelsBlue = new quint16[projwidth * projheight];

    for(int line = 0; line < projheight; line++)
    {
        scan = (QRgb *)imageptrs->ptrimageProjection->scanLine(line);

        for(int pixelx = 0; pixelx < projwidth; pixelx++)
        {
            rgb = scan[pixelx];
            pixelsRed[line * projwidth + pixelx] = qRed(rgb);
            pixelsGreen[line * projwidth + pixelx] = qGreen(rgb);
            pixelsBlue[line * projwidth + pixelx] = qBlue(rgb);

        }
    }

    imageptrs->CLAHE(pixelsRed, projwidth, projheight, 0, 1023, 16, 16, 256, opts.clahecliplimit);
    imageptrs->CLAHE(pixelsGreen, projwidth, projheight, 0, 1023, 16, 16, 256, opts.clahecliplimit);
    imageptrs->CLAHE(pixelsBlue, projwidth, projheight, 0, 1023, 16, 16, 256, opts.clahecliplimit);



    for(int line = 0; line < projheight; line++)
    {
        scan = (QRgb *)imageptrs->ptrimageProjection->scanLine(line);

        for(int pixelx = 0; pixelx < projwidth; pixelx++)
        {
            rgb = qRgb(pixelsRed[line * projwidth + pixelx],
                    pixelsGreen[line * projwidth + pixelx],
                    pixelsBlue[line * projwidth + pixelx]);
            scan[pixelx] = rgb;

        }
    }

    this->slotUpdateProjection();

    QApplication::restoreOverrideCursor();

    delete [] pixelsRed;
    delete [] pixelsGreen;
    delete [] pixelsBlue;
}


void FormImage::fillptrimage(quint16 *pix)
{
    QRgb *row_col;
    quint16 c;
    int r,g, b;

    qDebug() << "in fillptrimage";
    for(int i = 0; i < 8; i++)
    {
        for (int line = 463; line >= 0; line--)
        {
            row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(3711 - i * 464 - line);
            for (int pixelx = 3711; pixelx >= 0; pixelx--)
            {
                c = *(pix + i * 464 * 3712 + line * 3712  + pixelx);

                r = quint8(c/4);
                g = quint8(c/4);
                b = quint8(c/4);

                row_col[3711 - pixelx] = qRgb(r,g,b);
            }
        }
    }
}

void FormImage::fillptrimageHRV(quint16 *pixHRV)
{
    QRgb *row_col;
    quint16 c;
    int r,g, b;

    qDebug() << "in fillptrimageHRV";
    for(int i = 0; i < (segs->seglmeteosat->areatype == 1 ? 24 : 5); i++)
    {
        for (int line = 463; line >= 0; line--)
        {
            row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine((segs->seglmeteosat->areatype == 1 ? 24 : 5)*464 - i * 464 - line);
            for (int pixelx = 5567; pixelx >= 0; pixelx--)
            {
                c = *(pixHRV + i * 464 * 5568 + line * 5568  + pixelx);

                r = quint8(c/4);
                g = quint8(c/4);
                b = quint8(c/4);

                row_col[5567 - pixelx] = qRgb(r,g,b);
            }
        }
    }
}


void FormImage::OverlayGeostationary(QPainter *paint, SegmentListGeostationary *sl)
{

    qDebug() << "FormImage::OverlayGeostationary(QPainter *paint, SegmentListGeostationary *sl)";

    if (imageLabel->pixmap() == 0)
        return;
    pixgeoConversion pixconv;

    long coff;
    long loff;
    double cfac;
    double lfac;

    int col, save_col;
    int row, save_row;
    bool first = true;

    double radius;
    QVector3D pos;
    double lat_deg;
    double lon_deg;
    //QRgb rgbval = qRgb(255, 255, 255);
    int ret;


    if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
    {
        if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
        {
        } else
        {
            if(sl->LowerNorthLineActual == 0)
                return;
        }
    }

    if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
    {
        if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
        {
            lfac=LFAC_HRV_FENGYUN;
            cfac=CFAC_HRV_FENGYUN;

            coff=COFF_HRV_FENGYUN;
            loff=LOFF_HRV_FENGYUN;
        }
        else
        {
            lfac=LFAC_HRV;
            cfac=CFAC_HRV;

            coff=COFF_HRV;
            loff=LOFF_HRV;
        }
    }
    else
    {
        if(sl->getGeoSatellite() == SegmentListGeostationary::MET_10 || sl->getGeoSatellite() == SegmentListGeostationary::MET_9 || sl->getGeoSatellite() == SegmentListGeostationary::MET_8)
        {
            lfac=LFAC_NONHRV;
            cfac=CFAC_NONHRV;

            coff=COFF_NONHRV;
            loff=LOFF_NONHRV;
        }
        else if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
        {
            lfac=LFAC_NONHRV_FENGYUN;
            cfac=CFAC_NONHRV_FENGYUN;

            coff=COFF_NONHRV_FENGYUN;
            loff=LOFF_NONHRV_FENGYUN;
        }
        else if(sl->getGeoSatellite() == SegmentListGeostationary::MET_7)
        {
            if(paint->window().height() == 5000)
            {
                lfac=LFAC_NONHRV_MET7;
                cfac=CFAC_NONHRV_MET7;

                coff=COFF_NONHRV_MET7;
                loff=LOFF_NONHRV_MET7;
            }
            else
            {
                lfac=LFAC_NONHRV_MET7/2;
                cfac=CFAC_NONHRV_MET7/2;

                coff=COFF_NONHRV_MET7/2;
                loff=LOFF_NONHRV_MET7/2;
            }
        }
        else if(sl->getGeoSatellite() == SegmentListGeostationary::GOES_13 || sl->getGeoSatellite() == SegmentListGeostationary::GOES_15)
        {
            lfac=LFAC_NONHRV_GOES;
            cfac=CFAC_NONHRV_GOES;

            coff=COFF_NONHRV_GOES;
            loff=LOFF_NONHRV_GOES;
        }
        else if(sl->getGeoSatellite() == SegmentListGeostationary::H8)
        {
            lfac=LFAC_NONHRV_H8;
            cfac=CFAC_NONHRV_H8;

            coff=COFF_NONHRV_H8;
            loff=LOFF_NONHRV_H8;
        }
    }

    double sub_lon = sl->geosatlon;
    lat_deg = opts.obslat;
    lon_deg = opts.obslon;
    if (lon_deg > 180.0)
        lon_deg -= 360.0;

    qDebug() << QString("in  FormImage::OverlayGeostationary(QPainter *paint) kindofimage = %1 isRSS = %2 getgeosatellite = %3 sub_lon = %4").arg(sl->getKindofImage()).arg(sl->bisRSS).arg(sl->getGeoSatellite()).arg(sub_lon);

    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg,coff,loff,cfac,lfac,&col, &row);
    if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
    {
        row+=3;
        col+=2;
    }


    if(ret == 0)
    {
        if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
        {

        } else
        {
            if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
            {
                if (row > 11136 - sl->LowerNorthLineActual ) //LOWER
                {
                    col = col - (11136 - sl->LowerWestColumnActual);
                }
                else //UPPER
                {
                    col = col - (11136 - sl->UpperWestColumnActual - 1);
                }
            }
        }

        QPoint pt(col, row);
        QPoint ptleft(col-5, row);
        QPoint ptright(col+5, row);
        QPoint ptup(col, row-5);
        QPoint ptdown(col, row+5);

        paint->setPen(qRgb(255, 0, 0));
        paint->drawLine(ptleft,ptright);
        paint->drawLine(ptup,ptdown);
        //paint->drawEllipse(pt, 2, 2);
    }

    if(opts.gshhsglobe1On)
    {
        for (int i=0; i<gshhsdata->vxp_data_overlay[0]->nFeatures; i++)
        {
            for (int j=0; j<gshhsdata->vxp_data_overlay[0]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhsdata->vxp_data_overlay[0]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhsdata->vxp_data_overlay[0]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                if((lon_deg < 90.0 || lon_deg > -90.0)) // && ( sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color" ? (sl->bisRSS ? lat_deg > 30.0 : (sl->areatype == 0 ? lat_deg > 30.0 : true )) : true))
                {
                    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg,coff,loff,cfac,lfac,&col, &row);
                    if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
                    {
                        row+=3;
                        col+=2;
                    }
                    if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
                    {

                    } else
                    {
                        if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
                        {
                            if (row > 11136 - sl->LowerNorthLineActual ) //LOWER
                            {
                                if( save_row <= 11136 - sl->LowerNorthLineActual )
                                    first = true;
                                col = col - (11136 - sl->LowerWestColumnActual);
                            }
                            else //UPPER
                            {
                                if( save_row > 11136 - sl->LowerNorthLineActual )
                                    first = true;
                                col = col - (11136 - sl->UpperWestColumnActual - 1);
                            }
                        }
                    }

                    if(ret == 0)
                    {
//                        if(sl->getGeoSatellite() == SegmentListGeostationary::ELECTRO_N1)
//                        {
//                            col+=3;
//                        }
                        if (first)
                        {
                            first = false;
                            save_col = col;
                            save_row = row;
                        }
                        else
                        {
                            paint->setPen(opts.imageoverlaycolor1);
                            paint->drawLine(save_col, save_row, col, row);
                            save_col = col;
                            save_row = row;
                        }
                    }
                    else
                        first = true;
                }
            }
            first = true;
        }
    }

    if(opts.gshhsglobe2On)
    {
        first = true;

        for (int i=0; i<gshhsdata->vxp_data_overlay[1]->nFeatures; i++)
        {
            for (int j=0; j<gshhsdata->vxp_data_overlay[1]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhsdata->vxp_data_overlay[1]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhsdata->vxp_data_overlay[1]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                if(lon_deg < 90.0 || lon_deg > -90.0)
                {
                    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg,coff,loff,cfac,lfac,&col, &row);
                    if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
                    {
                        row+=3;
                        col+=2;
                    }

                    if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
                    {

                    }
                    else
                    {
                        if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
                        {
                            if (row > 11136 - sl->LowerNorthLineActual ) //LOWER
                            {
                                if( save_row <= 11136 - sl->LowerNorthLineActual )
                                    first = true;
                                col = col - (11136 - sl->LowerWestColumnActual);
                            }
                            else //UPPER
                            {
                                if( save_row > 11136 - sl->LowerNorthLineActual )
                                    first = true;
                                col = col - (11136 - sl->UpperWestColumnActual - 1);
                            }
                        }
                    }

                    if(ret == 0)
                    {
                        if (first)
                        {
                            first = false;
                            save_col = col;
                            save_row = row;
                        }
                        else
                        {
                            paint->setPen(opts.imageoverlaycolor2);
                            paint->drawLine(save_col, save_row, col, row);
                            save_col = col;
                            save_row = row;
                        }
                    }
                    else
                        first = true;
                }
            }
            first = true;
        }
    }

    if(opts.gshhsglobe3On)
    {
        first = true;

        for (int i=0; i<gshhsdata->vxp_data_overlay[2]->nFeatures; i++)
        {
            for (int j=0; j<gshhsdata->vxp_data_overlay[2]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhsdata->vxp_data_overlay[2]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhsdata->vxp_data_overlay[2]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                if(lon_deg < 90.0 || lon_deg > -90.0)
                {

                    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg,coff,loff,cfac,lfac,&col, &row);
                    if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
                    {
                        row+=3;
                        col+=2;
                    }

                    if(sl->getGeoSatellite() == SegmentListGeostationary::FY2E || sl->getGeoSatellite() == SegmentListGeostationary::FY2G)
                    {

                    }
                    else
                    {
                        if(sl->getKindofImage() == "HRV" || sl->getKindofImage()  == "HRV Color")
                        {
                            if (row > 11136 - sl->LowerNorthLineActual ) //LOWER
                            {
                                if( save_row <= 11136 - sl->LowerNorthLineActual )
                                    first = true;
                                col = col - (11136 - sl->LowerWestColumnActual);
                            }
                            else //UPPER
                            {
                                if( save_row > 11136 - sl->LowerNorthLineActual )
                                    first = true;
                                col = col - (11136 - sl->UpperWestColumnActual - 1);
                            }
                        }
                    }

                    if(ret == 0)
                    {
                        if (first)
                        {
                            first = false;
                            save_col = col;
                            save_row = row;
                        }
                        else
                        {
                            paint->setPen(opts.imageoverlaycolor3);
                            paint->drawLine(save_col, save_row, col, row);
                            save_col = col;
                            save_row = row;
                        }
                    }
                    else
                        first = true;
                }
            }
            first = true;
        }
    }
}




/*
void FormImage::OverlayAVHRRImage(QPainter *paint)
{
    QList<Segment*> *slmetop = segs->seglmetop->GetSegsSelectedptr();

    int cnt = slmetop->count();

    QList<Segment*>::iterator segsel = slmetop->begin();
    while ( segsel != slmetop->end() )
    {
        SegmentMetop *segm = (SegmentMetop *)(*segsel);

        for(int x = 1; x < 103; x++)
        {
            for(int y = 1; y < 1080;y++)
            {
                if(segm->earth_loc_lon[y-1][x-1] < opts.obslon && opts.obslon < segm->earth_loc_lon[y-1][x] &&
                   segm->earth_loc_lon[y][x-1] < opts.obslon && opts.obslon < segm->earth_loc_lon[y][x]) // &&
                   //segm->earth_loc_lat[y-1][x-1] > opts.obslat && opts.obslat > segm->earth_loc_lat[y][x-1] &&
                   //segm->earth_loc_lat[y-1][x] > opts.obslat && opts.obslat > segm->earth_loc_lat[y][x])
                {
                    QPoint pt(x, y);
                    paint->setPen(qRgb(255, 0, 0));
                    paint->drawEllipse(pt, 2, 2);
                }

            }
        }
        ++segsel;
    }

    //slmetop->earth_loc_lon[1080][103]
}
*/

void FormImage::OverlayProjection(QPainter *paint, SegmentListGeostationary *sl)
{
    qDebug() << QString("FormImage::OverlayProjection(QPainter *paint, SegmentListGeostationary *sl) opts.currenttoolbox = %1").arg(opts.currenttoolbox);

    bool first = true;
    double lat_deg;
    double lon_deg;
    bool bret;

    double map_x, map_y;
    double save_map_x, save_map_y;

    lat_deg = opts.obslat;
    lon_deg = opts.obslon;
    if (lon_deg > 180.0)
        lon_deg -= 360.0;

    if (opts.currenttoolbox == 0)           // LCC
        bret = imageptrs->lcc->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y);
    else if (opts.currenttoolbox == 1)      // GVP
        bret = imageptrs->gvp->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;
    else                                    //SG
        bret = imageptrs->sg->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;

    if(bret)
    {
        if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
        {
            map_x+=MAP_X;
            map_y+=MAP_Y;
        }

        QPoint pt(map_x, map_y);
        QPoint ptleft(map_x-5, map_y);
        QPoint ptright(map_x+5, map_y);
        QPoint ptup(map_x, map_y-5);
        QPoint ptdown(map_x, map_y+5);

        paint->setPen(qRgb(255, 0, 0));
        paint->drawLine(ptleft,ptright);
        paint->drawLine(ptup,ptdown);

//        QPoint pt(map_x-1, map_y-1);
//        paint->setPen(qRgb(255, 0, 0));
//        paint->drawEllipse(pt, 2, 2);
    }


    if(opts.gshhsglobe1On)
    {
        for (int i=0; i<gshhsdata->vxp_data_overlay[0]->nFeatures; i++)
        {
            for (int j=0; j<gshhsdata->vxp_data_overlay[0]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhsdata->vxp_data_overlay[0]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhsdata->vxp_data_overlay[0]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                if (opts.currenttoolbox == 0)       //LCC
                    bret = imageptrs->lcc->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y);
                else if (opts.currenttoolbox == 1)  //GVP
                    bret = imageptrs->gvp->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y);
                else                                //SG
                    bret = imageptrs->sg->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y);

                if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
                {
                    map_x+=MAP_X;
                    map_y+=MAP_Y;
                }

               if(bret)
                {
                    if (first)
                    {
                        first = false;
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                    else
                    {
                        paint->setPen(QColor(opts.projectionoverlaycolor1));
                        paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                }
                else
                    first = true;
            }
            first = true;
        }
    }

    first = true;

    if(opts.gshhsglobe2On)
    {
        for (int i=0; i<gshhsdata->vxp_data_overlay[1]->nFeatures; i++)
        {
            for (int j=0; j<gshhsdata->vxp_data_overlay[1]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhsdata->vxp_data_overlay[1]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhsdata->vxp_data_overlay[1]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                if (opts.currenttoolbox == 0)       //LCC
                    bret = imageptrs->lcc->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y);
                else if (opts.currenttoolbox == 1)  //GVP
                    bret = imageptrs->gvp->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;
                else                                //SG
                    bret = imageptrs->sg->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;

                if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
                {
                    map_x+=MAP_X;
                    map_y+=MAP_Y;
                }

                if(bret)
                {
                    if (first)
                    {
                        first = false;
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                    else
                    {
                        paint->setPen(QColor(opts.projectionoverlaycolor2));
                        paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                }
                else
                    first = true;
            }
            first = true;
        }
    }

    first = true;

    if(opts.gshhsglobe3On)
    {
        for (int i=0; i<gshhsdata->vxp_data_overlay[2]->nFeatures; i++)
        {
            for (int j=0; j<gshhsdata->vxp_data_overlay[2]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhsdata->vxp_data_overlay[2]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhsdata->vxp_data_overlay[2]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                if (opts.currenttoolbox == 0)       //LCC
                    bret = imageptrs->lcc->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y);
                else if (opts.currenttoolbox == 1)  //GVP
                    bret = imageptrs->gvp->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;
                else                                //SG
                    bret = imageptrs->sg->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;

                if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
                {
                    map_x+=MAP_X;
                    map_y+=MAP_Y;
                }

                if(bret)
                {
                    if (first)
                    {
                        first = false;
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                    else
                    {
                        paint->setPen(QColor(opts.projectionoverlaycolor3));
                        paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                }
                else
                    first = true;
            }
            first = true;
        }
    }

    if (opts.currenttoolbox == 0 && formtoolbox->GridOnProjLCC()) // LLC
    {
        for(double lon = -180.0; lon < 180.0; lon+=10.0)
        {
            first = true;

            //if(lon >= opts.mapextentwest && lon < opts.mapextenteast)
            {
                for(double lat = -90.0; lat < 90.0; lat+=0.5)
                {
                    bret = imageptrs->lcc->map_forward( lon*PI/180, lat*PI/180, map_x, map_y);

                    if(bret)
                    {
                        if (first)
                        {
                            first = false;
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                        else
                        {
                            paint->setPen(QColor(opts.projectionoverlaylonlatcolor));
                            paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                    }
                    else
                        first = true;

                }
            }
        }

        for(double lat = -80.0; lat < 81.0; lat+=10.0)
        {
            first = true;
            if(lat >= opts.mapextentsouth && lat < opts.mapextentnorth)
            {
                for(double lon = -180.0; lon < 180.0; lon+=1.0)
                {
                    bret = imageptrs->lcc->map_forward( lon*PI/180, lat*PI/180, map_x, map_y);

                    if(bret)
                    {
                        if (first)
                        {
                            first = false;
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                        else
                        {
                            paint->setPen(QColor(opts.projectionoverlaylonlatcolor));
                            paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                    }
                    else
                        first = true;

                }
            }
        }
    }

    if (opts.currenttoolbox == 1 && formtoolbox->GridOnProjGVP()) //GVP
    {
        for(double lon = -180.0; lon < 180.0; lon+=10.0)
        {
            first = true;
            {
                for(double lat = -90.0; lat < 90.0; lat+=0.5)
                {
                    bret = imageptrs->gvp->map_forward( lon*PI/180, lat*PI/180, map_x, map_y);

                    if(bret)
                    {
                        if (first)
                        {
                            first = false;
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                        else
                        {
                            paint->setPen(QColor(opts.projectionoverlaylonlatcolor));
                            paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                    }
                    else
                        first = true;

                }
            }
        }

        for(double lat = -80.0; lat < 81.0; lat+=10.0)
        {
            first = true;
            {
                for(double lon = -180.0; lon < 180.0; lon+=1.0)
                {
                    bret = imageptrs->gvp->map_forward( lon*PI/180, lat*PI/180, map_x, map_y);

                    if(bret)
                    {
                        if (first)
                        {
                            first = false;
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                        else
                        {
                            paint->setPen(QColor(opts.projectionoverlaylonlatcolor));
                            paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                    }
                    else
                        first = true;

                }
            }
        }
    }


    if (opts.currenttoolbox == 2 && formtoolbox->GridOnProjSG() ) //SG
    {
        for(double lon = -180.0; lon < 180.0; lon+=10.0)
        {
            first = true;
            {
                for(double lat = -90.0; lat < 90.0; lat+=0.5)
                {
                    bret = imageptrs->sg->map_forward( lon*PI/180, lat*PI/180, map_x, map_y);

                    if(bret)
                    {
                        if (first)
                        {
                            first = false;
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                        else
                        {
                            paint->setPen(QColor(opts.projectionoverlaylonlatcolor));
                            paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                    }
                    else
                        first = true;

                }
            }
        }

        for(double lat = -80.0; lat < 81.0; lat+=10.0)
        {
            first = true;
            {
                for(double lon = -180.0; lon < 180.0; lon+=1.0)
                {
                    bret = imageptrs->sg->map_forward( lon*PI/180, lat*PI/180, map_x, map_y);

                    if(bret)
                    {
                        if (first)
                        {
                            first = false;
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                        else
                        {
                            paint->setPen(QColor(opts.projectionoverlaylonlatcolor));
                            paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                            save_map_x = map_x;
                            save_map_y = map_y;
                        }
                    }
                    else
                        first = true;

                }
            }
        }
    }

    qDebug() << QString("End FormImage::OverlayProjection(QPainter *paint, SegmentListGeostationary *sl)");


}

void FormImage::ToInfraColorProjection()
{
    QRgb *row;
    float btemp;

    float mintemp;
    float maxtemp;

    int height = imageptrs->ptrimageProjection->height();
    int width = imageptrs->ptrimageProjection->width();

    dockinfrascales->getMinMaxTemp(&mintemp, &maxtemp);

    qDebug() << QString("FormImage::ToInfraColorProjection() min temp = %1 max temp = %2").arg(mintemp).arg(maxtemp);

    float delta = maxtemp - mintemp;

    float min = 9999999.0;
    float max = 0.0;
    int valcount = 0;

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            if(imageptrs->ptrProjectionBrightnessTemp.isNull())
                return;
            btemp = imageptrs->ptrProjectionBrightnessTemp[y * width + x];
            float fval = (btemp - mintemp)/delta;
            int val = qRound(fval*255.0);
            if(btemp > 0)
            {
                if(btemp>=max)
                    max=btemp;
                if(btemp<=min)
                    min=btemp;
            }

        }
    }

    delta = max - min;

    qDebug() << QString("----> ToInfraColorProjection() min = %1 max = %2 valcount = %3").arg(min).arg(max).arg(valcount);


    for(int y = 0; y < height; y++)
    {
        row = (QRgb*)imageptrs->ptrimageProjection->scanLine(y);

        for(int x = 0; x < width; x++)
        {
            //quint8 greyval = imageptrs->ptrProjectionInfra[y * width + x];

            if(imageptrs->ptrProjectionBrightnessTemp.isNull())
                return;
            btemp = imageptrs->ptrProjectionBrightnessTemp[y * width + x];
            float fval = (btemp - min)/delta;
            if(btemp > 0)
            {
                row[x] = dockinfrascales->getColor(fval).rgb();
            }

        }
    }

    changeinfraprojection = true;
}

void FormImage::FromInfraColorProjection()
{
    QRgb *row;
    float temp;
    int height = imageptrs->ptrimageProjection->height();
    int width = imageptrs->ptrimageProjection->width();

    for(int y = 0; y < height; y++)
    {
        row = (QRgb*)imageptrs->ptrimageProjection->scanLine(y);

        for(int x = 0; x < width; x++)
        {
            quint8 val = imageptrs->ptrProjectionInfra[y * width + x];
            row[x] = qRgb(val, val, val);
        }
    }

    changeinfraprojection = true;

}


void FormImage::test()
{

    pixgeoConversion pixconv;


    int col;
    int row;

    double radius;
    QVector3D pos;
    double lat_rad;
    double lon_rad;
    QRgb rgbval = qRgb(0, 255, 0);


    pos.setX(4024.915);
    pos.setY(304.966);
    pos.setZ(4921.832);

    long count = 0;

    for (int i=0; i<gshhsdata->vxp_data[0]->nFeatures; i++)
    {
        for (int j=0; j<gshhsdata->vxp_data[0]->pFeatures[i].nVerts; j++)
        {
            count++;

            pos.setX(gshhsdata->vxp_data[0]->pFeatures[i].pVerts[j].x());
            pos.setY(gshhsdata->vxp_data[0]->pFeatures[i].pVerts[j].y());
            pos.setZ(gshhsdata->vxp_data[0]->pFeatures[i].pVerts[j].z());
            //Pos2LatLonAlt(&lat_rad, &lon_rad, &radius, pos);
            qDebug() << QString("count %1 lat %2 lon %3").arg(count).arg(gshhsdata->vxp_data[0]->pFeatures[i].pLonLat[j].latmicro*1.0e-6).arg(gshhsdata->vxp_data[0]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6);


//            int ret = pixconv.geocoord2pixcoord(50.833, 4.333,coff,loff, &col, &row);

//            int ret = pixconv.geocoord2pixcoord(rad2deg(lat_rad), rad2deg(lon_rad),coff,loff, &col, &row);
            //qDebug() << QString("ret = %1 lat = %2 lon = %3 col = %4 row = %5").arg(ret).arg(lat_rad).arg(lon_rad).arg(col).arg(row);

            //if(ret == 0)
            {
                // this->imageLabel->setp
                //imageptrs->ptrimageMeteosat->setPixel(col, row, value); // QRgb(0, 0, 0) );


            }

        }
    }

}

void FormImage::slotRefreshOverlay()
{
    this->displayImage(this->channelshown);
}

void FormImage::slotRepaintProjectionImage()
{
    changeinfraprojection = true;
    this->displayImage(this->channelshown);
}

FormImage::~FormImage()
{

}

MyImageLabel::MyImageLabel(QLabel *parent ) :  QLabel(parent)
{
    //setMouseTracking(true);
}

void MyImageLabel::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << QString("myimagelabel mousemoveevent pos.x = %1 pos.y = %2").arg(event->pos().x()).arg(event->pos().y());

    QLabel::mouseMoveEvent(event);
}
