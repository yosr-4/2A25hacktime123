#include "connection.h"
#include <QDebug>

// Oracle ODBC DSN configured on the development machine.
static const char *SOURCE = "HackathonXE";
static const char *UTILISATEUR = "hackathon";
static const char *MOT_DE_PASSE = "hackathon26";

Connection::Connection()
    : db(QSqlDatabase::addDatabase("QODBC"))
{
}

Connection::~Connection()
{
    if (db.isOpen())
        db.close();
    if (QSqlDatabase::contains("hackathon_connection")) {
        QSqlDatabase named = QSqlDatabase::database("hackathon_connection", false);
        if (named.isOpen()) named.close();
    }
}

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

    if (!db.isOpen() && !db.open()) {
        qDebug() << "Oracle connection failed:" << db.lastError().text();
        return false;
    }

    // The teams CRUD module explicitly requests this named connection.
    if (!QSqlDatabase::contains("hackathon_connection")) {
        QSqlDatabase teamsDb = QSqlDatabase::cloneDatabase(db, "hackathon_connection");
        if (!teamsDb.open()) {
            qDebug() << "Teams Oracle connection failed:" << teamsDb.lastError().text();
            return false;
        }
    } else {
        QSqlDatabase teamsDb = QSqlDatabase::database("hackathon_connection");
        if (!teamsDb.isOpen() && !teamsDb.open()) {
            qDebug() << "Teams Oracle connection failed:" << teamsDb.lastError().text();
            return false;
        }
    }
    qDebug() << "Oracle connections ready (default + hackathon_connection).";
    return true;
}

QString Connection::lastError() const
{
    return db.lastError().text();
}
