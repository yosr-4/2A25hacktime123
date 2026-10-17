// Module « Gestion des équipes » (intégré dans HackTime : page 2 de SWHackTime).
// Classe MainWindow du projet d'origine renommée en Gequipe.
#include "gequipe.h"
#include "ui_gequipe.h"
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
#include <algorithm>

class CourbeChartWidget : public QWidget {
public:
    explicit CourbeChartWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedHeight(150);
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.fillRect(rect(), QColor("#FFFFFF"));

        painter.setPen(QPen(QColor("#CBD5E1"), 1));
        painter.drawLine(40, 120, width() - 20, 120);
        painter.drawLine(40, 20, 40, 120);

        painter.setPen(QColor("#64748B"));
        painter.setFont(QFont("Segoe UI", 8));
        painter.drawText(15, 25, "12");
        painter.drawText(15, 57, "8");
        painter.drawText(15, 88, "4");
        painter.drawText(20, 120, "0");

        painter.drawText(40, 135, "0");
        painter.drawText(100, 135, "2");
        painter.drawText(160, 135, "4");
        painter.drawText(220, 135, "6");
        painter.drawText(280, 135, "8");
        painter.drawText(340, 135, "10");

        QPolygonF pointsActive;
        pointsActive << QPointF(40, 110) << QPointF(100, 95) << QPointF(160, 75) << QPointF(220, 55) << QPointF(280, 30) << QPointF(340, 80) << QPointF(400, 35);
        painter.setPen(QPen(QColor("#2563EB"), 2));
        painter.drawPolyline(pointsActive);

        QPolygonF pointsIncomplete;
        pointsIncomplete << QPointF(40, 118) << QPointF(100, 105) << QPointF(160, 90) << QPointF(220, 70) << QPointF(280, 45) << QPointF(340, 100) << QPointF(390, 75);
        painter.setPen(QPen(QColor("#64748B"), 2));
        painter.drawPolyline(pointsIncomplete);
    }
};

class BarChartWidget : public QWidget {
    int activeCount;
    int incompleteCount;
public:
    BarChartWidget(int active, int incomplete, QWidget *parent = nullptr)
        : QWidget(parent), activeCount(active), incompleteCount(incomplete) {
        setFixedHeight(120);
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
        painter.drawText(75, 112, "Done");
        painter.drawText(165, 112, "In Progress");
    }
};

Gequipe::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Gequipe)
{
    ui->setupUi(this);

    QLabel *logoLabel = new QLabel(this);
    QPixmap pixmap(":/new/prefix1/logo.png");
    if (!pixmap.isNull()) {
        logoLabel->setPixmap(pixmap.scaled(230, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    logoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    logoLabel->setGeometry(5, 10, 230, 90);

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

    initialiserToutesLesEquipes();
    chargerPage(1);

    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &Gequipe::onTeamSelected);

    connect(ui->btn1, &QPushButton::clicked, this, [=](){ chargerPage(1); });
    connect(ui->btn2, &QPushButton::clicked, this, [=](){ chargerPage(2); });
    connect(ui->btn3, &QPushButton::clicked, this, [=](){ chargerPage(3); });
    connect(ui->btn4, &QPushButton::clicked, this, [=](){ chargerPage(4); });

    connect(ui->btnPrev, &QPushButton::clicked, this, [=](){ chargerPage(qMax(1, currentPage - 1)); });
    connect(ui->btnNext, &QPushButton::clicked, this, [=](){ chargerPage(qMin(4, currentPage + 1)); });

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &Gequipe::filtrerEquipes);

    connect(ui->comboBoxCompetition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Gequipe::filtrerParComboBox);
    connect(ui->comboBoxStatus, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Gequipe::filtrerParComboBox);
    connect(ui->comboBoxTrier, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Gequipe::filtrerParComboBox);

}

Gequipe::~MainWindow()
{
    delete ui;
}

void Gequipe::on_btnStatistiques_clicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Statistiques de Performance - Équipes");
    dialog.resize(750, 600);
    dialog.setStyleSheet("background-color: #F8FAFC; color: #0F172A; font-family: 'Segoe UI';");

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    QLabel *lblTitle = new QLabel("Statistiques de Performance & Rapports", &dialog);
    lblTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #1E293B; margin-bottom: 2px;");
    mainLayout->addWidget(lblTitle);

    int totalEquipes = toutesLesEquipes.size();
    int equipesActives = 0;
    int equipesIncompletes = 0;

    for (int i = 0; i < toutesLesEquipes.size(); ++i) {
        if (toutesLesEquipes[i].statut == "Active") {
            equipesActives++;
        } else {
            equipesIncompletes++;
        }
    }

    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(10);

    QFrame *card1 = new QFrame(&dialog);
    card1->setStyleSheet("background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; padding: 10px;");
    QVBoxLayout *c1Layout = new QVBoxLayout(card1);
    c1Layout->addWidget(new QLabel("Total Équipes", card1));
    QLabel *lblC1Val = new QLabel(QString::number(totalEquipes), card1);
    lblC1Val->setStyleSheet("color: #2563EB; font-size: 20px; font-weight: bold;");
    c1Layout->addWidget(lblC1Val);
    cardsLayout->addWidget(card1);

    QFrame *card2 = new QFrame(&dialog);
    card2->setStyleSheet("background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; padding: 10px;");
    QVBoxLayout *c2Layout = new QVBoxLayout(card2);
    c2Layout->addWidget(new QLabel("Équipes Actives (Done)", card2));
    QLabel *lblC2Val = new QLabel(QString::number(equipesActives), card2);
    lblC2Val->setStyleSheet("color: #10B981; font-size: 20px; font-weight: bold;");
    c2Layout->addWidget(lblC2Val);
    cardsLayout->addWidget(card2);

    QFrame *card3 = new QFrame(&dialog);
    card3->setStyleSheet("background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; padding: 10px;");
    QVBoxLayout *c3Layout = new QVBoxLayout(card3);
    c3Layout->addWidget(new QLabel("Incomplètes (In Progress)", card3));
    QLabel *lblC3Val = new QLabel(QString::number(equipesIncompletes), card3);
    lblC3Val->setStyleSheet("color: #F59E0B; font-size: 20px; font-weight: bold;");
    c3Layout->addWidget(lblC3Val);
    cardsLayout->addWidget(card3);

    mainLayout->addLayout(cardsLayout);

    QFrame *frameCourbe = new QFrame(&dialog);
    frameCourbe->setStyleSheet("background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; padding: 8px;");
    QVBoxLayout *fLayout1 = new QVBoxLayout(frameCourbe);
    QLabel *lblCHeader = new QLabel("📈 Enregistrements Hebdomadaires", frameCourbe);
    lblCHeader->setStyleSheet("font-weight: bold; color: #334155; font-size: 12px;");
    fLayout1->addWidget(lblCHeader);

    CourbeChartWidget *courbeWidget = new CourbeChartWidget(frameCourbe);
    fLayout1->addWidget(courbeWidget);
    mainLayout->addWidget(frameCourbe);

    QFrame *frameBar = new QFrame(&dialog);
    frameBar->setStyleSheet("background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; padding: 8px;");
    QVBoxLayout *fLayout2 = new QVBoxLayout(frameBar);
    QLabel *lblBHeader = new QLabel("📊 Statut des Projets (Done vs In Progress)", frameBar);
    lblBHeader->setStyleSheet("font-weight: bold; color: #334155; font-size: 12px;");
    fLayout2->addWidget(lblBHeader);

    BarChartWidget *barWidget = new BarChartWidget(equipesActives, equipesIncompletes, frameBar);
    fLayout2->addWidget(barWidget);
    mainLayout->addWidget(frameBar);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnFermer = new QPushButton("Fermer", &dialog);
    btnFermer->setStyleSheet("background-color: #64748B; color: white; font-weight: bold; padding: 6px 20px; border-radius: 4px;");
    connect(btnFermer, &QPushButton::clicked, &dialog, &QDialog::accept);
    btnLayout->addStretch();
    btnLayout->addWidget(btnFermer);
    mainLayout->addLayout(btnLayout);

    dialog.exec();
}

void Gequipe::chargerPage(int page) {
    int totalPages = 4;
    if (page < 1 || page > totalPages) return;
    currentPage = page;

    int itemsPerPage = 8;
    int startIndex = (currentPage - 1) * itemsPerPage;
    int endIndex = qMin(startIndex + itemsPerPage, toutesLesEquipes.size());
    int currentCount = endIndex - startIndex;

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
        lblMembres->setStyleSheet("color: #475569; font-weight: bold; min-width: 35px;");

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 6);
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

        QWidget *actionsWidget = new QWidget();
        QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->addWidget(btnEdit);
        actionsLayout->addWidget(btnDelete);
        actionsLayout->setAlignment(Qt::AlignCenter);
        actionsLayout->setContentsMargins(0, 0, 0, 0);
        actionsWidget->setLayout(actionsLayout);

        ui->tableWidget->setCellWidget(i, 6, actionsWidget);

        connect(btnEdit, &QPushButton::clicked, this, [=]() {
            QDialog editDialog(this);
            editDialog.setWindowTitle("Modifier l'équipe");
            editDialog.resize(380, 320);

            QVBoxLayout *mainLayout = new QVBoxLayout(&editDialog);
            QFormLayout *formLayout = new QFormLayout();

            QLineEdit *txtNom = new QLineEdit(eq.nom, &editDialog);
            QLineEdit *txtCompetition = new QLineEdit(eq.competition, &editDialog);
            QLineEdit *txtChef = new QLineEdit(eq.chef, &editDialog);
            QLineEdit *txtMembres = new QLineEdit(eq.membres, &editDialog);
            QLineEdit *txtDate = new QLineEdit(eq.date, &editDialog);

            QComboBox *comboStatut = new QComboBox(&editDialog);
            comboStatut->addItems({"Active", "Incomplète"});
            comboStatut->setCurrentText(eq.statut);

            formLayout->addRow("Nom de l'équipe :", txtNom);
            formLayout->addRow("Compétition :", txtCompetition);
            formLayout->addRow("Chef d'équipe :", txtChef);
            formLayout->addRow("Membres :", txtMembres);
            formLayout->addRow("Date :", txtDate);
            formLayout->addRow("Statut :", comboStatut);

            mainLayout->addLayout(formLayout);

            QPushButton *btnEnregistrer = new QPushButton("Enregistrer les modifications", &editDialog);
            btnEnregistrer->setStyleSheet("background-color: #2563EB; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");
            mainLayout->addWidget(btnEnregistrer);

            connect(btnEnregistrer, &QPushButton::clicked, &editDialog, &QDialog::accept);

            if (editDialog.exec() == QDialog::Accepted) {
                for (int j = 0; j < toutesLesEquipes.size(); ++j) {
                    if (toutesLesEquipes[j].nom == eq.nom && toutesLesEquipes[j].competition == eq.competition) {
                        toutesLesEquipes[j].nom = txtNom->text().trimmed();
                        toutesLesEquipes[j].competition = txtCompetition->text().trimmed();
                        toutesLesEquipes[j].chef = txtChef->text().trimmed();
                        toutesLesEquipes[j].membres = txtMembres->text().trimmed();
                        toutesLesEquipes[j].date = txtDate->text().trimmed();
                        toutesLesEquipes[j].statut = comboStatut->currentText();
                        break;
                    }
                }
                chargerPage(currentPage);
                QMessageBox::information(this, "Succès", "L'équipe a été modifiée avec succès !");
            }
        });

        connect(btnDelete, &QPushButton::clicked, this, [=]() {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer cette équipe ?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                for (int j = 0; j < toutesLesEquipes.size(); ++j) {
                    if (toutesLesEquipes[j].nom == eq.nom && toutesLesEquipes[j].competition == eq.competition) {
                        toutesLesEquipes.removeAt(j);
                        break;
                    }
                }
                chargerPage(currentPage);
                QMessageBox::information(this, "Succès", "L'équipe a été supprimée avec succès !");
            }
        });
    }
}

void Gequipe::onTeamSelected(int row, int column) {
    if (column == 6) return;
    if (row < 0 || row >= equipesAffichees.size()) return;

    Equipe eq = equipesAffichees[row];

    QString details = QString("Nom de l'équipe : %1\n"
                              "Compétition : %2\n"
                              "Chef d'équipe : %3\n"
                              "Membres : %4\n"
                              "Date d'inscription : %5\n"
                              "Statut : %6")
                          .arg(eq.nom)
                          .arg(eq.competition)
                          .arg(eq.chef)
                          .arg(eq.membres)
                          .arg(eq.date)
                          .arg(eq.statut);

    QMessageBox::information(this, "Détails de l'équipe", details);
}

void Gequipe::on_btnAjouter_clicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Ajouter une équipe");
    dialog.resize(380, 320);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();

    QLineEdit *txtNom = new QLineEdit(&dialog);
    QLineEdit *txtCompetition = new QLineEdit(&dialog);
    QLineEdit *txtChef = new QLineEdit(&dialog);
    QLineEdit *txtMembres = new QLineEdit(&dialog);
    txtMembres->setText("4/4");
    QLineEdit *txtDate = new QLineEdit(&dialog);
    txtDate->setText("05/10/2026");

    QComboBox *comboStatut = new QComboBox(&dialog);
    comboStatut->addItems({"Active", "Incomplète"});

    formLayout->addRow("Nom de l'équipe :", txtNom);
    formLayout->addRow("Compétition :", txtCompetition);
    formLayout->addRow("Chef d'équipe :", txtChef);
    formLayout->addRow("Membres :", txtMembres);
    formLayout->addRow("Date :", txtDate);
    formLayout->addRow("Statut :", comboStatut);

    mainLayout->addLayout(formLayout);

    QPushButton *btnEnregistrer = new QPushButton("Enregistrer", &dialog);
    btnEnregistrer->setStyleSheet("background-color: #2563EB; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");
    mainLayout->addWidget(btnEnregistrer);

    connect(btnEnregistrer, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        QString nom = txtNom->text().trimmed();
        QString competition = txtCompetition->text().trimmed();
        QString chef = txtChef->text().trimmed();
        QString membres = txtMembres->text().trimmed();
        QString date = txtDate->text().trimmed();
        QString statut = comboStatut->currentText();

        if (!nom.isEmpty() && !competition.isEmpty() && !chef.isEmpty()) {
            Equipe nouvelleEq = {nom, competition, chef, membres, date, statut};
            toutesLesEquipes.prepend(nouvelleEq);
            chargerPage(currentPage);
            QMessageBox::information(this, "Succès", "L'équipe a été ajoutée avec succès !");
        } else {
            QMessageBox::warning(this, "Erreur", "Veuillez remplir au moins le nom, la compétition et le chef d'équipe !");
        }
    }
}

void Gequipe::filtrerEquipes(const QString &text) {
    if (text.isEmpty()) {
        chargerPage(currentPage);
        return;
    }

    QList<Equipe> equipesFiltrees;
    for (int i = 0; i < toutesLesEquipes.size(); ++i) {
        const Equipe &eq = toutesLesEquipes.at(i);
        if (eq.nom.contains(text, Qt::CaseInsensitive) ||
            eq.chef.contains(text, Qt::CaseInsensitive) ||
            eq.competition.contains(text, Qt::CaseInsensitive)) {
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
        lblMembres->setStyleSheet("color: #475569; font-weight: bold; min-width: 35px;");

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 6);
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

        QWidget *actionsWidget = new QWidget();
        QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->addWidget(btnEdit);
        actionsLayout->addWidget(btnDelete);
        actionsLayout->setAlignment(Qt::AlignCenter);
        actionsLayout->setContentsMargins(0, 0, 0, 0);
        actionsWidget->setLayout(actionsLayout);

        ui->tableWidget->setCellWidget(i, 6, actionsWidget);

        connect(btnEdit, &QPushButton::clicked, this, [=]() {
            QDialog editDialog(this);
            editDialog.setWindowTitle("Modifier l'équipe");
            editDialog.resize(380, 320);

            QVBoxLayout *mainLayout = new QVBoxLayout(&editDialog);
            QFormLayout *formLayout = new QFormLayout();

            QLineEdit *txtNom = new QLineEdit(eq.nom, &editDialog);
            QLineEdit *txtCompetition = new QLineEdit(eq.competition, &editDialog);
            QLineEdit *txtChef = new QLineEdit(eq.chef, &editDialog);
            QLineEdit *txtMembres = new QLineEdit(eq.membres, &editDialog);
            QLineEdit *txtDate = new QLineEdit(eq.date, &editDialog);

            QComboBox *comboStatut = new QComboBox(&editDialog);
            comboStatut->addItems({"Active", "Incomplète"});
            comboStatut->setCurrentText(eq.statut);

            formLayout->addRow("Nom de l'équipe :", txtNom);
            formLayout->addRow("Compétition :", txtCompetition);
            formLayout->addRow("Chef d'équipe :", txtChef);
            formLayout->addRow("Membres :", txtMembres);
            formLayout->addRow("Date :", txtDate);
            formLayout->addRow("Statut :", comboStatut);

            mainLayout->addLayout(formLayout);

            QPushButton *btnEnregistrer = new QPushButton("Enregistrer les modifications", &editDialog);
            btnEnregistrer->setStyleSheet("background-color: #2563EB; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");
            mainLayout->addWidget(btnEnregistrer);

            connect(btnEnregistrer, &QPushButton::clicked, &editDialog, &QDialog::accept);

            if (editDialog.exec() == QDialog::Accepted) {
                for (int j = 0; j < toutesLesEquipes.size(); ++j) {
                    if (toutesLesEquipes[j].nom == eq.nom && toutesLesEquipes[j].competition == eq.competition) {
                        toutesLesEquipes[j].nom = txtNom->text().trimmed();
                        toutesLesEquipes[j].competition = txtCompetition->text().trimmed();
                        toutesLesEquipes[j].chef = txtChef->text().trimmed();
                        toutesLesEquipes[j].membres = txtMembres->text().trimmed();
                        toutesLesEquipes[j].date = txtDate->text().trimmed();
                        toutesLesEquipes[j].statut = comboStatut->currentText();
                        break;
                    }
                }
                filtrerEquipes(ui->lineEdit->text());
                QMessageBox::information(this, "Succès", "L'équipe a été modifiée avec succès !");
            }
        });

        connect(btnDelete, &QPushButton::clicked, this, [=]() {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer cette équipe ?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                for (int j = 0; j < toutesLesEquipes.size(); ++j) {
                    if (toutesLesEquipes[j].nom == eq.nom && toutesLesEquipes[j].competition == eq.competition) {
                        toutesLesEquipes.removeAt(j);
                        break;
                    }
                }
                filtrerEquipes(ui->lineEdit->text());
                QMessageBox::information(this, "Succès", "L'équipe a été supprimée avec succès !");
            }
        });
    }
}

void Gequipe::filtrerParComboBox() {
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
            return a.date < b.date;
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
        lblMembres->setStyleSheet("color: #475569; font-weight: bold; min-width: 35px;");

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 6);
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

        QWidget *actionsWidget = new QWidget();
        QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->addWidget(btnEdit);
        actionsLayout->addWidget(btnDelete);
        actionsLayout->setAlignment(Qt::AlignCenter);
        actionsLayout->setContentsMargins(0, 0, 0, 0);
        actionsWidget->setLayout(actionsLayout);

        ui->tableWidget->setCellWidget(i, 6, actionsWidget);

        connect(btnEdit, &QPushButton::clicked, this, [=]() {
            QDialog editDialog(this);
            editDialog.setWindowTitle("Modifier l'équipe");
            editDialog.resize(380, 320);

            QVBoxLayout *mainLayout = new QVBoxLayout(&editDialog);
            QFormLayout *formLayout = new QFormLayout();

            QLineEdit *txtNom = new QLineEdit(eq.nom, &editDialog);
            QLineEdit *txtCompetition = new QLineEdit(eq.competition, &editDialog);
            QLineEdit *txtChef = new QLineEdit(eq.chef, &editDialog);
            QLineEdit *txtMembres = new QLineEdit(eq.membres, &editDialog);
            QLineEdit *txtDate = new QLineEdit(eq.date, &editDialog);

            QComboBox *comboStatut = new QComboBox(&editDialog);
            comboStatut->addItems({"Active", "Incomplète"});
            comboStatut->setCurrentText(eq.statut);

            formLayout->addRow("Nom de l'équipe :", txtNom);
            formLayout->addRow("Compétition :", txtCompetition);
            formLayout->addRow("Chef d'équipe :", txtChef);
            formLayout->addRow("Membres :", txtMembres);
            formLayout->addRow("Date :", txtDate);
            formLayout->addRow("Statut :", comboStatut);

            mainLayout->addLayout(formLayout);

            QPushButton *btnEnregistrer = new QPushButton("Enregistrer les modifications", &editDialog);
            btnEnregistrer->setStyleSheet("background-color: #2563EB; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");
            mainLayout->addWidget(btnEnregistrer);

            connect(btnEnregistrer, &QPushButton::clicked, &editDialog, &QDialog::accept);

            if (editDialog.exec() == QDialog::Accepted) {
                for (int j = 0; j < toutesLesEquipes.size(); ++j) {
                    if (toutesLesEquipes[j].nom == eq.nom && toutesLesEquipes[j].competition == eq.competition) {
                        toutesLesEquipes[j].nom = txtNom->text().trimmed();
                        toutesLesEquipes[j].competition = txtCompetition->text().trimmed();
                        toutesLesEquipes[j].chef = txtChef->text().trimmed();
                        toutesLesEquipes[j].membres = txtMembres->text().trimmed();
                        toutesLesEquipes[j].date = txtDate->text().trimmed();
                        toutesLesEquipes[j].statut = comboStatut->currentText();
                        break;
                    }
                }
                filtrerParComboBox();
                QMessageBox::information(this, "Succès", "L'équipe a été modifiée avec succès !");
            }
        });

        connect(btnDelete, &QPushButton::clicked, this, [=]() {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer cette équipe ?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                for (int j = 0; j < toutesLesEquipes.size(); ++j) {
                    if (toutesLesEquipes[j].nom == eq.nom && toutesLesEquipes[j].competition == eq.competition) {
                        toutesLesEquipes.removeAt(j);
                        break;
                    }
                }
                filtrerParComboBox();
                QMessageBox::information(this, "Succès", "L'équipe a été supprimée avec succès !");
            }
        });
    }
}

void Gequipe::initialiserToutesLesEquipes() {
    toutesLesEquipes.append({"Matrix ReLoaded", "Cyber Security Jam", "Mariem Khedhri", "5/5", "25/10/2026", "Active"});
    toutesLesEquipes.append({"Bit Shifters", "IoT Challenge", "Omar Sghaier", "4/4", "26/10/2026", "Active"});
    toutesLesEquipes.append({"Kernel Panic", "DevOps Cup", "Nizar Chatti", "2/5", "27/10/2026", "Incomplète"});
    toutesLesEquipes.append({"Syntax Error", "FinTech Hack", "Syrine Guesmi", "4/4", "28/10/2026", "Active"});
    toutesLesEquipes.append({"Null Pointer", "AI CodeSprint", "Alaaeddine Chebbi", "3/6", "29/10/2026", "Incomplète"});
    toutesLesEquipes.append({"Segment Fault", "WebDev Masters", "Hiba Rezgui", "5/5", "30/10/2026", "Active"});
    toutesLesEquipes.append({"Deep Learning", "AI CodeSprint", "Bechir Tlili", "4/4", "31/10/2026", "Active"});
    toutesLesEquipes.append({"Quantum Hack", "Cloud Architecture", "Aziz Ferjani", "3/5", "01/11/2026", "Incomplète"});

    toutesLesEquipes.append({"Code Wizards", "Hack for a Better Future", "Ahmed Ben Ali", "5/5", "01/10/2026", "Active"});
    toutesLesEquipes.append({"Health Hackerz", "HealthTech Innovation", "Sara Trabelsi", "4/4", "02/10/2026", "Active"});
    toutesLesEquipes.append({"Edu Innovators", "EduTech Solutions", "Omar Khaled", "3/6", "03/10/2026", "Incomplète"});
    toutesLesEquipes.append({"Smart Minds", "Smart City Challenge", "Meriem Ayari", "5/5", "04/10/2026", "Active"});
    toutesLesEquipes.append({"Aero Tech", "AeroTech for Tomorrow", "Yassine Jabri", "2/3", "05/10/2026", "Incomplète"});
    toutesLesEquipes.append({"Green Energy", "Smart City Challenge", "Hamza Triki", "3/6", "16/10/2026", "Incomplète"});
    toutesLesEquipes.append({"Alpha Squad", "AI CodeSprint", "Rim Riahi", "4/4", "06/10/2026", "Active"});
    toutesLesEquipes.append({"Beta Coders", "WebDev Masters", "Mehdi Ben Salah", "3/5", "07/10/2026", "Active"});
    toutesLesEquipes.append({"Cyber Ninjas", "Cyber Security Jam", "Amine Gharbi", "5/5", "08/10/2026", "Active"});
    toutesLesEquipes.append({"Data Miners", "AI CodeSprint", "Nour Chabbouh", "2/4", "09/10/2026", "Incomplète"});
    toutesLesEquipes.append({"Cloud Nine", "Cloud Architecture", "Salma Kallel", "6/6", "10/10/2026", "Active"});
    toutesLesEquipes.append({"Pixel Perfect", "UI/UX Design Jam", "Malek Triki", "3/3", "11/10/2026", "Active"});
    toutesLesEquipes.append({"Byte Force", "IoT Challenge", "Skander Ben Ammar", "4/5", "12/10/2026", "Active"});
    toutesLesEquipes.append({"RoboTix", "Robotics Cup", "Farah Mejri", "5/5", "13/10/2026", "Active"});
    toutesLesEquipes.append({"Web Warriors", "WebDev Masters", "Zied Dridi", "2/4", "14/10/2026", "Incomplète"});
    toutesLesEquipes.append({"AlgoRhythm", "Competitive Programming", "Youssef Boughanmi", "3/3", "15/10/2026", "Active"});
    toutesLesEquipes.append({"InnoVibe", "FinTech Hack", "Chaima Sassi", "4/4", "17/10/2026", "Active"});
    toutesLesEquipes.append({"Neural Net", "AI CodeSprint", "Anas Ben Romdhane", "5/5", "18/10/2026", "Active"});
    toutesLesEquipes.append({"Fast Track", "Logistics Hack", "Wassim Jlassi", "3/4", "19/10/2026", "Incomplète"});
    toutesLesEquipes.append({"Blue Lock", "Cyber Security Jam", "Rayen Loukil", "4/4", "20/10/2026", "Active"});
    toutesLesEquipes.append({"EcoHack", "Green Tech", "Emna Bouazizi", "3/5", "21/10/2026", "Incomplète"});
    toutesLesEquipes.append({"Titanium", "Hardware Jam", "Kais Mansour", "2/3", "22/10/2026", "Active"});
    toutesLesEquipes.append({"OmniDev", "FullStack Jam", "Rania Ben Hassine", "5/5", "23/10/2026", "Active"});
    toutesLesEquipes.append({"BitByBit", "Coding Marathon", "Ala Sfaxi", "4/4", "24/10/2026", "Active"});

    ui->comboBoxCompetition->clear();
    ui->comboBoxCompetition->addItem("Toutes");

    QStringList competitions;
    for (int i = 0; i < toutesLesEquipes.size(); ++i) {
        const Equipe &eq = toutesLesEquipes.at(i);
        if (!competitions.contains(eq.competition)) {
            competitions.append(eq.competition);
        }
    }
    ui->comboBoxCompetition->addItems(competitions);
}