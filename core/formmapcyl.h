#ifndef FORMMAPCYL_H
#define FORMMAPCYL_H

#include "options.h"
#include "cylequidist.h"
#include "satellite.h"
#include "avhrrsatellite.h"
#include "mapcyl.h"
#include "globe.h"
#include "formtoolbox.h"
#include "productlist.h"

#include <QWidget>

namespace Ui {
    class FormMapCyl;
}

class FormToolbox;
struct ProductList;

class FormMapCyl : public QWidget
{
    Q_OBJECT

public:
    explicit FormMapCyl(QWidget *parent = 0, MapFieldCyl *p_mapcyl = 0, Globe *p_globe = 0, FormToolbox *p_formtoolbox = 0, AVHRRSatellite *seglist=0 );

    int ExtractSegment(QString ArchivePath, QString DestinationPath);
    bool AreThereSelectedSegments();
    void setCylOrGlobe(int ind);
    bool CheckUserAndPassword();

private:
    Ui::FormMapCyl *ui;
    AVHRRSatellite *segs;

    MapFieldCyl *mapcyl;
    Globe *globe;
    FormToolbox *formtoolbox;

    QString currentDownloadFilename1;
    QString currentDownloadFilename2;

    QList<ProductList> todownloadlist;

    void RenderQuicklookinTexture(QString completebasename);


    void RemoveAllSelected();
    void SetAllButtonsToFalse();
    bool IsProductDirFilledIn();
    bool QuicklookExist(QString completebasename);
    bool FileExist(QString completebasename, QString band_or_quicklook);
    bool WriteNetCDFFile(int *longitude_img, int *latitude_img, int tierowslength, int columnslength);
    int copy_data(struct archive *ar, struct archive *aw);


private slots:

    void toggleButton(eSegmentType segtype);
    void updatesatmap(int);
    void setScrollBarMaximum();
    void slotShowSegmentCount();

    void on_btnRemoveSelected_clicked();

    void on_btnMakeImage_clicked();

    void on_btnSaveTexture_clicked();

    void on_btnClearTexture_clicked();

    void on_verticalScrollBar_valueChanged(int value);

    void on_btnMetop_clicked();
    void on_btnMetopSGA1_clicked();

    void on_btnHRP_clicked();

    void on_btnRealTime_clicked();

    void on_btnVIIRSM_clicked();
    void on_btnVIIRSDNB_clicked();
    void on_btnVIIRSMNOAA20_clicked();
    void on_btnVIIRSDNBNOAA20_clicked();
    void on_btnVIIRSMNOAA21_clicked();
    void on_btnVIIRSDNBNOAA21_clicked();

    void on_btnAllSegments_clicked();

    void on_btnPhong_clicked();

    void on_btnOLCIefr_clicked();

    void on_btnOLCIerr_clicked();

    void on_btnMERSI_clicked();

public slots:
      void showSegmentList(int);
      void showSegmentCount();
      void changeScrollBar(int);
      void slotSetMapCylButtons(bool stat);

signals:
    void signalSegmentChanged(QString);
    void signalMakeImage();

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;


};

#endif // FORMMAPCYL_H

