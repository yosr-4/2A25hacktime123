#include "connection.h"

// ===== Paramètres de connexion : à remplacer par les tiens =====
// SOURCE       = source de données ODBC 64 bits (créée dans « Sources de données ODBC (64 bits) »)
// UTILISATEUR  = utilisateur Oracle créé pour le projet
// MOT_DE_PASSE = son mot de passe
static const char *SOURCE = "Source_Projet2A";
static const char *UTILISATEUR = "hackathon";
static const char *MOT_DE_PASSE = "hackathon26";

// Le pilote ODBC est choisi une seule fois, dans le constructeur du Singleton.
Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

// Ferme la connexion si elle est encore ouverte à la fin du programme.
Connection::~Connection()
{
    if (db.isOpen())
        db.close();
}

// Meyers Singleton : instance statique locale, créée au premier appel,
// vivante pendant toute la durée du programme.
Connection &Connection::createInstance()
{
    static Connection instance;
    return instance;
}

bool Connection::createConnection()
{
    db.setDatabaseName(SOURCE);
    db.setUserName(UTILISATEUR);
    db.setPassword(MOT_DE_PASSE);

    if (db.open()) {
        qDebug() << "Connexion établie";
        return true;
    }
    qDebug() << "Échec de la connexion :" << db.lastError().text();
    return false;
}

QString Connection::lastError() const
{
    return db.lastError().text();
}
