#include "employe.h"

#include <QSqlError>
#include <QVariant>
#include <algorithm>

// ======================================================================
//  Constructeurs
// ======================================================================
Employe::Employe()
    : id(0), role("ORGANISATEUR")
{
}

Employe::Employe(int id, QString nom, QString prenom, QString email, QString role, QString login, QString mdp)
    : id(id), nom(nom), prenom(prenom), email(email), role(role), login(login), mdp(mdp)
{
}

QString Employe::codeDepuisId(int id)
{
    return QString("E-%1").arg(id, 3, 10, QChar('0'));
}

QString Employe::code() const
{
    return codeDepuisId(id);
}

QString Employe::nomComplet() const
{
    return prenom + " " + nom;
}

QString Employe::initiales() const
{
    QString s;
    if (!prenom.isEmpty())
        s += prenom.at(0).toUpper();
    if (!nom.isEmpty())
        s += nom.at(0).toUpper();
    return s;
}

QStringList Employe::listeRoles()
{
    // Valeurs de la contrainte CK_EMPLOYE_ROLE
    return QStringList() << "ADMINISTRATEUR" << "ORGANISATEUR" << "JURY";
}

QString Employe::libelleRole(const QString &role)
{
    if (role == "ADMINISTRATEUR")
        return "Administrateur";
    if (role == "ORGANISATEUR")
        return "Organisateur";
    if (role == "JURY")
        return "Jury";
    return role;
}

QString Employe::libelleRole() const
{
    return libelleRole(role);
}

bool Employe::peutGerer() const
{
    return role != "JURY";
}

// Minuscules sans accents : « Réseau » -> « reseau » (recherche tolérante).
QString Employe::sansAccents(const QString &texte)
{
    const QString decompose = texte.normalized(QString::NormalizationForm_D);
    QString resultat;
    for (int i = 0; i < decompose.size(); ++i) {
        const QChar c = decompose.at(i);
        if (c.category() != QChar::Mark_NonSpacing)
            resultat += c.toLower();
    }
    return resultat.trimmed();
}

// ---------------------------------------------------------------- CRUD
// L'identifiant est donné par la séquence SEQ_EMPLOYE (trigger TRG_EMPLOYE_ID).

int Employe::prochainId()
{
    // Séquence NOCACHE : LAST_NUMBER est la prochaine valeur distribuée.
    QSqlQuery q;
    if (q.exec("SELECT LAST_NUMBER FROM USER_SEQUENCES WHERE SEQUENCE_NAME = 'SEQ_EMPLOYE'") && q.next())
        return q.value(0).toInt();
    return 1;
}

bool Employe::emailExiste(const QString &email, int saufId)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM EMPLOYE WHERE LOWER(EMAIL) = ? AND ID_EMPLOYE <> ?");
    q.addBindValue(email.trimmed().toLower());
    q.addBindValue(saufId);
    if (q.exec() && q.next())
        return q.value(0).toInt() > 0;
    return false;
}

bool Employe::loginExiste(const QString &login, int saufId)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM EMPLOYE WHERE LOGIN = ? AND ID_EMPLOYE <> ?");
    q.addBindValue(login.trimmed());
    q.addBindValue(saufId);
    if (q.exec() && q.next())
        return q.value(0).toInt() > 0;
    return false;
}

bool Employe::ajouter(QString *erreur)
{
    QSqlQuery q;
    q.prepare("INSERT INTO EMPLOYE (NOM, PRENOM, EMAIL, ROLE, LOGIN, MDP) VALUES (?, ?, ?, ?, ?, ?)");
    q.addBindValue(nom);
    q.addBindValue(prenom);
    q.addBindValue(email);
    q.addBindValue(role);
    q.addBindValue(login);
    q.addBindValue(mdp);
    if (!q.exec()) {
        if (erreur)
            *erreur = q.lastError().text();
        return false;
    }
    if (q.exec("SELECT SEQ_EMPLOYE.CURRVAL FROM DUAL") && q.next())
        id = q.value(0).toInt();
    return true;
}

bool Employe::modifier(QString *erreur)
{
    QSqlQuery q;
    if (mdp.isEmpty()) {
        q.prepare("UPDATE EMPLOYE SET NOM = ?, PRENOM = ?, EMAIL = ?, ROLE = ?, LOGIN = ?"
                  " WHERE ID_EMPLOYE = ?");
    } else {
        q.prepare("UPDATE EMPLOYE SET NOM = ?, PRENOM = ?, EMAIL = ?, ROLE = ?, LOGIN = ?, MDP = ?"
                  " WHERE ID_EMPLOYE = ?");
    }
    q.addBindValue(nom);
    q.addBindValue(prenom);
    q.addBindValue(email);
    q.addBindValue(role);
    q.addBindValue(login);
    if (!mdp.isEmpty())
        q.addBindValue(mdp);
    q.addBindValue(id);
    if (!q.exec()) {
        if (erreur)
            *erreur = q.lastError().text();
        return false;
    }
    return true;
}

bool Employe::supprimer(int id)
{
    QSqlQuery query;
    QString res = QString::number(id);
    query.prepare("DELETE FROM EMPLOYE WHERE ID_EMPLOYE = :id");
    query.bindValue(":id", res);
    return query.exec();
}

QSqlQueryModel *Employe::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_EMPLOYE, NOM, PRENOM, EMAIL, ROLE, LOGIN FROM EMPLOYE ORDER BY ID_EMPLOYE");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("E-mail"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Rôle"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Login"));
    return model;
}

QList<Employe> Employe::tous()
{
    QList<Employe> liste;
    QSqlQuery q;
    q.exec("SELECT ID_EMPLOYE, NOM, PRENOM, EMAIL, ROLE, LOGIN FROM EMPLOYE ORDER BY ID_EMPLOYE");
    while (q.next()) {
        Employe e;
        e.id = q.value(0).toInt();
        e.nom = q.value(1).toString();
        e.prenom = q.value(2).toString();
        e.email = q.value(3).toString();
        e.role = q.value(4).toString();
        e.login = q.value(5).toString();
        liste << e;
    }
    return liste;
}

// ------------------------------------------------------ Métiers de base

namespace {

int comparerTexte(const QString &a, const QString &b)
{
    return QString::localeAwareCompare(a.toLower(), b.toLower());
}

struct OrdreEmploye {
    Employe::Tri tri;
    bool croissant;
    const QMap<int, int> *competitions;

    int comparer(const Employe &a, const Employe &b) const
    {
        switch (tri) {
        case Employe::ParNom:    return comparerTexte(a.getNom(), b.getNom());
        case Employe::ParPrenom: return comparerTexte(a.getPrenom(), b.getPrenom());
        case Employe::ParEmail:  return comparerTexte(a.getEmail(), b.getEmail());
        case Employe::ParRole:   return comparerTexte(a.libelleRole(), b.libelleRole());
        case Employe::ParLogin:  return comparerTexte(a.getLogin(), b.getLogin());
        case Employe::ParCompetitions:
            return competitions->value(a.getId()) - competitions->value(b.getId());
        case Employe::ParId:
        default:
            return a.getId() - b.getId();
        }
    }

    bool operator()(const Employe &a, const Employe &b) const
    {
        int c = comparer(a, b);
        if (c == 0)
            c = a.getId() - b.getId();
        return croissant ? c < 0 : c > 0;
    }
};

bool statPlusGrande(const QPair<QString, int> &a, const QPair<QString, int> &b)
{
    if (a.second != b.second)
        return a.second > b.second;
    return QString::localeAwareCompare(a.first, b.first) < 0;
}

} // namespace

QList<Employe> Employe::filtrer(const QList<Employe> &liste, const QString &recherche,
                                const QString &role, const QMap<int, int> &competitions,
                                Tri tri, bool croissant)
{
    const QString q = sansAccents(recherche);
    QList<Employe> resultat;
    for (int i = 0; i < liste.size(); ++i) {
        const Employe &e = liste.at(i);
        if (!role.isEmpty() && e.role != role)
            continue;
        if (!q.isEmpty()) {
            const QString cible = sansAccents(e.nom + " " + e.prenom + " " + e.nom + " "
                                              + e.email + " " + e.login);
            if (!cible.contains(q))
                continue;
        }
        resultat << e;
    }
    OrdreEmploye ordre;
    ordre.tri = tri;
    ordre.croissant = croissant;
    ordre.competitions = &competitions;
    std::sort(resultat.begin(), resultat.end(), ordre);
    return resultat;
}

// Nombre d'employés par rôle, du plus grand au plus petit.
QList<QPair<QString, int> > Employe::statistiquesParRole(const QList<Employe> &liste)
{
    QMap<QString, int> compte;
    for (int i = 0; i < liste.size(); ++i)
        compte[liste.at(i).libelleRole()] += 1;
    QList<QPair<QString, int> > resultat;
    for (QMap<QString, int>::const_iterator it = compte.constBegin(); it != compte.constEnd(); ++it)
        resultat << qMakePair(it.key(), it.value());
    std::sort(resultat.begin(), resultat.end(), statPlusGrande);
    return resultat;
}

// ======================================================================
//  Compétitions gérées (table COMPETITION)
// ======================================================================
CompetitionGeree::CompetitionGeree()
    : id(0)
{
}

QString CompetitionGeree::code() const
{
    return QString("C-%1").arg(id, 2, 10, QChar('0'));
}

bool CompetitionGeree::estActive() const
{
    return statut != "TERMINEE";
}

bool CompetitionGeree::chevauche(const CompetitionGeree &autre) const
{
    return dateDebut <= autre.dateFin && autre.dateDebut <= dateFin;
}

QString CompetitionGeree::periode() const
{
    return dateDebut.toString("dd/MM/yyyy") + " → " + dateFin.toString("dd/MM/yyyy");
}

QString CompetitionGeree::libelleStatut() const
{
    if (statut == "PLANIFIEE")
        return "Planifiée";
    if (statut == "EN_COURS")
        return "En cours";
    if (statut == "TERMINEE")
        return "Terminée";
    return statut;
}

QString CompetitionGeree::libelleMode() const
{
    if (mode == "PRESENTIEL")
        return "Présentiel";
    if (mode == "EN_LIGNE")
        return "En ligne";
    if (mode == "HYBRIDE")
        return "Hybride";
    return mode;
}

QList<CompetitionGeree> CompetitionGeree::toutes()
{
    QList<CompetitionGeree> liste;
    QSqlQuery q;
    q.exec("SELECT ID_COMPETITION, NOM, THEME, TO_CHAR(DATE_DEBUT, 'YYYY-MM-DD'),"
           " TO_CHAR(DATE_FIN, 'YYYY-MM-DD'), STATUT, MODE_COMP"
           " FROM COMPETITION ORDER BY DATE_DEBUT, ID_COMPETITION");
    while (q.next()) {
        CompetitionGeree c;
        c.id = q.value(0).toInt();
        c.nom = q.value(1).toString();
        c.theme = q.value(2).toString();
        c.dateDebut = QDate::fromString(q.value(3).toString(), "yyyy-MM-dd");
        c.dateFin = QDate::fromString(q.value(4).toString(), "yyyy-MM-dd");
        c.statut = q.value(5).toString();
        c.mode = q.value(6).toString();
        liste << c;
    }

    // responsables de chaque compétition (association GERER)
    q.exec("SELECT ID_COMPETITION, ID_EMPLOYE FROM GERER ORDER BY ID_COMPETITION, ID_EMPLOYE");
    while (q.next()) {
        const int idCompetition = q.value(0).toInt();
        for (int i = 0; i < liste.size(); ++i) {
            if (liste.at(i).id == idCompetition)
                liste[i].responsables << q.value(1).toInt();
        }
    }
    return liste;
}

bool CompetitionGeree::changerResponsable(int idCompetition, int idEmploye)
{
    QSqlQuery q;
    q.prepare("DELETE FROM GERER WHERE ID_COMPETITION = :comp");
    q.bindValue(":comp", idCompetition);
    if (!q.exec())
        return false;
    if (idEmploye <= 0)
        return true;
    q.prepare("INSERT INTO GERER (ID_EMPLOYE, ID_COMPETITION) VALUES (:emp, :comp)");
    q.bindValue(":emp", idEmploye);
    q.bindValue(":comp", idCompetition);
    return q.exec();
}

bool CompetitionGeree::transfererGestion(int idCompetition, int ancien, int nouveau)
{
    QSqlQuery q;
    q.prepare("UPDATE GERER SET ID_EMPLOYE = :nouveau WHERE ID_COMPETITION = :comp AND ID_EMPLOYE = :ancien");
    q.bindValue(":nouveau", nouveau);
    q.bindValue(":comp", idCompetition);
    q.bindValue(":ancien", ancien);
    return q.exec();
}

// ======================================================================
//  Métiers avancés : charge de travail et répartition des compétitions
// ======================================================================
int Employe::charge(int idEmploye, const QList<CompetitionGeree> &competitions)
{
    int n = 0;
    for (int i = 0; i < competitions.size(); ++i) {
        if (competitions.at(i).responsables.contains(idEmploye) && competitions.at(i).estActive())
            ++n;
    }
    return n;
}

int Employe::nombreGerees(int idEmploye, const QList<CompetitionGeree> &competitions)
{
    int n = 0;
    for (int i = 0; i < competitions.size(); ++i) {
        if (competitions.at(i).responsables.contains(idEmploye))
            ++n;
    }
    return n;
}

QList<CompetitionGeree> Employe::activesDe(int idEmploye, const QList<CompetitionGeree> &competitions)
{
    QList<CompetitionGeree> liste;
    for (int i = 0; i < competitions.size(); ++i) {
        if (competitions.at(i).responsables.contains(idEmploye) && competitions.at(i).estActive())
            liste << competitions.at(i);
    }
    return liste;
}

bool Employe::chevauchement(int idEmploye, const QList<CompetitionGeree> &competitions)
{
    const QList<CompetitionGeree> actives = activesDe(idEmploye, competitions);
    for (int i = 0; i < actives.size(); ++i) {
        for (int j = i + 1; j < actives.size(); ++j) {
            if (actives.at(i).chevauche(actives.at(j)))
                return true;
        }
    }
    return false;
}

bool Employe::enAlerte(const Employe &e, const QList<CompetitionGeree> &competitions)
{
    const int n = charge(e.id, competitions);
    if (n == 0)
        return false;
    return !e.peutGerer() || n > MAX_ACTIVES || chevauchement(e.id, competitions);
}

QString Employe::etat(const Employe &e, const QList<CompetitionGeree> &competitions, QString *type)
{
    const int n = charge(e.id, competitions);
    if (!e.peutGerer()) {
        *type = n > 0 ? "rouge" : "gris";
        return n > 0 ? QString("Jury responsable") : QString("Non concerné");
    }
    if (chevauchement(e.id, competitions)) {
        *type = "rouge";
        return "Chevauchement";
    }
    if (n > MAX_ACTIVES) {
        *type = "rouge";
        return "Surcharge";
    }
    if (n == MAX_ACTIVES) {
        *type = "orange";
        return "Complet";
    }
    if (n == 0) {
        *type = "bleu";
        return "Libre";
    }
    *type = "vert";
    return "Normal";
}

QList<Alerte> Employe::alertes(const QList<Employe> &employes, const QList<CompetitionGeree> &competitions)
{
    QList<Alerte> liste;
    for (int i = 0; i < employes.size(); ++i) {
        const Employe &e = employes.at(i);
        if (!enAlerte(e, competitions))
            continue;
        const int n = charge(e.id, competitions);
        Alerte a;
        a.idEmploye = e.id;
        if (!e.peutGerer()) {
            a.message = QString("%1 est membre du jury mais gère %2 compétition(s) active(s).")
                            .arg(e.nomComplet()).arg(n);
        } else if (chevauchement(e.id, competitions)) {
            a.message = QString("%1 gère plusieurs compétitions aux mêmes dates.").arg(e.nomComplet());
        } else {
            a.message = QString("%1 est en surcharge : %2 compétitions actives (maximum %3).")
                            .arg(e.nomComplet()).arg(n).arg(MAX_ACTIVES);
        }
        liste << a;
    }
    return liste;
}

namespace {

// Charge d'un employé sans compter la compétition c, et chevauchement éventuel avec c.
void situation(int idEmploye, const CompetitionGeree &c, const QList<CompetitionGeree> &competitions,
               int *charge, bool *conflit)
{
    *charge = 0;
    *conflit = false;
    for (int i = 0; i < competitions.size(); ++i) {
        const CompetitionGeree &autre = competitions.at(i);
        if (autre.id == c.id || !autre.responsables.contains(idEmploye) || !autre.estActive())
            continue;
        ++*charge;
        if (autre.chevauche(c))
            *conflit = true;
    }
}

QString nomDe(int id, const QList<Employe> &employes)
{
    for (int i = 0; i < employes.size(); ++i) {
        if (employes.at(i).getId() == id)
            return employes.at(i).nomComplet();
    }
    return Employe::codeDepuisId(id);
}

// noms des responsables, séparés par des virgules
QString nomsDe(const QList<int> &ids, const QList<Employe> &employes)
{
    if (ids.isEmpty())
        return "aucun responsable";
    QStringList noms;
    for (int i = 0; i < ids.size(); ++i)
        noms << nomDe(ids.at(i), employes);
    return noms.join(", ");
}

bool commenceAvant(const CompetitionGeree &a, const CompetitionGeree &b)
{
    if (a.dateDebut != b.dateDebut)
        return a.dateDebut < b.dateDebut;
    return a.id < b.id;
}

int indexDe(int idCompetition, const QList<CompetitionGeree> &competitions)
{
    for (int i = 0; i < competitions.size(); ++i) {
        if (competitions.at(i).id == idCompetition)
            return i;
    }
    return -1;
}

} // namespace

// Choix du responsable d'une compétition :
//  - candidats : organisateurs et administrateurs, sans autre compétition aux
//    mêmes dates et sous la limite de MAX_ACTIVES compétitions actives ;
//  - le moins chargé l'emporte ; à égalité, un organisateur plutôt qu'un
//    administrateur, puis le responsable actuel (pas de changement inutile).
int Employe::choisir(const CompetitionGeree &c, const QList<Employe> &employes,
                         const QList<CompetitionGeree> &competitions, int exclu, int prefere)
{
    int meilleur = -1;
    int meilleureCharge = 0;
    for (int i = 0; i < employes.size(); ++i) {
        const Employe &e = employes.at(i);
        if (!e.peutGerer() || e.id == exclu || c.responsables.contains(e.id))
            continue;
        int n = 0;
        bool conflit = false;
        situation(e.id, c, competitions, &n, &conflit);
        if (conflit || n >= MAX_ACTIVES)
            continue;

        bool mieux = meilleur < 0 || n < meilleureCharge;
        if (!mieux && n == meilleureCharge) {
            const Employe &m = employes.at(meilleur);
            const int rangE = e.role == "ORGANISATEUR" ? 0 : 1;
            const int rangM = m.role == "ORGANISATEUR" ? 0 : 1;
            if (rangE != rangM)
                mieux = rangE < rangM;
            else
                mieux = e.id == prefere && m.id != prefere;
        }
        if (mieux) {
            meilleur = i;
            meilleureCharge = n;
        }
    }
    return meilleur;
}

QList<LigneRapport> Employe::corrigerAlertes()
{
    const QList<Employe> employes = Employe::tous();
    QList<CompetitionGeree> competitions = CompetitionGeree::toutes();
    QList<LigneRapport> rapport;

    for (int i = 0; i < employes.size(); ++i) {
        const Employe &e = employes.at(i);
        if (!enAlerte(e, competitions))
            continue;

        // Compétitions à confier à quelqu'un d'autre : toutes pour un jury ;
        // sinon on garde, dans l'ordre des dates, celles qui ne se chevauchent
        // pas, dans la limite de MAX_ACTIVES.
        QList<CompetitionGeree> actives = activesDe(e.id, competitions);
        std::sort(actives.begin(), actives.end(), commenceAvant);
        QList<CompetitionGeree> gardees;
        QList<CompetitionGeree> aDeplacer;
        for (int k = 0; k < actives.size(); ++k) {
            const CompetitionGeree &c = actives.at(k);
            bool conflit = false;
            for (int g = 0; g < gardees.size(); ++g) {
                if (gardees.at(g).chevauche(c))
                    conflit = true;
            }
            if (e.peutGerer() && !conflit && gardees.size() < MAX_ACTIVES)
                gardees << c;
            else
                aDeplacer << c;
        }

        // L'employé en alerte cède sa place dans GERER ; les autres
        // responsables de la compétition restent en place.
        for (int k = 0; k < aDeplacer.size(); ++k) {
            const int pos = indexDe(aDeplacer.at(k).id, competitions);
            CompetitionGeree &c = competitions[pos];
            LigneRapport ligne;
            ligne.competition = c;
            ligne.ancien = e.nomComplet();
            ligne.deplacee = false;

            const int choix = choisir(c, employes, competitions, e.id, 0);
            if (choix < 0) {
                ligne.raison = "aucun organisateur n'est libre à ces dates";
            } else if (CompetitionGeree::transfererGestion(c.id, e.id, employes.at(choix).id)) {
                c.responsables.replace(c.responsables.indexOf(e.id), employes.at(choix).id);
                ligne.deplacee = true;
                ligne.nouveau = employes.at(choix).nomComplet();
                ligne.competition = c;
            } else {
                ligne.raison = "la modification n'a pas pu être enregistrée";
            }
            rapport << ligne;
        }
    }
    return rapport;
}

QList<LigneRapport> Employe::toutRepartir()
{
    const QList<Employe> employes = Employe::tous();
    const QList<CompetitionGeree> depart = CompetitionGeree::toutes();

    // Les compétitions actives sont « libérées » puis confiées une à une,
    // dans l'ordre des dates, à un seul responsable ; les compétitions
    // terminées ne bougent pas.
    QList<CompetitionGeree> competitions = depart;
    QList<CompetitionGeree> ordre;
    for (int i = 0; i < competitions.size(); ++i) {
        if (competitions.at(i).estActive()) {
            ordre << competitions.at(i);
            competitions[i].responsables.clear();
        }
    }
    std::sort(ordre.begin(), ordre.end(), commenceAvant);

    QList<LigneRapport> rapport;
    for (int k = 0; k < ordre.size(); ++k) {
        const int pos = indexDe(ordre.at(k).id, competitions);
        CompetitionGeree &c = competitions[pos];
        const QList<int> anciens = ordre.at(k).responsables;
        const int prefere = anciens.isEmpty() ? 0 : anciens.first();
        const int choix = choisir(c, employes, competitions, -1, prefere);

        if (choix < 0) {
            c.responsables = anciens;
            LigneRapport ligne;
            ligne.competition = c;
            ligne.deplacee = false;
            ligne.ancien = nomsDe(anciens, employes);
            ligne.raison = "aucun organisateur n'est libre à ces dates, les responsables actuels sont conservés";
            rapport << ligne;
            continue;
        }

        const int nouveau = employes.at(choix).id;
        c.responsables = QList<int>() << nouveau;
        if (anciens == c.responsables)
            continue;
        LigneRapport ligne;
        ligne.competition = c;
        ligne.ancien = nomsDe(anciens, employes);
        ligne.nouveau = employes.at(choix).nomComplet();
        ligne.deplacee = CompetitionGeree::changerResponsable(c.id, nouveau);
        if (!ligne.deplacee)
            ligne.raison = "la modification n'a pas pu être enregistrée";
        rapport << ligne;
    }
    return rapport;
}

// ======================================================================
//  Employés de démonstration
// ======================================================================
namespace {

struct EmployeDemo {
    const char *nom, *prenom, *email, *role;
};

const EmployeDemo EMPLOYES[] = {
    {"Ben Salah", "Amine", "amine.bensalah@hacktime.tn", "ORGANISATEUR"},
    {"Trabelsi", "Ines", "ines.trabelsi@hacktime.tn", "ORGANISATEUR"},
    {"Gharbi", "Youssef", "youssef.gharbi@hacktime.tn", "ORGANISATEUR"},
    {"Mansouri", "Salma", "salma.mansouri@hacktime.tn", "ORGANISATEUR"},
    {"Jebali", "Karim", "karim.jebali@hacktime.tn", "JURY"},
    {"Hammami", "Nour", "nour.hammami@hacktime.tn", "JURY"}
};

} // namespace

// Ajoute les employés de démonstration absents de la table (rien n'est
// supprimé). Login = début de l'e-mail, mot de passe « hacktime26 ».
int Employe::ajouterDemo()
{
    int ajoutes = 0;
    const int nb = int(sizeof(EMPLOYES) / sizeof(EMPLOYES[0]));
    for (int i = 0; i < nb; ++i) {
        const EmployeDemo &e = EMPLOYES[i];
        const QString email = QString::fromUtf8(e.email);
        const QString login = email.section('@', 0, 0);

        QSqlQuery q;
        q.prepare("SELECT COUNT(*) FROM EMPLOYE WHERE LOWER(EMAIL) = ? OR LOGIN = ?");
        q.addBindValue(email);
        q.addBindValue(login);
        if (!q.exec() || !q.next())
            return -1;
        if (q.value(0).toInt() > 0)
            continue;

        q.prepare("INSERT INTO EMPLOYE (NOM, PRENOM, EMAIL, ROLE, LOGIN, MDP)"
                  " VALUES (?, ?, ?, ?, ?, 'hacktime26')");
        q.addBindValue(QString::fromUtf8(e.nom));
        q.addBindValue(QString::fromUtf8(e.prenom));
        q.addBindValue(email);
        q.addBindValue(QString::fromUtf8(e.role));
        q.addBindValue(login);
        if (!q.exec())
            return -1;
        ++ajoutes;
    }
    return ajoutes;
}
