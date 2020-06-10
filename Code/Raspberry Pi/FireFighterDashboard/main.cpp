#include "loading.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth>
#include <QBluetoothSocket>
#include <QBluetoothLocalDevice>

loading *loadForm;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    qDebug() << system("trust 20:17:07:73:45:61");
//    qDebug() << system("connect 20:17:07:73:45:61");
#if 0
    QBluetoothLocalDevice localDevice;
    QString localDeviceName;

    if (localDevice.isValid()) {
        // Turn bluetooth on
        localDevice.powerOn();

        // Read local device name
        localDeviceName = localDevice.name();

        qDebug() << localDeviceName;

        // Make it visible to others
        localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

        qDebug() << localDevice.connectedDevices();
    }

    QBluetoothDeviceDiscoveryAgent *discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    QBluetoothSocket *bleSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    QString deviceName;
    QString deviceAddress;
    QObject::connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                     new QObject(),
                     [&deviceAddress, &deviceName, &discoveryAgent](const QBluetoothDeviceInfo &device) {
        qDebug() << "Device name: " << device.name();
        qDebug() << "MAC: " << device.address();
        if (device.name() == "Rockerz 250") {
            deviceName = device.name();
            deviceAddress = device.address().toString();
            discoveryAgent->stop();
            qDebug() << device.serviceUuids();
        }
    });

    // Stop after 5000 mS
    discoveryAgent->setLowEnergyDiscoveryTimeout(5000);
    // Start the discovery process
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    static const QString serviceUuid(QStringLiteral("00001108-0000-1000-8000-00805f9b34fb"));
    bleSocket->connectToService(QBluetoothAddress(deviceAddress), QBluetoothUuid(serviceUuid),
                                                  QIODevice::ReadWrite);
    qDebug() << "Connected";

    system("paplay /home/pi/test.wav");
#endif

    loadForm = new loading();
    loadForm->showFullScreen();
    return a.exec();
}
