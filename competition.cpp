#include "competition.h"

#include <QDebug>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

static QString lastError;

// exécute la requête ; en cas d'échec, garde le message d'Oracle
static bool executer(QSqlQuery &query, const char *action)
{
    if (query.exec()) return true;
    lastError = query.lastError().text();
    qDebug() << action << "impossible :" << lastError;
    return false;
}

// libellé affiché <-> code enregistré dans la base (contraintes CHECK sans accents)
static QString modeVersCode(const QString &m)
{
    if (m == "En ligne") return "EN_LIGNE";
    if (m == "Hybride") return "HYBRIDE";
    return "PRESENTIEL";
}
static QString codeVersMode(const QString &c)
{
    if (c == "EN_LIGNE") return "En ligne";
    if (c == "HYBRIDE") return "Hybride";
    return "Présentiel";
}
static QString statutVersCode(const QString &s)
{
    if (s == "En cours") return "EN_COURS";
    if (s == "Terminée") return "TERMINEE";
    return "PLANIFIEE";
}
static QString codeVersStatut(const QString &c)
{
    if (c == "EN_COURS") return "En cours";
    if (c == "TERMINEE") return "Terminée";
    return "Planifiée";
}

// ID_JURY : NULL dans Oracle quand aucun jury n'est choisi
static QVariant valeurJury(int idJury)
{
    return idJury > 0 ? QVariant(idJury) : QVariant(QMetaType::fromType<int>());
}

QString Competition::derniereErreur() { return lastError; }

// ------------------------------------------------------------- constructeurs
Competition::Competition() {}

Competition::Competition(const QString &nom, const QString &theme, const QDate &dateDebut,
                         const QDate &dateFin, int nbMaxMembres, const QString &statut,
                         const QString &mode)
    : nom(nom), theme(theme), dateDebut(dateDebut), dateFin(dateFin),
      nbMaxMembres(nbMaxMembres), statut(statut), mode(mode)
{
}

// ------------------------------------------------------------- contrôle de saisie
QString Competition::verifier() const
{
    if (nom.trimmed().isEmpty()) return "Le nom de la compétition est obligatoire.";
    if (nom.length() > 100) return "Le nom ne doit pas dépasser 100 caractères.";
    // lettres seulement (accents acceptés), avec espaces, tirets et apostrophes entre les mots
    static const QRegularExpression lettres("^[\\p{L}]+([ '\\-][\\p{L}]+)*$");
    if (!lettres.match(nom.simplified()).hasMatch())
        return "Le nom doit contenir uniquement des lettres (pas de chiffres ni de symboles).";
    if (nomExiste(nom, id)) return "Une compétition porte déjà ce nom. Choisissez un autre nom.";
    if (theme.trimmed().isEmpty()) return "Le thème est obligatoire.";
    if (!dateDebut.isValid() || !dateFin.isValid()) return "Les dates sont obligatoires.";
    if (dateFin < dateDebut) return "La date de fin doit être après la date de début.";
    if (nbMaxMembres < 1 || nbMaxMembres > 50) return "Le nombre maximum doit être entre 1 et 50.";
    return QString();
}

// ------------------------------------------------------------- CRUD
bool Competition::ajouter()
{
    // Nouvel ID = plus grand ID + 1, calculé ici.
    // (On ne dépend pas de la séquence : si elle est en retard sur les données,
    //  Oracle donnerait un ID déjà utilisé -> erreur ORA-00001.)
    QSqlQuery max("SELECT NVL(MAX(ID_COMPETITION), 0) + 1 FROM COMPETITION");
    if (!max.next()) {
        lastError = max.lastError().text();
        return false;
    }
    id = max.value(0).toInt();

    QSqlQuery query;
    query.prepare("INSERT INTO COMPETITION (ID_COMPETITION, NOM, THEME, DATE_DEBUT, DATE_FIN, "
                  "NB_MAX_MEMBRES, STATUT, MODE_COMP, ID_JURY) "
                  "VALUES (:id, :nom, :theme, :debut, :fin, :max, :statut, :mode, :jury)");
    query.bindValue(":jury", valeurJury(idJury));
    query.bindValue(":id", id);
    query.bindValue(":nom", nom.simplified());
    query.bindValue(":theme", theme);
    query.bindValue(":debut", dateDebut);
    query.bindValue(":fin", dateFin);
    query.bindValue(":max", nbMaxMembres);
    query.bindValue(":statut", statutVersCode(statut));
    query.bindValue(":mode", modeVersCode(mode));
    if (!executer(query, "Ajout")) return false;

    // association « gerer » : la compétition est gérée par l'administrateur (premier employé)
    // (rien n'est ajouté si la table EMPLOYE est vide)
    QSqlQuery gerer;
    gerer.prepare("INSERT INTO GERER (ID_EMPLOYE, ID_COMPETITION) "
                  "SELECT MIN(ID_EMPLOYE), :id FROM EMPLOYE HAVING MIN(ID_EMPLOYE) IS NOT NULL");
    gerer.bindValue(":id", id);
    return executer(gerer, "Ajout dans GERER");
}

QSqlQueryModel *Competition::afficher()
{
    auto *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_COMPETITION, NOM, THEME, DATE_DEBUT, DATE_FIN, NB_MAX_MEMBRES, "
                    "STATUT, MODE_COMP FROM COMPETITION ORDER BY ID_COMPETITION");
    const QStringList titres = {"ID", "Nom", "Thème", "Début", "Fin", "Membres max", "Statut", "Mode"};
    for (int i = 0; i < titres.size(); ++i)
        model->setHeaderData(i, Qt::Horizontal, titres.at(i));
    return model;
}

bool Competition::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE COMPETITION SET NOM = :nom, THEME = :theme, DATE_DEBUT = :debut, "
                  "DATE_FIN = :fin, NB_MAX_MEMBRES = :max, STATUT = :statut, MODE_COMP = :mode, "
                  "ID_JURY = :jury WHERE ID_COMPETITION = :id");
    query.bindValue(":jury", valeurJury(idJury));
    query.bindValue(":nom", nom.trimmed());
    query.bindValue(":theme", theme);
    query.bindValue(":debut", dateDebut);
    query.bindValue(":fin", dateFin);
    query.bindValue(":max", nbMaxMembres);
    query.bindValue(":statut", statutVersCode(statut));
    query.bindValue(":mode", modeVersCode(mode));
    query.bindValue(":id", id);
    return executer(query, "Modification");
}

// les lignes liées de GERER, ATELIER et EQUIPE sont supprimées par ON DELETE CASCADE
bool Competition::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM COMPETITION WHERE ID_COMPETITION = :id");
    query.bindValue(":id", id);
    return executer(query, "Suppression");
}

// nom unique : même nom sans tenir compte des majuscules ni des espaces en trop.
// idExclu = la compétition en cours de modification (elle peut garder son nom).
bool Competition::nomExiste(const QString &nom, int idExclu)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM COMPETITION "
                  "WHERE UPPER(TRIM(NOM)) = UPPER(:nom) AND ID_COMPETITION <> :id");
    query.bindValue(":nom", nom.simplified());
    query.bindValue(":id", idExclu);
    return query.exec() && query.next() && query.value(0).toInt() > 0;
}

QVector<Competition> Competition::lister()
{
    QVector<Competition> out;
    QSqlQuery query;
    // LEFT JOIN : le nom du jury vient de la table EMPLOYE (une compétition peut ne pas avoir de jury)
    query.prepare("SELECT C.ID_COMPETITION, C.NOM, C.THEME, C.DATE_DEBUT, C.DATE_FIN, C.NB_MAX_MEMBRES, "
                  "C.STATUT, C.MODE_COMP, C.ID_JURY, E.PRENOM || ' ' || E.NOM "
                  "FROM COMPETITION C LEFT JOIN EMPLOYE E ON E.ID_EMPLOYE = C.ID_JURY "
                  "ORDER BY C.ID_COMPETITION");
    if (!executer(query, "Lecture")) return out;
    while (query.next()) {
        Competition c;
        c.id = query.value(0).toInt();
        c.nom = query.value(1).toString();
        c.theme = query.value(2).toString();
        c.dateDebut = query.value(3).toDate();
        c.dateFin = query.value(4).toDate();
        c.nbMaxMembres = query.value(5).toInt();
        c.statut = codeVersStatut(query.value(6).toString());
        c.mode = codeVersMode(query.value(7).toString());
        c.idJury = query.value(8).toInt();                  // NULL -> 0
        c.nomJury = query.value(9).toString().trimmed();
        out.append(c);
    }
    return out;
}

// Jurys = employés dont le rôle est JURY dans la table EMPLOYE (module Employés)
QList<QPair<int, QString>> Competition::listerJurys()
{
    QList<QPair<int, QString>> jurys;
    QSqlQuery query;
    query.prepare("SELECT ID_EMPLOYE, PRENOM, NOM FROM EMPLOYE WHERE ROLE = 'JURY' ORDER BY NOM, PRENOM");
    if (!executer(query, "Lecture des jurys")) return jurys;
    while (query.next())
        jurys.append({query.value(0).toInt(),
                      query.value(1).toString().trimmed() + " " + query.value(2).toString().trimmed()});
    return jurys;
}

// Connexion à l'application : on cherche l'employé dans la table EMPLOYE (LOGIN + MDP).
// La colonne ACCES dit quelle gestion il peut ouvrir. Si elle est vide :
// ADMINISTRATEUR -> TOUT, autre rôle -> EMPLOYES. Renvoie une chaîne vide si le compte est inconnu.
QString Competition::accesConnexion(const QString &login, const QString &mdp)
{
    QSqlQuery query;
    query.prepare("SELECT NVL(ACCES, CASE ROLE WHEN 'ADMINISTRATEUR' THEN 'TOUT' ELSE 'EMPLOYES' END) "
                  "FROM EMPLOYE WHERE LOGIN = :login AND MDP = :mdp");
    query.bindValue(":login", login);
    query.bindValue(":mdp", mdp);
    if (!query.exec()) {
        // colonne ACCES pas encore créée (script comptes_connexion.sql non exécuté) : accès selon le rôle
        query.prepare("SELECT CASE ROLE WHEN 'ADMINISTRATEUR' THEN 'TOUT' ELSE 'EMPLOYES' END "
                      "FROM EMPLOYE WHERE LOGIN = :login AND MDP = :mdp");
        query.bindValue(":login", login);
        query.bindValue(":mdp", mdp);
        if (!executer(query, "Connexion")) return QString();
    }
    if (!query.next()) return QString();
    return query.value(0).toString().trimmed().toUpper();
}

// Scénario Arduino : le bouton STATUT de la borne fait avancer le statut dans Oracle
bool Competition::changerStatut(int id, const QString &statut)
{
    QSqlQuery query;
    query.prepare("UPDATE COMPETITION SET STATUT = :statut WHERE ID_COMPETITION = :id");
    query.bindValue(":statut", statutVersCode(statut));
    query.bindValue(":id", id);
    return executer(query, "Changement de statut");
}
