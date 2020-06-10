#ifndef LOADING_H
#define LOADING_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class loading;
}

class loading : public QMainWindow
{
    Q_OBJECT

public:
    explicit loading(QWidget *parent = nullptr);
    ~loading();

    void bootUpPageLoad(void);

private slots:
    void bootUpPageUpdate();

private:
    Ui::loading *ui;
    QTimer *bootUpPgUpdateTimer;
    int progressBarCntr;
    int bootUpPageShowCntr;
};

#endif // LOADING_H
