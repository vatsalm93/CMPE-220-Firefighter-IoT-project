#include "network.h"
#include <QDebug>
#include <QDataStream>
#include <QNetworkInterface>

const quint16 port = 5824;

Network::Network(QObject *parent) : QObject(parent)
{
    current_state = LISTENING;

    // Instantiating the socket
    socket = new QUdpSocket(this);
//    socket->bind(port,QUdpSocket::ShareAddress);
    socket->bind(QHostAddress::Any, 65529);
    connect(socket,SIGNAL(readyRead()),this,SLOT(incomingPackets()));

    QNetworkInterface *inter=new QNetworkInterface();
    QList<QNetworkInterface> listInterNames = inter->allInterfaces();

    // Getting to know which interface is wifi
    int wifNum = 0;
    for (int i = 0; i < listInterNames.size(); ++i)
      {
        if (listInterNames.at(i).name() == "wlan0") {
          wifNum =  i;
        }
      }

    // Wifi interface
    QList<QNetworkAddressEntry> wifiInter = listInterNames.at(wifNum).addressEntries();

    ipv4Broad = wifiInter.at(0).broadcast();
    ipv4Local= wifiInter.at(0).ip();
}

/*
 *This slot is triggered by a signal in the audio module
 *every time a chunk of audio data is recorded from the microphone
 */
void Network::chunkAudioOutput_ready(QByteArray chunk)
{
    qDebug() << "NTW_FAC: sending output buffer.";
//    socket->writeDatagram(chunk.data(),chunk.size(),ipv4Broad,5824);
    socket->writeDatagram(chunk.data(),chunk.size(),QHostAddress("10.0.0.124"),65530);
}

/*
 *slot for catching recButton pressed event emited by the UI
 */
void Network::recButton_pressed()
{
    current_state = SENDING;
}

/*
 *slot for catching recButton released event emited by the UI
 */
void Network::recButton_released()
{
    current_state = LISTENING;
}

/*
 *Emits a signal containing data everytime a packet is received */
void Network::incomingPackets()
{
    QByteArray datagram;
    do {
      datagram.resize(socket->pendingDatagramSize());
      QHostAddress sender;
      quint16 senderPort;
      socket->readDatagram(datagram.data(), datagram.size(),&sender,&senderPort);

      //processes datagram if sender is not localhost and state is LISTENING
//      if ( ( current_state == LISTENING ) &&  (sender != ipv4Local) )
      if (current_state == LISTENING)
        emit audioPacket(datagram);
      datagram.clear();
    } while (socket->hasPendingDatagrams());
}
