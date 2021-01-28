// ( ͡° ͜ʖ ͡°)﻿

#include "formmapcyl.h"
#include "ui_formmapcyl.h"
#include "dialogpreferences.h"

#include <netcdf.h>

extern Options opts;
extern SegmentImage *imageptrs;


FormMapCyl::FormMapCyl(QWidget *parent, MapFieldCyl *p_mapcyl, Globe *p_globe, FormToolbox *p_formtoolbox, SatelliteList *satlist, AVHRRSatellite *seglist ) :
    QWidget(parent),
    ui(new Ui::FormMapCyl)
{
    ui->setupUi(this);

    qDebug() << "constructor formmapcyl";

    sats = satlist;
    segs = seglist;
    mapcyl = p_mapcyl;
    globe = p_globe;
    formtoolbox = p_formtoolbox;

    ui->stackedWidget->addWidget(mapcyl);
    ui->stackedWidget->addWidget(globe);

    ui->stackedWidget->setCurrentIndex(0);
    ui->tabWidget->setCurrentIndex(0);

    ui->btnMetop->setCheckable(true);
    ui->btnNoaa->setCheckable(true);
    ui->btnGAC->setCheckable(true);
    ui->btnHRP->setCheckable(true);
    ui->btnMetopAhrpt->setCheckable(true);
    ui->btnMetopBhrpt->setCheckable(true);
    ui->btnNoaa19hrpt->setCheckable(true);
    ui->btnM01hrpt->setCheckable(true);
    ui->btnM02hrpt->setCheckable(true);

    ui->btnVIIRSM->setCheckable(true);
    ui->btnVIIRSDNB->setCheckable(true);
    ui->btnVIIRSMNOAA20->setCheckable(true);
    ui->btnVIIRSDNBNOAA20->setCheckable(true);
    ui->btnOLCIefr->setCheckable(true);
    ui->btnOLCIerr->setCheckable(true);
    ui->btnSLSTR->setCheckable(true);
    ui->btnOLCIefrDatahub->setCheckable(true);
    ui->btnOLCIerrDatahub->setCheckable(true);
    ui->btnSLSTRDatahub->setCheckable(true);
    ui->btnMERSI->setCheckable(true);

    ui->btnRealTime->setCheckable(true);
    ui->btnPhong->setCheckable(true);
    ui->btnAllSegments->setCheckable(true);

    ui->pbProduct1->setValue(0);
    ui->pbProduct2->setValue(0);
    ui->pbXMLprogress->setValue(0);

    ui->twSelectedProducts->setColumnCount(6);
    QStringList hlst;
    hlst << "Status" << "Type" << "Date" << "Start" << "End" << "Size";
    ui->twSelectedProducts->setHorizontalHeaderLabels(hlst);
    ui->twSelectedProducts->verticalHeader()->setVisible(false);
    ui->twSelectedProducts->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->twSelectedProducts->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->twSelectedProducts->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->twSelectedProducts->setShowGrid(true);
    ui->twSelectedProducts->setStyleSheet("QTableView {selection-background-color: green;}");
    ui->lblTotalDownloadSize->setText("No selected segments");

    SetAllButtonsToFalse();
    if (opts.buttonMetop)
    {
        opts.buttonMetop = true;
    }
    else if (opts.buttonNoaa)
    {
        opts.buttonNoaa = true;
    }
    else if (opts.buttonGAC)
    {
        opts.buttonGAC = true;
    }
    else if (opts.buttonHRP)
    {
        opts.buttonHRP = true;
    }
    else if (opts.buttonRealTime)
    {
        opts.buttonRealTime = true;
    }
    else if (opts.buttonVIIRSM)
    {
        opts.buttonVIIRSM = true;
        formtoolbox->setTabWidgetIndex(TAB_VIIRS);
        formtoolbox->setTabWidgetVIIRSIndex(0);
    }
    else if (opts.buttonVIIRSDNB)
    {
        opts.buttonVIIRSDNB = true;
        formtoolbox->setTabWidgetIndex(TAB_VIIRS);
        formtoolbox->setTabWidgetVIIRSIndex(1);
    }
    else if (opts.buttonVIIRSMNOAA20)
    {
        opts.buttonVIIRSMNOAA20 = true;
        formtoolbox->setTabWidgetIndex(TAB_VIIRS);
        formtoolbox->setTabWidgetVIIRSIndex(0);
    }
    else if (opts.buttonVIIRSDNBNOAA20)
    {
        opts.buttonVIIRSDNBNOAA20 = true;
        formtoolbox->setTabWidgetIndex(TAB_VIIRS);
        formtoolbox->setTabWidgetVIIRSIndex(1);
    }
    else if (opts.buttonOLCIefr)
    {
        opts.buttonOLCIefr = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(0);
    }
    else if (opts.buttonOLCIerr)
    {
        opts.buttonOLCIerr = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(0);
    }
    else if (opts.buttonSLSTR)
    {
        opts.buttonSLSTR = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(1);
    }
    else if (opts.buttonMetopAhrpt)
    {
        opts.buttonMetopAhrpt = true;
        formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    }
    else if (opts.buttonMetopBhrpt)
    {
        opts.buttonMetopBhrpt = true;
        formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    }
    else if (opts.buttonNoaa19hrpt)
    {
        opts.buttonNoaa19hrpt = true;
        formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    }
    else if (opts.buttonM01hrpt)
    {
        opts.buttonM01hrpt = true;
        formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    }
    else if (opts.buttonM02hrpt)
    {
        opts.buttonM02hrpt = true;
        formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    }
    else if (opts.buttonDatahubOLCIefr)
    {
        opts.buttonDatahubOLCIefr = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(0);
    }
    else if (opts.buttonDatahubOLCIerr)
    {
        opts.buttonDatahubOLCIerr = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(0);
    }
    else if (opts.buttonDatahubSLSTR)
    {
        opts.buttonDatahubSLSTR = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(1);
    }
    else if (opts.buttonMERSI)
    {
        opts.buttonMERSI = true;
    }


    ui->btnMetop->setChecked(opts.buttonMetop);
    ui->btnNoaa->setChecked(opts.buttonNoaa);
    ui->btnGAC->setChecked(opts.buttonGAC);
    ui->btnHRP->setChecked(opts.buttonHRP);
    ui->btnMetopAhrpt->setChecked(opts.buttonMetopAhrpt);
    ui->btnMetopBhrpt->setChecked(opts.buttonMetopBhrpt);
    ui->btnNoaa19hrpt->setChecked(opts.buttonNoaa19hrpt);
    ui->btnM01hrpt->setChecked(opts.buttonM01hrpt);
    ui->btnM02hrpt->setChecked(opts.buttonM02hrpt);

    ui->btnVIIRSM->setChecked(opts.buttonVIIRSM);
    ui->btnVIIRSDNB->setChecked(opts.buttonVIIRSDNB);
    ui->btnVIIRSMNOAA20->setChecked(opts.buttonVIIRSMNOAA20);
    ui->btnVIIRSDNBNOAA20->setChecked(opts.buttonVIIRSDNBNOAA20);
    ui->btnOLCIefr->setChecked(opts.buttonOLCIefr);
    ui->btnOLCIerr->setChecked(opts.buttonOLCIerr);
    ui->btnSLSTR->setChecked(opts.buttonSLSTR);
    ui->btnOLCIefrDatahub->setChecked(opts.buttonDatahubOLCIefr);
    ui->btnOLCIerrDatahub->setChecked(opts.buttonDatahubOLCIerr);
    ui->btnSLSTRDatahub->setChecked(opts.buttonDatahubSLSTR);
    ui->btnMERSI->setChecked(opts.buttonMERSI);
    ui->btnRealTime->setChecked(opts.buttonRealTime);
    ui->btnPhong->setChecked(opts.buttonPhong);
    ui->btnAllSegments->setChecked(opts.buttonShowAllSegments);

    if(opts.buttonDatahubOLCIefr || opts.buttonDatahubOLCIerr || opts.buttonDatahubSLSTR)
        ui->btnMakeImage->setEnabled(false);
    else
        ui->btnMakeImage->setEnabled(true);

    connect( ui->btnMetop, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ) );
    connect( ui->btnNoaa, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnGAC, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnHRP, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnMetopAhrpt, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnMetopBhrpt, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnNoaa19hrpt, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnM01hrpt, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnM02hrpt, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnMERSI, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ) );

    //connect( ui->btnVIIRSM, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    //connect( ui->btnVIIRSDNB, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));

    //connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(showSegmentList(int)));
    connect(mapcyl, SIGNAL(wheelChange(int)), this, SLOT(changeScrollBar(int)));
    connect(mapcyl, SIGNAL(mapClicked()), this, SLOT(showSegmentCount()));
    connect(globe, SIGNAL(mapClicked()), this, SLOT(showSegmentCount()));

    segs->setShowAllSegments(ui->btnAllSegments->isChecked());

    this->showSegmentCount();

    ui->rdbDownloadXMLOLCIEFR->setChecked(true);
    ui->rdbDownloadXMLOLCIERR->setChecked(false);
    ui->rdbDownloadXMLSLSTR->setChecked(false);


}

void FormMapCyl::SetAllButtonsToFalse()
{
    opts.buttonMetop = false;
    opts.buttonNoaa = false;
    opts.buttonGAC = false;
    opts.buttonHRP = false;
    opts.buttonMetopAhrpt = false;
    opts.buttonMetopBhrpt = false;
    opts.buttonNoaa19hrpt = false;
    opts.buttonM01hrpt = false;
    opts.buttonM02hrpt = false;

    opts.buttonVIIRSM = false;
    opts.buttonVIIRSDNB = false;
    opts.buttonVIIRSMNOAA20 = false;
    opts.buttonVIIRSDNBNOAA20 = false;
    opts.buttonOLCIefr = false;
    opts.buttonOLCIerr = false;
    opts.buttonSLSTR = false;
    opts.buttonDatahubOLCIefr = false;
    opts.buttonDatahubOLCIerr = false;
    opts.buttonDatahubSLSTR = false;
    opts.buttonRealTime = false;

}

void FormMapCyl::slotSetMapCylButtons(bool stat)
{
    ui->btnSaveTexture->setEnabled(stat);
    ui->btnClearTexture->setEnabled(stat);
    ui->btnRealTime->setEnabled(stat);
    ui->btnPhong->setEnabled(stat);
    ui->btnAllSegments->setEnabled(stat);
    ui->btnRemoveSelected->setEnabled(stat);
    ui->btnMakeImage->setEnabled(stat);

    ui->btnMetop->setEnabled(stat);
    ui->btnHRP->setEnabled(stat);
    ui->btnGAC->setEnabled(stat);
    ui->btnNoaa->setEnabled(stat);
    ui->btnVIIRSM->setEnabled(stat);
    ui->btnVIIRSDNB->setEnabled(stat);
    ui->btnVIIRSMNOAA20->setEnabled(stat);
    ui->btnVIIRSDNBNOAA20->setEnabled(stat);
    ui->btnOLCIefr->setEnabled(stat);
    ui->btnOLCIerr->setEnabled(stat);
    ui->btnSLSTR->setEnabled(stat);
    ui->btnMERSI->setEnabled(stat);


    ui->btnMetopAhrpt->setEnabled(stat);
    ui->btnMetopBhrpt->setEnabled(stat);
    ui->btnNoaa19hrpt->setEnabled(stat);
    ui->btnM01hrpt->setEnabled(stat);
    ui->btnM02hrpt->setEnabled(stat);

    ui->btnOLCIefrDatahub->setEnabled(stat);
    ui->btnOLCIerrDatahub->setEnabled(stat);
    ui->btnSLSTRDatahub->setEnabled(stat);
    ui->btnDownloadQuicklook->setEnabled(stat);
    ui->btnDownloadCompleteProduct->setEnabled(stat);
    ui->btnDownloadPartialProduct->setEnabled(stat);
    ui->btnCancelDownloadProduct->setEnabled(stat);
}

// Key handler
void FormMapCyl::keyPressEvent(QKeyEvent *event)
{

    qDebug() << "FormMapCyl::keyPressEvent(QKeyEvent *event)";

    switch (event->key())
    {

    default:
        globe->keyPressEvent(event);
    }
}
void FormMapCyl::setCylOrGlobe(int ind)
{
    ui->stackedWidget->setCurrentIndex(ind);
    qDebug() << QString("setCylOrGlobe = %1").arg(ind);
    if (ind == 0)
        ui->btnSaveTexture->setVisible(false);
    else
        ui->btnSaveTexture->setVisible(true);
    qDebug() << QString("na setCylOrGlobe = %1").arg(ind);

}

void FormMapCyl::showSegmentCount()
{
    qDebug() << "FormMapCyl::showSegmentcount";

    int cntselmetop = segs->seglmetop->NbrOfSegmentsSelected();
    int cntselnoaa = segs->seglnoaa->NbrOfSegmentsSelected();
    int cntselhrp = segs->seglhrp->NbrOfSegmentsSelected();
    int cntselgac = segs->seglgac->NbrOfSegmentsSelected();
    int cntselviirsm = segs->seglviirsm->NbrOfSegmentsSelected();
    int cntselviirsdnb = segs->seglviirsdnb->NbrOfSegmentsSelected();
    int cntselviirsmnoaa20 = segs->seglviirsmnoaa20->NbrOfSegmentsSelected();
    int cntselviirsdnbnoaa20 = segs->seglviirsdnbnoaa20->NbrOfSegmentsSelected();
    int cntselolciefr = segs->seglolciefr->NbrOfSegmentsSelected();
    int cntselolcierr = segs->seglolcierr->NbrOfSegmentsSelected();
    int cntselslstr = segs->seglslstr->NbrOfSegmentsSelected();
    int cntseldatahubolciefr = segs->segldatahubolciefr->NbrOfSegmentsSelected();
    int cntseldatahubolcierr = segs->segldatahubolcierr->NbrOfSegmentsSelected();
    int cntseldatahubslstr = segs->segldatahubslstr->NbrOfSegmentsSelected();
    int cntselmersi = segs->seglmersi->NbrOfSegmentsSelected();


    int cntselmetopAhrpt = segs->seglmetopAhrpt->NbrOfSegmentsSelected();
    int cntselmetopBhrpt = segs->seglmetopBhrpt->NbrOfSegmentsSelected();
    int cntselnoaa19hrpt = segs->seglnoaa19hrpt->NbrOfSegmentsSelected();
    int cntselM02hrpt = segs->seglM02hrpt->NbrOfSegmentsSelected();
    int cntselM01hrpt = segs->seglM01hrpt->NbrOfSegmentsSelected();

    int cntmetop = segs->seglmetop->NbrOfSegments();
    int cntnoaa = segs->seglnoaa->NbrOfSegments();
    int cnthrp = segs->seglhrp->NbrOfSegments();
    int cntgac = segs->seglgac->NbrOfSegments();
    int cntviirsm = segs->seglviirsm->NbrOfSegments();
    int cntviirsdnb = segs->seglviirsdnb->NbrOfSegments();
    int cntviirsmnoaa20 = segs->seglviirsmnoaa20->NbrOfSegments();
    int cntviirsdnbnoaa20 = segs->seglviirsdnbnoaa20->NbrOfSegments();
    int cntolciefr = segs->seglolciefr->NbrOfSegments();
    int cntolcierr = segs->seglolcierr->NbrOfSegments();
    int cntslstr = segs->seglslstr->NbrOfSegments();
    int cntdatahubolciefr = segs->segldatahubolciefr->NbrOfSegments();
    int cntdatahubolcierr = segs->segldatahubolcierr->NbrOfSegments();
    int cntdatahubslstr = segs->segldatahubslstr->NbrOfSegments();
    int cntmersi = segs->seglmersi->NbrOfSegments();

    int cntmetopAhrpt = segs->seglmetopAhrpt->NbrOfSegments();
    int cntmetopBhrpt = segs->seglmetopBhrpt->NbrOfSegments();
    int cntnoaa19hrpt = segs->seglnoaa19hrpt->NbrOfSegments();
    int cntM02hrpt = segs->seglM02hrpt->NbrOfSegments();
    int cntM01hrpt = segs->seglM01hrpt->NbrOfSegments();

    long totseg = cntmetop + cntnoaa + cnthrp + cntgac + cntviirsm + cntviirsdnb + cntviirsmnoaa20 + cntviirsdnbnoaa20 + cntolciefr + cntolcierr + cntslstr +
            cntmetopAhrpt + cntmetopBhrpt + cntnoaa19hrpt + cntM01hrpt + cntM02hrpt + cntdatahubolciefr + cntdatahubolcierr + cntdatahubslstr + cntmersi;
    long totsegsel = cntselmetop + cntselnoaa + cntselhrp + cntselgac + cntselviirsm + cntselviirsdnb  + cntselviirsmnoaa20 + cntselviirsdnbnoaa20 + cntselolciefr + cntselolcierr + cntselslstr +
            cntselmetopAhrpt + cntselmetopBhrpt + cntselnoaa19hrpt + cntselM01hrpt + cntselM02hrpt + cntseldatahubolciefr + cntseldatahubolcierr + cntseldatahubslstr + cntselmersi;

    if ( totsegsel  > 0)
    {
        ui->btnRemoveSelected->setText( QString(" Remove %1 selected segments ").arg(totsegsel));
    }
    else
    {
        ui->btnRemoveSelected->setText(" No selected segments ");
    }

    ui->btnMetop->setText((QString(" Metop A/B/C # %1/%2 ").arg(cntselmetop).arg(cntmetop)));
    ui->btnNoaa->setText((QString(" NOAA-19 # %1/%2 ").arg(cntselnoaa).arg(cntnoaa)));
    ui->btnGAC->setText((QString(" NOAA-19 GAC # %1/%2 ").arg(cntselgac).arg(cntgac)));
    ui->btnHRP->setText((QString(" Metop A/B/C HRP # %1/%2 ").arg(cntselhrp).arg(cnthrp)));

    ui->btnVIIRSM->setText((QString(" NPP VIIRS M # %1/%2 ").arg(cntselviirsm).arg(cntviirsm)));
    ui->btnVIIRSDNB->setText((QString(" NPP VIIRS DNB # %1/%2 ").arg(cntselviirsdnb).arg(cntviirsdnb)));
    ui->btnVIIRSMNOAA20->setText((QString(" NOAA-20 VIIRS M # %1/%2 ").arg(cntselviirsmnoaa20).arg(cntviirsmnoaa20)));
    ui->btnVIIRSDNBNOAA20->setText((QString(" NOAA-20 VIIRS DNB # %1/%2 ").arg(cntselviirsdnbnoaa20).arg(cntviirsdnbnoaa20)));

    ui->btnOLCIefr->setText((QString(" OLCI EFR # %1/%2 ").arg(cntselolciefr).arg(cntolciefr)));
    ui->btnOLCIerr->setText((QString(" OLCI ERR # %1/%2 ").arg(cntselolcierr).arg(cntolcierr)));

    ui->btnSLSTR->setText((QString(" SLSTR # %1/%2 ").arg(cntselslstr).arg(cntslstr)));

    ui->btnMetopAhrpt->setText((QString(" Metop A # %1/%2 ").arg(cntselmetopAhrpt).arg(cntmetopAhrpt)));
    ui->btnMetopBhrpt->setText((QString(" Metop B # %1/%2 ").arg(cntselmetopBhrpt).arg(cntmetopBhrpt)));
    ui->btnNoaa19hrpt->setText((QString(" NOAA19 # %1/%2 ").arg(cntselnoaa19hrpt).arg(cntnoaa19hrpt)));
    ui->btnM02hrpt->setText((QString(" Metop A # %1/%2 ").arg(cntselM02hrpt).arg(cntM02hrpt)));
    ui->btnM01hrpt->setText((QString(" Metop B # %1/%2 ").arg(cntselM01hrpt).arg(cntM01hrpt)));

    ui->btnOLCIefrDatahub->setText((QString(" OLCI EFR # %1/%2 ").arg(cntseldatahubolciefr).arg(cntdatahubolciefr)));
    ui->btnOLCIerrDatahub->setText((QString(" OLCI ERR # %1/%2 ").arg(cntseldatahubolcierr).arg(cntdatahubolcierr)));
    ui->btnSLSTRDatahub->setText((QString(" SLSTR # %1/%2 ").arg(cntseldatahubslstr).arg(cntdatahubslstr)));

    ui->btnMERSI->setText((QString(" FY-3D # %1/%2 ").arg(cntselmersi).arg(cntmersi)));

}

void FormMapCyl::changeScrollBar(int value)
{
    qDebug() << QString("---------changeScrollBar value = %1").arg(value);
    ui->verticalScrollBar->setValue(ui->verticalScrollBar->value() + value);
}

void FormMapCyl::updatesatmap(int index)
{
    qDebug() << QString("stack index %1 selected scrollbar value %2").arg(index).arg(ui->verticalScrollBar->value());

    if (index == 0) //ephem
    {

    }
    if (index == 1 ) //cylequidist || Globe
    {
        if (ui->verticalScrollBar->value() == -1)
            ui->verticalScrollBar->setValue(0);

        this->setScrollBarMaximum();

        if (opts.buttonMetop )
        {
            segs->seglmetop->ShowSegment(ui->verticalScrollBar->value());
        } else
            if (opts.buttonNoaa )
            {
                segs->seglnoaa->ShowSegment(ui->verticalScrollBar->value());
            } else
                if (opts.buttonHRP)
                {
                    segs->seglhrp->ShowSegment(ui->verticalScrollBar->value());
                } else
                    if (opts.buttonGAC)
                    {
                        segs->seglgac->ShowSegment(ui->verticalScrollBar->value());
                    } else
                        if (opts.buttonMetopAhrpt)
                        {
                            segs->seglmetopAhrpt->ShowSegment(ui->verticalScrollBar->value());
                        } else
                            if (opts.buttonMetopBhrpt)
                            {
                                segs->seglmetopBhrpt->ShowSegment(ui->verticalScrollBar->value());
                            } else
                                if (opts.buttonNoaa19hrpt)
                                {
                                    segs->seglnoaa19hrpt->ShowSegment(ui->verticalScrollBar->value());
                                } else
                                    if (opts.buttonM01hrpt)
                                    {
                                        segs->seglM01hrpt->ShowSegment(ui->verticalScrollBar->value());
                                    } else
                                        if (opts.buttonM02hrpt)
                                        {
                                            segs->seglM02hrpt->ShowSegment(ui->verticalScrollBar->value());
                                        } else
                                            if (opts.buttonVIIRSM)
                                            {
                                                segs->seglviirsm->ShowSegment(ui->verticalScrollBar->value());
                                            } else
                                                if (opts.buttonVIIRSDNB)
                                                {
                                                    segs->seglviirsdnb->ShowSegment(ui->verticalScrollBar->value());
                                                } else
                                                    if (opts.buttonVIIRSMNOAA20)
                                                    {
                                                        segs->seglviirsmnoaa20->ShowSegment(ui->verticalScrollBar->value());
                                                    } else
                                                        if (opts.buttonVIIRSDNBNOAA20)
                                                        {
                                                            segs->seglviirsdnbnoaa20->ShowSegment(ui->verticalScrollBar->value());
                                                        } else
                                                            if (opts.buttonOLCIefr)
                                                            {
                                                                segs->seglolciefr->ShowSegment(ui->verticalScrollBar->value());
                                                            } else
                                                                if (opts.buttonOLCIerr)
                                                                {
                                                                    segs->seglolcierr->ShowSegment(ui->verticalScrollBar->value());
                                                                } else
                                                                    if (opts.buttonSLSTR)
                                                                    {
                                                                        segs->seglslstr->ShowSegment(ui->verticalScrollBar->value());
                                                                    } else
                                                                        if (opts.buttonMERSI)
                                                                        {
                                                                            segs->seglmersi->ShowSegment(ui->verticalScrollBar->value());
                                                                        }

        mapcyl->update();
    }
    if (index == 2)  //imagetab
    {
        QString tit;

        if (opts.buttonMetop )
        {
            tit = "Metop ";
        } else if (opts.buttonNoaa )
        {
            tit = "Noaa ";
        } else if (opts.buttonGAC)
        {
            tit = "GAC ";
        } else if (opts.buttonHRP)
        {
            tit = "HRP ";
        } else if (opts.buttonVIIRSM)
        {
            tit = "VIIRSM ";
        }  else if (opts.buttonVIIRSDNB)
        {
            tit = "VIIRSDNB ";
        } else if (opts.buttonOLCIefr)
        {
            tit = "OLCI EFR ";
        }  else if (opts.buttonOLCIerr)
        {
            tit = "OLCI ERR ";
        }  else if (opts.buttonSLSTR)
        {
            tit = "SLSTR ";
        }  else if (opts.buttonMERSI)
        {
            tit = "MERSI ";
        }


    }
}

void FormMapCyl::toggleButton(eSegmentType segtype)
{

    opts.buttonMetop = segtype == eSegmentType::SEG_METOP ? true : false;
    opts.buttonNoaa = segtype == eSegmentType::SEG_NOAA19 ? true : false;
    opts.buttonGAC = segtype == eSegmentType::SEG_GAC ? true : false;
    opts.buttonHRP = segtype == eSegmentType::SEG_HRP ? true : false;
    opts.buttonVIIRSM = segtype == eSegmentType::SEG_VIIRSM ? true : false;
    opts.buttonVIIRSDNB = segtype == eSegmentType::SEG_VIIRSDNB ? true : false;
    opts.buttonVIIRSMNOAA20 = segtype == eSegmentType::SEG_VIIRSMNOAA20 ? true : false;
    opts.buttonVIIRSDNBNOAA20 = segtype == eSegmentType::SEG_VIIRSDNBNOAA20 ? true : false;
    opts.buttonOLCIefr = segtype == eSegmentType::SEG_OLCIEFR ? true : false;
    opts.buttonOLCIerr = segtype == eSegmentType::SEG_OLCIERR ? true : false;
    opts.buttonSLSTR = segtype == eSegmentType::SEG_SLSTR ? true : false;
    opts.buttonDatahubOLCIefr = segtype == eSegmentType::SEG_DATAHUB_OLCIEFR ? true : false;
    opts.buttonDatahubOLCIerr = segtype == eSegmentType::SEG_DATAHUB_OLCIERR ? true : false;
    opts.buttonDatahubSLSTR = segtype == eSegmentType::SEG_DATAHUB_SLSTR ? true : false;
    opts.buttonRealTime = segtype == eSegmentType::SEG_NONE ? true : false;
    opts.buttonMetopAhrpt = segtype == eSegmentType::SEG_HRPT_METOPA ? true : false;
    opts.buttonMetopBhrpt = segtype == eSegmentType::SEG_HRPT_METOPB ? true : false;
    opts.buttonNoaa19hrpt = segtype == eSegmentType::SEG_HRPT_NOAA19 ? true : false;
    opts.buttonM01hrpt = segtype == eSegmentType::SEG_HRPT_M01 ? true : false;
    opts.buttonM02hrpt = segtype == eSegmentType::SEG_HRPT_M02 ? true : false;
    opts.buttonMERSI = segtype == eSegmentType::SEG_MERSI ? true : false;

    ui->btnMetop->setChecked(opts.buttonMetop);
    ui->btnNoaa->setChecked(opts.buttonNoaa);
    ui->btnGAC->setChecked(opts.buttonGAC);
    ui->btnHRP->setChecked(opts.buttonHRP);

    ui->btnMetopAhrpt->setChecked(opts.buttonMetopAhrpt);
    ui->btnMetopBhrpt->setChecked(opts.buttonMetopBhrpt);
    ui->btnNoaa19hrpt->setChecked(opts.buttonNoaa19hrpt);
    ui->btnM01hrpt->setChecked(opts.buttonM01hrpt);
    ui->btnM02hrpt->setChecked(opts.buttonM02hrpt);

    ui->btnVIIRSM->setChecked(opts.buttonVIIRSM);
    ui->btnVIIRSDNB->setChecked(opts.buttonVIIRSDNB);
    ui->btnVIIRSMNOAA20->setChecked(opts.buttonVIIRSMNOAA20);
    ui->btnVIIRSDNBNOAA20->setChecked(opts.buttonVIIRSDNBNOAA20);
    ui->btnOLCIefr->setChecked(opts.buttonOLCIefr);
    ui->btnOLCIerr->setChecked(opts.buttonOLCIerr);
    ui->btnSLSTR->setChecked(opts.buttonSLSTR);
    ui->btnOLCIefrDatahub->setChecked(opts.buttonDatahubOLCIefr);
    ui->btnOLCIerrDatahub->setChecked(opts.buttonDatahubOLCIerr);
    ui->btnSLSTRDatahub->setChecked(opts.buttonDatahubSLSTR);
    ui->btnMERSI->setChecked(opts.buttonMERSI);
    ui->btnRealTime->setChecked(opts.buttonRealTime);

    if(opts.buttonDatahubOLCIefr || opts.buttonDatahubOLCIerr || opts.buttonDatahubSLSTR)
        ui->btnMakeImage->setEnabled(false);
    else
        ui->btnMakeImage->setEnabled(true);


    this->showSegmentList(0);

    //imagetab->SetGammaSpinboxes();
    //this->RemoveAllSelected();
    this->setScrollBarMaximum();


    return;
}


void FormMapCyl::setScrollBarMaximum()
{


    if (opts.buttonMetop)
    {
        ui->verticalScrollBar->setMaximum(segs->seglmetop->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum metop = %1").arg(segs->seglmetop->NbrOfSegments());
    }
    else if (opts.buttonNoaa)
    {
        ui->verticalScrollBar->setMaximum(segs->seglnoaa->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum noaa = %1").arg(segs->seglnoaa->NbrOfSegments());
    }
    else if (opts.buttonHRP)
    {
        ui->verticalScrollBar->setMaximum(segs->seglhrp->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum HRP = %1").arg(segs->seglhrp->NbrOfSegments());
    }
    else if (opts.buttonGAC)
    {
        ui->verticalScrollBar->setMaximum(segs->seglgac->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum GAC = %1").arg(segs->seglgac->NbrOfSegments());
    }
    else if (opts.buttonMetopAhrpt)
    {
        ui->verticalScrollBar->setMaximum(segs->seglmetopAhrpt->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum hrpt Metop A = %1").arg(segs->seglmetopAhrpt->NbrOfSegments());
    }
    else if (opts.buttonMetopBhrpt)
    {
        ui->verticalScrollBar->setMaximum(segs->seglmetopBhrpt->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum hrpt Metop B = %1").arg(segs->seglmetopBhrpt->NbrOfSegments());
    }
    else if (opts.buttonNoaa19hrpt)
    {
        ui->verticalScrollBar->setMaximum(segs->seglnoaa19hrpt->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum hrpt noaa19 = %1").arg(segs->seglnoaa19hrpt->NbrOfSegments());
    }
    else if (opts.buttonM01hrpt)
    {
        ui->verticalScrollBar->setMaximum(segs->seglM01hrpt->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum hrpt M01 = %1").arg(segs->seglM01hrpt->NbrOfSegments());
    }
    else if (opts.buttonM02hrpt)
    {
        ui->verticalScrollBar->setMaximum(segs->seglM02hrpt->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum hrpt M02 = %1").arg(segs->seglM02hrpt->NbrOfSegments());
    }
    else if (opts.buttonVIIRSM)
    {
        ui->verticalScrollBar->setMaximum(segs->seglviirsm->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum VIIRSM = %1").arg(segs->seglviirsm->NbrOfSegments());
    }
    else if (opts.buttonVIIRSDNB)
    {
        ui->verticalScrollBar->setMaximum(segs->seglviirsdnb->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum VIIRSDNB = %1").arg(segs->seglviirsdnb->NbrOfSegments());
    }
    else if (opts.buttonVIIRSMNOAA20)
    {
        ui->verticalScrollBar->setMaximum(segs->seglviirsmnoaa20->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum VIIRSM NOAA-20 = %1").arg(segs->seglviirsmnoaa20->NbrOfSegments());
    }
    else if (opts.buttonVIIRSDNBNOAA20)
    {
        ui->verticalScrollBar->setMaximum(segs->seglviirsdnbnoaa20->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum VIIRSDNB NOAA-20 = %1").arg(segs->seglviirsdnbnoaa20->NbrOfSegments());
    }
    else if (opts.buttonOLCIefr)
    {
        ui->verticalScrollBar->setMaximum(segs->seglolciefr->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum OLCIefr = %1").arg(segs->seglolciefr->NbrOfSegments());
    }
    else if (opts.buttonOLCIerr)
    {
        ui->verticalScrollBar->setMaximum(segs->seglolcierr->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum OLCIerr = %1").arg(segs->seglolcierr->NbrOfSegments());
    }
    else if (opts.buttonSLSTR)
    {
        ui->verticalScrollBar->setMaximum(segs->seglslstr->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum SLSTR = %1").arg(segs->seglslstr->NbrOfSegments());
    }
    else if (opts.buttonDatahubOLCIefr)
    {
        ui->verticalScrollBar->setMaximum(segs->segldatahubolciefr->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum Datahub OLCIefr = %1").arg(segs->segldatahubolciefr->NbrOfSegments());
    }
    else if (opts.buttonDatahubOLCIerr)
    {
        ui->verticalScrollBar->setMaximum(segs->segldatahubolcierr->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum Datahub OLCIerr = %1").arg(segs->segldatahubolcierr->NbrOfSegments());
    }
    else if (opts.buttonDatahubSLSTR)
    {
        ui->verticalScrollBar->setMaximum(segs->segldatahubslstr->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum Datahub SLSTR = %1").arg(segs->segldatahubslstr->NbrOfSegments());
    }
    else if (opts.buttonMERSI)
    {
        ui->verticalScrollBar->setMaximum(segs->seglmersi->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum MERSI = %1").arg(segs->seglmersi->NbrOfSegments());
    }
    else if (opts.buttonRealTime)
    {
        ui->verticalScrollBar->setMaximum(0);
    }

    showSegmentList(0);
}

void FormMapCyl::showSegmentList(int value)
{

    QDateTime first, last;
    QString outp;
    int nbrseg;

    if(opts.buttonMetop)
    {
        segs->seglmetop->ShowSegment(value);
        segs->seglmetop->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglmetop->NbrOfSegments();

        outp = QString("Metop From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonNoaa)
    {
        segs->seglnoaa->ShowSegment(value);
        segs->seglnoaa->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglnoaa->NbrOfSegments();

        outp = QString("Noaa From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonHRP)
    {
        segs->seglhrp->ShowSegment(value);
        segs->seglhrp->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglhrp->NbrOfSegments();

        outp = QString("HRP From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonGAC)
    {
        segs->seglgac->ShowSegment(value);
        segs->seglgac->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglgac->NbrOfSegments();

        outp = QString("GAC From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    if(opts.buttonMetopAhrpt)
    {
        segs->seglmetopAhrpt->ShowSegment(value);
        segs->seglmetopAhrpt->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglmetopAhrpt->NbrOfSegments();

        outp = QString("Metop A HRPT From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    if(opts.buttonMetopBhrpt)
    {
        segs->seglmetopBhrpt->ShowSegment(value);
        segs->seglmetopBhrpt->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglmetopBhrpt->NbrOfSegments();

        outp = QString("Metop B HRPT From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    if(opts.buttonNoaa19hrpt)
    {
        segs->seglnoaa19hrpt->ShowSegment(value);
        segs->seglnoaa19hrpt->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglnoaa19hrpt->NbrOfSegments();

        outp = QString("Noaa19 HRPT From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    if(opts.buttonM01hrpt)
    {
        segs->seglM01hrpt->ShowSegment(value);
        segs->seglM01hrpt->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglM01hrpt->NbrOfSegments();

        outp = QString("M01 HRPT From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    if(opts.buttonM02hrpt)
    {
        segs->seglM02hrpt->ShowSegment(value);
        segs->seglM02hrpt->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglM02hrpt->NbrOfSegments();

        outp = QString("M02 HRPT From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonVIIRSM)
    {
        segs->seglviirsm->ShowSegment(value);
        segs->seglviirsm->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglviirsm->NbrOfSegments();

        outp = QString("NPP VIIRSM From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonVIIRSDNB)
    {
        segs->seglviirsdnb->ShowSegment(value);
        segs->seglviirsdnb->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglviirsdnb->NbrOfSegments();

        outp = QString("NPP VIIRSDNB From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonVIIRSMNOAA20)
    {
        segs->seglviirsmnoaa20->ShowSegment(value);
        segs->seglviirsmnoaa20->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglviirsmnoaa20->NbrOfSegments();

        outp = QString("NOAA-20 VIIRSM From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonVIIRSDNBNOAA20)
    {
        segs->seglviirsdnbnoaa20->ShowSegment(value);
        segs->seglviirsdnbnoaa20->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglviirsdnbnoaa20->NbrOfSegments();

        outp = QString("NOAA-20 VIIRSDNB From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonOLCIefr)
    {
        segs->seglolciefr->ShowSegment(value);
        segs->seglolciefr->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglolciefr->NbrOfSegments();

        outp = QString("OLCI EFR From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonOLCIerr)
    {
        segs->seglolcierr->ShowSegment(value);
        segs->seglolcierr->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglolcierr->NbrOfSegments();

        outp = QString("OLCI ERR From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonSLSTR)
    {
        segs->seglslstr->ShowSegment(value);
        segs->seglslstr->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglslstr->NbrOfSegments();

        outp = QString("SLSTR From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonDatahubOLCIefr)
    {
        segs->segldatahubolciefr->ShowSegment(value);
        segs->segldatahubolciefr->GetFirstLastVisible(&first, &last);
        nbrseg = segs->segldatahubolciefr->NbrOfSegments();

        outp = QString("Datahub OLCI EFR From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonDatahubOLCIerr)
    {
        segs->segldatahubolcierr->ShowSegment(value);
        segs->segldatahubolcierr->GetFirstLastVisible(&first, &last);
        nbrseg = segs->segldatahubolcierr->NbrOfSegments();

        outp = QString("Datahub OLCI ERR From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonDatahubSLSTR)
    {
        segs->segldatahubslstr->ShowSegment(value);
        segs->segldatahubslstr->GetFirstLastVisible(&first, &last);
        nbrseg = segs->segldatahubslstr->NbrOfSegments();

        outp = QString("Datahub SLSTR From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonMERSI)
    {
        segs->seglmersi->ShowSegment(value);
        segs->seglmersi->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglmersi->NbrOfSegments();

        outp = QString("MERSI From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonRealTime)
    {
        outp = QString("Real time");
    }

    emit signalSegmentChanged(outp);
    mapcyl->update();

}

void FormMapCyl::RemoveAllSelected()
{
    segs->RemoveAllSelectedAVHRR();
    segs->RemoveAllSelectedVIIRSM();
    segs->RemoveAllSelectedVIIRSDNB();
    segs->RemoveAllSelectedVIIRSMNOAA20();
    segs->RemoveAllSelectedVIIRSDNBNOAA20();
    segs->RemoveAllSelectedOLCIefr();
    segs->RemoveAllSelectedOLCIerr();
    segs->RemoveAllSelectedSLSTR();
    segs->RemoveAllSelectedDatahubOLCIefr();
    segs->RemoveAllSelectedDatahubOLCIerr();
    segs->RemoveAllSelectedDatahubSLSTR();
    segs->RemoveAllSelectedMERSI();

    imageptrs->ptrProjectionBrightnessTemp.reset();
    imageptrs->ptrProjectionInfra.reset();

    mapcyl->update();
    showSegmentCount();
    ui->twSelectedProducts->clearContents();
    ui->twSelectedProducts->setRowCount(0);

}

void FormMapCyl::slotShowSegmentCount()
{
    showSegmentCount();
}

void FormMapCyl::on_btnRemoveSelected_clicked()
{
    RemoveAllSelected();
}

bool FormMapCyl::AreThereSelectedSegments()
{

    if(opts.buttonMetop && segs->seglmetop->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonNoaa && segs->seglnoaa->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonHRP && segs->seglhrp->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonGAC && segs->seglgac->NbrOfSegmentsSelected() > 0)
        return true;

    if(opts.buttonMetopAhrpt && segs->seglmetopAhrpt->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonMetopBhrpt && segs->seglmetopBhrpt->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonNoaa19hrpt && segs->seglnoaa19hrpt->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonM01hrpt && segs->seglM01hrpt->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonM02hrpt && segs->seglM02hrpt->NbrOfSegmentsSelected() > 0)
        return true;

    if(opts.buttonVIIRSM && segs->seglviirsm->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonVIIRSDNB && segs->seglviirsdnb->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonVIIRSMNOAA20 && segs->seglviirsmnoaa20->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonVIIRSDNBNOAA20 && segs->seglviirsdnbnoaa20->NbrOfSegmentsSelected() > 0)
        return true;


    if(opts.buttonOLCIefr && segs->seglolciefr->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonOLCIerr && segs->seglolcierr->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonSLSTR && segs->seglslstr->NbrOfSegmentsSelected() > 0)
        return true;

    if(opts.buttonMERSI && segs->seglmersi->NbrOfSegmentsSelected() > 0)
        return true;

}

void FormMapCyl::on_btnMakeImage_clicked()
{
    if(!AreThereSelectedSegments())
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

    this->slotSetMapCylButtons(false);
    emit signalMakeImage();
}

void FormMapCyl::on_btnSaveTexture_clicked()
{

    QDateTime dt;
    dt = QDateTime::currentDateTime();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    imageptrs->pmOut->save("texture" + dt.toString("yyyyMMddhhmmss") + ".jpg");
    //globe->printTexture();
    qDebug() << "Texture saved to texture" + dt.toString("yyyyMMddhhmmss") + ".jpg";
    QApplication::restoreOverrideCursor();

}

void FormMapCyl::on_btnClearTexture_clicked()
{
    QPixmap pm = (*imageptrs->pmOriginal);
    *imageptrs->pmOut = (*imageptrs->pmOriginal).copy();
    opts.texture_changed = true;
}


void FormMapCyl::on_verticalScrollBar_valueChanged(int value)
{
    showSegmentList(value);
}

void FormMapCyl::on_btnNoaa_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    toggleButton(eSegmentType::SEG_NOAA19);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();

}

void FormMapCyl::on_btnMetop_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    toggleButton(eSegmentType::SEG_METOP);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}

void FormMapCyl::on_btnHRP_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    toggleButton(eSegmentType::SEG_HRP);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}

void FormMapCyl::on_btnGAC_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    toggleButton(eSegmentType::SEG_GAC);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}

void FormMapCyl::on_btnMetopAhrpt_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    toggleButton(eSegmentType::SEG_HRPT_METOPA);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}

void FormMapCyl::on_btnMetopBhrpt_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    toggleButton(eSegmentType::SEG_HRPT_METOPB);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}
void FormMapCyl::on_btnNoaa19hrpt_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    toggleButton(eSegmentType::SEG_HRPT_NOAA19);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}
void FormMapCyl::on_btnM01hrpt_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    toggleButton(eSegmentType::SEG_HRPT_M01);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}
void FormMapCyl::on_btnM02hrpt_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    toggleButton(eSegmentType::SEG_HRPT_M02);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}

void FormMapCyl::on_btnRealTime_clicked()
{
    toggleButton(eSegmentType::SEG_NONE);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();

}

void FormMapCyl::on_btnVIIRSM_clicked() // M-Bands
{

    formtoolbox->setTabWidgetIndex(TAB_VIIRS);
    formtoolbox->setTabWidgetVIIRSIndex(0);
    toggleButton(eSegmentType::SEG_VIIRSM);
    segs->RemoveAllSelectedAVHRR();
    segs->RemoveAllSelectedOLCIefr();
    segs->RemoveAllSelectedOLCIerr();
    segs->RemoveAllSelectedSLSTR();
    segs->RemoveAllSelectedDatahubOLCIefr();
    segs->RemoveAllSelectedDatahubOLCIerr();
    segs->RemoveAllSelectedDatahubSLSTR();
    segs->RemoveAllSelectedVIIRSDNB();
    segs->RemoveAllSelectedVIIRSMNOAA20();
    segs->RemoveAllSelectedVIIRSDNBNOAA20();


    mapcyl->update();
    this->showSegmentCount();
    this->setScrollBarMaximum();
    return;
}

void FormMapCyl::on_btnVIIRSDNB_clicked() // DNB Bands
{
    formtoolbox->setTabWidgetIndex(TAB_VIIRS);
    formtoolbox->setTabWidgetVIIRSIndex(1);
    toggleButton(eSegmentType::SEG_VIIRSDNB);
    segs->RemoveAllSelectedAVHRR();
    segs->RemoveAllSelectedOLCIefr();
    segs->RemoveAllSelectedOLCIerr();
    segs->RemoveAllSelectedSLSTR();
    segs->RemoveAllSelectedDatahubOLCIefr();
    segs->RemoveAllSelectedDatahubOLCIerr();
    segs->RemoveAllSelectedDatahubSLSTR();
    segs->RemoveAllSelectedVIIRSM();
    segs->RemoveAllSelectedVIIRSMNOAA20();
    segs->RemoveAllSelectedVIIRSDNBNOAA20();



    mapcyl->update();
    this->showSegmentCount();
    this->setScrollBarMaximum();
    return;
}

void FormMapCyl::on_btnVIIRSMNOAA20_clicked() // M-Bands
{

    formtoolbox->setTabWidgetIndex(TAB_VIIRS);
    formtoolbox->setTabWidgetVIIRSIndex(0);
    toggleButton(eSegmentType::SEG_VIIRSMNOAA20);
    segs->RemoveAllSelectedAVHRR();
    segs->RemoveAllSelectedOLCIefr();
    segs->RemoveAllSelectedOLCIerr();
    segs->RemoveAllSelectedSLSTR();
    segs->RemoveAllSelectedDatahubOLCIefr();
    segs->RemoveAllSelectedDatahubOLCIerr();
    segs->RemoveAllSelectedDatahubSLSTR();
    segs->RemoveAllSelectedVIIRSM();
    segs->RemoveAllSelectedVIIRSDNB();
    segs->RemoveAllSelectedVIIRSDNBNOAA20();

    mapcyl->update();
    this->showSegmentCount();
    this->setScrollBarMaximum();
    return;
}

void FormMapCyl::on_btnVIIRSDNBNOAA20_clicked() // DNB Bands
{
    formtoolbox->setTabWidgetIndex(TAB_VIIRS);
    formtoolbox->setTabWidgetVIIRSIndex(1);
    toggleButton(eSegmentType::SEG_VIIRSDNBNOAA20);
    segs->RemoveAllSelectedAVHRR();
    segs->RemoveAllSelectedOLCIefr();
    segs->RemoveAllSelectedOLCIerr();
    segs->RemoveAllSelectedSLSTR();
    segs->RemoveAllSelectedDatahubOLCIefr();
    segs->RemoveAllSelectedDatahubOLCIerr();
    segs->RemoveAllSelectedDatahubSLSTR();
    segs->RemoveAllSelectedVIIRSM();
    segs->RemoveAllSelectedVIIRSDNB();
    segs->RemoveAllSelectedVIIRSMNOAA20();

    mapcyl->update();
    this->showSegmentCount();
    this->setScrollBarMaximum();
    return;
}

void FormMapCyl::on_btnOLCIefr_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
    formtoolbox->setTabWidgetSentinelIndex(0);
    toggleButton(eSegmentType::SEG_OLCIEFR);
    this->RemoveAllSelected();
    //    mapcyl->update();
    //    this->showSegmentCount();
    this->setScrollBarMaximum();

    return;
}

void FormMapCyl::on_btnOLCIerr_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
    formtoolbox->setTabWidgetSentinelIndex(0);
    toggleButton(eSegmentType::SEG_OLCIERR);
    this->RemoveAllSelected();
    //    mapcyl->update();
    //    this->showSegmentCount();
    this->setScrollBarMaximum();

    return;
}

void FormMapCyl::on_btnSLSTR_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
    formtoolbox->setTabWidgetSentinelIndex(1);
    toggleButton(eSegmentType::SEG_SLSTR);
    this->RemoveAllSelected();
    //    mapcyl->update();
    //    this->showSegmentCount();
    this->setScrollBarMaximum();

    return;
}

void FormMapCyl::on_btnOLCIefrDatahub_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
    formtoolbox->setTabWidgetSentinelIndex(0);
    toggleButton(eSegmentType::SEG_DATAHUB_OLCIEFR);
    //this->RemoveAllSelected();
    mapcyl->update();
    this->showSegmentCount();
    this->setScrollBarMaximum();

    return;
}

void FormMapCyl::on_btnOLCIerrDatahub_clicked()
{

    formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
    formtoolbox->setTabWidgetSentinelIndex(0);
    toggleButton(eSegmentType::SEG_DATAHUB_OLCIERR);
    //this->RemoveAllSelected();
    mapcyl->update();
    this->showSegmentCount();
    this->setScrollBarMaximum();

    return;
}

void FormMapCyl::on_btnSLSTRDatahub_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
    formtoolbox->setTabWidgetSentinelIndex(1);
    toggleButton(eSegmentType::SEG_DATAHUB_SLSTR);
    //this->RemoveAllSelected();
    mapcyl->update();
    this->showSegmentCount();
    this->setScrollBarMaximum();

    return;
}

void FormMapCyl::on_btnMERSI_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_MERSI);
    toggleButton(eSegmentType::SEG_MERSI);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();

}

void FormMapCyl::on_btnAllSegments_clicked()
{
    segs->setShowAllSegments(ui->btnAllSegments->isChecked());
    opts.buttonShowAllSegments = ui->btnAllSegments->isChecked();
}

void FormMapCyl::on_btnPhong_clicked()
{
    opts.buttonPhong = ui->btnPhong->isChecked();
}

bool FormMapCyl::IsProductDirFilledIn()
{
    if(opts.productdirectory.isEmpty())
    {
        QMessageBox::critical(this, "Product directory is empty !", "Please first create a directory where you want to download the products.\n "
                                                                    "Open Scihub/CODA Config in Preferences and select the created directory.", QMessageBox::Ok);
        return false;
    }
    return true;
}

void FormMapCyl::on_btnDownloadCompleteProduct_clicked()
{

    if(!IsProductDirFilledIn())
        return;

    if(!CheckUserAndPassword())
        return;

    if(todownloadlist.count() ==  0)
        return;

    for(int i = 0; i < todownloadlist.count(); i++)
    {
        todownloadlist[i].band_or_quicklook = "complete";
    }


    SearchForFreeManager();

}

void FormMapCyl::on_btnDownloadQuicklook_clicked()
{
    if(!IsProductDirFilledIn())
        return;

    if(!CheckUserAndPassword())
        return;

    QList<ProductList> newtodownloadlist;

    if(todownloadlist.count() ==  0)
        return;

    for(int i = 0; i < todownloadlist.count(); i++)
    {
        if(!QuicklookExist(todownloadlist.at(i).completebasename))
        {
            todownloadlist[i].band_or_quicklook = "quicklook";
            newtodownloadlist.append(todownloadlist.at(i));
            ProductList newtoadd;
            newtoadd.completebasename = todownloadlist.at(i).completebasename;
            newtoadd.uuid = todownloadlist.at(i).uuid;
            newtoadd.status = "waiting";
            if(newtoadd.completebasename.mid(9, 3) == "RBT")
                newtoadd.band_or_quicklook = "geodetic_an.nc";
            else
                newtoadd.band_or_quicklook = "tie_geo_coordinates.nc";

            newtodownloadlist.append(newtoadd);
        }
        else
        {
            RenderQuicklookinTexture(todownloadlist.at(i).completebasename);
        }
    }

    todownloadlist = newtodownloadlist;

    showSelectedSegmentToDownloadList();

    SearchForFreeManager();

}

void FormMapCyl::on_btnDownloadPartialProduct_clicked()
{
    if(!IsProductDirFilledIn())
        return;

    if(!CheckUserAndPassword())
        return;

    if(ui->btnSLSTRDatahub->isChecked())
        return;

    QList<bool> bandlist = formtoolbox->getOLCIBandList();
    QList<int> colorlist = formtoolbox->getOLCIColorList();

    QList<ProductList> newtodownloadlist;

    if(todownloadlist.count() ==  0)
        return;

    for(int i = 0; i < todownloadlist.count(); i++)
    {
        if(!SegmentListOLCI::OLCIFileExist(todownloadlist.at(i).completebasename, "geo_coordinates.nc"))
        {
            todownloadlist[i].band_or_quicklook = "geo_coordinates.nc";
            newtodownloadlist.append(todownloadlist.at(i)); // 1
        }

        ProductList newtoadd;
        newtoadd.completebasename = todownloadlist.at(i).completebasename;
        newtoadd.uuid = todownloadlist.at(i).uuid;
        newtoadd.status = "waiting";

        if(!SegmentListOLCI::OLCIFileExist(todownloadlist.at(i).completebasename, "tie_geometries.nc"))
        {
            if(newtoadd.completebasename.mid(9, 3) == "RBT")
                newtoadd.band_or_quicklook = "";
            else
                newtoadd.band_or_quicklook = "tie_geometries.nc";
            newtodownloadlist.append(newtoadd); // 2
        }

        if(!SegmentListOLCI::OLCIFileExist(todownloadlist.at(i).completebasename, "qualityFlags.nc"))
        {
            newtoadd.band_or_quicklook = "qualityFlags.nc";
            newtodownloadlist.append(newtoadd); // 3
        }

        if(bandlist.at(0) == false) // No color
        {
            QString bandfile;
            SegmentListOLCI::getDatasetNameFromBand(bandlist, &bandfile );
            if(!SegmentListOLCI::OLCIFileExist(todownloadlist.at(i).completebasename, bandfile))
            {
                newtoadd.band_or_quicklook = bandfile;
                newtodownloadlist.append(newtoadd);
            }
        }
        else
        {
            QString colorfile;
            SegmentListOLCI::getDatasetNameFromColor(colorlist, 0, &colorfile);
            if(!SegmentListOLCI::OLCIFileExist(todownloadlist.at(i).completebasename, colorfile))
            {
                newtoadd.band_or_quicklook = colorfile;
                newtodownloadlist.append(newtoadd);
            }
            SegmentListOLCI::getDatasetNameFromColor(colorlist, 1, &colorfile);
            if(!SegmentListOLCI::OLCIFileExist(todownloadlist.at(i).completebasename, colorfile))
            {
                newtoadd.band_or_quicklook = colorfile;
                newtodownloadlist.append(newtoadd);
            }
            SegmentListOLCI::getDatasetNameFromColor(colorlist, 2, &colorfile);
            if(!SegmentListOLCI::OLCIFileExist(todownloadlist.at(i).completebasename, colorfile))
            {
                newtoadd.band_or_quicklook = colorfile;
                newtodownloadlist.append(newtoadd);
            }
        }

    }

    todownloadlist = newtodownloadlist;

    showSelectedSegmentToDownloadList();

    SearchForFreeManager();

}

void FormMapCyl::RenderQuicklookinTexture(QString completebasename)
{
    QColor rgb;
    int posx, posy;
    int retval;
    int tiegeofileid;
    int tiecolumnsid, tierowsid;
    size_t tiecolumnslength, tierowslength = 0;
    int *longitude_tie;
    int *latitude_tie;

    int geofileid;
    int columnsid, rowsid;
    size_t columnslength, rowslength;

    int *longitude_img;
    int *latitude_img;

    //    if(completebasename.mid(9, 3) == "RBT")
    //        return;

    QDir dir(opts.productdirectory);

    QString fileyear = completebasename.mid(16, 4);
    QString filemonth = completebasename.mid(20, 2);
    QString fileday = completebasename.mid(22,2);

    QString quicklookpath(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday + "/" + completebasename + "/quicklook/" + completebasename + ".jpg");
    QString tiegeopath(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday + "/" + completebasename);
    QString geopath(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday + "/" + completebasename);

    if(completebasename.mid(9, 3) == "RBT")
        geopath.append("/geodetic_an.nc");
    else
    {
        tiegeopath.append("/tie_geo_coordinates.nc");
        geopath.append("/geo_coordinates.nc");
    }

    bool bexistquicklookpath = QFileInfo::exists(quicklookpath) && QFileInfo(quicklookpath).isFile();
    bool bexisttiegeopath = QFileInfo::exists(tiegeopath) && QFileInfo(tiegeopath).isFile();
    bool bexistgeopath = QFileInfo::exists(geopath) && QFileInfo(geopath).isFile();

    qDebug() << "quicklookpath exist = " << bexistquicklookpath << " " << quicklookpath;
    qDebug() << "tiegeopath exist = " << bexisttiegeopath << " " << tiegeopath;
    qDebug() << "geopath exist = " << bexistgeopath << " " << geopath;

    if(!(bexistquicklookpath && (bexisttiegeopath || bexistgeopath)))
    {
        return;
    }

    bool bslstrfile = false;
    bool bolcifile = false;

    QImage img(quicklookpath);
    if(completebasename.mid(9, 3) == "EFR")
    {
        columnslength = 4865; // (4090, 4865)
        bslstrfile = false;
        bolcifile = true;
    }
    else if(completebasename.mid(9, 3) == "ERR")
    {
        columnslength = 1217; // (14997, 1217)
        bslstrfile = false;
        bolcifile = true;
    }
    else if(completebasename.mid(9, 3) == "RBT")
    {
        columnslength = 2400; // (2400, 1800)
        bslstrfile = true;
        bolcifile = false;
    }
    else
        columnslength = 0;

    if(bexistgeopath && bslstrfile)
    {
        QByteArray arraygeocoordinates = geopath.toUtf8();
        const char *pgeocoordinatesfile = arraygeocoordinates.constData();

        retval = nc_open(pgeocoordinatesfile, NC_NOWRITE, &geofileid);
        if(retval != NC_NOERR) qDebug() << "error opening geo_coordinates";

        retval = nc_inq_dimid(geofileid, "columns", &columnsid);
        if(retval != NC_NOERR) qDebug() << "error reading columns id";
        retval = nc_inq_dimlen(geofileid, columnsid, &columnslength);
        if(retval != NC_NOERR) qDebug() << "error reading columns length";

        retval = nc_inq_dimid(geofileid, "rows", &rowsid);
        if(retval != NC_NOERR) qDebug() << "error reading rows id";
        retval = nc_inq_dimlen(geofileid, rowsid, &rowslength); // 4091 or 14997
        if(retval != NC_NOERR) qDebug() << "error reading tie_rows length";

        longitude_img = new int[rowslength*columnslength];
        latitude_img = new int[rowslength*columnslength];

        tierowslength = rowslength;
        int longitudeid, latitudeid;

        retval = nc_inq_varid(geofileid, "longitude_an", &longitudeid);
        if (retval != NC_NOERR) qDebug() << "error reading longitude_an id";
        retval = nc_get_var_int(geofileid, longitudeid, longitude_img);
        if (retval != NC_NOERR) qDebug() << "error reading longitude_an values";

        retval = nc_inq_varid(geofileid, "latitude_an", &latitudeid);
        if (retval != NC_NOERR) qDebug() << "error reading latitude_an id";
        retval = nc_get_var_int(geofileid, latitudeid, latitude_img);
        if (retval != NC_NOERR) qDebug() << "error reading latitude_an values";

        retval = nc_close(geofileid);
        if (retval != NC_NOERR) qDebug() << "error closing geo_coordinates";

    }
    else if(bexistgeopath && bolcifile)
    {
        QByteArray arraygeocoordinates = geopath.toUtf8();
        const char *pgeocoordinatesfile = arraygeocoordinates.constData();

        retval = nc_open(pgeocoordinatesfile, NC_NOWRITE, &geofileid);
        if(retval != NC_NOERR) qDebug() << "error opening geo_coordinates";

        retval = nc_inq_dimid(geofileid, "columns", &columnsid);
        if(retval != NC_NOERR) qDebug() << "error reading columns id";
        retval = nc_inq_dimlen(geofileid, columnsid, &columnslength);
        if(retval != NC_NOERR) qDebug() << "error reading columns length";

        retval = nc_inq_dimid(geofileid, "rows", &rowsid);
        if(retval != NC_NOERR) qDebug() << "error reading rows id";
        retval = nc_inq_dimlen(geofileid, rowsid, &rowslength); // 4091 or 14997
        if(retval != NC_NOERR) qDebug() << "error reading tie_rows length";

        longitude_img = new int[rowslength*columnslength];
        latitude_img = new int[rowslength*columnslength];

        tierowslength = rowslength;
        int longitudeid, latitudeid;

        retval = nc_inq_varid(geofileid, "longitude", &longitudeid);
        if (retval != NC_NOERR) qDebug() << "error reading longitude id";
        retval = nc_get_var_int(geofileid, longitudeid, longitude_img);
        if (retval != NC_NOERR) qDebug() << "error reading longitude values";

        retval = nc_inq_varid(geofileid, "latitude", &latitudeid);
        if (retval != NC_NOERR) qDebug() << "error reading latitude id";
        retval = nc_get_var_int(geofileid, latitudeid, latitude_img);
        if (retval != NC_NOERR) qDebug() << "error reading latitude values";

        retval = nc_close(geofileid);
        if (retval != NC_NOERR) qDebug() << "error closing geo_coordinates";




    }
    else if(bexisttiegeopath && bolcifile)
    {
        QByteArray arraytiegeocoordinates = tiegeopath.toUtf8();
        const char *ptiegeocoordinatesfile = arraytiegeocoordinates.constData();

        retval = nc_open(ptiegeocoordinatesfile, NC_NOWRITE, &tiegeofileid);
        if(retval != NC_NOERR) qDebug() << "error opening tie_geo_coordinates";

        retval = nc_inq_dimid(tiegeofileid, "tie_columns", &tiecolumnsid);
        if(retval != NC_NOERR) qDebug() << "error reading tie_columns id";
        retval = nc_inq_dimlen(tiegeofileid, tiecolumnsid, &tiecolumnslength); // 77
        if(retval != NC_NOERR) qDebug() << "error reading tie_columns length";

        retval = nc_inq_dimid(tiegeofileid, "tie_rows", &tierowsid);
        if(retval != NC_NOERR) qDebug() << "error reading tie_rows id";
        retval = nc_inq_dimlen(tiegeofileid, tierowsid, &tierowslength); // 4091 or 14997
        if(retval != NC_NOERR) qDebug() << "error reading tie_rows length";

        longitude_tie = new int[tiecolumnslength * tierowslength]; // new int[77*4091];
        latitude_tie = new int[tiecolumnslength * tierowslength];

        longitude_img = new int[tierowslength*columnslength];
        latitude_img = new int[tierowslength*columnslength];


        int longitudetieid, latitudetieid;

        retval = nc_inq_varid(tiegeofileid, "longitude", &longitudetieid);
        if (retval != NC_NOERR) qDebug() << "error reading longitudetie id";
        retval = nc_get_var_int(tiegeofileid, longitudetieid, longitude_tie);
        if (retval != NC_NOERR) qDebug() << "error reading longitude_tie values";

        retval = nc_inq_varid(tiegeofileid, "latitude", &latitudetieid);
        if (retval != NC_NOERR) qDebug() << "error reading latitudetie id";
        retval = nc_get_var_int(tiegeofileid, latitudetieid, latitude_tie);
        if (retval != NC_NOERR) qDebug() << "error reading latitude_tie values";

        retval = nc_close(tiegeofileid);
        if (retval != NC_NOERR) qDebug() << "error closing tie_geo_coordinates";

        int val1, val2, diff;

        int factor = (columnslength-1)/(tiecolumnslength-1);


        //    Debug Debug: "rowslength = 4091 columnslength : 4865 earth_views_per_scanline = 4865"
        //    Debug Debug: "tierowslength = 4091 tiecolumnslength : 77 NbrOfLines = 4091"
        //    Debug Debug: "rowslength * columnslength = 19902715 factor = 64 "

        //    // Linear interpolation
        for(int j=0; j < tierowslength; j++)
        {
            val1 = 0;
            val2 = 0;
            for(int i=0; i < tiecolumnslength-1; i++) // tiecolumnslength = 77
            {

                val1 = longitude_tie[j*tiecolumnslength + i];
                val2 = longitude_tie[j*tiecolumnslength + i+1];
                if(val1 < -179000000 && val2 > 179000000)
                    diff = (360000000 - val2 + val1)/factor;
                else if( val1 > 179000000 && val2 < -179000000)
                    diff = (360000000 - val1 + val2)/factor;
                else
                    diff = (val2 - val1)/factor;

                for(int k=0; k < factor; k++)
                {
                    int limg = val1 + diff*k;
                    if(limg < -180000000)
                        limg = 360000000 + limg;
                    else if(limg > 180000000)
                        limg = 360000000 - limg;

                    longitude_img[j*columnslength + i*factor + k] = limg;
                }
            }
            longitude_img[j*columnslength + (tiecolumnslength - 1)*factor] = val2;
        }

        for(int j=0; j < tierowslength; j++)
        {
            val1 = 0;
            val2 = 0;
            for(int i=0; i < tiecolumnslength-1; i++) // tiecolumnslength = 77
            {
                val1 = latitude_tie[j*tiecolumnslength + i];
                val2 = latitude_tie[j*tiecolumnslength + i+1];
                diff = (val2 - val1)/factor;

                for(int k=0; k < factor; k++)
                {
                    int limg = val1 + diff*k;
                    if(limg < -90000000)
                        limg = -(180000000 + limg);
                    else if(limg > 90000000)
                        limg = 180000000 - limg;

                    latitude_img[j*columnslength + i*factor + k] = limg;
                }
            }
            latitude_img[j*columnslength + (tiecolumnslength - 1)*factor] = val2;
        }

        delete [] longitude_tie;
        delete [] latitude_tie;

    }


    QImage imgscaled = img.scaled(columnslength, tierowslength);
    //imgscaled.save("mytest.jpg");
    QPainter fb_painter(imageptrs->pmOut);

    int devwidth = (fb_painter.device())->width();
    int devheight = (fb_painter.device())->height();

    int imgwidth = img.width();
    int imgheight = img.height();


    qDebug() << QString("columnslength : %2 ").arg(columnslength);
    qDebug() << QString("imgwidth = %1 imgheight = %2 ").arg(imgwidth).arg(imgheight);


    fb_painter.setPen( Qt::black );
    fb_painter.setBrush( Qt::NoBrush );

    float flon, flat, fflon, fflat;


    rgb.setRgb(0, 255, 0);

    QString segtype = completebasename.mid(9, 3);
    for(int yimg = 0; yimg < imgscaled.height(); yimg+=4)
    {

        QRgb *row = (QRgb *)imgscaled.scanLine(yimg);
        for (int ximg = 0 ; ximg < imgscaled.width(); ximg+=4)
        {
            fflon = (float)longitude_img[yimg*columnslength + ximg]/1000000.0;
            fflat = (float)latitude_img[yimg*columnslength + ximg]/1000000.0;
            flon = fflon * PI/180.0;
            flat = fflat * PI/180.0;
            if(segtype == "ERR" && ximg > 40 && ximg < imgscaled.width() - 8)
            {
                sphericalToPixel( flon, flat, posx, posy, devwidth, devheight );
                rgb.setRgb(qRed(row[ximg]), qGreen(row[ximg]), qBlue(row[ximg]));
                fb_painter.setPen(rgb);
                fb_painter.drawPoint(posx , posy );
            }
            else if(segtype == "RBT" && ximg > 105 && ximg < imgscaled.width() - 40)
            {
                sphericalToPixel( flon, flat, posx, posy, devwidth, devheight );
                rgb.setRgb(qRed(row[ximg]), qGreen(row[ximg]), qBlue(row[ximg]));
                fb_painter.setPen(rgb);
                fb_painter.drawPoint(posx , posy );
            }
            else if(segtype == "EFR" && ximg > 157 && ximg < imgscaled.width() - 43)
            {
                sphericalToPixel( flon, flat, posx, posy, devwidth, devheight );
                rgb.setRgb(qRed(row[ximg]), qGreen(row[ximg]), qBlue(row[ximg]));
                fb_painter.setPen(rgb);
                fb_painter.drawPoint(posx , posy );
            }

        }

    }

    fb_painter.end();

    delete [] longitude_img;
    delete [] latitude_img;

    opts.texture_changed = true;

}

#define FILE_NAME "mynetcdf.nc"
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

bool FormMapCyl::WriteNetCDFFile(int *longitude_img, int *latitude_img, int tierowslength, int columnslength)
{
    int ncid, x_dimid, y_dimid, varid1, varid2;
    int retval;
    int dimids[2];
    //int data_out[tierowslength][columnslength];

    qDebug() << "Writing NetCDF file tierowslength = " << tierowslength << " columnslength = " << columnslength;

    /* Create some pretend data. */
    //    for (int x = 0; x < tierowslength; x++)
    //       for (int y = 0; y < columnslength; y++)
    //       {
    //          data_out[x][y] = x * columnslength + y;
    //       }

    //    int longitude[columnslength][tierowslength];

    //    for(int j = 0; j < tierowslength; j++)
    //    {
    //        for(int i = 0; i < columnslength; i++)
    //        {
    //            longitude[i][j] = longitude_img[j*columnslength + j];
    //        }
    //    }

    if ((retval = nc_create(FILE_NAME, NC_NETCDF4|NC_CLOBBER, &ncid)))
        ERR(retval);

    /* Define the dimensions in the root group. Dimensions are visible
     * in all subgroups. */
    if ((retval = nc_def_dim(ncid, "x", tierowslength, &x_dimid)))
        ERR(retval);
    if ((retval = nc_def_dim(ncid, "y", columnslength, &y_dimid)))
        ERR(retval);

    /* The dimids passes the IDs of the dimensions of the variable. */
    dimids[0] = x_dimid;
    dimids[1] = y_dimid;

    /* Define an unsigned 64bit integer variable in grp1, using dimensions
     * in the root group. */
    if ((retval = nc_def_var(ncid, "longitude_img", NC_INT, 2, dimids, &varid1)))
        ERR(retval);

    /* Write unsigned long long data to the file. For netCDF-4 files,
     * nc_enddef will be called automatically. */
    if ((retval = nc_put_var_int(ncid, varid1, longitude_img)))
        ERR(retval);

    /* Close the file. */
    if ((retval = nc_close(ncid)))
        ERR(retval);


}

bool FormMapCyl::QuicklookExist(QString completebasename)
{
    // S3A_OL_1_EFR____20201205T102330_20201205T102630_20201205T121305_0179_066_008_2340_LN1_O_NR_002.SEN3
    // S3A_OL_1_ERR____20201210T094858_20201210T103307_20201210T115918_2649_066_079______LN1_O_NR_002.SEN3
    // 01234567890123456789012345678901234567890123456789012345678901234567890123456789

    QDir dir(opts.productdirectory);
    QString returndirstr;

    QString fileyear = completebasename.mid(16, 4);
    QString filemonth = completebasename.mid(20, 2);
    QString fileday = completebasename.mid(22,2);

    QString jpgstr(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday + "/" + completebasename + "/quicklook/" + completebasename + ".jpg");
    QString geostr(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday + "/" + completebasename);

    if(completebasename.mid(9, 3) == "RBT")
        geostr.append("/geodetic_an.nc");
    else
        geostr.append("/tie_geo_coordinates.nc");

    QFile jpgfile(jpgstr);
    QFile geofile(geostr);
    if(jpgfile.exists() && geofile.exists())
        return true;
    else
        return false;

}

void FormMapCyl::SearchForFreeManager()
{
    eDatahub hub;
    if(opts.provideresaoreumetsat)
        hub = HUBESA;
    else
        hub = HUBEUMETSAT;

    qDebug() <<  "FormMapCyl::SearchForFreeManager()";

    for(int i = 0; i < todownloadlist.count() ; i++)
    {
        if(todownloadlist.at(i).status == "waiting")
        {
            if(!hubmanagerprod1.isProductDownloadBusy())
            {
                qDebug() << "hubmanagerprod1.DownloadProduct";
                QObject::connect(&hubmanagerprod1, &DatahubAccessManager::productFinished, this, &FormMapCyl::productFileDownloaded);
                QObject::connect(&hubmanagerprod1, &DatahubAccessManager::productProgress, this, &FormMapCyl::productDownloadProgress);
                todownloadlist[i].status = "busy";
                hubmanagerprod1.DownloadProduct(todownloadlist, i, hub, 0);
                showSelectedSegmentToDownloadList();
            }
            else if(!hubmanagerprod2.isProductDownloadBusy())
            {
                qDebug() << "hubmanagerprod2.DownloadProduct";

                QObject::connect(&hubmanagerprod2, &DatahubAccessManager::productFinished, this, &FormMapCyl::productFileDownloaded);
                QObject::connect(&hubmanagerprod2, &DatahubAccessManager::productProgress, this, &FormMapCyl::productDownloadProgress);
                todownloadlist[i].status = "busy";
                hubmanagerprod2.DownloadProduct(todownloadlist, i, hub, 1);
                showSelectedSegmentToDownloadList();
            }
        }
    }

    for(int i = 0; i < todownloadlist.count() ; i++)
    {
        if(todownloadlist.at(i).status == "busy")
        {
            ui->btnDownloadCompleteProduct->setEnabled(false);
            ui->btnDownloadPartialProduct->setEnabled(false);
            ui->btnDownloadQuicklook->setEnabled(false);
            break;
        }
    }

}

void FormMapCyl::productDownloadProgress(qint64 bytesReceived, qint64 bytesTotal, int whichdownload)
{
    if( whichdownload == 0)
    {
        ui->pbProduct1->setMaximum(bytesTotal);
        ui->pbProduct1->setValue(bytesReceived);
    }
    else
    {
        ui->pbProduct2->setMaximum(bytesTotal);
        ui->pbProduct2->setValue(bytesReceived);
    }
}

void FormMapCyl::productFileDownloaded(int whichdownload, int downloadindex, QString absoluteproductpath, QString absolutepath, QString filename)
{

    qDebug() << "productFileDownloaded absoluteproductpath = " << absoluteproductpath;
    qDebug() << "productFileDownloaded absolutepath = " << absolutepath;
    qDebug() << "productFileDownloaded filename = " << filename;
    qDebug() << "todownloadlist[downloadindex].band_or_quicklook = " << todownloadlist[downloadindex].band_or_quicklook;

    if(whichdownload == 0)
    {
        qDebug() << "FormMapCyl::productFileDownloaded whichdownload = 0";
        QObject::disconnect(&hubmanagerprod1, &DatahubAccessManager::productFinished, this, &FormMapCyl::productFileDownloaded);
        QObject::disconnect(&hubmanagerprod1, &DatahubAccessManager::productProgress, this, &FormMapCyl::productDownloadProgress);
        ui->pbProduct1->setValue(0);
    }
    else
    {
        qDebug() << "FormMapCyl::productFileDownloaded whichdownload = 1";
        QObject::disconnect(&hubmanagerprod2, &DatahubAccessManager::productFinished, this, &FormMapCyl::productFileDownloaded);
        QObject::disconnect(&hubmanagerprod2, &DatahubAccessManager::productProgress, this, &FormMapCyl::productDownloadProgress);
        ui->pbProduct2->setValue(0);
    }

    if(downloadindex < todownloadlist.count())
    {
        todownloadlist[downloadindex].status = "finished";
        todownloadlist[downloadindex].absoluteproductpath = absoluteproductpath;
    }
    showSelectedSegmentToDownloadList();

    qDebug() << "hubmanagerprod1.isProductDownloadBusy() = " << hubmanagerprod1.isProductDownloadBusy();
    qDebug() << "hubmanagerprod2.isProductDownloadBusy() = " << hubmanagerprod2.isProductDownloadBusy();

    if(todownloadlist[downloadindex].band_or_quicklook == "complete")
    {
        qDebug() << "Start extraction to " << absolutepath;
        QString ArchivePath = absoluteproductpath;
        QString DestinationPath = absolutepath;
        ExtractSegment(ArchivePath, DestinationPath);
        qDebug() << "Removing " << ArchivePath;
        QFile::remove(ArchivePath);
    }

    if((!hubmanagerprod1.isProductDownloadBusy()) && (!hubmanagerprod2.isProductDownloadBusy()))
    {
        ui->btnDownloadCompleteProduct->setEnabled(true);
        ui->btnDownloadPartialProduct->setEnabled(true);
        ui->btnDownloadQuicklook->setEnabled(true);
        ui->btnCancelDownloadProduct->setEnabled(true);
    }


    bool alldownloaded = true;
    for(int i = 0; i < todownloadlist.count() ; i++)
    {
        qDebug() << "status = " << todownloadlist.at(i).status;
        if(todownloadlist.at(i).status != "finished")
        {
            alldownloaded = false;
            break;
        }
    }


    if(alldownloaded)
    {
        for(int i = 0; i < todownloadlist.count(); i++)
        {
            qDebug() << "todownloadlist " << todownloadlist.at(i).band_or_quicklook << " " << todownloadlist.at(i).completebasename;
            if(todownloadlist.at(i).band_or_quicklook == "quicklook")
            {
                RenderQuicklookinTexture(todownloadlist.at(i).completebasename);
            }
        }
        segs->ReadDirectoriesDatahub(ui->calendarDatahub->selectedDate());
    }

    SearchForFreeManager();

}

int FormMapCyl::ExtractSegment(QString ArchivePath, QString DestinationPath)
{

    int flags = ARCHIVE_EXTRACT_TIME;
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;


    qDebug() << "Start ExtractSegment for absolutefilepath " + ArchivePath;

    QByteArray array = ArchivePath.toUtf8();
    const char* p = array.constData();

    a = archive_read_new();
    ext = archive_write_disk_new();
    //archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    archive_write_disk_set_options(ext, flags);

    r = archive_read_open_filename(a, p, 20480);
    if (r != ARCHIVE_OK)
    {
        qDebug() << "Input file " << ArchivePath << " not found ....";
        return(1);
    }

    //    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
    //    {
    //      qDebug() << QString("%1").arg(archive_entry_pathname(entry));
    //      archive_read_data_skip(a);  // Note 2
    //    }

    int nbrblocks = 1;

    for (;;)
    {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r != ARCHIVE_OK)
            qDebug() << "archive_read_next_header() " << QString(archive_error_string(a));
        const char* currentFile = archive_entry_pathname(entry);
        QString strcurrentFile = QString::fromUtf8((char*)currentFile);
        QString fullOutputPath = DestinationPath + "/" + strcurrentFile;
        QByteArray fullarray = fullOutputPath.toUtf8();
        const char* pfullarray = fullarray.constData();

        archive_entry_set_pathname(entry, pfullarray);
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK)
            qDebug() << "archive_write_header() " << QString(archive_error_string(ext));
        else
        {
            qDebug() << QString("Start copy_data ....%1").arg(nbrblocks);

            copy_data(a, ext);
            r = archive_write_finish_entry(ext);
            if (r != ARCHIVE_OK)
                qDebug() << "archive_write_finish_entry() " << QString(archive_error_string(ext));
            nbrblocks++;
        }
    }

    archive_read_close(a);
    archive_read_free(a);

    archive_write_close(ext);
    archive_write_free(ext);

    return(0);
}

int FormMapCyl::copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif


    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r != ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK) {
            qDebug() << "archive_write_data_block() " << QString(archive_error_string(aw));
            return (r);
        }
    }
}



void FormMapCyl::createSelectedSegmentToDownloadList()
{
    QList<Segment*> *sldatahubolciefr = segs->segldatahubolciefr->GetSegmentlistptr();
    QList<Segment*> *sldatahubolcierr = segs->segldatahubolcierr->GetSegmentlistptr();
    QList<Segment*> *sldatahubslstr = segs->segldatahubslstr->GetSegmentlistptr();

    todownloadlist.clear();

    //if (opts.buttonDatahubOLCIefr)
    {
        for(int i = 0; i < sldatahubolciefr->count(); i++)
        {
            if( sldatahubolciefr->at(i)->IsSelected())
            {
                ProductList prodlist;
                SegmentDatahub *segdatahub = (SegmentDatahub *)sldatahubolciefr->at(i);
                prodlist.completebasename = segdatahub->fileInfo.fileName();
                prodlist.uuid = segdatahub->getUUID();
                prodlist.size = segdatahub->getSize();
                prodlist.status = "waiting";
                prodlist.band_or_quicklook = "";
                todownloadlist.append(prodlist);
            }
        }
    }
    //else if (opts.buttonDatahubOLCIerr)
    {
        QList<Segment*>::iterator segitolcierr = sldatahubolcierr->begin();
        while ( segitolcierr != sldatahubolcierr->end() )
        {
            if((*segitolcierr)->IsSelected())
            {
                ProductList prodlist;
                prodlist.completebasename = (*segitolcierr)->fileInfo.fileName();
                prodlist.uuid = ((SegmentDatahub *)(*segitolcierr))->getUUID();
                prodlist.size = ((SegmentDatahub *)(*segitolcierr))->getSize();
                prodlist.status = "waiting";
                prodlist.band_or_quicklook = "";
                todownloadlist.append(prodlist);
            }
            ++segitolcierr;
        }

    }
    //else if (opts.buttonDatahubSLSTR)
    {
        QList<Segment*>::iterator segitslstr = sldatahubslstr->begin();
        while ( segitslstr != sldatahubslstr->end() )
        {
            if((*segitslstr)->IsSelected())
            {
                ProductList prodlist;
                prodlist.completebasename = (*segitslstr)->fileInfo.fileName();
                prodlist.uuid = ((SegmentDatahub *)(*segitslstr))->getUUID();
                prodlist.size = ((SegmentDatahub *)(*segitslstr))->getSize();
                prodlist.status = "waiting";
                prodlist.band_or_quicklook = "";
                todownloadlist.append(prodlist);
            }
            ++segitslstr;
        }

    }

    showSelectedSegmentToDownloadList();
}

void FormMapCyl::showSelectedSegmentToDownloadList()
{
    bool ok;

    QBrush background(Qt::green);
    double totalsize = 0;
    ui->twSelectedProducts->clearContents();
    ui->twSelectedProducts->setRowCount(0);

    //S3A_OL_1_EFR____20161026T121318_20161026T121318_20161026T163853_0000_010_166______MAR_O_NR_002.SEN3.tar
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012

    for( int i = 0; i < todownloadlist.count(); ++i )
    {
        ui->twSelectedProducts->insertRow(i);

        if(todownloadlist.at(i).status == "finished")
            background.setColor(Qt::green);
        else
            background.setColor(Qt::white);
        QTableWidgetItem *item0 = new QTableWidgetItem(todownloadlist.at(i).status);
        item0->setBackground(background);
        ui->twSelectedProducts->setItem(i, 0, item0);

        QTableWidgetItem *item1 = new QTableWidgetItem(todownloadlist.at(i).band_or_quicklook);  // complete or quiclook or band
        item1->setBackground(background);
        ui->twSelectedProducts->setItem(i, 1, item1);

        QString year = todownloadlist.at(i).completebasename.mid(16, 4);
        QString month = todownloadlist.at(i).completebasename.mid(20, 2);
        QString day = todownloadlist.at(i).completebasename.mid(22, 2);
        QTableWidgetItem *item2 = new QTableWidgetItem(year + "-" + month + "-" + day);
        item2->setBackground(background);
        ui->twSelectedProducts->setItem(i, 2, item2);

        QString hour = todownloadlist.at(i).completebasename.mid(25, 2);
        QString min = todownloadlist.at(i).completebasename.mid(27, 2);
        QString sec = todownloadlist.at(i).completebasename.mid(29, 2);
        QTableWidgetItem *item3 = new QTableWidgetItem(hour + ":" + min + ":" + sec);
        item3->setBackground(background);
        ui->twSelectedProducts->setItem(i, 3, item3);

        hour = todownloadlist.at(i).completebasename.mid(41, 2);
        min = todownloadlist.at(i).completebasename.mid(43, 2);
        sec = todownloadlist.at(i).completebasename.mid(45, 2);
        QTableWidgetItem *item4 = new QTableWidgetItem(hour + ":" + min + ":" + sec);
        item4->setBackground(background);
        ui->twSelectedProducts->setItem(i, 4, item4);

        int strsizelength = todownloadlist.at(i).size.length();
        QString strsize = todownloadlist.at(i).size.mid(0, todownloadlist.at(i).size.indexOf(" MB"));
        double lsize = strsize.toDouble(&ok);
        if(ok)
            totalsize += lsize;

        QTableWidgetItem *item5 = new QTableWidgetItem(todownloadlist.at(i).size);
        item5->setBackground(background);
        ui->twSelectedProducts->setItem(i, 5, item5);

        ui->twSelectedProducts->setColumnWidth(0, 60); // status
        ui->twSelectedProducts->setColumnWidth(1, 80); // type
        ui->twSelectedProducts->setColumnWidth(2, 80); // date
        ui->twSelectedProducts->setColumnWidth(3, 70); // start
        ui->twSelectedProducts->setColumnWidth(4, 70); // end
        ui->twSelectedProducts->setColumnWidth(5, 80); // size
    }

    //    float ftotalsize = (float)totalsize/1000000;
    ui->lblTotalDownloadSize->setText(QString("Total size complete product = %1 Mb").arg(totalsize, 0, 'f', 2));

    ui->twSelectedProducts->resizeRowsToContents();
    ui->twSelectedProducts->show();
}

void FormMapCyl::slotShowXMLProgress(QString str, int pages, bool downloadinprogress)
{
    ui->pbXMLprogress->setMaximum(10);

    ui->lblTotalAvailable->setText(str);
    if(downloadinprogress)
    {
        ui->btnDownloadQuicklook->setEnabled(false);
        ui->btnDownloadCompleteProduct->setEnabled(false);
        ui->btnDownloadPartialProduct->setEnabled(false);
        ui->btnCancelDownloadProduct->setEnabled(true);
    }
    else
    {
        ui->btnDownloadQuicklook->setEnabled(true);
        ui->btnDownloadCompleteProduct->setEnabled(true);
        ui->btnDownloadPartialProduct->setEnabled(true);
        ui->btnCancelDownloadProduct->setEnabled(true);
    }

    if(pages == 999)
        ui->pbXMLprogress->setValue(10);
    else
        ui->pbXMLprogress->setValue(pages);
}


void FormMapCyl::on_btnCancelDownloadProduct_clicked()
{
    hubmanagerprod1.CancelDownload();
    hubmanagerprod2.CancelDownload();
    ui->pbProduct1->setValue(0);
    ui->pbProduct2->setValue(0);

    todownloadlist.clear();

    RemoveAllSelected();

    ui->twSelectedProducts->clearContents();
    ui->twSelectedProducts->setRowCount(0);

    ui->btnDownloadCompleteProduct->setEnabled(true);
    ui->btnDownloadPartialProduct->setEnabled(true);
    ui->btnDownloadQuicklook->setEnabled(true);


}


void FormMapCyl::on_btnDownloadXMLFromDatahub_clicked()
{
    bool ok = false;
    QFile segfile("Segments.xml");
    segfile.remove();

    QString type;
    if(ui->rdbDownloadXMLOLCIEFR->isChecked())
        type = "EFR";
    else if(ui->rdbDownloadXMLOLCIERR->isChecked())
        type = "ERR";
    else if(ui->rdbDownloadXMLSLSTR->isChecked())
        type = "SLSTR";

    if(CheckUserAndPassword())
        segs->LoadXMLfromDatahub(ui->calendarDatahub->selectedDate(), type);
}

bool FormMapCyl::CheckUserAndPassword()
{

    bool okuser = false;
    bool okpassword = false;

    if(opts.datahubuser.isEmpty())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::critical(this, tr("Datahub User Id"),
                                      "The Datahub User Id is empty. Open the 'Preferences' , 'Scihub/CODA config' and fill in the User Id",
                                      QMessageBox::Ok);
        if (reply == QMessageBox::Ok)
            okuser = true;
    }
    else
        okuser = true;

    if(!opts.datahubuser.isEmpty())
    {
        if(opts.datahubpassword.isEmpty())
        {
            bool ok;
            QString text = QInputDialog::getText(this, tr("Your Datahub password "),
                                                 tr("Password : "), QLineEdit::Normal, "",&ok);
            if (ok && !text.isEmpty())
            {
                opts.datahubpassword = text;
                okpassword = true;
                qDebug() << "Password = " << text;
            }
            else
                okpassword = false;

        }
        else
            okpassword = true;
    }

    return okuser && okpassword;

}
