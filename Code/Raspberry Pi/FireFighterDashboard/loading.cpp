#include "loading.h"
#include "ui_loading.h"
#include "mainwindow.h"
#include "process.h"

Process *net_status;
extern loading *loadForm;
MainWindow *w;
//#05B8CC
/**
QProgressBar {
     border: 2px solid grey;
     border-radius: 5px;
     background-color: rgb(0,0,0);
 }

 QProgressBar::chunk {
     background-color: rgb(180,255,62);

     width: 20px;
 }
 */
loading::loading(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::loading)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint);
//    setFixedSize(400,300);
    bootUpPageLoad();
}

loading::~loading()
{
    delete bootUpPgUpdateTimer;
    delete ui;
    qDebug() << "Loading form destructor invoked";
}

void loading::bootUpPageLoad()
{
    progressBarCntr = 0;
    bootUpPageShowCntr = 0;

    bootUpPgUpdateTimer = new QTimer(this);
    ui->progressBar->setValue(0);
    ui->progressBar->setStyleSheet("QProgressBar::chunk {border-radius: 5px;width: 4px;background-color: rgb(255,241,0);}");
    //rgb(5,184,204)
    connect(bootUpPgUpdateTimer, SIGNAL(timeout()), this, SLOT(bootUpPageUpdate()));
    bootUpPgUpdateTimer->setTimerType(Qt::PreciseTimer);
    bootUpPgUpdateTimer->start(100);
}

void loading::bootUpPageUpdate()
{
    int val = ui->progressBar->value();

    if (val < 100) {
        ui->progressBar->setValue(val+1);
        ui->progressVal->setText(QString::number(val).rightJustified(1,' ') + "%");
    } else {
        bootUpPgUpdateTimer->stop();

        loadForm->hide();
        w = new MainWindow();
        w->setWindowFlags(Qt::FramelessWindowHint);
        w->showFullScreen();
        net_status = new Process;
        net_status->start();
    }
}
