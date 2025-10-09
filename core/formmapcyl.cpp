// ( ͡° ͜ʖ ͡°)﻿

#include "formmapcyl.h"
#include "ui_formmapcyl.h"
#include "dialogpreferences.h"
#include "satellite.h"

#include <netcdf.h>

extern Options opts;
extern SegmentImage *imageptrs;
extern SatelliteList satellitelist;

FormMapCyl::FormMapCyl(QWidget *parent, MapFieldCyl *p_mapcyl, Globe *p_globe, FormToolbox *p_formtoolbox, AVHRRSatellite *seglist ) :
    QWidget(parent),
    ui(new Ui::FormMapCyl)
{
    ui->setupUi(this);

    qDebug() << "constructor formmapcyl";

    segs = seglist;
    mapcyl = p_mapcyl;
    globe = p_globe;
    formtoolbox = p_formtoolbox;

    ui->stackedWidget->addWidget(mapcyl);
    if(opts.doOpenGL)
        ui->stackedWidget->addWidget(globe);

    ui->stackedWidget->setCurrentIndex(0);
    ui->tabWidget->setCurrentIndex(0);

    ui->btnMetop->setCheckable(true);
    ui->btnHRP->setCheckable(true);

    ui->btnVIIRSM->setCheckable(true);
    ui->btnVIIRSDNB->setCheckable(true);
    ui->btnVIIRSMNOAA20->setCheckable(true);
    ui->btnVIIRSDNBNOAA20->setCheckable(true);
    ui->btnVIIRSMNOAA21->setCheckable(true);
    ui->btnVIIRSDNBNOAA21->setCheckable(true);
    ui->btnOLCIefr->setCheckable(true);
    ui->btnOLCIerr->setCheckable(true);
    ui->btnMERSI->setCheckable(true);

    ui->btnRealTime->setCheckable(true);
    ui->btnPhong->setCheckable(true);
    ui->btnAllSegments->setCheckable(true);

    QStringList hlst;
    hlst << "Status" << "Type" << "Date" << "Start" << "End" << "Size";

    SetAllButtonsToFalse();
    if (opts.buttonMetop)
    {
        opts.buttonMetop = true;
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
    else if (opts.buttonVIIRSMNOAA21)
    {
        opts.buttonVIIRSMNOAA21 = true;
        formtoolbox->setTabWidgetIndex(TAB_VIIRS);
        formtoolbox->setTabWidgetVIIRSIndex(0);
    }
    else if (opts.buttonVIIRSDNBNOAA21)
    {
        opts.buttonVIIRSDNBNOAA21 = true;
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
    else if (opts.buttonMERSI)
    {
        opts.buttonMERSI = true;
    }


    ui->btnMetop->setChecked(opts.buttonMetop);
    ui->btnHRP->setChecked(opts.buttonHRP);

    ui->btnVIIRSM->setChecked(opts.buttonVIIRSM);
    ui->btnVIIRSDNB->setChecked(opts.buttonVIIRSDNB);
    ui->btnVIIRSMNOAA20->setChecked(opts.buttonVIIRSMNOAA20);
    ui->btnVIIRSDNBNOAA20->setChecked(opts.buttonVIIRSDNBNOAA20);
    ui->btnVIIRSMNOAA21->setChecked(opts.buttonVIIRSMNOAA21);
    ui->btnVIIRSDNBNOAA21->setChecked(opts.buttonVIIRSDNBNOAA21);
    ui->btnOLCIefr->setChecked(opts.buttonOLCIefr);
    ui->btnOLCIerr->setChecked(opts.buttonOLCIerr);
    ui->btnMERSI->setChecked(opts.buttonMERSI);
    ui->btnRealTime->setChecked(opts.buttonRealTime);
    ui->btnPhong->setChecked(opts.buttonPhong);
    ui->btnAllSegments->setChecked(opts.buttonShowAllSegments);

    ui->btnMakeImage->setEnabled(true);

    connect( ui->btnMetop, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ) );
    connect( ui->btnHRP, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnMERSI, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ) );

    //connect( ui->btnVIIRSM, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    //connect( ui->btnVIIRSDNB, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));

    //connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(showSegmentList(int)));
    connect(mapcyl, SIGNAL(wheelChange(int)), this, SLOT(changeScrollBar(int)));
    connect(mapcyl, SIGNAL(mapClicked()), this, SLOT(showSegmentCount()));

    if(opts.doOpenGL)
        connect(globe, SIGNAL(mapClicked()), this, SLOT(showSegmentCount()));

    segs->setShowAllSegments(ui->btnAllSegments->isChecked());

    this->showSegmentCount();

    opts.globalChangeFonts(this, opts.fontsize);

}


void FormMapCyl::SetAllButtonsToFalse()
{
    opts.buttonMetop = false;
    opts.buttonHRP = false;

    opts.buttonVIIRSM = false;
    opts.buttonVIIRSDNB = false;
    opts.buttonVIIRSMNOAA20 = false;
    opts.buttonVIIRSDNBNOAA20 = false;
    opts.buttonVIIRSMNOAA21 = false;
    opts.buttonVIIRSDNBNOAA21 = false;
    opts.buttonOLCIefr = false;
    opts.buttonOLCIerr = false;
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
    ui->btnVIIRSM->setEnabled(stat);
    ui->btnVIIRSDNB->setEnabled(stat);
    ui->btnVIIRSMNOAA20->setEnabled(stat);
    ui->btnVIIRSDNBNOAA20->setEnabled(stat);
    ui->btnVIIRSMNOAA21->setEnabled(stat);
    ui->btnVIIRSDNBNOAA21->setEnabled(stat);
    ui->btnOLCIefr->setEnabled(stat);
    ui->btnOLCIerr->setEnabled(stat);
    ui->btnMERSI->setEnabled(stat);

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
    int cntselmetopsga1 = segs->seglmetopsga1->NbrOfSegmentsSelected();
    int cntselhrp = segs->seglhrp->NbrOfSegmentsSelected();
    int cntselviirsm = segs->seglviirsm->NbrOfSegmentsSelected();
    int cntselviirsdnb = segs->seglviirsdnb->NbrOfSegmentsSelected();
    int cntselviirsmnoaa20 = segs->seglviirsmnoaa20->NbrOfSegmentsSelected();
    int cntselviirsdnbnoaa20 = segs->seglviirsdnbnoaa20->NbrOfSegmentsSelected();
    int cntselviirsmnoaa21 = segs->seglviirsmnoaa21->NbrOfSegmentsSelected();
    int cntselviirsdnbnoaa21 = segs->seglviirsdnbnoaa21->NbrOfSegmentsSelected();
    int cntselolciefr = segs->seglolciefr->NbrOfSegmentsSelected();
    int cntselolcierr = segs->seglolcierr->NbrOfSegmentsSelected();
    int cntselmersi = segs->seglmersi->NbrOfSegmentsSelected();


    int cntmetop = segs->seglmetop->NbrOfSegments();
    int cntmetopsga1 = segs->seglmetopsga1->NbrOfSegments();
    int cnthrp = segs->seglhrp->NbrOfSegments();
    int cntviirsm = segs->seglviirsm->NbrOfSegments();
    int cntviirsdnb = segs->seglviirsdnb->NbrOfSegments();
    int cntviirsmnoaa20 = segs->seglviirsmnoaa20->NbrOfSegments();
    int cntviirsdnbnoaa20 = segs->seglviirsdnbnoaa20->NbrOfSegments();
    int cntviirsmnoaa21 = segs->seglviirsmnoaa21->NbrOfSegments();
    int cntviirsdnbnoaa21 = segs->seglviirsdnbnoaa21->NbrOfSegments();
    int cntolciefr = segs->seglolciefr->NbrOfSegments();
    int cntolcierr = segs->seglolcierr->NbrOfSegments();
    int cntmersi = segs->seglmersi->NbrOfSegments();

    long totseg = cntmetop + cntmetopsga1 + cnthrp + cntviirsm + cntviirsdnb + cntviirsmnoaa20 + cntviirsdnbnoaa20 + cntviirsmnoaa21 + cntviirsdnbnoaa21
                  + cntolciefr + cntolcierr + cntmersi;
    long totsegsel = cntselmetop + cntselmetopsga1 + cntselhrp + cntselviirsm + cntselviirsdnb  + cntselviirsmnoaa20 + cntselviirsdnbnoaa20 + cntselviirsmnoaa21 + cntselviirsdnbnoaa21
                     + cntselolciefr + cntselolcierr + cntselmersi;

    if ( totsegsel  > 0)
    {
        ui->btnRemoveSelected->setText( QString(" Remove %1 selected segments ").arg(totsegsel));
    }
    else
    {
        ui->btnRemoveSelected->setText(" No selected segments ");
    }

    ui->btnMetop->setText((QString(" Metop A/B/C # %1/%2 ").arg(cntselmetop).arg(cntmetop)));
    ui->btnHRP->setText((QString(" Metop A/B/C HRP # %1/%2 ").arg(cntselhrp).arg(cnthrp)));

    ui->btnMetopSGA1->setText((QString(" Metop SGA1 # %1/%2 ").arg(cntselmetopsga1).arg(cntmetopsga1)));

    ui->btnVIIRSM->setText((QString(" NPP VIIRS M # %1/%2 ").arg(cntselviirsm).arg(cntviirsm)));
    ui->btnVIIRSDNB->setText((QString(" NPP VIIRS DNB # %1/%2 ").arg(cntselviirsdnb).arg(cntviirsdnb)));
    ui->btnVIIRSMNOAA20->setText((QString(" NOAA-20 VIIRS M # %1/%2 ").arg(cntselviirsmnoaa20).arg(cntviirsmnoaa20)));
    ui->btnVIIRSDNBNOAA20->setText((QString(" NOAA-20 VIIRS DNB # %1/%2 ").arg(cntselviirsdnbnoaa20).arg(cntviirsdnbnoaa20)));
    ui->btnVIIRSMNOAA21->setText((QString(" NOAA-21 VIIRS M # %1/%2 ").arg(cntselviirsmnoaa21).arg(cntviirsmnoaa21)));
    ui->btnVIIRSDNBNOAA21->setText((QString(" NOAA-21 VIIRS DNB # %1/%2 ").arg(cntselviirsdnbnoaa21).arg(cntviirsdnbnoaa21)));

    ui->btnOLCIefr->setText((QString(" OLCI EFR # %1/%2 ").arg(cntselolciefr).arg(cntolciefr)));
    ui->btnOLCIerr->setText((QString(" OLCI ERR # %1/%2 ").arg(cntselolcierr).arg(cntolcierr)));

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
            if (opts.buttonHRP)
            {
                segs->seglhrp->ShowSegment(ui->verticalScrollBar->value());
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
                                if (opts.buttonVIIRSMNOAA21)
                                {
                                    segs->seglviirsmnoaa21->ShowSegment(ui->verticalScrollBar->value());
                                } else
                                    if (opts.buttonVIIRSDNBNOAA21)
                                    {
                                        segs->seglviirsdnbnoaa21->ShowSegment(ui->verticalScrollBar->value());
                                    } else
                                        if (opts.buttonOLCIefr)
                                        {
                                            segs->seglolciefr->ShowSegment(ui->verticalScrollBar->value());
                                        } else
                                            if (opts.buttonOLCIerr)
                                            {
                                                segs->seglolcierr->ShowSegment(ui->verticalScrollBar->value());
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
        } else if (opts.buttonMERSI)
        {
            tit = "MERSI ";
        }


    }
}

void FormMapCyl::toggleButton(eSegmentType segtype)
{

    opts.buttonMetop = segtype == eSegmentType::SEG_METOP ? true : false;
    opts.buttonHRP = segtype == eSegmentType::SEG_HRP ? true : false;
    opts.buttonVIIRSM = segtype == eSegmentType::SEG_VIIRSM ? true : false;
    opts.buttonVIIRSDNB = segtype == eSegmentType::SEG_VIIRSDNB ? true : false;
    opts.buttonVIIRSMNOAA20 = segtype == eSegmentType::SEG_VIIRSMNOAA20 ? true : false;
    opts.buttonVIIRSDNBNOAA20 = segtype == eSegmentType::SEG_VIIRSDNBNOAA20 ? true : false;
    opts.buttonVIIRSMNOAA21 = segtype == eSegmentType::SEG_VIIRSMNOAA21 ? true : false;
    opts.buttonVIIRSDNBNOAA21 = segtype == eSegmentType::SEG_VIIRSDNBNOAA21 ? true : false;
    opts.buttonOLCIefr = segtype == eSegmentType::SEG_OLCIEFR ? true : false;
    opts.buttonOLCIerr = segtype == eSegmentType::SEG_OLCIERR ? true : false;
    opts.buttonRealTime = segtype == eSegmentType::SEG_NONE ? true : false;
    opts.buttonMERSI = segtype == eSegmentType::SEG_MERSI ? true : false;

    ui->btnMetop->setChecked(opts.buttonMetop);
    ui->btnHRP->setChecked(opts.buttonHRP);


    ui->btnVIIRSM->setChecked(opts.buttonVIIRSM);
    ui->btnVIIRSDNB->setChecked(opts.buttonVIIRSDNB);
    ui->btnVIIRSMNOAA20->setChecked(opts.buttonVIIRSMNOAA20);
    ui->btnVIIRSDNBNOAA20->setChecked(opts.buttonVIIRSDNBNOAA20);
    ui->btnVIIRSMNOAA21->setChecked(opts.buttonVIIRSMNOAA21);
    ui->btnVIIRSDNBNOAA21->setChecked(opts.buttonVIIRSDNBNOAA21);
    ui->btnOLCIefr->setChecked(opts.buttonOLCIefr);
    ui->btnOLCIerr->setChecked(opts.buttonOLCIerr);
    ui->btnMERSI->setChecked(opts.buttonMERSI);
    ui->btnRealTime->setChecked(opts.buttonRealTime);

    ui->btnMakeImage->setEnabled(true);


    this->showSegmentList(0);

    //imagetab->SetGammaSpinboxes();
    //this->RemoveAllSelected();
    this->setScrollBarMaximum();


    return;
}


void FormMapCyl::setScrollBarMaximum()
{

    ui->verticalScrollBar->setValue(0);

    if (opts.buttonMetop)
    {
        ui->verticalScrollBar->setMaximum(segs->seglmetop->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum metop = %1").arg(segs->seglmetop->NbrOfSegments());
    }
    else if (opts.buttonHRP)
    {
        ui->verticalScrollBar->setMaximum(segs->seglhrp->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum HRP = %1").arg(segs->seglhrp->NbrOfSegments());
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
    else if (opts.buttonVIIRSMNOAA21)
    {
        ui->verticalScrollBar->setMaximum(segs->seglviirsmnoaa21->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum VIIRSM NOAA-21 = %1").arg(segs->seglviirsmnoaa21->NbrOfSegments());
    }
    else if (opts.buttonVIIRSDNBNOAA21)
    {
        ui->verticalScrollBar->setMaximum(segs->seglviirsdnbnoaa21->NbrOfSegments());
        qDebug() << QString("setscrollbarmaximum VIIRSDNB NOAA-21 = %1").arg(segs->seglviirsdnbnoaa21->NbrOfSegments());
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
    else if(opts.buttonHRP)
    {
        segs->seglhrp->ShowSegment(value);
        segs->seglhrp->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglhrp->NbrOfSegments();

        outp = QString("HRP From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
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
    else if(opts.buttonVIIRSMNOAA21)
    {
        segs->seglviirsmnoaa21->ShowSegment(value);
        segs->seglviirsmnoaa21->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglviirsmnoaa21->NbrOfSegments();

        outp = QString("NOAA-21 VIIRSM From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonVIIRSDNBNOAA21)
    {
        segs->seglviirsdnbnoaa21->ShowSegment(value);
        segs->seglviirsdnbnoaa21->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglviirsdnbnoaa21->NbrOfSegments();

        outp = QString("NOAA-21 VIIRSDNB From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
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
    segs->RemoveAllSelectedVIIRSMNOAA21();
    segs->RemoveAllSelectedVIIRSDNBNOAA21();
    segs->RemoveAllSelectedOLCIefr();
    segs->RemoveAllSelectedOLCIerr();
    segs->RemoveAllSelectedMERSI();

    imageptrs->ptrProjectionBrightnessTemp.reset();

    mapcyl->update();
    showSegmentCount();

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
    if(opts.buttonHRP && segs->seglhrp->NbrOfSegmentsSelected() > 0)
        return true;

    if(opts.buttonVIIRSM && segs->seglviirsm->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonVIIRSDNB && segs->seglviirsdnb->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonVIIRSMNOAA20 && segs->seglviirsmnoaa20->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonVIIRSDNBNOAA20 && segs->seglviirsdnbnoaa20->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonVIIRSMNOAA21 && segs->seglviirsmnoaa21->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonVIIRSDNBNOAA21 && segs->seglviirsdnbnoaa21->NbrOfSegmentsSelected() > 0)
        return true;


    if(opts.buttonOLCIefr && segs->seglolciefr->NbrOfSegmentsSelected() > 0)
        return true;
    if(opts.buttonOLCIerr && segs->seglolcierr->NbrOfSegmentsSelected() > 0)
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

void FormMapCyl::on_btnMetop_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_AVHRR);
    toggleButton(eSegmentType::SEG_METOP);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}

void FormMapCyl::on_btnMetopSGA1_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_METIMAGE);
    toggleButton(eSegmentType::SEG_METOPSGA1);
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

    this->RemoveAllSelected();
    this->setScrollBarMaximum();


    // segs->RemoveAllSelectedAVHRR();
    // segs->RemoveAllSelectedOLCIefr();
    // segs->RemoveAllSelectedOLCIerr();
    // segs->RemoveAllSelectedSLSTR();
    // segs->RemoveAllSelectedDatahubOLCIefr();
    // segs->RemoveAllSelectedDatahubOLCIerr();
    // segs->RemoveAllSelectedDatahubSLSTR();
    // segs->RemoveAllSelectedVIIRSDNB();
    // segs->RemoveAllSelectedVIIRSMNOAA20();
    // segs->RemoveAllSelectedVIIRSDNBNOAA20();
    // segs->RemoveAllSelectedVIIRSMNOAA21();
    // segs->RemoveAllSelectedVIIRSDNBNOAA21();


    // mapcyl->update();
    // this->showSegmentCount();
    // this->setScrollBarMaximum();
    // return;
}

void FormMapCyl::on_btnVIIRSDNB_clicked() // DNB Bands
{
    formtoolbox->setTabWidgetIndex(TAB_VIIRS);
    formtoolbox->setTabWidgetVIIRSIndex(1);
    toggleButton(eSegmentType::SEG_VIIRSDNB);

    this->RemoveAllSelected();
    this->setScrollBarMaximum();

    // segs->RemoveAllSelectedAVHRR();
    // segs->RemoveAllSelectedOLCIefr();
    // segs->RemoveAllSelectedOLCIerr();
    // segs->RemoveAllSelectedSLSTR();
    // segs->RemoveAllSelectedDatahubOLCIefr();
    // segs->RemoveAllSelectedDatahubOLCIerr();
    // segs->RemoveAllSelectedDatahubSLSTR();
    // segs->RemoveAllSelectedVIIRSM();
    // segs->RemoveAllSelectedVIIRSMNOAA20();
    // segs->RemoveAllSelectedVIIRSDNBNOAA20();
    // segs->RemoveAllSelectedVIIRSMNOAA21();
    // segs->RemoveAllSelectedVIIRSDNBNOAA21();



    // mapcyl->update();
    // this->showSegmentCount();
    // this->setScrollBarMaximum();
    // return;
}

void FormMapCyl::on_btnVIIRSMNOAA20_clicked() // M-Bands
{

    formtoolbox->setTabWidgetIndex(TAB_VIIRS);
    formtoolbox->setTabWidgetVIIRSIndex(0);
    toggleButton(eSegmentType::SEG_VIIRSMNOAA20);

    this->RemoveAllSelected();
    this->setScrollBarMaximum();

    // segs->RemoveAllSelectedAVHRR();
    // segs->RemoveAllSelectedOLCIefr();
    // segs->RemoveAllSelectedOLCIerr();
    // segs->RemoveAllSelectedSLSTR();
    // segs->RemoveAllSelectedDatahubOLCIefr();
    // segs->RemoveAllSelectedDatahubOLCIerr();
    // segs->RemoveAllSelectedDatahubSLSTR();
    // segs->RemoveAllSelectedVIIRSM();
    // segs->RemoveAllSelectedVIIRSDNB();
    // segs->RemoveAllSelectedVIIRSDNBNOAA20();
    // segs->RemoveAllSelectedVIIRSMNOAA21();
    // segs->RemoveAllSelectedVIIRSDNBNOAA21();

    // mapcyl->update();
    // this->showSegmentCount();
    // this->setScrollBarMaximum();
    // return;
}

void FormMapCyl::on_btnVIIRSDNBNOAA20_clicked() // DNB Bands
{
    formtoolbox->setTabWidgetIndex(TAB_VIIRS);
    formtoolbox->setTabWidgetVIIRSIndex(1);
    toggleButton(eSegmentType::SEG_VIIRSDNBNOAA20);

    this->RemoveAllSelected();
    this->setScrollBarMaximum();

    // segs->RemoveAllSelectedAVHRR();
    // segs->RemoveAllSelectedOLCIefr();
    // segs->RemoveAllSelectedOLCIerr();
    // segs->RemoveAllSelectedSLSTR();
    // segs->RemoveAllSelectedDatahubOLCIefr();
    // segs->RemoveAllSelectedDatahubOLCIerr();
    // segs->RemoveAllSelectedDatahubSLSTR();
    // segs->RemoveAllSelectedVIIRSM();
    // segs->RemoveAllSelectedVIIRSDNB();
    // segs->RemoveAllSelectedVIIRSMNOAA20();
    // segs->RemoveAllSelectedVIIRSMNOAA21();
    // segs->RemoveAllSelectedVIIRSDNBNOAA21();

    // mapcyl->update();
    // this->showSegmentCount();
    // this->setScrollBarMaximum();
    // return;
}

void FormMapCyl::on_btnVIIRSMNOAA21_clicked() // M-Bands
{

    formtoolbox->setTabWidgetIndex(TAB_VIIRS);
    formtoolbox->setTabWidgetVIIRSIndex(0);
    toggleButton(eSegmentType::SEG_VIIRSMNOAA21);

    this->RemoveAllSelected();
    this->setScrollBarMaximum();

    // segs->RemoveAllSelectedAVHRR();
    // segs->RemoveAllSelectedOLCIefr();
    // segs->RemoveAllSelectedOLCIerr();
    // segs->RemoveAllSelectedSLSTR();
    // segs->RemoveAllSelectedDatahubOLCIefr();
    // segs->RemoveAllSelectedDatahubOLCIerr();
    // segs->RemoveAllSelectedDatahubSLSTR();
    // segs->RemoveAllSelectedVIIRSM();
    // segs->RemoveAllSelectedVIIRSDNB();
    // segs->RemoveAllSelectedVIIRSMNOAA20();
    // segs->RemoveAllSelectedVIIRSDNBNOAA20();
    // segs->RemoveAllSelectedVIIRSDNBNOAA21();

    // mapcyl->update();
    // this->showSegmentCount();
    // this->setScrollBarMaximum();
    // return;
}

void FormMapCyl::on_btnVIIRSDNBNOAA21_clicked() // DNB Bands
{
    formtoolbox->setTabWidgetIndex(TAB_VIIRS);
    formtoolbox->setTabWidgetVIIRSIndex(1);
    toggleButton(eSegmentType::SEG_VIIRSDNBNOAA21);

    this->RemoveAllSelected();
    this->setScrollBarMaximum();

    // segs->RemoveAllSelectedAVHRR();
    // segs->RemoveAllSelectedOLCIefr();
    // segs->RemoveAllSelectedOLCIerr();
    // segs->RemoveAllSelectedSLSTR();
    // segs->RemoveAllSelectedDatahubOLCIefr();
    // segs->RemoveAllSelectedDatahubOLCIerr();
    // segs->RemoveAllSelectedDatahubSLSTR();
    // segs->RemoveAllSelectedVIIRSM();
    // segs->RemoveAllSelectedVIIRSDNB();
    // segs->RemoveAllSelectedVIIRSMNOAA20();
    // segs->RemoveAllSelectedVIIRSDNBNOAA20();
    // segs->RemoveAllSelectedVIIRSMNOAA21();

    // mapcyl->update();
    // this->showSegmentCount();
    // this->setScrollBarMaximum();
    // return;
}

void FormMapCyl::on_btnOLCIefr_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
    formtoolbox->setTabWidgetSentinelIndex(0);
    toggleButton(eSegmentType::SEG_OLCIEFR);
    this->RemoveAllSelected();
    this->setScrollBarMaximum();

    return;
}

void FormMapCyl::on_btnOLCIerr_clicked()
{
    formtoolbox->setTabWidgetIndex(TAB_SENTINEL);
    formtoolbox->setTabWidgetSentinelIndex(0);
    toggleButton(eSegmentType::SEG_OLCIERR);
    this->RemoveAllSelected();
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
            flon = fflon * PIE/180.0;
            flat = fflat * PIE/180.0;
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
