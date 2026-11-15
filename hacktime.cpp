#include "hacktime.h"
#include "ui_hacktime.h"
#include "competition.h"
#include "employe.h"
#include "statsemploye.h"

#include <QAction>
#include <QDate>
#include <QEvent>
#include <QGuiApplication>
#include <QIcon>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QStackedWidget>
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
#include <QCalendarWidget>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFont>
#include <QImage>
#include <QListWidget>
#include <QPdfWriter>
#include <QStandardPaths>
#include <QTextCharFormat>
#include <QUrl>
#include <QColor>
#include <QCoreApplication>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QTableWidgetItem>
#include <QVariantAnimation>
#include <cmath>

// =====================================================================
// === CONSTRUCTEUR & DESTRUCTEUR ======================================
// =====================================================================

HackTime::HackTime(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HackTime)
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

    // MODULE COMPETITION
    if (filtreEvenementsComp(obj, ev)) return true;

    return QMainWindow::eventFilter(obj, ev);
}

// =====================================================================
// === MODULE COMPETITION ==============================================
// =====================================================================

// 💡 Étudiant responsable du module Compétition travaille ici uniquement

void HackTime::initCompetition()
{
    fondLogin = QPixmap(":/images/login_bg.jpg");
    preparerLoginComp();
    preparerFenetreComp();
    afficherLoginComp();
}

// bouton « Se connecter » (page de connexion)
void HackTime::on_BLoginCompet_clicked()
{
    seConnecterComp();
}

// ----- données fixes et petites fonctions du module (espace de noms CompUI)
namespace CompUI {
// compte de l'application
static const QString UTILISATEUR_COMP = "hacktime123";
static const QString MOT_DE_PASSE_COMP = "hacktime123";

static const int PAR_PAGE = 8;

static const QStringList THEMES = {"Environnement", "Santé", "Éducation", "Ville intelligente",
                                   "Agriculture", "Intelligence artificielle", "Développement",
                                   "Finance", "Autre"};
static const QStringList MODES = {"Présentiel", "En ligne", "Hybride"};
static const QStringList STATUTS = {"Planifiée", "En cours", "Terminée"};
static const QStringList SALLES = {"Amphi 1", "Amphi 2", "Salle A", "Salle B",
                                   "Lab Innovation", "Espace Coworking"};


static QString themeIcon(const QString &t)
{
    if (t == "Environnement") return "🍃";
    if (t == "Santé" || t == "Sante") return "❤️";
    if (t == "Éducation") return "🎓";
    if (t == "Ville intelligente") return "🏙️";
    if (t == "Agriculture") return "🌱";
    if (t == "Intelligence artificielle") return "🤖";
    if (t == "Développement") return "</>";
    if (t == "Finance") return "🪙";
    return "⭐";
}
static QString modeIcon(const QString &m)
{
    if (m == "En ligne") return "💻";
    if (m == "Hybride") return "🔀";
    return "🏢";
}
static QString statutBg(const QString &s)
{
    if (s == "En cours") return "#DDF3E4";
    if (s == "Terminée") return "#E5E9F0";
    return "#DCE9FF";
}
static QString statutFg(const QString &s)
{
    if (s == "En cours") return "#2E9E5B";
    if (s == "Terminée") return "#64748B";
    return "#2563EB";
}
static const QStringList VIDE = {"", "", "", ""};   // salle, jury, description, sponsors
static QString ouTiret(const QString &s) { return s.isEmpty() ? QString("-") : s; }

// le logo clignote doucement, en boucle
static QPropertyAnimation *clignoter(QWidget *w, QObject *parent)
{
    auto *effet = new QGraphicsOpacityEffect(w);
    w->setGraphicsEffect(effet);
    auto *anim = new QPropertyAnimation(effet, "opacity", parent);
    anim->setDuration(1400);
    anim->setKeyValueAt(0.0, 1.0);
    anim->setKeyValueAt(0.5, 0.25);
    anim->setKeyValueAt(1.0, 1.0);
    anim->setEasingCurve(QEasingCurve::InOutSine);
    anim->setLoopCount(-1);
    return anim;
}

// agrandit / réduit un widget avec une petite animation
static void animerTaille(QWidget *w, const QSize &cible)
{
    auto *anim = new QVariantAnimation(w);
    anim->setDuration(180);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setStartValue(w->size());
    anim->setEndValue(cible);
    QObject::connect(anim, &QVariantAnimation::valueChanged, w,
                     [w](const QVariant &v) { w->setFixedSize(v.toSize()); });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
} // namespace CompUI
using namespace CompUI;

// ----- LOGIN (page 0)
void HackTime::preparerLoginComp()
{
    // texte d'aide en bleu clair, lisible sur le fond sombre
    for (QLineEdit *e : {ui->EditLoginCompet, ui->EditPWDCompet}) {
        QPalette pal = e->palette();
        pal.setColor(QPalette::PlaceholderText, QColor(170, 192, 235));
        e->setPalette(pal);
    }

    // œil dans le mot de passe : afficher / masquer
    QAction *oeil = ui->EditPWDCompet->addAction(QIcon(":/images/eye_off.png"), QLineEdit::TrailingPosition);
    oeil->setToolTip("Afficher le mot de passe");
    connect(oeil, &QAction::triggered, this, [this, oeil]() {
        const bool cache = ui->EditPWDCompet->echoMode() == QLineEdit::Password;
        ui->EditPWDCompet->setEchoMode(cache ? QLineEdit::Normal : QLineEdit::Password);
        oeil->setIcon(QIcon(cache ? ":/images/eye.png" : ":/images/eye_off.png"));
        oeil->setToolTip(cache ? "Masquer le mot de passe" : "Afficher le mot de passe");
    });

    // champs et bouton qui s'agrandissent au survol (voir eventFilter)
    ui->loginFormLayout->setColumnMinimumWidth(1, 400);
    ui->loginFormLayout->setRowMinimumHeight(0, 52);
    ui->loginFormLayout->setRowMinimumHeight(1, 52);
    ui->loginFormLayout->setRowMinimumHeight(4, 58);
    ui->EditLoginCompet->setFixedSize(360, 46);
    ui->EditPWDCompet->setFixedSize(360, 46);
    ui->BLoginCompet->setFixedSize(170, 52);
    for (QWidget *w : {(QWidget *)ui->EditLoginCompet, (QWidget *)ui->EditPWDCompet,
                       (QWidget *)ui->BLoginCompet})
        w->installEventFilter(this);

    // image de fond dessinée dans eventFilter
    ui->pageLogin->installEventFilter(this);
    clignotementLoginComp = clignoter(ui->logoLoginCompet, this);

    // Entrée : du nom d'utilisateur au mot de passe, puis connexion
    connect(ui->EditLoginCompet, &QLineEdit::returnPressed, this, [this]() { ui->EditPWDCompet->setFocus(); });
    connect(ui->EditPWDCompet, &QLineEdit::returnPressed, ui->BLoginCompet, &QPushButton::click);
}

void HackTime::afficherLoginComp()
{
    ui->EditLoginCompet->clear();
    ui->EditPWDCompet->clear();
    ui->EditPWDCompet->setEchoMode(QLineEdit::Password);
    ui->lblErrLoginCompet->hide();
    allerLogin();
    clignotementLoginComp->start();
    ui->EditLoginCompet->setFocus();
}

// Connexion : nom d'utilisateur = hacktime123, mot de passe = hacktime123
void HackTime::seConnecterComp()
{
    const QString utilisateur = ui->EditLoginCompet->text().trimmed();
    const QString mdp = ui->EditPWDCompet->text();
    if (utilisateur != UTILISATEUR_COMP || mdp != MOT_DE_PASSE_COMP) {
        ui->lblErrLoginCompet->setText(utilisateur.isEmpty() || mdp.isEmpty()
                                           ? "Saisissez le nom d'utilisateur et le mot de passe."
                                           : "Nom d'utilisateur ou mot de passe incorrect.");
        ui->lblErrLoginCompet->show();
        ui->EditPWDCompet->clear();
        ui->EditPWDCompet->setFocus();
        return;
    }
    ui->lblErrLoginCompet->hide();
    roleComp = "Administrateur";
    clignotementLoginComp->stop();
    static_cast<QGraphicsOpacityEffect *>(clignotementLoginComp->targetObject())->setOpacity(1.0);

    ui->adminComp->setText(QString("👤  <b>%1</b>  ▾").arg(roleComp));
    ui->adminComp->setToolTip("Cliquer pour se déconnecter");
    ui->cardAddComp->setVisible(true);
    on_btnClearComp_clicked();

    allerCompetitions();
    afficherComp();
}

void HackTime::deconnexionComp()
{
    if (QMessageBox::question(this, "Déconnexion", "Se déconnecter de HackTime ?") != QMessageBox::Yes)
        return;
    roleComp.clear();
    afficherLoginComp();
}

bool HackTime::peutModifierComp() const { return roleComp == "Administrateur" || roleComp == "Organisateur"; }
bool HackTime::peutSupprimerComp() const { return roleComp == "Administrateur"; }

// ----- Événements : fond du login, survol, courbes, clic sur le nom, molette
bool HackTime::filtreEvenementsComp(QObject *obj, QEvent *ev)
{
    if (!ui) return false;   // fenêtre en cours de fermeture

    // fond « circuits » de la page de connexion, sans déformation
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

    // champs et bouton du login : plus grands au survol (et pendant la saisie)
    if (obj == ui->EditLoginCompet || obj == ui->EditPWDCompet || obj == ui->BLoginCompet) {
        QWidget *w = static_cast<QWidget *>(obj);
        const bool estBouton = (obj == ui->BLoginCompet);
        if (ev->type() == QEvent::Enter) w->setProperty("survol", true);
        if (ev->type() == QEvent::Leave) w->setProperty("survol", false);
        if (ev->type() == QEvent::Enter || ev->type() == QEvent::Leave
            || ev->type() == QEvent::FocusIn || ev->type() == QEvent::FocusOut) {
            const bool grand = w->property("survol").toBool() || (!estBouton && w->hasFocus());
            const QSize cible = estBouton ? (grand ? QSize(190, 58) : QSize(170, 52))
                                          : (grand ? QSize(400, 52) : QSize(360, 46));
            if (w->size() != cible) animerTaille(w, cible);
        }
        return false;
    }

    // courbes des statistiques, dessinées dans le QLabel « statsChart »
    if (obj == ui->statsChartComp && ev->type() == QEvent::Paint) {
        dessinerCourbesComp(ui->statsChartComp);
        return true;
    }

    // clic sur le nom en haut à droite -> déconnexion
    if (obj == ui->adminComp) {
        if (ev->type() == QEvent::MouseButtonPress) {
            deconnexionComp();
            return true;
        }
        return false;
    }

    // molette sur un champ du formulaire : fait défiler la colonne au lieu de changer la valeur
    if (ev->type() == QEvent::Wheel && obj->property("bloquerMoletteComp").toBool()) {
        ev->ignore();
        return true;
    }
    return false;
}

// ----- Mise en place de la page Compétitions (page 1)
void HackTime::preparerFenetreComp()
{
    ui->SWCompet->setCurrentIndex(0);
    ui->adminComp->setCursor(Qt::PointingHandCursor);
    ui->adminComp->installEventFilter(this);
    ui->rightScrollComp->viewport()->setAutoFillBackground(false);
    ui->rightColComp->setAutoFillBackground(false);
    clignoter(ui->logoComp, this)->start();
    ui->statsChartComp->setMinimumHeight(180);
    ui->statsChartComp->installEventFilter(this);

    // tableau
    QTableWidget *t = ui->tableComp;
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    const int largeurs[] = {70, 0, 170, 100, 100, 110, 100, 100, 120};
    for (int i = 0; i < t->columnCount() && i < 9; ++i)
        if (largeurs[i]) t->setColumnWidth(i, largeurs[i]);
    t->setColumnHidden(0, true);   // l'ID reste dans le tableau (pour les actions) mais n'est pas affiché
    ui->aIdLabelComp->hide();
    ui->aIdComp->hide();
    ui->dIdComp->hide();
    connect(t, &QTableWidget::itemSelectionChanged, this, [this]() {
        const auto lignes = ui->tableComp->selectionModel()->selectedRows();
        if (lignes.isEmpty()) return;
        idSelectionneComp = ui->tableComp->item(lignes.first().row(), 0)->text().toInt();
        afficherDetailsComp();
    });

    // MÉTIER : recherche, filtres et tri
    ui->fStatutComp->addItem("Tous");  ui->fStatutComp->addItems(STATUTS);
    ui->fTheme->addItem("Tous");   ui->fTheme->addItems(THEMES);
    ui->fDateComp->addItem("Toutes");
    ui->fSortComp->addItems({"Date début ↑", "Date début ↓", "Date fin ↑", "Date fin ↓", "Nom (A → Z)"});

    // recherche par nom : texte d'aide en gris
    for (QLineEdit *e : {ui->searchComp, ui->topSearchComp}) {
        e->setPlaceholderText("Rechercher un nom de compétition");
        QPalette pal = e->palette();
        pal.setColor(QPalette::PlaceholderText, QColor("#94A3B8"));
        e->setPalette(pal);
        e->setClearButtonEnabled(true);
    }
    // la recherche du haut et celle de la liste écrivent dans le même champ
    connect(ui->topSearchComp, &QLineEdit::textChanged, ui->searchComp, &QLineEdit::setText);
    connect(ui->searchComp, &QLineEdit::textChanged, this, [this]() { pageComp = 0; appliquerFiltresComp(); });
    for (QComboBox *c : {ui->fStatutComp, ui->fTheme, ui->fDateComp, ui->fSortComp})
        connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this](int) { pageComp = 0; appliquerFiltresComp(); });

    // formulaire d'ajout / de modification
    ui->aTheme->addItems(THEMES);
    ui->aMode->addItems(MODES);
    ui->aStatut->addItems(STATUTS);
    ui->aSalleComp->addItems(SALLES);
    ui->aConflictComp->hide();
    for (QWidget *f : {(QWidget *)ui->aTheme, (QWidget *)ui->aDebut, (QWidget *)ui->aFin,
                       (QWidget *)ui->aMode, (QWidget *)ui->aMax, (QWidget *)ui->aSalleComp,
                       (QWidget *)ui->aStatut}) {
        f->setFocusPolicy(Qt::StrongFocus);
        f->setProperty("bloquerMoletteComp", true);
        f->installEventFilter(this);
    }
    connect(ui->aNom, &QLineEdit::returnPressed, this, &HackTime::on_btnSaveComp_clicked);
    // nom : lettres seulement (accents, espaces, tirets et apostrophes acceptés) ; les chiffres ne s'écrivent pas
    ui->aNom->setValidator(new QRegularExpressionValidator(QRegularExpression("[\\p{L} '\\-]{0,100}"), ui->aNom));
    connect(ui->aMode, &QComboBox::currentTextChanged, this,
            [this](const QString &m) { ui->aSalleComp->setEnabled(m != "En ligne"); });

    // MÉTIERS : calendrier, export PDF, export Excel
    connect(ui->btnCalendar, &QPushButton::clicked, this, &HackTime::ouvrirCalendrierComp);
    connect(ui->btnExportPdfComp, &QPushButton::clicked, this, &HackTime::exporterPdfComp);
    connect(ui->btnExportExcelComp, &QPushButton::clicked, this, &HackTime::exporterExcelComp);

    // en revenant sur la page Compétitions, la liste des jurys est relue
    // (des employés JURY ont pu être ajoutés dans le module Employés)
    connect(ui->stackedHackTime, &QStackedWidget::currentChanged, this, [this]() {
        if (ui->stackedHackTime->currentWidget() != ui->pageCompetitions || roleComp.isEmpty()) return;
        const QVariant choisi = ui->aJury->currentData();
        QSignalBlocker bloque(ui->aJury);
        ui->aJury->clear();
        for (const auto &j : Competition::listerJurys())
            ui->aJury->addItem(j.second, j.first);          // texte = nom, donnée = ID_EMPLOYE
        ui->aJury->setCurrentIndex(ui->aJury->findData(choisi));
    });

    on_btnClearComp_clicked();
}

// ----- AFFICHER (lecture dans la base)
void HackTime::afficherComp()
{
    // liste déroulante « Jury » : les employés de rôle JURY (table EMPLOYE)
    {
        const QVariant choisi = ui->aJury->currentData();
        QSignalBlocker bloque(ui->aJury);
        ui->aJury->clear();
        for (const auto &j : Competition::listerJurys())
            ui->aJury->addItem(j.second, j.first);          // texte = nom, donnée = ID_EMPLOYE
        ui->aJury->setCurrentIndex(ui->aJury->findData(choisi));
    }

    listeComp = Competition::lister();
    chargerInfosComp();
    if (indexDeComp(idSelectionneComp) < 0)
        idSelectionneComp = listeComp.isEmpty() ? 0 : listeComp.first().getId();
    remplirAnneesComp();
    appliquerFiltresComp();
    mettreAJourStatistiquesComp();
    afficherDetailsComp();
}

// ----- MÉTIER : recherche, filtres et tri
// garde dans « filtree » les compétitions qui correspondent à la recherche
// et aux filtres, triées selon « Trier par », puis remplit le tableau
void HackTime::appliquerFiltresComp()
{
    const QString recherche = ui->searchComp->text().trimmed().toLower();
    const QString statut = ui->fStatutComp->currentText();
    const QString theme = ui->fTheme->currentText();
    const QString annee = ui->fDateComp->currentText();

    filtreeComp.clear();
    for (const Competition &c : listeComp) {
        // recherche : uniquement dans le nom de la compétition
        if (!recherche.isEmpty() && !c.getNom().toLower().contains(recherche)) continue;
        if (statut != "Tous" && c.getStatut() != statut) continue;
        if (theme != "Tous" && c.getTheme() != theme) continue;
        if (annee != "Toutes" && QString::number(c.getDateDebut().year()) != annee) continue;
        filtreeComp.append(c);
    }

    const int tri = ui->fSortComp->currentIndex();
    std::stable_sort(filtreeComp.begin(), filtreeComp.end(), [tri](const Competition &a, const Competition &b) {
        switch (tri) {
        case 0: return a.getDateDebut() < b.getDateDebut();   // Date début ↑
        case 1: return a.getDateDebut() > b.getDateDebut();   // Date début ↓
        case 2: return a.getDateFin() < b.getDateFin();       // Date fin ↑
        case 3: return a.getDateFin() > b.getDateFin();       // Date fin ↓
        default: return a.getNom().localeAwareCompare(b.getNom()) < 0;   // Nom (A → Z)
        }
    });

    const int nbPages = std::max(1, (int(filtreeComp.size()) + PAR_PAGE - 1) / PAR_PAGE);
    pageComp = std::max(0, std::min(pageComp, nbPages - 1));

    // la carte « Détails » suit le résultat : rien si aucun nom ne correspond
    bool visible = false;
    for (const Competition &c : filtreeComp)
        if (c.getId() == idSelectionneComp) visible = true;
    if (!visible) idSelectionneComp = filtreeComp.isEmpty() ? 0 : filtreeComp.first().getId();
    remplirTableauComp();
    afficherDetailsComp();
}

// filtre « Année » : les années de début présentes dans la base
void HackTime::remplirAnneesComp()
{
    const QString actuelle = ui->fDateComp->currentText();
    QStringList annees;
    for (const Competition &c : listeComp) {
        const QString a = QString::number(c.getDateDebut().year());
        if (!annees.contains(a)) annees << a;
    }
    annees.sort();
    QSignalBlocker bloque(ui->fDateComp);
    ui->fDateComp->clear();
    ui->fDateComp->addItem("Toutes");
    ui->fDateComp->addItems(annees);
    const int i = ui->fDateComp->findText(actuelle);
    ui->fDateComp->setCurrentIndex(i >= 0 ? i : 0);
}

// remet la recherche et les filtres à zéro (après un ajout)
void HackTime::reinitialiserFiltresComp()
{
    QSignalBlocker b1(ui->searchComp), b2(ui->topSearchComp), b3(ui->fStatutComp), b4(ui->fTheme), b5(ui->fDateComp);
    ui->searchComp->clear();
    ui->topSearchComp->clear();
    ui->fStatutComp->setCurrentIndex(0);
    ui->fTheme->setCurrentIndex(0);
    ui->fDateComp->setCurrentIndex(0);
}

void HackTime::remplirTableauComp()
{
    const int nbPages = std::max(1, (int(filtreeComp.size()) + PAR_PAGE - 1) / PAR_PAGE);
    const int debut = pageComp * PAR_PAGE;
    const int nb = std::max(0, std::min(PAR_PAGE, int(filtreeComp.size()) - debut));

    QTableWidget *t = ui->tableComp;
    {
        QSignalBlocker bloque(t);
        t->clearContents();
        t->setRowCount(nb);
        for (int r = 0; r < nb; ++r) {
            const Competition &c = filtreeComp.at(debut + r);
            auto cellule = [](const QString &texte, bool centre = false) {
                QTableWidgetItem *it = new QTableWidgetItem(texte);
                if (centre) it->setTextAlignment(Qt::AlignCenter);
                return it;
            };
            QTableWidgetItem *idItem = cellule(QString::number(c.getId()));
            idItem->setForeground(QColor("#1665E8"));
            t->setItem(r, 0, idItem);
            t->setItem(r, 1, cellule(c.getNom()));
            t->setItem(r, 2, cellule(themeIcon(c.getTheme()) + "  " + c.getTheme()));
            t->setItem(r, 3, cellule(c.getDateDebut().toString("dd/MM/yyyy")));
            t->setItem(r, 4, cellule(c.getDateFin().toString("dd/MM/yyyy")));
            t->setItem(r, 5, cellule(modeIcon(c.getMode()) + "  " + c.getMode()));
            t->setItem(r, 6, cellule(QString::number(c.getNbMaxMembres()), true));
            t->setCellWidget(r, 7, creerBadgeComp(c.getStatut()));
            t->setCellWidget(r, 8, creerActionsComp(c.getId()));
            if (c.getId() == idSelectionneComp) t->selectRow(r);
        }
    }
    const int total = int(filtreeComp.size());
    ui->footerComp->setText(QString("Affichage de %1 à %2 sur %3 compétitions")
                            .arg(total ? debut + 1 : 0).arg(debut + nb).arg(total));
    construirePagesComp(nbPages);
}

// boutons ‹ 1 2 3 › sous le tableau
void HackTime::construirePagesComp(int nbPages)
{
    while (QLayoutItem *it = ui->pagerLayoutComp->takeAt(0)) {
        if (QWidget *w = it->widget()) { w->hide(); w->deleteLater(); }
        delete it;
    }
    auto aller = [this](int p) { pageComp = p; remplirTableauComp(); };
    QPushButton *prec = new QPushButton("‹");
    prec->setObjectName("page");
    prec->setEnabled(pageComp > 0);
    connect(prec, &QPushButton::clicked, this, [this, aller]() { aller(pageComp - 1); });
    ui->pagerLayoutComp->addWidget(prec);
    for (int p = 0; p < nbPages; ++p) {
        QPushButton *b = new QPushButton(QString::number(p + 1));
        b->setObjectName(p == pageComp ? "pageOn" : "page");
        connect(b, &QPushButton::clicked, this, [aller, p]() { aller(p); });
        ui->pagerLayoutComp->addWidget(b);
    }
    QPushButton *suiv = new QPushButton("›");
    suiv->setObjectName("page");
    suiv->setEnabled(pageComp < nbPages - 1);
    connect(suiv, &QPushButton::clicked, this, [this, aller]() { aller(pageComp + 1); });
    ui->pagerLayoutComp->addWidget(suiv);
}

QWidget *HackTime::creerBadgeComp(const QString &statut)
{
    QLabel *lab = new QLabel(statut);
    lab->setAlignment(Qt::AlignCenter);
    lab->setStyleSheet(QString("background:%1;color:%2;border-radius:11px;padding:3px 12px;"
                               "font-size:12px;").arg(statutBg(statut), statutFg(statut)));
    QWidget *w = new QWidget;
    QHBoxLayout *l = new QHBoxLayout(w);
    l->setContentsMargins(4, 0, 4, 0);
    l->addWidget(lab);
    return w;
}

// 👁 voir   ✏️ modifier   🗑️ supprimer
QWidget *HackTime::creerActionsComp(int id)
{
    QWidget *w = new QWidget;
    QHBoxLayout *l = new QHBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(2);
    QPushButton *voir = new QPushButton("👁"), *modif = new QPushButton("✏️"), *suppr = new QPushButton("🗑️");
    voir->setToolTip("Voir les détails");
    modif->setToolTip("Modifier");
    suppr->setToolTip("Supprimer");
    for (QPushButton *b : {voir, modif, suppr}) {
        b->setObjectName("act");
        b->setCursor(Qt::PointingHandCursor);
        l->addWidget(b);
    }
    modif->setVisible(peutModifierComp());    // le jury ne modifie pas
    suppr->setVisible(peutSupprimerComp());   // seul l'administrateur supprime
    connect(voir, &QPushButton::clicked, this, [this, id]() {
        idSelectionneComp = id;
        afficherDetailsComp();
    });
    connect(modif, &QPushButton::clicked, this, [this, id]() { chargerFormulaireComp(id); });
    connect(suppr, &QPushButton::clicked, this, [this, id]() { supprimerComp(id); });
    return w;
}

int HackTime::indexDeComp(int id) const
{
    for (int i = 0; i < listeComp.size(); ++i)
        if (listeComp.at(i).getId() == id) return i;
    return -1;
}

// ----- AJOUTER / MODIFIER (formulaire de la colonne droite)
Competition HackTime::lireFormulaireComp() const
{
    Competition c(ui->aNom->text(), ui->aTheme->currentText(), ui->aDebut->date(),
                  ui->aFin->date(), ui->aMax->value(), ui->aStatut->currentText(),
                  ui->aMode->currentText());
    c.setId(idEnModificationComp);
    c.setIdJury(ui->aJury->currentData().toInt());   // colonne ID_JURY dans Oracle
    return c;
}

void HackTime::on_btnSaveComp_clicked()
{
    if (!peutModifierComp()) return;
    Competition c = lireFormulaireComp();
    const QString erreur = c.verifier();
    if (!erreur.isEmpty()) {
        ui->aErrComp->setText(erreur);
        ui->aErrComp->show();
        return;
    }
    ui->aErrComp->hide();

    const bool ajout = (idEnModificationComp == 0);
    const bool ok = ajout ? c.ajouter() : c.modifier();   // CRUD dans la base Oracle
    if (!ok) {
        QMessageBox::critical(this, ajout ? "Ajout" : "Modification",
                              (ajout ? "La compétition n'a pas été ajoutée :\n"
                                     : "La compétition n'a pas été modifiée :\n")
                                  + Competition::derniereErreur());
        return;
    }

    // informations de l'interface (pas dans la table COMPETITION)
    QStringList noms;
    for (const QString &s : ui->aSponsors->text().split(',', Qt::SkipEmptyParts))
        if (!s.simplified().isEmpty()) noms << s.simplified();
    infosComp[c.getId()] = {c.getMode() == "En ligne" ? QString() : ui->aSalleComp->currentText().simplified(),
                        QString(),   // le jury est enregistré dans Oracle (ID_JURY)
                        ui->aDescription->toPlainText().trimmed(), noms.join(", ")};
    enregistrerInfosComp();

    QMessageBox::information(this, ajout ? "Ajout" : "Modification",
                             QString("Compétition « %1 » %2.").arg(c.getNom(), ajout ? "ajoutée" : "modifiée"));
    idSelectionneComp = c.getId();
    on_btnClearComp_clicked();
    if (ajout) reinitialiserFiltresComp();   // la nouvelle compétition doit être visible
    afficherComp();
    if (ajout) {   // aller à la page du nouvel élément
        for (int i = 0; i < filtreeComp.size(); ++i)
            if (filtreeComp.at(i).getId() == idSelectionneComp) pageComp = i / PAR_PAGE;
        remplirTableauComp();
    }
}

void HackTime::on_btnClearComp_clicked()
{
    idEnModificationComp = 0;
    ui->lblAddComp->setText("Ajouter une compétition");
    ui->btnSaveComp->setText("Enregistrer");
    ui->aIdComp->setText("Auto");
    ui->aNom->clear();
    ui->aTheme->setCurrentIndex(0);
    ui->aDebut->setDate(QDate::currentDate());
    ui->aFin->setDate(QDate::currentDate().addDays(2));
    ui->aMode->setCurrentIndex(0);
    ui->aMax->setValue(5);
    ui->aStatut->setCurrentIndex(0);
    ui->aSalleComp->setCurrentIndex(-1);
    ui->aJury->setCurrentIndex(-1);
    ui->aDescription->clear();
    ui->aSponsors->clear();
    ui->aErrComp->hide();
}

void HackTime::chargerFormulaireComp(int id)
{
    const int i = indexDeComp(id);
    if (i < 0 || !peutModifierComp()) return;
    const Competition &c = listeComp.at(i);
    const QStringList x = infosComp.value(id, VIDE);

    idEnModificationComp = id;
    idSelectionneComp = id;
    ui->lblAddComp->setText("Modifier la compétition");
    ui->btnSaveComp->setText("Modifier");
    ui->aIdComp->setText(QString::number(id));
    ui->aNom->setText(c.getNom());
    ui->aTheme->setCurrentText(c.getTheme());
    ui->aDebut->setDate(c.getDateDebut());
    ui->aFin->setDate(c.getDateFin());
    ui->aMode->setCurrentText(c.getMode());
    ui->aMax->setValue(c.getNbMaxMembres());
    ui->aStatut->setCurrentText(c.getStatut());
    ui->aSalleComp->setCurrentText(x.value(0));
    // jury : l'employé ID_JURY lu dans Oracle
    ui->aJury->setCurrentIndex(ui->aJury->findData(c.getIdJury()));
    ui->aDescription->setPlainText(x.value(2));
    ui->aSponsors->setText(x.value(3));
    ui->aErrComp->hide();
    afficherDetailsComp();
    ui->rightScrollComp->ensureWidgetVisible(ui->cardAddComp);
    ui->aNom->setFocus();
}

// ----- SUPPRIMER
void HackTime::supprimerComp(int id)
{
    const int i = indexDeComp(id);
    if (i < 0 || !peutSupprimerComp()) return;
    const auto rep = QMessageBox::question(this, "Supprimer",
                                           QString("Supprimer la compétition « %1 » ?").arg(listeComp.at(i).getNom()));
    if (rep != QMessageBox::Yes) return;
    if (!Competition::supprimer(id)) {   // CRUD dans la base Oracle
        QMessageBox::critical(this, "Supprimer", "La compétition n'a pas été supprimée :\n"
                                                     + Competition::derniereErreur());
        return;
    }
    infosComp.remove(id);
    enregistrerInfosComp();
    if (idEnModificationComp == id) on_btnClearComp_clicked();
    afficherComp();
}

// ----- Informations de l'interface (salle, jury, description, sponsors)
static QString cheminInfos()
{
    return QCoreApplication::applicationDirPath() + "/competitions_extra.json";
}

void HackTime::chargerInfosComp()
{
    infosComp.clear();
    QFile f(cheminInfos());
    if (!f.open(QIODevice::ReadOnly)) return;
    const QJsonObject tout = QJsonDocument::fromJson(f.readAll()).object();
    for (auto it = tout.begin(); it != tout.end(); ++it) {
        const QJsonObject o = it.value().toObject();
        infosComp[it.key().toInt()] = {o.value("salle").toString(), o.value("jury").toString(),
                                   o.value("description").toString(), o.value("sponsors").toString()};
    }
}

void HackTime::enregistrerInfosComp()
{
    QJsonObject tout;
    for (auto it = infosComp.cbegin(); it != infosComp.cend(); ++it) {
        QJsonObject o;
        o["salle"] = it.value().value(0);
        o["jury"] = it.value().value(1);
        o["description"] = it.value().value(2);
        o["sponsors"] = it.value().value(3);
        tout[QString::number(it.key())] = o;
    }
    QFile f(cheminInfos());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QJsonDocument(tout).toJson());
}

// ----- Affichage seulement : détails, chiffres, statistiques
void HackTime::afficherDetailsComp()
{
    const int i = indexDeComp(idSelectionneComp);
    if (i < 0) {
        ui->dIcoComp->setText("🏆");
        ui->dNomComp->setText("Aucune sélection");
        ui->dIdComp->clear();
        ui->dBadgeComp->clear();
        ui->dBadgeComp->setStyleSheet("");
        for (QLabel *v : {ui->dTheme, ui->dDebut, ui->dFin, ui->dMode, ui->dMax, ui->dSalleComp,
                          ui->dJury, ui->dDescription, ui->dSponsors})
            v->setText("-");
        return;
    }
    const Competition &c = listeComp.at(i);
    const QStringList x = infosComp.value(c.getId(), VIDE);
    QLocale fr(QLocale::French);
    ui->dIcoComp->setText(themeIcon(c.getTheme()));
    ui->dNomComp->setText(c.getNom());
    ui->dIdComp->setText("ID : " + QString::number(c.getId()));
    ui->dBadgeComp->setText(c.getStatut());
    ui->dBadgeComp->setStyleSheet(QString("background:%1;color:%2;border-radius:10px;padding:2px 10px;"
                                      "font-size:11px;").arg(statutBg(c.getStatut()), statutFg(c.getStatut())));
    ui->dTheme->setText(c.getTheme());
    ui->dDebut->setText(fr.toString(c.getDateDebut(), "d MMMM yyyy"));
    ui->dFin->setText(fr.toString(c.getDateFin(), "d MMMM yyyy"));
    ui->dMode->setText(modeIcon(c.getMode()) + "  " + c.getMode());
    ui->dMax->setText(QString::number(c.getNbMaxMembres()));
    ui->dSalleComp->setText(c.getMode() == "En ligne" ? QString("En ligne (pas de salle)") : ouTiret(x.value(0)));
    ui->dJury->setText(ouTiret(c.getNomJury()));
    ui->dDescription->setText(ouTiret(x.value(2)));
    ui->dSponsors->setText(ouTiret(x.value(3)));
}

void HackTime::on_btnDetailsComp_clicked()
{
    const int i = indexDeComp(idSelectionneComp);
    if (i < 0) return;
    const Competition &c = listeComp.at(i);
    const QStringList x = infosComp.value(c.getId(), VIDE);
    QMessageBox::information(this, c.getNom(),
        QString("Thème : %1\nDébut : %2\nFin : %3\nMode : %4\nMembres max : %5\nStatut : %6")
                .arg(c.getTheme(), c.getDateDebut().toString("dd/MM/yyyy"),
                     c.getDateFin().toString("dd/MM/yyyy"), c.getMode())
                .arg(c.getNbMaxMembres()).arg(c.getStatut())
            + QString("\nSalle : %1\nJury : %2\nSponsors : %3\n\n%4")
                  .arg(ouTiret(x.value(0)), ouTiret(c.getNomJury()), ouTiret(x.value(3)), x.value(2)));
}

void HackTime::mettreAJourStatistiquesComp()
{
    const int total = int(listeComp.size());
    int equipes = 0;
    QMap<QString, int> parStatut, parTheme;
    QDate premier, dernier;
    for (const Competition &c : listeComp) {
        equipes += c.getNbMaxMembres();
        parStatut[c.getStatut()]++;
        parTheme[c.getTheme()]++;
        const QDate d = c.getDateDebut();
        if (!d.isValid()) continue;
        if (!premier.isValid() || d < premier) premier = d;
        if (!dernier.isValid() || d > dernier) dernier = d;
    }
    ui->kpiTotalComp->setText(QString::number(total));
    ui->kpiEquipes->setText(QString::number(equipes));
    ui->kpiPart->setText(QString::number(equipes * 4));   // estimation : 4 membres par équipe
    ui->kpiCours->setText(QString::number(parStatut.value("En cours")));

    // courbes : une valeur par mois de début
    moisComp.clear();
    equipesParMoisComp.clear();
    participantsParMoisComp.clear();
    if (premier.isValid()) {
        QLocale fr(QLocale::French);
        for (QDate m(premier.year(), premier.month(), 1); m <= dernier; m = m.addMonths(1)) {
            int n = 0;
            for (const Competition &c : listeComp)
                if (c.getDateDebut().year() == m.year() && c.getDateDebut().month() == m.month())
                    n += c.getNbMaxMembres();
            moisComp << fr.toString(m, "MMM yy");
            equipesParMoisComp << n;
            participantsParMoisComp << n * 4;
        }
    }
    ui->statsChartComp->setText(QString());
    ui->statsChartComp->update();

    const int faites = parStatut.value("En cours") + parStatut.value("Terminée");
    ui->lblTaux->setText(QString("<b>Taux de participation : %1 %</b>  (compétitions en cours ou terminées)")
                             .arg(total ? qRound(100.0 * faites / total) : 0));
    ui->legCoursComp->setText(QString::number(parStatut.value("En cours")));
    ui->legPlanifiee->setText(QString::number(parStatut.value("Planifiée")));
    ui->legTerminee->setText(QString::number(parStatut.value("Terminée")));

    // une barre par thème, du plus fréquent au moins fréquent
    QList<QPair<int, QString>> themes;
    for (auto it = parTheme.cbegin(); it != parTheme.cend(); ++it) themes.append({it.value(), it.key()});
    std::sort(themes.begin(), themes.end(), [](const QPair<int, QString> &a, const QPair<int, QString> &b) {
        return a.first != b.first ? a.first > b.first : a.second < b.second;
    });
    while (QLayoutItem *it = ui->themesLayout->takeAt(0)) {
        if (QWidget *w = it->widget()) { w->hide(); w->deleteLater(); }
        delete it;
    }
    for (const auto &t : themes) {
        QWidget *ligne = new QWidget;
        QHBoxLayout *l = new QHBoxLayout(ligne);
        l->setContentsMargins(0, 0, 0, 0);
        QLabel *nom = new QLabel(themeIcon(t.second) + "  " + t.second);
        nom->setMinimumWidth(150);
        QProgressBar *barre = new QProgressBar;
        barre->setObjectName("themeBar");
        barre->setRange(0, qMax(1, total));
        barre->setValue(t.first);
        barre->setTextVisible(false);
        barre->setFixedHeight(8);
        QLabel *nb = new QLabel(QString::number(t.first));
        nb->setMinimumWidth(20);
        nb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        l->addWidget(nom);
        l->addWidget(barre, 1);
        l->addWidget(nb);
        ui->themesLayout->addWidget(ligne);
    }
}

// deux courbes (équipes, participants estimés), un point par mois
void HackTime::dessinerCourbesComp(QWidget *zone)
{
    QPainter p(zone);
    p.setRenderHint(QPainter::Antialiasing);
    const QColor grille("#E5EAF3"), axe("#94A3B8"), gris("#64748B");
    const QStringList noms = {"Équipes", "Participants (estimés)"};
    const QList<QColor> couleurs = {QColor("#2563EB"), QColor("#FF6B00")};
    const QVector<QVector<double>> series = {equipesParMoisComp, participantsParMoisComp};

    // légende
    QFont petit = zone->font();
    petit.setPixelSize(11);
    p.setFont(petit);
    qreal lx = 4;
    for (int s = 0; s < 2; ++s) {
        p.setPen(QPen(couleurs[s], 2.4));
        p.drawLine(QPointF(lx, 9), QPointF(lx + 14, 9));
        p.setPen(gris);
        const qreal w = p.fontMetrics().horizontalAdvance(noms[s]);
        p.drawText(QRectF(lx + 18, 0, w + 2, 18), Qt::AlignLeft | Qt::AlignVCenter, noms[s]);
        lx += 18 + w + 14;
    }

    const QRectF zoneCourbe(34, 30, zone->width() - 44, zone->height() - 52);
    if (moisComp.isEmpty()) {
        p.setPen(gris);
        p.drawText(zoneCourbe, Qt::AlignCenter, "Aucune donnée");
        return;
    }

    // échelle « ronde » : 1, 2, 5, 10, 20, 50…
    double maxi = 1;
    for (const auto &s : series)
        for (double v : s) maxi = std::max(maxi, v);
    const int graduations = 4;
    const double brut = std::max(1.0, maxi / graduations);
    const double puissance = std::pow(10.0, std::floor(std::log10(brut)));
    const double n = brut / puissance;
    const double pas = (n <= 1 ? 1 : n <= 2 ? 2 : n <= 5 ? 5 : 10) * puissance;
    const double yMax = pas * graduations;
    const int nb = int(moisComp.size());
    auto Y = [&](double v) { return zoneCourbe.bottom() - zoneCourbe.height() * v / yMax; };
    auto X = [&](int i) {
        return nb <= 1 ? zoneCourbe.center().x() : zoneCourbe.left() + zoneCourbe.width() * i / (nb - 1);
    };

    for (int t = 0; t <= graduations; ++t) {
        const double v = pas * t;
        p.setPen(QPen(t == 0 ? axe : grille, t == 0 ? 1.0 : 0.8));
        p.drawLine(QPointF(zoneCourbe.left(), Y(v)), QPointF(zoneCourbe.right(), Y(v)));
        p.setPen(gris);
        p.drawText(QRectF(0, Y(v) - 8, 28, 16), Qt::AlignRight | Qt::AlignVCenter, QString::number(v));
    }
    const int chaque = std::max(1, (nb + 4) / 5);
    for (int i = 0; i < nb; ++i) {
        if (i % chaque != 0 && i != nb - 1) continue;
        p.drawText(QRectF(X(i) - 32, zoneCourbe.bottom() + 4, 64, 16), Qt::AlignCenter, moisComp[i]);
    }

    for (int s = 0; s < 2; ++s) {
        QPainterPath chemin;
        for (int i = 0; i < nb; ++i) {
            const QPointF pt(X(i), Y(series[s].value(i)));
            if (i == 0) chemin.moveTo(pt); else chemin.lineTo(pt);
        }
        p.setPen(QPen(couleurs[s], 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);
        p.drawPath(chemin);
        for (int i = 0; i < nb; ++i) {
            const QPointF pt(X(i), Y(series[s].value(i)));
            p.setPen(QPen(Qt::white, 1.5));
            p.setBrush(couleurs[s]);
            p.drawEllipse(pt, 3.6, 3.6);
            if (nb <= 8) {
                p.setPen(couleurs[s]);
                p.drawText(QRectF(pt.x() - 20, pt.y() - 19, 40, 14), Qt::AlignCenter,
                           QString::number(series[s].value(i)));
            }
        }
    }
}

// ----- MÉTIER : export PDF (liste des compétitions affichées)
void HackTime::exporterPdfComp()
{
    if (filtreeComp.isEmpty()) {
        QMessageBox::information(this, "Exporter PDF", "Aucune compétition à exporter.");
        return;
    }
    QString chemin = QFileDialog::getSaveFileName(
        this, "Enregistrer le PDF",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Competitions_HackTime.pdf",
        "Document PDF (*.pdf)");
    if (chemin.isEmpty()) return;
    if (!chemin.endsWith(".pdf", Qt::CaseInsensitive)) chemin += ".pdf";

    // le document est écrit en HTML, puis imprimé dans le PDF
    QLocale fr(QLocale::French);
    QString html =
        "<table width='100%'><tr><td><img src='logo' width='170'></td>"
        "<td align='right'><span style='font-size:18pt;font-weight:bold;color:#0E1A2B'>Liste des compétitions</span><br>"
        "<span style='color:#64748B'>Édité le " + fr.toString(QDate::currentDate(), "d MMMM yyyy") +
        " · " + QString::number(filtreeComp.size()) + " compétition(s)</span></td></tr></table><br>"
        "<table width='100%' cellspacing='0' cellpadding='6' style='border-collapse:collapse'>"
        "<tr style='background:#1665E8;color:white;font-weight:bold'>"
        "<td>Nom</td><td>Thème</td><td>Début</td><td>Fin</td><td>Mode</td>"
        "<td>Membres max</td><td>Statut</td><td>Salle</td><td>Jury</td><td>Sponsors</td></tr>";
    int ligne = 0;
    for (const Competition &c : filtreeComp) {
        const QStringList x = infosComp.value(c.getId(), VIDE);
        html += QString("<tr style='background:%1'>").arg(ligne++ % 2 ? "#F3F6FC" : "#FFFFFF");
        const QStringList cellules = {c.getNom(), c.getTheme(),
                                      c.getDateDebut().toString("dd/MM/yyyy"),
                                      c.getDateFin().toString("dd/MM/yyyy"), c.getMode(),
                                      QString::number(c.getNbMaxMembres()), c.getStatut(),
                                      ouTiret(x.value(0)), ouTiret(c.getNomJury()), ouTiret(x.value(3))};
        for (int i = 0; i < cellules.size(); ++i) {
            if (i == 6)   // Statut
                html += QString("<td><span style='color:%1;font-weight:bold'>%2</span></td>")
                            .arg(statutFg(c.getStatut()), cellules.at(i).toHtmlEscaped());
            else
                html += "<td>" + cellules.at(i).toHtmlEscaped() + "</td>";
        }
        html += "</tr>";
    }
    html += "</table><br><p style='color:#94A3B8;font-size:8pt'>HackTime · Gestion des compétitions</p>";

    QPdfWriter pdf(chemin);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageOrientation(QPageLayout::Landscape);
    pdf.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);
    pdf.setTitle("HackTime : liste des compétitions");
    pdf.setCreator("HackTime");

    QTextDocument doc;
    doc.addResource(QTextDocument::ImageResource, QUrl("logo"), QImage(":/images/logo_hacktime_clair.png"));
    doc.setDefaultFont(QFont("Segoe UI", 9));
    doc.setHtml(html);
    doc.print(&pdf);
    afficherFichierComp("Exporter PDF", chemin);
}

// ----- MÉTIER : export Excel (.xlsx)
//  Un .xlsx est une archive ZIP de fichiers XML : elle est écrite ici
//  directement, sans bibliothèque externe (fichiers « stockés », sans compression).
static quint32 calculerCrc32(const QByteArray &data)
{
    static quint32 table[256];
    static bool pret = false;
    if (!pret) {
        for (quint32 i = 0; i < 256; ++i) {
            quint32 c = i;
            for (int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            table[i] = c;
        }
        pret = true;
    }
    quint32 crc = 0xFFFFFFFFu;
    for (char ch : data) crc = table[(crc ^ quint8(ch)) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

static void ecrire16(QByteArray &out, quint16 v)
{
    out.append(char(v & 0xFF));
    out.append(char((v >> 8) & 0xFF));
}

static void ecrire32(QByteArray &out, quint32 v)
{
    for (int i = 0; i < 4; ++i) out.append(char((v >> (8 * i)) & 0xFF));
}

// archive ZIP : liste de {nom du fichier, contenu}
static QByteArray creerZip(const QList<QPair<QByteArray, QByteArray>> &fichiers)
{
    QByteArray out, central;
    for (const auto &f : fichiers) {
        const quint32 position = quint32(out.size());
        const quint32 crc = calculerCrc32(f.second);
        const quint32 taille = quint32(f.second.size());
        ecrire32(out, 0x04034b50); ecrire16(out, 20); ecrire16(out, 0); ecrire16(out, 0);
        ecrire16(out, 0); ecrire16(out, 0x21);
        ecrire32(out, crc); ecrire32(out, taille); ecrire32(out, taille);
        ecrire16(out, quint16(f.first.size())); ecrire16(out, 0);
        out += f.first;
        out += f.second;

        ecrire32(central, 0x02014b50); ecrire16(central, 20); ecrire16(central, 20);
        ecrire16(central, 0); ecrire16(central, 0); ecrire16(central, 0); ecrire16(central, 0x21);
        ecrire32(central, crc); ecrire32(central, taille); ecrire32(central, taille);
        ecrire16(central, quint16(f.first.size())); ecrire16(central, 0); ecrire16(central, 0);
        ecrire16(central, 0); ecrire16(central, 0); ecrire32(central, 0); ecrire32(central, position);
        central += f.first;
    }
    const quint32 debutCentral = quint32(out.size());
    out += central;
    ecrire32(out, 0x06054b50); ecrire16(out, 0); ecrire16(out, 0);
    ecrire16(out, quint16(fichiers.size())); ecrire16(out, quint16(fichiers.size()));
    ecrire32(out, quint32(central.size())); ecrire32(out, debutCentral); ecrire16(out, 0);
    return out;
}

static QString nomColonne(int index)   // 0 -> A, 1 -> B…
{
    QString nom;
    for (int n = index + 1; n > 0; n = (n - 1) / 26) nom.prepend(QChar('A' + (n - 1) % 26));
    return nom;
}

void HackTime::exporterExcelComp()
{
    if (filtreeComp.isEmpty()) {
        QMessageBox::information(this, "Exporter Excel", "Aucune compétition à exporter.");
        return;
    }
    QString chemin = QFileDialog::getSaveFileName(
        this, "Enregistrer le fichier Excel",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Competitions_HackTime.xlsx",
        "Classeur Excel (*.xlsx)");
    if (chemin.isEmpty()) return;
    if (!chemin.endsWith(".xlsx", Qt::CaseInsensitive)) chemin += ".xlsx";

    // lignes du tableau : en-tête, puis une ligne par compétition affichée
    QVector<QStringList> lignes;
    lignes.append({"Nom", "Thème", "Date début", "Date fin", "Mode", "Membres max",
                   "Statut", "Salle", "Jury", "Sponsors", "Description"});
    for (const Competition &c : filtreeComp) {
        const QStringList x = infosComp.value(c.getId(), VIDE);
        lignes.append({c.getNom(), c.getTheme(),
                       c.getDateDebut().toString("dd/MM/yyyy"), c.getDateFin().toString("dd/MM/yyyy"),
                       c.getMode(), QString::number(c.getNbMaxMembres()), c.getStatut(),
                       x.value(0), c.getNomJury(), x.value(3), x.value(2)});
    }
    const QList<int> colonnesNombres = {5};                          // Membres max
    const QList<int> largeurs = {34, 24, 13, 13, 13, 13, 13, 18, 34, 34, 60};

    const QByteArray entete = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    QString feuille =
        "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
        "<sheetViews><sheetView workbookViewId=\"0\">"
        "<pane ySplit=\"1\" topLeftCell=\"A2\" activePane=\"bottomLeft\" state=\"frozen\"/>"
        "</sheetView></sheetViews><cols>";
    for (int i = 0; i < largeurs.size(); ++i)
        feuille += QString("<col min=\"%1\" max=\"%1\" width=\"%2\" customWidth=\"1\"/>").arg(i + 1).arg(largeurs[i]);
    feuille += "</cols><sheetData>";
    for (int r = 0; r < lignes.size(); ++r) {
        feuille += QString("<row r=\"%1\"%2>").arg(r + 1).arg(r == 0 ? " ht=\"22\" customHeight=\"1\"" : "");
        for (int c = 0; c < lignes[r].size(); ++c) {
            const QString ref = nomColonne(c) + QString::number(r + 1);
            const QString style = r == 0 ? "1" : "2";    // 1 = en-tête bleu, 2 = cellule avec bordure
            const QString valeur = lignes[r][c].toHtmlEscaped();
            if (r > 0 && colonnesNombres.contains(c))
                feuille += QString("<c r=\"%1\" s=\"%2\"><v>%3</v></c>").arg(ref, style, valeur);
            else
                feuille += QString("<c r=\"%1\" s=\"%2\" t=\"inlineStr\"><is><t xml:space=\"preserve\">%3</t></is></c>")
                               .arg(ref, style, valeur);
        }
        feuille += "</row>";
    }
    feuille += QString("</sheetData><autoFilter ref=\"A1:%1%2\"/></worksheet>")
                   .arg(nomColonne(lignes[0].size() - 1)).arg(lignes.size());

    const QByteArray styles = entete +
        "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
        "<fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font>"
        "<font><b/><sz val=\"11\"/><color rgb=\"FFFFFFFF\"/><name val=\"Calibri\"/></font></fonts>"
        "<fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill>"
        "<fill><patternFill patternType=\"gray125\"/></fill>"
        "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF1665E8\"/><bgColor indexed=\"64\"/></patternFill></fill></fills>"
        "<borders count=\"2\"><border><left/><right/><top/><bottom/><diagonal/></border>"
        "<border><left style=\"thin\"><color rgb=\"FFD5DFEE\"/></left><right style=\"thin\"><color rgb=\"FFD5DFEE\"/></right>"
        "<top style=\"thin\"><color rgb=\"FFD5DFEE\"/></top><bottom style=\"thin\"><color rgb=\"FFD5DFEE\"/></bottom><diagonal/></border></borders>"
        "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
        "<cellXfs count=\"3\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
        "<xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"1\" xfId=\"0\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\" applyAlignment=\"1\"><alignment vertical=\"center\"/></xf>"
        "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"1\" xfId=\"0\" applyBorder=\"1\" applyAlignment=\"1\"><alignment vertical=\"center\"/></xf></cellXfs>"
        "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles></styleSheet>";
    const QByteArray classeur = entete +
        "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
        "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
        "<sheets><sheet name=\"Compétitions\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>";
    const QByteArray liensClasseur = entete +
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
        "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>"
        "</Relationships>";
    const QByteArray liensRacine = entete +
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
        "</Relationships>";
    const QByteArray types = entete +
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
        "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
        "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
        "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
        "<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
        "</Types>";

    const QByteArray xlsx = creerZip({{"[Content_Types].xml", types},
                                      {"_rels/.rels", liensRacine},
                                      {"xl/workbook.xml", classeur},
                                      {"xl/_rels/workbook.xml.rels", liensClasseur},
                                      {"xl/styles.xml", styles},
                                      {"xl/worksheets/sheet1.xml", entete + feuille.toUtf8()}});
    QFile f(chemin);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, "Exporter Excel",
                             "Impossible d'écrire le fichier. Il est peut-être ouvert dans Excel : fermez-le et réessayez.");
        return;
    }
    f.write(xlsx);
    f.close();
    afficherFichierComp("Exporter Excel", chemin);
}

// message de fin d'export, avec un bouton pour ouvrir le fichier
void HackTime::afficherFichierComp(const QString &titre, const QString &chemin)
{
    QMessageBox boite(QMessageBox::Information, titre,
                      "Fichier enregistré :\n" + QDir::toNativeSeparators(chemin), QMessageBox::NoButton, this);
    QPushButton *ouvrir = boite.addButton("Ouvrir", QMessageBox::AcceptRole);
    boite.addButton("Fermer", QMessageBox::RejectRole);
    boite.exec();
    if (boite.clickedButton() == ouvrir) QDesktopServices::openUrl(QUrl::fromLocalFile(chemin));
}

// ----- MÉTIER : calendrier des compétitions
//  Les jours occupés sont colorés selon le statut ; un clic sur un jour liste
//  les compétitions de ce jour ; un double-clic sélectionne la compétition.
void HackTime::ouvrirCalendrierComp()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Calendrier des compétitions");
    dlg.resize(600, 680);

    QCalendarWidget *cal = new QCalendarWidget(&dlg);
    cal->setGridVisible(true);
    cal->setFirstDayOfWeek(Qt::Monday);
    cal->setLocale(QLocale(QLocale::French));
    cal->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    cal->setMinimumHeight(330);

    // colorer chaque jour entre la date de début et la date de fin
    for (const Competition &c : listeComp) {
        QTextCharFormat format;
        format.setBackground(QColor(statutBg(c.getStatut())));
        format.setForeground(QColor(statutFg(c.getStatut())));
        format.setFontWeight(QFont::Bold);
        for (QDate d = c.getDateDebut(); d.isValid() && d <= c.getDateFin(); d = d.addDays(1))
            cal->setDateTextFormat(d, format);
    }

    QLabel *legende = new QLabel(
        QString("<span style='color:%1'>■</span> Planifiée &nbsp; <span style='color:%2'>■</span> En cours"
                " &nbsp; <span style='color:%3'>■</span> Terminée")
            .arg(statutFg("Planifiée"), statutFg("En cours"), statutFg("Terminée")), &dlg);
    QLabel *titreJour = new QLabel(&dlg);
    titreJour->setStyleSheet("font-weight:700; font-size:14px;");
    QLabel *infoJour = new QLabel(&dlg);
    infoJour->setStyleSheet("color:#64748B; font-size:12px;");
    QListWidget *jour = new QListWidget(&dlg);
    QLabel *aide = new QLabel("Clic : afficher la compétition dans le calendrier  ·  "
                              "Double-clic : l'afficher dans les détails.", &dlg);
    aide->setStyleSheet("color:#64748B; font-size:11px;");

    // toutes les compétitions, de la plus ancienne à la plus récente
    QVector<Competition> triees = listeComp;
    std::sort(triees.begin(), triees.end(), [](const Competition &x, const Competition &y) {
        return x.getDateDebut() < y.getDateDebut();
    });

    // la liste affiche TOUTES les compétitions ; celles du jour choisi sont mises en avant
    auto remplirJour = [titreJour, infoJour, jour, triees](const QDate &d) {
        QLocale fr(QLocale::French);
        titreJour->setText(QString("Toutes les compétitions (%1)").arg(triees.size()));
        jour->clear();
        int duJour = 0;
        QListWidgetItem *premier = nullptr;
        for (const Competition &c : triees) {
            const bool ceJour = d >= c.getDateDebut() && d <= c.getDateFin();
            QListWidgetItem *it = new QListWidgetItem(
                QString("%1%2  %3   ·   %4 → %5   ·   %6")
                    .arg(QString(ceJour ? "▶ " : "   "), themeIcon(c.getTheme()), c.getNom(),
                         c.getDateDebut().toString("dd/MM/yyyy"), c.getDateFin().toString("dd/MM/yyyy"),
                         c.getStatut()));
            it->setData(Qt::UserRole, c.getId());
            it->setForeground(QColor(statutFg(c.getStatut())));
            if (ceJour) {
                QFont gras = it->font();
                gras.setBold(true);
                it->setFont(gras);
                it->setBackground(QColor(statutBg(c.getStatut())));
                if (!premier) premier = it;
                ++duJour;
            }
            jour->addItem(it);
        }
        if (triees.isEmpty()) jour->addItem("Aucune compétition enregistrée.");
        infoJour->setText(QString("Le %1 : %2")
                              .arg(fr.toString(d, "dddd d MMMM yyyy"),
                                   duJour == 0 ? QString("aucune compétition")
                                               : QString("%1 compétition(s), marquée(s) ▶").arg(duJour)));
        if (premier) jour->scrollToItem(premier);
    };
    connect(cal, &QCalendarWidget::clicked, &dlg, remplirJour);
    connect(cal, &QCalendarWidget::selectionChanged, &dlg, [cal, remplirJour]() { remplirJour(cal->selectedDate()); });

    // clic sur une compétition : le calendrier va à sa date de début
    connect(jour, &QListWidget::itemClicked, &dlg, [this, cal](QListWidgetItem *it) {
        const int i = indexDeComp(it->data(Qt::UserRole).toInt());
        if (i < 0) return;
        const QDate debut = listeComp.at(i).getDateDebut();
        cal->setCurrentPage(debut.year(), debut.month());
        cal->setSelectedDate(debut);
    });
    // double-clic : la compétition s'affiche dans la carte « Détails »
    connect(jour, &QListWidget::itemDoubleClicked, &dlg, [this, &dlg](QListWidgetItem *it) {
        const int id = it->data(Qt::UserRole).toInt();
        if (id <= 0) return;
        idSelectionneComp = id;
        afficherDetailsComp();
        dlg.accept();
    });

    // ouvrir sur la compétition sélectionnée (sinon aujourd'hui), au bon mois
    const int i = indexDeComp(idSelectionneComp);
    const QDate depart = i >= 0 ? listeComp.at(i).getDateDebut() : QDate::currentDate();
    cal->setCurrentPage(depart.year(), depart.month());
    cal->setSelectedDate(depart);
    remplirJour(depart);

    QVBoxLayout *l = new QVBoxLayout(&dlg);
    l->addWidget(cal);
    l->addWidget(legende);
    l->addWidget(titreJour);
    l->addWidget(infoJour);
    l->addWidget(jour, 1);
    l->addWidget(aide);
    dlg.exec();
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
