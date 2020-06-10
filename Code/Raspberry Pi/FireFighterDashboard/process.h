#ifndef PROCESS_H
#define PROCESS_H

#include <QObject>
#include <QThread>

class Process : public QThread
{
    Q_OBJECT
public:
    explicit Process(QObject *parent = nullptr);
    void run();
    bool stop;
signals:

public slots:
};

#endif // PROCESS_H
