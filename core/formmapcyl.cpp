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

    ui->btnMetop->setCheckable(true);
    ui->btnNoaa->setCheckable(true);
    ui->btnGAC->setCheckable(true);
    ui->btnHRP->setCheckable(true);
    ui->btnVIIRS->setCheckable(true);
    ui->btnRealTime->setCheckable(true);
    ui->btnPhong->setCheckable(true);

    if (opts.buttonMetop)
    {
        opts.buttonNoaa = false;
        opts.buttonGAC = false;
        opts.buttonHRP = false;
        opts.buttonVIIRS = false;
        opts.buttonRealTime = false;
    }
    else
        if (opts.buttonNoaa)
        {
            opts.buttonMetop = false;
            opts.buttonGAC = false;
            opts.buttonHRP = false;
            opts.buttonVIIRS = false;
            opts.buttonRealTime = false;
        }
        else
            if (opts.buttonGAC)
            {
                opts.buttonNoaa = false;
                opts.buttonMetop = false;
                opts.buttonHRP = false;
                opts.buttonVIIRS = false;
                opts.buttonRealTime = false;
            }
            else
                if (opts.buttonHRP)
                {
                    opts.buttonNoaa = false;
                    opts.buttonGAC = false;
                    opts.buttonMetop = false;
                    opts.buttonVIIRS = false;
                    opts.buttonRealTime = false;
                }
                else
                    if (opts.buttonRealTime)
                    {
                        opts.buttonNoaa = false;
                        opts.buttonGAC = false;
                        opts.buttonHRP = false;
                        opts.buttonMetop = false;
                        opts.buttonVIIRS = false;
                    }
                    else
                        if (opts.buttonVIIRS)
                        {
                            opts.buttonNoaa = false;
                            opts.buttonGAC = false;
                            opts.buttonHRP = false;
                            opts.buttonMetop = false;
                            opts.buttonRealTime = false;
                        }

    ui->btnMetop->setChecked(opts.buttonMetop);
    ui->btnNoaa->setChecked(opts.buttonNoaa);
    ui->btnGAC->setChecked(opts.buttonGAC);
    ui->btnHRP->setChecked(opts.buttonHRP);
    ui->btnVIIRS->setChecked(opts.buttonVIIRS);
    ui->btnRealTime->setChecked(opts.buttonRealTime);
    ui->btnPhong->setChecked(true);


    connect( ui->btnMetop, SIGNAL( clicked() ), mapcyl, SLOT( showMetopSegments() ) );
    connect( ui->btnNoaa, SIGNAL( clicked() ), mapcyl, SLOT( showNoaaSegments() ));
    connect( ui->btnGAC, SIGNAL( clicked() ), mapcyl, SLOT( showGACSegments() ));
    connect( ui->btnHRP, SIGNAL( clicked() ), mapcyl, SLOT( showHRPSegments() ));
    connect( ui->btnVIIRS, SIGNAL( clicked() ), mapcyl, SLOT( showVIIRSSegments() ));

    connect( ui->btnMetop, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ) );
    connect( ui->btnNoaa, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnGAC, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnHRP, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));
    connect( ui->btnVIIRS, SIGNAL( clicked() ), formtoolbox, SLOT( setChannelComboBoxes() ));

    //connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(showSegmentList(int)));
    connect(mapcyl, SIGNAL(wheelChange(int)), this, SLOT(changeScrollBar(int)));
    connect(mapcyl, SIGNAL(mapClicked()), this, SLOT(showSegmentcount()));
    connect(globe, SIGNAL(mapClicked()), this, SLOT(showSegmentcount()));

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

void FormMapCyl::showSegmentcount()
{
    qDebug() << "FormMapCyl::showSegmentcount";

    long totseg = segs->seglmetop->NbrOfSegmentsSelected() + segs->seglnoaa->NbrOfSegmentsSelected()  + segs->seglhrp->NbrOfSegmentsSelected()
            + segs->seglgac->NbrOfSegmentsSelected() + segs->seglviirs->NbrOfSegmentsSelected();
    if ( totseg > 0)
    {
        ui->btnRemoveSelected->setText( QString(" Remove %1 selected segments ").arg(totseg));
    }
    else
    {
        ui->btnRemoveSelected->setText(" No selected segments ");
    }

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
        }
        if (opts.buttonNoaa )
        {
            segs->seglnoaa->ShowSegment(ui->verticalScrollBar->value());
        }
        if (opts.buttonHRP)
        {
            segs->seglhrp->ShowSegment(ui->verticalScrollBar->value());
        }
        if (opts.buttonGAC)
        {
            segs->seglgac->ShowSegment(ui->verticalScrollBar->value());
        }
        if (opts.buttonVIIRS)
        {
            segs->seglviirs->ShowSegment(ui->verticalScrollBar->value());
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
        } else if (opts.buttonVIIRS)
        {
            tit = "HRP ";
        }
/*
        switch(imagetab->GetChannelShown())
        {
        case 1:
            tit += "Channel 1";
            break;
        case 2:
            tit += "Channel 2";
            break;
        case 3:
            tit += "Channel 3";
            break;
        case 4:
            tit += "Channel 4";
            break;
        case 5:
            tit += "Channel 5";
            break;
        case 6:
            tit += "Color image";
            break;
        case 7:
            tit = "Expanded Color image";
            break;
        }

        qDebug() << QString("channelshown = %1").arg(imagetab->GetChannelShown());

        tabWidget->setTabText(4, tit);
*/
     }
}

void FormMapCyl::toggleButtonMetop()
{


    if (opts.buttonMetop == true)
        opts.buttonMetop = false;
    else
    {
        opts.buttonMetop = true;
        opts.buttonNoaa = false;
        opts.buttonGAC = false;
        opts.buttonHRP = false;
        opts.buttonVIIRS = false;
        opts.buttonRealTime = false;
        ui->btnMetop->setChecked(opts.buttonMetop);
        ui->btnNoaa->setChecked(opts.buttonNoaa);
        ui->btnGAC->setChecked(opts.buttonGAC);
        ui->btnHRP->setChecked(opts.buttonHRP);
        ui->btnVIIRS->setChecked(opts.buttonVIIRS);
        ui->btnRealTime->setChecked(opts.buttonRealTime);
        this->showSegmentList(0);
    }

    //imagetab->SetGammaSpinboxes();
    this->RemoveAllSelected();
    this->setScrollBarMaximum();

}

void FormMapCyl::toggleButtonNoaa()
{

    if (opts.buttonNoaa == true)
        opts.buttonNoaa = false;
    else
    {
        opts.buttonMetop = false;
        opts.buttonNoaa = true;
        opts.buttonGAC = false;
        opts.buttonHRP = false;
        opts.buttonVIIRS = false;
        opts.buttonRealTime = false;
        ui->btnMetop->setChecked(opts.buttonMetop);
        ui->btnNoaa->setChecked(opts.buttonNoaa);
        ui->btnGAC->setChecked(opts.buttonGAC);
        ui->btnHRP->setChecked(opts.buttonHRP);
        ui->btnVIIRS->setChecked(opts.buttonVIIRS);
        ui->btnRealTime->setChecked(opts.buttonRealTime);

        this->showSegmentList(0);
    }

    //imagetab->SetGammaSpinboxes();
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}


void FormMapCyl::toggleButtonGAC()
{

    if (opts.buttonGAC == true)
        opts.buttonGAC = false;
    else
    {
        opts.buttonMetop = false;
        opts.buttonNoaa = false;
        opts.buttonGAC = true;
        opts.buttonHRP = false;
        opts.buttonRealTime = false;
        opts.buttonVIIRS = false;
        ui->btnMetop->setChecked(opts.buttonMetop);
        ui->btnNoaa->setChecked(opts.buttonNoaa);
        ui->btnGAC->setChecked(opts.buttonGAC);
        ui->btnHRP->setChecked(opts.buttonHRP);
        ui->btnVIIRS->setChecked(opts.buttonVIIRS);
        ui->btnRealTime->setChecked(opts.buttonRealTime);

        this->showSegmentList(0);
    }

    //imagetab->SetGammaSpinboxes();
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}

void FormMapCyl::toggleButtonHRP()
{

    if (opts.buttonHRP == true)
        opts.buttonHRP = false;
    else
    {
        opts.buttonMetop = false;
        opts.buttonNoaa = false;
        opts.buttonGAC = false;
        opts.buttonHRP = true;
        opts.buttonVIIRS = false;
        opts.buttonRealTime = false;

        ui->btnMetop->setChecked(opts.buttonMetop);
        ui->btnNoaa->setChecked(opts.buttonNoaa);
        ui->btnGAC->setChecked(opts.buttonGAC);
        ui->btnHRP->setChecked(opts.buttonHRP);
        ui->btnVIIRS->setChecked(opts.buttonVIIRS);
        ui->btnRealTime->setChecked(opts.buttonRealTime);


        this->showSegmentList(0);
    }
    //imagetab->SetGammaSpinboxes();
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}

void FormMapCyl::toggleButtonVIIRS()
{

    qDebug() << "FormMapCyl::toggleButtonVIIRS()";

    if (opts.buttonVIIRS == true)
        opts.buttonVIIRS = false;
    else
    {
        opts.buttonMetop = false;
        opts.buttonNoaa = false;
        opts.buttonGAC = false;
        opts.buttonHRP = false;
        opts.buttonVIIRS = true;
        opts.buttonRealTime = false;

        ui->btnMetop->setChecked(opts.buttonMetop);
        ui->btnNoaa->setChecked(opts.buttonNoaa);
        ui->btnGAC->setChecked(opts.buttonGAC);
        ui->btnHRP->setChecked(opts.buttonHRP);
        ui->btnVIIRS->setChecked(opts.buttonVIIRS);
        ui->btnRealTime->setChecked(opts.buttonRealTime);


        this->showSegmentList(0);
    }
    //imagetab->SetGammaSpinboxes();
    this->RemoveAllSelected();
    this->setScrollBarMaximum();
}

void FormMapCyl::toggleButtonRealTime()
{
    if (opts.buttonRealTime == true)
        opts.buttonRealTime = false;
    else
    {
        opts.buttonMetop = false;
        opts.buttonNoaa = false;
        opts.buttonGAC = false;
        opts.buttonHRP = false;
        opts.buttonVIIRS = false;
        opts.buttonRealTime = true;

        ui->btnMetop->setChecked(opts.buttonMetop);
        ui->btnNoaa->setChecked(opts.buttonNoaa);
        ui->btnGAC->setChecked(opts.buttonGAC);
        ui->btnHRP->setChecked(opts.buttonHRP);
        ui->btnVIIRS->setChecked(opts.buttonVIIRS);
        ui->btnRealTime->setChecked(opts.buttonRealTime);


    }
    //this->RemoveAllSelected();
    this->setScrollBarMaximum();
}

void FormMapCyl::setScrollBarMaximum()
{

    qDebug() << QString("setscrollbarmaximum = %1").arg(segs->seglmetop->NbrOfSegments());

    if (opts.buttonMetop)
    {
        ui->verticalScrollBar->setMaximum(segs->seglmetop->NbrOfSegments());
    }
    else if (opts.buttonNoaa)
    {
        ui->verticalScrollBar->setMaximum(segs->seglnoaa->NbrOfSegments());
    }
    else if (opts.buttonHRP)
    {
        ui->verticalScrollBar->setMaximum(segs->seglhrp->NbrOfSegments());
    }
    else if (opts.buttonGAC)
    {
        ui->verticalScrollBar->setMaximum(segs->seglgac->NbrOfSegments());
    }
    else if (opts.buttonVIIRS)
    {
        ui->verticalScrollBar->setMaximum(segs->seglviirs->NbrOfSegments());
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
        //qDebug() << outp;
    }
    else if(opts.buttonGAC)
    {
        segs->seglgac->ShowSegment(value);
        segs->seglgac->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglgac->NbrOfSegments();

        outp = QString("GAC From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
    }
    else if(opts.buttonVIIRS)
    {
        segs->seglviirs->ShowSegment(value);
        segs->seglviirs->GetFirstLastVisible(&first, &last);
        nbrseg = segs->seglviirs->NbrOfSegments();

        outp = QString("VIIRS From %1 to %2  #Segments %3").arg(first.toString(Qt::TextDate)).arg(last.toString(Qt::TextDate)).arg(nbrseg);
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
    segs->RemoveAllSelectedVIIRS();

    mapcyl->update();
    ui->btnRemoveSelected->setText(" No selected segments ");

}

void FormMapCyl::slotNothingSelected()
{
    ui->btnRemoveSelected->setText(" No selected segments ");
}

void FormMapCyl::on_btnRemoveSelected_clicked()
{
    RemoveAllSelected();
}

void FormMapCyl::on_btnMakeImage_clicked()
{
    emit emitMakeImage();
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
    toggleButtonNoaa();
}

void FormMapCyl::on_btnMetop_clicked()
{
    formtoolbox->setTabWidgetIndex(0);
    toggleButtonMetop();
}

void FormMapCyl::on_btnHRP_clicked()
{
    formtoolbox->setTabWidgetIndex(0);
    toggleButtonHRP();
}

void FormMapCyl::on_btnGAC_clicked()
{
    formtoolbox->setTabWidgetIndex(0);
    toggleButtonGAC();
}

void FormMapCyl::on_btnRealTime_clicked()
{
    toggleButtonRealTime();
}

void FormMapCyl::on_btnVIIRS_clicked()
{
    formtoolbox->setTabWidgetIndex(1);
    toggleButtonVIIRS();
}

void FormMapCyl::on_btnAllSegments_clicked()
{
    segs->setShowAllSegments(ui->btnAllSegments->isChecked());
}

void FormMapCyl::on_btnPhong_clicked()
{
    if(opts.bPhongModel)
    {
        ui->btnPhong->setChecked(false);
        opts.bPhongModel = false;
    }
    else
    {
        ui->btnPhong->setChecked(true);
        opts.bPhongModel = true;
    }
}
