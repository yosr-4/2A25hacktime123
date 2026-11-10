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

QT_BEGIN_NAMESPACE
namespace Ui { class HackTime; }
class MainWindow;
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

private:
    // ----- initialisation de chaque module (une méthode par module)
    void initCompetition();
    void initEquipe();
    void initParticipant();
    void initAtelier();
    void initEmploye();

    Ui::HackTime *ui;
    MainWindow *teamManagementWidget = nullptr;

    // ----- MODULE COMPETITION
    void afficherComp();                 // AFFICHER : remplit le tableau depuis Oracle
    void afficherDetailsComp(int id);
    void chargerFormulaireComp(int id);  // ✏️ MODIFIER : charge la compétition dans le formulaire
    void supprimerComp(int id);          // 🗑️ SUPPRIMER
    QVector<Competition> listeComp;      // compétitions lues dans la base
    int idEnModificationComp = 0;        // 0 = ajout, sinon ID modifié
    QPixmap fondLogin, fondLoginAjuste;  // fond de la page de connexion
};

#endif // HACKTIME_H
