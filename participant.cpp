#include "participant.h"
#include <QSqlError>
#include <QDebug>
#include <QVariant>
#include <QMap>
#include <QSet>
#include <algorithm>
#include <random>
#include <cmath>

Participant::Participant() {}

Participant::Participant(QString id, QString nom, QString prenom, QString email, QString telephone, QString idEquipe) {
    this->id_participant = id;
    this->nom = nom;
    this->prenom = prenom;
    this->email = email;
    this->telephone = telephone;
    this->id_equipe = idEquipe;
}

// ======================= CRUD =======================

bool Participant::ajouter(QString id, QString nom, QString prenom, QString email, QString telephone,
                          QString idEquipe, QString competence, int niveau)
{
    const bool aEquipe = !idEquipe.trimmed().isEmpty();

    QSqlQuery query;
    query.prepare("INSERT INTO participant (id_participant, nom, prenom, email, telephone, id_equipe, "
                  "competence, niveau, statut, nb_relances) "
                  "VALUES (:id, :nom, :prenom, :email, :telephone, :equipe, :competence, :niveau, :statut, 0)");

    query.bindValue(":id", id.toInt());
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    if (aEquipe)
        query.bindValue(":equipe", idEquipe.toInt());
    else
        query.bindValue(":equipe", QVariant(QMetaType(QMetaType::Int)));
    query.bindValue(":competence", competence);
    query.bindValue(":niveau", niveau);
    query.bindValue(":statut", aEquipe ? "AFFECTE" : "INSCRIT");

    if (!query.exec()) {
        m_derniereErreur = query.lastError().text();
        qDebug() << "Erreur SQL Ajouter :" << m_derniereErreur;
        return false;
    }
    journaliser(id, "INSCRIPTION");
    if (aEquipe) journaliser(id, "AFFECTATION", idEquipe.trimmed());
    return true;
}

QSqlQueryModel* Participant::afficher() {
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT id_participant, nom, prenom, email, telephone, id_equipe FROM participant");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("E-mail"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Équipe"));
    return model;
}

bool Participant::supprimer(QString id) {
    QSqlQuery query;
    query.prepare("DELETE FROM participant WHERE id_participant = :id");
    query.bindValue(":id", id.toInt());
    const bool ok = query.exec();
    if (ok) {
        QSqlQuery h;
        h.prepare("DELETE FROM historique_participant WHERE id_participant = :id");
        h.bindValue(":id", id.toInt());
        h.exec();
    }
    return ok;
}

bool Participant::modifier(QString id, QString nom, QString prenom, QString email, QString telephone,
                           QString idEquipe, QString competence, int niveau)
{
    const bool aEquipe = !idEquipe.trimmed().isEmpty();

    // Le statut suit automatiquement l'équipe (règle métier du workflow)
    QString statut;
    QSqlQuery q;
    q.prepare("SELECT statut FROM participant WHERE id_participant = :id");
    q.bindValue(":id", id.toInt());
    if (q.exec() && q.next())
        statut = q.value(0).toString();
    QString nouveauStatut = statut;
    if (aEquipe && statut == "INSCRIT")
        nouveauStatut = "AFFECTE";
    else if (!aEquipe && (statut == "AFFECTE" || statut == "VALIDE"))
        nouveauStatut = "INSCRIT";

    QSqlQuery query;
    query.prepare("UPDATE participant SET nom = :nom, prenom = :prenom, email = :email, telephone = :telephone, "
                  "id_equipe = :equipe, competence = :competence, niveau = :niveau, statut = :statut "
                  "WHERE id_participant = :id");
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);
    if (aEquipe)
        query.bindValue(":equipe", idEquipe.toInt());
    else
        query.bindValue(":equipe", QVariant(QMetaType(QMetaType::Int)));
    query.bindValue(":competence", competence);
    query.bindValue(":niveau", niveau);
    query.bindValue(":statut", nouveauStatut.isEmpty() ? QString("INSCRIT") : nouveauStatut);
    query.bindValue(":id", id.toInt());
    if (!query.exec()) {
        m_derniereErreur = query.lastError().text();
        qDebug() << "Erreur SQL Modifier :" << m_derniereErreur;
        return false;
    }
    journaliser(id, "MODIFICATION");
    if (aEquipe && !statut.isEmpty() && statut != nouveauStatut)
        journaliser(id, "STATUT", nouveauStatut);
    return true;
}

// ======================= Métiers de base =======================

QSqlQueryModel* Participant::rechercher(QString critere) {
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT id_participant, nom, prenom, email, telephone FROM participant "
                  "WHERE nom LIKE :c OR prenom LIKE :c OR email LIKE :c OR telephone LIKE :c");
    query.bindValue(":c", "%" + critere + "%");
    query.exec();
    model->setQuery(std::move(query));
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("E-mail"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Téléphone"));
    return model;
}

int Participant::calculerTotalParticipants() {
    QSqlQuery query("SELECT COUNT(*) FROM participant");
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

// ======================= Base de données =======================

void Participant::assurerSchema()
{
    struct Col { const char *nom; const char *ddl; };
    const Col colonnes[] = {
        {"ID_EQUIPE",        "id_equipe NUMBER"},
        {"COMPETENCE",       "competence VARCHAR2(30)"},
        {"NIVEAU",           "niveau NUMBER(1) DEFAULT 1"},
        {"STATUT",           "statut VARCHAR2(20) DEFAULT 'INSCRIT'"},
        {"DATE_INSCRIPTION", "date_inscription DATE DEFAULT SYSDATE"},
        {"NB_RELANCES",      "nb_relances NUMBER DEFAULT 0"},
        {"DATE_POINTAGE",    "date_pointage DATE"}        // scénario Arduino : dernier pointage à la barrière
    };

    for (const Col &c : colonnes) {
        QSqlQuery q;
        q.prepare("SELECT COUNT(*) FROM user_tab_columns WHERE table_name = 'PARTICIPANT' AND column_name = :c");
        q.bindValue(":c", QString(c.nom));
        if (!q.exec() || !q.next()) {
            qDebug() << "assurerSchema : lecture impossible :" << q.lastError().text();
            return;
        }
        if (q.value(0).toInt() == 0) {
            QSqlQuery a;
            const QString ddl = QString("ALTER TABLE participant ADD (%1)").arg(c.ddl);
            if (!a.exec(ddl))
                qDebug() << "assurerSchema : échec" << ddl << a.lastError().text();
            else
                qDebug() << "assurerSchema : colonne ajoutée" << c.nom;
        }
    }

    // Table d'historique (ce que chaque participant a fait)
    QSqlQuery t;
    t.prepare("SELECT COUNT(*) FROM user_tables WHERE table_name = 'HISTORIQUE_PARTICIPANT'");
    if (t.exec() && t.next() && t.value(0).toInt() == 0) {
        QSqlQuery a;
        if (!a.exec("CREATE TABLE historique_participant (id_participant NUMBER, date_action DATE, "
                    "action VARCHAR2(30), details VARCHAR2(100))"))
            qDebug() << "assurerSchema : échec création historique" << a.lastError().text();
    }
}

QList<InfoParticipant> Participant::lister(const QString &recherche, int ordre)
{
    QString order;
    switch (ordre) {
    case 1:  order = "UPPER(nom), UPPER(prenom)"; break;
    case 2:  order = "UPPER(nom) DESC, UPPER(prenom) DESC"; break;
    case 3:  order = "UPPER(prenom), UPPER(nom)"; break;
    case 4:  order = "UPPER(prenom) DESC, UPPER(nom) DESC"; break;
    case 5:  order = "UPPER(email)"; break;
    case 6:  order = "UPPER(email) DESC"; break;
    default: order = "id_participant"; break;
    }

    QString sql = "SELECT id_participant, nom, prenom, email, telephone, id_equipe, competence, niveau, "
                  "statut, date_inscription, nb_relances FROM participant";
    const QString texte = recherche.trimmed();
    if (!texte.isEmpty()) {
        sql += " WHERE UPPER(nom) LIKE :c OR UPPER(prenom) LIKE :c OR UPPER(email) LIKE :c"
               " OR TO_CHAR(telephone) LIKE :c OR TO_CHAR(id_equipe) LIKE :c"
               " OR UPPER(nom || ' ' || prenom) LIKE :c OR UPPER(prenom || ' ' || nom) LIKE :c";
    }
    sql += " ORDER BY " + order;

    QSqlQuery q;
    q.prepare(sql);
    if (!texte.isEmpty())
        q.bindValue(":c", "%" + texte.toUpper() + "%");

    QList<InfoParticipant> liste;
    if (!q.exec()) {
        qDebug() << "Erreur SQL lister :" << q.lastError().text();
        return liste;
    }
    while (q.next()) {
        InfoParticipant p;
        p.id        = q.value(0).toString();
        p.nom       = q.value(1).toString();
        p.prenom    = q.value(2).toString();
        p.email     = q.value(3).toString();
        p.telephone = q.value(4).toString();
        p.idEquipe  = q.value(5).isNull() ? 0 : q.value(5).toInt();
        p.competence = q.value(6).toString();
        p.niveau    = q.value(7).isNull() ? 1 : q.value(7).toInt();
        p.statut    = q.value(8).toString().isEmpty() ? QString("INSCRIT") : q.value(8).toString();
        p.dateInscription = q.value(9).toDate();
        p.nbRelances = q.value(10).isNull() ? 0 : q.value(10).toInt();
        liste.append(p);
    }
    return liste;
}

bool Participant::affecterEquipe(const QString &id, int idEquipe)
{
    QSqlQuery q;
    q.prepare("UPDATE participant SET id_equipe = :e, "
              "statut = CASE WHEN statut = 'INSCRIT' THEN 'AFFECTE' ELSE statut END "
              "WHERE id_participant = :id");
    q.bindValue(":e", idEquipe);
    q.bindValue(":id", id.toInt());
    if (!q.exec()) {
        qDebug() << "Erreur SQL affecterEquipe :" << q.lastError().text();
        return false;
    }
    journaliser(id, "AFFECTATION", QString::number(idEquipe));
    return true;
}

bool Participant::changerStatut(const QString &id, const QString &statut)
{
    QSqlQuery q;
    if (statut == "INSCRIT") {
        // Réactivation : on remet les relances à zéro
        q.prepare("UPDATE participant SET statut = :s, nb_relances = 0 WHERE id_participant = :id");
    } else {
        q.prepare("UPDATE participant SET statut = :s WHERE id_participant = :id");
    }
    q.bindValue(":s", statut);
    q.bindValue(":id", id.toInt());
    if (!q.exec()) {
        qDebug() << "Erreur SQL changerStatut :" << q.lastError().text();
        return false;
    }
    journaliser(id, "STATUT", statut);
    return true;
}

bool Participant::enregistrerRelance(const QString &id)
{
    QSqlQuery q;
    q.prepare("UPDATE participant SET nb_relances = NVL(nb_relances, 0) + 1 WHERE id_participant = :id");
    q.bindValue(":id", id.toInt());
    if (!q.exec()) {
        qDebug() << "Erreur SQL enregistrerRelance :" << q.lastError().text();
        return false;
    }
    QString numero;
    QSqlQuery n;
    n.prepare("SELECT nb_relances FROM participant WHERE id_participant = :id");
    n.bindValue(":id", id.toInt());
    if (n.exec() && n.next()) numero = n.value(0).toString();
    journaliser(id, "RELANCE", numero);
    return true;
}

int Participant::prochainIdEquipe()
{
    QSqlQuery q("SELECT NVL(MAX(id_equipe), 0) + 1 FROM participant");
    if (q.next())
        return q.value(0).toInt();
    return 1;
}

int Participant::effectifEquipe(int idEquipe)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM participant WHERE id_equipe = :e");
    q.bindValue(":e", idEquipe);
    if (q.exec() && q.next())
        return q.value(0).toInt();
    return 0;
}


// ======================= Historique =======================

// ---------- Scénario Arduino : barrière d'accès ----------
InfoPointage Participant::pointer(const QString &id)
{
    InfoPointage r;
    QSqlQuery q;
    q.prepare("SELECT nom, prenom, statut, TRUNC(date_pointage) FROM participant WHERE id_participant = :id");
    q.bindValue(":id", id.toInt());
    if (!q.exec()) {
        qDebug() << "Erreur SQL pointer (lecture) :" << q.lastError().text();
        r.resultat = ResultatPointage::ErreurSql;
        r.message = "Erreur de base de données";
        return r;
    }
    if (!q.next()) {
        r.resultat = ResultatPointage::Introuvable;
        r.message = "Participant introuvable";
        return r;
    }
    r.nom = q.value(1).toString() + " " + q.value(0).toString();
    const QString statut = q.value(2).toString();
    const QDate dernier = q.value(3).toDate();

    // Règle 1 : seuls les participants « Validé » (équipe d'au moins 4 membres) peuvent entrer
    if (statut != "VALIDE") {
        r.resultat = ResultatPointage::StatutInsuffisant;
        r.message = QString("Accès refusé : statut « %1 » (il faut « Validé »)").arg(Metier::libelleStatut(statut));
        journaliser(id, "REFUS", Metier::libelleStatut(statut));
        return r;
    }
    // Règle 2 : une seule entrée par jour
    if (dernier.isValid() && dernier == QDate::currentDate()) {
        r.resultat = ResultatPointage::DejaPointe;
        r.message = "Déjà pointé aujourd'hui";
        return r;
    }
    // Entrée acceptée : UPDATE dans Oracle + historique
    QSqlQuery u;
    u.prepare("UPDATE participant SET date_pointage = SYSDATE WHERE id_participant = :id");
    u.bindValue(":id", id.toInt());
    if (!u.exec()) {
        qDebug() << "Erreur SQL pointer (UPDATE) :" << u.lastError().text();
        r.resultat = ResultatPointage::ErreurSql;
        r.message = "Erreur de base de données";
        return r;
    }
    journaliser(id, "POINTAGE", "Entrée à la barrière");
    r.resultat = ResultatPointage::Accepte;
    r.message = "Entrée validée : barrière ouverte";
    return r;
}

int Participant::nombrePresentsAujourdhui()
{
    QSqlQuery q("SELECT COUNT(*) FROM participant WHERE TRUNC(date_pointage) = TRUNC(SYSDATE)");
    return q.next() ? q.value(0).toInt() : 0;
}

QSet<QString> Participant::idsPointesAujourdhui()
{
    QSet<QString> ids;
    QSqlQuery q("SELECT id_participant FROM participant WHERE TRUNC(date_pointage) = TRUNC(SYSDATE)");
    while (q.next()) ids.insert(q.value(0).toString());
    return ids;
}

void Participant::journaliser(const QString &id, const QString &code, const QString &details)
{
    QSqlQuery q;
    q.prepare("INSERT INTO historique_participant (id_participant, date_action, action, details) "
              "VALUES (:id, :d, :a, :det)");
    q.bindValue(":id", id.toInt());
    q.bindValue(":d", QDateTime::currentDateTime());
    q.bindValue(":a", code);
    q.bindValue(":det", details);
    if (!q.exec())
        qDebug() << "Erreur SQL journaliser :" << q.lastError().text();
}

QList<EvenementHistorique> Participant::historique(const QString &id)
{
    QList<EvenementHistorique> liste;
    QSqlQuery q;
    q.prepare("SELECT date_action, action, details FROM historique_participant "
              "WHERE id_participant = :id ORDER BY date_action");
    q.bindValue(":id", id.toInt());
    if (!q.exec()) {
        qDebug() << "Erreur SQL historique :" << q.lastError().text();
        return liste;
    }
    while (q.next()) {
        EvenementHistorique e;
        e.date = q.value(0).toDateTime();
        e.code = q.value(1).toString();
        e.details = q.value(2).toString();
        liste.append(e);
    }
    return liste;
}

// =====================================================================
//  MÉTIERS AVANCÉS
// =====================================================================
namespace Metier {

// ======================= Libellés =======================

QString libelleStatut(const QString &code)
{
    if (code == "AFFECTE") return "Affecté";
    if (code == "VALIDE")  return "Validé";
    if (code == "INACTIF") return "Inactif";
    return "Inscrit";
}

QString libelleNiveau(int niveau)
{
    if (niveau >= 3) return "Avancé";
    if (niveau == 2) return "Intermédiaire";
    return "Débutant";
}

QString libelleCompetence(const QString &code)
{
    if (code == "DEV")       return "Développement";
    if (code == "DESIGN")    return "Design";
    if (code == "DATA")      return "Data / IA";
    if (code == "MARKETING") return "Marketing";
    if (code == "GESTION")   return "Gestion de projet";
    return "-";
}

QStringList codesCompetences()
{
    return {"DEV", "DESIGN", "DATA", "MARKETING", "GESTION"};
}

QString libelleEvenement(const EvenementHistorique &e)
{
    if (e.code == "INSCRIPTION")  return "Inscription au hackathon";
    if (e.code == "MODIFICATION") return "Informations du profil modifiées";
    if (e.code == "AFFECTATION")  return QString("Affecté à l'équipe n° %1").arg(e.details);
    if (e.code == "STATUT")       return QString("Statut passé à « %1 »").arg(libelleStatut(e.details));
    if (e.code == "CERTIFICAT")   return "Certificat de participation délivré";
    if (e.code == "POINTAGE")     return "Entrée pointée à la barrière (Arduino)";
    if (e.code == "REFUS")        return QString("Entrée refusée à la barrière (statut « %1 »)").arg(e.details);
    if (e.code == "RELANCE")      return QString("Relance n° %1 reçue").arg(e.details.isEmpty() ? QString("?") : e.details);
    return e.code;
}

// ======================= MÉTIER 2 : Score d'engagement =======================
//
//  Score /100 = profil (20) + équipe (25) + statut (25) + réactivité (20) + relances (10)
//    - Profil     : nom/prénom, e-mail, téléphone, compétence renseignés (5 points chacun)
//    - Équipe     : 25 points si le participant appartient à une équipe
//    - Statut     : Inscrit 5, Affecté 15, Validé 25, Inactif 0
//    - Réactivité : 20 points si équipe ; sinon 20 - 2 points par jour passé sans équipe
//    - Relances   : 10 - 5 points par relance restée sans réponse
//
//  Catégories : >= 70 Actif | 40..69 À surveiller | < 40 À risque | statut Inactif
//  "Nouveau" : inscrit depuis 2 jours ou moins, sans équipe (période de grâce, pas encore à surveiller)
Engagement calculerEngagement(const InfoParticipant &p, const QDate &aujourdhui)
{
    Engagement e;

    int profil = 0;
    if (!p.nom.trimmed().isEmpty() && !p.prenom.trimmed().isEmpty()) profil += 5;
    if (!p.email.trimmed().isEmpty())      profil += 5;
    if (!p.telephone.trimmed().isEmpty())  profil += 5;
    if (!p.competence.trimmed().isEmpty()) profil += 5;

    const int equipe = p.aEquipe() ? 25 : 0;

    int statut = 0;
    if (p.statut == "VALIDE")       statut = 25;
    else if (p.statut == "AFFECTE") statut = 15;
    else if (p.statut == "INSCRIT") statut = 5;

    int jours = p.dateInscription.isValid() ? int(p.dateInscription.daysTo(aujourdhui)) : 0;
    if (jours < 0) jours = 0;
    const int reactivite = p.aEquipe() ? 20 : std::max(0, 20 - 2 * jours);

    const int relances = std::max(0, 10 - 5 * p.nbRelances);

    e.details = {
        {"Profil complet (nom, e-mail, téléphone, compétence)", profil, 20},
        {"Appartenance à une équipe", equipe, 25},
        {"Avancement du statut", statut, 25},
        {"Réactivité depuis l'inscription", reactivite, 20},
        {"Réponse aux relances", relances, 10}
    };
    e.score = std::min(100, std::max(0, profil + equipe + statut + reactivite + relances));

    if (p.statut == "INACTIF") {
        e.categorie = "Inactif";
        e.couleurFond = "#e2e8f0"; e.couleurTexte = "#475569";
    } else if (!p.aEquipe() && p.statut == "INSCRIT" && jours <= 2 && p.nbRelances == 0) {
        e.categorie = "Nouveau";
        e.couleurFond = "#dbeafe"; e.couleurTexte = "#1e40af";
    } else if (e.score >= 70) {
        e.categorie = "Actif";
        e.couleurFond = "#dcfce7"; e.couleurTexte = "#166534";
    } else if (e.score >= 40) {
        e.categorie = "À surveiller";
        e.couleurFond = "#fef3c7"; e.couleurTexte = "#9a3412";
    } else {
        e.categorie = "À risque";
        e.couleurFond = "#fee2e2"; e.couleurTexte = "#991b1b";
    }
    return e;
}

QString motifRisque(const InfoParticipant &p, const QDate &aujourdhui)
{
    QStringList motifs;
    if (!p.aEquipe()) {
        int jours = p.dateInscription.isValid() ? int(p.dateInscription.daysTo(aujourdhui)) : 0;
        motifs << QString("Sans équipe depuis %1 j").arg(std::max(0, jours));
    }
    if (p.statut == "INSCRIT")
        motifs << "Statut Inscrit";
    if (p.nbRelances > 0)
        motifs << QString("%1 relance(s) sans réponse").arg(p.nbRelances);
    if (p.email.trimmed().isEmpty() || p.telephone.trimmed().isEmpty() || p.competence.trimmed().isEmpty())
        motifs << "Profil incomplet";
    if (motifs.isEmpty())
        motifs << "-";
    return motifs.join(" · ");
}

// ======================= MÉTIER 1 : Composition d'équipes =======================
//
//  Règle métier : une équipe doit avoir au minimum 4 participants.
//
//  Phase 1 - compléter les équipes existantes qui ont moins de 4 membres :
//            pour chaque équipe, on ajoute le participant (sans équipe) qui donne le meilleur score
//            (niveau équilibré + compétences complémentaires), jusqu'à atteindre 4 membres.
//  Phase 2 - avec les participants restants (au moins 4), création de nouvelles équipes :
//            nombre d'équipes = restants / 4, répartition "en serpentin" par niveau,
//            puis optimisation par échanges (on garde l'échange si le score total augmente).
//  Ceux qui restent (moins de 4) sont mis "en attente".
//
//  Score d'une équipe /100 = équilibre du niveau (40) + diversité des compétences (40) + taille (20)

static QString cleCompetence(const InfoParticipant &p)
{
    return p.competence.trimmed().isEmpty() ? QString("AUTRE") : p.competence;
}

double scoreGroupe(const QList<InfoParticipant> &membres, int tailleMin, double niveauGlobal, int nbCompetencesTotal)
{
    if (membres.isEmpty()) return 0.0;

    double somme = 0.0;
    QSet<QString> competences;
    for (const InfoParticipant &p : membres) {
        somme += p.niveau;
        competences.insert(cleCompetence(p));
    }
    const int taille = membres.size();
    const double moyenne = somme / taille;

    const double equilibre = 40.0 * std::max(0.0, 1.0 - std::fabs(moyenne - niveauGlobal) / 2.0);
    const int diversiteMax = std::max(1, std::min(taille, nbCompetencesTotal));
    const double diversite = 40.0 * double(competences.size()) / diversiteMax;

    double tailleScore;
    if (taille < tailleMin)            tailleScore = 20.0 * taille / tailleMin;
    else if (taille <= tailleMin + 1)  tailleScore = 20.0;
    else                               tailleScore = std::max(0.0, 20.0 - 5.0 * (taille - tailleMin - 1));

    return equilibre + diversite + tailleScore;
}

static void finaliserEquipe(EquipeProposee &eq, int tailleMin, double niveauGlobal, int nbComp)
{
    double somme = 0.0;
    QSet<QString> comp;
    for (const InfoParticipant &p : eq.membres) {
        somme += p.niveau;
        comp.insert(cleCompetence(p));
    }
    eq.niveauMoyen = eq.membres.isEmpty() ? 0.0 : somme / eq.membres.size();
    eq.nbCompetences = comp.size();
    eq.score = int(std::lround(scoreGroupe(eq.membres, tailleMin, niveauGlobal, nbComp)));
}

ResultatComposition composerEquipes(const QList<InfoParticipant> &tous, int tailleMin, int premierId)
{
    ResultatComposition res;
    if (tailleMin < 1) return res;

    // ---- Séparation : équipes existantes / participants sans équipe ----
    QList<InfoParticipant> pool;
    QMap<int, QList<InfoParticipant>> existantes;
    double sommeNiveaux = 0.0;
    int total = 0;
    QSet<QString> compGlobales;
    for (const InfoParticipant &p : tous) {
        if (p.statut == "INACTIF") continue;
        total++;
        sommeNiveaux += p.niveau;
        compGlobales.insert(cleCompetence(p));
        if (p.aEquipe()) existantes[p.idEquipe].append(p);
        else pool.append(p);
    }
    if (total == 0) return res;
    const double niveauGlobal = sommeNiveaux / total;
    const int nbComp = compGlobales.size();

    // ---- Phase 1 : compléter les équipes incomplètes (les plus proches de 4 d'abord) ----
    QList<int> incompletes;
    for (auto it = existantes.constBegin(); it != existantes.constEnd(); ++it)
        if (it.value().size() < tailleMin) incompletes.append(it.key());
    std::stable_sort(incompletes.begin(), incompletes.end(), [&](int a, int b) {
        if (existantes[a].size() != existantes[b].size()) return existantes[a].size() > existantes[b].size();
        return a < b;
    });

    for (int id : incompletes) {
        EquipeProposee eq;
        eq.idEquipe = id;
        eq.existante = true;
        eq.membres = existantes[id];
        for (int i = 0; i < eq.membres.size(); ++i) eq.nouveau.append(false);

        while (eq.membres.size() < tailleMin && !pool.isEmpty()) {
            int meilleur = 0;
            double meilleurScore = -1.0;
            for (int i = 0; i < pool.size(); ++i) {
                QList<InfoParticipant> essai = eq.membres;
                essai.append(pool[i]);
                const double s = scoreGroupe(essai, tailleMin, niveauGlobal, nbComp);
                if (s > meilleurScore + 1e-9) { meilleurScore = s; meilleur = i; }
            }
            eq.membres.append(pool.takeAt(meilleur));
            eq.nouveau.append(true);
        }
        eq.etat = (eq.membres.size() >= tailleMin) ? "Complétée" : "Incomplète";
        finaliserEquipe(eq, tailleMin, niveauGlobal, nbComp);
        res.equipes.append(eq);
    }

    // ---- Phase 2 : nouvelles équipes avec les participants restants ----
    const int n = pool.size();
    if (n >= tailleMin) {
        const int k = n / tailleMin;                         // nombre de nouvelles équipes
        const int utilises = std::min(n, k * kMaxEquipe);

        QList<int> ordre;
        for (int i = 0; i < n; ++i) ordre.append(i);
        std::stable_sort(ordre.begin(), ordre.end(), [&](int a, int b) {
            if (pool[a].niveau != pool[b].niveau) return pool[a].niveau > pool[b].niveau;
            return cleCompetence(pool[a]) < cleCompetence(pool[b]);
        });

        QList<QList<int>> groupes;
        for (int e = 0; e < k; ++e) groupes.append(QList<int>());
        for (int i = 0; i < utilises; ++i) {                 // répartition en serpentin
            const int tour = i / k;
            const int pos = i % k;
            const int e = (tour % 2 == 0) ? pos : (k - 1 - pos);
            groupes[e].append(ordre[i]);
        }
        QSet<int> utilisesSet;
        for (int i = 0; i < utilises; ++i) utilisesSet.insert(ordre[i]);

        auto membresDe = [&](const QList<int> &g) {
            QList<InfoParticipant> m;
            for (int idx : g) m.append(pool[idx]);
            return m;
        };
        auto totalScore = [&]() {
            double t = 0.0;
            for (const QList<int> &g : groupes)
                t += scoreGroupe(membresDe(g), tailleMin, niveauGlobal, nbComp);
            return t;
        };
        if (k > 1) {                                         // optimisation locale (graine fixe)
            std::mt19937 rng(2024);
            double meilleur = totalScore();
            for (int it = 0; it < 4000; ++it) {
                const int a = int(rng() % k);
                const int b = int(rng() % k);
                if (a == b || groupes[a].isEmpty() || groupes[b].isEmpty()) continue;
                const int ia = int(rng() % groupes[a].size());
                const int ib = int(rng() % groupes[b].size());
                std::swap(groupes[a][ia], groupes[b][ib]);
                const double t = totalScore();
                if (t > meilleur + 1e-9) meilleur = t;
                else std::swap(groupes[a][ia], groupes[b][ib]);
            }
        }

        for (int e = 0; e < k; ++e) {
            EquipeProposee eq;
            eq.idEquipe = premierId + e;
            eq.existante = false;
            QList<int> g = groupes[e];
            std::sort(g.begin(), g.end());
            eq.membres = membresDe(g);
            for (int i = 0; i < eq.membres.size(); ++i) eq.nouveau.append(true);
            eq.etat = "Nouvelle";
            finaliserEquipe(eq, tailleMin, niveauGlobal, nbComp);
            res.equipes.append(eq);
        }

        QList<InfoParticipant> reste;
        for (int i = 0; i < n; ++i)
            if (!utilisesSet.contains(i)) reste.append(pool[i]);
        pool = reste;
    }

    res.enAttente = pool;
    return res;
}

} // namespace Metier
