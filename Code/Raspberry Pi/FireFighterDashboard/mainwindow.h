#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QIODevice>
#include <QAudioInput>
#include <QAudioOutput>
#include <QUdpSocket>
#include <QAudioDeviceInfo>
#include <QDate>
#include <QTime>
#include <QFile>

typedef struct{
    char gps[4];
    double latitude;
    double longitude;
    char aq[3];
    double air_quality;
    char temp[5];
    double temperature;
    char beats[5];
    uint8_t bpm[10];
} firefighter_sensors_t;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void check_net_connectivity(void);

private slots:

    void on_pushButton_Exit_clicked();

    void on_pushButton_PTT_pressed();

    void on_pushButton_PTT_released();

    void recordMessage();

    void getSystemTime();

    void processIncomingPacket(void);

    void processSensorData(void);

private:
    Ui::MainWindow *ui;
    QAudioInput *microphone;
    QAudioOutput *speaker;
    QAudioFormat m_format;

    QAudioDeviceInfo m_inputDeviceInfo;
    QAudioDeviceInfo m_outputDeviceInfo;

    QIODevice *recorder;
    QIODevice *listen;

    QByteArray m_buffer;
    int volume;

    QUdpSocket *socket;
    QUdpSocket *socket_firefighter_data;

    QDate date;
    QTime time;

    void initializeSocket(const quint16 &port);
    void initializeSensorSocket(const quint16 &port);
    void initializeAudio(void);
    void createAudioInput(void);
    void createAudioOutput(void);

    void processSoundSamples(short **outData, QByteArray buffer, qint64 payload_length);

    int applyVolumeToSamples(const short &iSamples);

    void initializeDateTime(int msec);

//    void logDataIntoFile(const QString &dateTime,
//                         double aq,
//                         double temp,
//                         double lat,
//                         double lon, QString bpm );
    void logDataIntoFile(const QString &dateTime);

    QString hostIP;
    quint16 hostPortNum;
    QFile firefighterLogs;
    QString sysDateTime;
};

#endif // MAINWINDOW_H
