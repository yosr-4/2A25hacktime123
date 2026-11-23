#include "arduino.h"

#include <QDebug>

#ifdef SANS_SERIALPORT
// ----- Version sans module Serial Port : la borne Arduino est simplement "non connectée".
Arduino::Arduino() : serial(new QSerialPort) {}
Arduino::~Arduino() { delete serial; }
int Arduino::connect_arduino() { qDebug() << "Arduino : module Serial Port absent"; return -1; }
int Arduino::close_arduino() { return 1; }
int Arduino::write_to_arduino(const QByteArray &) { return 1; }
QByteArray Arduino::read_from_arduino() { return QByteArray(); }
#else
#include <QSerialPortInfo>

// identifiants USB : Arduino Uno d'origine (0x2341) et clones à puce CH340 (0x1A86)
static bool estUneCarteArduino(const QSerialPortInfo &port)
{
    if (!port.hasVendorIdentifier()) return false;
    const quint16 vendeur = port.vendorIdentifier();
    return vendeur == 0x2341 || vendeur == 0x2A03 || vendeur == 0x1A86;
}

Arduino::Arduino()
    : serial(new QSerialPort)
{
}

Arduino::~Arduino()
{
    close_arduino();
    delete serial;
}

int Arduino::connect_arduino()
{
    if (serial->isOpen()) return 0;               // déjà connecté
    arduino_is_available = false;
    // on essaie chaque carte Arduino branchée : si l'une est déjà utilisée (autre scénario),
    // on passe à la suivante au lieu d'abandonner
    for (const QSerialPortInfo &port : QSerialPortInfo::availablePorts()) {
        if (!estUneCarteArduino(port)) continue;
        arduino_is_available = true;
        serial->setPortName(port.portName());
        if (!serial->open(QSerialPort::ReadWrite)) continue;
        arduino_port_name = port.portName();
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        qDebug() << "Arduino :" << arduino_port_name;
        return 0;
    }
    qDebug() << "Arduino :" << (arduino_is_available ? QString("port occupé") : QString("non trouvé"));
    return arduino_is_available ? 1 : -1;
}

int Arduino::close_arduino()
{
    if (serial->isOpen()) {
        serial->close();
        return 0;
    }
    return 1;
}

int Arduino::write_to_arduino(const QByteArray &d)
{
    if (!serial->isWritable()) {
        qDebug() << "Arduino : écriture impossible";
        return 1;
    }
    serial->write(d);
    return 0;
}

QByteArray Arduino::read_from_arduino()
{
    return serial->isReadable() ? serial->readAll() : QByteArray();
}
#endif // SANS_SERIALPORT
