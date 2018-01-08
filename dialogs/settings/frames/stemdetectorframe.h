#ifndef STEMDETECTORFRAME_H
#define STEMDETECTORFRAME_H

#include <QWidget>
#include <utilities/commonstructs.h>

namespace Ui {
class StemDetectorFrame;
}

class StemDetectorFrame : public QWidget
{
    Q_OBJECT
signals:
    void detectorsChanged();

public:
    explicit StemDetectorFrame(QWidget *parent, std::vector<StemDetector> &d);

    ~StemDetectorFrame();

    std::vector<StemDetector> getChosenDetectors() {return chosenDetectors;}

private slots:
    void on_btnAdd_clicked();

    void on_btnDelete_clicked();

    void on_edtName_textChanged(const QString &arg1);

    void dlgOk_clicked();

    bool dlgApply_clicked();

    void dlgCancel_clicked();

    void doRadiiValid(QString dud);

private:
    Ui::StemDetectorFrame *ui;

    bool checkNameValid(std::string name);

    bool checkRadiiValid();

    void addItemToList(StemDetector det);

    void setNewName();

    std::vector<StemDetector>& chosenDetectors;
};

#endif // STEMDETECTORFRAME_H
