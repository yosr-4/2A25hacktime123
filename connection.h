#ifndef CONNECTION_H
#define CONNECTION_H

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>

/* Classe Connection (atelier « Connexion Qt/Oracle - patron Singleton »)
 * Une seule connexion à la base pour toute l'application (Meyers Singleton).
 * - createInstance() : retourne l'instance unique
 * - createConnection() : configure et ouvre la base
 */
class Connection {
public:
    static Connection &createInstance();   // retourne une référence vers l'instance unique
    bool createConnection();                // établit la connexion à la base
    QString lastError() const;              // message de la dernière erreur

private:
    QSqlDatabase db;   // connexion persistante, gardée par le Singleton

    Connection();      // constructeur privé : personne ne peut créer d'autre instance
    ~Connection();     // destructeur privé : ferme proprement la connexion

    // copie et affectation interdites : l'instance reste unique
    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;
};

#endif // CONNECTION_H
