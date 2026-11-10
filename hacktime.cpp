#include "hacktime.h"
#include "ui_hacktime.h"
#include "competition.h"
#include "employe.h"
#include "statsemploye.h"

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
#include <QScreen>
// module Employés
#include <QActionGroup>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QPageLayout>
#include <QPageSize>
#include <QPrinter>
#include <QProgressBar>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>

// =====================================================================
// === CONSTRUCTEUR & DESTRUCTEUR ======================================
// =====================================================================

HackTime::HackTime(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HackTime)
    , fondLogin(":/images/login_bg.jpg")
{
    ui->setupUi(this);

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

// ======================================================================
//  Petites fonctions utilitaires pour l'affichage
// ======================================================================
namespace {

const int LIGNES_PAR_PAGE_EMP = 8;

// Colonnes de la liste des employés
enum { ColEmpId, ColEmpNom, ColEmpPrenom, ColEmpEmail, ColEmpRole, ColEmpLogin, ColEmpCompetitions, ColEmpCharge, ColEmpActions };

// Tri associé à chaque colonne de la liste
const Employe::Tri TRIS_EMP[] = {Employe::ParId, Employe::ParNom, Employe::ParPrenom, Employe::ParEmail,
                             Employe::ParRole, Employe::ParLogin, Employe::ParCompetitions,
                             Employe::ParCompetitions};

// Catégories du filtre « Charge » et des statistiques par charge
const char *const CATEGORIES_CHARGE_EMP[] = {"Libre", "Normal", "Complet", "En alerte", "Non concerné"};

QToolButton *boutonIconeEmp(const QString &icone, const QString &infobulle)
{
    QToolButton *b = new QToolButton;
    b->setObjectName("icone");
    b->setText(icone);
    b->setToolTip(infobulle);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

QString couleurRoleEmp(const QString &role)
{
    if (role == "ADMINISTRATEUR")
        return "bleu";
    if (role == "JURY")
        return "orange";
    return "vert";
}

QString couleurStatutEmp(const QString &statut)
{
    if (statut == "EN_COURS")
        return "vert";
    if (statut == "PLANIFIEE")
        return "bleu";
    return "gris";
}

QString stylePastilleEmp(const QString &couleur)
{
    QString fond = "#e5e9f1";
    QString encre = "#5d6880";
    if (couleur == "vert") {
        fond = "#dcf5e5"; encre = "#1c9552";
    } else if (couleur == "orange") {
        fond = "#fff0dc"; encre = "#a85f00";
    } else if (couleur == "bleu") {
        fond = "#e2ebff"; encre = "#1a5ae8";
    } else if (couleur == "rouge") {
        fond = "#fde5e5"; encre = "#d33a3a";
    }
    return QString("background:%1;color:%2;border-radius:8px;padding:4px 12px;font-weight:bold;")
        .arg(fond, encre);
}

QLabel *pastilleEmp(const QString &libelle, const QString &couleur)
{
    QLabel *label = new QLabel(libelle);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(stylePastilleEmp(couleur));
    return label;
}

// Place un widget dans une cellule de tableau, aligné à gauche.
QWidget *dansCelluleEmp(QWidget *w)
{
    QWidget *conteneur = new QWidget;
    QHBoxLayout *l = new QHBoxLayout(conteneur);
    l->setContentsMargins(8, 4, 8, 4);
    l->addWidget(w);
    l->addStretch();
    return conteneur;
}

// Le tableau défile horizontalement au lieu d'élargir la fenêtre au-delà de
// l'écran ; les colonnes à pastille ont une largeur fixe (la pastille est un
// widget, ResizeToContents ne la voit pas).
void tableauSoupleEmp(QTableWidget *table, const QList<QPair<int, int> > &largeursFixes)
{
    table->setSizePolicy(QSizePolicy::Ignored, table->sizePolicy().verticalPolicy());
    table->setMinimumWidth(0);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    for (int i = 0; i < largeursFixes.size(); ++i) {
        table->horizontalHeader()->setSectionResizeMode(largeursFixes.at(i).first, QHeaderView::Fixed);
        table->horizontalHeader()->resizeSection(largeursFixes.at(i).first, largeursFixes.at(i).second);
    }
}

QTableWidgetItem *celluleEmp(const QString &contenu)
{
    QTableWidgetItem *item = new QTableWidgetItem(contenu);
    item->setToolTip(contenu);
    return item;
}

// Barre de charge : verte, orange quand le maximum est atteint, rouge en alerte.
void reglerBarreEmp(QProgressBar *barre, int actives, bool alerte)
{
    const int pourcentage = actives * 100 / Employe::MAX_ACTIVES;
    QString couleur = "#1c9552";
    if (alerte)
        couleur = "#d33a3a";
    else if (actives >= Employe::MAX_ACTIVES)
        couleur = "#ff8c1a";

    barre->setRange(0, 100);
    barre->setValue(qMin(100, pourcentage));
    barre->setTextVisible(false);
    barre->setFixedHeight(12);
    barre->setStyleSheet(QString("QProgressBar{border:none;background:#eef2f9;border-radius:6px;}"
                                 "QProgressBar::chunk{background:%1;border-radius:6px;}").arg(couleur));
}

QString texteChargeEmp(int actives)
{
    return QString("%1 / %2").arg(actives).arg(Employe::MAX_ACTIVES);
}

bool chargePlusForteEmp(const QPair<int, int> &a, const QPair<int, int> &b)
{
    if (a.first != b.first)
        return a.first > b.first;
    return a.second < b.second;
}

QString champCsvEmp(const QString &valeur)
{
    QString v = valeur;
    v.replace("\"", "\"\"");
    return "\"" + v + "\"";
}

} // namespace

// ---------------------------------------------------------------- Initialisation du module
void HackTime::initEmploye()
{
    // Ce que hacktime.ui ne décrit pas : menus, listes fournies par les
    // classes métier, largeur des colonnes, graphique et signaux.
    preparerBarreHauteEmp();
    preparerOngletListeEmp();
    preparerOngletStatistiquesEmp();
    preparerOngletChargeEmp();
    preparerOngletAffectationEmp();
    preparerCarteDetailsEmp();
    preparerCarteFormulaireEmp();

    // Graphique des statistiques, placé dans le cadre « stats » de hacktime.ui
    m_statsEmp = new StatsWidget(ui->stats);
    QVBoxLayout *dispositionStats = new QVBoxLayout(ui->stats);
    dispositionStats->setContentsMargins(0, 0, 0, 0);
    dispositionStats->addWidget(m_statsEmp);

    // Message temporaire affiché en bas de la page
    m_toastEmp = new QLabel(ui->pageEmployes);
    m_toastEmp->hide();
    m_toastTimerEmp = new QTimer(this);
    m_toastTimerEmp->setSingleShot(true);
    connect(m_toastTimerEmp, &QTimer::timeout, m_toastEmp, &QWidget::hide);

    m_employes = Employe::tous();
    if (!m_employes.isEmpty())
        m_selectionEmp = m_employes.first().getId();
    rafraichirToutEmp();
    remplirFormulaireEmp(nullptr);
}
// ---------------------------------------------------------------- Barre du haut
void HackTime::preparerBarreHauteEmp()
{
    // Cloche : alertes de charge
    m_menuAlertesEmp = new QMenu(this);
    ui->cloche->setMenu(m_menuAlertesEmp);

    // Utilisateur connecté : Administrateur ou Responsable RH
    QMenu *menu = new QMenu(this);
    QActionGroup *groupe = new QActionGroup(menu);
    const QStringList roles = QStringList() << "Administrateur" << "Responsable RH";
    for (int i = 0; i < roles.size(); ++i) {
        const QString role = roles.at(i);
        QAction *action = menu->addAction(role);
        action->setCheckable(true);
        action->setChecked(i == 0);
        groupe->addAction(action);
        connect(action, &QAction::triggered, this, [this, role]() {
            m_utilisateurEmp = role;
            ui->boutonUtilisateur->setText(role == "Administrateur" ? "👤  Admin  ▾" : "👤  Resp. RH  ▾");
            messageEmp("Connecté en tant que " + role + ".");
        });
    }
    menu->addSeparator();
    QAction *demo = menu->addAction("Ajouter des employés de démonstration");
    connect(demo, &QAction::triggered, this, &HackTime::ajouterEmployesDemo);
    ui->boutonUtilisateur->setMenu(menu);

    connect(ui->rechercheHaut, &QLineEdit::textChanged, this, [this](const QString &valeur) {
        {
            const QSignalBlocker bloqueur(ui->recherche);
            ui->recherche->setText(valeur);
        }
        ui->onglets->setCurrentIndex(0);
        m_pageEmp = 0;
        rafraichirListeEmp();
    });
}

// ---------------------------------------------------------------- Onglet « Liste »
void HackTime::preparerOngletListeEmp()
{
    ui->triCombo->addItem("Ordre d'ajout", int(Employe::ParId));
    ui->triCombo->addItem("Nom", int(Employe::ParNom));
    ui->triCombo->addItem("Rôle", int(Employe::ParRole));
    ui->triCombo->addItem("Login", int(Employe::ParLogin));
    ui->triCombo->addItem("Compétitions gérées", int(Employe::ParCompetitions));

    ui->filtreRole->addItem("Tous");
    const QStringList roles = Employe::listeRoles();
    for (int i = 0; i < roles.size(); ++i)
        ui->filtreRole->addItem(Employe::libelleRole(roles.at(i)), roles.at(i));

    ui->filtreCharge->addItem("Toutes");
    for (int i = 0; i < 5; ++i)
        ui->filtreCharge->addItem(QString::fromUtf8(CATEGORIES_CHARGE_EMP[i]));

    // Largeur des colonnes et tri par clic sur l'en-tête
    QHeaderView *enTete = ui->tableEmployes->horizontalHeader();
    enTete->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    enTete->setSectionResizeMode(QHeaderView::ResizeToContents);
    enTete->setSectionResizeMode(ColEmpEmail, QHeaderView::Stretch);
    tableauSoupleEmp(ui->tableEmployes, QList<QPair<int, int> >() << qMakePair(int(ColEmpRole), 150)
                                                              << qMakePair(int(ColEmpCharge), 150));
    // L'identifiant reste stocké dans la colonne cachée (sélection), mais n'est pas affiché
    ui->tableEmployes->setColumnHidden(ColEmpId, true);
    enTete->setSectionsClickable(true);
    enTete->setSortIndicatorShown(true);

    // Signaux
    connect(ui->boutonPdf, &QPushButton::clicked, this, &HackTime::exporterPdfEmp);
    connect(ui->boutonExcel, &QPushButton::clicked, this, &HackTime::exporterExcelEmp);
    connect(ui->recherche, &QLineEdit::textChanged, this, [this](const QString &valeur) {
        {
            const QSignalBlocker bloqueur(ui->rechercheHaut);
            ui->rechercheHaut->setText(valeur);
        }
        m_pageEmp = 0;
        rafraichirListeEmp();
    });
    QComboBox *filtres[] = {ui->filtreRole, ui->filtreCharge};
    for (int i = 0; i < 2; ++i) {
        connect(filtres[i], QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
            m_pageEmp = 0;
            rafraichirListeEmp();
        });
    }
    connect(ui->triCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_triEmp = Employe::Tri(ui->triCombo->itemData(index).toInt());
        m_croissantEmp = m_triEmp != Employe::ParCompetitions;
        rafraichirListeEmp();
    });
    connect(enTete, &QHeaderView::sectionClicked, this, &HackTime::trierParColonneEmp);
    connect(ui->tableEmployes, &QTableWidget::itemSelectionChanged, this, [this]() {
        const QList<QTableWidgetItem *> selection = ui->tableEmployes->selectedItems();
        if (selection.isEmpty())
            return;
        QTableWidgetItem *premier = ui->tableEmployes->item(selection.first()->row(), ColEmpId);
        if (premier)
            selectionnerEmploye(premier->data(Qt::UserRole).toInt());
    });
    connect(ui->tableEmployes, &QTableWidget::cellDoubleClicked, this, [this](int ligne, int) {
        QTableWidgetItem *premier = ui->tableEmployes->item(ligne, ColEmpId);
        if (!premier)
            return;
        const int id = premier->data(Qt::UserRole).toInt();
        QTimer::singleShot(0, this, [this, id]() { modifierEmploye(id); });
    });
    connect(ui->pagePrecedente, &QPushButton::clicked, this, [this]() { changerPageEmp(-1); });
    connect(ui->pageSuivante, &QPushButton::clicked, this, [this]() { changerPageEmp(1); });
}

// ---------------------------------------------------------------- Onglet « Statistiques »
void HackTime::preparerOngletStatistiquesEmp()
{
    connect(ui->statsRole, &QPushButton::clicked, this, [this]() {
        m_statsParChargeEmp = false;
        rafraichirStatistiquesEmp();
    });
    connect(ui->statsCharge, &QPushButton::clicked, this, [this]() {
        m_statsParChargeEmp = true;
        rafraichirStatistiquesEmp();
    });
}

// ---------------------------------------------------------------- Onglet « Charge de travail »
void HackTime::preparerOngletChargeEmp()
{
    QHeaderView *enTete = ui->tableCharge->horizontalHeader();
    enTete->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    enTete->setSectionResizeMode(QHeaderView::ResizeToContents);
    enTete->setSectionResizeMode(2, QHeaderView::Fixed);
    enTete->resizeSection(2, 190);
    enTete->setSectionResizeMode(5, QHeaderView::Stretch);
    tableauSoupleEmp(ui->tableCharge, QList<QPair<int, int> >() << qMakePair(4, 175));

    connect(ui->boutonCorrigerCharge, &QPushButton::clicked, this, &HackTime::corrigerAlertesEmp);
}

// ---------------------------------------------------------------- Onglet « Affectation »
void HackTime::preparerOngletAffectationEmp()
{
    ui->rapportVue->hide();

    QHeaderView *enTete = ui->tableCompetitions->horizontalHeader();
    enTete->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    enTete->setSectionResizeMode(QHeaderView::ResizeToContents);
    enTete->setSectionResizeMode(1, QHeaderView::Stretch);
    enTete->setSectionResizeMode(5, QHeaderView::Fixed);
    enTete->resizeSection(5, 270);
    tableauSoupleEmp(ui->tableCompetitions, QList<QPair<int, int> >() << qMakePair(3, 130));
    ui->tableCompetitions->setColumnHidden(0, true);

    connect(ui->boutonCorriger, &QPushButton::clicked, this, &HackTime::corrigerAlertesEmp);
    connect(ui->boutonToutRepartir, &QPushButton::clicked, this, &HackTime::toutRepartirEmp);
}

// ---------------------------------------------------------------- Colonne de droite
void HackTime::preparerCarteDetailsEmp()
{
    ui->detCompetitions->setWordWrap(true);
    ui->detId->hide();
    connect(ui->boutonModifier, &QPushButton::clicked, this, [this]() { modifierEmploye(m_selectionEmp); });
    connect(ui->boutonSupprimer, &QPushButton::clicked, this, [this]() { supprimerEmploye(m_selectionEmp); });
}

void HackTime::preparerCarteFormulaireEmp()
{
    // Identifiant attribué par la séquence Oracle : pas de champ dans le formulaire
    ui->libelleFId->hide();
    ui->fId->hide();

    const QStringList roles = Employe::listeRoles();
    for (int i = 0; i < roles.size(); ++i)
        ui->fRole->addItem(Employe::libelleRole(roles.at(i)), roles.at(i));
    ui->fNom->setMaxLength(50);
    ui->fPrenom->setMaxLength(50);
    ui->fEmail->setMaxLength(100);

    // Login proposé à partir de l'e-mail tant qu'il n'a pas été saisi à la main
    connect(ui->fEmail, &QLineEdit::textEdited, this, [this](const QString &email) {
        const QString login = ui->fLogin->text();
        if (login.isEmpty() || login == m_loginSuggereEmp) {
            m_loginSuggereEmp = email.section('@', 0, 0).trimmed().toLower().left(30);
            ui->fLogin->setText(m_loginSuggereEmp);
        }
    });

    ui->formErreur->hide();

    connect(ui->boutonEnregistrer, &QPushButton::clicked, this, &HackTime::enregistrerEmploye);
    connect(ui->fEffacer, &QPushButton::clicked, this, &HackTime::effacerFormulaireEmp);
}

// ======================================================================
//  Mise à jour de l'affichage
// ======================================================================
const Employe *HackTime::employeParId(int id) const
{
    for (int i = 0; i < m_employes.size(); ++i) {
        if (m_employes.at(i).getId() == id)
            return &m_employes.at(i);
    }
    return nullptr;
}

QMap<int, int> HackTime::chargeParEmploye() const
{
    QMap<int, int> charges;
    for (int i = 0; i < m_employes.size(); ++i)
        charges[m_employes.at(i).getId()] = Employe::charge(m_employes.at(i).getId(), m_competitionsEmp);
    return charges;
}

// Catégorie du filtre « Charge » : Libre, Normal, Complet, En alerte, Non concerné
QString HackTime::categorieCharge(const Employe &e) const
{
    QString couleur;
    const QString etat = Employe::etat(e, m_competitionsEmp, &couleur);
    if (couleur == "rouge")
        return "En alerte";
    return etat;
}

QList<Employe> HackTime::listeFiltreeEmp() const
{
    const QString role = ui->filtreRole->currentData().toString();
    const QList<Employe> liste = Employe::filtrer(m_employes, ui->recherche->text(), role,
                                                  chargeParEmploye(), m_triEmp, m_croissantEmp);
    if (ui->filtreCharge->currentIndex() <= 0)
        return liste;

    const QString categorie = ui->filtreCharge->currentText();
    QList<Employe> resultat;
    for (int i = 0; i < liste.size(); ++i) {
        if (categorieCharge(liste.at(i)) == categorie)
            resultat << liste.at(i);
    }
    return resultat;
}

QList<int> HackTime::idsEnAlerte() const
{
    QList<int> ids;
    const QList<Alerte> liste = Employe::alertes(m_employes, m_competitionsEmp);
    for (int i = 0; i < liste.size(); ++i)
        ids << liste.at(i).idEmploye;
    return ids;
}

void HackTime::rafraichirToutEmp()
{
    m_employes = Employe::tous();
    m_competitionsEmp = CompetitionGeree::toutes();
    if (!employeParId(m_selectionEmp))
        m_selectionEmp = m_employes.isEmpty() ? 0 : m_employes.first().getId();

    rafraichirIndicateursEmp();
    rafraichirListeEmp();
    rafraichirStatistiquesEmp();
    rafraichirChargeEmp();
    rafraichirAffectationEmp();
    rafraichirDetailsEmp();
}

void HackTime::rafraichirIndicateursEmp()
{
    int organisateurs = 0;
    for (int i = 0; i < m_employes.size(); ++i) {
        if (m_employes.at(i).getRole() == "ORGANISATEUR")
            ++organisateurs;
    }
    int actives = 0;
    for (int i = 0; i < m_competitionsEmp.size(); ++i) {
        if (m_competitionsEmp.at(i).estActive())
            ++actives;
    }
    const QList<Alerte> liste = Employe::alertes(m_employes, m_competitionsEmp);
    const int nombre = liste.size();

    ui->kpiTotalEmp->setText(QString::number(m_employes.size()));
    ui->kpiActifs->setText(QString::number(organisateurs));
    ui->kpiDepartements->setText(QString::number(actives));
    ui->kpiSurcharge->setText(QString::number(nombre));
    ui->kpiSurcharge->setStyleSheet(nombre > 0 ? "color:#d33a3a;" : "");

    // Cloche et menu des alertes
    ui->cloche->setText(nombre > 0 ? QString("🔔 %1").arg(nombre) : QString("🔔"));
    ui->cloche->setStyleSheet(nombre > 0 ? "QToolButton{color:#d33a3a;}" : "");
    m_menuAlertesEmp->clear();
    if (liste.isEmpty()) {
        QAction *aucune = m_menuAlertesEmp->addAction("Aucune alerte : la gestion des compétitions est bien répartie.");
        aucune->setEnabled(false);
    }
    for (int i = 0; i < liste.size(); ++i) {
        const int id = liste.at(i).idEmploye;
        QAction *action = m_menuAlertesEmp->addAction("⚠  " + liste.at(i).message);
        connect(action, &QAction::triggered, this, [this, id]() {
            ui->onglets->setCurrentIndex(2);
            selectionnerEmploye(id);
        });
    }
    ui->onglets->setTabText(2, nombre > 0 ? QString("⏱  Charge de travail (%1)").arg(nombre)
                                        : QString("⏱  Charge de travail"));
}

void HackTime::rafraichirListeEmp()
{
    const QList<Employe> lignes = listeFiltreeEmp();
    const int total = lignes.size();
    const int pages = qMax(1, (total + LIGNES_PAR_PAGE_EMP - 1) / LIGNES_PAR_PAGE_EMP);
    m_pageEmp = qBound(0, m_pageEmp, pages - 1);
    const int debut = m_pageEmp * LIGNES_PAR_PAGE_EMP;
    const int fin = qMin(total, debut + LIGNES_PAR_PAGE_EMP);

    QFont gras = ui->tableEmployes->font();
    gras.setBold(true);

    {
        const QSignalBlocker bloqueur(ui->tableEmployes);
        ui->tableEmployes->clearContents();
        ui->tableEmployes->setRowCount(fin - debut);
        int ligneSelection = -1;

        for (int i = debut; i < fin; ++i) {
            const Employe &e = lignes.at(i);
            const int ligne = i - debut;
            const int id = e.getId();
            const int actives = Employe::charge(id, m_competitionsEmp);
            const int gerees = Employe::nombreGerees(id, m_competitionsEmp);

            // Colonne cachée : garde l'identifiant pour retrouver l'employé sélectionné
            QTableWidgetItem *cle = new QTableWidgetItem;
            cle->setData(Qt::UserRole, id);
            ui->tableEmployes->setItem(ligne, ColEmpId, cle);

            QTableWidgetItem *nom = celluleEmp(e.getNom());
            if (Employe::enAlerte(e, m_competitionsEmp)) {
                nom->setText(e.getNom() + "  ⚠");
                nom->setToolTip("En alerte de charge");
            }
            ui->tableEmployes->setItem(ligne, ColEmpNom, nom);
            ui->tableEmployes->setItem(ligne, ColEmpPrenom, celluleEmp(e.getPrenom()));
            QTableWidgetItem *email = celluleEmp(e.getEmail());
            email->setForeground(QColor("#65708b"));
            ui->tableEmployes->setItem(ligne, ColEmpEmail, email);
            ui->tableEmployes->setItem(ligne, ColEmpRole, new QTableWidgetItem);
            ui->tableEmployes->setCellWidget(ligne, ColEmpRole, dansCelluleEmp(pastilleEmp(e.libelleRole(), couleurRoleEmp(e.getRole()))));
            ui->tableEmployes->setItem(ligne, ColEmpLogin, celluleEmp(e.getLogin()));
            QTableWidgetItem *competitions = celluleEmp(QString("%1 / %2").arg(actives).arg(gerees));
            competitions->setToolTip(QString("%1 compétition(s) active(s) sur %2 gérée(s)").arg(actives).arg(gerees));
            ui->tableEmployes->setItem(ligne, ColEmpCompetitions, competitions);

            QString couleur;
            const QString etat = Employe::etat(e, m_competitionsEmp, &couleur);
            ui->tableEmployes->setItem(ligne, ColEmpCharge, new QTableWidgetItem);
            ui->tableEmployes->setCellWidget(ligne, ColEmpCharge, dansCelluleEmp(pastilleEmp(etat, couleur)));

            // Boutons Modifier / Supprimer (l'action est différée : le tableau
            // ne doit pas être reconstruit pendant le clic sur un de ses boutons)
            QWidget *actions = new QWidget;
            QHBoxLayout *la = new QHBoxLayout(actions);
            la->setContentsMargins(4, 0, 4, 0);
            la->setSpacing(2);
            QToolButton *modifier = boutonIconeEmp("✏️", "Modifier " + e.nomComplet());
            QToolButton *supprimer = boutonIconeEmp("🗑️", "Supprimer " + e.nomComplet());
            la->addWidget(modifier);
            la->addWidget(supprimer);
            la->addStretch();
            ui->tableEmployes->setItem(ligne, ColEmpActions, new QTableWidgetItem);
            ui->tableEmployes->setCellWidget(ligne, ColEmpActions, actions);
            connect(modifier, &QToolButton::clicked, this, [this, id]() {
                QTimer::singleShot(0, this, [this, id]() { modifierEmploye(id); });
            });
            connect(supprimer, &QToolButton::clicked, this, [this, id]() {
                QTimer::singleShot(0, this, [this, id]() { supprimerEmploye(id); });
            });

            if (id == m_selectionEmp)
                ligneSelection = ligne;
        }

        if (ligneSelection >= 0)
            ui->tableEmployes->selectRow(ligneSelection);
        else
            ui->tableEmployes->clearSelection();
    }

    // Indicateur de tri dans l'en-tête
    int colonneTri = 0;
    for (int c = ColEmpCharge; c >= 0; --c) {
        if (TRIS_EMP[c] == m_triEmp)
            colonneTri = c;
    }
    ui->tableEmployes->horizontalHeader()->setSortIndicator(colonneTri, m_croissantEmp ? Qt::AscendingOrder
                                                                         : Qt::DescendingOrder);

    // Pied de liste
    if (total == 0) {
        ui->piedListe->setText(m_employes.isEmpty()
                                 ? "Aucun employé enregistré. Ajoutez le premier avec le formulaire."
                                 : "Aucun employé ne correspond à cette recherche.");
    } else {
        ui->piedListe->setText(QString("Affichage de %1 à %2 sur %3 employé%4")
                                 .arg(debut + 1).arg(fin).arg(total).arg(QString(total > 1 ? "s" : "")));
    }
    ui->pageCourante->setText(QString("%1").arg(m_pageEmp + 1));
    ui->pageCourante->setToolTip(QString("Page %1 sur %2").arg(m_pageEmp + 1).arg(pages));
    ui->pagePrecedente->setEnabled(m_pageEmp > 0);
    ui->pageSuivante->setEnabled(m_pageEmp < pages - 1);
}

void HackTime::rafraichirStatistiquesEmp()
{
    if (!m_statsParChargeEmp) {
        m_statsEmp->setDonnees(Employe::statistiquesParRole(m_employes));
        return;
    }
    QList<QPair<QString, int> > donnees;
    for (int c = 0; c < 5; ++c) {
        const QString categorie = QString::fromUtf8(CATEGORIES_CHARGE_EMP[c]);
        int n = 0;
        for (int i = 0; i < m_employes.size(); ++i) {
            if (categorieCharge(m_employes.at(i)) == categorie)
                ++n;
        }
        if (n > 0)
            donnees << qMakePair(categorie, n);
    }
    m_statsEmp->setDonnees(donnees);
}

void HackTime::rafraichirChargeEmp()
{
    const QList<Alerte> liste = Employe::alertes(m_employes, m_competitionsEmp);
    ui->cadreAlerte->setVisible(!liste.isEmpty());
    ui->cadreOk->setVisible(liste.isEmpty());
    if (!liste.isEmpty()) {
        QString html = QString("<b>%1 alerte%2 de charge</b><ul style=\"margin-top:4px;\">")
                           .arg(liste.size()).arg(QString(liste.size() > 1 ? "s" : ""));
        for (int i = 0; i < liste.size(); ++i)
            html += "<li>" + liste.at(i).message.toHtmlEscaped() + "</li>";
        html += "</ul>";
        ui->texteAlerte->setText(html);
    }

    // Employés les plus chargés en premier
    QList<QPair<int, int> > ordre;
    for (int i = 0; i < m_employes.size(); ++i)
        ordre << qMakePair(Employe::charge(m_employes.at(i).getId(), m_competitionsEmp), i);
    std::sort(ordre.begin(), ordre.end(), chargePlusForteEmp);

    ui->tableCharge->clearContents();
    ui->tableCharge->setRowCount(ordre.size());
    QFont gras = ui->tableCharge->font();
    gras.setBold(true);

    for (int ligne = 0; ligne < ordre.size(); ++ligne) {
        const Employe &e = m_employes.at(ordre.at(ligne).second);
        const int actives = ordre.at(ligne).first;

        QTableWidgetItem *nom = celluleEmp(e.nomComplet());
        nom->setFont(gras);
        ui->tableCharge->setItem(ligne, 0, nom);
        QTableWidgetItem *role = celluleEmp(e.libelleRole());
        role->setForeground(QColor("#65708b"));
        ui->tableCharge->setItem(ligne, 1, role);

        QProgressBar *barre = new QProgressBar;
        reglerBarreEmp(barre, actives, Employe::enAlerte(e, m_competitionsEmp));
        QWidget *conteneur = new QWidget;
        QHBoxLayout *lb = new QHBoxLayout(conteneur);
        lb->setContentsMargins(8, 0, 8, 0);
        lb->addWidget(barre);
        ui->tableCharge->setItem(ligne, 2, new QTableWidgetItem);
        ui->tableCharge->setCellWidget(ligne, 2, conteneur);

        QTableWidgetItem *nombre = celluleEmp(texteChargeEmp(actives));
        nombre->setFont(gras);
        ui->tableCharge->setItem(ligne, 3, nombre);

        QString couleur;
        const QString etat = Employe::etat(e, m_competitionsEmp, &couleur);
        ui->tableCharge->setItem(ligne, 4, new QTableWidgetItem);
        ui->tableCharge->setCellWidget(ligne, 4, dansCelluleEmp(pastilleEmp(etat, couleur)));

        const QList<CompetitionGeree> gerees = Employe::activesDe(e.getId(), m_competitionsEmp);
        QStringList resume;
        QStringList detail;
        for (int c = 0; c < gerees.size(); ++c) {
            resume << gerees.at(c).nom;
            detail << QString("%1 (%2)").arg(gerees.at(c).nom, gerees.at(c).periode());
        }
        QTableWidgetItem *competitions = new QTableWidgetItem(resume.isEmpty() ? QString("Aucune") : resume.join(", "));
        competitions->setToolTip(detail.join("\n"));
        if (resume.isEmpty())
            competitions->setForeground(QColor("#65708b"));
        ui->tableCharge->setItem(ligne, 5, competitions);
    }
}

void HackTime::rafraichirAffectationEmp()
{
    ui->tableCompetitions->clearContents();
    ui->tableCompetitions->setRowCount(m_competitionsEmp.size());
    QFont gras = ui->tableCompetitions->font();
    gras.setBold(true);

    for (int ligne = 0; ligne < m_competitionsEmp.size(); ++ligne) {
        const CompetitionGeree &c = m_competitionsEmp.at(ligne);
        const int idCompetition = c.id;

        ui->tableCompetitions->setItem(ligne, 0, new QTableWidgetItem);
        QTableWidgetItem *nom = celluleEmp(c.nom);
        nom->setFont(gras);
        nom->setToolTip(c.nom + " - " + c.theme);
        ui->tableCompetitions->setItem(ligne, 1, nom);
        ui->tableCompetitions->setItem(ligne, 2, celluleEmp(c.periode()));
        ui->tableCompetitions->setItem(ligne, 3, new QTableWidgetItem);
        ui->tableCompetitions->setCellWidget(ligne, 3, dansCelluleEmp(pastilleEmp(c.libelleStatut(), couleurStatutEmp(c.statut))));
        ui->tableCompetitions->setItem(ligne, 4, celluleEmp(c.libelleMode()));

        // Responsable : organisateurs et administrateurs d'abord, puis le jury
        QComboBox *choix = new QComboBox;
        choix->addItem("Aucun responsable", 0);
        QList<int> jury;
        for (int i = 0; i < m_employes.size(); ++i) {
            const Employe &e = m_employes.at(i);
            if (!e.peutGerer()) {
                jury << i;
                continue;
            }
            choix->addItem(QString("%1 (%2)").arg(e.nomComplet(), texteChargeEmp(Employe::charge(e.getId(), m_competitionsEmp))),
                           e.getId());
        }
        if (!jury.isEmpty())
            choix->insertSeparator(choix->count());
        for (int i = 0; i < jury.size(); ++i) {
            const Employe &e = m_employes.at(jury.at(i));
            choix->addItem(e.nomComplet() + " - jury", e.getId());
        }
        // GERER : la liste montre le premier responsable, les autres en info-bulle
        const int position = choix->findData(c.responsables.isEmpty() ? 0 : c.responsables.first());
        choix->setCurrentIndex(position);
        if (c.responsables.size() > 1) {
            QStringList noms;
            for (int r = 0; r < c.responsables.size(); ++r) {
                const Employe *resp = employeParId(c.responsables.at(r));
                if (resp)
                    noms << resp->nomComplet();
            }
            choix->setToolTip("Responsables : " + noms.join(", "));
        }
        choix->setEnabled(c.estActive());
        if (!c.estActive())
            choix->setToolTip("Compétition terminée");

        QWidget *conteneur = new QWidget;
        QHBoxLayout *lc = new QHBoxLayout(conteneur);
        lc->setContentsMargins(6, 4, 6, 4);
        lc->addWidget(choix);
        ui->tableCompetitions->setItem(ligne, 5, new QTableWidgetItem);
        ui->tableCompetitions->setCellWidget(ligne, 5, conteneur);
        connect(choix, QOverload<int>::of(&QComboBox::activated), this, [this, choix, idCompetition](int index) {
            const int idEmploye = choix->itemData(index).toInt();
            QTimer::singleShot(0, this, [this, idCompetition, idEmploye]() {
                changerResponsableEmp(idCompetition, idEmploye);
            });
        });
    }

    // Compte rendu de la dernière répartition automatique
    ui->rapportVue->setVisible(m_rapportVisibleEmp);
    if (!m_rapportVisibleEmp)
        return;
    if (m_rapportEmp.isEmpty()) {
        ui->rapportVue->setHtml("Aucun changement : la répartition actuelle est déjà la meilleure.");
        return;
    }
    int reussies = 0;
    QString lignes;
    for (int i = 0; i < m_rapportEmp.size(); ++i) {
        const LigneRapport &r = m_rapportEmp.at(i);
        if (r.deplacee) {
            ++reussies;
            lignes += QString("<li>%1 : %2 → <b>%3</b></li>")
                          .arg(r.competition.nom.toHtmlEscaped(), r.ancien.toHtmlEscaped(), r.nouveau.toHtmlEscaped());
        } else {
            lignes += QString("<li style=\"color:#d33a3a;\">%1 : %2.</li>")
                          .arg(r.competition.nom.toHtmlEscaped(), r.raison.toHtmlEscaped());
        }
    }
    const int echecs = m_rapportEmp.size() - reussies;
    QString titre = QString("<b>%1 compétition%2 confiée%2 à un nouveau responsable")
                        .arg(reussies).arg(QString(reussies > 1 ? "s" : ""));
    if (echecs > 0)
        titre += QString(", %1 sans solution").arg(echecs);
    titre += "</b>";
    ui->rapportVue->setHtml(titre + "<ul style=\"margin-top:4px;\">" + lignes + "</ul>");
}

void HackTime::rafraichirDetailsEmp()
{
    const Employe *e = employeParId(m_selectionEmp);
    ui->detVide->setVisible(e == nullptr);
    ui->detContenu->setVisible(e != nullptr);
    if (!e)
        return;

    const int actives = Employe::charge(e->getId(), m_competitionsEmp);
    QString couleur;
    const QString etat = Employe::etat(*e, m_competitionsEmp, &couleur);

    ui->detAvatar->setText(e->initiales());
    ui->detNom->setText(e->nomComplet());
    ui->detStatut->setText(e->libelleRole());
    ui->detStatut->setStyleSheet(stylePastilleEmp(couleurRoleEmp(e->getRole())));
    ui->detEmail->setText(e->getEmail());
    ui->detRole->setText(e->libelleRole());
    ui->detLogin->setText(e->getLogin());
    ui->detNbCompetitions->setText(QString("%1 compétition(s), dont %2 active(s)")
                                       .arg(Employe::nombreGerees(e->getId(), m_competitionsEmp)).arg(actives));

    const QList<CompetitionGeree> gerees = Employe::activesDe(e->getId(), m_competitionsEmp);
    QStringList noms;
    for (int i = 0; i < gerees.size(); ++i)
        noms << QString("%1 (%2)").arg(gerees.at(i).nom, gerees.at(i).periode());
    ui->detCompetitions->setText(noms.isEmpty() ? QString("Aucune") : noms.join("\n"));

    ui->detCharge->setText(QString("%1 compétitions actives  (%2)").arg(texteChargeEmp(actives), etat));
    ui->detCharge->setStyleSheet(couleur == "rouge" ? "color:#d33a3a;font-weight:bold;" : "");
    reglerBarreEmp(ui->detBarre, actives, Employe::enAlerte(*e, m_competitionsEmp));
}

void HackTime::remplirFormulaireEmp(const Employe *e)
{
    ui->formErreur->hide();
    m_editionEmp = e ? e->getId() : 0;
    m_loginSuggereEmp.clear();
    ui->formTitre->setText(e ? "✏️  Modifier " + e->nomComplet() : QString("＋  Ajouter un employé"));
    ui->fEffacer->setText(e ? "Annuler" : "Effacer");

    ui->fNom->setText(e ? e->getNom() : QString());
    ui->fPrenom->setText(e ? e->getPrenom() : QString());
    ui->fEmail->setText(e ? e->getEmail() : QString());
    ui->fRole->setCurrentIndex(qMax(0, ui->fRole->findData(e ? e->getRole() : QString("ORGANISATEUR"))));
    ui->fLogin->setText(e ? e->getLogin() : QString());
    ui->fMdp->clear();
    ui->fMdp->setPlaceholderText(e ? "Laisser vide pour le garder" : "6 caractères, lettres et chiffres");
}

void HackTime::messageEmp(const QString &contenu, bool erreur)
{
    m_toastEmp->setText(contenu);
    m_toastEmp->setStyleSheet(QString("background:%1;color:white;border-radius:10px;"
                                   "padding:12px 18px;font-weight:bold;")
                               .arg(QString(erreur ? "#b02a2a" : "#0f1a38")));
    m_toastEmp->adjustSize();
    QWidget *centre = ui->pageEmployes;
    m_toastEmp->move((centre->width() - m_toastEmp->width()) / 2, centre->height() - m_toastEmp->height() - 26);
    m_toastEmp->raise();
    m_toastEmp->show();
    m_toastTimerEmp->start(4000);
}

void HackTime::signalerNouvellesAlertesEmp(const QList<int> &avant)
{
    const QList<Alerte> liste = Employe::alertes(m_employes, m_competitionsEmp);
    for (int i = 0; i < liste.size(); ++i) {
        if (!avant.contains(liste.at(i).idEmploye)) {
            messageEmp("Alerte : " + liste.at(i).message, true);
            return;
        }
    }
}

// ======================================================================
//  Employés : ajouter, consulter, modifier, supprimer
// ======================================================================
void HackTime::selectionnerEmploye(int id)
{
    m_selectionEmp = id;
    rafraichirDetailsEmp();

    // Surligne la ligne correspondante si elle est sur la page affichée
    const QSignalBlocker bloqueur(ui->tableEmployes);
    for (int ligne = 0; ligne < ui->tableEmployes->rowCount(); ++ligne) {
        QTableWidgetItem *premier = ui->tableEmployes->item(ligne, ColEmpId);
        if (premier && premier->data(Qt::UserRole).toInt() == id) {
            ui->tableEmployes->selectRow(ligne);
            return;
        }
    }
    ui->tableEmployes->clearSelection();
}

void HackTime::modifierEmploye(int id)
{
    const Employe *e = employeParId(id);
    if (!e)
        return;
    selectionnerEmploye(id);
    remplirFormulaireEmp(e);
    ui->fNom->setFocus();
}

void HackTime::effacerFormulaireEmp()
{
    remplirFormulaireEmp(nullptr);
}

void HackTime::enregistrerEmploye()
{
    // Récupération des informations saisies dans le formulaire
    QString nom = ui->fNom->text().trimmed();
    QString prenom = ui->fPrenom->text().trimmed();
    QString email = ui->fEmail->text().trimmed().toLower();
    QString role = ui->fRole->currentData().toString();
    QString login = ui->fLogin->text().trimmed();
    QString mdp = ui->fMdp->text();

    Employe e(m_editionEmp, nom, prenom, email, role, login, mdp);

    // Contrôles de saisie
    static const QRegularExpression formatEmail("^[^\\s@]+@[^\\s@]+\\.[^\\s@]{2,}$");
    static const QRegularExpression formatLogin("^[A-Za-z0-9._-]+$");
    static const QRegularExpression contientLettre("[A-Za-z]");
    static const QRegularExpression contientChiffre("[0-9]");
    const bool modification = m_editionEmp != 0;
    QString erreur;
    QWidget *champ = nullptr;
    if (e.getNom().isEmpty()) {
        erreur = "Saisissez le nom.";
        champ = ui->fNom;
    } else if (e.getPrenom().isEmpty()) {
        erreur = "Saisissez le prénom.";
        champ = ui->fPrenom;
    } else if (e.getEmail().isEmpty()) {
        erreur = "Saisissez l'adresse e-mail.";
        champ = ui->fEmail;
    } else if (!formatEmail.match(e.getEmail()).hasMatch()) {
        erreur = "Adresse e-mail invalide. Exemple : prenom.nom@hacktime.tn";
        champ = ui->fEmail;
    } else if (Employe::emailExiste(e.getEmail(), m_editionEmp)) {
        erreur = "Cette adresse e-mail est déjà utilisée par un autre employé.";
        champ = ui->fEmail;
    } else if (e.getLogin().isEmpty()) {
        erreur = "Saisissez le login.";
        champ = ui->fLogin;
    } else if (!formatLogin.match(e.getLogin()).hasMatch()) {
        erreur = "Le login ne peut contenir que des lettres sans accent, des chiffres, « . », « _ » et « - ».";
        champ = ui->fLogin;
    } else if (Employe::loginExiste(e.getLogin(), m_editionEmp)) {
        erreur = "Ce login est déjà utilisé par un autre employé.";
        champ = ui->fLogin;
    } else if (!modification && mdp.isEmpty()) {
        erreur = "Saisissez le mot de passe.";
        champ = ui->fMdp;
    } else if (!mdp.isEmpty() && mdp.size() < 6) {
        erreur = "Le mot de passe doit contenir au moins 6 caractères.";
        champ = ui->fMdp;
    } else if (!mdp.isEmpty() && (!mdp.contains(contientLettre) || !mdp.contains(contientChiffre))) {
        erreur = "Le mot de passe doit contenir des lettres et des chiffres.";
        champ = ui->fMdp;
    }
    if (!erreur.isEmpty()) {
        ui->formErreur->setText(erreur);
        ui->formErreur->show();
        champ->setFocus();
        return;
    }

    const QList<int> avant = idsEnAlerte();
    QString erreurSql;
    const bool ok = modification ? e.modifier(&erreurSql) : e.ajouter(&erreurSql);
    if (!ok) {
        messageEmp("Enregistrement impossible : " + erreurSql, true);
        return;
    }

    const QString nomComplet = e.nomComplet();
    m_selectionEmp = e.getId();
    m_editionEmp = 0;
    rafraichirToutEmp();
    remplirFormulaireEmp(nullptr);

    // Affiche la page de la liste où se trouve l'employé enregistré
    const QList<Employe> lignes = listeFiltreeEmp();
    for (int i = 0; i < lignes.size(); ++i) {
        if (lignes.at(i).getId() == m_selectionEmp) {
            m_pageEmp = i / LIGNES_PAR_PAGE_EMP;
            rafraichirListeEmp();
            break;
        }
    }

    messageEmp(modification ? nomComplet + " a été enregistré."
                         : nomComplet + " a été ajouté.");
    signalerNouvellesAlertesEmp(avant);
}

void HackTime::supprimerEmploye(int id)
{
    const Employe *e = employeParId(id);
    if (!e)
        return;
    const QString nomComplet = e->nomComplet();

    // ses lignes dans GERER sont supprimées avec lui (ON DELETE CASCADE)
    const int gerees = Employe::nombreGerees(id, m_competitionsEmp);
    QString detail = "La fiche de cet employé sera supprimée définitivement de la base.";
    if (gerees > 0)
        detail += QString("\nIl ne gérera plus ses %1 compétition(s).").arg(gerees);

    QMessageBox boite(this);
    boite.setWindowTitle("Supprimer un employé");
    boite.setIcon(QMessageBox::Warning);
    boite.setText("Supprimer " + nomComplet + " ?");
    boite.setInformativeText(detail);
    QPushButton *annuler = boite.addButton("Annuler", QMessageBox::RejectRole);
    QPushButton *confirmer = boite.addButton("Supprimer", QMessageBox::DestructiveRole);
    boite.setDefaultButton(annuler);
    boite.exec();
    if (boite.clickedButton() != confirmer)
        return;

    bool test = Etmp.supprimer(id);
    if (!test) {
        messageEmp("La suppression a échoué. Réessayez.", true);
        return;
    }
    const bool etaitEnEdition = (m_editionEmp == id);
    if (etaitEnEdition)
        m_editionEmp = 0;
    rafraichirToutEmp();
    if (etaitEnEdition)
        remplirFormulaireEmp(nullptr);
    messageEmp(nomComplet + " a été supprimé.");
}

// ======================================================================
//  Liste : pagination, tri, export
// ======================================================================
void HackTime::changerPageEmp(int decalage)
{
    m_pageEmp += decalage;
    rafraichirListeEmp();
}

void HackTime::trierParColonneEmp(int colonne)
{
    if (colonne < 0 || colonne > ColEmpCharge) {
        rafraichirListeEmp();   // colonne « Actions » : on remet simplement l'indicateur en place
        return;
    }
    if (m_triEmp == TRIS_EMP[colonne]) {
        m_croissantEmp = !m_croissantEmp;
    } else {
        m_triEmp = TRIS_EMP[colonne];
        m_croissantEmp = m_triEmp != Employe::ParCompetitions;
    }
    {
        const QSignalBlocker bloqueur(ui->triCombo);
        const int index = ui->triCombo->findData(int(m_triEmp));
        if (index >= 0)
            ui->triCombo->setCurrentIndex(index);
    }
    rafraichirListeEmp();
}

void HackTime::exporterPdfEmp()
{
    const QList<Employe> lignes = listeFiltreeEmp();
    if (lignes.isEmpty()) {
        messageEmp("Aucun employé à exporter avec ces filtres.", true);
        return;
    }
    QString chemin = QFileDialog::getSaveFileName(this, "Exporter la liste en PDF",
                                                  "employes-hacktime.pdf", "Document PDF (*.pdf)");
    if (chemin.isEmpty())
        return;
    if (!chemin.endsWith(".pdf", Qt::CaseInsensitive))
        chemin += ".pdf";

    QString html;
    html += "<h2 style=\"color:#0b1b58;\">HackTime - Liste des employés</h2>";
    html += QString("<p style=\"color:#65708b;\">%1 employé(s) - exporté le %2 par %3</p>")
                .arg(lignes.size())
                .arg(QDate::currentDate().toString("dd/MM/yyyy"), m_utilisateurEmp.toHtmlEscaped());
    html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"5\">";
    html += "<tr>";
    const QStringList entetes = QStringList() << "Nom" << "Prénom" << "E-mail" << "Rôle"
                                              << "Login" << "Compétitions actives" << "Charge";
    for (int i = 0; i < entetes.size(); ++i)
        html += "<th align=\"left\" bgcolor=\"#0b1b58\"><font color=\"#ffffff\">" + entetes.at(i) + "</font></th>";
    html += "</tr>";
    for (int i = 0; i < lignes.size(); ++i) {
        const Employe &e = lignes.at(i);
        QString couleur;
        const QString fond = (i % 2 == 0) ? "#f3f6fc" : "#ffffff";
        const QStringList valeurs = QStringList() << e.getNom() << e.getPrenom() << e.getEmail() << e.libelleRole()
                                                  << e.getLogin()
                                                  << texteChargeEmp(Employe::charge(e.getId(), m_competitionsEmp))
                                                  << Employe::etat(e, m_competitionsEmp, &couleur);
        html += "<tr>";
        for (int c = 0; c < valeurs.size(); ++c)
            html += "<td bgcolor=\"" + fond + "\">" + valeurs.at(c).toHtmlEscaped() + "</td>";
        html += "</tr>";
    }
    html += "</table>";

    QPrinter imprimante(QPrinter::HighResolution);
    imprimante.setOutputFormat(QPrinter::PdfFormat);
    imprimante.setOutputFileName(chemin);
    imprimante.setPageSize(QPageSize(QPageSize::A4));
    imprimante.setPageOrientation(QPageLayout::Landscape);

    QTextDocument document;
    document.setDefaultFont(QFont("Arial", 9));
    document.setHtml(html);
    document.print(&imprimante);

    if (QFile::exists(chemin))
        messageEmp("Liste exportée en PDF : " + chemin);
    else
        messageEmp("L'export PDF a échoué. Vérifiez que le dossier est accessible.", true);
}

// Export Excel : fichier CSV (séparateur « ; », encodage UTF-8 avec BOM),
// qu'Excel ouvre directement avec les colonnes et les accents corrects.
void HackTime::exporterExcelEmp()
{
    const QList<Employe> lignes = listeFiltreeEmp();
    if (lignes.isEmpty()) {
        messageEmp("Aucun employé à exporter avec ces filtres.", true);
        return;
    }
    QString chemin = QFileDialog::getSaveFileName(this, "Exporter la liste pour Excel",
                                                  "employes-hacktime.csv", "Fichier Excel CSV (*.csv)");
    if (chemin.isEmpty())
        return;
    if (!chemin.endsWith(".csv", Qt::CaseInsensitive))
        chemin += ".csv";

    QStringList contenu;
    contenu << (QStringList() << "Nom" << "Prénom" << "E-mail" << "Rôle" << "Login"
                              << "Compétitions actives" << "Charge").join(";");
    for (int i = 0; i < lignes.size(); ++i) {
        const Employe &e = lignes.at(i);
        QString couleur;
        contenu << (QStringList() << champCsvEmp(e.getNom()) << champCsvEmp(e.getPrenom())
                                  << champCsvEmp(e.getEmail()) << champCsvEmp(e.libelleRole()) << champCsvEmp(e.getLogin())
                                  << champCsvEmp(texteChargeEmp(Employe::charge(e.getId(), m_competitionsEmp)))
                                  << champCsvEmp(Employe::etat(e, m_competitionsEmp, &couleur)))
                       .join(";");
    }

    QFile fichier(chemin);
    if (!fichier.open(QIODevice::WriteOnly)) {
        messageEmp("Impossible d'écrire le fichier. Fermez-le dans Excel s'il est ouvert, puis réessayez.", true);
        return;
    }
    fichier.write("\xEF\xBB\xBF");
    fichier.write(contenu.join("\r\n").toUtf8());
    fichier.write("\r\n");
    fichier.close();
    messageEmp("Liste exportée pour Excel : " + chemin);
}

// ======================================================================
//  Charge et répartition des compétitions
// ======================================================================
void HackTime::afficherRapportEmp(const QList<LigneRapport> &rapport)
{
    m_rapportEmp = rapport;
    m_rapportVisibleEmp = true;
    rafraichirToutEmp();
    ui->onglets->setCurrentIndex(3);

    int echecs = 0;
    for (int i = 0; i < rapport.size(); ++i) {
        if (!rapport.at(i).deplacee)
            ++echecs;
    }
    if (echecs > 0)
        messageEmp(QString("%1 compétition(s) sans responsable disponible : ajoutez un organisateur.").arg(echecs), true);
    else if (rapport.isEmpty())
        messageEmp("Aucun changement nécessaire.");
    else
        messageEmp(QString("%1 compétition(s) confiée(s) à un nouveau responsable.").arg(rapport.size()));
}

void HackTime::corrigerAlertesEmp()
{
    afficherRapportEmp(Employe::corrigerAlertes());
}

void HackTime::toutRepartirEmp()
{
    afficherRapportEmp(Employe::toutRepartir());
}

void HackTime::changerResponsableEmp(int idCompetition, int idEmploye)
{
    const QList<int> avant = idsEnAlerte();
    if (!CompetitionGeree::changerResponsable(idCompetition, idEmploye)) {
        messageEmp("Le responsable n'a pas pu être enregistré. Réessayez.", true);
        return;
    }
    m_rapportVisibleEmp = false;
    rafraichirToutEmp();
    signalerNouvellesAlertesEmp(avant);
}

void HackTime::ajouterEmployesDemo()
{
    QMessageBox boite(this);
    boite.setWindowTitle("Employés de démonstration");
    boite.setIcon(QMessageBox::Question);
    boite.setText("Ajouter des employés de démonstration ?");
    boite.setInformativeText("4 organisateurs et 2 membres du jury sont ajoutés à la table EMPLOYE "
                             "(mot de passe « hacktime26 »). Rien n'est supprimé ni modifié.");
    QPushButton *annuler = boite.addButton("Annuler", QMessageBox::RejectRole);
    QPushButton *confirmer = boite.addButton("Ajouter", QMessageBox::AcceptRole);
    boite.setDefaultButton(confirmer);
    boite.exec();
    if (boite.clickedButton() == annuler)
        return;

    const int ajoutes = Employe::ajouterDemo();
    if (ajoutes < 0) {
        messageEmp("L'ajout des employés de démonstration a échoué.", true);
        return;
    }
    rafraichirToutEmp();
    messageEmp(ajoutes == 0 ? QString("Les employés de démonstration sont déjà dans la base.")
                         : QString("%1 employé(s) de démonstration ajouté(s).").arg(ajoutes));
}
