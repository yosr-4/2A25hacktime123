#ifndef PARTICIPANTUI_H
#define PARTICIPANTUI_H

// Module "Gestion des Participants" : interface (page pageParticipants de hacktime.ui).
// Les boutons / champs de la page portent le suffixe "Part" (tablePart, searchBoxPart, btnPdfPart...).
// Création : HackTime::initParticipant() -> new ModuleParticipants(ui, this)

#include <QObject>
#include <QList>
#include <QString>
#include "participant.h"

class QTableWidget;
class QLabel;
class QHBoxLayout;
class QWidget;
class QEvent;
class BornePointage;

namespace Ui { class HackTime; }

class ModuleParticipants : public QObject
{
    Q_OBJECT

public:
    ModuleParticipants(Ui::HackTime *interface, QWidget *fenetre);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onAjouterParticipantClicked();
    void onModifierParticipantClicked(int row);
    void onVoirParticipantClicked(int row);
    void onSupprimerParticipantClicked(int row);
    void exporterPdf();
    void exporterExcel();
    void onPageClicked();
    void onPrevPageClicked();
    void onNextPageClicked();

private:
    Ui::HackTime *ui;
    QWidget *m_page;                    // pageParticipants (parent des fenêtres du module)

    QTableWidget *table = nullptr;
    QLabel *lblActifs = nullptr;
    QLabel *lblNouveaux = nullptr;
    QHBoxLayout *paginationLayout = nullptr;
    QString m_styleBase;

    // ---- Scénario Arduino : barrière d'accès (participantborne.h) ----
    BornePointage *borne = nullptr;
    QLabel *lblBorneEtat = nullptr;
    QLabel *lblBornePresents = nullptr;
    QLabel *lblBorneMessage = nullptr;

    QList<InfoParticipant> m_liste;     // participants actuellement affichés (après recherche et tri)

    int nouveauxInscritsCount = 0;
    int currentPage = 1;
    int itemsPerPage = 5;

    void setupCustomUi();
    int ligneSelectionnee() const;
    const InfoParticipant *participantDeLigne(int row) const;
    QWidget* createActionButtonsWidget(int row);
    void updateCounters();
    void chargerDonneesSQL();
    void showPage(int page);
    void updatePagination();
    void applyStyleSheet();

    void creerCarteBorne();
    void majCarteBorne();
    void selectionnerFraction(double fraction);   // potentiomètre : choisit une ligne du tableau
    void pointerSelection(bool depuisBorne);      // bouton ENTRER (ou bouton de test) : pointage via Oracle
    void selectionnerParId(const QString &id);
};

#endif // PARTICIPANTUI_H
