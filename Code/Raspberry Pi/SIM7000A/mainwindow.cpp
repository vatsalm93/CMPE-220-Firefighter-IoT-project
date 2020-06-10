#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <QThread>

const QString portNMEA = "/dev/ttyUSB1";
const QString portAT = "/dev/ttyUSB2";

//"\r\n+CGNSINF: 1,1,20190727032055.00,37.335236,-121.888412,30.200,0.00,83.1,1,,1.5,1.8,1.0,,12,4,,,24,,\r\n\r\nOK\r\n"
#pragma pack(1)
typedef struct {
    char runStatus[2];
    char fixStatus[2];
    char utcDateTime[19];
    char latitude[11];
    char longitude[12];
    char mslAltitude[9];
    char speedOverGnd[7];
    char courseOverGnd[7];
    char fixMode[2];
    //reserved place 1 in the response
    char hdop[5];
    char pdop[5];
    char vdop[5];
    //reserved place 2 in the response
    char satellitesInView[3];
    char satellitesUsed[3];
    char glonassSatellitesInView[3];
    //reserved place 3 in the response
    char cn0Max[3];
    char hpa[7];
    char vpa[7];
} at_cgnsinf_t;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->edit_gnssPower->setText(QString("OFF"));
    ui->checkBox_fixStatus->setDisabled(true);
    ui->checkBox_fixStatus->setCheckState(Qt::Unchecked);
    ui->edit_lat->setReadOnly(true);
    ui->edit_lon->setReadOnly(true);

    serial_AT = new QSerialPort(this);
    serial_AT->setBaudRate(QSerialPort::Baud9600);
    serial_AT->setPortName(portAT);
    serial_AT->setDataBits(QSerialPort::Data8);
    serial_AT->setParity(QSerialPort::NoParity);
    if (false == serial_AT->open(QIODevice::ReadWrite)) {
        qDebug() << "Serial port AT failed...";
        exit(-1);
    }

    QByteArray response;
    int counter = 1;
    while (1) {
        if (counter >= 3) {
            qDebug() << "No OK response for AT..Exiting";
            exit(-1);
        }
        qDebug() << "AT\r";
        serial_AT->write("AT\r");
        if (serial_AT->waitForReadyRead(100)) {
            response = serial_AT->readAll();
            if ("\r\nOK\r\n" == response)
                break;
            else
                counter++;
        } else {
            qDebug() << "Time out";
            counter++;
        }
        QThread::msleep(1000);
    }

    counter = 1;
    while (1) {
        response.clear();
        if (counter >= 3) {
            qDebug() << "No OK response for AT+CGPSPWR=1..Exiting";
            exit(-1);
        }
        qDebug() << "AT+CGPSPWR=1\r";
        serial_AT->write("AT+CGPSPWR=1\r");
        if (serial_AT->waitForReadyRead(100)) {
            response = serial_AT->readAll();
            if ("\r\nOK\r\n" == response)
                break;
            else
                counter++;
        } else {
            qDebug() << "Time out";
            counter++;
        }
        QThread::msleep(1000);
    }

    counter = 1;
    while (1) {
        response.clear();
        if (counter >= 3) {
            qDebug() << "No response for AT+CGPSPWR?..Exiting";
            exit(-1);
        }
        qDebug() << "AT+CGPSPWR?\r";
        serial_AT->write("AT+CGPSPWR?\r");
        if (serial_AT->waitForReadyRead(100)) {
            response = serial_AT->readAll();
            qDebug() << response;
            if ("\r\n+CGPSPWR: 1\r\n\r\nOK\r\n" == response) {
                ui->edit_gnssPower->setText(QString("ON"));
                break;
            }
            else
                counter++;
        } else {
            qDebug() << "Time out";
            counter++;
        }
        QThread::msleep(1000);
    }
    counter = 1;

    queryGPSData = new QTimer(this);
    queryGPSData->setTimerType(Qt::PreciseTimer);
    queryGPSData->start(1000);
    connect(queryGPSData, &QTimer::timeout, this, &MainWindow::sendQueryToLTEModule);

    connect (serial_AT, &QSerialPort::readyRead, this, &MainWindow::processNMEAResponse);

}

MainWindow::~MainWindow()
{
    delete queryGPSData;
    delete serial_AT;
    delete ui;
}

int MainWindow::delay_s(int sec)
{
    int ret = 0;
    // converting time into milli seconds
    int msec = 1000 * sec;

    clock_t start = clock();

    // looping till required time is not achieved
    while(clock() < start + msec) {;}
    ret = 1;
    return ret;
}

int MainWindow::delay_ms(int msec)
{
    clock_t start = clock();

    // looping till required time is not achieved
    while(clock() < start + msec) {;}
    return 1;
}

void MainWindow::processNMEAResponse()
{
    QByteArray datagram = serial_AT->readAll();
    qDebug() << datagram;
    char *line = datagram.data();
    //"\r\n+CGNSINF: 1,1,20190727032055.00,37.335236,-121.888412,30.200,0.00,83.1,1,,1.5,1.8,1.0,,12,4,,,24,,\r\n\r\nOK\r\n"
    at_cgnsinf_t gnss_data;
    memset(&gnss_data, 0, sizeof(at_cgnsinf_t));
    if (sscanf(line, "\r\n+CGNSINF: %[^,],%[^,],%[^,],%[^,],%[^,]",
               gnss_data.runStatus,
               gnss_data.fixStatus,
               gnss_data.utcDateTime,
               gnss_data.latitude,
               gnss_data.longitude))
    {
        if ('1' == gnss_data.fixStatus[0]) {
            ui->checkBox_fixStatus->setCheckState(Qt::Checked);
            double lat1 = atof(gnss_data.latitude);
            double lon1 = atof(gnss_data.longitude);
            ui->edit_lat->setText(QString::number(lat1));
            ui->edit_lon->setText(QString::number(lon1));
        }
        else {
            ui->checkBox_fixStatus->setCheckState(Qt::Unchecked);
            ui->edit_lat->clear();
            ui->edit_lon->clear();
        }
    }
}

void MainWindow::sendQueryToLTEModule()
{
    serial_AT->write("AT+CGNSINF\r");
}

#if 0
char line1[] = "\r\nAT+CGNSINF: "
               "1,"
               "1,"
               "20190727032055.000,"
               "37.335236,"
               "-121.888412,"
               "30.200,"
               "0.00,"
               "83.1,"
               "1,"
               ","
               "1.5,"
               "1.8,"
               "1.0,"
               ","
               "12,"
               "4,"
               ","
               ","
               "24,"
               ","
               "";
#endif
