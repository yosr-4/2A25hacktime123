#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QList>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSoundEffect>
#include <QEvent>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onAjouterParticipantClicked();
    void onActionRapideClicked();
    void onPageClicked();
    void onPrevPageClicked();
    void onNextPageClicked();
    void onVoirParticipantClicked(int row);
    void onModifierParticipantClicked(int row);
    void onSupprimerParticipantClicked(int row);

private:
    QTableWidget *table = nullptr;
    QLabel *lblActifs = nullptr;
    QLabel *lblNouveaux = nullptr;
    QLabel *logoLabel = nullptr;
    QHBoxLayout *paginationLayout = nullptr;

    QSoundEffect *clickSound = nullptr;

    int nouveauxInscritsCount = 0;
    int currentPage = 1;
    int itemsPerPage = 5;

    void setupCustomUi();
    void applyStyleSheet();
    QWidget* createActionButtonsWidget(int row);
    void updateCounters();
    void updatePagination();
    void showPage(int page);
    void setupLogoAnimation();
    void setupSoundEffects();
    void playClickSound();
};

#endif