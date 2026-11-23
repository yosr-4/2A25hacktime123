// =====================================================================
//  Classe Arduino : liaison série (USB) entre l'application Qt et la carte Arduino.
//  Classe commune (comme Connection), utilisée par le scénario Arduino.
// =====================================================================
#ifndef ARDUINO_H
#define ARDUINO_H

#include <QByteArray>
#include <QObject>
#include <QString>

#ifndef SANS_SERIALPORT
#include <QSerialPort>
#else
// Qt Serial Port non installé : classe vide qui permet de compiler sans la borne Arduino.
class QSerialPort : public QObject
{
    Q_OBJECT
public:
    bool isOpen() const { return false; }
signals:
    void readyRead();
};
#endif

class Arduino
{
public:
    Arduino();
    ~Arduino();

    int connect_arduino();                    // 0 = connecté, 1 = port non ouvert, -1 = carte absente
    int close_arduino();
    int write_to_arduino(const QByteArray &d);
    QByteArray read_from_arduino();
    QSerialPort *getserial() { return serial; }
    QString getarduino_port_name() const { return arduino_port_name; }
    bool estConnecte() const { return serial->isOpen(); }

private:
    QSerialPort *serial;
    QString arduino_port_name;
    bool arduino_is_available = false;
};

#endif // ARDUINO_H
