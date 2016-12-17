#include "dialogsaveimage.h"
#include "ui_dialogsaveimage.h"
#include <QRadioButton>

DialogSaveImage::DialogSaveImage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSaveImage)
{
    ui->setupUi(this);
    QFileDialog* f = new QFileDialog();
    ui->saveimageLayout->addWidget(f);
    QRadioButton *rdb = new QRadioButton();
    rdb->setText("Set copyright text on image");
    ui->saveimageLayout->addWidget(rdb);
    connect(f,SIGNAL(finished(int)), this, SLOT(closethis(int)));
}

void DialogSaveImage::closethis(int ret)
{
    qDebug() << "closethis = " << ret;
    this->close();
}

DialogSaveImage::~DialogSaveImage()
{
    delete ui;
}
