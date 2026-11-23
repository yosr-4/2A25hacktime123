#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QString>
#include <QDate>
#include <QDateTime>
#include <QList>
#include <QSet>
#include <QSqlQuery>
#include <QSqlQueryModel>

// Règle métier : une équipe doit avoir au minimum 4 participants (et au maximum 6)
constexpr int kMinEquipe = 4;
constexpr int kMaxEquipe = 6;

// Photographie d'un participant (ligne de la table participant)
struct InfoParticipant {
    QString id;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    int     idEquipe = 0;            // 0 = pas encore d'équipe
    QString competence;              // code : DEV, DESIGN, DATA, MARKETING, GESTION
    int     niveau = 1;              // 1 = Débutant, 2 = Intermédiaire, 3 = Avancé
    QString statut = "INSCRIT";      // INSCRIT, AFFECTE, VALIDE, INACTIF
    QDate   dateInscription;
    int     nbRelances = 0;

    bool aEquipe() const { return idEquipe > 0; }
    QString nomComplet() const { return prenom + " " + nom; }
};

// Une ligne de l'historique d'un participant (ce qu'il a fait / ce qui lui est arrivé)
struct EvenementHistorique {
    QDateTime date;
    QString   code;       // INSCRIPTION, MODIFICATION, AFFECTATION, STATUT, RELANCE
    QString   details;
};

// Résultat d'un pointage à la borne Arduino (barrière d'accès)
enum class ResultatPointage { Accepte, StatutInsuffisant, DejaPointe, Introuvable, ErreurSql };
struct InfoPointage {
    ResultatPointage resultat = ResultatPointage::Introuvable;
    QString nom;        // nom complet du participant
    QString message;    // phrase prête à afficher
};

class Participant {
private:
    QString id_participant;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QString id_equipe;
    QString m_derniereErreur;

public:
    Participant();
    Participant(QString id, QString nom, QString prenom, QString email, QString telephone, QString idEquipe = QString());

    // ---------- CRUD ----------
    bool ajouter(QString id, QString nom, QString prenom, QString email, QString telephone,
                 QString idEquipe, QString competence, int niveau);
    QSqlQueryModel* afficher();
    bool supprimer(QString id);
    bool modifier(QString id, QString nom, QString prenom, QString email, QString telephone,
                  QString idEquipe, QString competence, int niveau);
    QString derniereErreur() const { return m_derniereErreur; }

    // ---------- Recherche / statistiques (métiers de base) ----------
    QSqlQueryModel* rechercher(QString critere);
    int calculerTotalParticipants();

    // ---------- Base de données ----------
    // Ajoute automatiquement les colonnes / la table d'historique si elles n'existent pas
    static void assurerSchema();
    // Liste (avec recherche nom/prénom/e-mail/téléphone/équipe) triée selon l'index :
    // 0 ID, 1 Nom A-Z, 2 Nom Z-A, 3 Prénom A-Z, 4 Prénom Z-A, 5 E-mail A-Z, 6 E-mail Z-A
    static QList<InfoParticipant> lister(const QString &recherche = QString(), int ordre = 0);
    static bool affecterEquipe(const QString &id, int idEquipe);
    static bool changerStatut(const QString &id, const QString &statut);
    static bool enregistrerRelance(const QString &id);
    static int  prochainIdEquipe();
    static int  effectifEquipe(int idEquipe);

    // ---------- Scénario Arduino : barrière d'accès (pointage à l'entrée) ----------
    // Vérifie dans Oracle que le participant est « Validé » et pas déjà pointé aujourd'hui,
    // puis enregistre l'entrée (UPDATE date_pointage + historique). Le résultat commande la barrière.
    static InfoPointage pointer(const QString &id);
    static int nombrePresentsAujourdhui();
    static QSet<QString> idsPointesAujourdhui();

    // ---------- Historique ----------
    static void journaliser(const QString &id, const QString &code, const QString &details = QString());
    static QList<EvenementHistorique> historique(const QString &id);
};

// =====================================================================
//  MÉTIERS AVANCÉS DU MODULE PARTICIPANTS
//   Métier 1 : Composition intelligente d'équipes (minimum 4 par équipe)
//   Métier 2 : Score d'engagement & détection des participants à risque
// =====================================================================
namespace Metier {

// ---------- Libellés (la base stocke des codes sans accents) ----------
QString libelleStatut(const QString &code);          // "AFFECTE" -> "Affecté"
QString libelleNiveau(int niveau);                   // 2 -> "Intermédiaire"
QString libelleCompetence(const QString &code);      // "DEV" -> "Développement"
QStringList codesCompetences();                      // {"DEV","DESIGN","DATA","MARKETING","GESTION"}
QString libelleEvenement(const EvenementHistorique &e);

// ---------- MÉTIER 2 : Score d'engagement ----------
struct CritereEngagement { QString libelle; int points; int maximum; };
struct Engagement {
    int     score = 0;        // 0..100
    QString categorie;        // "Actif", "Nouveau", "À surveiller", "À risque", "Inactif"
    QString couleurTexte;
    QString couleurFond;
    QList<CritereEngagement> details;
};
Engagement calculerEngagement(const InfoParticipant &p, const QDate &aujourdhui = QDate::currentDate());
QString motifRisque(const InfoParticipant &p, const QDate &aujourdhui = QDate::currentDate());

// ---------- MÉTIER 1 : Composition intelligente d'équipes ----------
struct EquipeProposee {
    int  idEquipe = 0;
    bool existante = false;                 // true = équipe déjà en base qu'on complète
    QList<InfoParticipant> membres;         // membres actuels + nouveaux
    QList<bool> nouveau;                    // même taille que membres : true = vient d'être ajouté
    double niveauMoyen = 0;
    int    nbCompetences = 0;
    int    score = 0;                       // score de compatibilité /100
    QString etat;                           // "Nouvelle", "Complétée", "Incomplète"
    int nbNouveaux() const { int n = 0; for (bool b : nouveau) if (b) n++; return n; }
};
struct ResultatComposition {
    QList<EquipeProposee> equipes;
    QList<InfoParticipant> enAttente;       // participants qui ne peuvent pas encore former une équipe
};
// `tous` = tous les participants (les inactifs sont ignorés)
ResultatComposition composerEquipes(const QList<InfoParticipant> &tous, int tailleMin, int premierIdNouvelleEquipe);

// Score de compatibilité d'un groupe (0..100)
double scoreGroupe(const QList<InfoParticipant> &membres, int tailleMin, double niveauGlobal, int nbCompetencesTotal);

} // namespace Metier

#endif // PARTICIPANT_H
