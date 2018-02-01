#include "formimage.h"
#include "segmentimage.h"
#include "options.h"
#include "gshhsdata.h"
#include "pixgeoconversion.h"

#include <qtconcurrentrun.h>

extern Options opts;
extern SegmentImage *imageptrs;
extern gshhsData *gshhsdata;
extern bool ptrimagebusy;

#include <QMutex>
#include <QDebug>

#define MAP_X 3
#define MAP_Y 3

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
    zoomValueolci = opts.zoomfactorolci;
    zoomValueslstr = opts.zoomfactorslstr;

    scaleFactor = (double)getZoomValue()/100;
    qDebug() << QString("FormImage::FormImage scalefactor = %1").arg(scaleFactor);
    imageLabel = new ImageLabel(this, segs);
//    imageLabel = new AspectRatioPixmapLabel;
    imageLabel->setFormImagePtr(this);
    imageLabel->setScaledContents(true);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(imageLabel);
    this->setLayout(mainLayout);

    overlaymeteosat = true;
    overlayprojection = true;
    overlayolci = true;
    refreshoverlay = true;
    changeinfraprojection = false;

    this->setSegmentType(SEG_NONE);

    metopcount = 0;
    noaacount = 0;
    gaccount = 0;
    hrpcount = 0;

    metopAhrptcount = 0;
    metopBhrptcount = 0;
    noaa19hrptcount = 0;
    M01hrptcount = 0;
    M02hrptcount = 0;

    viirsmcount = 0;
    viirsdnbcount = 0;
    olciefrcount = 0;
    olcierrcount = 0;
    slstrcount = 0;
    txtInfo = "";

    this->currentgeooverlay = 0;
    this->setupGeoOverlay(0);
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

bool FormImage::toggleOverlayOLCI()
{
    displayImage(channelshown);
    if(overlayolci)
        overlayolci = false;
    else
        overlayolci = true;
    this->update();
    return overlayolci;
}

bool FormImage::toggleOverlayGridOnOLCI()
{
    displayImage(channelshown);
    this->update();
    return overlayolci;
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

    qDebug() << "FormImage::setPixmapToLabel(bool settoolboxbuttons) width = " << imageptrs->ptrimageSLSTR->size().width() << " height = "
             << imageptrs->ptrimageSLSTR->size().height() << " channelshown = " << channelshown;

    refreshoverlay = true;

    emit allsegmentsreceivedbuttons(settoolboxbuttons);

    switch(channelshown)
    {
    case IMAGE_AVHRR_CH1:
        displayAVHRRImageInfo();
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[0]) ));
        break;
    case IMAGE_AVHRR_CH2:
        displayAVHRRImageInfo();
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[1]) ));
        break;
    case IMAGE_AVHRR_CH3:
        displayAVHRRImageInfo();
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[2]) ));
        break;
    case IMAGE_AVHRR_CH4:
        displayAVHRRImageInfo();
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[3]) ));
        break;
    case IMAGE_AVHRR_CH5:
        displayAVHRRImageInfo();
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[4]) ));
        break;
    case IMAGE_AVHRR_COL:
        displayAVHRRImageInfo();
        imageLabel->setPixmap(QPixmap::fromImage(*(imageptrs->ptrimagecomp_col)));
        break;
    case IMAGE_AVHRR_EXPAND:
        displayAVHRRImageInfo();
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrexpand_col)));
        break;
    case IMAGE_GEOSTATIONARY:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageGeostationary)));
        break;
    case IMAGE_PROJECTION:
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageProjection)));
        break;
    case IMAGE_VIIRSM:
        displayVIIRSImageInfo(SEG_VIIRSM);
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageViirsM)));
        break;
    case IMAGE_VIIRSDNB:
        displayVIIRSImageInfo(SEG_VIIRSDNB);
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageViirsDNB)));
        break;
    case IMAGE_OLCI:
        displaySentinelImageInfo(SEG_OLCIEFR);
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageOLCI)));
        break;
    case IMAGE_SLSTR:
        displaySentinelImageInfo(SEG_SLSTR);
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageSLSTR)));
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
    qDebug() << QString("FormImage ptrimageolci bytecount = %1").arg(imageptrs->ptrimageOLCI->byteCount());
    qDebug() << QString("FormImage ptrimageslstr bytecount = %1").arg(imageptrs->ptrimageSLSTR->byteCount());

    this->channelshown = channel;

    QPixmap pm(800, 200);
    pm.fill(Qt::red);
    QPainter painter(&pm);

    QFont f("Courier", 40, QFont::Bold);
    painter.setFont(f);
    painter.setPen(Qt::yellow);

    if(ptrimagebusy)
    {
        QPixmap pm(800, 200);
        pm.fill(Qt::red);
        QPainter painter(&pm);

        QFont f("Courier", 40, QFont::Bold);
        painter.setFont(f);
        painter.setPen(Qt::yellow);

        painter.drawText(10, 100, "Calculating image busy");

        imageLabel->setPixmap(pm);
    }
    else
    {
        switch(channelshown)
        {
        case IMAGE_AVHRR_CH1:
            if(imageptrs->ptrimagecomp_ch[0]->byteCount() == 0)
            {
                painter.drawText(10, 100, "No AVHRR image");
                imageLabel->setPixmap(pm);
            }
            else
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[0]) ));
            break;
        case IMAGE_AVHRR_CH2:
            if(imageptrs->ptrimagecomp_ch[1]->byteCount() == 0)
            {
                painter.drawText(10, 100, "No AVHRR image");
                imageLabel->setPixmap(pm);
            }
            else
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[1]) ));
            break;
        case IMAGE_AVHRR_CH3:
            if(imageptrs->ptrimagecomp_ch[2]->byteCount() == 0)
            {
                painter.drawText(10, 100, "No AVHRR image");
                imageLabel->setPixmap(pm);
            }
            else
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[2]) ));
            break;
        case IMAGE_AVHRR_CH4:
            if(imageptrs->ptrimagecomp_ch[3]->byteCount() == 0)
            {
                painter.drawText(10, 100, "No AVHRR image");
                imageLabel->setPixmap(pm);
            }
            else
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[3]) ));
            break;
        case IMAGE_AVHRR_CH5:
            if(imageptrs->ptrimagecomp_ch[4]->byteCount() == 0)
            {
                painter.drawText(10, 100, "No AVHRR image");
                imageLabel->setPixmap(pm);
            }
            else
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_ch[4]) ));
            break;
        case IMAGE_AVHRR_COL:
            if(imageptrs->ptrimagecomp_col->byteCount() == 0)
            {
                painter.drawText(10, 100, "No AVHRR image");
                imageLabel->setPixmap(pm);
            }
            else
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimagecomp_col)));
            break;
        case IMAGE_AVHRR_EXPAND:
            if(imageptrs->ptrexpand_col->byteCount() == 0)
            {
                painter.drawText(10, 100, "No AVHRR image");
                imageLabel->setPixmap(pm);
            }
            else
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrexpand_col)));
            break;
        case IMAGE_GEOSTATIONARY:
            if(imageptrs->ptrimageGeostationary->byteCount() == 0)
            {
                painter.drawText(10, 100, "No geostationary image");
                imageLabel->setPixmap(pm);
            }
            else
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageGeostationary)));
            break;
        case IMAGE_PROJECTION:
            if(imageptrs->ptrimageProjection->byteCount() == 0)
            {
                painter.drawText(10, 100, "No Projection image");
                imageLabel->setPixmap(pm);
            }
            else
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageProjection)));
            break;
        case IMAGE_VIIRSM:
            if(imageptrs->ptrimageViirsM->byteCount() == 0)
            {
                painter.drawText(10, 100, "No VIIRS M image");
                imageLabel->setPixmap(pm);
            }
            else
            {
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageViirsM)));
                displayVIIRSImageInfo(SEG_VIIRSM);
            }
            break;
        case IMAGE_VIIRSDNB:
            if(imageptrs->ptrimageViirsDNB->byteCount() == 0)
            {
                painter.drawText(10, 100, "No VIIRS DNB image");
                imageLabel->setPixmap(pm);
            }
            else
            {
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageViirsDNB)));
                displayVIIRSImageInfo(SEG_VIIRSDNB);
            }
            break;
        case IMAGE_OLCI:
            if(imageptrs->ptrimageOLCI->byteCount() == 0)
            {
                painter.drawText(10, 100, "No OLCI image");
                imageLabel->setPixmap(pm);
            }
            else
            {
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageOLCI)));
                this->displaySentinelImageInfo(imageptrs->olcitype);
            }
            break;
        case IMAGE_SLSTR:
            if(imageptrs->ptrimageSLSTR->byteCount() == 0)
            {
                painter.drawText(10, 100, "No SLSTR image");
                imageLabel->setPixmap(pm);
            }
            else
            {
                imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageSLSTR)));
                this->displaySentinelImageInfo(SEG_SLSTR);
            }
            break;

        }
    }
    refreshoverlay = true;

    this->update();
    this->adjustImage();


    qDebug() << QString("after FormImage::displayImage(eImageType channel) channel = %1").arg(channel);

}

void FormImage::slotMakeImage()
{
    this->MakeImage();
}

void FormImage::MakeImage()
{

    if(opts.buttonMetop || opts.buttonNoaa || opts.buttonHRP || opts.buttonGAC)
    {
        metopcount = segs->seglmetop->NbrOfSegmentsSelected();
        noaacount = segs->seglnoaa->NbrOfSegmentsSelected();
        hrpcount = segs->seglhrp->NbrOfSegmentsSelected();
        gaccount = segs->seglgac->NbrOfSegmentsSelected();
    }
    else if(opts.buttonMetopAhrpt || opts.buttonMetopBhrpt || opts.buttonNoaa19hrpt || opts.buttonM01hrpt || opts.buttonM02hrpt)
    {
        metopAhrptcount = segs->seglmetopAhrpt->NbrOfSegmentsSelected();
        metopBhrptcount = segs->seglmetopBhrpt->NbrOfSegmentsSelected();
        noaa19hrptcount = segs->seglnoaa19hrpt->NbrOfSegmentsSelected();
        M01hrptcount = segs->seglM01hrpt->NbrOfSegmentsSelected();
        M02hrptcount = segs->seglM02hrpt->NbrOfSegmentsSelected();
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
    else if(opts.buttonSLSTR)
    {
        slstrcount = segs->seglslstr->NbrOfSegmentsSelected();
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

    qDebug() << QString("in FormImage::ComposeImage nbr of metopAhrpt segments selected = %1").arg(metopAhrptcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of metopBhrpt segments selected = %1").arg(metopBhrptcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of noaa19hrpt segments selected = %1").arg(noaa19hrptcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of M01hrpt segments selected = %1").arg(M01hrptcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of M02hrpt segments selected = %1").arg(M02hrptcount);

    qDebug() << QString("in FormImage::ComposeImage nbr of viirsm segments selected = %1").arg(viirsmcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of viirsdnb segments selected = %1").arg(viirsdnbcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of olciefr segments selected = %1").arg(olciefrcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of olcierr segments selected = %1").arg(olcierrcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of slstr segments selected = %1").arg(slstrcount);

    if(opts.buttonMetop || opts.buttonNoaa || opts.buttonGAC || opts.buttonHRP ||
            opts.buttonMetopAhrpt || opts.buttonMetopBhrpt || opts.buttonNoaa19hrpt || opts.buttonM01hrpt || opts.buttonM02hrpt )
    {
        if (metopcount + noaacount + hrpcount + gaccount + metopAhrptcount + metopBhrptcount + noaa19hrptcount + M01hrptcount + M02hrptcount > 0)
        {
            if (metopcount > 0 && opts.buttonMetop)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_METOP);
                segs->seglmetop->ComposeAVHRRImage();
            }
            else if (noaacount > 0 && opts.buttonNoaa)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_NOAA);
                segs->seglnoaa->ComposeAVHRRImage();
            }
            else if (hrpcount > 0 && opts.buttonHRP)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_HRP);
                segs->seglhrp->ComposeAVHRRImage();
            }
            else if (gaccount > 0 && opts.buttonGAC)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_GAC);
                segs->seglgac->ComposeAVHRRImage();
            }
            else if (metopAhrptcount > 0 && opts.buttonMetopAhrpt)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_HRPT_METOPA);
                segs->seglmetopAhrpt->ComposeAVHRRImage();
            }
            else if (metopBhrptcount > 0 && opts.buttonMetopBhrpt)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_HRPT_METOPB);
                segs->seglmetopBhrpt->ComposeAVHRRImage();
            }
            else if (noaa19hrptcount > 0 && opts.buttonNoaa19hrpt)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_HRPT_NOAA19);
                segs->seglnoaa19hrpt->ComposeAVHRRImage();
            }
            else if (M01hrptcount > 0 && opts.buttonM01hrpt)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_HRPT_M01);
                segs->seglM01hrpt->ComposeAVHRRImage();
            }
            else if (M02hrptcount > 0 && opts.buttonM02hrpt)
            {
                formtoolbox->setToolboxButtons(false);

                this->kindofimage = "AVHRR Color";
                this->setSegmentType(SEG_HRPT_M02);
                segs->seglM02hrpt->ComposeAVHRRImage();
            }

        }
        else
            return;
    }
    else if(viirsmcount > 0 && opts.buttonVIIRSM)
    {
        if(!formtoolbox->comboColVIIRSOK())
        {
            QMessageBox msgBox;
            msgBox.setText("Need color choices for 3 different bands in the VIIRS tab.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            int ret = msgBox.exec();

            switch (ret) {
            case QMessageBox::Ok:
                break;
            default:
                break;
            }

            return;
        }

        formtoolbox->setToolboxButtons(false);

        this->displayImage(IMAGE_VIIRSM);
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


        this->displayImage(IMAGE_VIIRSDNB);
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
        if(!formtoolbox->comboColOLCIOK())
        {
            QMessageBox msgBox;
            msgBox.setText("Need color choices for 3 different bands in the OLCI tab.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            int ret = msgBox.exec();

            switch (ret) {
            case QMessageBox::Ok:
                break;
            default:
                break;
            }

            return;
        }

        formtoolbox->setToolboxButtons(false);

        imageptrs->olcitype = SEG_OLCIEFR;
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
        if(!formtoolbox->comboColOLCIOK())
        {
            QMessageBox msgBox;
            msgBox.setText("Need color choices for 3 different bands in the OLCI tab.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            int ret = msgBox.exec();

            switch (ret) {
            case QMessageBox::Ok:
                break;
            default:
                break;
            }

            return;
        }

        formtoolbox->setToolboxButtons(false);

        imageptrs->olcitype = SEG_OLCIERR;
        this->displayImage(IMAGE_OLCI);
        this->kindofimage = "OLCIERR";
        this->setSegmentType(SEG_OLCIERR);

        bandlist = formtoolbox->getOLCIBandList();
        colorlist = formtoolbox->getOLCIColorList();
        invertlist = formtoolbox->getOLCIInvertList();

        //          in Workerthread
        segs->seglolcierr->ComposeOLCIImage(bandlist, colorlist, invertlist, true);
    }
    else if(slstrcount > 0 && opts.buttonSLSTR)
    {
        if(!formtoolbox->comboColSLSTROK())
        {
            QMessageBox msgBox;
            msgBox.setText("Need color choices for 3 different bands in the SLSTR tab.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            int ret = msgBox.exec();

            switch (ret) {
            case QMessageBox::Ok:
                break;
            default:
                break;
            }

            return;
        }

        formtoolbox->setToolboxButtons(false);

        this->displayImage(IMAGE_SLSTR);
        this->kindofimage = "SLSTR";
        this->setSegmentType(SEG_SLSTR);

        bandlist = formtoolbox->getSLSTRBandList();
        colorlist = formtoolbox->getSLSTRColorList();
        invertlist = formtoolbox->getSLSTRInvertList();
        eSLSTRImageView slstrimageview = formtoolbox->getSLSTRImageView();

        //          in Workerthread
        segs->seglslstr->ComposeSLSTRImage(bandlist, colorlist, invertlist, true, slstrimageview);
    }
    else
        return;
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
        displayImage(IMAGE_VIIRSM);

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


bool FormImage::ShowOLCIefrImage(int histogrammethod, bool normalized)
{
    bool ret = false;

    olciefrcount = segs->seglolciefr->NbrOfSegmentsSelected();

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    qDebug() << QString("in FormImage::ShowOLCIefrImage nbr of olci efr segments selected = %1").arg(olciefrcount);

    if (olciefrcount > 0)
    {

//        ret = true;
//        displayImage(IMAGE_OLCI);

//        emit allsegmentsreceivedbuttons(false);

        this->kindofimage = "OLCIEFR";

        bandlist = formtoolbox->getOLCIBandList();
        colorlist = formtoolbox->getOLCIColorList();
        invertlist = formtoolbox->getOLCIInvertList();
        segs->seglolciefr->setHistogramMethod(histogrammethod, normalized);
        segs->seglolciefr->ComposeOLCIImage(bandlist, colorlist, invertlist, false); // parameter false = no decompression of the files
    }
    else
        ret = false;

    return ret;

}


bool FormImage::ShowOLCIerrImage(int histogrammethod, bool normalized)
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
        segs->seglolcierr->setHistogramMethod(histogrammethod, normalized);
        segs->seglolcierr->ComposeOLCIImage(bandlist, colorlist, invertlist, false);
    }
    else
        ret = false;

    return ret;

}

bool FormImage::ShowSLSTRImage(int histogrammethod)
{
    bool ret = false;

    slstrcount = segs->seglslstr->NbrOfSegmentsSelected();

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    qDebug() << QString("in FormImage::ShowSLSTRImage nbr of slstr segments selected = %1").arg(slstrcount);

    if (slstrcount > 0)
    {

        ret = true;
        displayImage(IMAGE_SLSTR);

        emit allsegmentsreceivedbuttons(false);

        this->kindofimage = "SLSTR";

        bandlist = formtoolbox->getSLSTRBandList();
        colorlist = formtoolbox->getSLSTRColorList();
        invertlist = formtoolbox->getSLSTRInvertList();
        segs->seglslstr->setHistogramMethod(histogrammethod);
        segs->seglslstr->ComposeSLSTRImage(bandlist, colorlist, invertlist, false, formtoolbox->getSLSTRImageView());
    }
    else
        ret = false;

    return ret;

}

bool FormImage::ShowHistogramImageOLCI(int histogrammethod, bool normalized)
{
    int olciefrcount = segs->seglolciefr->NbrOfSegmentsSelected();
    int olcierrcount = segs->seglolcierr->NbrOfSegmentsSelected();

    if(olciefrcount > 0)
    {
        segs->seglolciefr->setHistogramMethod(histogrammethod, normalized);
        segs->seglolciefr->ChangeHistogramMethod();
        return true;
    }
    else if(olcierrcount > 0)
    {
        segs->seglolcierr->setHistogramMethod(histogrammethod, normalized);
        segs->seglolcierr->ChangeHistogramMethod();
        return true;
    }
    return false;
}

bool FormImage::ShowHistogramImageSLSTR(int histogrammethod)
{
    int slstrcount = segs->seglslstr->NbrOfSegmentsSelected();

    if(slstrcount > 0)
    {
        segs->seglslstr->setHistogramMethod(histogrammethod);
        segs->seglslstr->ChangeHistogramMethod();
        return true;
    }
    else
        return false;
}

bool FormImage::ShowVIIRSDNBImage()
{
    bool ret = false;

    viirsdnbcount = segs->seglviirsdnb->NbrOfSegmentsSelected();

    qDebug() << QString("in FormImage::ShowVIIRSDNBImage nbr of viirs segments selected = %1").arg(viirsdnbcount);
    if (viirsdnbcount > 0)
    {
        ret = true;
        displayImage(IMAGE_VIIRSDNB);

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
    case IMAGE_AVHRR_CH1:
    case IMAGE_AVHRR_CH2:
    case IMAGE_AVHRR_CH3:
    case IMAGE_AVHRR_CH4:
    case IMAGE_AVHRR_CH5:
    case IMAGE_AVHRR_COL:
    case IMAGE_AVHRR_EXPAND:
        zoomValueavhrr = z;
        opts.zoomfactoravhrr = z;
        break;
    case IMAGE_GEOSTATIONARY:
        zoomValuemeteosat = z;
        opts.zoomfactormeteosat = z;
        break;
    case IMAGE_PROJECTION:
        zoomValueprojection = z;
        opts.zoomfactorprojection = z;
        break;
    case IMAGE_VIIRSM:
    case IMAGE_VIIRSDNB:
        zoomValueviirs = z;
        opts.zoomfactorviirs = z;
        break;
    case IMAGE_OLCI:
        zoomValueolci = z;
        opts.zoomfactorolci = z;
        break;
    case IMAGE_SLSTR:
        zoomValueslstr = z;
        opts.zoomfactorslstr = z;
        break;

    }
}

int FormImage::getZoomValue()
{
    int zoomValue;
    switch(channelshown)
    {
    case IMAGE_AVHRR_CH1:
    case IMAGE_AVHRR_CH2:
    case IMAGE_AVHRR_CH3:
    case IMAGE_AVHRR_CH4:
    case IMAGE_AVHRR_CH5:
    case IMAGE_AVHRR_COL:
    case IMAGE_AVHRR_EXPAND:
        zoomValue = zoomValueavhrr;
        break;
    case IMAGE_GEOSTATIONARY:
        zoomValue = zoomValuemeteosat;
        break;
    case IMAGE_PROJECTION:
        zoomValue = zoomValueprojection;
        break;
    case IMAGE_VIIRSM:
    case IMAGE_VIIRSDNB:
        zoomValue = zoomValueviirs;
        break;
    case IMAGE_OLCI:
        zoomValue = zoomValueolci;
        break;
    case IMAGE_SLSTR:
        zoomValue = zoomValueslstr;
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


    SegmentListGeostationary *slgeo = NULL;

    slgeo = segs->getActiveSegmentList();

    if(channelshown == IMAGE_GEOSTATIONARY)
        displayGeoImageInfo();


    if (channelshown == IMAGE_GEOSTATIONARY && overlaymeteosat && refreshoverlay)
    {
        if(slgeo != NULL)
        {
            this->OverlayGeostationary(&painter, slgeo);
            refreshoverlay = false;
        }
    }
    if(channelshown == IMAGE_PROJECTION && overlayprojection && refreshoverlay)
    {
        this->OverlayProjection(&painter);
        refreshoverlay = false;
    }

    if(channelshown == IMAGE_OLCI && overlayolci && refreshoverlay)
    {
        this->OverlayOLCI(&painter);
        refreshoverlay = false;
    }

    if(channelshown == IMAGE_PROJECTION && changeinfraprojection)
    {
        changeinfraprojection = false;
    }

    this->adjustImage();

    //qDebug() << "FormImage::paintEvent( QPaintEvent * )";

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
    case SEG_HRPT_METOPA:
        segtype = "Metop A HRPT";
        break;
    case SEG_HRPT_METOPB:
        segtype = "Metop B HRPT";
        break;
    case SEG_HRPT_NOAA19:
        segtype = "Noaa 19 HRPT";
        break;
    case SEG_HRPT_M01:
        segtype = "Metop B HRPT";
        break;
    case SEG_HRPT_M02:
        segtype = "Metop A HRPT";
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
    else if( type == SEG_HRPT_METOPA)
        nbrselected = segs->seglmetopAhrpt->NbrOfSegmentsSelected();
    else if( type == SEG_HRPT_METOPB)
        nbrselected = segs->seglmetopBhrpt->NbrOfSegmentsSelected();
    else if( type == SEG_HRPT_NOAA19)
        nbrselected = segs->seglnoaa19hrpt->NbrOfSegmentsSelected();
    else if( type == SEG_HRPT_M01)
        nbrselected = segs->seglM01hrpt->NbrOfSegmentsSelected();
    else if( type == SEG_HRPT_M02)
        nbrselected = segs->seglM02hrpt->NbrOfSegmentsSelected();

    txtInfo = QString("<!DOCTYPE html>"
                      "<html><head><title>Info</title></head>"
                      "<body>"
                      "<h3 style='color:blue'>Image Information</h3>"
                      "<p>Segment type = %1<br>"
                      "Nbr of segments = %2<br>"
                      "Image width = %3 height = %4<br>"
                      "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimagecomp_col->width()).arg(imageptrs->ptrimagecomp_col->height());


    formtoolbox->writeInfoToTextEdit(txtInfo);

}

void FormImage::displayVIIRSImageInfo(eSegmentType type)
{
    QString segtype;
    int nbrselected;
    float moonillum;

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
        formtoolbox->writeInfoToTextEdit(txtInfo);

    } else
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
        formtoolbox->writeInfoToTextEdit(txtInfo);

    }


}

void FormImage::displayGeoImageInfo()
{
    for(int i = 0; i < opts.geosatellites.length(); i++)
    {
        if(segs->seglgeo[i]->bActiveSegmentList == true)
        {
            displayGeoImageInformation(opts.geosatellites[i].fullname);
        }
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


    formtoolbox->writeInfoToTextEdit(txtInfo);
}

void FormImage::displaySentinelImageInfo(eSegmentType type)
{
    QString segtype;
    int nbrselected;
    long nbrofsaturatedpixels;

    switch(type)
    {
    case SEG_NONE:
        segtype = "None";
        break;
    case SEG_OLCIEFR:
        segtype = "OLCI efr";
        nbrselected = segs->seglolciefr->NbrOfSegmentsSelected();
        nbrofsaturatedpixels = segs->seglolciefr->NbrOfSaturatedPixels();
        break;
    case SEG_OLCIERR:
        segtype = "OLCI err";
        nbrselected = segs->seglolcierr->NbrOfSegmentsSelected();
        nbrofsaturatedpixels = segs->seglolcierr->NbrOfSaturatedPixels();
        break;
    case SEG_SLSTR:
        segtype = "SLSTR";
        nbrselected = segs->seglslstr->NbrOfSegmentsSelected();
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
                          "Nbr of saturated pixels = %5<br>"
                          "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimageOLCI->width()).arg(imageptrs->ptrimageOLCI->height()).arg(nbrofsaturatedpixels);
        formtoolbox->writeInfoToTextEdit(txtInfo);
    }
    else if(type == SEG_SLSTR)
    {
        txtInfo = QString("<!DOCTYPE html>"
                          "<html><head><title>Info</title></head>"
                          "<body>"
                          "<h3 style='color:blue'>Image Information</h3>"
                          "<p>Segment type = %1<br>"
                          "Nbr of segments = %2<br>"
                          "Image width = %3 height = %4<br>"
                          "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimageSLSTR->width()).arg(imageptrs->ptrimageSLSTR->height());
        formtoolbox->writeInfoToTextEdit(txtInfo);
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
    else if(channelshown == IMAGE_VIIRSM)
    {
       w=imageptrs->ptrimageViirsM->width();
       h=imageptrs->ptrimageViirsM->height();
    }
    else if(channelshown == IMAGE_VIIRSDNB)
    {
       w=imageptrs->ptrimageViirsDNB->width();
       h=imageptrs->ptrimageViirsDNB->height();
    }
    else if(channelshown == IMAGE_OLCI)
    {
       w=imageptrs->ptrimageOLCI->width();
       h=imageptrs->ptrimageOLCI->height();
    }
    else if(channelshown == IMAGE_SLSTR)
    {
       w=imageptrs->ptrimageSLSTR->width();
       h=imageptrs->ptrimageSLSTR->height();
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
        imageLabel->resize(imageLabel->pixmap()->size());
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

void FormImage::slotUpdateGeosat()
{

    qDebug() << "start FormImage::slotUpdateGeosat()";

    refreshoverlay = true;

    imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageGeostationary)));
    this->adjustImage();
    emit allsegmentsreceivedbuttons(true);

    this->update();

}

void FormImage::slotcomposefinished(QString kindofimage)
{
    imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageGeostationary)));
    refreshoverlay = true;

    SegmentListGeostationary *sl = NULL;
    sl = segs->getActiveSegmentList();


    if(sl->getGeoSatellite() == eGeoSatellite::H8)
    {
        EnhanceDarkSpace(sl->getGeoSatelliteIndex());
        imageLabel->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageGeostationary)));
        refreshoverlay = true;
    }

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
            emit render3dgeo(sl->getGeoSatelliteIndex());
        }
    }
    else
        emit allsegmentsreceivedbuttons(true);

    QApplication::restoreOverrideCursor();
    qDebug() << "end FormImage::slotcomposefinished()";
    this->update();

}

void FormImage::EnhanceDarkSpace(int geoindex)
{
    QRgb *row_col;
    QRgb rgb;
    quint16 r,g, b;
    double l2, el2_1, el2_2;
    int x0, y0;
    int maxhimred, minhimred;
    int maxhimgreen, minhimgreen;
    int maxhimblue, minhimblue;
    long cnt = 0;

    maxhimred = 0;
    minhimred = 65535;
    maxhimgreen = 0;
    minhimgreen = 65535;
    maxhimblue = 0;
    minhimblue = 65535;

    int ela = opts.geosatellites.at(geoindex).coff - 28; //- 28;
    int elb = opts.geosatellites.at(geoindex).loff - 40; //- 40;
    double ela2 = (double)(ela*ela);
    double elb2 = (double)(elb*elb);
    double eta1 = 1.003;
    double eta2 = 1.005;
    double ka1, ka2;

    for (int line = 0; line < 5500; line++)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);


//        for (int pixelx = 0; pixelx < 5500; pixelx++)
//        {
//            y0 = line - 2750;
//            x0 = pixelx - 2750;
//            rgb = row_col[pixelx];

//                    double x2 = (double)(x0*x0);
//                    double y2 = (double)(y0*y0);

////                    ka = (eta - x2/ela2);
////                    el2 = ka * elb2;

////                    if(y2 < el2)
////                        row_col[pixelx] = qRgb(255, 0, 0);


//                    //eta = 1.1;
//                    ka = (eta - x2/ela2);
//                    el2 = ka * elb2;

//                    if(y2 > el2 && (y2 - 80000) <= el2)
//                    {
//                        row_col[pixelx] = qRgb(0, 255, 0);
//                        //calchimawari(rgb, minhimred, maxhimred, minhimgreen, maxhimgreen, minhimblue, maxhimblue);
//                    }
//                    //else if((y2 <= el2) && ((el2 - 900000) < y2))
//                    //    row_col[pixelx] = qRgb(0, 255, 0);

//        }


        for (int pixelx = 0; pixelx < 5500; pixelx++)
        {
            y0 = line - 2750;
            x0 = pixelx - 2750;
            rgb = row_col[pixelx];

            double x2 = (double)(x0*x0);
            double y2 = (double)(y0*y0);

            ka1 = (eta1 - x2/ela2);
            el2_1 = ka1 * elb2;


            ka2 = (eta2 - x2/ela2);
            el2_2 = ka2 * elb2;

            if(y2 > el2_1) //  || y2 > el2_2)
                calchimawari(rgb, minhimred, maxhimred, minhimgreen, maxhimgreen, minhimblue, maxhimblue);

        }

    }

    qDebug() << QString("Himawari min rgb = %1 %2 %3 max rgb = %4 %5 %6  cnt = %7").arg(minhimred).arg(minhimgreen).arg(minhimblue).arg(maxhimred).arg(maxhimgreen).arg(maxhimblue).arg(cnt);

    SetupContrastStretch(minhimred, 0, maxhimred, 255, minhimgreen, 0, maxhimgreen, 255, minhimblue, 0, maxhimblue, 255 );

    for (int line = 0; line < 5500; line++)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);


        for (int pixelx = 0; pixelx < 5500; pixelx++)
        {
            y0 = line - 2750;
            x0 = pixelx - 2750;
            rgb = row_col[pixelx];

            double x2 = (double)(x0*x0);
            double y2 = (double)(y0*y0);

            double ka2 = (eta2 - x2/ela2);
            el2_2 = ka2 * elb2;

            if(y2 > el2_2)
            {
                row_col[pixelx] = ContrastStretch(rgb);
            }
        }
    }


}

QRgb FormImage::ContrastStretch(QRgb val)
{
    double resred, resgreen, resblue;
    resred = double(qRed(val))*A1red + B1red;
    resgreen = double(qGreen(val))*A1green + B1green;
    resblue = double(qBlue(val))*A1blue + B1blue;
    resred = resred > 255.0 ? 255 : quint16(resred);
    resgreen = resgreen > 255.0 ? 255 : quint16(resgreen);
    resblue = resblue > 255.0 ? 255 : quint16(resblue);
    return qRgb(resred, resgreen, resblue);
}

void FormImage::SetupContrastStretch(quint16 x1r, quint16 y1r, quint16 x2r, quint16 y2r, quint16 x1g, quint16 y1g, quint16 x2g, quint16 y2g, quint16 x1b, quint16 y1b, quint16 x2b, quint16 y2b)
{
    double d_x1r = (double)x1r;
    double d_x2r = (double)x2r;
    double d_y1r = (double)y1r;
    double d_y2r = (double)y2r;

    double d_x1g = (double)x1g;
    double d_x2g = (double)x2g;
    double d_y1g = (double)y1g;
    double d_y2g = (double)y2g;

    double d_x1b = (double)x1b;
    double d_x2b = (double)x2b;
    double d_y1b = (double)y1b;
    double d_y2b = (double)y2b;

    A1red = (d_y2r - d_y1r)/(d_x2r - d_x1r);
    B1red = (d_y2r - (A1red*d_x2r));
    A1green = (d_y2g - d_y1g)/(d_x2g - d_x1g);
    B1green = (d_y2g - (A1green*d_x2g));
    A1blue = (d_y2b - d_y1b)/(d_x2b - d_x1b);
    B1blue = (d_y2b - (A1blue*d_x2b));
}


void FormImage::calchimawari(QRgb rgb, int &minred, int &maxred, int &mingreen, int &maxgreen, int &minblue, int &maxblue)
{
    quint16 r,g,b;

    r = qRed(rgb);
    g = qGreen(rgb);
    b = qBlue(rgb);

    if(minred > r)
        minred = r;
    if(maxred < r)
        maxred = r;
    if(mingreen > g)
        mingreen = g;
    if(maxgreen < g)
        maxgreen = g;
    if(minblue > b)
        minblue = b;
    if(maxblue < b)
        maxblue = b;

}

void FormImage::UpdateProjection()
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

void FormImage::recalculateCLAHEOLCI(QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    quint16 *pixelsRed;
    quint16 *pixelsGreen;
    quint16 *pixelsBlue;

    size_t npix;

    SegmentListOLCI *sl = segs->seglolciefr;
    if(sl->GetSegsSelectedptr()->count() > 0)
    {
        qDebug() << "segs->seglolci count selected = " << sl->GetSegsSelectedptr()->count();
        qDebug() << "height = " << imageptrs->ptrimageOLCI->height();
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

    sl = segs->getActiveSegmentList();
    if(sl == NULL)
        return;


    if (sl->getKindofImage() == "HRV Color")
        return;

    size_t npix;
    size_t npixHRV;
    if(sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_8)
    {
        npix = 3712*3712;
        if (sl->areatype == 1)
            npixHRV = 5568*11136;
        else
            npixHRV = 5568*5*464;
    }
    else if(sl->getGeoSatellite() == eGeoSatellite::MET_9)
    {
        npix = 3712*3*464;
        npixHRV = 5568*5*464;
    }
    else if(sl->getGeoSatellite() == eGeoSatellite::GOES_15)
    {
        npix = 2816*7*464;
        npixHRV = 0;
    }
    else if(sl->getGeoSatellite() == eGeoSatellite::GOES_16)
    {
        npix = 5424*5424;
        npixHRV = 0;
    }
    else if(sl->getGeoSatellite() == eGeoSatellite::GOMS2)
    {
        npix = 2784*6*464;
        npixHRV = 0;
    }
    else if(sl->getGeoSatellite() == eGeoSatellite::FY2E || sl->getGeoSatellite() == eGeoSatellite::FY2G)
    {
        npix = 2288*2288;
        npixHRV = 9152*9152;
    }
    else if(sl->getGeoSatellite() == eGeoSatellite::H8)
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

    if(sl->getKindofImage() == "VIS_IR Color" && (sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8 ))
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
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::GOMS2)
    {
        pixelsRed = new quint16[npix];
        pixelsGreen = new quint16[npix];
        pixelsBlue = new quint16[npix];

        for( int i = 0; i < 6; i++)
        {
            if(sl->isPresentRed[i])
                memcpy(pixelsRed + i * 464 * 2784, imageptrs->ptrRed[i], 464 * 2784 * sizeof(quint16));
        }
        for( int i = 0; i < 6; i++)
        {
            if(sl->isPresentGreen[i])
                memcpy(pixelsGreen + i * 464 * 2784, imageptrs->ptrGreen[i], 464 * 2784 * sizeof(quint16));
        }
        for( int i = 0; i < 6; i++)
        {
            if(sl->isPresentBlue[i])
                memcpy(pixelsBlue + i * 464 * 2784, imageptrs->ptrBlue[i], 464 * 2784 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::H8)
    {

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
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::GOES_16)
    {
        pixelsRed = new quint16[npix];
        pixelsGreen = new quint16[npix];
        pixelsBlue = new quint16[npix];

        memcpy(pixelsRed, imageptrs->ptrRed[0], npix * sizeof(quint16));
        memcpy(pixelsGreen, imageptrs->ptrGreen[0], npix * sizeof(quint16));
        memcpy(pixelsBlue, imageptrs->ptrBlue[0], npix * sizeof(quint16));

        for(int i = 0; i < npix; i++)
        {
            if(*(pixelsRed+i) == imageptrs->fillvalue[0] )
                *(pixelsRed + i) = 0;
            if(*(pixelsGreen+i) == imageptrs->fillvalue[1] )
                *(pixelsGreen + i) = 0;
            if(*(pixelsBlue+i) == imageptrs->fillvalue[2] )
                *(pixelsBlue + i) = 0;
            if(imageptrs->ptrDQF[0][i] == 2)
            {
                *(pixelsRed+i) = 1023;
                *(pixelsGreen+i) = 1023;
                *(pixelsBlue+i) = 1023;
            }
        }

    }
    else if(sl->getKindofImage() == "HRV" && (sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8 ))
    {
        pixelsHRV = new quint16[npixHRV];
        for( int i = 0, k = 0; i < (sl->bisRSS ? 5 : ( sl->areatype == 1 ? 24 : 5)); i++)
        {
            k = (sl->bisRSS ? 19 + i : (sl->areatype == 1 ? i : 19 + i));
            if(sl->isPresentHRV[k])
            {
                memcpy(pixelsHRV + i * 464 * 5568, imageptrs->ptrHRV[k], 464 * 5568 * sizeof(quint16));
            }
        }
    }
    else if(sl->getKindofImage() == "HRV" && (sl->getGeoSatellite() == eGeoSatellite::FY2E || sl->getGeoSatellite() == eGeoSatellite::FY2G ))
    {
        pixelsHRV = new quint16[npixHRV];
        memcpy(pixelsHRV, imageptrs->ptrHRV[0], 9152 * 9152 * sizeof(quint16));
    }
    else if(sl->getKindofImage() == "VIS_IR" && (sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8 ))
    {
        pixelsRed = new quint16[npix];
        for( int i = (sl->bisRSS ? 5 : 0); i < 8 ; i++)
        {
            if(sl->isPresentRed[i])
                memcpy(pixelsRed + (sl->bisRSS ? i - 5 : i) * 464 * 3712, imageptrs->ptrRed[i], 464 * 3712 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR" && sl->getGeoSatellite() == eGeoSatellite::GOMS2)
    {
        pixelsRed = new quint16[npix];
        for( int i = 0; i < 6 ; i++)
        {
            if(sl->isPresentRed[i])
                memcpy(pixelsRed + i * 464 * 2784, imageptrs->ptrRed[i], 464 * 2784 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR" && (sl->getGeoSatellite() == eGeoSatellite::GOES_15))
    {
        pixelsRed = new quint16[npix];
        for( int i = 0; i < 7 ; i++)
        {
            if(sl->isPresentRed[i])
                memcpy(pixelsRed + i * 464 * 2816, imageptrs->ptrRed[i], 464 * 2816 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR" && (sl->getGeoSatellite() == eGeoSatellite::FY2E || sl->getGeoSatellite() == eGeoSatellite::FY2G ))
    {
        pixelsRed = new quint16[npix];
        memcpy(pixelsRed, imageptrs->ptrRed[0], 2288 * 2288 * sizeof(quint16));
    }
    else if(sl->getKindofImage() == "VIS_IR" && (sl->getGeoSatellite() == eGeoSatellite::GOES_16 ))
    {
        pixelsRed = new quint16[npix];
        memcpy(pixelsRed, imageptrs->ptrRed[0], npix * sizeof(quint16));

        for(int i = 0; i < npix; i++)
        {
            if(*(pixelsRed+i) == imageptrs->fillvalue[0] )
                *(pixelsRed + i) = 0;
            if(imageptrs->ptrDQF[0][i] == 2)
                *(pixelsRed+i) = 1023;
        }
    }
    else if(sl->getKindofImage() == "VIS_IR" && sl->getGeoSatellite() == eGeoSatellite::H8)
    {
        pixelsRed = new quint16[npix];
        for( int i = 0; i < 10 ; i++)
        {
            if(sl->isPresentRed[i])
                memcpy(pixelsRed + i * 550 * 5500, imageptrs->ptrRed[i], 550 * 5500 * sizeof(quint16));
        }
    }

    int ret = 0;


    if(sl->getKindofImage() == "VIS_IR Color" && (sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8 ))
    {
        imageptrs->CLAHE(pixelsRed, 3712, (sl->bisRSS ? 3*464 : 3712), 0, 1023, 16, 16, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsGreen, 3712, (sl->bisRSS ? 3*464 : 3712), 0, 1023, 16, 16, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsBlue, 3712, (sl->bisRSS ? 3*464 : 3712), 0, 1023, 16, 16, 256, opts.clahecliplimit);
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::GOMS2)
    {
        imageptrs->CLAHE(pixelsRed, 2784, 2784, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsGreen, 2784, 2784, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsBlue, 2784, 2784, 0, 1023, 16, 16, 256, opts.clahecliplimit);
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::H8 )
    {
        ret = imageptrs->CLAHE(pixelsRed, 5500, 5500, 0, 1023, 10, 10, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsGreen, 5500, 5500, 0, 1023, 10, 10, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsBlue, 5500, 5500, 0, 1023, 10, 10, 256, opts.clahecliplimit);
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::GOES_16 )
    {
        ret = imageptrs->CLAHE(pixelsRed, 5424, 5424, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        qDebug() << QString("pixelsRed ret = %1").arg(ret);
        ret = imageptrs->CLAHE(pixelsGreen, 5424, 5424, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        qDebug() << QString("pixelsGreen ret = %1").arg(ret);
        ret = imageptrs->CLAHE(pixelsBlue, 5424, 5424, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        qDebug() << QString("pixelsBlue ret = %1").arg(ret);
    }
    else if(sl->getKindofImage() == "HRV" && (sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8))
    {
        if(sl->bisRSS)
        {
            imageptrs->CLAHE(pixelsHRV, 5568, 5*464, 0, 1023, 16, 16, 256, opts.clahecliplimit);

        }
        else
        {
            if(sl->areatype == 1)
            {
                imageptrs->CLAHE(pixelsHRV, 5568, 11136, 0, 1023, 16, 16, 256, opts.clahecliplimit);
            }
            else
            {
                imageptrs->CLAHE(pixelsHRV, 5568, 5*464, 0, 1023, 16, 16, 256, opts.clahecliplimit);
            }
        }
    }
    else if(sl->getKindofImage() == "HRV" && (sl->getGeoSatellite() == eGeoSatellite::FY2E || sl->getGeoSatellite() == eGeoSatellite::FY2G ))
    {
        imageptrs->CLAHE(pixelsHRV, 9152, 9152, 0, 255, 16, 16, 256, opts.clahecliplimit);
    }
    else if(sl->getKindofImage() == "VIS_IR")
    {
        if(sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_8)
            imageptrs->CLAHE(pixelsRed, 3712, 3712, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == eGeoSatellite::MET_9)
            imageptrs->CLAHE(pixelsRed, 3712, 3*464, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == eGeoSatellite::GOES_15)
            imageptrs->CLAHE(pixelsRed, 2816, 464*7, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == eGeoSatellite::GOMS2)
        {
            ret = imageptrs->CLAHE(pixelsRed, 2784, 464*6, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        }
        else if(sl->getGeoSatellite() == eGeoSatellite::FY2E || sl->getGeoSatellite() == eGeoSatellite::FY2G)
            imageptrs->CLAHE(pixelsRed, 2288, 2288, 0, 255, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == eGeoSatellite::GOES_16)
            imageptrs->CLAHE(pixelsRed, 5424, 5424, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == eGeoSatellite::H8)
            imageptrs->CLAHE(pixelsRed, 5500, 5500, 0, 1023, 10, 10, 256, opts.clahecliplimit);
    }

    qDebug() << "---> After CLAHE";

    if(sl->getKindofImage() == "VIS_IR Color" && (sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8 ))
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
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::H8)
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
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::GOES_16)
    {

        for (int line = 0; line < 5424; line++)
        {
            row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
            for (int pixelx = 0; pixelx < 5424; pixelx++)
            {
                cred = *(pixelsRed + line * 5424  + pixelx);
                cgreen = *(pixelsGreen + line * 5424  + pixelx);
                cblue = *(pixelsBlue + line * 5424  + pixelx);

                r = quint8(inversevector[0] ? 255 - (int)(cred/4) : (int)(cred/4));
                g = quint8(inversevector[1] ? 255 - (int)(cgreen/4) : (int)(cgreen/4));
                b = quint8(inversevector[2] ? 255 - (int)(cblue/4) : (int)(cblue/4));

                row_col[pixelx] = qRgb(r,g,b);
            }
        }

    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::GOMS2)
    {

        for(int i = 0; i < 6; i++)
        {
            for (int line = 0; line < 464; line++)
            {
                row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(i * 464 + line);
                for (int pixelx = 0; pixelx < 2784; pixelx++)
                {
                    cred = *(pixelsRed + i * 464 * 2784 + line * 2784  + pixelx);
                    cgreen = *(pixelsGreen + i * 464 * 2784 + line * 2784  + pixelx);
                    cblue = *(pixelsBlue + i * 464 * 2784 + line * 2784  + pixelx);

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
        if(sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8)
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
        else if(sl->getGeoSatellite() == eGeoSatellite::FY2E || sl->getGeoSatellite() == eGeoSatellite::FY2G)
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
        if(sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8 )
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
        else if(sl->getGeoSatellite() == eGeoSatellite::GOES_15)
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
        else if(sl->getGeoSatellite() == eGeoSatellite::GOMS2)
        {
            for(int i = 0 ; i < 6; i++)
            {
                //for (int line = 463; line >= 0; line--)
                for (int line = 0; line < 464; line++)
                {
                    row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(i * 464 + line);
                    for (int pixelx = 0; pixelx < 2784; pixelx++)
                    {
                        c = *(pixelsRed + i * 464 * 2784 + line * 2784 + pixelx);

//                        if(i == 0 && line == 300)
//                            qDebug() << pixelx << " " << c;

                        r = quint8(inversevector[0] ? 255 - c/4 : c/4);
                        g = quint8(inversevector[0] ? 255 - c/4 : c/4);
                        b = quint8(inversevector[0] ? 255 - c/4 : c/4);

                        row_col[pixelx] = qRgb(r,g,b);
                    }
                }
            }
        }
        else if(sl->getGeoSatellite() == eGeoSatellite::H8 )
        {
            qDebug() << "in if(sl->getGeoSatellite() == eGeoSatellite::H8 )";

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
        else if(sl->getGeoSatellite() == eGeoSatellite::FY2E || sl->getGeoSatellite() == eGeoSatellite::FY2G)
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
        else if(sl->getGeoSatellite() == eGeoSatellite::GOES_16)
        {
            qDebug() << "recalculate CLAHE ; VIS_IR and GOES_16 move to ptrImageGeostationary";

            for (int line = 0; line < 5424; line++)
            {
                row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
                for (int pixelx = 0; pixelx < 5424; pixelx++)
                {
                    c = *(pixelsRed + line * 5424 + pixelx);

                    r = quint8(inversevector[0] ? 255 - (int)(c/4) : (int)(c/4));
                    g = quint8(inversevector[0] ? 255 - (int)(c/4) : (int)(c/4));
                    b = quint8(inversevector[0] ? 255 - (int)(c/4) : (int)(c/4));

                    row_col[pixelx] = qRgb(r,g,b);
                }
            }
        }
    }


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
            emit render3dgeo(sl->getGeoSatelliteIndex());

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

    this->UpdateProjection();

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
    for(int i = 0; i < (segs->seglgeo[0]->areatype == 1 ? 24 : 5); i++)
    {
        for (int line = 463; line >= 0; line--)
        {
            row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine((segs->seglgeo[0]->areatype == 1 ? 24 : 5)*464 - i * 464 - line);
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
    if(sl == NULL)
        return;

    pixgeoConversion pixconv;

    int col, save_col;
    int row, save_row;
    bool first = true;

    double lat_deg;
    double lon_deg;
    int ret;
    bool hrvimage;

    long coff;
    long loff;
    double cfac;
    double lfac;

    int geoindex = sl->getGeoSatelliteIndex();

    if(sl != NULL)
    {
        if(sl->getKindofImage()  == "HRV" || sl->getKindofImage()  == "HRV Color")
            hrvimage = true;
        else
            hrvimage = false;

//        if(sl->getGeoSatellite() == eGeoSatellite::H8)
//        {
//            QPoint pt(opts.geosatellites.at(geoindex).coff, opts.geosatellites.at(geoindex).loff);
//            paint->setPen(qRgb(0, 0, 255));
//            paint->drawEllipse(pt, opts.geosatellites.at(geoindex).coff - 28, opts.geosatellites.at(geoindex).loff - 40);
//        }
    }
    else
        return;

    double sub_lon = sl->geosatlon;
    lat_deg = opts.obslat;
    lon_deg = opts.obslon;
    if (lon_deg > 180.0)
        lon_deg -= 360.0;

    coff = hrvimage ? opts.geosatellites.at(geoindex).coffhrv : opts.geosatellites.at(geoindex).coff;
    loff = hrvimage ? opts.geosatellites.at(geoindex).loffhrv : opts.geosatellites.at(geoindex).loff;
    cfac = hrvimage ? opts.geosatellites.at(geoindex).cfachrv : opts.geosatellites.at(geoindex).cfac;
    lfac = hrvimage ? opts.geosatellites.at(geoindex).lfachrv : opts.geosatellites.at(geoindex).lfac;


    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg, coff, loff, cfac, lfac, &col, &row);
    if(ret == 0)
    {
        if(hrvimage)
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


    if(!hrvimage)
    {
        for(int i = 0; i < this->geooverlay.count(); i++)
        {
            if (this->geooverlay.at(i).x() < 0)
            {
                first = true;
            }
            else if(first == true)
            {
                first = false;
                save_col = (int)this->geooverlay.at(i).x();
                save_row = (int)this->geooverlay.at(i).y();
            }
            else
            {
                paint->setPen(opts.geoimageoverlaycolor1);
                paint->drawLine(save_col, save_row, (int)this->geooverlay.at(i).x(), (int)this->geooverlay.at(i).y());
                save_col = (int)this->geooverlay.at(i).x();
                save_row = (int)this->geooverlay.at(i).y();
            }
        }
    }
    else
        OverlayGeostationaryHRV(paint, sl, geoindex);
}

void FormImage::OverlayGeostationaryHRV(QPainter *paint, SegmentListGeostationary *sl, int geoindex)
{

    long coff;
    long loff;
    double cfac;
    double lfac;

    int col, save_col;
    int row, save_row;
    bool first = true;

    double lat_deg;
    double lon_deg;
    int ret;

    pixgeoConversion pixconv;

    //if(sl->LowerNorthLineActual == 0)
    //    return;

    coff = opts.geosatellites.at(geoindex).coffhrv;
    loff = opts.geosatellites.at(geoindex).loffhrv;
    cfac = opts.geosatellites.at(geoindex).cfachrv;
    lfac = opts.geosatellites.at(geoindex).lfachrv;

    double sub_lon = sl->geosatlon;


    if(opts.gshhsglobe1On)
    {
        first = true;

        for (int i=0; i<gshhsdata->vxp_data_overlay[0]->nFeatures; i++)
        {
            for (int j=0; j<gshhsdata->vxp_data_overlay[0]->pFeatures[i].nVerts; j++)
            {
                lat_deg = gshhsdata->vxp_data_overlay[0]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                lon_deg = gshhsdata->vxp_data_overlay[0]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                if (lon_deg > 180.0)
                    lon_deg -= 360.0;

                if((lon_deg < 90.0 || lon_deg > -90.0))
                {
                    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg, coff, loff, cfac, lfac, &col, &row);
                    row+=3;
                    col+=2;

                    if(ret == 0)
                    {
                        if(sl->getGeoSatellite() != eGeoSatellite::FY2E && sl->getGeoSatellite() != eGeoSatellite::FY2G)
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

                        if (first)
                        {
                            first = false;
                            save_col = col;
                            save_row = row;
                        }
                        else
                        {
                            paint->setPen(opts.geoimageoverlaycolor1);
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
                    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg, coff, loff, cfac, lfac, &col, &row);
                    row+=3;
                    col+=2;

                    if(ret == 0)
                    {
                        if(sl->getGeoSatellite() != eGeoSatellite::FY2E && sl->getGeoSatellite() != eGeoSatellite::FY2G)
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

                        if (first)
                        {
                            first = false;
                            save_col = col;
                            save_row = row;
                        }
                        else
                        {
                            paint->setPen(opts.geoimageoverlaycolor2);
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

                    ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg, coff, loff, cfac, lfac, &col, &row);
                    row+=3;
                    col+=2;

                    if(ret == 0)
                    {
                        if(sl->getGeoSatellite() != eGeoSatellite::FY2E && sl->getGeoSatellite() != eGeoSatellite::FY2G)
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

                        if (first)
                        {
                            first = false;
                            save_col = col;
                            save_row = row;
                        }
                        else
                        {
                            paint->setPen(opts.geoimageoverlaycolor3);
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

    //this->update();
}

void FormImage::setupGeoOverlay(int geoindex)
{

    int col, save_col;
    int row, save_row;
    bool first = true;

    double lat_deg;
    double lon_deg;
    int ret;
    double sub_lon;

    double fgf_x, fgf_y;

    pixgeoConversion pixconv;

    sub_lon = opts.geosatellites.at(geoindex).longitude;

    this->geooverlay.clear();

    double scale_x = 0.000056;
    double scale_y = -0.000056;
    double offset_x = -0.151844;
    double offset_y = 0.151844;
    int sat = 1;

    for(int k = 0; k < 2; k++)
    {
        if(opts.gshhsglobe1On)
        {
            first = true;

            for (int i=0; i<gshhsdata->vxp_data_overlay[k]->nFeatures; i++)
            {
                for (int j=0; j<gshhsdata->vxp_data_overlay[k]->pFeatures[i].nVerts; j++)
                {
                    lat_deg = gshhsdata->vxp_data_overlay[k]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
                    lon_deg = gshhsdata->vxp_data_overlay[k]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
                    if (lon_deg > 180.0)
                        lon_deg -= 360.0;

                    if(lon_deg < 90.0 || lon_deg > -90.0)
                    {
                        if(opts.geosatellites.at(geoindex).shortname == "GOES_16")
                        {
                            pixconv.earth_to_fgf_(&sat, &lon_deg, &lat_deg, &scale_x, &offset_x, &scale_y, &offset_y, &sub_lon, &fgf_x, &fgf_y);
                            if(fgf_x >= 0 && fgf_x < opts.geosatellites.at(geoindex).imagewidth && fgf_y >= 0 && fgf_y < opts.geosatellites.at(geoindex).imageheight)
                            {
                                col = (int)fgf_x;
                                row = (int)fgf_y;
                                ret = 0;
                            }
                            else
                                ret = 1;
                        }
                        else
                        {
                            ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg, opts.geosatellites.at(geoindex).coff,
                                                        opts.geosatellites.at(geoindex).loff, opts.geosatellites.at(geoindex).cfac, opts.geosatellites.at(geoindex).lfac, &col, &row);

                        }
                        if(ret == 0)
                        {
                            if (first)
                            {
                                first = false;
                                save_col = col;
                                save_row = row;
                                this->geooverlay.append(QVector2D(-1, -1));
                            }
                            else
                            {
                                save_col = col;
                                save_row = row;
                            }

                            this->geooverlay.append(QVector2D(col, row));

                        }
                        else
                            first = true;
                    }
                }
                first = true;
            }
        }
    }

    qDebug() << QString("geooverlay length = %1").arg(this->geooverlay.count());

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

void FormImage::OverlayProjection(QPainter *paint)
{
    qDebug() << QString("FormImage::OverlayProjection(QPainter *paint, SegmentListGeostationary *sl) opts.currenttoolbox = %1").arg(opts.currenttoolbox);

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

    bool first = true;

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
                    bret = imageptrs->gvp->map_forward( lon*PI/180.0, lat*PI/180.0, map_x, map_y);

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

void FormImage::OverlayOLCI(QPainter *paint)
{
    SegmentListOLCI *sl;
    qDebug() << "FormImage::OverlayOLCI(QPainter *paint) 0";
    int height = imageptrs->ptrimageOLCI->height();
    int width = imageptrs->ptrimageOLCI->width();
    if( height == 0 || width == 0)
        return;
    qDebug() << "FormImage::OverlayOLCI(QPainter *paint) 1";
    qDebug() << "opts.buttonOLCIefr = " << opts.buttonOLCIefr;
    qDebug() << "opts.buttonOLCIerr = " << opts.buttonOLCIerr;
    qDebug() << "opts.buttonSLSTR   = " << opts.buttonSLSTR;

    if(opts.buttonOLCIefr)
    {
        if(segs->seglolciefr->GetSegmentlistptr()->count() == 0)
            return;
    }
    else if(opts.buttonOLCIerr)
    {
        if(segs->seglolcierr->GetSegmentlistptr()->count() == 0)
            return;
    }
    else if(opts.buttonSLSTR)
            return;
    qDebug() << "FormImage::OverlayOLCI(QPainter *paint) 2";

    if(opts.buttonOLCIefr)
        sl = segs->seglolciefr;
    else if(opts.buttonOLCIerr)
        sl = segs->seglolcierr;

    long nbrpt = 0;

    qDebug() << "FormImage::OverlayOLCI(QPainter *paint) 3";
    qDebug() << "FormImage::OverlayOLCI(QPainter *paint) ptrimageOLCI height = " << imageptrs->ptrimageOLCI->height();
    qDebug() << "opts.buttonOLCIefr = " << opts.buttonOLCIefr;
    qDebug() << "opts.buttonOLCIerr = " << opts.buttonOLCIerr;
    qDebug() << "opts.buttonSLSTR   = " << opts.buttonSLSTR;
    qDebug() << "segs->seglolciefr->GetSegmentlistptr()->count() = " << segs->seglolciefr->GetSegmentlistptr()->count();
    qDebug() << "segs->seglolcierr->GetSegmentlistptr()->count() = " << segs->seglolcierr->GetSegmentlistptr()->count();
    qDebug() << "segs->seglslstr->GetSegmentlistptr()->count() = " << segs->seglslstr->GetSegmentlistptr()->count();

    if(sl->GetSegsSelectedptr()->count() > 0)
    {

        qDebug() << "FormImage::OverlayOLCI(QPainter *paint) 4";

        qDebug() << "segs->seglolci count selected = " << sl->GetSegsSelectedptr()->count();
        qDebug() << "height = " << imageptrs->ptrimageOLCI->height();
        qDebug() << "width = " << imageptrs->ptrimageOLCI->width();
        qDebug() << this->kindofimage;

        QList<Segment*>::iterator segsel = sl->GetSegsSelectedptr()->begin();
        int heightinsegment = 0;
        while ( segsel != sl->GetSegsSelectedptr()->end() )
        {
            SegmentOLCI *segm = (SegmentOLCI *)(*segsel);
            paint->setPen(QColor(opts.olciimageoverlaycolor));
            QPolygon copycoastline = segm->coastline.translated(0, heightinsegment);
            paint->drawPoints(copycoastline);

//            if(opts.gridonolciimage)
//            {
//                paint->setPen(QColor(opts.projectionoverlaylonlatcolor));
//                QPolygon copylatlonline = segm->latlonline.translated(0, heightinsegment);
//                paint->drawPoints(copylatlonline);
//            }

            heightinsegment += segm->GetNbrOfLines();
            ++segsel;
        }
    }


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

bool FormImage::SaveAsPNG48bits(bool mapto65535)
{
    QString filestr;

    filestr.append("./");


    if (this->channelshown == IMAGE_OLCI)
    {
        filestr += "olci_image.png";
    }
    else return(false);


    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save image"), filestr,
                                                    tr("*.png"));
    if (fileName.isEmpty())
        return(false);
    else
    {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            if(fileName.mid(fileName.length()-4) != ".png" && fileName.mid(fileName.length()-4) != ".PNG")
                fileName.append(".png");

            int olciefrcount = segs->seglolciefr->NbrOfSegmentsSelected();
            int olcierrcount = segs->seglolcierr->NbrOfSegmentsSelected();

            if(olciefrcount > 0)
            {
                segs->seglolciefr->Compose48bitPNG(fileName, mapto65535);
                QApplication::restoreOverrideCursor();
            }
            else if(olcierrcount > 0)
            {
                segs->seglolcierr->Compose48bitPNG(fileName, mapto65535);
                QApplication::restoreOverrideCursor();
            }
            else
            {
                QApplication::restoreOverrideCursor();
                return(false);
            }


    }

    return(true);
}

FormImage::~FormImage()
{

}

//////////////////////////////////////////////////////////////////////////////////
ImageLabel::ImageLabel(QWidget *parent, AVHRRSatellite *seglist) :  QLabel(parent)
{
    segs = seglist;
    setMouseTracking(true);

}

void ImageLabel::mouseMoveEvent(QMouseEvent *event)
{
    double lon, lat;
    float flon, flat;

    bool bret;
    double factorheight = (double)this->originalpixmapsize.height()/(double)this->scaledpixmapsize.height();
    double factorwidth = (double)this->originalpixmapsize.width()/(double)this->scaledpixmapsize.width();
    double xpos = (double)event->pos().x() * factorwidth;
    double ypos = (double)event->pos().y() * factorheight;

    if(xpos >= this->originalpixmapsize.width() || ypos >= this->originalpixmapsize.height() )
        return;

    if(formimage->getPictureSize() != QSize(-1,-1))
    {
        if(formimage->channelshown == IMAGE_PROJECTION)
        {
            if (opts.currenttoolbox == 0)       //LCC
                bret = imageptrs->lcc->map_inverse(xpos, ypos, lon, lat);
            else if (opts.currenttoolbox == 1)  //GVP
                bret = imageptrs->gvp->map_inverse(xpos, ypos, lon, lat);
            else                                //SG
                bret = imageptrs->sg->map_inverse(xpos, ypos, lon, lat);

            if(bret)
                emit coordinateChanged(QString("longitude = %1 latitude = %2   ")
                                       .arg(lon*180.0/PI, 0, 'f', 2).arg(lat*180.0/PI, 0, 'f', 2));
        }
        else if(formimage->channelshown == IMAGE_OLCI)
        {
            if(segs->seglolciefr->GetSegsSelectedptr()->count() > 0)
                bret = segs->seglolciefr->searchLatLon(qRound(xpos), qRound(ypos), flon, flat);
            else if(segs->seglolcierr->GetSegsSelectedptr()->count() > 0)
                bret = segs->seglolcierr->searchLatLon(qRound(xpos), qRound(ypos), flon, flat);
            else
                bret = false;

            if(bret)
                emit coordinateChanged(QString("longitude = %1 latitude = %2   ")
                                       .arg(flon, 0, 'f', 2).arg(flat, 0, 'f', 2));


        }
        else
        {
            emit coordinateChanged(" ");
        }
    }
    else
    {
        emit coordinateChanged(" ");
    }

    QLabel::mouseMoveEvent(event);
}

void ImageLabel::setPixmap ( const QPixmap & p)
{
    this->originalpixmapsize = p.size();
    this->scaledpixmapsize = p.size();
    QLabel::setPixmap(p);
}

void ImageLabel::resize(const QSize &s)
{
    this->scaledpixmapsize = s;
    QLabel::resize(s);
}

///////////////////////////////////////////////////////////////////////////////////////

//AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent) :
//    QLabel(parent)
//{
//    this->setMinimumSize(1,1);
//    setScaledContents(false);
//}

//void AspectRatioPixmapLabel::setPixmap ( const QPixmap & p)
//{
//    pix = p;
//    QLabel::setPixmap(scaledPixmap());
//}

//int AspectRatioPixmapLabel::heightForWidth( int width ) const
//{
//    return pix.isNull() ? this->height() : ((qreal)pix.height()*width)/pix.width();
//}

//QSize AspectRatioPixmapLabel::sizeHint() const
//{
//    int w = this->width();
//    return QSize( w, heightForWidth(w) );
//}

//QPixmap AspectRatioPixmapLabel::scaledPixmap() const
//{
//    return pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
//}

//void AspectRatioPixmapLabel::resizeEvent(QResizeEvent * e)
//{
//    if(!pix.isNull())
//        QLabel::setPixmap(scaledPixmap());
//}
//////////////////////////////////////////////////////////////////////////////////////
/// \brief AspectRatioPixmapLabel::AspectRatioPixmapLabel
/// \param pixmap
/// \param parent
///
//AspectRatioPixmapLabel::AspectRatioPixmapLabel(const QPixmap &pixmap, QWidget *parent) :
//    QLabel(parent)
//{
//    QLabel::setPixmap(pixmap);
//    setScaledContents(true);
//    QSizePolicy policy(QSizePolicy::Maximum, QSizePolicy::Maximum);
//    policy.setHeightForWidth(true);
//    this->setSizePolicy(policy);
//}

//int AspectRatioPixmapLabel::heightForWidth(int width) const
//{
//    if (width > pixmap()->width()) {
//        return pixmap()->height();
//    } else {
//        return ((qreal)pixmap()->height()*width)/pixmap()->width();
//    }
//}
