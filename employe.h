#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QDate>
#include <QList>
#include <QMap>
#include <QPair>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QString>
#include <QStringList>

// Compétition vue par le module Employés : association « gérer »
//   Gerer (#id_employe, #id_competition)  (Employe n,n Competition)
// Le module lit COMPETITION et GERER, et ne modifie que GERER (responsables).
class CompetitionGeree
{
public:
    CompetitionGeree();

    int id;
    QString nom;
    QString theme;
    QDate dateDebut;
    QDate dateFin;
    QString statut;     // PLANIFIEE, EN_COURS, TERMINEE
    QString mode;       // PRESENTIEL, EN_LIGNE, HYBRIDE
    QList<int> responsables;   // employés qui gèrent la compétition (table GERER)

    QString code() const;
    bool estActive() const;
    bool chevauche(const CompetitionGeree &autre) const;
    QString periode() const;
    QString libelleStatut() const;
    QString libelleMode() const;

    static QList<CompetitionGeree> toutes();
    // remplace tous les responsables par un seul (0 = aucun responsable)
    static bool changerResponsable(int idCompetition, int idEmploye);
    // confie à « nouveau » la place de « ancien » dans GERER
    static bool transfererGestion(int idCompetition, int ancien, int nouveau);
};

struct Alerte
{
    int idEmploye;
    QString message;
};

struct LigneRapport
{
    CompetitionGeree competition;
    bool deplacee;
    QString ancien;
    QString nouveau;
    QString raison;
};

// Entité Employé : table EMPLOYE (ID_EMPLOYE, NOM, PRENOM, EMAIL, ROLE, LOGIN, MDP)
class Employe
{
    int id;
    QString nom, prenom, email, role, login, mdp;

public:
    enum Tri { ParId, ParNom, ParPrenom, ParEmail, ParRole, ParLogin, ParCompetitions };

    // Nombre maximum de compétitions actives gérées en même temps
    static const int MAX_ACTIVES = 2;

    // Constructeurs
    Employe();
    Employe(int id, QString nom, QString prenom, QString email, QString role, QString login, QString mdp);

    // Getters
    int getId() const { return id; }
    QString getNom() const { return nom; }
    QString getPrenom() const { return prenom; }
    QString getEmail() const { return email; }
    QString getRole() const { return role; }
    QString getLogin() const { return login; }
    QString getMdp() const { return mdp; }

    // Setters
    void setId(int id) { this->id = id; }
    void setNom(QString n) { nom = n; }
    void setPrenom(QString p) { prenom = p; }
    void setEmail(QString e) { email = e; }
    void setRole(QString r) { role = r; }
    void setLogin(QString l) { login = l; }
    void setMdp(QString m) { mdp = m; }

    QString code() const;
    QString nomComplet() const;
    QString initiales() const;
    QString libelleRole() const;
    bool peutGerer() const;

    // Fonctionnalités de base (CRUD)
    bool ajouter(QString *erreur = nullptr);
    bool modifier(QString *erreur = nullptr);
    bool supprimer(int id);
    QSqlQueryModel *afficher();
    static QList<Employe> tous();

    // Contrôles de saisie
    static int prochainId();
    static bool emailExiste(const QString &email, int saufId = 0);
    static bool loginExiste(const QString &login, int saufId = 0);

    // Recherche, tri et statistiques
    static QList<Employe> filtrer(const QList<Employe> &liste, const QString &recherche,
                                  const QString &role, const QMap<int, int> &competitions,
                                  Tri tri, bool croissant);
    static QList<QPair<QString, int> > statistiquesParRole(const QList<Employe> &liste);

    // Métiers avancés : charge de gestion des compétitions et répartition
    static int charge(int idEmploye, const QList<CompetitionGeree> &competitions);
    static int nombreGerees(int idEmploye, const QList<CompetitionGeree> &competitions);
    static QList<CompetitionGeree> activesDe(int idEmploye, const QList<CompetitionGeree> &competitions);
    static bool chevauchement(int idEmploye, const QList<CompetitionGeree> &competitions);
    static bool enAlerte(const Employe &e, const QList<CompetitionGeree> &competitions);
    static QString etat(const Employe &e, const QList<CompetitionGeree> &competitions, QString *couleur);
    static QList<Alerte> alertes(const QList<Employe> &employes, const QList<CompetitionGeree> &competitions);
    static QList<LigneRapport> corrigerAlertes();
    static QList<LigneRapport> toutRepartir();

    // Employés de démonstration : nombre ajouté, -1 en cas d'erreur
    static int ajouterDemo();

    // Référentiels
    static QStringList listeRoles();
    static QString libelleRole(const QString &role);
    static QString codeDepuisId(int id);
    static QString sansAccents(const QString &texte);

private:
    static int choisir(const CompetitionGeree &c, const QList<Employe> &employes,
                       const QList<CompetitionGeree> &competitions, int exclu, int prefere);
};

#endif // EMPLOYE_H
