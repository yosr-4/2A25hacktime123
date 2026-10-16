#include "connection.h"

Connection::Connection() {}

bool Connection::createconnect()
{
    db = QSqlDatabase::addDatabase("QOCI"); // Driver Oracle
    db.setDatabaseName("xe");               // Nom de la base Oracle
    db.setUserName("system");             // Nom d'utilisateur
    db.setPassword("oracle");             // Mot de passe

    if (db.open()) {
        return true;
    } else {
        qDebug() << "Erreur de connexion BDD :" << db.lastError().text();
        return false;
    }
}

void Connection::closeConnection()
{
    db.close();
}