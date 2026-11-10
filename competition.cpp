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
                  "NB_MAX_MEMBRES, STATUT, MODE_COMP) "
                  "VALUES (:id, :nom, :theme, :debut, :fin, :max, :statut, :mode)");
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
                  "DATE_FIN = :fin, NB_MAX_MEMBRES = :max, STATUT = :statut, MODE_COMP = :mode "
                  "WHERE ID_COMPETITION = :id");
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
    query.prepare("SELECT ID_COMPETITION, NOM, THEME, DATE_DEBUT, DATE_FIN, NB_MAX_MEMBRES, "
                  "STATUT, MODE_COMP FROM COMPETITION ORDER BY ID_COMPETITION");
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
        out.append(c);
    }
    return out;
}
