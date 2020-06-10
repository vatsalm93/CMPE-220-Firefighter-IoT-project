#include "process.h"
#include <QMutex>
#include "mainwindow.h"
extern MainWindow *w;
Process::Process(QObject *parent) :
    QThread(parent)
{
    this->stop = true;
}

void Process::run()
{
    while (stop) {
        QMutex mutex;
        mutex.lock();
        w->check_net_connectivity();
        mutex.unlock();
        this->msleep(100);
    }
}
