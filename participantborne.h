#ifndef PARTICIPANTBORNE_H
#define PARTICIPANTBORNE_H

// Scénario Arduino du module Participants : « Barrière d'accès / pointage des participants ».
// Cette classe parle à la carte (classe Arduino commune) ; la décision d'entrée est prise
// dans Participant::pointer() (base Oracle). Programme de la carte : arduino/barriere_acces.
//
//  Arduino -> Qt : "P|0..1023" (potentiomètre), "V" (bouton ENTRER)
//  Qt -> Arduino : "O" (accepté), "R" (refusé), "D" (déjà pointé)

#include <QObject>
#include <QByteArray>
#include <QString>
#include "arduino.h"
#include "participant.h"

class BornePointage : public QObject
{
    Q_OBJECT

public:
    explicit BornePointage(QObject *parent = nullptr);

    bool connecter();                       // à rappeler tant que la carte n'est pas connectée
    bool estConnecte() const { return m_arduino.estConnecte(); }
    QString etat() const { return m_etat; } // texte pour l'interface
    void repondre(ResultatPointage r);      // envoie O / R / D à la carte
    void recevoir(const QByteArray &octets);// analyse les octets reçus (une ligne = une commande)
    QByteArray dernierEnvoi() const { return m_dernierEnvoi; }

signals:
    void positionChangee(double fraction);  // potentiomètre : 0.0 (gauche) à 1.0 (droite)
    void entrerDemande();                   // bouton ENTRER appuyé

private:
    void lireSerie();
    Arduino m_arduino;
    QByteArray m_tampon;
    QByteArray m_dernierEnvoi;
    QString m_etat = "Borne non connectée";
    bool m_signalBranche = false;
};

#endif // PARTICIPANTBORNE_H
