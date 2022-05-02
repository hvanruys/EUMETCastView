#include "formmovie.h"
#include "poi.h"
#include "ui_formmovie.h"

extern Options opts;
extern SegmentImage *imageptrs;
extern Poi poi;

FormMovie::FormMovie(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormMovie)
{
    ui->setupUi(this);

    for(int i = 0; i < opts.geosatellites.count(); i++)
    {
        if(opts.geosatellites.at(i).shortname == "MET_11")
        {

        }
    }

    ui->spbThreadcount->setValue(opts.threadcount);

    for(int i = 0; i < opts.pathlist.count(); i++)
        ui->tePathlist->append(opts.pathlist.at(i));

    setupSpectrum();
    setupSatname();

    ui->leSingleImage->setText(opts.singleimage);
    ui->leGshhsOverlay1->setText(opts.videogshhsoverlayfile1);
    ui->leGshhsOverlay2->setText(opts.videogshhsoverlayfile2);
    ui->leGshhsOverlay3->setText(opts.videogshhsoverlayfile3);
    ui->rbGshhs1->setChecked(opts.videogshhsglobe1On);
    ui->rbGshhs2->setChecked(opts.videogshhsglobe2On);
    ui->rbGshhs3->setChecked(opts.videogshhsglobe3On);
    ui->spbGamma->setValue(opts.videogamma);
    //ui->cmbRss->set

    QColor color;

    ui->btnOverlayColor1->setText(opts.videooverlaycolor1);
    color.setNamedColor(opts.videooverlaycolor1);
    ui->lblOverlayColor1->setPalette(QPalette(color));
    ui->lblOverlayColor1->setAutoFillBackground(true);

    ui->btnOverlayColor2->setText(opts.videooverlaycolor2);
    color.setNamedColor(opts.videooverlaycolor2);
    ui->lblOverlayColor2->setPalette(QPalette(color));
    ui->lblOverlayColor2->setAutoFillBackground(true);

    ui->btnOverlayColor3->setText(opts.videooverlaycolor3);
    color.setNamedColor(opts.videooverlaycolor3);
    ui->lblOverlayColor3->setPalette(QPalette(color));
    ui->lblOverlayColor3->setAutoFillBackground(true);

    ui->btnOverlayGridColor->setText(opts.videooverlaygridcolor);
    color.setNamedColor(opts.videooverlaygridcolor);
    ui->lblOverlayGridColor->setPalette(QPalette(color));
    ui->lblOverlayGridColor->setAutoFillBackground(true);

    ui->spbFontSize->setValue(opts.videooverlaydatefontsize);


    ui->leVideoHeight->setText(poi.strlGVPMapHeight.at(0));
    ui->leVideoWidth->setText(poi.strlGVPMapWidth.at(0));
    ui->chkOverlayBorder->setChecked(opts.videooverlayborder);
    ui->chkOverlayDate->setChecked(opts.videooverlaydate);

    ui->leScale->setText(poi.strlGVPScale.at(0));
    ui->leHeight->setText(poi.strlGVPHeight.at(0));
    ui->leLatitude->setText(poi.strlGVPLat.at(0));
    ui->leLongitude->setText(poi.strlGVPLon.at(0));
    ui->chkDisplayGrid->setChecked(poi.strlGVPGridOnProj.at(0).toInt());
    ui->leFalseEasting->setText(poi.strlGVPFalseEasting.at(0));
    ui->leFalseNorthing->setText(poi.strlGVPFalseNorthing.at(0));

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::LocalHost, 7755);

    connect(udpSocket, &QUdpSocket::readyRead, this, &FormMovie::readPendingDatagrams);



}

void FormMovie::readPendingDatagrams()
    {
        while (udpSocket->hasPendingDatagrams()) {
            QNetworkDatagram datagram = udpSocket->receiveDatagram();
            QString replyData = QString(datagram.data());
            writeTolistwidget(replyData);
        }
    }

void FormMovie::saveOverlayColorsToOptions()
{

    opts.videooverlaycolor1 = ui->btnOverlayColor1->text();
    opts.videooverlaycolor2 = ui->btnOverlayColor2->text();
    opts.videooverlaycolor3 = ui->btnOverlayColor3->text();
    opts.videooverlaygridcolor = ui->btnOverlayGridColor->text();

}

void FormMovie::saveSpectrumToOptions()
{
    opts.dayred = ui->cmbDayRed->currentText();
    opts.daygreen = ui->cmbDayGreen->currentText();
    opts.dayblue = ui->cmbDayBlue->currentText();
    opts.nightred = ui->cmbNightRed->currentText();
    opts.nightgreen = ui->cmbNightGreen->currentText();
    opts.nightblue = ui->cmbNightBlue->currentText();
    opts.dayredinverse = ui->rbDayRedInverse->isChecked();
    opts.daygreeninverse = ui->rbDayGreenInverse->isChecked();
    opts.dayblueinverse = ui->rbDayBlueInverse->isChecked();
    opts.nightredinverse = ui->rbNightRedInverse->isChecked();
    opts.nightgreeninverse = ui->rbNightGreenInverse->isChecked();
    opts.nightblueinverse = ui->rbNightBlueInverse->isChecked();

}

void FormMovie::setupSpectrum()
{
    QStringList spectrum;
    spectrum << "------" << "VIS006" << "VIS008" << "IR_016" << "IR_039" << "WV_062" << "WV_073" << "IR_087" << "IR_097" << "IR_108" << "IR_120" << "IR134";
    ui->cmbDayRed->addItems(spectrum);
    ui->cmbDayGreen->addItems(spectrum);
    ui->cmbDayBlue->addItems(spectrum);
    ui->cmbNightRed->addItems(spectrum);
    ui->cmbNightGreen->addItems(spectrum);
    ui->cmbNightBlue->addItems(spectrum);

    ui->cmbDayRed->setCurrentText(opts.dayred);
    ui->cmbDayGreen->setCurrentText(opts.daygreen);
    ui->cmbDayBlue->setCurrentText(opts.dayblue);
    ui->cmbNightRed->setCurrentText(opts.nightred);
    ui->cmbNightGreen->setCurrentText(opts.nightgreen);
    ui->cmbNightBlue->setCurrentText(opts.nightblue);

    ui->rbDayRedInverse->setChecked(opts.dayredinverse);
    ui->rbDayGreenInverse->setChecked(opts.daygreeninverse);
    ui->rbDayBlueInverse->setChecked(opts.dayblueinverse);

    ui->rbNightRedInverse->setChecked(opts.nightredinverse);
    ui->rbNightGreenInverse->setChecked(opts.nightgreeninverse);
    ui->rbNightBlueInverse->setChecked(opts.nightblueinverse);

}

void FormMovie::setupSatname()
{
    QStringList satnames;
    satnames << "MET_11" << "MET_10" << "MET_9" << "MET_8";
    ui->cmbSatname->addItems(satnames);
    ui->cmbSatname->setCurrentText(opts.satname);

}



bool FormMovie::saveFormToOptions()
{
    bool ret = false;

    QString pathlistdata = ui->tePathlist->toPlainText();
    qDebug() << pathlistdata;
    QStringList list;
    list = pathlistdata.split(QRegExp("\\s+"));
    opts.pathlist.clear();
    opts.pathlist = list;

    opts.videooverlaydatefontsize = ui->spbFontSize->value();
    if(ui->leSingleImage->text().length() > 0 && ui->leSingleImage->text().length() != 12)
    {
        QMessageBox msgBox;
        msgBox.setText("The 'SingleImage' string must be 12 digits long (YYYYMMDDHHmm) or empty.");
        msgBox.exec();
        ret = false;
    }
    else
    {
        opts.singleimage = ui->leSingleImage->text();
        saveOverlayColorsToOptions();
        saveSpectrumToOptions();
        opts.satname = ui->cmbSatname->currentText();
        opts.videoresolutionheight = ui->leVideoHeight->text().toInt();
        opts.videoresolutionwidth = ui->leVideoWidth->text().toInt();
        opts.videogamma = ui->spbGamma->value();
        opts.videooverlayborder = ui->chkOverlayBorder->isChecked();
        opts.videooverlaydate = ui->chkOverlayDate->isChecked();
        opts.videooverlaydatefontsize = ui->spbFontSize->value();


        ret = true;
    }

    return(ret);

}

FormMovie::~FormMovie()
{
    saveFormToOptions();
    delete ui;
}

void FormMovie::getProjectionData()
{
    int tabwidgetindex = formtoolbox->getTabWidgetIndex();
    eProjectionType projtype = formtoolbox->getCurrentProjectionType();
    int toolboxindex = formtoolbox->getToolboxIndex();
    QString lbl = QString("tabwidgetindex = %1 toolboxindex = %2 height = %3 width = %4").arg(tabwidgetindex).arg(toolboxindex).arg(formtoolbox->getGVPMapHeight()).arg(formtoolbox->getGVPMapWidth());


}

void FormMovie::on_btnCreateXML_clicked()
{
    if(saveFormToOptions() == false)
        return;

    QDomDocument doc("EUMETCastVideo");
    QDomElement root = doc.createElement("root");
    doc.appendChild(root);

    QDomElement tag = doc.createElement("threadcount");
    root.appendChild(tag);

    int tc = ui->spbThreadcount->value();
    QDomText t = doc.createTextNode(QString("%1").arg(tc));
    tag.appendChild(t);

    QDomElement tagroot = doc.createElement("pathlist");
    root.appendChild(tagroot);


    QString pathlistdata = ui->tePathlist->toPlainText();
    QStringList list;
    list = pathlistdata.split(QRegExp("\\s+"));

    foreach (const QString &str, list) {
        tag = doc.createElement("path");
        tagroot.appendChild(tag);
        QDomText t = doc.createTextNode(str);
        tag.appendChild(t);
    }

    tag = doc.createElement("satname");
    root.appendChild(tag);
    t = doc.createTextNode(ui->cmbSatname->currentText());
    tag.appendChild(t);

    if(ui->cmbSatname->currentText() == "MET_11")
    {
        tag = doc.createElement("pattern");
        root.appendChild(tag);
        t = doc.createTextNode("H-000-MSG4__-MSG4_????___-??????___-??????___-????????????-?_");
        tag.appendChild(t);

        tag = doc.createElement("rss");
        root.appendChild(tag);
        t = doc.createTextNode("0");
        tag.appendChild(t);

    }
    else if(ui->cmbSatname->currentText() == "MET_10")
    {
        tag = doc.createElement("pattern");
        root.appendChild(tag);
        t = doc.createTextNode("H-000-MSG3__-MSG3_????___-??????___-??????___-????????????-?_");
        tag.appendChild(t);

        tag = doc.createElement("rss");
        root.appendChild(tag);
        t = doc.createTextNode("1");
        tag.appendChild(t);

    }
    else if(ui->cmbSatname->currentText() == "MET_9")
    {
        tag = doc.createElement("pattern");
        root.appendChild(tag);
        t = doc.createTextNode("H-000-MSG2__-MSG2_????___-??????___-??????___-????????????-?_");
        tag.appendChild(t);

        tag = doc.createElement("rss");
        root.appendChild(tag);
        t = doc.createTextNode("0");
        tag.appendChild(t);

    }
    else if(ui->cmbSatname->currentText() == "MET_8")
    {
        tag = doc.createElement("pattern");
        root.appendChild(tag);
        t = doc.createTextNode("H-000-MSG1__-MSG1_????___-??????___-??????___-????????????-?_");
        tag.appendChild(t);

        tag = doc.createElement("rss");
        root.appendChild(tag);
        t = doc.createTextNode("0");
        tag.appendChild(t);

    }
    else
        return;

    tag = doc.createElement("singleimage");
    root.appendChild(tag);
    t = doc.createTextNode(ui->leSingleImage->text());
    tag.appendChild(t);

    tag = doc.createElement("gamma");
    root.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->spbGamma->value()));
    tag.appendChild(t);


    tagroot = doc.createElement("gshhs");
    root.appendChild(tagroot);

    tag = doc.createElement("gshhsoverlayfile1");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->leGshhsOverlay1->text());
    tag.appendChild(t);

    tag = doc.createElement("gshhsoverlayfile2");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->leGshhsOverlay2->text());
    tag.appendChild(t);

    tag = doc.createElement("gshhsoverlayfile3");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->leGshhsOverlay3->text());
    tag.appendChild(t);

    tag = doc.createElement("gshhsglobe1On");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->rbGshhs1->isChecked()));
    tag.appendChild(t);

    tag = doc.createElement("gshhsglobe2On");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->rbGshhs2->isChecked()));
    tag.appendChild(t);

    tag = doc.createElement("gshhsglobe3On");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->rbGshhs3->isChecked()));
    tag.appendChild(t);

    tagroot = doc.createElement("resolution");
    root.appendChild(tagroot);

    tag = doc.createElement("height");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->leVideoHeight->text());
    tag.appendChild(t);

    tag = doc.createElement("width");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->leVideoWidth->text());
    tag.appendChild(t);

    tagroot = doc.createElement("dayspectrum");
    root.appendChild(tagroot);

    tag = doc.createElement("dayred");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->cmbDayRed->currentText() == "------" ? "" : ui->cmbDayRed->currentText());
    tag.appendChild(t);

    tag = doc.createElement("daygreen");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->cmbDayGreen->currentText() == "------" ? "" : ui->cmbDayGreen->currentText());
    tag.appendChild(t);

    tag = doc.createElement("dayblue");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->cmbDayBlue->currentText() == "------" ? "" : ui->cmbDayBlue->currentText());
    tag.appendChild(t);

    tag = doc.createElement("dayredinverse");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->rbDayRedInverse->isChecked()));
    tag.appendChild(t);

    tag = doc.createElement("daygreeninverse");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->rbDayGreenInverse->isChecked()));
    tag.appendChild(t);

    tag = doc.createElement("dayblueinverse");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->rbDayBlueInverse->isChecked()));
    tag.appendChild(t);

    tag = doc.createElement("dayhrv");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->chkHRV->isChecked()));
    tag.appendChild(t);


    tagroot = doc.createElement("nightspectrum");
    root.appendChild(tagroot);

    tag = doc.createElement("nightred");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->cmbNightRed->currentText() == "------" ? "" : ui->cmbNightRed->currentText());
    tag.appendChild(t);

    tag = doc.createElement("nightgreen");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->cmbNightGreen->currentText() == "------" ? "" : ui->cmbNightGreen->currentText());
    tag.appendChild(t);

    tag = doc.createElement("nightblue");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->cmbNightBlue->currentText() == "------" ? "" : ui->cmbNightBlue->currentText());
    tag.appendChild(t);

    tag = doc.createElement("nightredinverse");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->rbNightRedInverse->isChecked()));
    tag.appendChild(t);

    tag = doc.createElement("nightgreeninverse");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->rbNightGreenInverse->isChecked()));
    tag.appendChild(t);

    tag = doc.createElement("nightblueinverse");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->rbNightBlueInverse->isChecked()));
    tag.appendChild(t);

    tagroot = doc.createElement("overlay");
    root.appendChild(tagroot);

    tag = doc.createElement("coff");
    tagroot.appendChild(tag);
    t = doc.createTextNode("1856");
    tag.appendChild(t);

    tag = doc.createElement("loff");
    tagroot.appendChild(tag);
    t = doc.createTextNode("1856");
    tag.appendChild(t);

    tag = doc.createElement("cfac");
    tagroot.appendChild(tag);
    t = doc.createTextNode("781648343.0");
    tag.appendChild(t);

    tag = doc.createElement("lfac");
    tagroot.appendChild(tag);
    t = doc.createTextNode("781648343.0");
    tag.appendChild(t);

    tag = doc.createElement("coffhrv");
    tagroot.appendChild(tag);
    t = doc.createTextNode("5566");
    tag.appendChild(t);

    tag = doc.createElement("loffhrv");
    tagroot.appendChild(tag);
    t = doc.createTextNode("5566");
    tag.appendChild(t);

    tag = doc.createElement("cfachrv");
    tagroot.appendChild(tag);
    t = doc.createTextNode("2344944937.0");
    tag.appendChild(t);

    tag = doc.createElement("lfachrv");
    tagroot.appendChild(tag);
    t = doc.createTextNode("2344944937.0");
    tag.appendChild(t);

    if(ui->cmbSatname->currentText() == "MET_11")
    {
        tag = doc.createElement("satlon");
        tagroot.appendChild(tag);
        t = doc.createTextNode("0.0");
        tag.appendChild(t);
    }
    else if(ui->cmbSatname->currentText() == "MET_10")
    {
        tag = doc.createElement("satlon");
        tagroot.appendChild(tag);
        t = doc.createTextNode("9.5");
        tag.appendChild(t);
    }
    else if(ui->cmbSatname->currentText() == "MET_9")
    {
        tag = doc.createElement("satlon");
        tagroot.appendChild(tag);
        t = doc.createTextNode("45.5");
        tag.appendChild(t);
    }
    else if(ui->cmbSatname->currentText() == "MET_8")
    {
        tag = doc.createElement("satlon");
        tagroot.appendChild(tag);
        t = doc.createTextNode("41.5");
        tag.appendChild(t);
    }

    tag = doc.createElement("homelon");
    tagroot.appendChild(tag);
    t = doc.createTextNode("3.5537");
    tag.appendChild(t);

    tag = doc.createElement("homelat");
    tagroot.appendChild(tag);
    t = doc.createTextNode("50.8330");
    tag.appendChild(t);

    tag = doc.createElement("projectionoverlaycolor1");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->btnOverlayColor1->text());
    tag.appendChild(t);

    tag = doc.createElement("projectionoverlaycolor2");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->btnOverlayColor2->text());
    tag.appendChild(t);

    tag = doc.createElement("projectionoverlaycolor3");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->btnOverlayColor3->text());
    tag.appendChild(t);

    tag = doc.createElement("projectionoverlaygridcolor");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->btnOverlayGridColor->text());
    tag.appendChild(t);

    tag = doc.createElement("overlayborder");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->chkOverlayBorder->isChecked()));
    tag.appendChild(t);

    tag = doc.createElement("overlaydate");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->chkOverlayDate->isChecked()));
    tag.appendChild(t);

    tag = doc.createElement("overlaydatefontsize");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->spbFontSize->value()));
    tag.appendChild(t);

    tagroot = doc.createElement("projectiontype");
    root.appendChild(tagroot);
    t = doc.createTextNode("GVP");
    tagroot.appendChild(t);

    tagroot = doc.createElement("gvpprojectionparameters");
    root.appendChild(tagroot);

    tag = doc.createElement("latitude");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->leLatitude->text());
    tag.appendChild(t);

    tag = doc.createElement("longitude");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->leLongitude->text());
    tag.appendChild(t);

    tag = doc.createElement("scale");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->leScale->text());
    tag.appendChild(t);

    tag = doc.createElement("height");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->leHeight->text());
    tag.appendChild(t);

    tag = doc.createElement("gridonprojection");
    tagroot.appendChild(tag);
    t = doc.createTextNode(QString("%1").arg(ui->chkDisplayGrid->isChecked()));
    tag.appendChild(t);

    tag = doc.createElement("falseeasting");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->leFalseEasting->text());
    tag.appendChild(t);

    tag = doc.createElement("falsenorthing");
    tagroot.appendChild(tag);
    t = doc.createTextNode(ui->leFalseNorthing->text());
    tag.appendChild(t);

    if(ui->chkHRV->isChecked())
    {
        tagroot = doc.createElement("videooutputname");
        root.appendChild(tagroot);
        t = doc.createTextNode("PROJHRV");
        tagroot.appendChild(t);
    }
    else
    {
        tagroot = doc.createElement("videooutputname");
        root.appendChild(tagroot);
        t = doc.createTextNode("PROJ");
        tagroot.appendChild(t);
    }







    QString xmlstring = doc.toString();

    QFile file("EUMETCastVideo.xml");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    out << xmlstring;
    file.close();
    ui->lwTraffic->addItem("EUMETCastVideo.xml is created !");

    QProcess process;
    process.setProgram("EUMETCastVideo");
    //process.setArguments({"Jeopardy_Theme.mp3"});
    //process.setWorkingDirectory(musicDirPath);
    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());
    qint64 pid;
    bool isstarted = process.startDetached(&pid);
    if(!isstarted)
    {
        QMessageBox msgBox;
        msgBox.setText("The process 'EUMETCastVideo' is not started !");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
    else
    {
        ui->lwTraffic->addItem("The process 'EUMETCastVideo' is started !");
    }
}

void FormMovie::writeTolistwidget(QString txt)
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    ui->lwTraffic->insertItem(0, QString(now.toString(Qt::ISODate)) + " " + txt);
    if (ui->lwTraffic->count() > 300)
    {
        delete ui->lwTraffic->item(ui->lwTraffic->count() - 1);
    }
}

void FormMovie::on_btnOverlayColor1_clicked()
{
    QColor color(opts.videooverlaycolor1);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnOverlayColor1->setText(color.name());
        ui->lblOverlayColor1->setPalette(QPalette(color));
        ui->lblOverlayColor1->setAutoFillBackground(true);
        opts.videooverlaycolor1 = ui->btnOverlayColor1->text();

    }
}

void FormMovie::on_btnOverlayColor2_clicked()
{
    QColor color(opts.videooverlaycolor2);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnOverlayColor2->setText(color.name());
        ui->lblOverlayColor2->setPalette(QPalette(color));
        ui->lblOverlayColor2->setAutoFillBackground(true);
        opts.videooverlaycolor2 = ui->btnOverlayColor2->text();

    }
}


void FormMovie::on_btnOverlayColor3_clicked()
{
    QColor color(opts.videooverlaycolor3);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnOverlayColor3->setText(color.name());
        ui->lblOverlayColor3->setPalette(QPalette(color));
        ui->lblOverlayColor3->setAutoFillBackground(true);
        opts.videooverlaycolor3 = ui->btnOverlayColor3->text();

    }
}


void FormMovie::on_btnOverlayGridColor_clicked()
{
    QColor color(opts.videooverlaygridcolor);
    color = QColorDialog::getColor(color, this);

    if (color.isValid())
    {
        ui->btnOverlayGridColor->setText(color.name());
        ui->lblOverlayGridColor->setPalette(QPalette(color));
        ui->lblOverlayGridColor->setAutoFillBackground(true);
        opts.videooverlaygridcolor = ui->btnOverlayGridColor->text();

    }
}


void FormMovie::on_btnAddPath_clicked()
{
    QString newpath;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if ( !dir.isEmpty() )
    {
        ui->tePathlist->append(dir);
    }

}

void FormMovie::setGVPlat(double latitude)
{
    ui->leLatitude->setText(QString("%1").arg(latitude));
}

void FormMovie::setGVPlon(double longitude)
{
    ui->leLongitude->setText(QString("%1").arg(longitude));
}

void FormMovie::setGVPscale(double scale)
{
    ui->leScale->setText(QString("%1").arg(scale));
}

void FormMovie::setGVPheight(int height)
{
    ui->leHeight->setText(QString("%1").arg(height));
}

void FormMovie::setGVPMapHeight(int height)
{
    ui->leVideoHeight->setText(QString("%1").arg(height));
}

void FormMovie::setGVPMapWidth(int width)
{
    ui->leVideoWidth->setText(QString("%1").arg(width));
}

void FormMovie::setGVPFalseEasting(double easting)
{
    ui->leFalseEasting->setText(QString("%1").arg(easting));
}

void FormMovie::setGVPFalseNorthing(double northing)
{
    ui->leFalseNorthing->setText(QString("%1").arg(northing));
}

void FormMovie::setGVPDisplayGrid(bool grid)
{
    ui->chkDisplayGrid->setChecked(grid);
}

