#include "interface.h"
#include "ui_interface.h"
#include <QString>
#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFrame>
#include <QPainter>
#include <QPolygonF>
#include <QPaintEvent>
#include <QPixmap>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QVariant>
#include <QStringList>
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QAbstractItemView>
#include <algorithm>
#include <QTextEdit>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QVector>
#include <QResizeEvent>


class CourbeChartWidget : public QWidget {
    QVector<int> monthlyValues;
    QStringList monthLabels;
public:
    explicit CourbeChartWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setObjectName("courbeChartEquipes");
        setMinimumHeight(150);
        monthlyValues = QVector<int>(6, 0);
        monthLabels = {"M-5", "M-4", "M-3", "M-2", "M-1", "Ce mois"};
    }
    void setData(const QVector<int> &values, const QStringList &labels) {
        monthlyValues = values;
        monthLabels = labels;
        update();
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor("#FFFFFF"));
        const int left = 30, right = width() - 12, top = 12, bottom = height() - 28;
        painter.setPen(QPen(QColor("#E2E8F0"), 1));
        for (int i=0;i<4;++i) {
            int y = top + i * (bottom-top) / 3;
            painter.drawLine(left, y, right, y);
        }
        if (monthlyValues.isEmpty()) return;
        int maximum = 1;
        for (int v : monthlyValues) maximum = qMax(maximum, v);
        painter.setPen(QColor("#64748B"));
        painter.setFont(QFont("Segoe UI", 7));
        painter.drawText(3, top+4, QString::number(maximum));
        painter.drawText(10, bottom, "0");
        QPolygonF points;
        const int n = monthlyValues.size();
        for (int i=0;i<n;++i) {
            qreal x = left + (n == 1 ? 0 : (right-left) * i / qreal(n-1));
            qreal y = bottom - (bottom-top) * monthlyValues[i] / qreal(maximum);
            points << QPointF(x,y);
            if (i < monthLabels.size()) painter.drawText(QRectF(x-24,bottom+5,48,18), Qt::AlignHCenter, monthLabels[i]);
        }
        painter.setPen(QPen(QColor("#2563EB"), 2.5));
        painter.drawPolyline(points);
        painter.setBrush(QColor("#2563EB"));
        painter.setPen(Qt::NoPen);
        for (const QPointF &pt : points) painter.drawEllipse(pt, 3.5, 3.5);
    }
};

class BarChartWidget : public QWidget {
    int activeCount;
    int incompleteCount;
public:
    BarChartWidget(int active, int incomplete, QWidget *parent = nullptr)
        : QWidget(parent), activeCount(active), incompleteCount(incomplete) {
        setObjectName("barChartEquipes");
        setMinimumHeight(120);
    }
    void setData(int active, int incomplete) {
        activeCount = active;
        incompleteCount = incomplete;
        update();
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.fillRect(rect(), QColor("#FFFFFF"));

        painter.setPen(QPen(QColor("#CBD5E1"), 1));
        painter.drawLine(30, 95, width() - 20, 95);
        painter.drawLine(30, 15, 30, 95);

        painter.setPen(QColor("#64748B"));
        painter.setFont(QFont("Segoe UI", 8));
        painter.drawText(10, 20, "10");
        painter.drawText(15, 55, "5");
        painter.drawText(15, 95, "0");

        int maxVal = qMax(1, qMax(activeCount, incompleteCount));
        int h1 = (activeCount * 75) / maxVal;
        int h2 = (incompleteCount * 75) / maxVal;

        painter.setBrush(QColor("#10B981"));
        painter.setPen(Qt::NoPen);
        painter.drawRect(80, 95 - h1, 35, h1);

        painter.setBrush(QColor("#F59E0B"));
        painter.setPen(Qt::NoPen);
        painter.drawRect(180, 95 - h2, 35, h2);

        painter.setPen(QColor("#0F172A"));
        painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
        painter.drawText(75, 112, "Actives");
        painter.drawText(165, 112, "Incomplètes");
        painter.setPen(QColor("#334155"));
        painter.drawText(88, qMax(12, 88 - h1), QString::number(activeCount));
        painter.drawText(188, qMax(12, 88 - h2), QString::number(incompleteCount));
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Harmonisation visuelle avec le module Compétitions (contenu clair, sidebar bleu nuit).
    setMinimumSize(1200, 650);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet(R"(
        QMainWindow, QWidget#centralwidget, QWidget#page {
            background-color: #F4F7FB;
            color: #1E293B;
            font-family: "Segoe UI";
            font-size: 10pt;
        }
        QFrame, QGroupBox {
            background-color: #FFFFFF;
            color: #1E293B;
            border: 1px solid #E2E8F0;
            border-radius: 10px;
        }
        QTableWidget {
            background-color: #FFFFFF;
            alternate-background-color: #F8FAFC;
            color: #1E293B;
            gridline-color: #E2E8F0;
            border: 1px solid #E2E8F0;
            selection-background-color: #DBEAFE;
            selection-color: #1E3A8A;
        }
        QHeaderView::section {
            background-color: #F1F5F9;
            color: #1E293B;
            padding: 8px;
            border: none;
            border-bottom: 1px solid #CBD5E1;
            font-weight: 600;
        }
        QLineEdit, QComboBox, QTextEdit, QDateEdit, QSpinBox {
            background-color: #FFFFFF;
            color: #1E293B;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            padding: 6px;
        }
        QLineEdit:focus, QComboBox:focus, QTextEdit:focus {
            border: 1px solid #2563EB;
        }
        QScrollArea, QScrollArea QWidget {
            background-color: #FFFFFF;
        }
    )");

    if (QWidget *sidebar = findChild<QWidget*>("widget_2")) {
        sidebar->setStyleSheet(R"(
            QWidget { background-color: #0F172A; }
            QPushButton {
                background-color: transparent;
                color: #E2E8F0;
                border: none;
                text-align: left;
                padding: 10px 14px;
                font-size: 11pt;
                min-height: 42px;
                border-radius: 8px;
            }
            QPushButton:hover { background-color: #1E293B; color: #FFFFFF; }
        )");
    }

    // Le panneau droit est construit en C++ : aucune modification du schema Oracle.
    initialiserPanneauDroit();

    // Réserver la partie droite à la fiche permanente et conserver la liste à gauche.
    if (auto frameListe = ui->page->findChild<QFrame*>("frame_5")) {
        frameListe->setGeometry(0, 210, 770, 500);
    }
    ui->lineEdit->setGeometry(10, 40, 190, 28);
    ui->comboBoxCompetition->setGeometry(205, 40, 155, 28);
    ui->comboBoxStatus->setGeometry(365, 40, 105, 28);
    ui->comboBoxTrier->setGeometry(475, 40, 150, 28);
    ui->tableWidget->setGeometry(10, 70, 750, 331);
    ui->btnAjouter->hide();
    ui->btnStatistiques->hide();
    ui->btnPrev->move(610, 420);
    ui->btn1->move(635, 420);
    ui->btn2->move(660, 420);
    ui->btn3->move(685, 420);
    ui->btn4->move(710, 420);
    ui->btnNext->move(735, 420);

    // Logo placé dans la barre latérale sombre pour conserver le texte blanc lisible.
    QWidget *sidebarLogo = findChild<QWidget*>("widget_2");
    QLabel *logoLabel = new QLabel(sidebarLogo ? sidebarLogo : this);
    QPixmap pixmap(":/new/prefix1/logo.png");
    if (!pixmap.isNull()) {
        logoLabel->setPixmap(pixmap.scaled(178, 78, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setGeometry(6, 10, 178, 78);
    logoLabel->setStyleSheet("background: transparent; border: none;");
    logoLabel->raise();

    QGraphicsOpacityEffect *fadeEffect = new QGraphicsOpacityEffect(this);
    logoLabel->setGraphicsEffect(fadeEffect);
    QPropertyAnimation *anim = new QPropertyAnimation(fadeEffect, "opacity", this);
    anim->setDuration(2000);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->start();

    QString btnBaseStyle = R"(
        QPushButton {
            background-color: transparent;
            color: #94A3B8;
            text-align: left;
            padding-left: 20px;
            font-size: 14px;
            font-weight: 500;
            border: none;
            border-radius: 8px;
            min-height: 42px;
        }
        QPushButton:hover {
            background-color: #1E293B;
            color: #FFFFFF;
        }
    )";

    QString btnActiveStyle = R"(
        QPushButton {
            background-color: #2563EB;
            color: #FFFFFF;
            text-align: left;
            padding-left: 20px;
            font-size: 14px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
            min-height: 42px;
        }
    )";

    QList<QPushButton*> sidebarButtons = this->findChildren<QPushButton*>();
    for (QPushButton *btn : sidebarButtons) {
        if (btn == ui->btnAjouter) continue;
        QString text = btn->text().trimmed();
        if (text.contains("Tableau", Qt::CaseInsensitive)) {
            btn->setText("  📊   Tableau de bord");
            btn->setStyleSheet(btnBaseStyle);
        } else if (text.contains("Compétition", Qt::CaseInsensitive)) {
            btn->setText("  🏆   Compétitions");
            btn->setStyleSheet(btnBaseStyle);
        } else if (text.contains("Équipe", Qt::CaseInsensitive) || text.contains("Equipe", Qt::CaseInsensitive)) {
            btn->setText("  👥   Équipes");
            btn->setStyleSheet(btnActiveStyle);
        } else if (text.contains("Participant", Qt::CaseInsensitive)) {
            btn->setText("  👤   Participants");
            btn->setStyleSheet(btnBaseStyle);
        } else if (text.contains("Atelier", Qt::CaseInsensitive)) {
            btn->setText("  🛠️   Atelier");
            btn->setStyleSheet(btnBaseStyle);
        } else if (text.contains("Employé", Qt::CaseInsensitive) || text.contains("Employe", Qt::CaseInsensitive)) {
            btn->setText("  🧑‍💼   Employés");
            btn->setStyleSheet(btnBaseStyle);
        } else if (text.contains("Paramètre", Qt::CaseInsensitive) || text.contains("Parametre", Qt::CaseInsensitive)) {
            btn->setText("  ⚙️   Paramètres");
            btn->setStyleSheet(btnBaseStyle);
        }
    }

    ui->tableWidget->setColumnCount(7);
    ui->tableWidget->setHorizontalHeaderLabels({"Nom de l'équipe", "Compétition", "Chef d'équipe", "Membres", "Date d'inscription", "Statut", "Actions"});
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEnabled(true);

    // Charger les equipes depuis Oracle (et non depuis la liste de donnees fictives).
    toutesLesEquipes.clear();
    if (!chargerEquipesDepuisOracle()) {
        QMessageBox::warning(this, "Oracle",
                             "Impossible de charger les equipes depuis Oracle.\n"
                             "Verifiez la connexion et la table EQUIPE.");
    }
    ui->comboBoxCompetition->clear();
    ui->comboBoxCompetition->addItem("Toutes");
    QStringList nomsCompetitions;
    QSqlDatabase dbCompetitions = QSqlDatabase::database("hackathon_connection");
    if (dbCompetitions.isValid() && dbCompetitions.isOpen()) {
        QSqlQuery competitionsQuery(dbCompetitions);
        if (competitionsQuery.exec("SELECT NOM FROM COMPETITION ORDER BY ID_COMPETITION")) {
            while (competitionsQuery.next())
                nomsCompetitions.append(competitionsQuery.value(0).toString());
        }
    }
    ui->comboBoxCompetition->addItems(nomsCompetitions);
    chargerPage(1);

    qDebug() << "DEBUG - Nombre total :" << toutesLesEquipes.size();
    qDebug() << "DEBUG - Nombre affiche :" << equipesAffichees.size();

    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &MainWindow::onTeamSelected);

    connect(ui->btn1, &QPushButton::clicked, this, [=](){ chargerPage(1); });
    connect(ui->btn2, &QPushButton::clicked, this, [=](){ chargerPage(2); });
    connect(ui->btn3, &QPushButton::clicked, this, [=](){ chargerPage(3); });
    connect(ui->btn4, &QPushButton::clicked, this, [=](){ chargerPage(4); });

    connect(ui->btnPrev, &QPushButton::clicked, this, [=](){ chargerPage(qMax(1, currentPage - 1)); });
    connect(ui->btnNext, &QPushButton::clicked, this, [=](){ chargerPage(qMin(4, currentPage + 1)); });

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &MainWindow::filtrerEquipes);

    connect(ui->comboBoxCompetition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::filtrerParComboBox);
    connect(ui->comboBoxStatus, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::filtrerParComboBox);
    connect(ui->comboBoxTrier, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::filtrerParComboBox);

    connect(ui->btnAjouter, &QPushButton::clicked, this, &MainWindow::on_btnAjouter_clicked);
    connect(ui->btnStatistiques, &QPushButton::clicked, this, &MainWindow::on_btnStatistiques_clicked);

    // Actualiser les equipes pour recuperer les participants ajoutes depuis l'autre interface.
    QTimer *actualisationTimer = new QTimer(this);
    connect(actualisationTimer, &QTimer::timeout, this, [this]() {
        if (chargerEquipesDepuisOracle()) {
            if (!ui->lineEdit->text().trimmed().isEmpty())
                filtrerEquipes(ui->lineEdit->text());
            else
                filtrerParComboBox();
        }
    });
    actualisationTimer->start(30000); // toutes les 30 secondes
    layoutReady = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (!layoutReady || !ui || !ui->centralwidget || !ui->page)
        return;

    // Étendre le conteneur principal à toute la surface disponible.
    if (QWidget *root = ui->centralwidget->findChild<QWidget*>("horizontalLayoutWidget"))
        root->setGeometry(ui->centralwidget->rect());

    const int pageW = ui->page->width();
    const int pageH = ui->page->height();
    const int marge = 10;
    const int largeurPanneauDroit = 320;
    const int espace = 20;
    const int largeurListe = qMax(620, pageW - largeurPanneauDroit - espace - marge);

    if (auto frameListe = ui->page->findChild<QFrame*>("frame_5")) {
        const int hauteurListe = qMax(390, pageH - 220);
        frameListe->setGeometry(0, 210, largeurListe, hauteurListe);

        const int largeurFiltres = largeurListe - 30;
        ui->lineEdit->setGeometry(10, 40, qMax(150, largeurFiltres * 27 / 100), 30);
        ui->comboBoxCompetition->setGeometry(15 + largeurFiltres * 27 / 100, 40,
                                              qMax(130, largeurFiltres * 23 / 100), 30);
        ui->comboBoxStatus->setGeometry(20 + largeurFiltres * 50 / 100, 40,
                                         qMax(100, largeurFiltres * 15 / 100), 30);
        ui->comboBoxTrier->setGeometry(25 + largeurFiltres * 66 / 100, 40,
                                        qMax(125, largeurFiltres * 20 / 100), 30);
        ui->tableWidget->setGeometry(10, 70, largeurListe - 20, hauteurListe - 115);

        const int yPagination = hauteurListe - 34;
        ui->btnPrev->move(largeurListe - 180, yPagination);
        ui->btn1->move(largeurListe - 155, yPagination);
        ui->btn2->move(largeurListe - 130, yPagination);
        ui->btn3->move(largeurListe - 105, yPagination);
        ui->btn4->move(largeurListe - 80, yPagination);
        ui->btnNext->move(largeurListe - 55, yPagination);
    }

    if (panneauDroit) {
        panneauDroit->setGeometry(pageW - largeurPanneauDroit - marge, 210,
                                  largeurPanneauDroit, qMax(390, pageH - 220));
    }

    if (QLabel *titre = ui->page->findChild<QLabel*>("label"))
        titre->setGeometry(10, 30, qMax(780, pageW - 350), 81);
}

void MainWindow::on_btnStatistiques_clicked()
{
    actualiserStatistiques();
}

void MainWindow::initialiserPanneauDroit()
{
    QWidget *page = ui->page;
    panneauDroit = new QFrame(page);
    panneauDroit->setObjectName("panneauDroitEquipes");
    panneauDroit->setGeometry(780, 210, 320, 441);
    panneauDroit->setStyleSheet(
        "QFrame#panneauDroitEquipes { background:#FFFFFF; border:1px solid #DCE5F0; border-radius:9px; }"
        "QLabel { border:0; }"
        "QLineEdit, QComboBox { min-height:28px; padding:3px 7px; border:1px solid #CBD5E1; border-radius:5px; background:#FFFFFF; }"
        "QPushButton { min-height:29px; border-radius:5px; padding:4px 8px; }"
        );

    // Le contenu est défilable : les trois blocs restent dans le panneau fixe.
    QVBoxLayout *outer = new QVBoxLayout(panneauDroit);
    outer->setContentsMargins(5, 5, 5, 5);
    QScrollArea *scroll = new QScrollArea(panneauDroit);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setStyleSheet("QScrollArea { background:transparent; border:0; } QScrollBar:vertical { width:9px; background:#F1F5F9; } QScrollBar::handle:vertical { background:#94A3B8; border-radius:4px; min-height:25px; }");
    QWidget *contenu = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(contenu);
    layout->setContentsMargins(8, 7, 8, 7);
    layout->setSpacing(7);

    QLabel *titreAjout = new QLabel("＋  Ajouter une équipe", contenu);
    titreAjout->setStyleSheet("font-size:15px;font-weight:700;color:#1E293B;");
    layout->addWidget(titreAjout);

    editNomEquipe = new QLineEdit(contenu);
    editNomEquipe->setPlaceholderText("Nom de l'équipe");
    comboNouvelleCompetition = new QComboBox(contenu);
    comboNouvelleCompetition->setPlaceholderText("Choisir une compétition");
    editDateEquipe = new QLineEdit(QDate::currentDate().toString("dd/MM/yyyy"), contenu);
    editDateEquipe->setPlaceholderText("Date d'inscription (JJ/MM/AAAA)");
    layout->addWidget(editNomEquipe);
    layout->addWidget(comboNouvelleCompetition);
    layout->addWidget(editDateEquipe);

    // Chef et membres : le chef est choisi après l'inscription des participants.
    QLabel *lblChefAjout = new QLabel("👑  Chef : à définir après inscription", contenu);
    QLabel *lblMembresAjout = new QLabel("👥  Membres : 0 / —", contenu);
    QLabel *lblStatutAjout = new QLabel("●  Statut : Incomplète", contenu);
    lblStatutAjout->setObjectName("lblStatutNouvelleEquipe");
    lblChefAjout->setStyleSheet("color:#475569;padding:2px;");
    lblMembresAjout->setStyleSheet("color:#475569;padding:2px;");
    lblStatutAjout->setStyleSheet("color:#D97706;font-weight:600;padding:2px;");
    layout->addWidget(lblChefAjout);
    layout->addWidget(lblMembresAjout);
    layout->addWidget(lblStatutAjout);

    // La capacité provient de la compétition choisie (NB_MAX_MEMBRES), sans modifier le schéma.
    connect(comboNouvelleCompetition, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this, lblMembresAjout](int index) {
                if (index < 0) { lblMembresAjout->setText("👥  Membres : 0 / —"); return; }
                QSqlDatabase db = QSqlDatabase::database("hackathon_connection");
                QSqlQuery q(db);
                q.prepare("SELECT NB_MAX_MEMBRES FROM COMPETITION WHERE ID_COMPETITION = ?");
                q.addBindValue(comboNouvelleCompetition->currentData());
                if (q.exec() && q.next())
                    lblMembresAjout->setText(QString("👥  Membres : 0 / %1").arg(q.value(0).toInt()));
            });

    QHBoxLayout *actionsAjout = new QHBoxLayout;
    QPushButton *btnSave = new QPushButton("✓  Enregistrer", contenu);
    QPushButton *btnEffacer = new QPushButton("Effacer", contenu);
    btnSave->setStyleSheet("QPushButton{background:#2563EB;color:white;font-weight:bold;padding:8px;border:0;border-radius:5px;} QPushButton:hover{background:#1D4ED8;}");
    btnEffacer->setStyleSheet("QPushButton{background:#F1F5F9;color:#334155;padding:8px;border:1px solid #CBD5E1;border-radius:5px;}");
    actionsAjout->addWidget(btnSave, 2);
    actionsAjout->addWidget(btnEffacer, 1);
    layout->addLayout(actionsAjout);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::on_btnAjouter_clicked);
    connect(btnEffacer, &QPushButton::clicked, this, [this, lblMembresAjout, lblStatutAjout]() {
        editNomEquipe->clear();
        editDateEquipe->setText(QDate::currentDate().toString("dd/MM/yyyy"));
        lblMembresAjout->setText("👥  Membres : 0 / —");
        lblStatutAjout->setText("●  Statut : Incomplète");
        comboNouvelleCompetition->setCurrentIndex(-1);
    });

    QFrame *separator1 = new QFrame(contenu);
    separator1->setFrameShape(QFrame::HLine);
    separator1->setStyleSheet("color:#E2E8F0;");
    layout->addWidget(separator1);

    QLabel *titreStats = new QLabel("📊  Statistiques de performance", contenu);
    titreStats->setStyleSheet("font-size:15px;font-weight:700;color:#1E293B;");
    layout->addWidget(titreStats);

    QGridLayout *statsGrid = new QGridLayout;
    statsGrid->setHorizontalSpacing(6);
    statsGrid->setVerticalSpacing(4);
    auto makeStat = [contenu](const QString &caption, const QString &color, QLabel **value) {
        QFrame *card = new QFrame(contenu);
        card->setStyleSheet("QFrame{background:#F8FAFC;border:1px solid #E2E8F0;border-radius:6px;}");
        QVBoxLayout *v = new QVBoxLayout(card);
        v->setContentsMargins(7,5,7,5);
        QLabel *t = new QLabel(caption, card);
        t->setStyleSheet("color:#64748B;font-size:10px;border:0;");
        *value = new QLabel("0", card);
        (*value)->setStyleSheet(QString("color:%1;font-size:19px;font-weight:700;border:0;").arg(color));
        v->addWidget(t); v->addWidget(*value);
        return card;
    };
    QLabel *vTotal=nullptr, *vActives=nullptr, *vCours=nullptr, *vIncompletes=nullptr;
    statsGrid->addWidget(makeStat("Total équipes", "#2563EB", &vTotal),0,0);
    statsGrid->addWidget(makeStat("Actives", "#10B981", &vActives),0,1);
    statsGrid->addWidget(makeStat("En cours", "#F59E0B", &vCours),1,0);
    statsGrid->addWidget(makeStat("Incomplètes", "#EF4444", &vIncompletes),1,1);
    layout->addLayout(statsGrid);

    lblStatTotal = vTotal;
    lblStatActives = vActives;
    lblStatCours = vCours;
    lblStatIncompletes = vIncompletes;

    QLabel *courbeTitre = new QLabel("📈  Évolution des inscriptions", contenu);
    courbeTitre->setStyleSheet("font-weight:600;color:#334155;");
    layout->addWidget(courbeTitre);
    layout->addWidget(new CourbeChartWidget(contenu));
    QLabel *barTitre = new QLabel("📊  État des équipes", contenu);
    barTitre->setStyleSheet("font-weight:600;color:#334155;");
    layout->addWidget(barTitre);
    layout->addWidget(new BarChartWidget(0, 0, contenu));

    QFrame *separator2 = new QFrame(contenu);
    separator2->setFrameShape(QFrame::HLine);
    separator2->setStyleSheet("color:#E2E8F0;");
    layout->addWidget(separator2);
    QLabel *titreDetails = new QLabel("🔎  Détails de l'équipe sélectionnée", contenu);
    titreDetails->setStyleSheet("font-size:14px;font-weight:700;color:#1E293B;");
    layout->addWidget(titreDetails);

    auto addDetail = [contenu, layout](const QString &icon, const QString &initial, QLabel **target) {
        QHBoxLayout *line = new QHBoxLayout;
        QLabel *ico = new QLabel(icon, contenu);
        ico->setFixedWidth(23);
        ico->setStyleSheet("font-size:15px;");
        *target = new QLabel(initial, contenu);
        (*target)->setWordWrap(true);
        (*target)->setStyleSheet("color:#475569;padding:2px;");
        line->addWidget(ico); line->addWidget(*target,1);
        layout->addLayout(line);
    };
    addDetail("🏷️", "Nom : —", &lblDetailNom);
    addDetail("🏆", "Compétition : —", &lblDetailCompetition);
    addDetail("👑", "Chef : —", &lblDetailChef);
    addDetail("👥", "Membres : —", &lblDetailMembres);
    addDetail("📅", "Date : —", &lblDetailDate);
    addDetail("●", "Statut : —", &lblDetailStatut);

    layout->addStretch(1);
    scroll->setWidget(contenu);
    outer->addWidget(scroll);

    QSqlDatabase db = QSqlDatabase::database("hackathon_connection");
    if (db.isValid() && db.isOpen()) {
        QSqlQuery q(db);
        if (q.exec("SELECT ID_COMPETITION, NOM FROM COMPETITION ORDER BY NOM"))
            while (q.next()) comboNouvelleCompetition->addItem(q.value(1).toString(), q.value(0).toInt());
    }
    actualiserStatistiques();
}

void MainWindow::actualiserStatistiques()
{
    if (!lblStatTotal) return;
    int actives = 0, enCours = 0, incompletes = 0;
    for (const Equipe &e : toutesLesEquipes) {
        if (e.statut == "Active") ++actives;
        else if (e.statut == "En cours") ++enCours;
        else ++incompletes;
    }
    lblStatTotal->setText(QString::number(toutesLesEquipes.size()));
    lblStatActives->setText(QString::number(actives));
    lblStatCours->setText(QString::number(enCours));
    lblStatIncompletes->setText(QString::number(incompletes));
    if (QWidget *chartWidget =
        panneauDroit->findChild<QWidget*>("barChartEquipes")) {
        auto *chart = static_cast<BarChartWidget*>(chartWidget);
        chart->setData(actives, incompletes);
    }

    if (QWidget *courbeWidget =
        panneauDroit->findChild<QWidget*>("courbeChartEquipes")) {
        auto *courbe = static_cast<CourbeChartWidget*>(courbeWidget);

        QVector<int> values(6, 0);
        QStringList labels;

        QDate first = QDate::currentDate().addMonths(-5);

        for (int i = 0; i < 6; ++i)
            labels << first.addMonths(i).toString("MMM");

        for (const Equipe &e : toutesLesEquipes) {
            QDate d = QDate::fromString(e.date, "dd/MM/yyyy");
            if (!d.isValid())
                d = QDate::fromString(e.date, "yyyy-MM-dd");

            if (!d.isValid())
                continue;

            int offset = (d.year() - first.year()) * 12
                         + d.month() - first.month();

            if (offset >= 0 && offset < 6)
                ++values[offset];
        }

        courbe->setData(values, labels);
    }
}

void MainWindow::afficherDetailsEquipe(const Equipe &e)
{
    if (!lblDetailNom) return;
    lblDetailNom->setText("Nom : " + e.nom);
    lblDetailCompetition->setText("Compétition : " + e.competition);
    lblDetailChef->setText("Chef : " + e.chef);
    lblDetailMembres->setText("Membres : " + e.membres + (e.nomsMembres.isEmpty() ? "" : " (" + e.nomsMembres.join(", ") + ")"));
    lblDetailDate->setText("Date : " + e.date);
    lblDetailStatut->setText("Statut : " + e.statut);
}

void MainWindow::chargerPage(int page) {
    const int itemsPerPage = 8;
    const int totalPages = qMax(1, (toutesLesEquipes.size() + itemsPerPage - 1) / itemsPerPage);
    if (page < 1 || page > totalPages) return;
    currentPage = page;
    actualiserStatistiques();

    const int startIndex = (currentPage - 1) * itemsPerPage;
    const int endIndex = qMin(startIndex + itemsPerPage, toutesLesEquipes.size());
    const int currentCount = qMax(0, endIndex - startIndex);

    ui->tableWidget->setRowCount(currentCount);
    equipesAffichees.clear();

    for (int i = 0; i < currentCount; ++i) {
        Equipe eq = toutesLesEquipes[startIndex + i];
        equipesAffichees.append(eq);

        QLabel *lblNom = new QLabel(eq.nom);
        lblNom->setStyleSheet("color: #1E293B; font-weight: bold; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 0, lblNom);

        QLabel *lblComp = new QLabel(eq.competition);
        lblComp->setStyleSheet("color: #475569; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 1, lblComp);

        QLabel *lblChef = new QLabel(eq.chef);
        lblChef->setStyleSheet("color: #475569; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 2, lblChef);

        QWidget *membresWidget = new QWidget();
        QHBoxLayout *membresLayout = new QHBoxLayout(membresWidget);
        QLabel *lblMembres = new QLabel(eq.membres);
        lblMembres->setToolTip(eq.nomsMembres.isEmpty() ? "Aucun membre" : eq.nomsMembres.join("\n"));
        lblMembres->setStyleSheet("color: #475569; font-weight: bold; min-width: 35px;");

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, qMax(1, eq.maxMembres));
        progressBar->setValue(eq.membres.section('/', 0, 0).toInt());
        progressBar->setTextVisible(false);
        progressBar->setStyleSheet("QProgressBar { border: none; background: #F1F5F9; border-radius: 3px; max-height: 8px; } QProgressBar::chunk { background-color: #2563EB; border-radius: 3px; }");

        membresLayout->addWidget(lblMembres);
        membresLayout->addWidget(progressBar);
        membresLayout->setContentsMargins(2, 0, 5, 0);
        ui->tableWidget->setCellWidget(i, 3, membresWidget);

        QLabel *lblDate = new QLabel(eq.date);
        lblDate->setStyleSheet("color: #475569; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 4, lblDate);

        QLabel *lblStatut = new QLabel(eq.statut);
        if (eq.statut == "Active") {
            lblStatut->setStyleSheet("color: #10B981; font-weight: bold; padding-left: 5px;");
        } else if (eq.statut == "En cours") {
            lblStatut->setStyleSheet("color: #2563EB; font-weight: bold; padding-left: 5px;");
        } else {
            lblStatut->setStyleSheet("color: #F59E0B; font-weight: bold; padding-left: 5px;");
        }
        ui->tableWidget->setCellWidget(i, 5, lblStatut);

        QPushButton *btnEdit = new QPushButton("✏️", this);
        btnEdit->setToolTip("Modifier cette équipe");
        btnEdit->setStyleSheet("background-color: #FEF3C7; color: #D97706; border: none; border-radius: 4px; padding: 4px; font-size: 14px; margin-right: 4px;");

        QPushButton *btnDelete = new QPushButton("🗑️", this);
        btnDelete->setToolTip("Supprimer cette équipe");
        btnDelete->setStyleSheet("background-color: #FEE2E2; color: #EF4444; border: none; border-radius: 4px; padding: 4px; font-size: 14px;");
        QPushButton *btnChef = new QPushButton("👑", this);
        btnChef->setToolTip("Gestion du chef d'équipe");
        btnChef->setStyleSheet(
            "background-color: #FEF3C7; color: #B45309;"
            "border: none; border-radius: 4px;"
            "padding: 4px; font-size: 14px;"
            );
        QWidget *actionsWidget = new QWidget();
        QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->addWidget(btnEdit);
        actionsLayout->addWidget(btnChef);
        actionsLayout->addWidget(btnDelete);
        actionsLayout->setAlignment(Qt::AlignCenter);
        actionsLayout->setContentsMargins(0, 0, 0, 0);
        actionsWidget->setLayout(actionsLayout);

        ui->tableWidget->setCellWidget(i, 6, actionsWidget);
        connect(btnChef, &QPushButton::clicked, this, [this, eq]() {
            ouvrirGestionChef(eq.idEquipe);
        });

        connect(btnEdit, &QPushButton::clicked, this, [=]() {
            QDialog editDialog(this);
            editDialog.setWindowTitle("Modifier l'équipe");
            editDialog.setFixedSize(460, 330);

            QVBoxLayout *mainLayout = new QVBoxLayout(&editDialog);
            mainLayout->setContentsMargins(12, 12, 12, 12);
            mainLayout->setSpacing(8);
            QFormLayout *formLayout = new QFormLayout();
            formLayout->setVerticalSpacing(6);

            QLineEdit *txtNom = new QLineEdit(eq.nom, &editDialog);
            QComboBox *comboCompetition = new QComboBox(&editDialog);
            QLineEdit *txtDate = new QLineEdit(eq.date, &editDialog);
            QComboBox *comboChef = new QComboBox(&editDialog);

            QSqlDatabase db = QSqlDatabase::database("hackathon_connection");

            // Afficher le nom de la compétition, tout en conservant son ID dans currentData().
            QSqlQuery competitionsQuery(db);
            if (!competitionsQuery.exec(
                    "SELECT ID_COMPETITION, NOM FROM COMPETITION ORDER BY NOM")) {
                QMessageBox::critical(this, "Erreur Oracle",
                                      "Impossible de charger les compétitions :\n" +
                                          competitionsQuery.lastError().text());
                return;
            }
            while (competitionsQuery.next()) {
                comboCompetition->addItem(
                    competitionsQuery.value(1).toString(),
                    competitionsQuery.value(0).toInt());
            }
            int indexCompetition = comboCompetition->findData(eq.idCompetition);
            if (indexCompetition >= 0)
                comboCompetition->setCurrentIndex(indexCompetition);


            comboChef->addItem("À définir après inscription", -1);

            QSqlQuery chefsQuery(db);

            chefsQuery.prepare(
                "SELECT ID_PARTICIPANT, NOM, PRENOM "
                "FROM PARTICIPANT "
                "WHERE ID_EQUIPE = ? "
                "ORDER BY NOM, PRENOM"
                );

            chefsQuery.addBindValue(eq.idEquipe);

            if (!chefsQuery.exec()) {
                qDebug() << "Erreur chargement chefs :"
                         << chefsQuery.lastError().text();
            } else {
                qDebug() << "ID Equipe :" << eq.idEquipe;

                while (chefsQuery.next()) {
                    int id = chefsQuery.value(0).toInt();

                    QString nom = chefsQuery.value(2).toString().trimmed()
                                  + " "
                                  + chefsQuery.value(1).toString().trimmed();

                    qDebug() << "Chef trouvé :" << id << nom;

                    comboChef->addItem(nom, id);
                }

                qDebug() << "Nombre de chefs :" << comboChef->count();
            }

            int indexChef = comboChef->findData(eq.idChef);
            comboChef->setCurrentIndex(indexChef >= 0 ? indexChef : 0);
            formLayout->addRow("Nom de l'équipe :", txtNom);
            formLayout->addRow("Compétition :", comboCompetition);
            formLayout->addRow("Date d'inscription (JJ/MM/AAAA) :", txtDate);
            formLayout->addRow("Chef d'équipe :", comboChef);
            // Informations des membres : affichage uniquement
            // Affichage compact du nombre et de la liste des membres (lecture seule).
            QLabel *lblTitreMembres = new QLabel("Nombre de membres :", &editDialog);
            QPushButton *lblMembres = new QPushButton(
                QString("%1 / %2").arg(eq.nomsMembres.size()).arg(eq.maxMembres),
                &editDialog);
            lblMembres->setCursor(Qt::PointingHandCursor);
            lblMembres->setToolTip("Cliquer pour gérer les membres de cette équipe");
            lblMembres->setStyleSheet("QPushButton { color: #2563EB; font-weight: bold; border: 1px solid #BFDBFE; border-radius: 5px; padding: 5px 12px; background: #EFF6FF; } QPushButton:hover { background: #DBEAFE; }");

            QHBoxLayout *membresHeader = new QHBoxLayout();
            membresHeader->setContentsMargins(0, 2, 0, 2);
            membresHeader->setSpacing(8);
            membresHeader->addWidget(lblTitreMembres);
            membresHeader->addWidget(lblMembres);
            membresHeader->addStretch();

            QTextEdit *txtMembres = new QTextEdit(&editDialog);
            txtMembres->setReadOnly(true);
            txtMembres->setFixedHeight(65);
            txtMembres->setPlaceholderText("Aucun membre inscrit.");
            txtMembres->setPlainText(eq.nomsMembres.isEmpty()
                                         ? "Aucun membre inscrit."
                                         : eq.nomsMembres.join("\n"));

            connect(lblMembres, &QPushButton::clicked, &editDialog,
                    [this, eq, lblMembres, txtMembres]() {
                        ouvrirGestionMembres(eq.idEquipe, eq.maxMembres, lblMembres, txtMembres);
                    });

            mainLayout->addLayout(formLayout);
            mainLayout->addLayout(membresHeader);
            mainLayout->addWidget(new QLabel("Liste des membres :", &editDialog));
            mainLayout->addWidget(txtMembres);

            QPushButton *btnEnregistrer = new QPushButton("Enregistrer les modifications", &editDialog);
            btnEnregistrer->setStyleSheet("background-color: #2563EB; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");
            mainLayout->addWidget(btnEnregistrer);

            connect(btnEnregistrer, &QPushButton::clicked, &editDialog, &QDialog::accept);

            if (editDialog.exec() == QDialog::Accepted) {
                const QString nouveauNom = txtNom->text().trimmed();
                const int nouvelIdCompetition = comboCompetition->currentData().toInt();
                const QDate dateInscription = QDate::fromString(txtDate->text().trimmed(), "dd/MM/yyyy");
                if (nouveauNom.isEmpty() || nouvelIdCompetition <= 0 || !dateInscription.isValid()) {
                    QMessageBox::warning(this, "Données invalides",
                                         "Saisissez un nom, choisissez une compétition et vérifiez la date au format JJ/MM/AAAA.");
                    return;
                }

                QSqlDatabase db = QSqlDatabase::database("hackathon_connection");
                QSqlQuery updateQuery(db);
                const int idChefSelectionne = comboChef->currentData().toInt();

                if (idChefSelectionne <= 0) {
                    // Aucun chef sélectionné : NULL est écrit directement dans Oracle.
                    updateQuery.prepare(
                        "UPDATE EQUIPE SET NOM_EQUIPE = ?, "
                        "DATE_INSCRIPTION = TO_DATE(?, 'DD/MM/YYYY'), "
                        "ID_COMPETITION = ?, ID_CHEF = NULL "
                        "WHERE ID_EQUIPE = ?");
                    updateQuery.addBindValue(nouveauNom);
                    updateQuery.addBindValue(dateInscription.toString("dd/MM/yyyy"));
                    updateQuery.addBindValue(nouvelIdCompetition);
                    updateQuery.addBindValue(eq.idEquipe);
                } else {
                    // Un chef est sélectionné : son identifiant est une valeur numérique.
                    updateQuery.prepare(
                        "UPDATE EQUIPE SET NOM_EQUIPE = ?, "
                        "DATE_INSCRIPTION = TO_DATE(?, 'DD/MM/YYYY'), "
                        "ID_COMPETITION = ?, ID_CHEF = ? "
                        "WHERE ID_EQUIPE = ?");
                    updateQuery.addBindValue(nouveauNom);
                    updateQuery.addBindValue(dateInscription.toString("dd/MM/yyyy"));
                    updateQuery.addBindValue(nouvelIdCompetition);
                    updateQuery.addBindValue(idChefSelectionne);
                    updateQuery.addBindValue(eq.idEquipe);
                }

                if (!updateQuery.exec()) {
                    QMessageBox::critical(this, "Erreur Oracle", "Modification impossible :\n" + updateQuery.lastError().text());
                    return;
                }
                if (!chargerEquipesDepuisOracle()) {
                    QMessageBox::warning(this, "Oracle", "La modification est enregistrée, mais le rechargement a échoué.");
                    return;
                }
                filtrerParComboBox();
                QMessageBox::information(this, "Succès", "L'équipe a été modifiée dans Oracle.");
            }
        });

        connect(btnDelete, &QPushButton::clicked, this, [=]() {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer cette équipe ?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                QSqlDatabase db = QSqlDatabase::database("hackathon_connection");
                QSqlQuery countQuery(db);
                countQuery.prepare("SELECT COUNT(*) FROM PARTICIPANT WHERE ID_EQUIPE = ?");
                countQuery.addBindValue(eq.idEquipe);
                if (!countQuery.exec() || !countQuery.next()) {
                    QMessageBox::critical(this, "Erreur Oracle", "Impossible de vérifier les participants liés à cette équipe.\n" + countQuery.lastError().text());
                    return;
                }
                if (countQuery.value(0).toInt() > 0) {
                    QMessageBox::warning(this, "Suppression impossible",
                                         "Cette équipe possède des participants. Supprimez ou réaffectez d'abord ces participants.");
                    return;
                }
                QSqlQuery deleteQuery(db);
                deleteQuery.prepare("DELETE FROM EQUIPE WHERE ID_EQUIPE = ?");
                deleteQuery.addBindValue(eq.idEquipe);
                if (!deleteQuery.exec()) {
                    QMessageBox::critical(this, "Erreur Oracle", "Suppression impossible :\n" + deleteQuery.lastError().text());
                    return;
                }
                if (!chargerEquipesDepuisOracle()) {
                    QMessageBox::warning(this, "Oracle", "La suppression est effectuée, mais le rechargement a échoué.");
                    return;
                }
                filtrerParComboBox();
                QMessageBox::information(this, "Succès", "L'équipe a été supprimée d'Oracle.");
            }
        });
    }
}

void MainWindow::onTeamSelected(int row, int column)
{
    Q_UNUSED(column);
    if (row < 0 || row >= equipesAffichees.size()) return;
    afficherDetailsEquipe(equipesAffichees.at(row));
}


void MainWindow::on_btnAjouter_clicked()
{
    if (!editNomEquipe || !comboNouvelleCompetition || !editDateEquipe) return;
    const QString nom = editNomEquipe->text().trimmed();
    const int idCompetition = comboNouvelleCompetition->currentData().toInt();
    const QDate dateInscription = QDate::fromString(editDateEquipe->text().trimmed(), "dd/MM/yyyy");

    if (nom.isEmpty() || idCompetition <= 0 || !dateInscription.isValid()) {
        QMessageBox::warning(this, "Données invalides",
                             "Vérifiez le nom, la compétition et la date (JJ/MM/AAAA).");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("hackathon_connection");
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::critical(this, "Oracle", "La connexion Oracle est indisponible.");
        return;
    }

    QSqlQuery insertQuery(db);
    insertQuery.prepare(
        "INSERT INTO EQUIPE (ID_EQUIPE, NOM_EQUIPE, DATE_INSCRIPTION, ID_COMPETITION, ID_CHEF) "
        "VALUES ((SELECT NVL(MAX(ID_EQUIPE), 0) + 1 FROM EQUIPE), ?, TO_DATE(?, 'DD/MM/YYYY'), ?, NULL)");
    insertQuery.addBindValue(nom);
    insertQuery.addBindValue(dateInscription.toString("dd/MM/yyyy"));
    insertQuery.addBindValue(idCompetition);

    if (!insertQuery.exec()) {
        QMessageBox::critical(this, "Erreur Oracle", "Ajout impossible :\n" + insertQuery.lastError().text());
        return;
    }
    if (!chargerEquipesDepuisOracle()) {
        QMessageBox::warning(this, "Oracle", "Équipe ajoutée, mais actualisation impossible.");
        return;
    }
    editNomEquipe->clear();
    editDateEquipe->setText(QDate::currentDate().toString("dd/MM/yyyy"));
    chargerPage(1);
    actualiserStatistiques();
    QMessageBox::information(this, "Succès", "L'équipe a été ajoutée dans Oracle.");
}

void MainWindow::filtrerEquipes(const QString &text) {
    if (text.isEmpty()) {
        chargerPage(currentPage);
        return;
    }

    QList<Equipe> equipesFiltrees;
    for (int i = 0; i < toutesLesEquipes.size(); ++i) {
        const Equipe &eq = toutesLesEquipes.at(i);
        bool membreTrouve = false;
        for (const QString &nomMembre : eq.nomsMembres) {
            if (nomMembre.contains(text, Qt::CaseInsensitive)) {
                membreTrouve = true;
                break;
            }
        }
        if (eq.nom.contains(text, Qt::CaseInsensitive) ||
            eq.chef.contains(text, Qt::CaseInsensitive) ||
            eq.competition.contains(text, Qt::CaseInsensitive) ||
            membreTrouve) {
            equipesFiltrees.append(eq);
        }
    }

    ui->tableWidget->setRowCount(equipesFiltrees.size());
    equipesAffichees = equipesFiltrees;

    for (int i = 0; i < equipesFiltrees.size(); ++i) {
        Equipe eq = equipesFiltrees[i];

        QLabel *lblNom = new QLabel(eq.nom);
        lblNom->setStyleSheet("color: #1E293B; font-weight: bold; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 0, lblNom);

        QLabel *lblComp = new QLabel(eq.competition);
        lblComp->setStyleSheet("color: #475569; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 1, lblComp);

        QLabel *lblChef = new QLabel(eq.chef);
        lblChef->setStyleSheet("color: #475569; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 2, lblChef);

        QWidget *membresWidget = new QWidget();
        QHBoxLayout *membresLayout = new QHBoxLayout(membresWidget);
        QLabel *lblMembres = new QLabel(eq.membres);
        lblMembres->setToolTip(eq.nomsMembres.isEmpty() ? "Aucun membre" : eq.nomsMembres.join("\n"));
        lblMembres->setStyleSheet("color: #475569; font-weight: bold; min-width: 35px;");

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, qMax(1, eq.maxMembres));
        progressBar->setValue(eq.membres.section('/', 0, 0).toInt());
        progressBar->setTextVisible(false);
        progressBar->setStyleSheet("QProgressBar { border: none; background: #F1F5F9; border-radius: 3px; max-height: 8px; } QProgressBar::chunk { background-color: #2563EB; border-radius: 3px; }");

        membresLayout->addWidget(lblMembres);
        membresLayout->addWidget(progressBar);
        membresLayout->setContentsMargins(2, 0, 5, 0);
        ui->tableWidget->setCellWidget(i, 3, membresWidget);

        QLabel *lblDate = new QLabel(eq.date);
        lblDate->setStyleSheet("color: #475569; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 4, lblDate);

        QLabel *lblStatut = new QLabel(eq.statut);
        if (eq.statut == "Active") {
            lblStatut->setStyleSheet("color: #10B981; font-weight: bold; padding-left: 5px;");
        } else if (eq.statut == "En cours") {
            lblStatut->setStyleSheet("color: #2563EB; font-weight: bold; padding-left: 5px;");
        } else {
            lblStatut->setStyleSheet("color: #F59E0B; font-weight: bold; padding-left: 5px;");
        }
        ui->tableWidget->setCellWidget(i, 5, lblStatut);

        QPushButton *btnEdit = new QPushButton("✏️", this);
        btnEdit->setToolTip("Modifier cette équipe");
        btnEdit->setStyleSheet("background-color: #FEF3C7; color: #D97706; border: none; border-radius: 4px; padding: 4px; font-size: 14px; margin-right: 4px;");

        QPushButton *btnDelete = new QPushButton("🗑️", this);
        btnDelete->setToolTip("Supprimer cette équipe");
        btnDelete->setStyleSheet("background-color: #FEE2E2; color: #EF4444; border: none; border-radius: 4px; padding: 4px; font-size: 14px;");
        QPushButton *btnChef = new QPushButton("👑", this);
        btnChef->setToolTip("Gestion du chef d'équipe");
        btnChef->setStyleSheet(
            "background-color: #FEF3C7; color: #B45309;"
            "border: none; border-radius: 4px;"
            "padding: 4px; font-size: 14px;"
            );
        QWidget *actionsWidget = new QWidget();
        QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->addWidget(btnEdit);
        actionsLayout->addWidget(btnChef);
        actionsLayout->addWidget(btnDelete);
        actionsLayout->setAlignment(Qt::AlignCenter);
        actionsLayout->setContentsMargins(0, 0, 0, 0);
        actionsWidget->setLayout(actionsLayout);

        ui->tableWidget->setCellWidget(i, 6, actionsWidget);
        connect(btnChef, &QPushButton::clicked, this, [this, eq]() {
            ouvrirGestionChef(eq.idEquipe);
        });

        connect(btnEdit, &QPushButton::clicked, this, [=]() {
            QDialog editDialog(this);
            editDialog.setWindowTitle("Modifier l'équipe");
            editDialog.setFixedSize(460, 330);

            QVBoxLayout *mainLayout = new QVBoxLayout(&editDialog);
            mainLayout->setContentsMargins(12, 12, 12, 12);
            mainLayout->setSpacing(8);
            QFormLayout *formLayout = new QFormLayout();
            formLayout->setVerticalSpacing(6);

            QLineEdit *txtNom = new QLineEdit(eq.nom, &editDialog);
            QComboBox *comboCompetition = new QComboBox(&editDialog);
            QLineEdit *txtDate = new QLineEdit(eq.date, &editDialog);
            QComboBox *comboChef = new QComboBox(&editDialog);

            QSqlDatabase db = QSqlDatabase::database("hackathon_connection");

            // Afficher le nom de la compétition, tout en conservant son ID dans currentData().
            QSqlQuery competitionsQuery(db);
            if (!competitionsQuery.exec(
                    "SELECT ID_COMPETITION, NOM FROM COMPETITION ORDER BY NOM")) {
                QMessageBox::critical(this, "Erreur Oracle",
                                      "Impossible de charger les compétitions :\n" +
                                          competitionsQuery.lastError().text());
                return;
            }
            while (competitionsQuery.next()) {
                comboCompetition->addItem(
                    competitionsQuery.value(1).toString(),
                    competitionsQuery.value(0).toInt());
            }
            int indexCompetition = comboCompetition->findData(eq.idCompetition);
            if (indexCompetition >= 0)
                comboCompetition->setCurrentIndex(indexCompetition);

            comboChef->addItem("À définir après inscription", -1);
            QSqlQuery chefsQuery(db);

            chefsQuery.prepare(
                "SELECT ID_PARTICIPANT, NOM, PRENOM "
                "FROM PARTICIPANT WHERE ID_EQUIPE = ? "
                "ORDER BY NOM, PRENOM");

            chefsQuery.addBindValue(eq.idEquipe);

            if (chefsQuery.exec()) {
                while (chefsQuery.next()) {
                    int id = chefsQuery.value(0).toInt();
                    QString nom = chefsQuery.value(2).toString()
                                  + " " + chefsQuery.value(1).toString();

                    comboChef->addItem(nom, id);
                }
            }

            int indexChef = comboChef->findData(eq.idChef);
            comboChef->setCurrentIndex(indexChef >= 0 ? indexChef : 0);
            formLayout->addRow("Nom de l'équipe :", txtNom);
            formLayout->addRow("Compétition :", comboCompetition);
            formLayout->addRow("Date d'inscription (JJ/MM/AAAA) :", txtDate);
            formLayout->addRow("Chef d'équipe :", comboChef);

            QLabel *lblTitreMembres = new QLabel("Nombre de membres :", &editDialog);
            QPushButton *lblMembres = new QPushButton(
                QString("%1 / %2").arg(eq.nomsMembres.size()).arg(eq.maxMembres),
                &editDialog);
            lblMembres->setCursor(Qt::PointingHandCursor);
            lblMembres->setToolTip("Cliquer pour gérer les membres de cette équipe");
            lblMembres->setStyleSheet("QPushButton { color: #2563EB; font-weight: bold; border: 1px solid #BFDBFE; border-radius: 5px; padding: 5px 12px; background: #EFF6FF; } QPushButton:hover { background: #DBEAFE; }");

            QHBoxLayout *membresHeader = new QHBoxLayout();
            membresHeader->setContentsMargins(0, 2, 0, 2);
            membresHeader->setSpacing(8);
            membresHeader->addWidget(lblTitreMembres);
            membresHeader->addWidget(lblMembres);
            membresHeader->addStretch();

            QTextEdit *txtMembres = new QTextEdit(&editDialog);
            txtMembres->setReadOnly(true);
            txtMembres->setFixedHeight(65);
            txtMembres->setPlainText(eq.nomsMembres.isEmpty()
                                         ? "Aucun membre inscrit."
                                         : eq.nomsMembres.join("\n"));

            connect(lblMembres, &QPushButton::clicked, &editDialog,
                    [this, eq, lblMembres, txtMembres]() {
                        ouvrirGestionMembres(eq.idEquipe, eq.maxMembres, lblMembres, txtMembres);
                    });

            mainLayout->addLayout(formLayout);
            mainLayout->addLayout(membresHeader);
            mainLayout->addWidget(new QLabel("Liste des membres :", &editDialog));
            mainLayout->addWidget(txtMembres);

            QPushButton *btnEnregistrer = new QPushButton("Enregistrer les modifications", &editDialog);
            btnEnregistrer->setStyleSheet("background-color: #2563EB; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");
            mainLayout->addWidget(btnEnregistrer);

            connect(btnEnregistrer, &QPushButton::clicked, &editDialog, &QDialog::accept);

            if (editDialog.exec() == QDialog::Accepted) {
                const QString nouveauNom = txtNom->text().trimmed();
                const int nouvelIdCompetition = comboCompetition->currentData().toInt();
                const QDate dateInscription = QDate::fromString(txtDate->text().trimmed(), "dd/MM/yyyy");
                if (nouveauNom.isEmpty() || nouvelIdCompetition <= 0 || !dateInscription.isValid()) {
                    QMessageBox::warning(this, "Données invalides",
                                         "Saisissez un nom, choisissez une compétition et vérifiez la date au format JJ/MM/AAAA.");
                    return;
                }

                QSqlDatabase db = QSqlDatabase::database("hackathon_connection");
                QSqlQuery updateQuery(db);
                const int idChefSelectionne = comboChef->currentData().toInt();

                if (idChefSelectionne <= 0) {
                    // Aucun chef sélectionné : NULL est écrit directement dans Oracle.
                    updateQuery.prepare(
                        "UPDATE EQUIPE SET NOM_EQUIPE = ?, "
                        "DATE_INSCRIPTION = TO_DATE(?, 'DD/MM/YYYY'), "
                        "ID_COMPETITION = ?, ID_CHEF = NULL "
                        "WHERE ID_EQUIPE = ?");
                    updateQuery.addBindValue(nouveauNom);
                    updateQuery.addBindValue(dateInscription.toString("dd/MM/yyyy"));
                    updateQuery.addBindValue(nouvelIdCompetition);
                    updateQuery.addBindValue(eq.idEquipe);
                } else {
                    // Un chef est sélectionné : son identifiant est une valeur numérique.
                    updateQuery.prepare(
                        "UPDATE EQUIPE SET NOM_EQUIPE = ?, "
                        "DATE_INSCRIPTION = TO_DATE(?, 'DD/MM/YYYY'), "
                        "ID_COMPETITION = ?, ID_CHEF = ? "
                        "WHERE ID_EQUIPE = ?");
                    updateQuery.addBindValue(nouveauNom);
                    updateQuery.addBindValue(dateInscription.toString("dd/MM/yyyy"));
                    updateQuery.addBindValue(nouvelIdCompetition);
                    updateQuery.addBindValue(idChefSelectionne);
                    updateQuery.addBindValue(eq.idEquipe);
                }

                if (!updateQuery.exec()) {
                    QMessageBox::critical(this, "Erreur Oracle", "Modification impossible :\n" + updateQuery.lastError().text());
                    return;
                }
                if (!chargerEquipesDepuisOracle()) {
                    QMessageBox::warning(this, "Oracle", "La modification est enregistrée, mais le rechargement a échoué.");
                    return;
                }
                filtrerParComboBox();
                QMessageBox::information(this, "Succès", "L'équipe a été modifiée dans Oracle.");
            }
        });

        connect(btnDelete, &QPushButton::clicked, this, [=]() {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer cette équipe ?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                QSqlDatabase db = QSqlDatabase::database("hackathon_connection");
                QSqlQuery countQuery(db);
                countQuery.prepare("SELECT COUNT(*) FROM PARTICIPANT WHERE ID_EQUIPE = ?");
                countQuery.addBindValue(eq.idEquipe);
                if (!countQuery.exec() || !countQuery.next()) {
                    QMessageBox::critical(this, "Erreur Oracle", "Impossible de vérifier les participants liés à cette équipe.\n" + countQuery.lastError().text());
                    return;
                }
                if (countQuery.value(0).toInt() > 0) {
                    QMessageBox::warning(this, "Suppression impossible",
                                         "Cette équipe possède des participants. Supprimez ou réaffectez d'abord ces participants.");
                    return;
                }
                QSqlQuery deleteQuery(db);
                deleteQuery.prepare("DELETE FROM EQUIPE WHERE ID_EQUIPE = ?");
                deleteQuery.addBindValue(eq.idEquipe);
                if (!deleteQuery.exec()) {
                    QMessageBox::critical(this, "Erreur Oracle", "Suppression impossible :\n" + deleteQuery.lastError().text());
                    return;
                }
                if (!chargerEquipesDepuisOracle()) {
                    QMessageBox::warning(this, "Oracle", "La suppression est effectuée, mais le rechargement a échoué.");
                    return;
                }
                filtrerParComboBox();
                QMessageBox::information(this, "Succès", "L'équipe a été supprimée d'Oracle.");
            }
        });
    }
}

void MainWindow::filtrerParComboBox() {
    QString compFiltre = ui->comboBoxCompetition->currentText();
    QString statutFiltre = ui->comboBoxStatus->currentText();
    QString triFiltre = ui->comboBoxTrier->currentText();

    QList<Equipe> equipesFiltrees;
    for (int i = 0; i < toutesLesEquipes.size(); ++i) {
        const Equipe &eq = toutesLesEquipes.at(i);
        bool matchComp = (compFiltre == "Toutes" || eq.competition == compFiltre);
        bool matchStatut = (statutFiltre == "Tous" || eq.statut.toLower() == statutFiltre.toLower() || (statutFiltre == "Incomplète" && eq.statut == "Incomplète"));

        if (matchComp && matchStatut) {
            equipesFiltrees.append(eq);
        }
    }

    if (triFiltre == "Nom") {
        std::sort(equipesFiltrees.begin(), equipesFiltrees.end(), [](const Equipe &a, const Equipe &b) {
            return a.nom < b.nom;
        });
    } else if (triFiltre == "Membres") {
        std::sort(equipesFiltrees.begin(), equipesFiltrees.end(), [](const Equipe &a, const Equipe &b) {
            return a.membres.section('/', 0, 0).toInt() > b.membres.section('/', 0, 0).toInt();
        });
    } else if (triFiltre == "Date d'inscription") {
        std::sort(equipesFiltrees.begin(), equipesFiltrees.end(), [](const Equipe &a, const Equipe &b) {
            QDate dateA = QDate::fromString(a.date, "dd/MM/yyyy");
            QDate dateB = QDate::fromString(b.date, "dd/MM/yyyy");

            return dateA < dateB;
        });
    }

    ui->tableWidget->setRowCount(equipesFiltrees.size());
    equipesAffichees = equipesFiltrees;

    for (int i = 0; i < equipesFiltrees.size(); ++i) {
        Equipe eq = equipesFiltrees[i];

        QLabel *lblNom = new QLabel(eq.nom);
        lblNom->setStyleSheet("color: #1E293B; font-weight: bold; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 0, lblNom);

        QLabel *lblComp = new QLabel(eq.competition);
        lblComp->setStyleSheet("color: #475569; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 1, lblComp);

        QLabel *lblChef = new QLabel(eq.chef);
        lblChef->setStyleSheet("color: #475569; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 2, lblChef);

        QWidget *membresWidget = new QWidget();
        QHBoxLayout *membresLayout = new QHBoxLayout(membresWidget);
        QLabel *lblMembres = new QLabel(eq.membres);
        lblMembres->setToolTip(eq.nomsMembres.isEmpty() ? "Aucun membre" : eq.nomsMembres.join("\n"));
        lblMembres->setStyleSheet("color: #475569; font-weight: bold; min-width: 35px;");

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, qMax(1, eq.maxMembres));
        progressBar->setValue(eq.membres.section('/', 0, 0).toInt());
        progressBar->setTextVisible(false);
        progressBar->setStyleSheet("QProgressBar { border: none; background: #F1F5F9; border-radius: 3px; max-height: 8px; } QProgressBar::chunk { background-color: #2563EB; border-radius: 3px; }");

        membresLayout->addWidget(lblMembres);
        membresLayout->addWidget(progressBar);
        membresLayout->setContentsMargins(2, 0, 5, 0);
        ui->tableWidget->setCellWidget(i, 3, membresWidget);

        QLabel *lblDate = new QLabel(eq.date);
        lblDate->setStyleSheet("color: #475569; padding-left: 5px;");
        ui->tableWidget->setCellWidget(i, 4, lblDate);

        QLabel *lblStatut = new QLabel(eq.statut);
        if (eq.statut == "Active") {
            lblStatut->setStyleSheet("color: #10B981; font-weight: bold; padding-left: 5px;");
        } else if (eq.statut == "En cours") {
            lblStatut->setStyleSheet("color: #2563EB; font-weight: bold; padding-left: 5px;");
        } else {
            lblStatut->setStyleSheet("color: #F59E0B; font-weight: bold; padding-left: 5px;");
        }
        ui->tableWidget->setCellWidget(i, 5, lblStatut);

        QPushButton *btnEdit = new QPushButton("✏️", this);
        btnEdit->setToolTip("Modifier cette équipe");
        btnEdit->setStyleSheet("background-color: #FEF3C7; color: #D97706; border: none; border-radius: 4px; padding: 4px; font-size: 14px; margin-right: 4px;");

        QPushButton *btnDelete = new QPushButton("🗑️", this);
        btnDelete->setToolTip("Supprimer cette équipe");
        btnDelete->setStyleSheet("background-color: #FEE2E2; color: #EF4444; border: none; border-radius: 4px; padding: 4px; font-size: 14px;");
        QPushButton *btnChef = new QPushButton("👑", this);
        btnChef->setToolTip("Gestion du chef d'équipe");
        btnChef->setStyleSheet(
            "background-color: #FEF3C7; color: #B45309;"
            "border: none; border-radius: 4px;"
            "padding: 4px; font-size: 14px;"
            );
        QWidget *actionsWidget = new QWidget();
        QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->addWidget(btnEdit);
        actionsLayout->addWidget(btnChef);
        actionsLayout->addWidget(btnDelete);
        actionsLayout->setAlignment(Qt::AlignCenter);
        actionsLayout->setContentsMargins(0, 0, 0, 0);
        actionsWidget->setLayout(actionsLayout);

        ui->tableWidget->setCellWidget(i, 6, actionsWidget);
        connect(btnChef, &QPushButton::clicked, this, [this, eq]() {
            ouvrirGestionChef(eq.idEquipe);
        });

        connect(btnEdit, &QPushButton::clicked, this, [=]() {
            QDialog editDialog(this);
            editDialog.setWindowTitle("Modifier l'équipe");
            editDialog.setFixedSize(460, 330);

            QVBoxLayout *mainLayout = new QVBoxLayout(&editDialog);
            mainLayout->setContentsMargins(12, 12, 12, 12);
            mainLayout->setSpacing(8);
            QFormLayout *formLayout = new QFormLayout();
            formLayout->setVerticalSpacing(6);

            QLineEdit *txtNom = new QLineEdit(eq.nom, &editDialog);
            QComboBox *comboCompetition = new QComboBox(&editDialog);
            QLineEdit *txtDate = new QLineEdit(eq.date, &editDialog);
            QComboBox *comboChef = new QComboBox(&editDialog);

            QSqlDatabase db = QSqlDatabase::database("hackathon_connection");

            // Afficher le nom de la compétition, tout en conservant son ID dans currentData().
            QSqlQuery competitionsQuery(db);
            if (!competitionsQuery.exec(
                    "SELECT ID_COMPETITION, NOM FROM COMPETITION ORDER BY NOM")) {
                QMessageBox::critical(this, "Erreur Oracle",
                                      "Impossible de charger les compétitions :\n" +
                                          competitionsQuery.lastError().text());
                return;
            }
            while (competitionsQuery.next()) {
                comboCompetition->addItem(
                    competitionsQuery.value(1).toString(),
                    competitionsQuery.value(0).toInt());
            }
            int indexCompetition = comboCompetition->findData(eq.idCompetition);
            if (indexCompetition >= 0)
                comboCompetition->setCurrentIndex(indexCompetition);

            comboChef->addItem("À définir après inscription", -1);
            QSqlQuery chefsQuery(db);

            chefsQuery.prepare(
                "SELECT ID_PARTICIPANT, NOM, PRENOM "
                "FROM PARTICIPANT WHERE ID_EQUIPE = ? "
                "ORDER BY NOM, PRENOM");

            chefsQuery.addBindValue(eq.idEquipe);

            if (chefsQuery.exec()) {
                while (chefsQuery.next()) {
                    int id = chefsQuery.value(0).toInt();
                    QString nom = chefsQuery.value(2).toString()
                                  + " " + chefsQuery.value(1).toString();

                    comboChef->addItem(nom, id);
                }
            }
            comboChef->setMaxVisibleItems(10);

            int indexChef = comboChef->findData(eq.idChef);
            comboChef->setCurrentIndex(indexChef >= 0 ? indexChef : 0);
            formLayout->addRow("Nom de l'équipe :", txtNom);
            formLayout->addRow("Compétition :", comboCompetition);
            formLayout->addRow("Date d'inscription (JJ/MM/AAAA) :", txtDate);
            formLayout->addRow("Chef d'équipe :", comboChef);

            QLabel *lblTitreMembres = new QLabel("Nombre de membres :", &editDialog);
            QPushButton *lblMembres = new QPushButton(
                QString("%1 / %2").arg(eq.nomsMembres.size()).arg(eq.maxMembres),
                &editDialog);
            lblMembres->setCursor(Qt::PointingHandCursor);
            lblMembres->setToolTip("Cliquer pour gérer les membres de cette équipe");
            lblMembres->setStyleSheet("QPushButton { color: #2563EB; font-weight: bold; border: 1px solid #BFDBFE; border-radius: 5px; padding: 5px 12px; background: #EFF6FF; } QPushButton:hover { background: #DBEAFE; }");

            QHBoxLayout *membresHeader = new QHBoxLayout();
            membresHeader->setContentsMargins(0, 2, 0, 2);
            membresHeader->setSpacing(8);
            membresHeader->addWidget(lblTitreMembres);
            membresHeader->addWidget(lblMembres);
            membresHeader->addStretch();

            QTextEdit *txtMembres = new QTextEdit(&editDialog);
            txtMembres->setReadOnly(true);
            txtMembres->setFixedHeight(65);
            txtMembres->setPlainText(eq.nomsMembres.isEmpty()
                                         ? "Aucun membre inscrit."
                                         : eq.nomsMembres.join("\n"));

            connect(lblMembres, &QPushButton::clicked, &editDialog,
                    [this, eq, lblMembres, txtMembres]() {
                        ouvrirGestionMembres(eq.idEquipe, eq.maxMembres, lblMembres, txtMembres);
                    });

            mainLayout->addLayout(formLayout);
            mainLayout->addLayout(membresHeader);
            mainLayout->addWidget(new QLabel("Liste des membres :", &editDialog));
            mainLayout->addWidget(txtMembres);

            QPushButton *btnEnregistrer = new QPushButton("Enregistrer les modifications", &editDialog);
            btnEnregistrer->setStyleSheet("background-color: #2563EB; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");
            mainLayout->addWidget(btnEnregistrer);

            connect(btnEnregistrer, &QPushButton::clicked, &editDialog, &QDialog::accept);

            if (editDialog.exec() == QDialog::Accepted) {
                const QString nouveauNom = txtNom->text().trimmed();
                const int nouvelIdCompetition = comboCompetition->currentData().toInt();
                const QDate dateInscription = QDate::fromString(txtDate->text().trimmed(), "dd/MM/yyyy");
                if (nouveauNom.isEmpty() || nouvelIdCompetition <= 0 || !dateInscription.isValid()) {
                    QMessageBox::warning(this, "Données invalides",
                                         "Saisissez un nom, choisissez une compétition et vérifiez la date au format JJ/MM/AAAA.");
                    return;
                }

                QSqlDatabase db = QSqlDatabase::database("hackathon_connection");
                QSqlQuery updateQuery(db);
                const int idChefSelectionne = comboChef->currentData().toInt();

                if (idChefSelectionne <= 0) {
                    // Aucun chef sélectionné : NULL est écrit directement dans Oracle.
                    updateQuery.prepare(
                        "UPDATE EQUIPE SET NOM_EQUIPE = ?, "
                        "DATE_INSCRIPTION = TO_DATE(?, 'DD/MM/YYYY'), "
                        "ID_COMPETITION = ?, ID_CHEF = NULL "
                        "WHERE ID_EQUIPE = ?");
                    updateQuery.addBindValue(nouveauNom);
                    updateQuery.addBindValue(dateInscription.toString("dd/MM/yyyy"));
                    updateQuery.addBindValue(nouvelIdCompetition);
                    updateQuery.addBindValue(eq.idEquipe);
                } else {
                    // Un chef est sélectionné : son identifiant est une valeur numérique.
                    updateQuery.prepare(
                        "UPDATE EQUIPE SET NOM_EQUIPE = ?, "
                        "DATE_INSCRIPTION = TO_DATE(?, 'DD/MM/YYYY'), "
                        "ID_COMPETITION = ?, ID_CHEF = ? "
                        "WHERE ID_EQUIPE = ?");
                    updateQuery.addBindValue(nouveauNom);
                    updateQuery.addBindValue(dateInscription.toString("dd/MM/yyyy"));
                    updateQuery.addBindValue(nouvelIdCompetition);
                    updateQuery.addBindValue(idChefSelectionne);
                    updateQuery.addBindValue(eq.idEquipe);
                }

                if (!updateQuery.exec()) {
                    QMessageBox::critical(this, "Erreur Oracle", "Modification impossible :\n" + updateQuery.lastError().text());
                    return;
                }
                if (!chargerEquipesDepuisOracle()) {
                    QMessageBox::warning(this, "Oracle", "La modification est enregistrée, mais le rechargement a échoué.");
                    return;
                }
                filtrerParComboBox();
                QMessageBox::information(this, "Succès", "L'équipe a été modifiée dans Oracle.");
            }
        });

        connect(btnDelete, &QPushButton::clicked, this, [=]() {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer cette équipe ?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                QSqlDatabase db = QSqlDatabase::database("hackathon_connection");
                QSqlQuery countQuery(db);
                countQuery.prepare("SELECT COUNT(*) FROM PARTICIPANT WHERE ID_EQUIPE = ?");
                countQuery.addBindValue(eq.idEquipe);
                if (!countQuery.exec() || !countQuery.next()) {
                    QMessageBox::critical(this, "Erreur Oracle", "Impossible de vérifier les participants liés à cette équipe.\n" + countQuery.lastError().text());
                    return;
                }
                if (countQuery.value(0).toInt() > 0) {
                    QMessageBox::warning(this, "Suppression impossible",
                                         "Cette équipe possède des participants. Supprimez ou réaffectez d'abord ces participants.");
                    return;
                }
                QSqlQuery deleteQuery(db);
                deleteQuery.prepare("DELETE FROM EQUIPE WHERE ID_EQUIPE = ?");
                deleteQuery.addBindValue(eq.idEquipe);
                if (!deleteQuery.exec()) {
                    QMessageBox::critical(this, "Erreur Oracle", "Suppression impossible :\n" + deleteQuery.lastError().text());
                    return;
                }
                if (!chargerEquipesDepuisOracle()) {
                    QMessageBox::warning(this, "Oracle", "La suppression est effectuée, mais le rechargement a échoué.");
                    return;
                }
                filtrerParComboBox();
                QMessageBox::information(this, "Succès", "L'équipe a été supprimée d'Oracle.");
            }
        });
    }
}

void MainWindow::ouvrirGestionChef(int idEquipe)
{
    QSqlDatabase db = QSqlDatabase::database("hackathon_connection");

    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::critical(this, "Oracle",
                              "La connexion à la base de données est indisponible.");
        return;
    }

    // Vérifier et charger les participants de cette équipe
    QSqlQuery query(db);
    query.prepare(
        "SELECT ID_PARTICIPANT, NOM, PRENOM "
        "FROM PARTICIPANT "
        "WHERE ID_EQUIPE = ? "
        "ORDER BY NOM, PRENOM"
        );
    query.addBindValue(idEquipe);

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur Oracle",
                              query.lastError().text());
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Gestion du chef d'équipe");
    dialog.setFixedSize(420, 200);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *titre = new QLabel("Désigner le chef de cette équipe", &dialog);
    titre->setStyleSheet(
        "font-size: 15px; font-weight: bold; color: #1E3A8A;"
        );
    layout->addWidget(titre);

    QComboBox *comboChef = new QComboBox(&dialog);
    comboChef->addItem("Aucun chef", -1);

    int nombreParticipants = 0;

    while (query.next()) {
        int idParticipant = query.value(0).toInt();

        QString nomComplet =
            query.value(2).toString().trimmed() + " " +
            query.value(1).toString().trimmed();

        comboChef->addItem(nomComplet, idParticipant);
        nombreParticipants++;
    }

    if (nombreParticipants == 0) {
        QMessageBox::information(
            this, "Gestion du chef",
            "Cette équipe ne possède aucun participant.\n"
            "Inscrivez d'abord des membres."
            );
        return;
    }

    // Récupérer le chef actuellement désigné
    QSqlQuery chefActuel(db);
    chefActuel.prepare(
        "SELECT ID_CHEF FROM EQUIPE WHERE ID_EQUIPE = ?"
        );
    chefActuel.addBindValue(idEquipe);

    if (!chefActuel.exec() || !chefActuel.next()) {
        QMessageBox::critical(this, "Erreur Oracle",
                              chefActuel.lastError().text());
        return;
    }

    int idChefActuel = chefActuel.value(0).isNull()
                           ? -1
                           : chefActuel.value(0).toInt();

    int indexActuel = comboChef->findData(idChefActuel);
    comboChef->setCurrentIndex(indexActuel >= 0 ? indexActuel : 0);

    layout->addWidget(new QLabel("Chef d'équipe :", &dialog));
    layout->addWidget(comboChef);

    QHBoxLayout *buttons = new QHBoxLayout();

    QPushButton *btnEnregistrer =
        new QPushButton("Enregistrer", &dialog);
    QPushButton *btnAnnuler =
        new QPushButton("Annuler", &dialog);

    btnEnregistrer->setStyleSheet(
        "background-color: #2563EB; color: white;"
        "font-weight: bold; padding: 7px;"
        );

    buttons->addStretch();
    buttons->addWidget(btnEnregistrer);
    buttons->addWidget(btnAnnuler);

    layout->addLayout(buttons);

    connect(btnEnregistrer, &QPushButton::clicked,
            &dialog, &QDialog::accept);

    connect(btnAnnuler, &QPushButton::clicked,
            &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    int nouvelIdChef = comboChef->currentData().toInt();

    QSqlQuery updateQuery(db);

    if (nouvelIdChef <= 0) {
        updateQuery.prepare(
            "UPDATE EQUIPE SET ID_CHEF = NULL "
            "WHERE ID_EQUIPE = ?"
            );
        updateQuery.addBindValue(idEquipe);
    } else {
        updateQuery.prepare(
            "UPDATE EQUIPE SET ID_CHEF = ? "
            "WHERE ID_EQUIPE = ?"
            );
        updateQuery.addBindValue(nouvelIdChef);
        updateQuery.addBindValue(idEquipe);
    }

    if (!updateQuery.exec()) {
        QMessageBox::critical(
            this, "Erreur Oracle",
            "Impossible de modifier le chef :\n" +
                updateQuery.lastError().text()
            );
        return;
    }

    if (!chargerEquipesDepuisOracle()) {
        QMessageBox::warning(
            this, "Oracle",
            "Le chef est enregistré, mais le rechargement a échoué."
            );
        return;
    }

    if (!ui->lineEdit->text().trimmed().isEmpty())
        filtrerEquipes(ui->lineEdit->text());
    else
        filtrerParComboBox();

    QMessageBox::information(
        this, "Succès",
        "Le chef d'équipe a été mis à jour."
        );
}

void MainWindow::ouvrirGestionMembres(int idEquipe, int maxMembres,
                                      QPushButton *labelNombre,
                                      QTextEdit *listeMembres)
{
    QSqlDatabase db = QSqlDatabase::database("hackathon_connection");
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::critical(this, "Oracle", "La connexion à la base de données est indisponible.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Gestion des membres de l'équipe");
    dialog.resize(760, 430);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *titre = new QLabel(QString("Membres de l'équipe (ID : %1)").arg(idEquipe), &dialog);
    titre->setStyleSheet("font-size: 16px; font-weight: bold; color: #1E3A8A;");
    layout->addWidget(titre);

    QTableWidget *table = new QTableWidget(&dialog);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"ID", "Nom", "Prénom", "Email", "Téléphone"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setColumnHidden(0, true);
    layout->addWidget(table);

    QLabel *info = new QLabel(&dialog);
    layout->addWidget(info);

    QHBoxLayout *buttons = new QHBoxLayout();
    QPushButton *btnAjouter = new QPushButton("Ajouter un membre", &dialog);
    QPushButton *btnModifier = new QPushButton("Modifier le membre", &dialog);
    QPushButton *btnSupprimer = new QPushButton("Supprimer le membre", &dialog);
    QPushButton *btnFermer = new QPushButton("Fermer", &dialog);
    btnAjouter->setStyleSheet("background:#2563EB;color:white;padding:7px;font-weight:bold;");
    buttons->addWidget(btnAjouter);
    buttons->addWidget(btnModifier);
    buttons->addWidget(btnSupprimer);
    buttons->addStretch();
    buttons->addWidget(btnFermer);
    layout->addLayout(buttons);

    auto chargerTable = [&]() -> int {
        table->setRowCount(0);
        QSqlQuery q(db);
        q.prepare("SELECT ID_PARTICIPANT, NOM, PRENOM, EMAIL, TELEPHONE "
                  "FROM PARTICIPANT WHERE ID_EQUIPE = ? ORDER BY NOM, PRENOM");
        q.addBindValue(idEquipe);
        if (!q.exec()) {
            QMessageBox::critical(&dialog, "Erreur Oracle", q.lastError().text());
            return -1;
        }
        while (q.next()) {
            int row = table->rowCount();
            table->insertRow(row);
            for (int col = 0; col < 5; ++col)
                table->setItem(row, col, new QTableWidgetItem(q.value(col).toString()));
        }
        int count = table->rowCount();
        info->setText(QString("Nombre de membres : %1 / %2").arg(count).arg(maxMembres));
        return count;
    };

    auto editerParticipant = [&](bool modification) -> bool {
        int selectedRow = table->currentRow();
        if (modification && selectedRow < 0) {
            QMessageBox::information(&dialog, "Sélection", "Sélectionnez d'abord un membre.");
            return false;
        }
        int idParticipant = modification ? table->item(selectedRow, 0)->text().toInt() : 0;
        QDialog form(&dialog);
        form.setWindowTitle(modification ? "Modifier un membre" : "Ajouter un membre");
        QFormLayout formLayout(&form);
        QLineEdit nom(&form), prenom(&form), email(&form), telephone(&form);
        if (modification) {
            nom.setText(table->item(selectedRow, 1)->text());
            prenom.setText(table->item(selectedRow, 2)->text());
            email.setText(table->item(selectedRow, 3)->text());
            telephone.setText(table->item(selectedRow, 4)->text());
        }
        formLayout.addRow("Nom :", &nom);
        formLayout.addRow("Prénom :", &prenom);
        formLayout.addRow("Email :", &email);
        formLayout.addRow("Téléphone :", &telephone);
        QDialogButtonBox box(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &form);
        formLayout.addRow(&box);
        QObject::connect(&box, &QDialogButtonBox::accepted, &form, &QDialog::accept);
        QObject::connect(&box, &QDialogButtonBox::rejected, &form, &QDialog::reject);
        if (form.exec() != QDialog::Accepted)
            return false;
        if (nom.text().trimmed().isEmpty() || prenom.text().trimmed().isEmpty()) {
            QMessageBox::warning(&form, "Données invalides", "Le nom et le prénom sont obligatoires.");
            return false;
        }

        QSqlQuery q(db);
        if (modification) {
            q.prepare("UPDATE PARTICIPANT SET NOM = ?, PRENOM = ?, EMAIL = ?, TELEPHONE = ? "
                      "WHERE ID_PARTICIPANT = ? AND ID_EQUIPE = ?");
            q.addBindValue(nom.text().trimmed());
            q.addBindValue(prenom.text().trimmed());
            q.addBindValue(email.text().trimmed());
            q.addBindValue(telephone.text().trimmed());
            q.addBindValue(idParticipant);
            q.addBindValue(idEquipe);
        } else {
            q.prepare("INSERT INTO PARTICIPANT "
                      "(ID_PARTICIPANT, NOM, PRENOM, EMAIL, TELEPHONE, ID_EQUIPE, DATE_PARTICIPANT) "
                      "SELECT NVL(MAX(ID_PARTICIPANT), 0) + 1, ?, ?, ?, ?, ?, SYSDATE FROM PARTICIPANT");
            q.addBindValue(nom.text().trimmed());
            q.addBindValue(prenom.text().trimmed());
            q.addBindValue(email.text().trimmed());
            q.addBindValue(telephone.text().trimmed());
            q.addBindValue(idEquipe);
        }
        if (!q.exec()) {
            QMessageBox::critical(&form, "Erreur Oracle", q.lastError().text());
            return false;
        }
        return true;
    };

    connect(btnAjouter, &QPushButton::clicked, &dialog, [&]() {
        int count = table->rowCount();
        if (maxMembres > 0 && count >= maxMembres) {
            QMessageBox::warning(&dialog, "Capacité atteinte", "Cette équipe a atteint le nombre maximal de membres.");
            return;
        }
        if (editerParticipant(false)) chargerTable();
    });
    connect(btnModifier, &QPushButton::clicked, &dialog, [&]() {
        if (editerParticipant(true)) chargerTable();
    });
    connect(btnSupprimer, &QPushButton::clicked, &dialog, [&]() {
        int row = table->currentRow();
        if (row < 0) {
            QMessageBox::information(&dialog, "Sélection", "Sélectionnez d'abord un membre.");
            return;
        }
        int idParticipant = table->item(row, 0)->text().toInt();
        if (QMessageBox::question(&dialog, "Confirmation", "Supprimer ce membre de l'équipe ?",
                                  QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            return;
        QSqlQuery q(db);
        q.prepare("UPDATE EQUIPE SET ID_CHEF = NULL WHERE ID_EQUIPE = ? AND ID_CHEF = ?");
        q.addBindValue(idEquipe);
        q.addBindValue(idParticipant);
        if (!q.exec()) {
            QMessageBox::critical(&dialog, "Erreur Oracle", q.lastError().text());
            return;
        }
        q.prepare("DELETE FROM PARTICIPANT WHERE ID_PARTICIPANT = ? AND ID_EQUIPE = ?");
        q.addBindValue(idParticipant);
        q.addBindValue(idEquipe);
        if (!q.exec()) {
            QMessageBox::critical(&dialog, "Erreur Oracle", q.lastError().text());
            return;
        }
        chargerTable();
    });
    connect(btnFermer, &QPushButton::clicked, &dialog, &QDialog::accept);

    chargerTable();
    dialog.exec();

    if (chargerEquipesDepuisOracle()) {
        for (const Equipe &e : toutesLesEquipes) {
            if (e.idEquipe == idEquipe) {
                if (labelNombre)
                    labelNombre->setText(QString("%1 / %2").arg(e.nomsMembres.size()).arg(e.maxMembres));
                if (listeMembres)
                    listeMembres->setPlainText(e.nomsMembres.isEmpty()
                                                   ? "Aucun membre inscrit."
                                                   : e.nomsMembres.join("\n"));
                break;
            }
        }
        filtrerParComboBox();
    }
}
bool MainWindow::chargerEquipesDepuisOracle()
{
    QSqlDatabase db = QSqlDatabase::database("hackathon_connection");
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "Connexion Oracle indisponible :" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);
    const QString sql =
        "SELECT e.ID_EQUIPE, e.NOM_EQUIPE, e.DATE_INSCRIPTION, "
        "e.ID_COMPETITION, c.NOM, c.NB_MAX_MEMBRES, e.ID_CHEF "
        "FROM EQUIPE e LEFT JOIN COMPETITION c "
        "ON c.ID_COMPETITION = e.ID_COMPETITION "
        "ORDER BY e.ID_EQUIPE";
    if (!query.exec(sql)) {
        qDebug() << "Erreur lecture EQUIPE :" << query.lastError().text();
        return false;
    }

    QList<Equipe> equipesChargees;
    while (query.next()) {
        Equipe eq;
        eq.idEquipe = query.value(0).toInt();
        eq.nom = query.value(1).toString();
        eq.date = query.value(2).toDate().toString("dd/MM/yyyy");
        eq.idCompetition = query.value(3).toInt();
        eq.idChef = query.value(6).toInt();
        eq.competition = query.value(4).toString();
        if (eq.competition.isEmpty())
            eq.competition = QString::number(eq.idCompetition);

        eq.maxMembres = query.value(5).toInt();

        // Lire les vrais participants rattaches a cette equipe.
        QSqlQuery membresQuery(db);
        membresQuery.prepare(
            "SELECT NOM, PRENOM FROM PARTICIPANT "
            "WHERE ID_EQUIPE = ? ORDER BY NOM, PRENOM");
        membresQuery.addBindValue(eq.idEquipe);

        if (!membresQuery.exec()) {
            qDebug() << "Erreur lecture PARTICIPANT pour equipe"
                     << eq.idEquipe << ":" << membresQuery.lastError().text();
            return false;
        }

        while (membresQuery.next()) {
            const QString nom = membresQuery.value(0).toString().trimmed();
            const QString prenom = membresQuery.value(1).toString().trimmed();
            const QString nomComplet = (prenom + " " + nom).trimmed();
            if (!nomComplet.isEmpty())
                eq.nomsMembres.append(nomComplet);
        }

        const int nbMembres = eq.nomsMembres.size();

        // Le chef est un participant de la même équipe, référencé par ID_CHEF.
        eq.chef = "-";
        if (eq.idChef > 0) {
            QSqlQuery chefQuery(db);
            chefQuery.prepare(
                "SELECT NOM, PRENOM FROM PARTICIPANT "
                "WHERE ID_PARTICIPANT = ? AND ID_EQUIPE = ?");
            chefQuery.addBindValue(eq.idChef);
            chefQuery.addBindValue(eq.idEquipe);
            if (chefQuery.exec() && chefQuery.next()) {
                const QString nom = chefQuery.value(0).toString().trimmed();
                const QString prenom = chefQuery.value(1).toString().trimmed();
                eq.chef = (prenom + " " + nom).trimmed();
            }
        }

        eq.membres = QString::number(nbMembres) + "/" +
                     QString::number(eq.maxMembres);

        if (nbMembres == 0)
            eq.statut = "Incomplète";
        else if (eq.maxMembres > 0 && nbMembres >= eq.maxMembres)
            eq.statut = "Active";
        else
            eq.statut = "En cours";

        equipesChargees.append(eq);
    }

    toutesLesEquipes = equipesChargees;
    equipesAffichees = toutesLesEquipes;
    actualiserStatistiques();
    qDebug() << "Nombre d'equipes chargees :" << toutesLesEquipes.size();
    return true;
}
