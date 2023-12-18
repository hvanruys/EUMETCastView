#include "formimage.h"
#include "pixgeoconversion.h"
#include "gshhsdata.h"
#include "satellite.h"
#include "ColorSpace.h"
#include "Conversion.h"

#include <QDebug>
#include <QMessageBox>
#include <QPrintDialog>
#include <QFileDialog>
#include <QWheelEvent>
#include <QtMath>
#include <QMatrix>

extern Options opts;
extern SegmentImage *imageptrs;
extern gshhsData *gshhsdata;
extern bool ptrimagebusy;
extern SatelliteList SatelliteList;



//SegmentListGeostationary *sm = seglist->getActiveSegmentList();
//connect(ui->hslRed, SIGNAL(valueChanged(int)), sm, SLOT(setRedValue(int)));

FormImage::FormImage(QWidget *parent, AVHRRSatellite *seglist) :
    QGraphicsView(parent), m_rotateAngle(0), m_ViewInitialized(false)
{
    m_scene = new QGraphicsScene(this);
    this->setScene(m_scene);
    this->setBackgroundBrush(QBrush(QColor(38,38,38,255),Qt::SolidPattern));
    this->setDragMode(NoDrag);

    segs = seglist;
    channelshown = IMAGE_GEOSTATIONARY;
    m_image = NULL;

    overlaymeteosat = true;
    overlayprojection = true;
    overlayolci = true;

    metopcount = 0;
    noaacount = 0;
    hrpcount = 0;
    gaccount = 0;
    metopAhrptcount = 0;
    metopBhrptcount = 0;
    noaa19hrptcount = 0;
    M01hrptcount = 0;
    M02hrptcount = 0;
    viirsmcount = 0;
    viirsdnbcount = 0;
    viirsmcountnoaa20 = 0;
    viirsdnbcountnoaa20 = 0;
    olciefrcount = 0;
    olcierrcount = 0;
    slstrcount = 0;
    mersicount = 0;

    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    this->setupGeoOverlay(0);

}

bool FormImage::toggleOverlayMeteosat()
{
    if(overlaymeteosat)
        overlaymeteosat = false;
    else
        overlaymeteosat = true;
    m_scene->update();
    return overlaymeteosat;
}

bool FormImage::toggleOverlayOLCI()
{
    if(overlayolci)
        overlayolci = false;
    else
        overlayolci = true;
    m_scene->update();
    return overlayolci;
}

bool FormImage::toggleOverlayGridOnOLCI()
{
    m_scene->update();
    return overlayolci;
}

bool FormImage::toggleOverlayProjection()
{
    if(overlayprojection)
        overlayprojection = false;
    else
        overlayprojection = true;
    m_scene->update();
    return overlayprojection;
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
    QString spectrum;
    SegmentListGeostationary *slgeo = NULL;

    slgeo = segs->getActiveSegmentList();

    QString type = slgeo->getKindofImage();
    QVector<QString> spectrumvector = slgeo->getSpectrumVector();
    if(spectrumvector.size() > 0)
        spectrum = ( type == "VIS_IR" ? spectrumvector.at(0) : "");
    else
        spectrum = "";

    txtInfo = QString("<!DOCTYPE html>"
                      "<html><head><title>Info</title></head>"
                      "<body>"
                      "<h4 style='color:blue'>Image Information</h4>"
                      "<p>Satellite = %1<br>"
                      "Image type = %2 %3<br>"
                      "Image width = %4 height = %5</p>"
                      "</body></html>").arg(satname).arg(type).arg(spectrum).arg(imageptrs->ptrimageGeostationary->width()).arg(imageptrs->ptrimageGeostationary->height());


    formtoolbox->writeInfoToTextEdit(txtInfo);
}
void FormImage::resetView()
{
    if(m_image == NULL){
        return;
    }

    m_ViewInitialized = false;

    m_scene->clear();
    //    m_image = NULL; //QImage();
    m_fileName.clear();
    m_rotateAngle = 0;
    this->setDragMode(NoDrag);
    this->resetTransform();
}

void FormImage::fitWindow()
{
    if(!m_ViewInitialized)
    {
        qDebug() << "FormImage::fitWindow() m_ViewInitialized = false";
        return;
    }

    if(m_image == NULL)
    {
        qDebug() << "FormImage::fitWindow() m_image ==NULL";
        return;
    }

    this->resetTransform();

    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    this->setWindowTitle();
}

void FormImage::originalSize()
{
    if(!m_ViewInitialized)
        return;
    if(m_image->isNull())
        return;

    this->setDragMode(ScrollHandDrag);
    this->resetTransform();
    this->centerOn(m_pixmapItem);
    this->setWindowTitle();
}

void FormImage::setWindowTitle()
{
    QTransform transf = this->transform();
    int zoomfactor = (int)(transf.m11()*100);
    QString title = QString("EUMETCast Viewer ( %1 %)").arg(zoomfactor);
    emit signalMainWindowTitleChanged(title);

}

void FormImage::rotateView(const int nVal)
{
    if(m_image->isNull()){
        return;
    }

    // rotate view
    this->rotate(nVal);
    //this->show(); // this necessary?

    // update angle
    m_rotateAngle += nVal;  // a=a+(-90)== -90

    // reset angle
    if(m_rotateAngle >= 360 || m_rotateAngle <= -360){
        m_rotateAngle =0;
    }
}

void FormImage::printView()
{
    if(!m_ViewInitialized)
        return;

    if(m_image->isNull()){
        return;
    }
#ifndef QT_NO_PRINTER
    if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);
        m_scene->render(&painter);
    }
#endif
}

bool FormImage::saveViewToDisk(QString &strError)
{
    if(!m_ViewInitialized)
        return false;

    if(m_image->isNull()){
        strError = QObject::tr("Save failed.");
        return false;
    }

    // save a copy
    QImage imageCopy = *m_image;

    // Output file dialog
    QString fileFormat = getImageFormat(m_fileName);
    QString strFilePath = QFileDialog::getSaveFileName(
                this,
                tr("Save File"),
                QDir::homePath(),
                fileFormat);


    // If Cancel is pressed, getSaveFileName() returns a null string.
    if(strFilePath==""){
        strError = QObject::tr("");
        return false;
    }

    // ensure output path has proper extension
    if(!strFilePath.endsWith(fileFormat))
        strFilePath += "."+fileFormat;

    // save image in modified state
    if(isModified()) {
        QTransform t;
        t.rotate(m_rotateAngle);
        imageCopy = imageCopy.transformed(t, Qt::SmoothTransformation);
    }

    // quality factor (-1 default, 100 max)
    // note: -1 is about 4 times smaller than original, 100 is larger than original
    if(!imageCopy.save(strFilePath,fileFormat.toLocal8Bit().constData(),100)){
        strError = QObject::tr("Save failed.");
        return false;
    }
    return true;
}

QString FormImage::getImageFormat(QString strFileName)
{
    QString str = strFileName.mid(strFileName.lastIndexOf(".")+1,-1);
    if(str.toLower() == "tif")
        str = "tiff";
    return str;
}

// preserve fitWindow state on window resize
void FormImage::resizeEvent(QResizeEvent *event)
{
    if(!m_ViewInitialized)
        return;

    QGraphicsView::resizeEvent(event);  // call base implementation
    this->centerOn(m_pixmapItem);
    this->setWindowTitle();
}

// scale image on wheelEvent
void FormImage::wheelEvent(QWheelEvent *event)
{
    //if(!m_IsViewInitialized)
    //    return;

    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // zoomIn factor
    qreal factor = 1.15;

    // zoomOut factor
    if(event->angleDelta().y() < 0)
        factor = 1.0 / factor;

    // scale  the View
    this->scale(factor,factor);
    //qDebug() << "viewport width = " << this->viewport()->width() << " height = " << this->viewport()->height();
    //qDebug() << "boundingsrect item width = " << this->m_pixmapItem->boundingRect().width() << " height = " << this->m_pixmapItem->boundingRect().height();
    //QRectF itemBR = this->m_pixmapItem->boundingRect();
    //QRectF itemsceneBR = this->m_pixmapItem->sceneBoundingRect();
    //qDebug() << "itemsceneBR width = " << itemsceneBR.width() << " height = " << itemsceneBR.height();
    //QPolygonF sceneBR = this->m_pixmapItem->mapToScene(itemBR);
    //QPolygonF viewBR = this->m_pixmapItem->mapFromScene(sceneBR);// viewBR is your rect/point/whatever
    //qDebug() << "sceneBR width = " << sceneBR.boundingRect().width() << " height = " << sceneBR.boundingRect().height();
    //qDebug() << "viewBR width = " << viewBR.boundingRect().width() << " height = " << viewBR.boundingRect().height();
    //qDebug() << "viewBR width = " << viewBR. .boundingRect().width() << " height = " << viewBR.boundingRect().height();
    //qDebug() << "angledelta.y() = " << event->angleDelta().y();

    this->setWindowTitle();
    event->accept();
}

void FormImage::zoomIn()
{
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    qreal factor = 1.15;
    this->scale(factor,factor);
    this->setWindowTitle();
}

void FormImage::zoomOut()
{
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    qreal factor = 1.15;
    factor = 1.0 / factor;
    this->scale(factor,factor);
    this->setWindowTitle();
}

void FormImage::slotcomposefinished(QString kindofimage, int index)
{
    qDebug() << "slotcomposefinished " << kindofimage << " index = " << index;

    this->setPixmapToScene(true);

    SegmentListGeostationary *sl = NULL;
    sl = segs->getActiveSegmentList();


    //    if(sl->getGeoSatellite() == eGeoSatellite::H8)
    //    {
    //        EnhanceDarkSpace(sl->getGeoSatelliteIndex());
    //        imageGraphicsView->setPixmap(QPixmap::fromImage( *(imageptrs->ptrimageGeostationary)));
    //        refreshoverlay = true;
    //    }

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
            emit render3dgeo(sl->getGeoSatelliteIndex());
        }
    }
    else
        emit allsegmentsreceivedbuttons(true);

    qDebug() << "end FormImage::slotcomposefinished()";
    this->update();

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
    else if(opts.buttonVIIRSMNOAA20)
    {
        viirsmcountnoaa20 = segs->seglviirsmnoaa20->NbrOfSegmentsSelected();
    }
    else if(opts.buttonVIIRSDNBNOAA20)
    {
        viirsdnbcountnoaa20 = segs->seglviirsdnbnoaa20->NbrOfSegmentsSelected();
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
    else if(opts.buttonMERSI)
    {
        mersicount = segs->seglmersi->NbrOfSegmentsSelected();
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
    qDebug() << QString("in FormImage::ComposeImage nbr of viirsmnoaa20 segments selected = %1").arg(viirsmcountnoaa20);
    qDebug() << QString("in FormImage::ComposeImage nbr of viirsdnbnoaa20 segments selected = %1").arg(viirsdnbcountnoaa20);
    qDebug() << QString("in FormImage::ComposeImage nbr of olciefr segments selected = %1").arg(olciefrcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of olcierr segments selected = %1").arg(olcierrcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of slstr segments selected = %1").arg(slstrcount);
    qDebug() << QString("in FormImage::ComposeImage nbr of mersi segments selected = %1").arg(mersicount);

    if(opts.buttonMetop || opts.buttonNoaa || opts.buttonGAC || opts.buttonHRP ||
            opts.buttonMetopAhrpt || opts.buttonMetopBhrpt || opts.buttonNoaa19hrpt || opts.buttonM01hrpt || opts.buttonM02hrpt )
    {
        if (metopcount + noaacount + hrpcount + gaccount + metopAhrptcount + metopBhrptcount + noaa19hrptcount + M01hrptcount + M02hrptcount > 0)
        {
            this->channelshown = IMAGE_AVHRR_COL;
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
                this->setSegmentType(SEG_NOAA19);
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

        this->channelshown = IMAGE_VIIRSM;

        formtoolbox->setToolboxButtons(false);

        this->displayImage(IMAGE_VIIRSM, true);
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
        this->channelshown = IMAGE_VIIRSDNB;

        formtoolbox->setToolboxButtons(false);
        segs->seglviirsdnb->graphvalues.reset(new long[150 * 180]);
        for(int i = 0; i < 150 * 180; i++)
            segs->seglviirsdnb->graphvalues[i] = 0;


        this->displayImage(IMAGE_VIIRSDNB, true);
        this->kindofimage = "VIIRSDNB";
        this->setSegmentType(SEG_VIIRSDNB);

        bandlist = formtoolbox->getVIIRSMBandList();
        colorlist = formtoolbox->getVIIRSMColorList();
        invertlist = formtoolbox->getVIIRSMInvertList();
        //          in Workerthread
        segs->seglviirsdnb->ComposeVIIRSImage(bandlist, colorlist, invertlist);
    }
    else if(viirsmcountnoaa20 > 0 && opts.buttonVIIRSMNOAA20)
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

        this->channelshown = IMAGE_VIIRSM;

        formtoolbox->setToolboxButtons(false);

        this->displayImage(IMAGE_VIIRSM, true);
        this->kindofimage = "VIIRSM";
        this->setSegmentType(SEG_VIIRSMNOAA20);
        bandlist = formtoolbox->getVIIRSMBandList();
        colorlist = formtoolbox->getVIIRSMColorList();
        invertlist = formtoolbox->getVIIRSMInvertList();
        //          in Workerthread
        segs->seglviirsmnoaa20->ComposeVIIRSImage(bandlist, colorlist, invertlist);
        //          in main thread
        //            segs->seglviirsm->ComposeVIIRSImageSerial(bandlist, colorlist, invertlist);
    }
    else if(viirsdnbcountnoaa20 > 0 && opts.buttonVIIRSDNBNOAA20)
    {
        this->channelshown = IMAGE_VIIRSDNB;
        formtoolbox->setToolboxButtons(false);
        segs->seglviirsdnbnoaa20->graphvalues.reset(new long[150 * 180]);
        for(int i = 0; i < 150 * 180; i++)
            segs->seglviirsdnbnoaa20->graphvalues[i] = 0;


        this->displayImage(IMAGE_VIIRSDNB, true);
        this->kindofimage = "VIIRSDNB";
        this->setSegmentType(SEG_VIIRSDNBNOAA20);

        bandlist = formtoolbox->getVIIRSMBandList();
        colorlist = formtoolbox->getVIIRSMColorList();
        invertlist = formtoolbox->getVIIRSMInvertList();
        //          in Workerthread
        segs->seglviirsdnbnoaa20->ComposeVIIRSImage(bandlist, colorlist, invertlist);
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

        if(segs->seglolciefr->NbrOfSegmentsSelected() == 0)
        {
            QMessageBox msgBox;
            msgBox.setText("You need to select one or more segments.");
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

        this->channelshown = IMAGE_OLCI;

        formtoolbox->setToolboxButtons(false);

        imageptrs->olcitype = SEG_OLCIEFR;
        this->displayImage(IMAGE_OLCI, true);
        this->kindofimage = "OLCIEFR";
        this->setSegmentType(SEG_OLCIEFR);

        bandlist = formtoolbox->getOLCIBandList();
        colorlist = formtoolbox->getOLCIColorList();
        invertlist = formtoolbox->getOLCIInvertList();

        int histogrammethod = formtoolbox->getOLCIHistogrammethod();
        bool normalized = formtoolbox->getOLCINormalized();

        QStringList missing;

        if(segs->seglolciefr->CheckForOLCIFiles(bandlist, colorlist, missing) == false)
        {
            formtoolbox->setToolboxButtons(true);

            emit setmapcylbuttons(true);

            QMessageBox msgBox;
            QString txt = "In one or more segments, the following files are missing : \n";
            for(int i = 0; i < missing.count(); i++)
            {
                txt.append(missing.at(i) + "\n");
            }
            msgBox.setText(txt);
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
        else
        { //          in Workerthread
            segs->seglolciefr->ComposeOLCIImage(bandlist, colorlist, invertlist, true, histogrammethod, normalized);
        }
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

        this->channelshown = IMAGE_OLCI;

        formtoolbox->setToolboxButtons(false);

        imageptrs->olcitype = SEG_OLCIERR;
        this->displayImage(IMAGE_OLCI, true);
        this->kindofimage = "OLCIERR";
        this->setSegmentType(SEG_OLCIERR);

        bandlist = formtoolbox->getOLCIBandList();
        colorlist = formtoolbox->getOLCIColorList();
        invertlist = formtoolbox->getOLCIInvertList();

        int histogrammethod = formtoolbox->getOLCIHistogrammethod();
        bool normalized = formtoolbox->getOLCINormalized();


        QStringList missing;

        if(segs->seglolcierr->CheckForOLCIFiles(bandlist, colorlist, missing) == false)
        {
            formtoolbox->setToolboxButtons(true);

            emit setmapcylbuttons(true);

            QMessageBox msgBox;
            QString txt = "In one or more segments, the following files are missing : \n";
            for(int i = 0; i < missing.count(); i++)
            {
                txt.append(missing.at(i) + "\n");
            }
            msgBox.setText(txt);
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
        else     //          in Workerthread
            segs->seglolcierr->ComposeOLCIImage(bandlist, colorlist, invertlist, true, histogrammethod, normalized);
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

        this->channelshown = IMAGE_SLSTR;

        this->displayImage(IMAGE_SLSTR, true);
        this->kindofimage = "SLSTR";
        this->setSegmentType(SEG_SLSTR);

        bandlist = formtoolbox->getSLSTRBandList();
        colorlist = formtoolbox->getSLSTRColorList();
        invertlist = formtoolbox->getSLSTRInvertList();
        eSLSTRImageView slstrimageview = formtoolbox->getSLSTRImageView(); //Oblique or Nadir

        QStringList missing;

        if(segs->seglslstr->CheckForSLSTRFiles(bandlist, colorlist, missing) == false)
        {
            formtoolbox->setToolboxButtons(true);

            emit setmapcylbuttons(true);

            QMessageBox msgBox;
            QString txt = "One or more files are missing ; download the complete product";
            for(int i = 0; i < missing.count(); i++)
            {
                txt.append(missing.at(i) + "\n");
            }
            msgBox.setText(txt);
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
        else     //          in Workerthread
            segs->seglslstr->ComposeSLSTRImage(bandlist, colorlist, invertlist, true, slstrimageview);
    }
    else if(mersicount > 0 && opts.buttonMERSI)
    {
        if(!formtoolbox->comboColMERSIOK())
        {
            QMessageBox msgBox;
            msgBox.setText("Need color choices for 3 different bands in the MERSI tab.");
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

        this->channelshown = IMAGE_MERSI;

        formtoolbox->setToolboxButtons(false);

        this->displayImage(IMAGE_MERSI, true);
        this->kindofimage = "MERSI";
        this->setSegmentType(SEG_MERSI);

        bandlist = formtoolbox->getMERSIBandList();
        colorlist = formtoolbox->getMERSIColorList();
        invertlist = formtoolbox->getMERSIInvertList();

        int histogrammethod = formtoolbox->getMERSIHistogrammethod();
        bool normalized = false;

        //          in Workerthread
        segs->seglmersi->ComposeMERSIImage(bandlist, colorlist, invertlist, false, histogrammethod, normalized);
    }
    else
        return;
}

void FormImage::setPixmapToScene(bool settoolboxbuttons)
{

    qDebug() << "FormImage::setPixmapToScene(bool settoolboxbuttons) channelshown = " << ImageTypeToString(channelshown);

    resetView();

    emit allsegmentsreceivedbuttons(settoolboxbuttons);
    emit setmapcylbuttons(true);

    switch(channelshown)
    {
    case IMAGE_AVHRR_CH1:
        displayAVHRRImageInfo();
        m_image = imageptrs->ptrimagecomp_ch[0];
        formtoolbox->setOMimagesize(m_image->width(), m_image->height());
        break;
    case IMAGE_AVHRR_CH2:
        displayAVHRRImageInfo();
        m_image = imageptrs->ptrimagecomp_ch[1];
        formtoolbox->setOMimagesize(m_image->width(), m_image->height());
        break;
    case IMAGE_AVHRR_CH3:
        displayAVHRRImageInfo();
        m_image = imageptrs->ptrimagecomp_ch[2];
        formtoolbox->setOMimagesize(m_image->width(), m_image->height());
        break;
    case IMAGE_AVHRR_CH4:
        displayAVHRRImageInfo();
        m_image = imageptrs->ptrimagecomp_ch[3];
        formtoolbox->setOMimagesize(m_image->width(), m_image->height());
        break;
    case IMAGE_AVHRR_CH5:
        displayAVHRRImageInfo();
        m_image = imageptrs->ptrimagecomp_ch[4];
        formtoolbox->setOMimagesize(m_image->width(), m_image->height());
        break;
    case IMAGE_AVHRR_COL:
        displayAVHRRImageInfo();
        m_image = imageptrs->ptrimagecomp_col;
        formtoolbox->setOMimagesize(m_image->width(), m_image->height());
        break;
    case IMAGE_AVHRR_EXPAND:
        displayAVHRRImageInfo();
        m_image = imageptrs->ptrexpand_col;
        break;
    case IMAGE_GEOSTATIONARY:
        displayGeoImageInfo();
        m_image = imageptrs->ptrimageGeostationary;
        break;
    case IMAGE_PROJECTION:
        formtoolbox->writeInfoToTextEdit("");
        m_image = imageptrs->ptrimageProjection;
        break;
    case IMAGE_VIIRSM:
        displayVIIRSImageInfo(segmenttype);
        m_image = imageptrs->ptrimageViirsM;
        formtoolbox->setOMimagesize(imageptrs->ptrimageViirsM->width(), imageptrs->ptrimageViirsM->height());
        break;
    case IMAGE_VIIRSDNB:
        displayVIIRSImageInfo(segmenttype);
        m_image = imageptrs->ptrimageViirsDNB;
        formtoolbox->setOMimagesize(imageptrs->ptrimageViirsDNB->width(), imageptrs->ptrimageViirsDNB->height());
        break;
    case IMAGE_OLCI:
        displaySentinelImageInfo(SEG_OLCIEFR);
        m_image = imageptrs->ptrimageOLCI;
        break;
    case IMAGE_SLSTR:
        displaySentinelImageInfo(SEG_SLSTR);
        m_image = imageptrs->ptrimageSLSTR;
        break;
    case IMAGE_MERSI:
        displayMERSIImageInfo(SEG_MERSI);
        m_image = imageptrs->ptrimageMERSI;
        formtoolbox->setOMimagesize(imageptrs->ptrimageMERSI->width(), imageptrs->ptrimageMERSI->height());
        break;
    case IMAGE_NONE:
        return;

    }

    m_pixmap = QPixmap::fromImage(*m_image);
    m_pixmapItem = m_scene->addPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());
    this->centerOn(m_pixmapItem);

    m_ViewInitialized = true;

    fitWindow();
    this->setDragMode(ScrollHandDrag);


    QApplication::processEvents();

    qDebug() << QString("FormImage::setPixmapToScene() channelshown = %1").arg(this->channelshown);

}

void FormImage::displayImage(eImageType channel, bool resize)
{

    qDebug() << QString("FormImage::displayImage(eImageType channel) channel = %1").arg(ImageTypeToString(channel));
    qDebug() << QString("FormImage imageptrs->ptrimagecol bytecount = %1").arg(imageptrs->ptrimagecomp_col->byteCount());
    qDebug() << QString("FormImage imageptrs->ptrimageProjection bytecount = %1").arg(imageptrs->ptrimageProjection->byteCount());
    //    qDebug() << QString("FormImage ptrimageviirsm bytecount = %1").arg(imageptrs->ptrimageViirsM->byteCount());
    //    qDebug() << QString("FormImage ptrimageviirsdnb bytecount = %1").arg(imageptrs->ptrimageViirsDNB->byteCount());
    //    qDebug() << QString("FormImage ptrimageolci bytecount = %1").arg(imageptrs->ptrimageOLCI->byteCount());
    //    qDebug() << QString("FormImage ptrimageslstr bytecount = %1").arg(imageptrs->ptrimageSLSTR->byteCount());
    //    qDebug() << QString("FormImage ptrimagemersi bytecount = %1").arg(imageptrs->ptrimageMERSI->byteCount());

    this->channelshown = channel;

    QPixmap pm(800, 200);
    pm.fill(Qt::red);
    QPainter painter(&pm);

    QFont f("Courier", 20, QFont::Bold);
    painter.setFont(f);
    painter.setPen(Qt::yellow);
    QFontMetrics fm(f);

    if(ptrimagebusy)
    {
        QString str("Calculating image busy");
        int pixelsWide = fm.horizontalAdvance(str);
        painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
        m_pixmap = pm;
    }
    else
    {
        resetView();

        switch(channelshown)
        {
        case IMAGE_NONE:
            break;

        case IMAGE_AVHRR_CH1:
            if(imageptrs->ptrimagecomp_ch[0]->sizeInBytes() == 0)
            {
                QString str("No AVHRR image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                displayAVHRRImageInfo();
                m_image = imageptrs->ptrimagecomp_ch[0];
                m_pixmap = QPixmap::fromImage(*m_image);
            }
            break;
        case IMAGE_AVHRR_CH2:
            if(imageptrs->ptrimagecomp_ch[1]->sizeInBytes() == 0)
            {
                QString str("No AVHRR image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                displayAVHRRImageInfo();
                m_image = imageptrs->ptrimagecomp_ch[1];
                m_pixmap = QPixmap::fromImage(*m_image);
            }
            break;
        case IMAGE_AVHRR_CH3:
            if(imageptrs->ptrimagecomp_ch[2]->sizeInBytes() == 0)
            {
                QString str("No AVHRR image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                displayAVHRRImageInfo();
                m_image = imageptrs->ptrimagecomp_ch[2];
                m_pixmap = QPixmap::fromImage(*m_image);
            }
            break;
        case IMAGE_AVHRR_CH4:
            if(imageptrs->ptrimagecomp_ch[3]->sizeInBytes() == 0)
            {
                QString str("No AVHRR image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                displayAVHRRImageInfo();
                m_image = imageptrs->ptrimagecomp_ch[3];
                m_pixmap = QPixmap::fromImage(*m_image);
            }
            break;
        case IMAGE_AVHRR_CH5:
            if(imageptrs->ptrimagecomp_ch[4]->sizeInBytes() == 0)
            {
                QString str("No AVHRR image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                displayAVHRRImageInfo();
                m_image = imageptrs->ptrimagecomp_ch[4];
                m_pixmap = QPixmap::fromImage(*m_image);
            }
            break;
        case IMAGE_AVHRR_COL:
            if(imageptrs->ptrimagecomp_col->sizeInBytes() == 0)
            {
                QString str("No AVHRR image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                displayAVHRRImageInfo();
                m_image = imageptrs->ptrimagecomp_col;
                m_pixmap = QPixmap::fromImage(*m_image);
            }
            break;
        case IMAGE_AVHRR_EXPAND:
            if(imageptrs->ptrexpand_col->sizeInBytes() == 0)
            {
                QString str("No AVHRR image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                displayAVHRRImageInfo();
                m_image = imageptrs->ptrexpand_col;
                m_pixmap = QPixmap::fromImage(*m_image);
            }
            break;
        case IMAGE_GEOSTATIONARY:
            if(imageptrs->ptrimageGeostationary->sizeInBytes() == 0)
            {
                QString str("No geostationary image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                displayGeoImageInfo();
                m_image = imageptrs->ptrimageGeostationary;
                m_pixmap = QPixmap::fromImage(*m_image);

            }
            break;
        case IMAGE_PROJECTION:
            if(imageptrs->ptrimageProjection->sizeInBytes() == 0)
            {
                QString str("No Projection image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                //formtoolbox->writeInfoToTextEdit("Projection image");
                m_image = imageptrs->ptrimageProjection;
                m_pixmap = QPixmap::fromImage(*m_image);
            }
            break;
        case IMAGE_VIIRSM:
            if(imageptrs->ptrimageViirsM->sizeInBytes() == 0)
            {
                QString str("No VIIRS M image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                m_image = imageptrs->ptrimageViirsM;
                m_pixmap = QPixmap::fromImage(*m_image);
                if(segmenttype == eSegmentType::SEG_VIIRSM)
                    displayVIIRSImageInfo(SEG_VIIRSM);
                else if(segmenttype == eSegmentType::SEG_VIIRSMNOAA20)
                    displayVIIRSImageInfo(SEG_VIIRSMNOAA20);
            }
            break;
        case IMAGE_VIIRSDNB:
            if(imageptrs->ptrimageViirsDNB->sizeInBytes() == 0)
            {
                QString str("No VIIRS DNB image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                m_image = imageptrs->ptrimageViirsDNB;
                m_pixmap = QPixmap::fromImage(*m_image);
                if(segmenttype == eSegmentType::SEG_VIIRSDNB)
                    displayVIIRSImageInfo(SEG_VIIRSDNB);
                else if(segmenttype == eSegmentType::SEG_VIIRSDNBNOAA20)
                    displayVIIRSImageInfo(SEG_VIIRSDNBNOAA20);
            }
            break;
        case IMAGE_OLCI:
            if(imageptrs->ptrimageOLCI->sizeInBytes() == 0)
            {
                QString str("No OLCI image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                m_image = imageptrs->ptrimageOLCI;
                m_pixmap = QPixmap::fromImage(*m_image);
                this->displaySentinelImageInfo(imageptrs->olcitype);
            }
            break;
        case IMAGE_SLSTR:
            if(imageptrs->ptrimageSLSTR->sizeInBytes() == 0)
            {
                QString str("No SLSTR image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                m_image = imageptrs->ptrimageSLSTR;
                m_pixmap = QPixmap::fromImage(*m_image);
                this->displaySentinelImageInfo(SEG_SLSTR);
            }
            break;
        case IMAGE_MERSI:
            if(imageptrs->ptrimageMERSI->sizeInBytes() == 0)
            {
                QString str("No MERSI image");
                int pixelsWide = fm.horizontalAdvance(str);
                painter.drawText((pm.width() - pixelsWide)/2, pm.height()/2, str);
                m_pixmap = pm;
            }
            else
            {
                m_image = imageptrs->ptrimageMERSI;
                m_pixmap = QPixmap::fromImage(*m_image);
                this->displayMERSIImageInfo(SEG_MERSI);
            }
            break;

        }
    }

    m_pixmapItem = m_scene->addPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());
    this->centerOn(m_pixmapItem);

    m_ViewInitialized = true;

    if(resize)
        fitWindow();
    this->setDragMode(ScrollHandDrag);

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
    case SEG_NOAA19:
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

    if (type == SEG_NOAA19)
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
                      "<h4 style='color:blue'>Image Information</h4>"
                      "<p>Segment type = %1<br>"
                      "Nbr of segments = %2<br>"
                      "Image width = %3 height = %4<br>"
                      "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimagecomp_col->width()).arg(imageptrs->ptrimagecomp_col->height());


    formtoolbox->writeInfoToTextEdit(txtInfo);

}

//void FormImage::setupGeoOverlay(int geoindex)
//{

//    int col, save_col;
//    int row, save_row;
//    bool first = true;

//    double lat_deg;
//    double lon_deg;
//    int ret;
//    double sub_lon;

//    double fgf_x, fgf_y;

//    pixgeoConversion pixconv;

//    sub_lon = opts.geosatellites.at(geoindex).longitude;

//    for(int i = 0; i < 3; i++)
//        this->geooverlay[i].clear();

//    double scale_x = 0.000056;
//    double scale_y = -0.000056;
//    double offset_x = -0.151844;
//    double offset_y = 0.151844;
//    int sat = 1;

//    for(int k = 0; k < 3; k++)
//    {
//        if(opts.gshhsglobe1On)
//        {
//            first = true;

//            for (int i=0; i<gshhsdata->vxp_data_overlay[k]->nFeatures; i++)
//            {
//                for (int j=0; j<gshhsdata->vxp_data_overlay[k]->pFeatures[i].nVerts; j++)
//                {
//                    lat_deg = gshhsdata->vxp_data_overlay[k]->pFeatures[i].pLonLat[j].latmicro*1.0e-6;
//                    lon_deg = gshhsdata->vxp_data_overlay[k]->pFeatures[i].pLonLat[j].lonmicro*1.0e-6;
//                    if (lon_deg > 180.0)
//                        lon_deg -= 360.0;

//                    if(lon_deg < 90.0 || lon_deg > -90.0)
//                    {
//                        if(opts.geosatellites.at(geoindex).shortname == "GOES_16" || opts.geosatellites.at(geoindex).shortname == "GOES_17")
//                        {
//                            pixconv.earth_to_fgf_(&sat, &lon_deg, &lat_deg, &scale_x, &offset_x, &scale_y, &offset_y, &sub_lon, &fgf_x, &fgf_y);
//                            if(fgf_x >= 0 && fgf_x < opts.geosatellites.at(geoindex).imagewidth && fgf_y >= 0 && fgf_y < opts.geosatellites.at(geoindex).imageheight)
//                            {
//                                col = (int)fgf_x;
//                                row = (int)fgf_y;
//                                ret = 0;
//                            }
//                            else
//                                ret = 1;
//                        }
//                        else
//                        {
//                            ret = pixconv.geocoord2pixcoord(sub_lon, lat_deg, lon_deg, opts.geosatellites.at(geoindex).coff,
//                                                            opts.geosatellites.at(geoindex).loff, opts.geosatellites.at(geoindex).cfac, opts.geosatellites.at(geoindex).lfac, &col, &row);

//                        }
//                        if(ret == 0)
//                        {
//                            if (first)
//                            {
//                                first = false;
//                                save_col = col;
//                                save_row = row;
//                                this->geooverlay[k].append(QVector2D(-1, -1));
//                            }
//                            else
//                            {
//                                save_col = col;
//                                save_row = row;
//                            }

//                            this->geooverlay[k].append(QVector2D(col, row));

//                        }
//                        else
//                            first = true;
//                    }
//                }
//                first = true;
//            }
//        }
//    }
//}

void FormImage::setupGeoOverlay(int geoindex)
{
    for(int i = 0; i < 3; i++)
        this->geooverlay[i].clear();

    setupGshhs(geoindex, 0);
    setupGshhs(geoindex, 1);
    setupGshhs(geoindex, 2);

}

void FormImage::setupGshhs(int geoindex, int k)
{
    bool first = true;
    double lat_deg;
    double lon_deg;
    int ret;
    pixgeoConversion pixconv;
    int col, save_col;
    int row, save_row;
    double sub_lon;
    double fgf_x, fgf_y;

    double scale_x = 0.000056;
    double scale_y = -0.000056;
    double offset_x = -0.151844;
    double offset_y = 0.151844;
    int sat = 1;

    sub_lon = opts.geosatellites.at(geoindex).longitude;

    if(!(opts.geosatellites.at(geoindex).shortname == "GOES_16" || opts.geosatellites.at(geoindex).shortname == "GOES_17" ||
         opts.geosatellites.at(geoindex).shortname == "GOES_18" ||
         opts.geosatellites.at(geoindex).shortname == "FY2H" || opts.geosatellites.at(geoindex).shortname == "FY2G"))
        geoindex = 11;

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
                if(opts.geosatellites.at(geoindex).shortname == "GOES_16" || opts.geosatellites.at(geoindex).shortname == "GOES_17" || opts.geosatellites.at(geoindex).shortname == "GOES_18")
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
                        this->geooverlay[k].append(QVector2D(-1, -1));
                    }
                    else
                    {
                        save_col = col;
                        save_row = row;
                    }

                    this->geooverlay[k].append(QVector2D(col, row));

                }
                else
                    first = true;
            }
        }
        first = true;
    }
}
void FormImage::drawForeground(QPainter *painter, const QRectF &rect)
{

    //qDebug() << "BEFORE FormImage::drawForeground ViewInitialized = " << m_ViewInitialized;

    if(!m_ViewInitialized)
        return;

    if(m_image == NULL)
        return;

    if(m_image->isNull())
        return;

    drawOverlays(painter);

    //    painter->translate(m_image->width()/2, m_image->height()/2);

    //    int diameter = 3580;
    //    painter->setPen(QPen(QColor(255, 0, 0), 1));
    //    painter->drawEllipse(QRect(-diameter / 2, -diameter / 2, diameter, diameter));

    //    diameter = 3650;
    //    painter->setPen(QPen(QColor(0, 255, 255), 1));
    //    painter->drawEllipse(QRect(-diameter / 2, -diameter / 2, diameter, diameter));

    //    diameter = 3615;
    //    painter->setPen(QPen(QColor(0, 0, 255), 30));
    //    painter->drawEllipse(QRect(-diameter / 2, -diameter / 2, diameter, diameter));


    //qDebug() << "AFTER FormImage::drawForeground";

}

void FormImage::savePNGImage(QString fileName)
{

    QImage image_copy = m_image->copy();
    QPainter painter(&image_copy);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);


    painter.begin(&image_copy);
    //getScene()->render(&painter, m_image->rect() );

    drawOverlays(&painter);

    painter.end();

    image_copy.save(fileName);


}

void FormImage::drawOverlays(QPainter *painter)
{
    //qDebug() << "FormImage::drawOverlays(QPainter *painter)";

    painter->setPen(opts.geoimageoverlaycolor1);
    //painter->drawRect(QRect(0, 0, 3711, 3711));

    QFont font = painter->font();


    SegmentListGeostationary *slgeo = NULL;

    slgeo = segs->getActiveSegmentList();

    if (channelshown == IMAGE_GEOSTATIONARY && overlaymeteosat)
    {
        if(slgeo != NULL)
        {
            if(this->m_image == NULL)
                return;

            if(m_image->width() <= 3712)
                font.setPixelSize(50);
            else if(m_image->width() == 5568)
                font.setPixelSize(100);
            else if(m_image->width() > 9000)
                font.setPixelSize(150);
            else
                font.setPixelSize(100);

            painter->setFont(font);

            painter->drawText(0, font.pixelSize(), slgeo->geosatname);
            QStringList rowchosen = formtoolbox->getRowchosen();
            QVector<QString> spectrumvector = formtoolbox->getSpectrumVector();
            QVector<bool> inversevector = formtoolbox->getInverseVector();
            //            painter->drawText(m_image->width() - (font.pixelSize() == 50 ? 200 : 600), font.pixelSize(), spectrumvector.at(0));
            //            painter->drawText(m_image->width() - (font.pixelSize() == 50 ? 200 : 600), 2 * font.pixelSize(), spectrumvector.at(1));
            //            painter->drawText(m_image->width() - (font.pixelSize() == 50 ? 200 : 600), 3 * font.pixelSize(), spectrumvector.at(2));

            QFontMetrics fm(font);
            int pixelsWide = fm.horizontalAdvance(spectrumvector.at(0));
            int pixelsHigh = fm.height();

            painter->drawText(m_image->width() - pixelsWide, font.pixelSize(), spectrumvector.at(0));
            painter->drawText(m_image->width() - pixelsWide, 2 * font.pixelSize(), spectrumvector.at(1));
            painter->drawText(m_image->width() - pixelsWide, 3 * font.pixelSize(), spectrumvector.at(2));

            // "2023-06-02   07:15"
            if(!rowchosen.isEmpty())
            {
                painter->drawText(0, 2 * font.pixelSize(), rowchosen[0] );
            }

            if(slgeo->getGeoSatellite() == eGeoSatellite::H9)
                this->OverlayGeostationaryH8(painter, slgeo);
            else
                this->OverlayGeostationary(painter, slgeo);
        }
    }

    if(channelshown == IMAGE_PROJECTION && overlayprojection)
    {
        this->OverlayProjection(painter);
    }

    if(channelshown == IMAGE_OLCI && overlayolci)
    {
        this->OverlayOLCI(painter);
    }
}

void FormImage::OverlayGeostationary(QPainter *paint, SegmentListGeostationary *sl)
{

    if(m_image == NULL){
        qDebug() << "m_image is null";
        return;
    }

    //qDebug() << "FormImage::OverlayGeostationary(QPainter *paint, SegmentListGeostationary *sl) image width = " << m_image->width();
    //    "getGeoSatellite = " << sl->getGeoSatellite();

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

        //                if(sl->getGeoSatellite() == eGeoSatellite::H8)
        //                {
        //                    QPoint pt(opts.geosatellites.at(geoindex).coff, opts.geosatellites.at(geoindex).loff);
        //                    paint->setPen(qRgb(0, 0, 255));
        //                    paint->drawEllipse(pt, opts.geosatellites.at(geoindex).coff - 28, opts.geosatellites.at(geoindex).loff - 40);
        //                }
    }
    else
        return;


    coff = hrvimage ? opts.geosatellites.at(geoindex).coffhrv : opts.geosatellites.at(geoindex).coff;
    loff = hrvimage ? opts.geosatellites.at(geoindex).loffhrv : opts.geosatellites.at(geoindex).loff;
    cfac = hrvimage ? opts.geosatellites.at(geoindex).cfachrv : opts.geosatellites.at(geoindex).cfac;
    lfac = hrvimage ? opts.geosatellites.at(geoindex).lfachrv : opts.geosatellites.at(geoindex).lfac;

    if(sl->getGeoSatellite() == eGeoSatellite::MTG_I1)
    {
        coff = m_image->width() == 11136 ? opts.geosatellites.at(geoindex).coffhrv : opts.geosatellites.at(geoindex).coff;
        loff = m_image->width() == 11136 ? opts.geosatellites.at(geoindex).loffhrv : opts.geosatellites.at(geoindex).loff;
        cfac = m_image->width() == 11136 ? opts.geosatellites.at(geoindex).cfachrv : opts.geosatellites.at(geoindex).cfac;
        lfac = m_image->width() == 11136 ? opts.geosatellites.at(geoindex).lfachrv : opts.geosatellites.at(geoindex).lfac;
    }

    qDebug() << "coff = " << coff << " loff = " << loff << " cfac = " << cfac << " lfac = " << lfac;

    double sub_lon = sl->geosatlon;
    lat_deg = opts.obslat;
    lon_deg = opts.obslon;
    if (lon_deg > 180.0)
        lon_deg -= 360.0;

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
        paint->drawEllipse(pt, 12, 12);
    }

    paint->setPen(QColor(opts.projectionoverlaylonlatcolor));

    for(double lon = -180.0; lon < 180.0; lon+=10.0)
    {
        first = true;
        {
            for(double lat = -90.0; lat < 90.0; lat+=0.5)
            {
                ret =pixconv.geocoord2pixcoord(sub_lon, lat, lon, coff, loff, cfac, lfac, &col, &row);
                if(hrvimage && sl->getGeoSatellite() != eGeoSatellite::FY2H && sl->getGeoSatellite() != eGeoSatellite::FY2G)
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
                        if(row < m_image->height() && col < m_image->width() && row > -1 && col > -1)
                            paint->drawLine(save_col, save_row, col, row);
                        save_col = col;
                        save_row = row;
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
                ret =pixconv.geocoord2pixcoord(sub_lon, lat, lon, coff, loff, cfac, lfac, &col, &row);
                if(hrvimage && sl->getGeoSatellite() != eGeoSatellite::FY2H && sl->getGeoSatellite() != eGeoSatellite::FY2G)
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
                        if(row < m_image->height() && col < m_image->width() && row > -1 && col > -1)
                            paint->drawLine(save_col, save_row, col, row);
                        save_col = col;
                        save_row = row;
                    }
                }
                else
                    first = true;

            }
        }
    }

    float factor = 1.0;
    if(m_image->width() == 3712)
        factor = 1.5; //3.0;
    else if(m_image->width() == 5568)
        factor = 1.0;
    else if(m_image->width() == 11136)
        factor = 0.5;
    else if(m_image->width() == 2288) // FY
        factor = 1.0;
    else if(m_image->width() == 9152) // FY - VIS
        factor = 0.25;
    else if(m_image->width() == 2784) // Electro
        factor = 2.0; //4.0;
    else if(m_image->width() == 5500) // Himawari-8
        factor = 1.975574713;


    if(!hrvimage || (sl->getGeoSatellite() == eGeoSatellite::FY2H || sl->getGeoSatellite() == eGeoSatellite::FY2G))
    {
        for(int k = 0; k < 3; k++)
        {
            if(k == 0) paint->setPen(opts.geoimageoverlaycolor1); //QColor(opts.projectionoverlaylonlatcolor));
            if(k == 1) paint->setPen(opts.geoimageoverlaycolor2); //QColor(opts.projectionoverlaylonlatcolor));
            if(k == 2) paint->setPen(opts.geoimageoverlaycolor3); //QColor(opts.projectionoverlaylonlatcolor));

            for(int i = 0; i < this->geooverlay[k].count(); i++)
            {
                if (this->geooverlay[k].at(i).x() < 0)
                {
                    first = true;
                }
                else if(first == true)
                {
                    first = false;
                    save_col = (int)this->geooverlay[k].at(i).x()/factor;
                    save_row = (int)this->geooverlay[k].at(i).y()/factor;
                }
                else
                {
                    col = this->geooverlay[k].at(i).x()/factor;
                    row = this->geooverlay[k].at(i).y()/factor;
                    if(row < m_image->height() && col < m_image->width())
                        paint->drawLine(save_col, save_row, col, row);

                    save_col = (int)this->geooverlay[k].at(i).x()/factor;
                    save_row = (int)this->geooverlay[k].at(i).y()/factor;
                }
            }
        }
    }
    else
        OverlayGeostationaryHRV1(paint, sl, geoindex);
}

void FormImage::DrawLongLat(QPainter *paint, SegmentListGeostationary *sl, int coff, int loff, double cfac, double lfac, bool hrvimage)
{
    int col, save_col;
    int row, save_row;
    bool first = true;

    double lat_deg;
    double lon_deg;
    int ret;

    pixgeoConversion pixconv;

    double sub_lon = sl->geosatlon;
    lat_deg = opts.obslat;
    lon_deg = opts.obslon;
    if (lon_deg > 180.0)
        lon_deg -= 360.0;

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
        paint->drawEllipse(pt, 12, 12);
    }

    paint->setPen(QColor(opts.projectionoverlaylonlatcolor));

    for(double lon = -180.0; lon < 180.0; lon+=10.0)
    {
        first = true;
        {
            for(double lat = -90.0; lat < 90.0; lat+=0.5)
            {
                ret =pixconv.geocoord2pixcoord(sub_lon, lat, lon, coff, loff, cfac, lfac, &col, &row);
                if(hrvimage && sl->getGeoSatellite() != eGeoSatellite::FY2H && sl->getGeoSatellite() != eGeoSatellite::FY2G)
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
                        if(row < m_image->height() && col < m_image->width() && row > -1 && col > -1)
                            paint->drawLine(save_col, save_row, col, row);
                        save_col = col;
                        save_row = row;
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
                ret =pixconv.geocoord2pixcoord(sub_lon, lat, lon, coff, loff, cfac, lfac, &col, &row);
                if(hrvimage && sl->getGeoSatellite() != eGeoSatellite::FY2H && sl->getGeoSatellite() != eGeoSatellite::FY2G)
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
                        if(row < m_image->height() && col < m_image->width() && row > -1 && col > -1)
                            paint->drawLine(save_col, save_row, col, row);
                        save_col = col;
                        save_row = row;
                    }
                }
                else
                    first = true;

            }
        }
    }


}

void FormImage::OverlayGeostationaryHRV1(QPainter *paint, SegmentListGeostationary *sl, int geoindex)
{

    long coff;
    long loff;
    double cfac;
    double lfac;

    int col = 0, save_col = 0;
    int row = 0, save_row = 0;
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

    for(int index = 0; index < 3; index++)
    {
        first = true;
        for(int i = 0; i < this->geooverlay[index].count(); i++)
        {
            col = (int)this->geooverlay[index].at(i).x();
            row = (int)this->geooverlay[index].at(i).y();

            if(sl->getGeoSatellite() != eGeoSatellite::FY2H && sl->getGeoSatellite() != eGeoSatellite::FY2G)
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

            if (this->geooverlay[index].at(i).x() < 0)
            {
                first = true;
            }
            else if(first == true)
            {
                first = false;
                save_col = col;
                save_row = row;
            }
            else
            {
                if(index == 0)
                    paint->setPen(opts.geoimageoverlaycolor1);
                else if(index == 1)
                    paint->setPen(opts.geoimageoverlaycolor2);
                else if(index == 2)
                    paint->setPen(opts.geoimageoverlaycolor3);

                if(row < m_image->height() && col < m_image->width() && row > -1 && col > -1)
                    paint->drawLine(save_col, save_row, col, row);
                save_col = col;
                save_row = row;
            }
        }
    }



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
                    row+=5; //3;
                    col+=3; //2;

                    if(ret == 0)
                    {
                        if(sl->getGeoSatellite() != eGeoSatellite::FY2H && sl->getGeoSatellite() != eGeoSatellite::FY2G)
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
                            if(col < imageptrs->ptrimageGeostationary->width() && row < imageptrs->ptrimageGeostationary->height() &&
                                    col >= 0 && row >= 0)
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
                    row+=5; //3;
                    col+=3; //2;

                    if(ret == 0)
                    {
                        if(sl->getGeoSatellite() != eGeoSatellite::FY2H && sl->getGeoSatellite() != eGeoSatellite::FY2G)
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
                            if(col < imageptrs->ptrimageGeostationary->width() && row < imageptrs->ptrimageGeostationary->height() &&
                                    col >= 0 && row >= 0)
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
                    row+=5; //3;
                    col+=3; //2;

                    if(ret == 0)
                    {
                        if(sl->getGeoSatellite() != eGeoSatellite::FY2H && sl->getGeoSatellite() != eGeoSatellite::FY2G)
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
                            if(col < imageptrs->ptrimageGeostationary->width() && row < imageptrs->ptrimageGeostationary->height() &&
                                    col >= 0 && row >= 0)
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

void FormImage::OverlayGeostationaryH8(QPainter *paint, SegmentListGeostationary *sl)
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

    //qDebug() << "FormImage::OverlayGeostationaryH8";

    int geoindex = sl->getGeoSatelliteIndex();
    pixgeoConversion pixconv;

    //if(sl->LowerNorthLineActual == 0)
    //    return;

    coff = opts.geosatellites.at(geoindex).coff;
    loff = opts.geosatellites.at(geoindex).loff;
    cfac = opts.geosatellites.at(geoindex).cfac;
    lfac = opts.geosatellites.at(geoindex).lfac;


    double sub_lon = sl->geosatlon;

    this->DrawLongLat(paint, sl, coff, loff, cfac, lfac, false);

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
                //                    row+=5; //3;
                //                    col+=3; //2;

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
                        paint->setPen(opts.geoimageoverlaycolor1);
                        if(col < imageptrs->ptrimageGeostationary->width() && row < imageptrs->ptrimageGeostationary->height() &&
                                col >= 0 && row >= 0)
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
                //                    row+=5; //3;
                //                    col+=3; //2;

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
                        paint->setPen(opts.geoimageoverlaycolor2);
                        if(col < imageptrs->ptrimageGeostationary->width() && row < imageptrs->ptrimageGeostationary->height() &&
                                col >= 0 && row >= 0)
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
                //                    row+=5; //3;
                //                    col+=3; //2;

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
                        paint->setPen(opts.geoimageoverlaycolor3);
                        if(col < imageptrs->ptrimageGeostationary->width() && row < imageptrs->ptrimageGeostationary->height() &&
                                col >= 0 && row >= 0)
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


    //this->update();
}

void FormImage::OverlayProjection(QPainter *paint)
{
    //qDebug() << QString("FormImage::OverlayProjection(QPainter *paint, SegmentListGeostationary *sl) opts.currenttoolbox = %1").arg(opts.currenttoolbox);
    if(!paint->isActive())
        return;

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
    else if (opts.currenttoolbox == 2)      //SG
        bret = imageptrs->sg->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;
    else if (opts.currenttoolbox == 3)      //OM
        bret = imageptrs->om->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;
    else
        bret = false;

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

    //    if(opts.gshhsglobe1On)
    //    {
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
            else if (opts.currenttoolbox == 2)  //SG
                bret = imageptrs->sg->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y);
            else if (opts.currenttoolbox == 3)  //OM
                bret = imageptrs->om->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y);
            else
                bret = false;

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
                    if(abs(save_map_y - map_y) < 100 && (abs(save_map_x - map_x) < 100))
                    {
                        paint->setPen(QColor(opts.projectionoverlaycolor1));
                        paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                    }
                    save_map_x = map_x;
                    save_map_y = map_y;
                }
            }
            else
                first = true;
        }
        first = true;
    }
    //    }

    first = true;

    //    if(opts.gshhsglobe2On)
    //    {
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
            else if (opts.currenttoolbox == 2)  //SG
                bret = imageptrs->sg->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;
            else if (opts.currenttoolbox == 3)  //OM
                bret = imageptrs->om->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;
            else
                bret = false;

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
                    if(abs(save_map_y - map_y) < 100 && (abs(save_map_x - map_x) < 100))
                    {
                        paint->setPen(QColor(opts.projectionoverlaycolor2));
                        paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                    }
                    save_map_x = map_x;
                    save_map_y = map_y;
                }
            }
            else
                first = true;
        }
        first = true;
    }
    //    }

    first = true;

    //    if(opts.gshhsglobe3On)
    //    {
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
            else if (opts.currenttoolbox == 2)  //SG
                bret = imageptrs->sg->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;
            else if (opts.currenttoolbox == 3)  //OM
                bret = imageptrs->om->map_forward( lon_deg*PI/180, lat_deg*PI/180, map_x, map_y) ;
            else
                bret = false;

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
                    if(abs(save_map_y - map_y) < 100 && (abs(save_map_x - map_x) < 100))
                    {
                        paint->setPen(QColor(opts.projectionoverlaycolor3));
                        paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                    }
                    save_map_x = map_x;
                    save_map_y = map_y;
                }
            }
            else
                first = true;
        }
        first = true;
    }
    //    }

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

    if (opts.currenttoolbox == 3)  //OM
    {
        QPolygonF poly;
        if(opts.buttonMERSI)
            segs->seglmersi->GetContourPolygon(&poly);
        else if(opts.buttonVIIRSM)
            segs->seglviirsm->GetContourPolygon(&poly);
        else if(opts.buttonVIIRSMNOAA20)
            segs->seglviirsmnoaa20->GetContourPolygon(&poly);
        else if(opts.buttonMetop)
            segs->seglmetop->GetContourPolygonAVHRR(&poly);

        for(int i = 0; i < poly.size(); i++)
        {
            bret = imageptrs->om->map_forward( poly.at(i).x()*PI/180, poly.at(i).y()*PI/180, map_x, map_y);
            if(bret)
            {
                paint->setPen(QColor(255, 0, 255));
                paint->drawPoint(map_x, map_y);
            }
        }

        if(opts.sattrackinimage)
        {
            QPolygonF track;
            if(opts.buttonMERSI)
                segs->seglmersi->GetTrackPolygon(&track);
            else if(opts.buttonVIIRSM)
                segs->seglviirsm->GetTrackPolygon(&track);
            else if(opts.buttonVIIRSMNOAA20)
                segs->seglviirsmnoaa20->GetTrackPolygon(&track);
            else if(opts.buttonMetop)
                segs->seglmetop->GetTrackPolygonAVHRR(&track);

            first = true;
            for(int i = 0; i < track.size(); i++)
            {
                bret = imageptrs->om->map_forward( track.at(i).x()*PI/180, track.at(i).y()*PI/180, map_x, map_y);
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
                        paint->setPen(QColor(0, 0, 255));
                        paint->drawLine(save_map_x, save_map_y, map_x, map_y);
                        save_map_x = map_x;
                        save_map_y = map_y;
                    }
                }
            }
        }
    }

    if (opts.currenttoolbox == 3 && formtoolbox->GridOnProjOM() ) //OM
    {

        first = true;

        for(double lon = -180.0; lon < 180.0; lon+=10.0)
        {
            first = true;
            {
                for(double lat = -80.0; lat < 81.0; lat+=0.5)
                {
                    bret = imageptrs->om->map_forward( lon*PI/180, lat*PI/180, map_x, map_y);

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
                            //                            paint->drawPoint(map_x, map_y);
                            if(abs(save_map_x - map_x) < 200 && abs(save_map_y - map_y) < 200)
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
                    bret = imageptrs->om->map_forward( lon*PI/180, lat*PI/180, map_x, map_y);

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
                            //paint->drawPoint(map_x, map_y);
                            if(abs(save_map_x - map_x) < 200 && abs(save_map_y - map_y) < 200)
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
    else
        return;
    qDebug() << "FormImage::OverlayOLCI(QPainter *paint) 2";

    if(opts.buttonOLCIefr)
        sl = segs->seglolciefr;
    else if(opts.buttonOLCIerr)
        sl = segs->seglolcierr;
    else return;

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

    qDebug() << "end FormImage::OverlayOLCI(QPainter *paint)";

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
    case SEG_VIIRSMNOAA20:
        segtype = "VIIRSMNOAA20";
        nbrselected = segs->seglviirsmnoaa20->NbrOfSegmentsSelected();

        break;
    case SEG_VIIRSDNBNOAA20:
        segtype = "VIIRSDNBNOAA20";
        nbrselected = segs->seglviirsdnbnoaa20->NbrOfSegmentsSelected();
        moonillum = segs->seglviirsdnbnoaa20->getMoonIllumination();

        break;
    default:
        segtype = "NA";
        break;
    }


    if(type == SEG_VIIRSM || type == SEG_VIIRSMNOAA20)
    {
        txtInfo = QString("<!DOCTYPE html>"
                          "<html><head><title>Info</title></head>"
                          "<body>"
                          "<h4 style='color:blue'>Image Information</h4>"
                          "<p>Segment type = %1<br>"
                          "Nbr of segments = %2<br>"
                          "Image width = %3 height = %4<br>"
                          "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimageViirsM->width()).arg(imageptrs->ptrimageViirsM->height());
        formtoolbox->writeInfoToTextEdit(txtInfo);

    } else
        if(type == SEG_VIIRSDNB || type == SEG_VIIRSDNBNOAA20)
        {
            txtInfo = QString("<!DOCTYPE html>"
                              "<html><head><title>Info</title></head>"
                              "<body>"
                              "<h4 style='color:blue'>Image Information</h4>"
                              "<p>Segment type = %1<br>"
                              "Nbr of segments = %2<br>"
                              "Image width = %3 height = %4<br>"
                              "Moon illumination = %5 %</p>"
                              "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimageViirsDNB->width())
                    .arg(imageptrs->ptrimageViirsDNB->height()).arg(moonillum, 4, 'f', 2);
            formtoolbox->writeInfoToTextEdit(txtInfo);

        }


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
                          "<h4 style='color:blue'>Image Information</h4>"
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
                          "<h4 style='color:blue'>Image Information</h4>"
                          "<p>Segment type = %1<br>"
                          "Nbr of segments = %2<br>"
                          "Image width = %3 height = %4<br>"
                          "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimageSLSTR->width()).arg(imageptrs->ptrimageSLSTR->height());
        formtoolbox->writeInfoToTextEdit(txtInfo);
    }

}

void FormImage::displayMERSIImageInfo(eSegmentType type)
{
    QString segtype;
    int nbrselected;

    segtype = "MERSI";
    nbrselected = segs->seglmersi->NbrOfSegmentsSelected();

    txtInfo = QString("<!DOCTYPE html>"
                      "<html><head><title>Info</title></head>"
                      "<body>"
                      "<h4 style='color:blue'>Image Information</h4>"
                      "<p>Segment type = %1<br>"
                      "Nbr of segments = %2<br>"
                      "Image width = %3 height = %4<br>"
                      "</body></html>").arg(segtype).arg(nbrselected).arg(imageptrs->ptrimageMERSI->width()).arg(imageptrs->ptrimageMERSI->height());
    formtoolbox->writeInfoToTextEdit(txtInfo);


}

bool FormImage::ShowVIIRSMImage()
{
    bool ret = false;

    if(opts.buttonVIIRSM)
        viirsmcount = segs->seglviirsm->NbrOfSegmentsSelected();
    else if(opts.buttonVIIRSMNOAA20)
        viirsmcount = segs->seglviirsmnoaa20->NbrOfSegmentsSelected();

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    qDebug() << QString("in FormImage::ShowVIIRSImage nbr of viirs segments selected = %1").arg(viirsmcount);

    if (viirsmcount > 0)
    {

        ret = true;
        displayImage(IMAGE_VIIRSM, true);

        emit allsegmentsreceivedbuttons(false);

        this->kindofimage = "VIIRSM";

        bandlist = formtoolbox->getVIIRSMBandList();
        colorlist = formtoolbox->getVIIRSMColorList();
        invertlist = formtoolbox->getVIIRSMInvertList();

        if(opts.buttonVIIRSM)
            segs->seglviirsm->ShowImageSerial(bandlist, colorlist, invertlist);
        else if(opts.buttonVIIRSMNOAA20)
            segs->seglviirsmnoaa20->ShowImageSerial(bandlist, colorlist, invertlist);
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
        displayImage(IMAGE_VIIRSDNB, true);

        emit allsegmentsreceivedbuttons(false);

        this->kindofimage = "VIIRSDNB";

        segs->seglviirsdnb->ShowImageSerial();
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

        QStringList missing;
        segs->seglolciefr->setHistogramMethod(histogrammethod, normalized);
        if(segs->seglolciefr->CheckForOLCIFiles(bandlist, colorlist, missing) == false)  // parameter false = no decompression of the files
        {
            formtoolbox->setToolboxButtons(true);

            emit setmapcylbuttons(true);

            QMessageBox msgBox;
            QString txt = "In one or more segments, the following files are missing : \n";
            for(int i = 0; i < missing.count(); i++)
            {
                txt.append(missing.at(i) + "\n");
            }
            msgBox.setText(txt);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            int ret = msgBox.exec();

            switch (ret) {
            case QMessageBox::Ok:
                break;
            default:
                break;
            }
            return false;
        }
        else
            segs->seglolciefr->ComposeOLCIImage(bandlist, colorlist, invertlist, false, histogrammethod, normalized);

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
        displayImage(IMAGE_OLCI, true);

        emit allsegmentsreceivedbuttons(false);

        this->kindofimage = "OLCIERR";

        bandlist = formtoolbox->getOLCIBandList();
        colorlist = formtoolbox->getOLCIColorList();
        invertlist = formtoolbox->getOLCIInvertList();

        QStringList missing;
        segs->seglolcierr->setHistogramMethod(histogrammethod, normalized);
        if(segs->seglolcierr->CheckForOLCIFiles(bandlist, colorlist, missing) == false)
        {
            formtoolbox->setToolboxButtons(true);

            emit setmapcylbuttons(true);

            QMessageBox msgBox;
            QString txt = "In one or more segments, the following files are missing : \n";
            for(int i = 0; i < missing.count(); i++)
            {
                txt.append(missing.at(i) + "\n");
            }
            msgBox.setText(txt);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            int ret = msgBox.exec();

            switch (ret) {
            case QMessageBox::Ok:
                break;
            default:
                break;
            }
            return false;
        }
        else
            segs->seglolcierr->ComposeOLCIImage(bandlist, colorlist, invertlist, false, histogrammethod, normalized);

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
        displayImage(IMAGE_SLSTR, true);

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

bool FormImage::ShowMERSIImage(int histogrammethod, bool normalized)
{
    bool ret = false;

    mersicount = segs->seglmersi->NbrOfSegmentsSelected();

    QList<bool> bandlist;
    QList<int> colorlist;
    QList<bool> invertlist;

    qDebug() << QString("in FormImage::ShowMERSIImage nbr of mersi segments selected = %1").arg(mersicount);

    if (mersicount > 0)
    {

        ret = true;
        displayImage(IMAGE_MERSI, true);

        emit allsegmentsreceivedbuttons(false);

        this->kindofimage = "MERSI";

        bandlist = formtoolbox->getMERSIBandList(); // 16 items
        colorlist = formtoolbox->getMERSIColorList(); // 15 items
        invertlist = formtoolbox->getMERSIInvertList();

        //        qDebug()<< "FormImage::ShowMERSIImage() bandlist";
        //        for(int i = 0; i < 16; i++)
        //        {
        //            qDebug() << bandlist.at(i);
        //        }

        //        qDebug()<< "FormImage::ShowMERSIImage() colorlist";
        //        for(int i = 0; i < 15; i++)
        //        {
        //            qDebug() << colorlist.at(i);
        //        }

        segs->seglmersi->setHistogramMethod(histogrammethod, normalized);
        segs->seglmersi->ComposeMERSIImage(bandlist, colorlist, invertlist, false, histogrammethod, normalized);
    }
    else
        ret = false;

    return ret;
}

bool FormImage::ShowAVHRRImage(int histogrammethod, bool normalized)
{
    bool ret = false;

    this->kindofimage = "AVHRR Color";
    this->channelshown = IMAGE_AVHRR_COL;

    if(segmenttype == SEG_METOP)
        metopcount = segs->seglmetop->NbrOfSegmentsSelected();
    else if(segmenttype == SEG_NOAA19)
        noaacount = segs->seglnoaa->NbrOfSegmentsSelected();
    else if(segmenttype == SEG_HRP)
        hrpcount = segs->seglhrp->NbrOfSegmentsSelected();
    else if(segmenttype == SEG_GAC)
        gaccount = segs->seglgac->NbrOfSegmentsSelected();

    if (metopcount > 0 || noaacount > 0 || hrpcount > 0  || gaccount > 0)
    {
        ret = true;

        if(segmenttype == SEG_METOP)
        {
            segs->seglmetop->setHistogramMethod(histogrammethod);
            segs->seglmetop->UpdateAVHRRImageInThread();
        }
        else if(segmenttype == SEG_NOAA19)
        {
            segs->seglnoaa->setHistogramMethod(histogrammethod);
            segs->seglnoaa->UpdateAVHRRImageInThread();
        }
        else if(segmenttype == SEG_HRP)
        {
            segs->seglhrp->setHistogramMethod(histogrammethod);
            segs->seglhrp->UpdateAVHRRImageInThread();
        }
        else if(segmenttype == SEG_GAC)
        {
            segs->seglgac->setHistogramMethod(histogrammethod);
            segs->seglgac->UpdateAVHRRImageInThread();
        }
    }
    else
        ret = false;

    return ret;
}

void FormImage::recalculateCLAHEGeo(QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    SegmentListGeostationary *sl;

    sl = segs->getActiveSegmentList();
    if(sl == NULL)
        return;

    //    if(sl->getGeoSatellite() == eGeoSatellite::MTG_I1)
    //        recalculateCLAHEMTG(spectrumvector, inversevector);
    //    else
    recalculateCLAHEMeteosat1(spectrumvector, inversevector);
}

void FormImage::recalculateCLAHEMeteosat1(QVector<QString> spectrumvector, QVector<bool> inversevector)
{
    QRgb *row_col;
    quint16 cred, cgreen, cblue;
    QRgb c;

    double *L;
    double *a;
    double *b;

    double Lu;
    double a_lab;
    double b_lab;
    size_t npix;

    ushort *pixelsL;

    uint numberOfBytes;

    SegmentListGeostationary *sl;

    sl = segs->getActiveSegmentList();

    QApplication::setOverrideCursor( Qt::WaitCursor ); // this might take time

    formtoolbox->setProgressValue(10);


    int width = imageptrs->ptrimageGeostationary->width();
    int height = imageptrs->ptrimageGeostationary->height();
    npix = width*height;
    numberOfBytes = static_cast<uint>(imageptrs->ptrimageGeostationary->sizeInBytes());

    L = new double[width*height];
    a = new double[width*height];
    b = new double[width*height];


    qDebug() << Q_FUNC_INFO << "image width = " << width << " height = " << height << " numberofbytes = " << numberOfBytes << " npix = " << npix;

    ColorSpace::Rgb srcColor;
    ColorSpace::Lab dstColor;


    for (int line = height - 1; line >= 0; line--)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            c = row_col[pixelx];
            srcColor.r = qRed(c);
            srcColor.g = qGreen(c);
            srcColor.b = qBlue(c);

            srcColor.To<ColorSpace::Lab>(&dstColor);

            dstColor.l = (dstColor.l < 0.0 ? 0.0 : dstColor.l);
            dstColor.l = (dstColor.l > 100.0 ? 100.0 : dstColor.l);
            dstColor.a = (dstColor.a < -128.0 ? -128.0 : dstColor.a);
            dstColor.a = (dstColor.a > 128.0 ? 128.0 : dstColor.a);
            dstColor.b = (dstColor.b < -128.0 ? -128.0 : dstColor.b);
            dstColor.b = (dstColor.b > 128.0 ? 128.0 : dstColor.b);
            L[line * width + pixelx] = dstColor.l;
            a[line * width + pixelx] = dstColor.a;
            b[line * width + pixelx] = dstColor.b;

        }
    }

    formtoolbox->setProgressValue(30);

    pixelsL = new ushort[npix];

    for (int line = height - 1; line >= 0; line--)
    {
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            pixelsL[line * width + pixelx] = (ushort)qRound(L[line * width + pixelx] * 255.0 / 100.0);
            pixelsL[line * width + pixelx] = (pixelsL[line * width + pixelx] > 255 ? 255 : pixelsL[line * width + pixelx]);
            pixelsL[line * width + pixelx] = (pixelsL[line * width + pixelx] > 255 ? 255 : pixelsL[line * width + pixelx]);

        }
    }

    formtoolbox->setProgressValue(60);

    if(sl->getGeoSatellite() == eGeoSatellite::H9)
        imageptrs->CLAHE(pixelsL, width, height, 0, 255, 10, 10, 256, opts.clahecliplimit);
    else
        imageptrs->CLAHE(pixelsL, width, height, 0, 255, 16, 16, 256, opts.clahecliplimit);

    for (int line = height - 1; line >= 0; line--)
    {
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            L[line * width + pixelx] = (double)(pixelsL[line * width + pixelx] * 100.0 / 255.0);
            L[line * width + pixelx] = (L[line * width + pixelx] > 100.0 ? 100.0 : L[line * width + pixelx]);
            L[line * width + pixelx] = (L[line * width + pixelx] < 0.0 ? 0.0 : L[line * width + pixelx]);
        }
    }

    formtoolbox->setProgressValue(80);


    ColorSpace::Lab srcColor1;
    ColorSpace::Rgb dstColor1;

    for (int line = height - 1; line >= 0; line--)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            Lu = L[line * width + pixelx];
            a_lab = a[line * width + pixelx];
            b_lab = b[line * width + pixelx];
            srcColor1.l = Lu;
            srcColor1.a = a_lab;
            srcColor1.b = b_lab;

            srcColor1.To<ColorSpace::Rgb>(&dstColor1);

            dstColor1.r = (dstColor1.r > 255.0 ? 255.0 : dstColor1.r);
            dstColor1.g = (dstColor1.g > 255.0 ? 255.0 : dstColor1.g);
            dstColor1.b = (dstColor1.b > 255.0 ? 255.0 : dstColor1.b);

            dstColor1.r = (dstColor1.r < 0.0 ? 0.0 : dstColor1.r);
            dstColor1.g = (dstColor1.g < 0.0 ? 0.0 : dstColor1.g);
            dstColor1.b = (dstColor1.b < 0.0 ? 0.0 : dstColor1.b);


            row_col[pixelx] = qRgb((int)dstColor1.r, (int)dstColor1.g, (int)dstColor1.b);
        }
    }

    formtoolbox->setProgressValue(100);

    delete [] pixelsL;

    delete [] L;
    delete [] a;
    delete [] b;

    if(sl->getKindofImage() != "HRV" && sl->getKindofImage() != "HRV Color")
        if(opts.imageontextureOnMet)
            emit render3dgeo(sl->getGeoSatelliteIndex());

    QApplication::restoreOverrideCursor();
}

void FormImage::recalculateCLAHEAVHRR()
{
    QImage *m_image;
    int height = 0, width = 0;

    switch(channelshown)
    {
    case IMAGE_AVHRR_CH1:
        m_image = imageptrs->ptrimagecomp_ch[0];
        break;
    case IMAGE_AVHRR_CH2:
        m_image = imageptrs->ptrimagecomp_ch[1];
        break;
    case IMAGE_AVHRR_CH3:
        m_image = imageptrs->ptrimagecomp_ch[2];
        break;
    case IMAGE_AVHRR_CH4:
        m_image = imageptrs->ptrimagecomp_ch[3];
        break;
    case IMAGE_AVHRR_CH5:
        m_image = imageptrs->ptrimagecomp_ch[4];
        break;
    case IMAGE_AVHRR_COL:
        m_image = imageptrs->ptrimagecomp_col;
        break;
    default:
        return;
    }

    height = m_image->height();
    width = m_image->width();

    int context_X = 0;
    int context_Y = 0;

    if(height%16 == 0 && width%16 == 0)
    {
       context_X = 16;
       context_Y = 16;
    }
    else if(height%10 == 0 && width%10 == 0)
    {
        context_X = 10;
        context_Y = 10;
    }
    else if(height%16 == 0 && width%10 == 0)
    {
        context_X = 10;
        context_Y = 16;
    }
    else if(height%10 == 0 && width%16 == 0)
    {
        context_X = 16;
        context_Y = 10;
    }
    else
    {
        return;
    }

    qDebug() << "context_X = " << context_X << " context_Y = " << context_Y;

    QRgb *row_col;
    quint16 cred, cgreen, cblue;
    QRgb c;

    double *L;
    double *a;
    double *b;

    double Lu;
    double a_lab;
    double b_lab;
    size_t npix;

    ushort *pixelsL;

    SegmentListGeostationary *sl;

    sl = segs->getActiveSegmentList();

    QApplication::setOverrideCursor( Qt::WaitCursor ); // this might take time

    formtoolbox->setProgressValue(10);

    npix = width*height;

    L = new double[width*height];
    a = new double[width*height];
    b = new double[width*height];


    qDebug() << Q_FUNC_INFO << "image width = " << width << " height = " << height << " npix = " << npix;

    ColorSpace::Rgb srcColor;
    ColorSpace::Lab dstColor;


    for (int line = height - 1; line >= 0; line--)
    {
        row_col = (QRgb*)m_image->scanLine(line);
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            c = row_col[pixelx];
            srcColor.r = qRed(c);
            srcColor.g = qGreen(c);
            srcColor.b = qBlue(c);

            srcColor.To<ColorSpace::Lab>(&dstColor);

            dstColor.l = (dstColor.l < 0.0 ? 0.0 : dstColor.l);
            dstColor.l = (dstColor.l > 100.0 ? 100.0 : dstColor.l);
            dstColor.a = (dstColor.a < -128.0 ? -128.0 : dstColor.a);
            dstColor.a = (dstColor.a > 128.0 ? 128.0 : dstColor.a);
            dstColor.b = (dstColor.b < -128.0 ? -128.0 : dstColor.b);
            dstColor.b = (dstColor.b > 128.0 ? 128.0 : dstColor.b);
            L[line * width + pixelx] = dstColor.l;
            a[line * width + pixelx] = dstColor.a;
            b[line * width + pixelx] = dstColor.b;

        }
    }

    formtoolbox->setProgressValue(30);

    pixelsL = new ushort[npix];

    for (int line = height - 1; line >= 0; line--)
    {
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            pixelsL[line * width + pixelx] = (ushort)qRound(L[line * width + pixelx] * 255.0 / 100.0);
            pixelsL[line * width + pixelx] = (pixelsL[line * width + pixelx] > 255 ? 255 : pixelsL[line * width + pixelx]);
            pixelsL[line * width + pixelx] = (pixelsL[line * width + pixelx] > 255 ? 255 : pixelsL[line * width + pixelx]);

        }
    }

    formtoolbox->setProgressValue(60);

    imageptrs->CLAHE(pixelsL, width, height, 0, 255, context_X, context_Y, 256, opts.clahecliplimit);

    for (int line = height - 1; line >= 0; line--)
    {
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            L[line * width + pixelx] = (double)(pixelsL[line * width + pixelx] * 100.0 / 255.0);
            L[line * width + pixelx] = (L[line * width + pixelx] > 100.0 ? 100.0 : L[line * width + pixelx]);
            L[line * width + pixelx] = (L[line * width + pixelx] < 0.0 ? 0.0 : L[line * width + pixelx]);
        }
    }

    formtoolbox->setProgressValue(80);


    ColorSpace::Lab srcColor1;
    ColorSpace::Rgb dstColor1;

    for (int line = height - 1; line >= 0; line--)
    {
        row_col = (QRgb*)m_image->scanLine(line);
        for (int pixelx = 0; pixelx < width; pixelx++)
        {
            Lu = L[line * width + pixelx];
            a_lab = a[line * width + pixelx];
            b_lab = b[line * width + pixelx];
            srcColor1.l = Lu;
            srcColor1.a = a_lab;
            srcColor1.b = b_lab;

            srcColor1.To<ColorSpace::Rgb>(&dstColor1);

            dstColor1.r = (dstColor1.r > 255.0 ? 255.0 : dstColor1.r);
            dstColor1.g = (dstColor1.g > 255.0 ? 255.0 : dstColor1.g);
            dstColor1.b = (dstColor1.b > 255.0 ? 255.0 : dstColor1.b);

            dstColor1.r = (dstColor1.r < 0.0 ? 0.0 : dstColor1.r);
            dstColor1.g = (dstColor1.g < 0.0 ? 0.0 : dstColor1.g);
            dstColor1.b = (dstColor1.b < 0.0 ? 0.0 : dstColor1.b);


            row_col[pixelx] = qRgb((int)dstColor1.r, (int)dstColor1.g, (int)dstColor1.b);
        }
    }

    formtoolbox->setProgressValue(100);

    delete [] pixelsL;

    delete [] L;
    delete [] a;
    delete [] b;

    this->displayImage(channelshown, true);

//    if(opts.imageontextureOnAVHRR)
//        emit render3dgeo(sl->getGeoSatelliteIndex());

    QApplication::restoreOverrideCursor();
}

void FormImage::recalculateCLAHEMeteosat(QVector<QString> spectrumvector, QVector<bool> inversevector)
{

    QRgb *row_col;
    quint16 cred, cgreen, cblue, c;
    quint16 r,g, b;

    SegmentListGeostationary *sl;

    sl = segs->getActiveSegmentList();

    if (sl->getKindofImage() == "HRV Color")
        return;

    int geoindex = sl->getGeoSatelliteIndex();
    size_t npix;
    size_t npixHRV;

    npix = opts.geosatellites.at(geoindex).imageheight * opts.geosatellites.at(geoindex).imagewidth;
    if (sl->areatype == 1)
        npixHRV = opts.geosatellites.at(geoindex).imageheighthrv1 * opts.geosatellites.at(geoindex).imagewidthhrv1;
    else
        npixHRV = opts.geosatellites.at(geoindex).imageheighthrv0 * opts.geosatellites.at(geoindex).imagewidthhrv0;

    if(imageptrs->ptrRed[0] == NULL)
        return;


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
            if(imageptrs->ptrRed[i] != NULL)
                memcpy(pixelsRed + (sl->bisRSS ? i - 5 : i) * 464 * 3712, imageptrs->ptrRed[i], 464 * 3712 * sizeof(quint16));
            else
                memset(pixelsRed + (sl->bisRSS ? i - 5 : i) * 464 * 3712, 0, 464 * 3712 * sizeof(quint16));
        }
        for( int i = (sl ->bisRSS ? 5 : 0); i < 8; i++)
        {
            if(imageptrs->ptrGreen[i] != NULL)
                memcpy(pixelsGreen + (sl->bisRSS ? i - 5 : i) * 464 * 3712, imageptrs->ptrGreen[i], 464 * 3712 * sizeof(quint16));
            else
                memset(pixelsGreen + (sl->bisRSS ? i - 5 : i) * 464 * 3712, 0, 464 * 3712 * sizeof(quint16));
        }
        for( int i = (sl ->bisRSS ? 5 : 0); i < 8; i++)
        {
            if(imageptrs->ptrBlue[i] != NULL)
                memcpy(pixelsBlue + (sl->bisRSS ? i - 5 : i) * 464 * 3712, imageptrs->ptrBlue[i], 464 * 3712 * sizeof(quint16));
            else
                memset(pixelsBlue + (sl->bisRSS ? i - 5 : i) * 464 * 3712, 0, 464 * 3712 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::GOMS3)
    {
        pixelsRed = new quint16[npix];
        pixelsGreen = new quint16[npix];
        pixelsBlue = new quint16[npix];

        for( int i = 0; i < 6; i++)
        {
            if(imageptrs->ptrRed[i] != NULL)
                memcpy(pixelsRed + i * 464 * 2784, imageptrs->ptrRed[i], 464 * 2784 * sizeof(quint16));
            else
                memset(pixelsRed + i * 464 * 2784, 0, 464 * 2784 * sizeof(quint16));
        }
        for( int i = 0; i < 6; i++)
        {
            if(imageptrs->ptrGreen[i] != NULL)
                memcpy(pixelsGreen + i * 464 * 2784, imageptrs->ptrGreen[i], 464 * 2784 * sizeof(quint16));
            else
                memset(pixelsGreen + i * 464 * 2784, 0, 464 * 2784 * sizeof(quint16));
        }
        for( int i = 0; i < 6; i++)
        {
            if(imageptrs->ptrBlue[i] != NULL)
                memcpy(pixelsBlue + i * 464 * 2784, imageptrs->ptrBlue[i], 464 * 2784 * sizeof(quint16));
            else
                memset(pixelsBlue + i * 464 * 2784, 0, 464 * 2784 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::H9)
    {

        pixelsRed = new quint16[npix];
        pixelsGreen = new quint16[npix];
        pixelsBlue = new quint16[npix];

        for( int i = 0; i < 10; i++)
        {
            if(imageptrs->ptrRed[i] != NULL)
                memcpy(pixelsRed + i * 550 * 5500, imageptrs->ptrRed[i], 550 * 5500 * sizeof(quint16));
            else
                memset(pixelsRed + i * 550 * 5500, 0, 550 * 5500 * sizeof(quint16));
        }
        for( int i = 0; i < 10; i++)
        {
            if(imageptrs->ptrGreen[i] != NULL)
                memcpy(pixelsGreen + i * 550 * 5500, imageptrs->ptrGreen[i], 550 * 5500 * sizeof(quint16));
            else
                memset(pixelsGreen + i * 550 * 5500, 0, 550 * 5500 * sizeof(quint16));
        }
        for( int i = 0; i < 10; i++)
        {
            if(imageptrs->ptrBlue[i] != NULL)
                memcpy(pixelsBlue + i * 550 * 5500, imageptrs->ptrBlue[i], 550 * 5500 * sizeof(quint16));
            else
                memset(pixelsBlue + i * 550 * 5500, 0, 550 * 5500 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && (sl->getGeoSatellite() == eGeoSatellite::GOES_16 || sl->getGeoSatellite() == eGeoSatellite::GOES_17 || sl->getGeoSatellite() == eGeoSatellite::GOES_18))
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
            //            if(imageptrs->ptrDQF[0][i] == 2)
            //            {
            //                *(pixelsRed+i) = 1023;
            //                *(pixelsGreen+i) = 1023;
            //                *(pixelsBlue+i) = 1023;
            //            }
        }

    }
    else if(sl->getKindofImage() == "HRV" && (sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8 ))
    {
        pixelsHRV = new quint16[npixHRV];
        for( int i = 0, k = 0; i < (sl->bisRSS ? 5 : ( sl->areatype == 1 ? 24 : 5)); i++)
        {
            k = (sl->bisRSS ? 19 + i : (sl->areatype == 1 ? i : 19 + i));
            if(imageptrs->ptrHRV[k] != NULL)
                memcpy(pixelsHRV + i * 464 * 5568, imageptrs->ptrHRV[k], 464 * 5568 * sizeof(quint16));
            else
                memset(pixelsHRV + i * 464 * 5568, 0, 464 * 5568 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "HRV" && (sl->getGeoSatellite() == eGeoSatellite::FY2H || sl->getGeoSatellite() == eGeoSatellite::FY2G ))
    {
        pixelsHRV = new quint16[npixHRV];
        memcpy(pixelsHRV, imageptrs->ptrHRV[0], 9152 * 9152 * sizeof(quint16));
    }
    else if(sl->getKindofImage() == "VIS_IR" && (sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8 ))
    {
        pixelsRed = new quint16[npix];
        for( int i = (sl->bisRSS ? 5 : 0); i < 8 ; i++)
        {
            if(imageptrs->ptrRed[i] != NULL)
                memcpy(pixelsRed + (sl->bisRSS ? i - 5 : i) * 464 * 3712, imageptrs->ptrRed[i], 464 * 3712 * sizeof(quint16));
            else
                memset(pixelsRed + (sl->bisRSS ? i - 5 : i) * 464 * 3712, 0, 464 * 3712 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR" && sl->getGeoSatellite() == eGeoSatellite::GOMS3)
    {
        pixelsRed = new quint16[npix];
        for( int i = 0; i < 6 ; i++)
        {
            if(imageptrs->ptrRed[i] != NULL)
                memcpy(pixelsRed + i * 464 * 2784, imageptrs->ptrRed[i], 464 * 2784 * sizeof(quint16));
            else
                memset(pixelsRed + i * 464 * 2784, 0, 464 * 2784 * sizeof(quint16));
        }
    }
    else if(sl->getKindofImage() == "VIS_IR" && (sl->getGeoSatellite() == eGeoSatellite::FY2H || sl->getGeoSatellite() == eGeoSatellite::FY2G ))
    {
        pixelsRed = new quint16[npix];
        memcpy(pixelsRed, imageptrs->ptrRed[0], npix * sizeof(quint16));
    }
    else if(sl->getKindofImage() == "VIS_IR" && (sl->getGeoSatellite() == eGeoSatellite::GOES_16 || sl->getGeoSatellite() == eGeoSatellite::GOES_17 || sl->getGeoSatellite() == eGeoSatellite::GOES_18 ))
    {
        pixelsRed = new quint16[npix];
        memcpy(pixelsRed, imageptrs->ptrRed[0], npix * sizeof(quint16));

        for(int i = 0; i < npix; i++)
        {
            if(*(pixelsRed+i) == imageptrs->fillvalue[0] )
                *(pixelsRed + i) = 0;
            //            if(imageptrs->ptrDQF[0][i] == 2)
            //                *(pixelsRed+i) = 1023;
        }
    }
    else if(sl->getKindofImage() == "VIS_IR" && sl->getGeoSatellite() == eGeoSatellite::H9)
    {
        pixelsRed = new quint16[npix];
        for( int i = 0; i < 10 ; i++)
        {
            if(imageptrs->ptrRed[i] != NULL)
                memcpy(pixelsRed + i * 550 * 5500, imageptrs->ptrRed[i], 550 * 5500 * sizeof(quint16));
            else
                memset(pixelsRed + i * 550 * 5500, 0, 550 * 5500 * sizeof(quint16));
        }
    }

    int ret = 0;


    if(sl->getKindofImage() == "VIS_IR Color" && (sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8 ))
    {
        imageptrs->CLAHE(pixelsRed, 3712, (sl->bisRSS ? 3*464 : 3712), 0, 1023, 16, 16, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsGreen, 3712, (sl->bisRSS ? 3*464 : 3712), 0, 1023, 16, 16, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsBlue, 3712, (sl->bisRSS ? 3*464 : 3712), 0, 1023, 16, 16, 256, opts.clahecliplimit);
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::GOMS3)
    {
        imageptrs->CLAHE(pixelsRed, 2784, 2784, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsGreen, 2784, 2784, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsBlue, 2784, 2784, 0, 1023, 16, 16, 256, opts.clahecliplimit);
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::H9 )
    {
        ret = imageptrs->CLAHE(pixelsRed, 5500, 5500, 0, 1023, 10, 10, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsGreen, 5500, 5500, 0, 1023, 10, 10, 256, opts.clahecliplimit);
        imageptrs->CLAHE(pixelsBlue, 5500, 5500, 0, 1023, 10, 10, 256, opts.clahecliplimit);
    }
    else if(sl->getKindofImage() == "VIS_IR Color" && (sl->getGeoSatellite() == eGeoSatellite::GOES_16 || sl->getGeoSatellite() == eGeoSatellite::GOES_17 || sl->getGeoSatellite() == eGeoSatellite::GOES_18) )
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
    else if(sl->getKindofImage() == "HRV" && (sl->getGeoSatellite() == eGeoSatellite::FY2H || sl->getGeoSatellite() == eGeoSatellite::FY2G ))
    {
        imageptrs->CLAHE(pixelsHRV, 9152, 9152, 0, 255, 16, 16, 256, opts.clahecliplimit);
    }
    else if(sl->getKindofImage() == "VIS_IR")
    {
        if(sl->getGeoSatellite() == eGeoSatellite::MET_11 || sl->getGeoSatellite() == eGeoSatellite::MET_10 || sl->getGeoSatellite() == eGeoSatellite::MET_9 || sl->getGeoSatellite() == eGeoSatellite::MET_8)
            imageptrs->CLAHE(pixelsRed, 3712, (sl->bisRSS ? 3*464 : 3712), 0, 1023, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == eGeoSatellite::GOMS3)
        {
            ret = imageptrs->CLAHE(pixelsRed, 2784, 464*6, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        }
        else if(sl->getGeoSatellite() == eGeoSatellite::FY2H || sl->getGeoSatellite() == eGeoSatellite::FY2G)
            imageptrs->CLAHE(pixelsRed, 2288, 2288, 0, 255, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == eGeoSatellite::GOES_16 || sl->getGeoSatellite() == eGeoSatellite::GOES_17 || sl->getGeoSatellite() == eGeoSatellite::GOES_18)
            imageptrs->CLAHE(pixelsRed, 5424, 5424, 0, 1023, 16, 16, 256, opts.clahecliplimit);
        else if(sl->getGeoSatellite() == eGeoSatellite::H9)
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
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::H9)
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
    else if(sl->getKindofImage() == "VIS_IR Color" && (sl->getGeoSatellite() == eGeoSatellite::GOES_16 || sl->getGeoSatellite() == eGeoSatellite::GOES_17 || sl->getGeoSatellite() == eGeoSatellite::GOES_18))
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
    else if(sl->getKindofImage() == "VIS_IR Color" && sl->getGeoSatellite() == eGeoSatellite::GOMS3)
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
        else if(sl->getGeoSatellite() == eGeoSatellite::FY2H || sl->getGeoSatellite() == eGeoSatellite::FY2G)
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
        else if(sl->getGeoSatellite() == eGeoSatellite::GOMS3)
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
        else if(sl->getGeoSatellite() == eGeoSatellite::H9 )
        {
            qDebug() << "in if(sl->getGeoSatellite() == eGeoSatellite::H9 )";

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
        else if(sl->getGeoSatellite() == eGeoSatellite::FY2H || sl->getGeoSatellite() == eGeoSatellite::FY2G)
        {
            qDebug() << "recalculate CLAHE ; VIS_IR and FY2H/G move to ptrImageGeostationary";

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
        else if(sl->getGeoSatellite() == eGeoSatellite::GOES_16 || sl->getGeoSatellite() == eGeoSatellite::GOES_17 || sl->getGeoSatellite() == eGeoSatellite::GOES_18)
        {
            qDebug() << "recalculate CLAHE ; VIS_IR and GOES_16/_17/_18 move to ptrImageGeostationary";

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

void FormImage::recalculateCLAHEMTG(QVector<QString> spectrumvector, QVector<bool> inversevector)
{

    QRgb *row_col;
    quint16 cred, cgreen, cblue, c;
    quint16 r,g, b;

    SegmentListGeostationary *sl;

    sl = segs->getActiveSegmentList();


    int geoindex = sl->getGeoSatelliteIndex();

    if(imageptrs->ptrMTG[0][0] == NULL)
        return;

}

void FormImage::slotSetRedValue(int red)
{
    qDebug() << "red value = " << red;
    SegmentListGeostationary *sm =  segs->getActiveSegmentList();
    int geoindex = sm->getGeoSatelliteIndex();

    QVector<int> vec;

    for(int i = 0; i < 32; i++)
    {
        vec.append(i*116);
    }

    auto callbackMethod = std::bind(concurrentSetRed, sm, std::placeholders::_1, red);
    QtConcurrent::blockingMap(vec, callbackMethod);

    this->slotUpdateGeosat();

    //    this->scene()->update(this->sceneRect());

    //    QWidget* viewport = this->viewport();
    //    viewport->update();

    //    QList<QGraphicsItem*> all = scene()->items();
    //    qDebug() << "geoindex = " << geoindex << "nbr of items = " << all.size();

    //        for (int i = 0; i < all.size(); i++)
    //        {

    //            //VEPointItem *gi = static_cast<VEPointItem*>(all[i]);
    //            all[i]->update();
    //        }
    //    this->invalidateScene(); // updateScene(this->sceneRect());


}

void FormImage::concurrentSetRed(SegmentListGeostationary *sm, const int &line, const int &value)
{
    QRgb *row_col;
    quint16 r,g, b;

    int geoindex = sm->getGeoSatelliteIndex();

    float red = 1.0/value;

    for(int i = line; i < line + 116; i++)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
        for(int pixelx = 0; pixelx < opts.geosatellites[geoindex].imagewidth; pixelx++)
        {
            r = qRound(qRed(row_col[pixelx]) * red);
            g = qGreen(row_col[pixelx]);
            b = qBlue(row_col[pixelx]);
            row_col[pixelx] = qRgb(r, g , b);
        }
    }

}

void FormImage::slotUpdateGeosat()
{

    qDebug() << "start FormImage::slotUpdateGeosat()";

    this->displayImage(IMAGE_GEOSTATIONARY, true);
    emit allsegmentsreceivedbuttons(true);

    this->update();

}

void FormImage::CLAHERGBRecipe(float cliplimit)
{
    QRgb *row_col;
    quint16 red, green, blue;
    int npix = 3712 * 3712;
    quint16 *pixelsRed = new quint16[npix];
    quint16 *pixelsGreen = new quint16[npix];
    quint16 *pixelsBlue = new quint16[npix];

    qDebug() << "ptrimageGeostationary width = " << imageptrs->ptrimageGeostationary->width();
    if(imageptrs->ptrimageGeostationary->width() != 3712)
        return;

    for (int line = 0; line < 3712; line++)
    {
        for (int pixelx = 0; pixelx < 3712; pixelx++)
        {
            int i_image = line * 3712 + pixelx;
            pixelsRed[i_image] = imageptrs->ptrimageRGBRecipeRed[i_image];
            pixelsGreen[i_image] = imageptrs->ptrimageRGBRecipeGreen[i_image];
            pixelsBlue[i_image] = imageptrs->ptrimageRGBRecipeBlue[i_image];
        }
    }

    imageptrs->CLAHE(pixelsRed, 3712, 3712, 0, 255, 16, 16, 256, cliplimit);
    imageptrs->CLAHE(pixelsGreen, 3712, 3712, 0, 255, 16, 16, 256, cliplimit);
    imageptrs->CLAHE(pixelsBlue, 3712, 3712, 0, 255, 16, 16, 256, cliplimit);


    for (int line = 0; line < 3712; line++)
    {
        row_col = (QRgb*)imageptrs->ptrimageGeostationary->scanLine(line);
        for (int pixelx = 0; pixelx < 3712; pixelx++)
        {
            int i_image = line * 3712 + pixelx;

            red = pixelsRed[i_image];
            green = pixelsGreen[i_image];
            blue = pixelsBlue[i_image];

            row_col[pixelx] = qRgb((int)red, (int)green, (int)blue);
        }
    }


    delete [] pixelsRed;
    delete [] pixelsGreen;
    delete [] pixelsBlue;
}

