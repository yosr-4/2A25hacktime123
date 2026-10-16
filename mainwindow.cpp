#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QFrame>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <QDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QEvent>
#include <QUrl>
#include <QApplication>
#include <QWidget>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTextStream>
#include <QFile>
#include <QTextDocument>
#include <QPdfWriter>
#include <QPageSize>
#include <QBuffer>

// Widget personnalisé pour dessiner les graphiques proprement
class ChartWidget : public QWidget {
protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.fillRect(rect(), QColor("#ffffff"));
        painter.setPen(QPen(QColor("#cbd5e1"), 1));
        painter.drawRect(rect().adjusted(0, 0, -1, -1));

        if (objectName() == "curveChart") {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#64748b"));
            painter.drawRect(45, 12, 16, 10);
            painter.setFont(QFont("Arial", 9, QFont::Bold));
            painter.setPen(QColor("#0f172a"));
            painter.drawText(70, 20, "Enregistrements Hebdomadaires");

            int originX = 50;
            int originY = 190;
            int axisWidth = 320;
            int axisHeight = 120;

            painter.setPen(QPen(QColor("#f1f5f9"), 1, Qt::SolidLine));
            for (int i = 0; i <= 6; ++i) {
                int y = originY - (i * (axisHeight / 6));
                painter.drawLine(originX, y, originX + axisWidth, y);

                painter.setPen(QColor("#475569"));
                painter.setFont(QFont("Arial", 8, QFont::Bold));
                painter.drawText(originX - 30, y + 4, QString::number(i * 2));
                painter.setPen(QPen(QColor("#f1f5f9"), 1, Qt::SolidLine));
            }

            painter.setPen(QColor("#475569"));
            painter.setFont(QFont("Arial", 8, QFont::Bold));
            for (int xVal = 0; xVal <= 10; xVal += 2) {
                int x = originX + (xVal * (axisWidth / 10));
                painter.drawText(x - 6, originY + 18, QString::number(xVal));
            }

            painter.save();
            painter.translate(15, originY - 60);
            painter.rotate(-90);
            painter.setFont(QFont("Arial", 8, QFont::Bold));
            painter.drawText(0, 0, "Participants/Temps");
            painter.restore();

            painter.setFont(QFont("Arial", 8, QFont::Bold));
            painter.drawText(originX + (axisWidth / 2) - 45, originY + 35, "Participants/Temps");

            QVector<QPointF> pts1 = {{0, 2}, {2, 4}, {4, 6}, {6, 9}, {7, 10}, {8, 6}, {10, 11}};
            QPolygonF poly1;
            for(auto &p : pts1) {
                double px = originX + (p.x() * (axisWidth / 10.0));
                double py = originY - (p.y() * (axisHeight / 12.0));
                poly1 << QPointF(px, py);
            }
            painter.setPen(QPen(QColor("#2563eb"), 2.5));
            painter.drawPolyline(poly1);

            QVector<QPointF> pts2 = {{0, 1}, {2, 2}, {4, 4}, {6, 8}, {7, 9}, {8, 3}, {10, 6}};
            QPolygonF poly2;
            for(auto &p : pts2) {
                double px = originX + (p.x() * (axisWidth / 10.0));
                double py = originY - (p.y() * (axisHeight / 12.0));
                poly2 << QPointF(px, py);
            }
            painter.setPen(QPen(QColor("#64748b"), 2.5));
            painter.drawPolyline(poly2);
        }
        else if (objectName() == "barChart") {
            int originX = 50;
            int originY = 160;
            int axisWidth = 320;
            int axisHeight = 110;

            painter.setPen(QPen(QColor("#f1f5f9"), 1, Qt::SolidLine));
            for (int i = 0; i <= 5; ++i) {
                int y = originY - (i * (axisHeight / 5));
                painter.drawLine(originX, y, originX + axisWidth, y);

                painter.setPen(QColor("#475569"));
                painter.setFont(QFont("Arial", 8, QFont::Bold));
                painter.drawText(originX - 25, y + 4, QString::number(i * 2));
                painter.setPen(QPen(QColor("#f1f5f9"), 1, Qt::SolidLine));
            }

            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#16a34a"));
            painter.drawRect(originX + 50, originY - (10 * (axisHeight / 10.0)), 28, 10 * (axisHeight / 10.0));

            painter.setBrush(QColor("#dc2626"));
            painter.drawRect(originX + 85, originY - (2 * (axisHeight / 10.0)), 28, 2 * (axisHeight / 10.0));

            painter.setFont(QFont("Arial", 9, QFont::Bold));
            painter.setPen(QColor("#0f172a"));
            painter.drawText(originX + 75, originY + 20, "Done");

            painter.setBrush(QColor("#16a34a"));
            painter.drawRect(originX + 210, originY - (2 * (axisHeight / 10.0)), 28, 2 * (axisHeight / 10.0));

            painter.setBrush(QColor("#dc2626"));
            painter.drawRect(originX + 245, originY - (2 * (axisHeight / 10.0)), 28, 2 * (axisHeight / 10.0));

            painter.drawText(originX + 222, originY + 20, "In Progress");
        }
    }
};

QPixmap createHackTimeLogoPixmap() {
    QPixmap pixmap(220, 60);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF iconRect(5, 5, 50, 50);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(13, 27, 42));
    painter.drawRoundedRect(iconRect, 10, 10);

    QRectF clockRect(10, 10, 40, 40);

    QPen bluePen(QColor(0, 122, 255), 3);
    painter.setPen(bluePen);
    painter.drawArc(clockRect, 45 * 16, 270 * 16);

    QPen orangePen(QColor(242, 107, 15), 3);
    painter.setPen(orangePen);
    painter.drawArc(clockRect, 15 * 16, 60 * 16);

    QFont codeFont("Arial", 8, QFont::Bold);
    painter.setFont(codeFont);
    painter.setPen(QColor(242, 107, 15));
    painter.drawText(QRectF(20, 6, 20, 12), Qt::AlignCenter, "</>");

    painter.setPen(QPen(QColor(0, 122, 255), 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(30, 30, 22, 22);
    painter.drawLine(30, 30, 42, 24);

    painter.setPen(QPen(QColor(242, 107, 15), 2));
    painter.drawLine(14, 30, 17, 30);
    painter.drawLine(43, 30, 46, 30);
    painter.drawLine(30, 43, 30, 46);

    QFont titleFont("Arial", 18, QFont::Bold);
    painter.setFont(titleFont);

    painter.setPen(QColor(255, 255, 255));
    painter.drawText(65, 32, "Hack");

    painter.setPen(QColor(242, 107, 15));
    painter.drawText(122, 32, "Time");

    QFont subFont("Arial", 5, QFont::Bold);
    painter.setFont(subFont);
    painter.setPen(QColor(148, 163, 184));
    painter.drawText(66, 45, "GESTION INTELLIGENTE DE HACKATHONS");

    return pixmap;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupSoundEffects();
    setupCustomUi();
    applyStyleSheet();
    setupLogoAnimation();

    if (qApp) {
        qApp->installEventFilter(this);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupSoundEffects()
{
    clickSound = new QSoundEffect(this);
    clickSound->setSource(QUrl::fromLocalFile("click.wav"));
    clickSound->setVolume(0.7f);
}

void MainWindow::playClickSound()
{
    if (clickSound) {
        if (clickSound->isPlaying()) {
            clickSound->stop();
        }
        clickSound->play();
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QPushButton *btn = qobject_cast<QPushButton*>(watched);
        if (btn) {
            playClickSound();
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setupLogoAnimation()
{
    if (!logoLabel) return;

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(logoLabel);
    logoLabel->setGraphicsEffect(effect);

    QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(1500);
    anim->setStartValue(0.4);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->setLoopCount(-1);
    anim->start();
}

QWidget* MainWindow::createActionButtonsWidget(int row)
{
    QWidget *actionsWidget = new QWidget();
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(6);
    actionsLayout->setAlignment(Qt::AlignCenter);

    QPushButton *btnVoir = new QPushButton("👁");
    QPushButton *btnModif = new QPushButton("✏");
    QPushButton *btnSuppr = new QPushButton("🗑️");

    QString btnStyle = "QPushButton { border: none; background: transparent; font-size: 14px; cursor: pointer; }"
                       "QPushButton:hover { font-size: 16px; }";
    btnVoir->setStyleSheet(btnStyle);
    btnModif->setStyleSheet(btnStyle);
    btnSuppr->setStyleSheet(btnStyle);

    connect(btnVoir, &QPushButton::clicked, this, [this, row]() { onVoirParticipantClicked(row); });
    connect(btnModif, &QPushButton::clicked, this, [this, row]() { onModifierParticipantClicked(row); });
    connect(btnSuppr, &QPushButton::clicked, this, [this, row]() { onSupprimerParticipantClicked(row); });

    actionsLayout->addWidget(btnVoir);
    actionsLayout->addWidget(btnModif);
    actionsLayout->addWidget(btnSuppr);

    return actionsWidget;
}

void MainWindow::updateCounters()
{
    if (lblActifs && table) {
        lblActifs->setText(QString::number(table->rowCount()));
    }
    if (lblNouveaux) {
        lblNouveaux->setText(QString::number(nouveauxInscritsCount));
    }
}

void MainWindow::setupCustomUi()
{
    this->resize(1300, 850);
    this->setWindowTitle("Hackathon - HackTime");

    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. SIDEBAR
    QFrame *sidebar = new QFrame();
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(250);

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(15, 20, 15, 20);
    sidebarLayout->setSpacing(10);

    logoLabel = new QLabel();
    logoLabel->setPixmap(createHackTimeLogoPixmap());
    sidebarLayout->addWidget(logoLabel);
    sidebarLayout->addSpacing(25);

    struct MenuItem {
        QString name;
        QString icon;
    };

    const QList<MenuItem> menuItems = {
        {"Tableau de bord", "📊"},
        {"Compétitions", "🏆"},
        {"Équipes", "👥"},
        {"Participants", "👤"},
        {"Atelier", "🛠️"},
        {"Employés", "👨‍💼"},
        {"Paramètres", "⚙️"}
    };

    for (const MenuItem &item : menuItems) {
        QPushButton *btn = new QPushButton(QString("  %1   %2").arg(item.icon, item.name));
        btn->setCursor(Qt::PointingHandCursor);
        if (item.name == "Participants") {
            btn->setObjectName("activeBtn");
        } else {
            btn->setObjectName("menuBtn");
        }
        sidebarLayout->addWidget(btn);
    }

    sidebarLayout->addStretch();

    QLabel *sidebarFooter = new QLabel("Des idées aujourd'hui,\nles solutions de demain");
    sidebarFooter->setStyleSheet("color: #64748b; font-style: italic; font-size: 11px; padding-left: 5px;");
    sidebarLayout->addWidget(sidebarFooter);

    mainLayout->addWidget(sidebar);

    // 2. MAIN CONTENT AREA
    QWidget *contentArea = new QWidget();
    contentArea->setObjectName("contentArea");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(25, 20, 25, 20);
    contentLayout->setSpacing(15);

    // Ligne Titre + Boutons PDF & Excel
    QHBoxLayout *topHeaderLayout = new QHBoxLayout();
    QLabel *pageTitle = new QLabel("Gestion des Participants");
    pageTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: #0f172a;");
    topHeaderLayout->addWidget(pageTitle);
    topHeaderLayout->addStretch();

    // Bouton PDF (Tableau Blanc, kbir w msetti fi waset el-page bil-gdè)
    QPushButton *btnPdf = new QPushButton("📥  PDF");
    btnPdf->setObjectName("exportBtn");
    btnPdf->setCursor(Qt::PointingHandCursor);
    connect(btnPdf, &QPushButton::clicked, this, [this]() {
        QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", desktopPath + "/participants.pdf", "Documents PDF (*.pdf)");
        if (!fileName.isEmpty()) {
            QPdfWriter pdfWriter(fileName);
            pdfWriter.setPageSize(QPageSize(QPageSize::A4));
            pdfWriter.setResolution(300);

            QPixmap logoPix = createHackTimeLogoPixmap();
            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            logoPix.save(&buffer, "PNG");
            QString logoBase64 = byteArray.toBase64();

            QTextDocument doc;
            QString html = QString("<html><head><style>") +
                           QString("body { font-family: 'Helvetica', 'Arial', sans-serif; background-color: #0d1b2a; color: #f8fafc; margin: 0; padding: 40px; text-align: center; }") +
                           QString(".header { text-align: center; margin-bottom: 30px; border-bottom: 2px solid #1e293b; padding-bottom: 20px; }") +
                           QString("h1 { color: #ffffff; font-size: 24px; margin: 12px 0 5px 0; }") +
                           QString("p { color: #94a3b8; font-size: 12px; margin: 0; }") +
                           QString(".table-wrapper { width: 100%; display: flex; justify-content: center; align-items: center; margin: 30px 0; }") +
                           QString("table { width: 95%; border-collapse: collapse; background-color: #ffffff; color: #0f172a; border-radius: 8px; overflow: hidden; margin: 0 auto; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }") +
                           QString("th { background-color: #2563eb; color: #ffffff; font-size: 13px; font-weight: bold; padding: 14px 12px; text-align: left; border: 1px solid #1d4ed8; }") +
                           QString("td { font-size: 12px; padding: 12px 12px; color: #0f172a !important; background-color: #ffffff !important; border: 1px solid #cbd5e1; text-align: left; }") +
                           QString("tr:nth-child(even) td { background-color: #f8fafc !important; }") +
                           QString(".footer { text-align: center; margin-top: 40px; font-size: 10px; color: #64748b; border-top: 1px solid #1e293b; padding-top: 15px; }") +
                           QString("</style></head><body>") +
                           QString("<div class=\"header\">") +
                           QString("<img src=\"data:image/png;base64,") + logoBase64 + QString("\" width=\"180\" height=\"49\" style=\"margin-bottom: 10px;\" /><br/>") +
                           QString("<h1>Liste Officielle des Participants</h1>") +
                           QString("<p>Plateforme de Gestion Intelligente des Hackathons - HackTime</p>") +
                           QString("</div>") +
                           QString("<div class=\"table-wrapper\">") +
                           QString("<table>") +
                           QString("<tr>") +
                           QString("<th style=\"width: 10%;\">ID</th>") +
                           QString("<th style=\"width: 22%;\">Nom Complet</th>") +
                           QString("<th style=\"width: 30%;\">E-mail</th>") +
                           QString("<th style=\"width: 14%;\">Université</th>") +
                           QString("<th style=\"width: 14%;\">Compétence</th>") +
                           QString("<th style=\"width: 10%;\">Équipe</th>") +
                           QString("</tr>");

            for (int i = 0; i < table->rowCount(); ++i) {
                html += QString("<tr><td><b>%1</b></td><td>%2</td><td><span style='color:#2563eb; font-weight:bold;'>%3</span></td><td>%4</td><td>%5</td><td>%6</td></tr>")
                .arg(table->item(i, 0) ? table->item(i, 0)->text() : "")
                    .arg(table->item(i, 1) ? table->item(i, 1)->text() : "")
                    .arg(table->item(i, 2) ? table->item(i, 2)->text() : "")
                    .arg(table->item(i, 3) ? table->item(i, 3)->text() : "")
                    .arg(table->item(i, 4) ? table->item(i, 4)->text() : "")
                    .arg(table->item(i, 5) ? table->item(i, 5)->text() : "");
            }

            html += QString("</table>") +
                    QString("</div>") +
                    QString("<div class=\"footer\">") +
                    QString("HackTime Platform &copy; 2026 - Tous droits réservés") +
                    QString("</div>") +
                    QString("</body></html>");

            doc.setHtml(html);
            doc.print(&pdfWriter);

            QMessageBox::information(this, "Succès", "Fichier PDF avec grand tableau blanc centré exporté sur le Bureau !");
        }
    });

    // Bouton Excel
    QPushButton *btnExcel = new QPushButton("📥  Excel");
    btnExcel->setObjectName("exportBtn");
    btnExcel->setCursor(Qt::PointingHandCursor);
    connect(btnExcel, &QPushButton::clicked, this, [this]() {
        QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        QString fileName = QFileDialog::getSaveFileName(this, "Exporter en Excel", desktopPath + "/participants.csv", "Fichiers CSV (*.csv)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << "ID,Nom Complet,E-mail,Université,Compétence,Équipe\n";
                for (int i = 0; i < table->rowCount(); ++i) {
                    out << (table->item(i, 0) ? table->item(i, 0)->text() : "") << ",";
                    out << "\"" << (table->item(i, 1) ? table->item(i, 1)->text() : "") << "\",";
                    out << "\"" << (table->item(i, 2) ? table->item(i, 2)->text() : "") << "\",";
                    out << (table->item(i, 3) ? table->item(i, 3)->text() : "") << ",";
                    out << (table->item(i, 4) ? table->item(i, 4)->text() : "") << ",";
                    out << (table->item(i, 5) ? table->item(i, 5)->text() : "") << "\n";
                }
                file.close();
                QMessageBox::information(this, "Succès", "Fichier Excel (CSV) exporté avec succès sur le Bureau !");
            }
        }
    });

    topHeaderLayout->addWidget(btnPdf);
    topHeaderLayout->addWidget(btnExcel);
    contentLayout->addLayout(topHeaderLayout);

    // STATS CARDS
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);

    QFrame *card1 = new QFrame();
    card1->setObjectName("card");
    QVBoxLayout *c1Layout = new QVBoxLayout(card1);
    QLabel *c1Title = new QLabel("Participants Actifs");
    c1Title->setStyleSheet("color: #475569; font-weight: 600; font-size: 13px;");
    lblActifs = new QLabel("10");
    lblActifs->setStyleSheet("font-size: 28px; font-weight: bold; color: #0f172a;");
    c1Layout->addWidget(c1Title);
    c1Layout->addWidget(lblActifs);
    statsLayout->addWidget(card1);

    QFrame *card2 = new QFrame();
    card2->setObjectName("card");
    QVBoxLayout *c2Layout = new QVBoxLayout(card2);
    QLabel *c2Title = new QLabel("Nouveaux Inscrits");
    c2Title->setStyleSheet("color: #475569; font-weight: 600; font-size: 13px;");
    lblNouveaux = new QLabel("0");
    lblNouveaux->setStyleSheet("font-size: 28px; font-weight: bold; color: #ea580c;");
    c2Layout->addWidget(c2Title);
    c2Layout->addWidget(lblNouveaux);
    statsLayout->addWidget(card2);

    QFrame *card3 = new QFrame();
    card3->setObjectName("card");
    QVBoxLayout *c3Layout = new QVBoxLayout(card3);
    QLabel *c3Title = new QLabel("Progression Équipe");
    c3Title->setStyleSheet("color: #475569; font-weight: 600; font-size: 13px;");

    QPushButton *btnStatsDialog = new QPushButton("📊 Afficher Rapports & Statistiques");
    btnStatsDialog->setStyleSheet("background-color: #2563eb; color: white; padding: 6px 10px; font-weight: bold; border-radius: 6px; font-size: 11px;");
    btnStatsDialog->setCursor(Qt::PointingHandCursor);

    connect(btnStatsDialog, &QPushButton::clicked, this, [this]() {
        QDialog statsDialog(this);
        statsDialog.setWindowTitle("Statistiques de Performance - Équipe Visionary");
        statsDialog.resize(850, 600);

        QVBoxLayout *dlgLayout = new QVBoxLayout(&statsDialog);
        dlgLayout->setContentsMargins(25, 25, 25, 25);
        dlgLayout->setSpacing(15);

        QLabel *dlgTitle = new QLabel("<b>Statistiques de Performance - Équipe Visionary</b>");
        dlgTitle->setStyleSheet("font-size: 18px; color: #0f172a; margin-bottom: 5px;");
        dlgLayout->addWidget(dlgTitle);

        QHBoxLayout *dlgContent = new QHBoxLayout();
        dlgContent->setSpacing(25);

        QVBoxLayout *textStatsLayout = new QVBoxLayout();
        textStatsLayout->setSpacing(18);
        textStatsLayout->addWidget(new QLabel("✅ <b>Projets Terminés:</b> <span style='color:#16a34a; font-size:18px; font-weight:bold;'>2</span>"));
        textStatsLayout->addWidget(new QLabel("⏳ <b>Heures de Mentoring:</b> <span style='color:#2563eb; font-size:18px; font-weight:bold;'>15h</span>"));
        textStatsLayout->addWidget(new QLabel("📝 <b>Retours Participants:</b> <span style='color:#ca8a04; font-size:18px; font-weight:bold;'>4.8/5</span>"));
        textStatsLayout->addWidget(new QLabel("🏆 <b>Équipe en tête:</b> <span style='color:#ea580c; font-size:18px; font-weight:bold;'>Visionary</span>"));
        textStatsLayout->addStretch();
        dlgContent->addLayout(textStatsLayout, 1);

        QVBoxLayout *graphLayout = new QVBoxLayout();
        graphLayout->setSpacing(10);

        ChartWidget *curveChart = new ChartWidget();
        curveChart->setObjectName("curveChart");
        curveChart->setFixedSize(400, 210);

        QLabel *graphTitle2 = new QLabel("Statut des Projets (Done vs In Progress)");
        graphTitle2->setStyleSheet("font-weight: bold; font-size: 12px; color: #475569; margin-top: 5px;");
        ChartWidget *barChart = new ChartWidget();
        barChart->setObjectName("barChart");
        barChart->setFixedSize(400, 180);

        graphLayout->addWidget(curveChart);
        graphLayout->addWidget(graphTitle2);
        graphLayout->addWidget(barChart);

        dlgContent->addLayout(graphLayout, 1);
        dlgContent->addStretch();

        dlgLayout->addLayout(dlgContent);

        QPushButton *closeBtn = new QPushButton("Fermer", &statsDialog);
        closeBtn->setStyleSheet("background-color: #64748b; color: white; padding: 8px 22px; font-weight: bold; border-radius: 6px; font-size: 13px;");
        closeBtn->setCursor(Qt::PointingHandCursor);
        connect(closeBtn, &QPushButton::clicked, &statsDialog, &QDialog::accept);

        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->addStretch();
        btnLayout->addWidget(closeBtn);
        dlgLayout->addLayout(btnLayout);

        statsDialog.exec();
    });

    c3Layout->addWidget(c3Title);
    c3Layout->addWidget(btnStatsDialog);
    statsLayout->addWidget(card3);

    contentLayout->addLayout(statsLayout);

    // MIDDLE SECTION
    QHBoxLayout *middleLayout = new QHBoxLayout();
    middleLayout->setSpacing(20);

    QFrame *tableCard = new QFrame();
    tableCard->setObjectName("card");
    QVBoxLayout *tcLayout = new QVBoxLayout(tableCard);

    QHBoxLayout *tableHeader = new QHBoxLayout();
    QLabel *tableTitle = new QLabel("Liste des Participants");
    tableTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #0f172a;");

    // Champ de recherche
    QLineEdit *searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Rechercher un participant...");
    searchBox->setFixedWidth(240);
    searchBox->setStyleSheet("padding: 6px 10px; border: 1px solid #cbd5e1; border-radius: 6px; background-color: #f8fafc; font-size: 12px;");

    // Bouton de Tri Alphabétique
    QPushButton *sortBtn = new QPushButton("🔤 Tri A-Z");
    sortBtn->setObjectName("exportBtn");
    sortBtn->setCursor(Qt::PointingHandCursor);

    QPushButton *addBtn = new QPushButton("+ Ajouter un Participant");
    addBtn->setObjectName("primaryBtn");
    addBtn->setCursor(Qt::PointingHandCursor);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAjouterParticipantClicked);

    tableHeader->addWidget(tableTitle);
    tableHeader->addStretch();
    tableHeader->addWidget(searchBox);
    tableHeader->addWidget(sortBtn);
    tableHeader->addWidget(addBtn);
    tcLayout->addLayout(tableHeader);

    // TABLE
    table = new QTableWidget(5, 8);
    table->setHorizontalHeaderLabels({"ID", "Nom Comple", "E-mail", "Université", "Compétence", "Équipe", "Statut", "Actions"});

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);
    table->setColumnWidth(6, 120);
    table->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);

    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(45);
    table->setShowGrid(false);

    for(int i = 0; i < 5; ++i) {
        table->setItem(i, 0, new QTableWidgetItem(QString("PA-00%1").arg(i+1)));
        table->setItem(i, 1, new QTableWidgetItem(i == 2 ? "Sorah Benali" : "Sarah Benali"));
        table->setItem(i, 2, new QTableWidgetItem(i == 2 ? "sarah.benali@esprit.tn" : "sarah.b@esprit.tn"));
        table->setItem(i, 3, new QTableWidgetItem("INSAT"));
        table->setItem(i, 4, new QTableWidgetItem("Dev Full-Stack"));
        table->setItem(i, 5, new QTableWidgetItem("Team Visionary"));

        QLabel *status = new QLabel(i < 2 ? "Done" : "In Progress");
        status->setAlignment(Qt::AlignCenter);
        if(i < 2) {
            status->setStyleSheet("QLabel { background-color: #dcfce7; color: #166534; border-radius: 12px; font-weight: bold; font-size: 11px; padding: 0px 4px; }");
        } else {
            status->setStyleSheet("QLabel { background-color: #fef3c7; color: #9a3412; border-radius: 12px; font-weight: bold; font-size: 10px; padding: 0px 2px; }");
        }

        QWidget *statusContainer = new QWidget();
        QHBoxLayout *statusLayout = new QHBoxLayout(statusContainer);
        statusLayout->setContentsMargins(2, 2, 2, 2);
        statusLayout->setAlignment(Qt::AlignCenter);
        status->setFixedSize(95, 24);
        statusLayout->addWidget(status);

        table->setCellWidget(i, 6, statusContainer);
        table->setCellWidget(i, 7, createActionButtonsWidget(i));
    }

    // Recherche en direct
    connect(searchBox, &QLineEdit::textChanged, this, [this](const QString &text) {
        for (int i = 0; i < table->rowCount(); ++i) {
            bool match = false;
            for (int j = 0; j < table->columnCount() - 1; ++j) {
                QTableWidgetItem *item = table->item(i, j);
                if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
            table->setRowHidden(i, !match);
        }
    });

    // Tri alphabétique sur TOUS les participants
    connect(sortBtn, &QPushButton::clicked, this, [this]() {
        static bool asc = true;
        int rowCount = table->rowCount();

        for(int i = 0; i < rowCount; ++i) table->setRowHidden(i, false);

        for(int i = 0; i < rowCount - 1; ++i) {
            for(int j = i + 1; j < rowCount; ++j) {
                QTableWidgetItem *item1 = table->item(i, 1);
                QTableWidgetItem *item2 = table->item(j, 1);
                QString t1 = item1 ? item1->text() : "";
                QString t2 = item2 ? item2->text() : "";

                bool condition = asc ? (t1.compare(t2, Qt::CaseInsensitive) > 0)
                                     : (t1.compare(t2, Qt::CaseInsensitive) < 0);
                if(condition) {
                    for(int col = 0; col < 6; ++col) {
                        QTableWidgetItem *temp = table->takeItem(i, col);
                        table->setItem(i, col, table->takeItem(j, col));
                        table->setItem(j, col, temp);
                    }
                }
            }
        }
        for(int i = 0; i < rowCount; ++i) {
            bool isDone = (i < 2);
            QLabel *status = new QLabel(isDone ? "Done" : "In Progress");
            status->setAlignment(Qt::AlignCenter);
            if(isDone) {
                status->setStyleSheet("QLabel { background-color: #dcfce7; color: #166534; border-radius: 12px; font-weight: bold; font-size: 11px; padding: 0px 4px; }");
            } else {
                status->setStyleSheet("QLabel { background-color: #fef3c7; color: #9a3412; border-radius: 12px; font-weight: bold; font-size: 10px; padding: 0px 2px; }");
            }
            QWidget *statusContainer = new QWidget();
            QHBoxLayout *statusLayout = new QHBoxLayout(statusContainer);
            statusLayout->setContentsMargins(2, 2, 2, 2);
            statusLayout->setAlignment(Qt::AlignCenter);
            status->setFixedSize(95, 24);
            statusLayout->addWidget(status);

            table->setCellWidget(i, 6, statusContainer);
            table->setCellWidget(i, 7, createActionButtonsWidget(i));
        }

        asc = !asc;
        showPage(currentPage);
    });

    tcLayout->addWidget(table);

    // PAGINATION
    paginationLayout = new QHBoxLayout();
    paginationLayout->addStretch();
    tcLayout->addLayout(paginationLayout);

    middleLayout->addWidget(tableCard, 3);

    // ACTIONS RAPIDES
    QFrame *actionsCard = new QFrame();
    actionsCard->setObjectName("card");
    QVBoxLayout *acLayout = new QVBoxLayout(actionsCard);

    QLabel *actionsTitle = new QLabel("Actions rapides");
    actionsTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #0f172a; margin-bottom: 10px;");
    acLayout->addWidget(actionsTitle);

    const QStringList quickActions = {
        "👥  Affecter à une équipe  →",
        "✉️  Envoyer invitation mentor  →",
        "📩  Rappeler aux retardataires  →",
        "🎯  Valider statut projet  →",
        "📊  Générer rapport de progression  →",
        "📜  Exporter Certificat de Participation  →"
    };

    for(const QString &act : quickActions) {
        QPushButton *actBtn = new QPushButton(act);
        actBtn->setObjectName("actionBtn");
        actBtn->setCursor(Qt::PointingHandCursor);
        connect(actBtn, &QPushButton::clicked, this, &MainWindow::onActionRapideClicked);
        acLayout->addWidget(actBtn);
    }
    acLayout->addStretch();

    middleLayout->addWidget(actionsCard, 1);
    contentLayout->addLayout(middleLayout);

    mainLayout->addWidget(contentArea);

    this->setCentralWidget(centralWidget);
    updateCounters();
    showPage(1);
}

void MainWindow::showPage(int page)
{
    if (!table) return;

    int totalRows = table->rowCount();
    int totalPages = (totalRows + itemsPerPage - 1) / itemsPerPage;
    if (totalPages == 0) totalPages = 1;

    if (page < 1) page = 1;
    if (page > totalPages) page = totalPages;

    currentPage = page;

    int startRow = (currentPage - 1) * itemsPerPage;
    int endRow = startRow + itemsPerPage;

    for (int i = 0; i < totalRows; ++i) {
        if (i >= startRow && i < endRow) {
            table->setRowHidden(i, false);
        } else {
            table->setRowHidden(i, true);
        }
    }

    updatePagination();
}

void MainWindow::updatePagination()
{
    if (!paginationLayout) return;

    QLayoutItem *child;
    while ((child = paginationLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    paginationLayout->addStretch();

    int totalRows = table ? table->rowCount() : 0;
    int totalPages = (totalRows + itemsPerPage - 1) / itemsPerPage;
    if (totalPages == 0) totalPages = 1;

    QPushButton *btnPrev = new QPushButton("<");
    btnPrev->setCursor(Qt::PointingHandCursor);
    btnPrev->setObjectName("pageBtn");
    btnPrev->setEnabled(currentPage > 1);
    connect(btnPrev, &QPushButton::clicked, this, &MainWindow::onPrevPageClicked);
    paginationLayout->addWidget(btnPrev);

    for (int i = 1; i <= totalPages; ++i) {
        QPushButton *btn = new QPushButton(QString::number(i));
        btn->setCursor(Qt::PointingHandCursor);

        if (i == currentPage) {
            btn->setObjectName("pageActive");
        } else {
            btn->setObjectName("pageBtn");
        }

        connect(btn, &QPushButton::clicked, this, &MainWindow::onPageClicked);
        paginationLayout->addWidget(btn);
    }

    QPushButton *btnNext = new QPushButton(">");
    btnNext->setCursor(Qt::PointingHandCursor);
    btnNext->setObjectName("pageBtn");
    btnNext->setEnabled(currentPage < totalPages);
    connect(btnNext, &QPushButton::clicked, this, &MainWindow::onNextPageClicked);
    paginationLayout->addWidget(btnNext);

    applyStyleSheet();
}

void MainWindow::onPageClicked()
{
    QPushButton *clickedBtn = qobject_cast<QPushButton*>(sender());
    if (!clickedBtn) return;

    bool ok;
    int page = clickedBtn->text().toInt(&ok);
    if (ok) {
        showPage(page);
    }
}

void MainWindow::onPrevPageClicked()
{
    if (currentPage > 1) {
        showPage(currentPage - 1);
    }
}

void MainWindow::onNextPageClicked()
{
    int totalRows = table ? table->rowCount() : 0;
    int totalPages = (totalRows + itemsPerPage - 1) / itemsPerPage;
    if (currentPage < totalPages) {
        showPage(currentPage + 1);
    }
}

void MainWindow::onAjouterParticipantClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Ajouter un Participant");
    dialog.resize(400, 300);

    QFormLayout form(&dialog);

    QLineEdit *nomInput = new QLineEdit(&dialog);
    QLineEdit *emailInput = new QLineEdit(&dialog);
    QLineEdit *univInput = new QLineEdit(&dialog);
    QLineEdit *compInput = new QLineEdit(&dialog);

    form.addRow("Nom Complet :", nomInput);
    form.addRow("E-mail :", emailInput);
    form.addRow("Université :", univInput);
    form.addRow("Compétence :", compInput);

    QPushButton *submitBtn = new QPushButton("Ajouter", &dialog);
    submitBtn->setStyleSheet("background-color: #2563eb; color: white; padding: 8px; font-weight: bold; border-radius: 6px;");
    form.addWidget(submitBtn);

    connect(submitBtn, &QPushButton::clicked, this, [this, &dialog, nomInput, emailInput, univInput, compInput]() {
        if(nomInput->text().isEmpty() || emailInput->text().isEmpty()) {
            QMessageBox::warning(&dialog, "Attention", "Veuillez remplir au moins le nom et l'email !");
            return;
        }

        int rowCount = table->rowCount();
        table->insertRow(rowCount);

        table->setItem(rowCount, 0, new QTableWidgetItem(QString("PA-00%1").arg(rowCount + 1)));
        table->setItem(rowCount, 1, new QTableWidgetItem(nomInput->text()));
        table->setItem(rowCount, 2, new QTableWidgetItem(emailInput->text()));
        table->setItem(rowCount, 3, new QTableWidgetItem(univInput->text().isEmpty() ? "INSAT" : univInput->text()));
        table->setItem(rowCount, 4, new QTableWidgetItem(compInput->text().isEmpty() ? "Dev" : compInput->text()));
        table->setItem(rowCount, 5, new QTableWidgetItem("Non affecté"));

        QLabel *status = new QLabel("In Progress");
        status->setAlignment(Qt::AlignCenter);
        status->setStyleSheet("QLabel { background-color: #fef3c7; color: #9a3412; border-radius: 12px; font-weight: bold; font-size: 10px; padding: 0px 2px; }");

        QWidget *statusContainer = new QWidget();
        QHBoxLayout *statusLayout = new QHBoxLayout(statusContainer);
        statusLayout->setContentsMargins(2, 2, 2, 2);
        statusLayout->setAlignment(Qt::AlignCenter);
        status->setFixedSize(95, 24);
        statusLayout->addWidget(status);

        table->setCellWidget(rowCount, 6, statusContainer);
        table->setCellWidget(rowCount, 7, createActionButtonsWidget(rowCount));

        nouveauxInscritsCount++;
        updateCounters();

        int totalPages = (table->rowCount() + itemsPerPage - 1) / itemsPerPage;
        showPage(totalPages);

        QMessageBox::information(this, "Succès", "Participant ajouté avec succès !");
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onVoirParticipantClicked(int row)
{
    if (!table || row < 0 || row >= table->rowCount()) return;

    QString id = table->item(row, 0) ? table->item(row, 0)->text() : "";
    QString nom = table->item(row, 1) ? table->item(row, 1)->text() : "";
    QString email = table->item(row, 2) ? table->item(row, 2)->text() : "";
    QString univ = table->item(row, 3) ? table->item(row, 3)->text() : "";
    QString comp = table->item(row, 4) ? table->item(row, 4)->text() : "";
    QString equipe = table->item(row, 5) ? table->item(row, 5)->text() : "";

    QString details = QString("<b>ID:</b> %1<br>"
                              "<b>Nom:</b> %2<br>"
                              "<b>Email:</b> %3<br>"
                              "<b>Université:</b> %4<br>"
                              "<b>Compétence:</b> %5<br>"
                              "<b>Équipe:</b> %6")
                          .arg(id, nom, email, univ, comp, equipe);

    QMessageBox::information(this, "Détails du Participant", details);
}

void MainWindow::onModifierParticipantClicked(int row)
{
    if (!table || row < 0 || row >= table->rowCount()) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Modifier le Participant");
    dialog.resize(400, 300);

    QFormLayout form(&dialog);

    QLineEdit *nomInput = new QLineEdit(table->item(row, 1) ? table->item(row, 1)->text() : "", &dialog);
    QLineEdit *emailInput = new QLineEdit(table->item(row, 2) ? table->item(row, 2)->text() : "", &dialog);
    QLineEdit *univInput = new QLineEdit(table->item(row, 3) ? table->item(row, 3)->text() : "", &dialog);
    QLineEdit *compInput = new QLineEdit(table->item(row, 4) ? table->item(row, 4)->text() : "", &dialog);

    form.addRow("Nom Complet :", nomInput);
    form.addRow("E-mail :", emailInput);
    form.addRow("Université :", univInput);
    form.addRow("Compétence :", compInput);

    QPushButton *submitBtn = new QPushButton("Enregistrer", &dialog);
    submitBtn->setStyleSheet("background-color: #2563eb; color: white; padding: 8px; font-weight: bold; border-radius: 6px;");
    form.addWidget(submitBtn);

    connect(submitBtn, &QPushButton::clicked, this, [this, &dialog, row, nomInput, emailInput, univInput, compInput]() {
        if(table->item(row, 1)) table->item(row, 1)->setText(nomInput->text());
        if(table->item(row, 2)) table->item(row, 2)->setText(emailInput->text());
        if(table->item(row, 3)) table->item(row, 3)->setText(univInput->text());
        if(table->item(row, 4)) table->item(row, 4)->setText(compInput->text());

        QMessageBox::information(this, "Succès", "Informations mises à jour !");
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onSupprimerParticipantClicked(int row)
{
    if (!table || row < 0 || row >= table->rowCount()) return;

    QString nom = table->item(row, 1) ? table->item(row, 1)->text() : "";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  QString("Voulez-vous vraiment supprimer %1 ?").arg(nom),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        table->removeRow(row);
        updateCounters();

        for(int i = 0; i < table->rowCount(); ++i) {
            table->setCellWidget(i, 7, createActionButtonsWidget(i));
        }

        showPage(currentPage);

        QMessageBox::information(this, "Supprimé", "Participant supprimé avec succès !");
    }
}

void MainWindow::onActionRapideClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString text = btn->text();

    if(text.contains("Affecter à une équipe")) {
        QMessageBox::information(this, "Action", "Action: Affecter à une équipe");
    }
    else if(text.contains("Envoyer invitation mentor")) {
        QMessageBox::information(this, "Action", "Action: Envoyer invitation mentor");
    }
    else if(text.contains("Rappeler aux retardataires")) {
        QMessageBox::information(this, "Action", "Action: Rappeler aux retardataires");
    }
    else if(text.contains("Valider statut projet")) {
        QMessageBox::information(this, "Action", "Action: Valider statut projet");
    }
    else if(text.contains("Générer rapport")) {
        QMessageBox::information(this, "Action", "Action: Générer rapport");
    }
    else if(text.contains("Exporter Certificat")) {
        QMessageBox::information(this, "Action", "Exporter Certificat");
    }
}

void MainWindow::applyStyleSheet()
{
    QString style = R"(
        QMainWindow { background-color: #f8fafc; }
        #sidebar { background-color: #0d1b2a; }
        #menuBtn { background: transparent; color: #94a3b8; text-align: left; padding: 10px 15px; border: none; border-radius: 8px; font-size: 13px; font-weight: 500; }
        #menuBtn:hover { background-color: #1e293b; color: #f8fafc; }
        #activeBtn { background-color: #2563eb; color: white; text-align: left; padding: 10px 15px; border-radius: 8px; font-size: 13px; font-weight: bold; }
        #card { background-color: #ffffff; border-radius: 12px; padding: 15px; border: 1px solid #e2e8f0; }
        #primaryBtn { background-color: #2563eb; color: white; padding: 8px 16px; border-radius: 8px; font-weight: bold; border: none; }
        #primaryBtn:hover { background-color: #1d4ed8; }
        #exportBtn { background-color: #ffffff; border: 1px solid #cbd5e1; padding: 6px 14px; border-radius: 6px; color: #0f172a; font-weight: 600; font-size: 12px; }
        #exportBtn:hover { background-color: #f1f5f9; border-color: #94a3b8; }
        #actionBtn { background-color: #f8fafc; border: 1px solid #e2e8f0; padding: 10px 12px; border-radius: 8px; text-align: left; color: #334155; font-size: 12px; font-weight: 500; }
        #actionBtn:hover { background-color: #f1f5f9; border-color: #cbd5e1; }
        #pageBtn { background-color: #f1f5f9; border: 1px solid #e2e8f0; border-radius: 6px; padding: 5px 10px; color: #475569; }
        #pageBtn:disabled { background-color: #f8fafc; color: #cbd5e1; border-color: #f1f5f9; }
        #pageActive { background-color: #2563eb; color: white; border-radius: 6px; padding: 5px 10px; font-weight: bold; border: none; }
        QTableWidget { border: none; background: transparent; gridline-color: transparent; }
        QTableWidget::item { padding: 8px; color: #334155; border-bottom: 1px solid #f1f5f9; }
        QHeaderView::section { background-color: #f8fafc; border: none; padding: 8px; font-weight: bold; color: #64748b; font-size: 12px; }
    )";

    this->setStyleSheet(style);
}