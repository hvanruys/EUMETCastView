// ( ͡° ͜ʖ ͡°)﻿

#include "formmapcyl.h"
#include "ui_formmapcyl.h"

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
    ui->btnOLCIefr->setCheckable(true);
    ui->btnOLCIerr->setCheckable(true);
    ui->btnSLSTR->setCheckable(true);
    ui->btnOLCIefrDatahub->setCheckable(true);
    ui->btnOLCIerrDatahub->setCheckable(true);
    ui->btnSLSTRDatahub->setCheckable(true);

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

    if (opts.buttonMetop)
    {
        SetAllButtonsToFalse();
        opts.buttonMetop = true;
    }
    else if (opts.buttonNoaa)
    {
        SetAllButtonsToFalse();
        opts.buttonNoaa = true;
    }
    else if (opts.buttonGAC)
    {
        SetAllButtonsToFalse();
        opts.buttonGAC = true;
    }
    else if (opts.buttonHRP)
    {
        SetAllButtonsToFalse();
        opts.buttonHRP = true;
    }
    else if (opts.buttonRealTime)
    {
        SetAllButtonsToFalse();
        opts.buttonRealTime = true;
    }
    else if (opts.buttonVIIRSM)
    {
        SetAllButtonsToFalse();
        opts.buttonVIIRSM = true;
        formtoolbox->setTabWidgetIndex(TAB_VIIRS);
        formtoolbox->setTabWidgetVIIRSIndex(0);
    }
    else if (opts.buttonVIIRSDNB)
    {
        SetAllButtonsToFalse();
        opts.buttonVIIRSDNB = true;
        formtoolbox->setTabWidgetIndex(TAB_VIIRS);
        formtoolbox->setTabWidgetVIIRSIndex(1);
    }
    else if (opts.buttonOLCIefr)
    {
        SetAllButtonsToFalse();
        opts.buttonOLCIefr = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(0);
    }
    else if (opts.buttonOLCIerr)
    {
        SetAllButtonsToFalse();
        opts.buttonOLCIerr = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(0);
    }
    else if (opts.buttonSLSTR)
    {
        SetAllButtonsToFalse();
        opts.buttonSLSTR = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(1);
    }
    else if (opts.buttonMetopAhrpt)
    {
        SetAllButtonsToFalse();
        opts.buttonMetopAhrpt = true;
        formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    }
    else if (opts.buttonMetopBhrpt)
    {
        SetAllButtonsToFalse();
        opts.buttonMetopBhrpt = true;
        formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    }
    else if (opts.buttonNoaa19hrpt)
    {
        SetAllButtonsToFalse();
        opts.buttonNoaa19hrpt = true;
        formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    }
    else if (opts.buttonM01hrpt)
    {
        SetAllButtonsToFalse();
        opts.buttonM01hrpt = true;
        formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    }
    else if (opts.buttonM02hrpt)
    {
        SetAllButtonsToFalse();
        opts.buttonM02hrpt = true;
        formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    }
    else if (opts.buttonDatahubOLCIefr)
    {
        SetAllButtonsToFalse();
        opts.buttonDatahubOLCIefr = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(0);
    }
    else if (opts.buttonDatahubOLCIerr)
    {
        SetAllButtonsToFalse();
        opts.buttonDatahubOLCIerr = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(0);
    }
    else if (opts.buttonDatahubSLSTR)
    {
        SetAllButtonsToFalse();
        opts.buttonDatahubSLSTR = true;
        formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
        formtoolbox->setTabWidgetSentinelIndex(1);
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
    ui->btnOLCIefr->setChecked(opts.buttonOLCIefr);
    ui->btnOLCIerr->setChecked(opts.buttonOLCIerr);
    ui->btnSLSTR->setChecked(opts.buttonSLSTR);
    ui->btnOLCIefrDatahub->setChecked(opts.buttonDatahubOLCIefr);
    ui->btnOLCIerrDatahub->setChecked(opts.buttonDatahubOLCIerr);
    ui->btnSLSTRDatahub->setChecked(opts.buttonDatahubSLSTR);
    ui->btnRealTime->setChecked(opts.buttonRealTime);
    ui->btnPhong->setChecked(opts.buttonPhong);
    ui->btnAllSegments->setChecked(opts.buttonShowAllSegments);

    connect( ui->btnMetop, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ) );
    connect( ui->btnNoaa, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnGAC, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnHRP, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnMetopAhrpt, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnMetopBhrpt, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnNoaa19hrpt, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnM01hrpt, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnM02hrpt, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));

    //connect( ui->btnVIIRSM, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    //connect( ui->btnVIIRSDNB, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));

    //connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(showSegmentList(int)));
    connect(mapcyl, SIGNAL(wheelChange(int)), this, SLOT(changeScrollBar(int)));
    connect(mapcyl, SIGNAL(mapClicked()), this, SLOT(showSegmentCount()));
    connect(globe, SIGNAL(mapClicked()), this, SLOT(showSegmentCount()));

    segs->setShowAllSegments(ui->btnAllSegments->isChecked());

    this->showSegmentCount();

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
    ui->btnOLCIefr->setEnabled(stat);
    ui->btnOLCIerr->setEnabled(stat);
    ui->btnSLSTR->setEnabled(stat);

    ui->btnMetopAhrpt->setEnabled(stat);
    ui->btnMetopBhrpt->setEnabled(stat);
    ui->btnNoaa19hrpt->setEnabled(stat);
    ui->btnM01hrpt->setEnabled(stat);
    ui->btnM02hrpt->setEnabled(stat);

    ui->btnDownloadFromDatahub->setEnabled(stat);
    ui->btnOLCIefrDatahub->setEnabled(stat);
    ui->btnOLCIerrDatahub->setEnabled(stat);
    ui->btnSLSTRDatahub->setEnabled(stat);
    ui->btnDownloadQuicklook->setEnabled(stat);
    ui->btnDownloadProduct->setEnabled(stat);
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
    int cntselolciefr = segs->seglolciefr->NbrOfSegmentsSelected();
    int cntselolcierr = segs->seglolcierr->NbrOfSegmentsSelected();
    int cntselslstr = segs->seglslstr->NbrOfSegmentsSelected();
    int cntseldatahubolciefr = segs->segldatahubolciefr->NbrOfSegmentsSelected();
    int cntseldatahubolcierr = segs->segldatahubolcierr->NbrOfSegmentsSelected();
    int cntseldatahubslstr = segs->segldatahubslstr->NbrOfSegmentsSelected();


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
    int cntolciefr = segs->seglolciefr->NbrOfSegments();
    int cntolcierr = segs->seglolcierr->NbrOfSegments();
    int cntslstr = segs->seglslstr->NbrOfSegments();
    int cntdatahubolciefr = segs->segldatahubolciefr->NbrOfSegments();
    int cntdatahubolcierr = segs->segldatahubolcierr->NbrOfSegments();
    int cntdatahubslstr = segs->segldatahubslstr->NbrOfSegments();

    int cntmetopAhrpt = segs->seglmetopAhrpt->NbrOfSegments();
    int cntmetopBhrpt = segs->seglmetopBhrpt->NbrOfSegments();
    int cntnoaa19hrpt = segs->seglnoaa19hrpt->NbrOfSegments();
    int cntM02hrpt = segs->seglM02hrpt->NbrOfSegments();
    int cntM01hrpt = segs->seglM01hrpt->NbrOfSegments();

    long totseg = cntmetop + cntnoaa + cnthrp + cntgac + cntviirsm + cntviirsdnb + cntolciefr + cntolcierr + cntslstr +
            cntmetopAhrpt + cntmetopBhrpt + cntnoaa19hrpt + cntM01hrpt + cntM02hrpt + cntdatahubolciefr + cntdatahubolcierr + cntdatahubslstr;
    long totsegsel = cntselmetop + cntselnoaa + cntselhrp + cntselgac + cntselviirsm + cntselviirsdnb + cntselolciefr + cntselolcierr + cntselslstr +
            cntselmetopAhrpt + cntselmetopBhrpt + cntselnoaa19hrpt + cntselM01hrpt + cntselM02hrpt + cntseldatahubolciefr + cntseldatahubolcierr + cntseldatahubslstr;

    if ( totsegsel  > 0)
    {
        ui->btnRemoveSelected->setText( QString(" Remove %1 selected segments ").arg(totsegsel));
    }
    else
    {
        ui->btnRemoveSelected->setText(" No selected segments ");
    }

    ui->btnMetop->setText((QString(" Metop # %1/%2 ").arg(cntselmetop).arg(cntmetop)));
    ui->btnNoaa->setText((QString(" Noaa # %1/%2 ").arg(cntselnoaa).arg(cntnoaa)));
    ui->btnGAC->setText((QString(" GAC # %1/%2 ").arg(cntselgac).arg(cntgac)));
    ui->btnHRP->setText((QString(" HRP # %1/%2 ").arg(cntselhrp).arg(cnthrp)));

    ui->btnVIIRSM->setText((QString(" VIIRS M # %1/%2 ").arg(cntselviirsm).arg(cntviirsm)));
    ui->btnVIIRSDNB->setText((QString(" VIIRS DNB # %1/%2 ").arg(cntselviirsdnb).arg(cntviirsdnb)));

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
        }

     }
}

void FormMapCyl::toggleButton(eSegmentType segtype)
{

    opts.buttonMetop = segtype == eSegmentType::SEG_METOP ? true : false;
    opts.buttonNoaa = segtype == eSegmentType::SEG_NOAA ? true : false;
    opts.buttonGAC = segtype == eSegmentType::SEG_GAC ? true : false;
    opts.buttonHRP = segtype == eSegmentType::SEG_HRP ? true : false;
    opts.buttonVIIRSM = segtype == eSegmentType::SEG_VIIRSM ? true : false;
    opts.buttonVIIRSDNB = segtype == eSegmentType::SEG_VIIRSDNB ? true : false;
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
    ui->btnOLCIefr->setChecked(opts.buttonOLCIefr);
    ui->btnOLCIerr->setChecked(opts.buttonOLCIerr);
    ui->btnSLSTR->setChecked(opts.buttonSLSTR);
    ui->btnOLCIefrDatahub->setChecked(opts.buttonDatahubOLCIefr);
    ui->btnOLCIerrDatahub->setChecked(opts.buttonDatahubOLCIerr);
    ui->btnSLSTRDatahub->setChecked(opts.buttonDatahubSLSTR);
    ui->btnRealTime->setChecked(opts.buttonRealTime);
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

        outp = QString("VIIRSM From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonVIIRSDNB)
    {
        segs->seglviirsdnb->ShowSegment(value);
        segs->seglviirsdnb->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglviirsdnb->NbrOfSegments();

        outp = QString("VIIRSDNB From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
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
    segs->RemoveAllSelectedOLCIefr();
    segs->RemoveAllSelectedOLCIerr();
    segs->RemoveAllSelectedSLSTR();
    segs->RemoveAllSelectedDatahubOLCIefr();
    segs->RemoveAllSelectedDatahubOLCIerr();
    segs->RemoveAllSelectedDatahubSLSTR();

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

void FormMapCyl::on_btnMakeImage_clicked()
{
    if(!formtoolbox->comboColVIIRSOK())
    {
        QMessageBox msgBox;
        msgBox.setText("Need color choices for 3 different bands in the VIIRS tab.");
        //msgBox.setInformativeText("Do you want to save your changes?");
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
    toggleButton(eSegmentType::SEG_NOAA);
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
    qDebug() << "FormMapCyl::on_btnOLCIerrDatahub_clicked()";

    formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
    formtoolbox->setTabWidgetSentinelIndex(0);
    toggleButton(eSegmentType::SEG_DATAHUB_OLCIERR);
    //this->RemoveAllSelected();
    mapcyl->update();
    this->showSegmentCount();
    this->setScrollBarMaximum();
    qDebug() << "einde FormMapCyl::on_btnOLCIerrDatahub_clicked()";

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

void FormMapCyl::on_btnAllSegments_clicked()
{
    segs->setShowAllSegments(ui->btnAllSegments->isChecked());
    opts.buttonShowAllSegments = ui->btnAllSegments->isChecked();
}

void FormMapCyl::on_btnPhong_clicked()
{
    opts.buttonPhong = ui->btnPhong->isChecked();
}

void FormMapCyl::on_btnDownloadProduct_clicked()
{

    if(todownloadlist.count() ==  0)
        return;

    SearchForFreeManager(false);

}

void FormMapCyl::on_btnDownloadQuicklook_clicked()
{

    if(todownloadlist.count() ==  0)
        return;

    SearchForFreeManager(true);

}

void FormMapCyl::SearchForFreeManager(bool quicklook)
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
                hubmanagerprod1.DownloadProduct(todownloadlist, i, hub, 0, quicklook);
                showSelectedSegmentToDownloadList();
            }
            else if(!hubmanagerprod2.isProductDownloadBusy())
            {
                qDebug() << "hubmanagerprod2.DownloadProduct";

                QObject::connect(&hubmanagerprod2, &DatahubAccessManager::productFinished, this, &FormMapCyl::productFileDownloaded);
                QObject::connect(&hubmanagerprod2, &DatahubAccessManager::productProgress, this, &FormMapCyl::productDownloadProgress);
                todownloadlist[i].status = "busy";
                hubmanagerprod2.DownloadProduct(todownloadlist, i, hub, 1, quicklook );
                showSelectedSegmentToDownloadList();
            }
        }
    }

    for(int i = 0; i < todownloadlist.count() ; i++)
    {
        if(todownloadlist.at(i).status == "busy")
        {
            ui->btnDownloadProduct->setEnabled(false);
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

void FormMapCyl::productFileDownloaded(int whichdownload, int downloadindex, bool quicklook)
{
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
        todownloadlist[downloadindex].status = "finished";

    showSelectedSegmentToDownloadList();

    qDebug() << "hubmanagerprod1.isProductDownloadBusy() = " << hubmanagerprod1.isProductDownloadBusy();
    qDebug() << "hubmanagerprod2.isProductDownloadBusy() = " << hubmanagerprod2.isProductDownloadBusy();

    if((!hubmanagerprod1.isProductDownloadBusy()) && (!hubmanagerprod2.isProductDownloadBusy()))
    {
        ui->btnDownloadProduct->setEnabled(true);
        ui->btnDownloadQuicklook->setEnabled(true);
        ui->btnCancelDownloadProduct->setEnabled(true);
    }

    SearchForFreeManager(quicklook);

}

void FormMapCyl::createSelectedSegmentToDownloadList()
{
    QList<Segment*> *sldatahubolciefr = segs->segldatahubolciefr->GetSegmentlistptr();
    QList<Segment*> *sldatahubolcierr = segs->segldatahubolcierr->GetSegmentlistptr();
    QList<Segment*> *sldatahubslstr = segs->segldatahubslstr->GetSegmentlistptr();

    todownloadlist.clear();

    //if (opts.buttonDatahubOLCIefr)
    {
        QList<Segment*>::iterator segitolciefr = sldatahubolciefr->begin();
        while ( segitolciefr != sldatahubolciefr->end() )
        {
            if((*segitolciefr)->IsSelected())
            {
                ProductList prodlist;
                prodlist.productname = (*segitolciefr)->fileInfo.fileName();
                prodlist.uuid = ((SegmentDatahub *)(*segitolciefr))->getUUID();
                prodlist.size = ((SegmentDatahub *)(*segitolciefr))->getSize();
                prodlist.status = "waiting";
                todownloadlist.append(prodlist);
            }
            ++segitolciefr;
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
                prodlist.productname = (*segitolcierr)->fileInfo.fileName();
                prodlist.uuid = ((SegmentDatahub *)(*segitolcierr))->getUUID();
                prodlist.size = ((SegmentDatahub *)(*segitolcierr))->getSize();
                prodlist.status = "waiting";
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
                prodlist.productname = (*segitslstr)->fileInfo.fileName();
                prodlist.uuid = ((SegmentDatahub *)(*segitslstr))->getUUID();
                prodlist.size = ((SegmentDatahub *)(*segitslstr))->getSize();
                prodlist.status = "waiting";
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

    long totalsize = 0;
    ui->twSelectedProducts->clearContents();
    ui->twSelectedProducts->setRowCount(0);

    //S3A_OL_1_EFR____20161026T121318_20161026T121318_20161026T163853_0000_010_166______MAR_O_NR_002.SEN3.tar
    //0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012

    for( int i = 0; i < todownloadlist.count(); ++i )
    {
        ui->twSelectedProducts->insertRow(i);
        QTableWidgetItem *item0 = new QTableWidgetItem(todownloadlist.at(i).status);
        ui->twSelectedProducts->setItem(i, 0, item0);
        QTableWidgetItem *item1 = new QTableWidgetItem("");
        if(todownloadlist.at(i).productname.mid(0, 12) == "S3A_OL_1_EFR")
        {
            item1->setText("OLCI EFR");
        }
        else if(todownloadlist.at(i).productname.mid(0, 12) == "S3A_OL_1_ERR")
        {
            item1->setText("OLCI ERR");
        }
        else if(todownloadlist.at(i).productname.mid(0, 12) == "S3A_SL_1_RBT")
        {
            item1->setText("SLSTR");
        }

        ui->twSelectedProducts->setItem(i, 1, item1);

        QString year = todownloadlist.at(i).productname.mid(16, 4);
        QString month = todownloadlist.at(i).productname.mid(20, 2);
        QString day = todownloadlist.at(i).productname.mid(22, 2);
        QTableWidgetItem *item2 = new QTableWidgetItem(year + "-" + month + "-" + day);
        ui->twSelectedProducts->setItem(i, 2, item2);

        QString hour = todownloadlist.at(i).productname.mid(25, 2);
        QString min = todownloadlist.at(i).productname.mid(27, 2);
        QString sec = todownloadlist.at(i).productname.mid(29, 2);
        QTableWidgetItem *item3 = new QTableWidgetItem(hour + ":" + min + ":" + sec);
        ui->twSelectedProducts->setItem(i, 3, item3);

        hour = todownloadlist.at(i).productname.mid(41, 2);
        min = todownloadlist.at(i).productname.mid(43, 2);
        sec = todownloadlist.at(i).productname.mid(45, 2);
        QTableWidgetItem *item4 = new QTableWidgetItem(hour + ":" + min + ":" + sec);
        ui->twSelectedProducts->setItem(i, 4, item4);

        long lsize = todownloadlist.at(i).size.toLong(&ok);
        totalsize += lsize;
        if(ok)
        {
            float fsize = (float)lsize/1000000;
            QString strsize = QString("%1 Mb").arg(fsize, 0, 'f', 2);
            QTableWidgetItem *item5 = new QTableWidgetItem(strsize);
            ui->twSelectedProducts->setItem(i, 5, item5);
        }

        ui->twSelectedProducts->setColumnWidth(0, 60); // status
        ui->twSelectedProducts->setColumnWidth(1, 80); // type
        ui->twSelectedProducts->setColumnWidth(2, 80); // date
        ui->twSelectedProducts->setColumnWidth(3, 70); // start
        ui->twSelectedProducts->setColumnWidth(4, 70); // end
        ui->twSelectedProducts->setColumnWidth(5, 80); // size
    }

    float ftotalsize = (float)totalsize/1000000;
    ui->lblTotalDownloadSize->setText(QString("Total size to download = %1 Mb").arg(ftotalsize, 0, 'f', 2));

    ui->twSelectedProducts->resizeRowsToContents();
    ui->twSelectedProducts->show();
}

void FormMapCyl::slotShowXMLProgress(QString str, int pages, bool downloadinprogress)
{
    ui->pbXMLprogress->setMaximum(10);

    ui->lblTotalAvailable->setText(str);
    if(downloadinprogress)
    {
        ui->btnDownloadProduct->setEnabled(false);
        ui->btnCancelDownloadProduct->setEnabled(false);
    }
    else
    {
        ui->btnDownloadProduct->setEnabled(true);
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
}


void FormMapCyl::on_btnDownloadFromDatahub_clicked()
{
    segs->LoadXMLfromDatahub();
}
