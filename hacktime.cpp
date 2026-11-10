#include "hacktime.h"
#include "ui_hacktime.h"
#include "competition.h"
#include "interface.h"

#include <QAction>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QTableWidgetItem>
#include <QDate>
#include <QEvent>
#include <QGuiApplication>
#include <QIcon>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScreen>

// =====================================================================
// === CONSTRUCTEUR & DESTRUCTEUR ======================================
// =====================================================================

HackTime::HackTime(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HackTime)
    , fondLogin(":/images/login_bg.jpg")
{
    ui->setupUi(this);

    // Integrate the existing Oracle-backed Teams CRUD screen into HackTime.
    // Keep the original page container and navigation; replace only its placeholder content.
    if (ui->pageEquipes) {
        if (QWidget *placeholder = ui->pageEquipes->findChild<QWidget*>("horizontalLayoutWidget"))
            placeholder->hide();
        QVBoxLayout *teamsLayout = qobject_cast<QVBoxLayout*>(ui->pageEquipes->layout());
        if (!teamsLayout) {
            teamsLayout = new QVBoxLayout(ui->pageEquipes);
            teamsLayout->setContentsMargins(0, 0, 0, 0);
            teamsLayout->setSpacing(0);
        }
        teamManagementWidget = new MainWindow(ui->pageEquipes);
        teamManagementWidget->setWindowFlags(Qt::Widget);
        teamsLayout->addWidget(teamManagementWidget);
        teamManagementWidget->show();
    }

    // la fenêtre ne dépasse pas l'écran
    QSize taille(1536, 960);
    if (QScreen *ecran = QGuiApplication::primaryScreen())
        taille = taille.boundedTo(ecran->availableGeometry().size() - QSize(20, 50));
    resize(taille);

    // ----- navigation : boutons des barres latérales de tous les modules
    // Compétitions
    connect(ui->btnCompetitionsComp, &QPushButton::clicked, this, &HackTime::allerCompetitions);
    connect(ui->btnEquipesComp, &QPushButton::clicked, this, &HackTime::allerEquipes);
    connect(ui->btnParticipantsComp, &QPushButton::clicked, this, &HackTime::allerParticipants);
    connect(ui->btnAtelierComp, &QPushButton::clicked, this, &HackTime::allerAteliers);
    connect(ui->btnEmployesComp, &QPushButton::clicked, this, &HackTime::allerEmployes);
    // Équipes
    connect(ui->pushButton_2, &QPushButton::clicked, this, &HackTime::allerCompetitions);
    connect(ui->pushButton_9, &QPushButton::clicked, this, &HackTime::allerEquipes);
    connect(ui->pushButton_8, &QPushButton::clicked, this, &HackTime::allerParticipants);
    connect(ui->pushButton_7, &QPushButton::clicked, this, &HackTime::allerAteliers);
    connect(ui->pushButton_6, &QPushButton::clicked, this, &HackTime::allerEmployes);
    // Participants
    connect(ui->btnCompetitionsPart, &QPushButton::clicked, this, &HackTime::allerCompetitions);
    connect(ui->btnEquipesPart, &QPushButton::clicked, this, &HackTime::allerEquipes);
    connect(ui->btnParticipantsPart, &QPushButton::clicked, this, &HackTime::allerParticipants);
    connect(ui->btnAtelierPart, &QPushButton::clicked, this, &HackTime::allerAteliers);
    connect(ui->btnEmployesPart, &QPushButton::clicked, this, &HackTime::allerEmployes);
    // Ateliers
    connect(ui->btnCompetitionsAtelier, &QPushButton::clicked, this, &HackTime::allerCompetitions);
    connect(ui->btnEquipesAtelier, &QPushButton::clicked, this, &HackTime::allerEquipes);
    connect(ui->btnParticipantsAtelier, &QPushButton::clicked, this, &HackTime::allerParticipants);
    connect(ui->btnAtelierAtelier, &QPushButton::clicked, this, &HackTime::allerAteliers);
    connect(ui->btnEmployesAtelier, &QPushButton::clicked, this, &HackTime::allerEmployes);
    // Employés
    connect(ui->boutonCompetitions, &QPushButton::clicked, this, &HackTime::allerCompetitions);
    connect(ui->boutonEquipes, &QPushButton::clicked, this, &HackTime::allerEquipes);
    connect(ui->boutonParticipants, &QPushButton::clicked, this, &HackTime::allerParticipants);
    connect(ui->boutonAtelier, &QPushButton::clicked, this, &HackTime::allerAteliers);
    connect(ui->boutonEmployes, &QPushButton::clicked, this, &HackTime::allerEmployes);

    initCompetition();
    initEquipe();
    initParticipant();
    initAtelier();
    initEmploye();

    allerLogin();
}

HackTime::~HackTime()
{
    delete ui;
    ui = nullptr;
}

// =====================================================================
// === SLOTS GÉNÉRAUX (NAVIGATION ENTRE PAGES) =========================
// =====================================================================

void HackTime::allerLogin()
{
    ui->stackedHackTime->setCurrentWidget(ui->pageLogin);
    setWindowTitle("HackTime - Connexion");
}

void HackTime::allerCompetitions()
{
    ui->stackedHackTime->setCurrentWidget(ui->pageCompetitions);
    setWindowTitle("HackTime - Compétitions");
}

void HackTime::allerEquipes()
{
    ui->stackedHackTime->setCurrentWidget(ui->pageEquipes);
    setWindowTitle("HackTime - Équipes");
}

void HackTime::allerParticipants()
{
    ui->stackedHackTime->setCurrentWidget(ui->pageParticipants);
    setWindowTitle("HackTime - Participants");
}

void HackTime::allerAteliers()
{
    ui->stackedHackTime->setCurrentWidget(ui->pageAteliers);
    setWindowTitle("HackTime - Ateliers");
}

void HackTime::allerEmployes()
{
    ui->stackedHackTime->setCurrentWidget(ui->pageEmployes);
    setWindowTitle("HackTime - Employés");
}

// Événements communs : chaque module ajoute ses cas dans sa zone ci-dessous.
bool HackTime::eventFilter(QObject *obj, QEvent *ev)
{
    if (!ui) return false;   // fenêtre en cours de fermeture

    // MODULE COMPETITION : fond de la page de connexion, sans déformation
    if (obj == ui->pageLogin && ev->type() == QEvent::Paint) {
        QWidget *p = ui->pageLogin;
        const QSize voulue = fondLogin.size().scaled(p->size(), Qt::KeepAspectRatioByExpanding);
        if (fondLoginAjuste.size() != voulue)
            fondLoginAjuste = fondLogin.scaled(p->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPainter painter(p);
        painter.drawPixmap((p->width() - fondLoginAjuste.width()) / 2,
                           (p->height() - fondLoginAjuste.height()) / 2, fondLoginAjuste);
        return true;
    }
    return QMainWindow::eventFilter(obj, ev);
}

// =====================================================================
// === MODULE COMPETITION ==============================================
// =====================================================================

// 💡 Étudiant responsable du module Compétition travaille ici uniquement
//    Fonctionnel : CRUD (Ajouter, Afficher, Modifier, Supprimer) sur la table
//    COMPETITION (+ GERER) avec la classe métier Competition.
//    Les autres boutons de la page sont seulement affichés.

// libellés affichés dans les listes déroulantes
static const QStringList THEMES_COMP = {"Environnement", "Santé", "Éducation", "Ville intelligente",
                                        "Agriculture", "Intelligence artificielle", "Développement",
                                        "Finance", "Autre"};
static const QStringList MODES_COMP = {"Présentiel", "En ligne", "Hybride"};
static const QStringList STATUTS_COMP = {"Planifiée", "En cours", "Terminée"};

static QString couleurFondStatutComp(const QString &s)
{
    if (s == "En cours") return "#DDF3E4";
    if (s == "Terminée") return "#E5E9F0";
    return "#DCE9FF";
}
static QString couleurTexteStatutComp(const QString &s)
{
    if (s == "En cours") return "#2E9E5B";
    if (s == "Terminée") return "#64748B";
    return "#2563EB";
}

void HackTime::initCompetition()
{
    // ----- page de connexion
    fondLogin = QPixmap(":/images/login_bg.jpg");
    ui->pageLogin->installEventFilter(this);          // image de fond (eventFilter)
    ui->lblErrLoginCompet->hide();
    QAction *oeil = ui->EditPWDCompet->addAction(QIcon(":/images/eye_off.png"), QLineEdit::TrailingPosition);
    connect(oeil, &QAction::triggered, this, [this, oeil]() {
        const bool cache = ui->EditPWDCompet->echoMode() == QLineEdit::Password;
        ui->EditPWDCompet->setEchoMode(cache ? QLineEdit::Normal : QLineEdit::Password);
        oeil->setIcon(QIcon(cache ? ":/images/eye.png" : ":/images/eye_off.png"));
    });
    connect(ui->EditLoginCompet, &QLineEdit::returnPressed, this, [this]() { ui->EditPWDCompet->setFocus(); });
    connect(ui->EditPWDCompet, &QLineEdit::returnPressed, ui->BLoginCompet, &QPushButton::click);

    // ----- page Compétitions
    ui->SWCompet->setCurrentIndex(0);
    ui->adminComp->setText("👤  <b>Administrateur</b>  ▾");
    ui->aTheme->addItems(THEMES_COMP);
    ui->aMode->addItems(MODES_COMP);
    ui->aStatut->addItems(STATUTS_COMP);
    ui->fStatutComp->addItem("Tous");  ui->fStatutComp->addItems(STATUTS_COMP);
    ui->fTheme->addItem("Tous");       ui->fTheme->addItems(THEMES_COMP);
    ui->fDateComp->addItem("Toutes");
    ui->fSortComp->addItems({"Date début ↑", "Date début ↓", "Date fin ↑", "Date fin ↓", "Nom (A → Z)"});
    for (QLineEdit *e : {ui->searchComp, ui->topSearchComp})
        e->setPlaceholderText("Rechercher un nom de compétition");
    ui->aSalleComp->addItems({"Amphi 1", "Amphi 2", "Salle A", "Salle B", "Lab Innovation", "Espace Coworking"});
    ui->aConflictComp->hide();
    ui->aErrComp->hide();
    ui->pagerBoxComp->hide();

    // nom : lettres seulement (accents, espaces, tirets, apostrophes)
    ui->aNom->setValidator(new QRegularExpressionValidator(QRegularExpression("[\\p{L} '\\-]{0,100}"), ui->aNom));

    // l'ID existe dans la base mais n'est pas affiché
    ui->aIdLabelComp->hide();
    ui->aIdComp->hide();
    ui->dIdComp->hide();

    // tableau : ID (caché), Nom, Thème, Début, Fin, Mode, Membres max, Statut, Actions
    QTableWidget *t = ui->tableComp;
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    t->setColumnHidden(0, true);
    connect(t, &QTableWidget::itemSelectionChanged, this, [this]() {
        const int r = ui->tableComp->currentRow();
        if (r >= 0 && ui->tableComp->item(r, 0))
            afficherDetailsComp(ui->tableComp->item(r, 0)->text().toInt());
    });

    on_btnClearComp_clicked();
}

// ---------------------------------------------------------------- Connexion
// aucune vérification : n'importe quel nom et mot de passe ouvrent l'application
void HackTime::on_BLoginCompet_clicked()
{
    allerCompetitions();
    afficherComp();
}

// ---------------------------------------------------------------- AFFICHER
void HackTime::afficherComp()
{
    listeComp = Competition::lister();          // SELECT dans la table COMPETITION
    QTableWidget *t = ui->tableComp;
    t->clearContents();
    t->setRowCount(listeComp.size());
    int enCours = 0, planifiees = 0, terminees = 0, places = 0;
    for (int r = 0; r < listeComp.size(); ++r) {
        const Competition &c = listeComp.at(r);
        t->setItem(r, 0, new QTableWidgetItem(QString::number(c.getId())));
        t->setItem(r, 1, new QTableWidgetItem(c.getNom()));
        t->setItem(r, 2, new QTableWidgetItem(c.getTheme()));
        t->setItem(r, 3, new QTableWidgetItem(c.getDateDebut().toString("dd/MM/yyyy")));
        t->setItem(r, 4, new QTableWidgetItem(c.getDateFin().toString("dd/MM/yyyy")));
        t->setItem(r, 5, new QTableWidgetItem(c.getMode()));
        QTableWidgetItem *max = new QTableWidgetItem(QString::number(c.getNbMaxMembres()));
        max->setTextAlignment(Qt::AlignCenter);
        t->setItem(r, 6, max);

        // statut en badge de couleur
        QLabel *badge = new QLabel(c.getStatut());
        badge->setAlignment(Qt::AlignCenter);
        badge->setStyleSheet(QString("background:%1;color:%2;border-radius:11px;padding:3px 12px;font-size:12px;")
                                 .arg(couleurFondStatutComp(c.getStatut()), couleurTexteStatutComp(c.getStatut())));
        t->setCellWidget(r, 7, badge);

        // ✏️ modifier   🗑️ supprimer
        QWidget *actions = new QWidget;
        QHBoxLayout *l = new QHBoxLayout(actions);
        l->setContentsMargins(0, 0, 0, 0);
        QPushButton *modif = new QPushButton("✏️"), *suppr = new QPushButton("🗑️");
        modif->setObjectName("act");
        suppr->setObjectName("act");
        modif->setToolTip("Modifier");
        suppr->setToolTip("Supprimer");
        l->addWidget(modif);
        l->addWidget(suppr);
        const int id = c.getId();
        connect(modif, &QPushButton::clicked, this, [this, id]() { chargerFormulaireComp(id); });
        connect(suppr, &QPushButton::clicked, this, [this, id]() { supprimerComp(id); });
        t->setCellWidget(r, 8, actions);

        places += c.getNbMaxMembres();
        if (c.getStatut() == "En cours") ++enCours;
        else if (c.getStatut() == "Terminée") ++terminees;
        else ++planifiees;
    }
    ui->footerComp->setText(QString("%1 compétition(s)").arg(listeComp.size()));

    // chiffres affichés (lecture seule)
    ui->kpiTotalComp->setText(QString::number(listeComp.size()));
    ui->kpiEquipes->setText(QString::number(places));
    ui->kpiPart->setText(QString::number(places * 4));
    ui->kpiCours->setText(QString::number(enCours));
    ui->legCoursComp->setText(QString::number(enCours));
    ui->legPlanifiee->setText(QString::number(planifiees));
    ui->legTerminee->setText(QString::number(terminees));

    if (listeComp.isEmpty()) afficherDetailsComp(0);
    else t->selectRow(0);
}

void HackTime::afficherDetailsComp(int id)
{
    for (const Competition &c : listeComp) {
        if (c.getId() != id) continue;
        ui->dNomComp->setText(c.getNom());
        ui->dBadgeComp->setText(c.getStatut());
        ui->dBadgeComp->setStyleSheet(QString("background:%1;color:%2;border-radius:10px;padding:2px 10px;font-size:11px;")
                                          .arg(couleurFondStatutComp(c.getStatut()), couleurTexteStatutComp(c.getStatut())));
        ui->dTheme->setText(c.getTheme());
        ui->dDebut->setText(c.getDateDebut().toString("dd/MM/yyyy"));
        ui->dFin->setText(c.getDateFin().toString("dd/MM/yyyy"));
        ui->dMode->setText(c.getMode());
        ui->dMax->setText(QString::number(c.getNbMaxMembres()));
        return;
    }
    ui->dNomComp->setText("Aucune sélection");
    ui->dBadgeComp->clear();
    ui->dBadgeComp->setStyleSheet("");
    for (QLabel *v : {ui->dTheme, ui->dDebut, ui->dFin, ui->dMode, ui->dMax})
        v->setText("-");
}

// ---------------------------------------------------------------- AJOUTER / MODIFIER
void HackTime::on_btnSaveComp_clicked()
{
    Competition c(ui->aNom->text(), ui->aTheme->currentText(), ui->aDebut->date(), ui->aFin->date(),
                  ui->aMax->value(), ui->aStatut->currentText(), ui->aMode->currentText());
    c.setId(idEnModificationComp);

    const QString erreur = c.verifier();      // contrôle de saisie (nom unique, lettres, dates…)
    if (!erreur.isEmpty()) {
        ui->aErrComp->setText(erreur);
        ui->aErrComp->show();
        return;
    }
    ui->aErrComp->hide();

    const bool ajout = (idEnModificationComp == 0);
    const bool ok = ajout ? c.ajouter() : c.modifier();     // INSERT ou UPDATE
    if (!ok) {
        QMessageBox::critical(this, ajout ? "Ajouter" : "Modifier",
                              "Opération impossible :\n" + Competition::derniereErreur());
        return;
    }
    QMessageBox::information(this, ajout ? "Ajouter" : "Modifier",
                             ajout ? "Compétition ajoutée." : "Compétition modifiée.");
    on_btnClearComp_clicked();
    afficherComp();
}

void HackTime::on_btnClearComp_clicked()
{
    idEnModificationComp = 0;
    ui->lblAddComp->setText("Ajouter une compétition");
    ui->btnSaveComp->setText("Enregistrer");
    ui->aNom->clear();
    ui->aTheme->setCurrentIndex(0);
    ui->aDebut->setDate(QDate::currentDate());
    ui->aFin->setDate(QDate::currentDate().addDays(2));
    ui->aMode->setCurrentIndex(0);
    ui->aMax->setValue(5);
    ui->aStatut->setCurrentIndex(0);
    ui->aErrComp->hide();
}

// ✏️ : la compétition est chargée dans le formulaire, le bouton devient « Modifier »
void HackTime::chargerFormulaireComp(int id)
{
    for (const Competition &c : listeComp) {
        if (c.getId() != id) continue;
        idEnModificationComp = id;
        ui->lblAddComp->setText("Modifier la compétition");
        ui->btnSaveComp->setText("Modifier");
        ui->aNom->setText(c.getNom());
        ui->aTheme->setCurrentText(c.getTheme());
        ui->aDebut->setDate(c.getDateDebut());
        ui->aFin->setDate(c.getDateFin());
        ui->aMode->setCurrentText(c.getMode());
        ui->aMax->setValue(c.getNbMaxMembres());
        ui->aStatut->setCurrentText(c.getStatut());
        ui->aErrComp->hide();
        ui->rightScrollComp->ensureWidgetVisible(ui->cardAddComp);
        ui->aNom->setFocus();
        return;
    }
}

// ---------------------------------------------------------------- SUPPRIMER
void HackTime::supprimerComp(int id)
{
    if (QMessageBox::question(this, "Supprimer", "Supprimer cette compétition ?") != QMessageBox::Yes)
        return;
    if (!Competition::supprimer(id)) {           // DELETE (GERER suit par ON DELETE CASCADE)
        QMessageBox::critical(this, "Supprimer", "Suppression impossible :\n" + Competition::derniereErreur());
        return;
    }
    if (idEnModificationComp == id) on_btnClearComp_clicked();
    QMessageBox::information(this, "Supprimer", "Compétition supprimée.");
    afficherComp();
}

// =====================================================================
// === MODULE EQUIPE ===================================================
// =====================================================================

// 💡 Étudiant responsable du module Équipe écrit ici

void HackTime::initEquipe()
{
    // Le panneau droit (ajout, statistiques, détails) est maintenant dans hacktime.ui :
    // panneauDroitEquipes. Les graphiques sont à placer dans zoneCourbeEquipe et zoneBarresEquipe.
    ui->comboNouvelleCompetitionEquipe->addItems({"Compétition 1", "Compétition 2"});
    ui->comboNouvelleCompetitionEquipe->setCurrentIndex(-1);
    ui->editDateEquipe->setText(QDate::currentDate().toString("dd/MM/yyyy"));
    connect(ui->btnEffacerEquipe, &QPushButton::clicked, this, [this]() {
        ui->editNomEquipe->clear();
        ui->editDateEquipe->setText(QDate::currentDate().toString("dd/MM/yyyy"));
        ui->comboNouvelleCompetitionEquipe->setCurrentIndex(-1);
    });
}

// =====================================================================
// === MODULE PARTICIPANT ==============================================
// =====================================================================

// 💡 Étudiant responsable du module Participant écrit ici

void HackTime::initParticipant()
{
}

// =====================================================================
// === MODULE ATELIER ==================================================
// =====================================================================

// 💡 Étudiant responsable du module Atelier écrit ici

void HackTime::initAtelier()
{
}

// =====================================================================
// === MODULE EMPLOYE ==================================================
// =====================================================================

// 💡 Étudiant responsable du module Employé écrit ici

void HackTime::initEmploye()
{
}
