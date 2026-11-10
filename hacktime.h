// =====================================================================
//  HackTime : fenêtre principale du projet intégré (une seule classe
//  d'interface, un seul fichier hacktime.ui).
//  stackedHackTime (stackedWidget principal) :
//    0 pageLogin         : connexion
//    1 pageCompetitions  : Gestion des compétitions
//    2 pageEquipes       : Gestion des équipes
//    3 pageParticipants  : Gestion des participants
//    4 pageAteliers      : Gestion des ateliers
//    5 pageEmployes      : Gestion des employés
//  Chaque étudiant écrit UNIQUEMENT dans la zone de son module.
// =====================================================================
#ifndef HACKTIME_H
#define HACKTIME_H

#include <QMainWindow>
#include <QPixmap>
#include <QVector>
#include "competition.h"
#include "employe.h"

#include <QList>
#include <QMap>

class QLabel;
class QMenu;
class QTimer;
class StatsWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class HackTime; }
QT_END_NAMESPACE

class HackTime : public QMainWindow
{
    Q_OBJECT

public:
    HackTime(QWidget *parent = nullptr);
    ~HackTime();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private slots:

    // =================================================================
    // === SLOTS GÉNÉRAUX (NAVIGATION ENTRE PAGES) =====================
    // =================================================================
    void allerLogin();
    void allerCompetitions();
    void allerEquipes();
    void allerParticipants();
    void allerAteliers();
    void allerEmployes();

    // =================================================================
    // === MODULE COMPETITION ==========================================
    // =================================================================

    // 💡 Étudiant responsable du module Compétition travaille ici uniquement

    void on_BLoginCompet_clicked();     // « Se connecter »
    void on_btnSaveComp_clicked();      // AJOUTER / MODIFIER
    void on_btnClearComp_clicked();     // vider le formulaire

    // =================================================================
    // === MODULE EQUIPE ===============================================
    // =================================================================

    // 💡 Étudiant responsable du module Équipe écrit ici

    // =================================================================
    // === MODULE PARTICIPANT ==========================================
    // =================================================================

    // 💡 Étudiant responsable du module Participant écrit ici

    // =================================================================
    // === MODULE ATELIER ==============================================
    // =================================================================

    // 💡 Étudiant responsable du module Atelier écrit ici

    // =================================================================
    // === MODULE EMPLOYE ==============================================
    // =================================================================

    // 💡 Étudiant responsable du module Employé écrit ici

    // Employés (CRUD)
    void enregistrerEmploye();
    void effacerFormulaireEmp();
    void modifierEmploye(int id);
    void supprimerEmploye(int id);
    void selectionnerEmploye(int id);

    // Liste
    void rafraichirListeEmp();
    void changerPageEmp(int decalage);
    void trierParColonneEmp(int colonne);
    void exporterPdfEmp();
    void exporterExcelEmp();

    // Charge et répartition des compétitions
    void corrigerAlertesEmp();
    void toutRepartirEmp();
    void changerResponsableEmp(int idCompetition, int idEmploye);

    void ajouterEmployesDemo();

private:
    // ----- initialisation de chaque module (une méthode par module)
    void initCompetition();
    void initEquipe();
    void initParticipant();
    void initAtelier();
    void initEmploye();

    Ui::HackTime *ui;

    // ----- MODULE COMPETITION
    void afficherComp();                 // AFFICHER : remplit le tableau depuis Oracle
    void afficherDetailsComp(int id);
    void chargerFormulaireComp(int id);  // ✏️ MODIFIER : charge la compétition dans le formulaire
    void supprimerComp(int id);          // 🗑️ SUPPRIMER
    QVector<Competition> listeComp;      // compétitions lues dans la base
    int idEnModificationComp = 0;        // 0 = ajout, sinon ID modifié
    QPixmap fondLogin, fondLoginAjuste;  // fond de la page de connexion

    // ----- MODULE EMPLOYE
    void preparerBarreHauteEmp();
    void preparerOngletListeEmp();
    void preparerOngletStatistiquesEmp();
    void preparerOngletChargeEmp();
    void preparerOngletAffectationEmp();
    void preparerCarteDetailsEmp();
    void preparerCarteFormulaireEmp();
    void rafraichirToutEmp();
    void rafraichirIndicateursEmp();
    void rafraichirStatistiquesEmp();
    void rafraichirChargeEmp();
    void rafraichirAffectationEmp();
    void rafraichirDetailsEmp();
    void remplirFormulaireEmp(const Employe *e);
    void messageEmp(const QString &texte, bool erreur = false);
    void signalerNouvellesAlertesEmp(const QList<int> &avant);
    void afficherRapportEmp(const QList<LigneRapport> &rapport);
    QList<Employe> listeFiltreeEmp() const;
    QMap<int, int> chargeParEmploye() const;
    QList<int> idsEnAlerte() const;
    QString categorieCharge(const Employe &e) const;
    const Employe *employeParId(int id) const;

    Employe Etmp;                               // objet utilisé pour appeler supprimer()
    QList<Employe> m_employes;                  // employés lus dans la base
    QList<CompetitionGeree> m_competitionsEmp;  // compétitions et leurs responsables (GERER)
    QList<LigneRapport> m_rapportEmp;           // compte rendu de la dernière répartition
    bool m_rapportVisibleEmp = false;
    int m_selectionEmp = 0;                     // employé affiché dans « Détails »
    int m_editionEmp = 0;                       // employé en cours de modification (0 = ajout)
    int m_pageEmp = 0;
    Employe::Tri m_triEmp = Employe::ParId;
    bool m_croissantEmp = true;
    bool m_statsParChargeEmp = false;
    QString m_utilisateurEmp = "Administrateur";
    QString m_loginSuggereEmp;                  // login proposé à partir de l'e-mail
    QMenu *m_menuAlertesEmp = nullptr;          // alertes de la cloche
    QLabel *m_toastEmp = nullptr;               // message temporaire en bas de la page
    QTimer *m_toastTimerEmp = nullptr;
    StatsWidget *m_statsEmp = nullptr;          // graphique de l'onglet Statistiques
};

#endif // HACKTIME_H
