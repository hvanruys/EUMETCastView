#ifndef DIALOGSAVEIMAGE_H
#define DIALOGSAVEIMAGE_H

#include <QDialog>
#include <QFileDialog>
#include <QDebug>

namespace Ui {
class DialogSaveImage;
}

class DialogSaveImage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSaveImage(QWidget *parent = 0);
    ~DialogSaveImage();

private:
    Ui::DialogSaveImage *ui;

private slots:
    void closethis(int ret);
};

#endif // DIALOGSAVEIMAGE_H
