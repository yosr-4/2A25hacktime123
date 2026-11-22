#include "arduino.h"

#include <QDebug>
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
    arduino_is_available = false;
    for (const QSerialPortInfo &port : QSerialPortInfo::availablePorts()) {
        if (estUneCarteArduino(port)) {
            arduino_is_available = true;
            arduino_port_name = port.portName();
            break;
        }
    }
    qDebug() << "Arduino :" << (arduino_is_available ? arduino_port_name : QString("non trouvé"));
    if (!arduino_is_available) return -1;

    serial->setPortName(arduino_port_name);
    if (!serial->open(QSerialPort::ReadWrite)) return 1;
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    return 0;
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
