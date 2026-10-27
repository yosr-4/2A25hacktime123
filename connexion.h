#ifndef CONNEXION_H
#define CONNEXION_H

#include <QString>

// ---------------------------------------------------------------
//  Connexion à la base commune HackTime.
//  1) On essaie d'abord MySQL / MariaDB (paramètres ci-dessous).
//  2) Si le serveur n'est pas joignable, on bascule automatiquement
//     sur une base SQLite locale (créée + remplie d'exemples) pour
//     que l'application tourne toujours dans Qt Creator.
//
//  >>> Adaptez les constantes de connexion.cpp à votre base commune.
// ---------------------------------------------------------------
class Connexion
{
public:
    static bool open(QString *message = nullptr);
    static bool isSqliteFallback();

private:
    static void createSqliteSchema();
    static bool s_sqlite;
};

#endif // CONNEXION_H
