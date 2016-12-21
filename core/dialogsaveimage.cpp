#include "dialogsaveimage.h"
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QCheckBox>
#include <QListWidget>
#include <QDebug>
#include <QComboBox>

DialogSaveImage::DialogSaveImage()
{


    QList<QDialogButtonBox *> allDialogButtons = this->findChildren<QDialogButtonBox *>();
    QList<QComboBox *> allComboBoxs = this->findChildren<QComboBox *>();
    qDebug() << "Number of QDialogButtonBox = " << allDialogButtons.count() << " name = " << allDialogButtons.at(0)->objectName();
    qDebug() << "Number of QComboBox = " << allComboBoxs.count(); //  << " name = " << allDialogButtons.at(0)->objectName();
    foreach(QComboBox *cmb, allComboBoxs)
    {
        qDebug() << cmb->objectName();
    }

    QList<QGridLayout *> allGridLayout = this->findChildren<QGridLayout *>(QString(), Qt::FindDirectChildrenOnly);
    qDebug() << "Number of QGridLayout = " << allGridLayout.count();

    foreach(QGridLayout *bl, allGridLayout)
    {
        qDebug() << bl->objectName();
    }


}

void DialogSaveImage::addCheckBoxIn()
{
    QDialogButtonBox *box = this->findChild<QDialogButtonBox*>("buttonBox");
    Q_ASSERT(box);
    QBoxLayout *l = box->findChild<QBoxLayout*>();
    Q_ASSERT(l);

//    QCheckBox *toProj = new QCheckBox("copyright text :", box);
//    toProj->setChecked(true);
//    l->insertWidget(2, toProj);
    QList<QGridLayout *> gridlist = this->findChildren<QGridLayout*>(QString(), Qt::FindDirectChildrenOnly);
    QGridLayout *glayout = gridlist.at(0);

    QCheckBox *chkCopyright = new QCheckBox("copyright text :", this);
    chkCopyright->setChecked(true);
    glayout->addWidget(chkCopyright, 4, 0);
}

void DialogSaveImage::slotFile(QString fileselected)
{
    qDebug() << "File selected = " << fileselected;
}
