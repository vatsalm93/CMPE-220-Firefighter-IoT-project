#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QUdpSocket>

class Network : public QObject
{
    Q_OBJECT
public:
    explicit Network(QObject *parent = nullptr);

signals:
    void audioPacket(QByteArray audio_packet);

public slots:
    void chunkAudioOutput_ready(QByteArray chunk);
    //  void restart_receiving();
    //  void stop_receiving();
    void recButton_pressed();
    void recButton_released();

private slots:
    void incomingPackets();

private:
    enum state {LISTENING, SENDING};
    state current_state;
    QUdpSocket* socket;
    QHostAddress ipv4Broad;
    QHostAddress ipv4Local;
};

#endif // NETWORK_H
