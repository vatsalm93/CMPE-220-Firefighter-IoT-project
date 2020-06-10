#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include "loading.h"
#include <QFile>
#include "process.h"

extern Process *net_status;
extern loading *loadForm;
extern MainWindow *w;

const int BufferSize = 14096;
const quint16 myPort = 9998;
const quint16 mySensorPort = 9997;

static firefighter_sensors_t *sdata;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lineEdit_Heartbeat->setReadOnly(true);
    ui->lineEdit_Air_Quality->setReadOnly(true);
    ui->lineEdit_Temperature->setReadOnly(true);
    ui->lineEdit_lat->setReadOnly(true);
    ui->lineEdit_lon->setReadOnly(true);

    m_inputDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    m_outputDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();

    microphone = nullptr;
    speaker = nullptr;
    volume = 100;

    m_buffer = QByteArray(BufferSize, 0);

    QFile file("../FireFighterDashboard/Files/ethernet.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "File open failed";
        exit(-1);
    }

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        qDebug() << line;
        hostIP = QString(line);

        line = file.readLine();
        hostPortNum = QString(line).toUShort();
    }

    if (file.isOpen()) {
        qDebug() << "Closing file";
        file.close();
    }

    firefighterLogs.setFileName("../FireFighterDashboard/Logs/firefighterlogs.txt");
    if (!firefighterLogs.open(QIODevice::WriteOnly)) {
        qDebug() << "Error: Unable to open firefighterlogs.txt";
        exit(-1);
    }
    qDebug() << "File firefighterlogs.txt opened successfully for logging data";

    initializeSocket(myPort);
    initializeSensorSocket(mySensorPort);
    initializeAudio();
    initializeDateTime(1);
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow form destructor invoked";
    delete socket;
    delete microphone;
    delete speaker;
    delete ui;
    delete loadForm;
}

void MainWindow::check_net_connectivity()
{
    int net_code = system("ping -c 1 -w 1 www.google.com 2>&1 >/dev/null");
    if (net_code == 0) {
        ui->lblNetConnectivity->setStyleSheet("image: url(:/Images/active_connection.png);");
        this->setDisabled(false);
    } else {
        ui->lblNetConnectivity->setStyleSheet("image: url(:/Images/inactive_connection.png);");
        this->setDisabled(true);
    }
}

void MainWindow::on_pushButton_Exit_clicked()
{
    firefighterLogs.close();
    net_status->stop = false;
    delete w;
//    delete loadForm;
}

void MainWindow::on_pushButton_PTT_pressed()
{
    recorder = microphone->start();
    recorder->open(QIODevice::ReadWrite);

    connect(recorder, SIGNAL(readyRead()), this, SLOT(recordMessage()));
}

void MainWindow::on_pushButton_PTT_released()
{
    microphone->stop();
}

void MainWindow::recordMessage()
{
    if (microphone == nullptr) {
        return;
    }

    //Check the number of samples in input buffer
    qint64 len = microphone->bytesReady();

    //Limit sample size
    if (4096 < len) {
        len = 4096;
    }

    //Read sound samples from microphone to buffer
    qint64 l = recorder->read(m_buffer.data(), len);
    if (0 < l) {
        short *outputPayLoad = nullptr;
        processSoundSamples(&outputPayLoad, m_buffer, len);
        socket->writeDatagram(reinterpret_cast<char *>(outputPayLoad), len, QHostAddress(hostIP), hostPortNum);
    }
}

void MainWindow::initializeAudio()
{
    m_format.setSampleRate(8000);
    m_format.setChannelCount(1);
    m_format.setSampleSize(16);
    m_format.setSampleType(QAudioFormat::SignedInt);
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setCodec(QString("audio/pcm"));

    QAudioDeviceInfo infoIn(m_inputDeviceInfo);
    if (!infoIn.isFormatSupported(m_format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        m_format = infoIn.nearestFormat(m_format);
    }

    QAudioDeviceInfo infoOut(m_outputDeviceInfo);
    if (!infoOut.isFormatSupported(m_format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        m_format = infoOut.nearestFormat(m_format);
    }
    QList<QAudioDeviceInfo> list = infoIn.availableDevices(QAudio::AudioInput);
    for (auto it = list.cbegin(); it != list.cend(); it++) {
        qDebug() << it->deviceName();
    }

    qDebug() << infoIn.supportedCodecs();
    qDebug() << infoIn.deviceName();
    qDebug() << infoIn.preferredFormat();
    qDebug() << infoIn.supportedByteOrders();
    qDebug() << infoIn.supportedSampleRates();
    qDebug() << infoIn.supportedSampleSizes();
    qDebug() << infoIn.supportedSampleTypes();
    qDebug() << infoIn.supportedChannelCounts();

    qDebug() << "Default output device->" << infoOut.deviceName();

    createAudioInput();
    createAudioOutput();
}

void MainWindow::createAudioInput()
{
    microphone = new QAudioInput(m_inputDeviceInfo, m_format, this);
}

void MainWindow::createAudioOutput()
{
    speaker = new QAudioOutput(m_outputDeviceInfo, m_format, this);
}

void MainWindow::processSoundSamples(short **outData, QByteArray buffer, qint64 payload_length)
{
    //Assign sound samples to short array
    short *resultingData = reinterpret_cast<short *>(buffer.data());
    short *temp = resultingData;
    temp[0] = resultingData[0];
    int i;
    //Remove noise using low pass filter algorithm. (Simple algorithm used to remove noise)
    for (i = 1; i < payload_length; i++) {
        temp[i] = static_cast<short>(0.333 * (resultingData[i]) + (1.0 - 0.333) * (temp[i - 1]));
    }

    //Change volume of each samples
    for (i = 0; i < payload_length; i++) {
        temp[i] = static_cast<short>(applyVolumeToSamples(temp[i]));
    }
    *outData = temp;
}

int MainWindow::applyVolumeToSamples(const short &iSamples)
{
    return std::max(std::min(((iSamples * volume) / 50), 35535), -35535);
}

void MainWindow::initializeDateTime(int msec)
{
    QTimer *updateTime = new QTimer(this);
    connect(updateTime,SIGNAL(timeout()),this,SLOT(getSystemTime()));
    updateTime->setTimerType(Qt::PreciseTimer);
    updateTime->start(msec);
}

void MainWindow::logDataIntoFile(const QString &dateTime)
{
    QString logs;
    QString bpm = QString::fromUtf8((char *)&sdata->bpm);
    logs = QString::number(sdata->air_quality)
            + "," + QString::number(sdata->temperature)
            + "," + QString::number(sdata->latitude)
            + "," + QString::number(sdata->longitude)
            + '\n';

    if (firefighterLogs.write(logs.toLocal8Bit())) {
        qDebug() << "Data logging successfull....";
    }
}

void MainWindow::getSystemTime()
{
    date                    = QDate::currentDate();
    time                    = QTime::currentTime();
    QString dateTime;
    sysDateTime.append(date.toString("MM/dd/yyyy")).append(",").append(time.toString("hh:mm:ss"));
    dateTime.append(date.toString("MM/dd/yyyy")).append("\n").append(time.toString("hh:mm:ss"));
    ui->lblSysDate->setText(dateTime);
}

void MainWindow::processIncomingPacket()
{
    QByteArray datagram;
    do {
        datagram.resize(static_cast<int>(socket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;
        qint64 bytesRead = socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        qDebug() << "Receiving";
        if (speaker->state() == QAudio::StoppedState) {
            listen = speaker->start();
        }

        //write received sample to output device for playback audio
        listen->write(datagram.data(), bytesRead);
    } while (socket->hasPendingDatagrams());
}

void MainWindow::processSensorData()
{

    while (socket_firefighter_data->hasPendingDatagrams()) {
        QByteArray sensorFrame;
        sensorFrame.resize(static_cast<int>(socket_firefighter_data->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;
        qint64 bytesRead = socket_firefighter_data->readDatagram(sensorFrame.data(), sensorFrame.size(), &sender, &senderPort);

        sdata = reinterpret_cast<firefighter_sensors_t *>(sensorFrame.data());
        qDebug() << "Air Quality -> " << sdata->air_quality;
        qDebug() << "Temperature -> " << sdata->temperature;
        qDebug() << "Heart Rate -> " << sdata->bpm;
        qDebug() << "Lat -> " << sdata->latitude;
        qDebug() << "Lon -> " << sdata->longitude;
        logDataIntoFile(sysDateTime);
        ui->lineEdit_Air_Quality->setText(QString::number(sdata->air_quality));
        ui->lineEdit_Temperature->setText(QString::number(sdata->temperature));
        ui->lineEdit_lat->setText(QString::number(sdata->latitude));
        ui->lineEdit_lon->setText(QString::number(sdata->longitude));
        ui->lineEdit_Heartbeat->setText(QString::fromUtf8((char *)&sdata->bpm));
        sensorFrame.clear();
    }
}

void MainWindow::initializeSocket(const quint16 &port)
{
    socket = new QUdpSocket(this);
    if (true == socket->bind(QHostAddress::Any, port)) {
        connect(socket, SIGNAL(readyRead()), this, SLOT(processIncomingPacket()));
    } else {
        qDebug() << "Error: Socket binding failed!";
        exit(-1);
    }
}

void MainWindow::initializeSensorSocket(const quint16 &port)
{
    socket_firefighter_data = new QUdpSocket(this);
    if (true == socket_firefighter_data->bind(QHostAddress::Any, port)) {
        connect(socket_firefighter_data, SIGNAL(readyRead()), this, SLOT(processSensorData()));
    } else {
        qDebug() << "Error: Sensor Socket binding failed!";
        exit(-1);
    }
}
