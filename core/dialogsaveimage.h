#ifndef DIALOGSAVEIMAGE_H
#define DIALOGSAVEIMAGE_H

#include <QFileDialog>

class DialogSaveImage : public QFileDialog
{
public:
    DialogSaveImage();
    void addCheckBoxIn();

public slots:
    void slotFile(QString fileselected);

};

#endif // DIALOGSAVEIMAGE_H
