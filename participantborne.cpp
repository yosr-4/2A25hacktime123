#include "participantborne.h"

BornePointage::BornePointage(QObject *parent)
    : QObject(parent)
{
}

bool BornePointage::connecter()
{
    if (m_arduino.estConnecte()) return true;
    const int r = m_arduino.connect_arduino();
    if (r != 0) {
        m_etat = (r == -1) ? "Borne non branchée" : "Borne : port occupé";
        return false;
    }
    if (!m_signalBranche) {
        connect(m_arduino.getserial(), &QSerialPort::readyRead, this, &BornePointage::lireSerie);
        m_signalBranche = true;
    }
    m_etat = "Borne connectée (" + m_arduino.getarduino_port_name() + ")";
    return true;
}

void BornePointage::repondre(ResultatPointage r)
{
    char code = 'R';
    if (r == ResultatPointage::Accepte) code = 'O';
    else if (r == ResultatPointage::DejaPointe) code = 'D';
    m_dernierEnvoi = QByteArray(1, code) + "\n";
    if (m_arduino.estConnecte())
        m_arduino.write_to_arduino(m_dernierEnvoi);
}

void BornePointage::lireSerie()
{
    recevoir(m_arduino.read_from_arduino());
}

void BornePointage::recevoir(const QByteArray &octets)
{
    m_tampon += octets;
    int fin;
    while ((fin = m_tampon.indexOf('\n')) >= 0) {
        const QString ligne = QString::fromLatin1(m_tampon.left(fin)).trimmed();
        m_tampon.remove(0, fin + 1);
        if (ligne == "V") {
            emit entrerDemande();
        } else if (ligne.startsWith("P|")) {
            bool ok = false;
            const int valeur = ligne.mid(2).toInt(&ok);
            if (ok) emit positionChangee(qBound(0, valeur, 1023) / 1023.0);
        }
    }
}
