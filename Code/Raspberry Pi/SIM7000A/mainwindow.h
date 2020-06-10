#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int delay_s(int sec);
    int delay_ms(int msec);
private slots:
    void processNMEAResponse(void);
    void sendQueryToLTEModule(void);
private:
    Ui::MainWindow *ui;
    QSerialPort *serial_AT;
    QSerialPort *serial_NMEA;

    QTimer *queryGPSData;
};

#endif // MAINWINDOW_H
