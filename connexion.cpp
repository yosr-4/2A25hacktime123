#include "connexion.h"

#include <QCoreApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

// ======= PARAMÈTRES DE LA BASE COMMUNE (à adapter) =======
static const char *DB_DRIVER = "QMYSQL";      // ou "QODBC" si vous utilisez une source ODBC
static const char *DB_HOST   = "127.0.0.1";
static const int   DB_PORT   = 3306;
static const char *DB_NAME   = "hacktime";
static const char *DB_USER   = "root";
static const char *DB_PASS   = "";
// =========================================================

bool Connexion::s_sqlite = false;

bool Connexion::isSqliteFallback() { return s_sqlite; }

// Tentative de connexion à la base commune MySQL.
static bool tryMysql(QString *why)
{
    if (!QSqlDatabase::isDriverAvailable(DB_DRIVER)) {
        *why = QString("pilote %1 absent").arg(DB_DRIVER);
        return false;
    }
    bool ok = false;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(DB_DRIVER);
        db.setHostName(DB_HOST);
        db.setPort(DB_PORT);
        db.setDatabaseName(DB_NAME);
        db.setUserName(DB_USER);
        db.setPassword(DB_PASS);
        db.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=3");
        ok = db.open();
        if (!ok) *why = db.lastError().text();
    }
    if (!ok) QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    return ok;
}

bool Connexion::open(QString *message)
{
    // --- 1) Base commune (MySQL) ---
    QString why;
    if (tryMysql(&why)) {
        s_sqlite = false;
        if (message) *message = QString("Connecté à la base « %1 » (%2).").arg(DB_NAME, DB_HOST);
        return true;
    }

    // --- 2) Repli SQLite locale ---
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        if (message) *message = "Connexion impossible : " + why;
        return false;
    }
    QSqlDatabase lite = QSqlDatabase::addDatabase("QSQLITE");
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    lite.setDatabaseName(dir + "/hacktime_local.db");
    if (!lite.open()) {
        if (message) *message = "Connexion impossible : " + lite.lastError().text();
        return false;
    }
    s_sqlite = true;
    createSqliteSchema();
    if (message) *message = "MySQL indisponible (" + why + ") → base locale SQLite utilisée.";
    return true;
}

// Schéma identique au MLD du projet + données d'exemple.
void Connexion::createSqliteSchema()
{
    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS Employe ("
           "id_employe INTEGER PRIMARY KEY AUTOINCREMENT, nom TEXT, prenom TEXT, email TEXT,"
           "role TEXT, login TEXT, mdp TEXT)");
    q.exec("CREATE TABLE IF NOT EXISTS Competition ("
           "id_competition INTEGER PRIMARY KEY AUTOINCREMENT, nom TEXT, theme TEXT,"
           "date_debut TEXT, date_fin TEXT, nb_max_membres INTEGER, statut TEXT, mode TEXT,"
           "id_employe INTEGER REFERENCES Employe(id_employe))");
    q.exec("CREATE TABLE IF NOT EXISTS Atelier ("
           "id_atelier INTEGER PRIMARY KEY AUTOINCREMENT, titre TEXT NOT NULL, description TEXT,"
           "date_atelier TEXT, duree INTEGER, salle TEXT, capacite INTEGER,"
           "id_competition INTEGER REFERENCES Competition(id_competition))");
    q.exec("CREATE TABLE IF NOT EXISTS Equipe ("
           "id_equipe INTEGER PRIMARY KEY AUTOINCREMENT, nom_equipe TEXT, date_inscription TEXT,"
           "id_competition INTEGER REFERENCES Competition(id_competition))");
    q.exec("CREATE TABLE IF NOT EXISTS Participant ("
           "id_participant INTEGER PRIMARY KEY AUTOINCREMENT, nom TEXT, prenom TEXT, email TEXT,"
           "telephone TEXT, id_equipe INTEGER REFERENCES Equipe(id_equipe))");

    q.exec("SELECT COUNT(*) FROM Competition");
    if (q.next() && q.value(0).toInt() > 0) return;

    q.exec("INSERT INTO Employe (nom,prenom,email,role,login,mdp) VALUES ('Admin','HackTime','admin@hacktime.tn','admin','admin','admin')");
    q.exec("INSERT INTO Competition (nom,theme,date_debut,date_fin,nb_max_membres,statut,mode,id_employe) VALUES "
           "('HackTime 2026','Intelligence artificielle','2026-10-15','2026-10-17',5,'En cours','Présentiel',1),"
           "('Green Hack','Environnement','2026-11-20','2026-11-22',4,'Planifiée','Hybride',1),"
           "('CyberChallenge','Cybersécurité','2026-06-05','2026-06-07',6,'Terminée','En ligne',1)");
    q.exec("INSERT INTO Atelier (titre,description,date_atelier,duree,salle,capacite,id_competition) VALUES "
           "('Workshop IA & ML','Initiation aux modèles de machine learning.','2026-10-20 10:00:00',120,'Salle 204',30,1),"
           "('Git & collaboration','Bonnes pratiques de travail en équipe.','2026-10-20 14:00:00',90,'Salle 101',40,1),"
           "('Pitch en 3 minutes','Savoir présenter son projet au jury.','2026-10-27 09:30:00',60,'Salle 204',25,1),"
           "('Éco-conception logicielle','Réduire l''empreinte de vos applications.','2026-11-18 10:00:00',120,'Salle 305',35,2),"
           "('Capteurs & IoT','Prototypage rapide avec Arduino.','2026-11-19 15:00:00',180,'Labo 1',20,2),"
           "('Introduction au pentest','Les bases de la sécurité offensive.','2026-06-03 10:00:00',150,'Salle 204',30,3),"
           "('Cryptographie appliquée','Chiffrement symétrique et asymétrique.','2026-06-04 14:00:00',90,'Salle 101',45,3)");
}
