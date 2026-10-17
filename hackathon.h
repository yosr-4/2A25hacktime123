#ifndef HACKATHON_H
#define HACKATHON_H

#include <QMainWindow>
#include <QVector>
#include "competition.h"

QT_BEGIN_NAMESPACE
namespace Ui { class hackathon; }
QT_END_NAMESPACE

class Gparticipant;
class Gequipe;

class hackathon : public QMainWindow {
    Q_OBJECT

public:
    explicit hackathon(QWidget *parent = nullptr);
    ~hackathon() override;
    void setRole(const QString &role);   // appelé après la connexion

signals:
    void loggedOut();                     // l'utilisateur s'est déconnecté

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    // ----- mise en place (une fois, au démarrage)
    void applyRole();          // droits selon le rôle connecté
    void logout();
    bool canEdit() const;      // Administrateur, Organisateur
    bool canDelete() const;    // Administrateur
    void setupNavigation();
    void showModule(int index);   // SWHackTime : 0 = Compétitions, 1 = Participants, 2 = Équipes
    void setupExports();
    void setupTable();
    void setupFilters();
    void setupAddForm();

    // ----- formulaire d'ajout (colonne droite, sous les détails)
    void resetAddForm();
    void showAddError(const QString &msg);
    void submitAddForm();
    Comp formComp() const;
    void updateAddConflicts();   // métier innovant : conflits détectés pendant la saisie
    void showAllConflicts();     // rapport des conflits sur tout le planning
    void openCalendar();         // calendrier des événements (vue par mois)

    // ----- actions sur les compétitions
    void editCompetition(const QString &id);
    void deleteCompetition(const QString &id);
    void selectRow(const QString &id);
    void showDetails();
    void notImplemented(const QString &module);

    // ----- exports (PDF : planning des sponsors ; Excel : liste)
    void exportPdf();
    void exportExcel();
    void showExported(const QString &title, const QString &path);

    // ----- affichage
    void onSelect();
    void onFilter();
    void resetFilters();
    void refreshYears();
    void refresh(bool rebuildYears = true);
    void buildPager(int pages);
    void goTo(int p);
    void updateKpis();
    void updateDetails();
    QWidget *makeBadge(const QString &statut);
    QWidget *makeActions(const QString &id);

    // ----- utilitaires
    QString nextId() const;
    int indexOf(const QString &id) const;
    QVector<Comp> applyFilters() const;

    Ui::hackathon *ui;          // tous les widgets dessinés dans hackathon.ui
    QVector<Comp> data, filtered;
    int page = 0;
    QString selectedId;
    Gparticipant *participants = nullptr;   // module Participants (Yasmine)
    Gequipe *equipes = nullptr;             // module Équipes
    QString role;               // Administrateur, Organisateur ou Jury
};

#endif // HACKATHON_H
