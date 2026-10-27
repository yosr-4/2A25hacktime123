#include "ateliermainwindow.h"
#include "ui_ateliermainwindow.h"

#include <QAbstractItemView>
#include <QDate>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPrinter>
#include <QSet>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QTextDocument>
#include <algorithm>

// ------------------------------------------------------------------ helpers
static QColor statutColor(Atelier::Statut s)
{
    switch (s) {
    case Atelier::AVenir:  return QColor("#1665E8");
    case Atelier::EnCours: return QColor("#16A34A");
    case Atelier::Termine: return QColor("#94A3B8");
    }
    return QColor("#1B2A4A");
}

static QString badgeStyle(Atelier::Statut s)
{
    switch (s) {
    case Atelier::AVenir:  return "background:#E8F0FE;color:#1665E8;";
    case Atelier::EnCours: return "background:#E6F7EE;color:#16A34A;";
    case Atelier::Termine: return "background:#EEF2F8;color:#64748B;";
    }
    return QString();
}

// ------------------------------------------------------------------ ctor
AtelierMainWindow::AtelierMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AtelierMainWindow)
{
    ui->setupUi(this);

    // Table : largeurs de colonnes
    QHeaderView *h = ui->table->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::Interactive);
    h->setSectionResizeMode(1, QHeaderView::Stretch);   // Titre
    h->setSectionResizeMode(2, QHeaderView::Stretch);   // Compétition
    ui->table->setColumnWidth(0, 70);
    ui->table->setColumnWidth(3, 130);
    ui->table->setColumnWidth(4, 80);
    ui->table->setColumnWidth(5, 100);
    ui->table->setColumnWidth(6, 80);
    h->setSectionResizeMode(7, QHeaderView::Fixed);
    ui->table->setColumnWidth(7, 100);
    h->setStretchLastSection(false);

    // Filtres
    ui->fStatut->addItem("Tous", -1);
    ui->fStatut->addItem("À venir", Atelier::AVenir);
    ui->fStatut->addItem("En cours", Atelier::EnCours);
    ui->fStatut->addItem("Terminé", Atelier::Termine);
    ui->fSort->addItems({"Date (récent d'abord)", "Date (ancien d'abord)", "Titre (A → Z)",
                         "Capacité (décroissante)", "Compétition"});

    ui->aDate->setDateTime(QDateTime(QDate::currentDate(), QTime(10, 0)));

    // Connexions
    connect(ui->search, &QLineEdit::textChanged, this, &AtelierMainWindow::applyFilters);
    connect(ui->topSearch, &QLineEdit::textChanged, ui->search, &QLineEdit::setText);
    connect(ui->fCompetition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AtelierMainWindow::applyFilters);
    connect(ui->fStatut, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AtelierMainWindow::applyFilters);
    connect(ui->fDate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AtelierMainWindow::applyFilters);
    connect(ui->fSort, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AtelierMainWindow::applyFilters);

    connect(ui->table, &QTableWidget::itemSelectionChanged, this, &AtelierMainWindow::onSelectionChanged);
    connect(ui->table, &QTableWidget::cellDoubleClicked, this, [this](int, int) { onEdit(); });

    connect(ui->btnSave, &QPushButton::clicked, this, &AtelierMainWindow::onSave);
    connect(ui->btnClear, &QPushButton::clicked, this, &AtelierMainWindow::onClear);
    connect(ui->btnEdit, &QPushButton::clicked, this, &AtelierMainWindow::onEdit);
    connect(ui->btnDelete, &QPushButton::clicked, this, &AtelierMainWindow::onDelete);
    connect(ui->btnDetails, &QPushButton::clicked, this, &AtelierMainWindow::onDetails);
    connect(ui->btnDuplicate, &QPushButton::clicked, this, &AtelierMainWindow::onDuplicate);
    connect(ui->btnProchain, &QPushButton::clicked, this, &AtelierMainWindow::onProchain);
    connect(ui->btnReset, &QPushButton::clicked, this, &AtelierMainWindow::onResetFilters);
    connect(ui->btnPrev, &QPushButton::clicked, this, &AtelierMainWindow::onPrevPage);
    connect(ui->btnNext, &QPushButton::clicked, this, &AtelierMainWindow::onNextPage);
    connect(ui->btnExportPdf, &QPushButton::clicked, this, &AtelierMainWindow::onExportPdf);
    connect(ui->btnExportExcel, &QPushButton::clicked, this, &AtelierMainWindow::onExportCsv);

    for (QPushButton *b : {ui->btnDashboard, ui->btnCompetitions, ui->btnEquipes,
                           ui->btnParticipants, ui->btnEmployes, ui->btnParametres})
        connect(b, &QPushButton::clicked, this, &AtelierMainWindow::onOtherModule);
    connect(ui->btnAtelier, &QPushButton::clicked, this, [this] { statusBar()->showMessage("Module Atelier", 2000); });

    loadCompetitions();
    reload();
    fillForm(Atelier(), false);
}

AtelierMainWindow::~AtelierMainWindow() { delete ui; }

// ------------------------------------------------------------------ data
void AtelierMainWindow::loadCompetitions()
{
    const QVariant prevForm = ui->aComp->currentData();
    const QVariant prevFilter = ui->fCompetition->currentData();
    QSignalBlocker b1(ui->aComp), b2(ui->fCompetition);
    ui->aComp->clear();
    ui->fCompetition->clear();
    ui->fCompetition->addItem("Toutes", 0);

    QString err;
    const auto comps = AtelierDao::competitions(&err);
    if (comps.isEmpty())
        ui->aComp->addItem("— Aucune compétition —", 0);
    for (const auto &c : comps) {
        ui->aComp->addItem(c.second, c.first);
        ui->fCompetition->addItem(c.second, c.first);
    }
    int i = ui->aComp->findData(prevForm);
    if (i >= 0) ui->aComp->setCurrentIndex(i);
    i = ui->fCompetition->findData(prevFilter);
    if (i >= 0) ui->fCompetition->setCurrentIndex(i);
}

void AtelierMainWindow::reload(int selectId)
{
    QString err;
    m_all = AtelierDao::all(&err);
    if (!err.isEmpty())
        QMessageBox::warning(this, "Base de données", "Lecture des ateliers impossible :\n" + err);

    rebuildYearFilter();
    updateKpis();
    applyFilters();
    if (selectId > 0) selectRowById(selectId);
}

void AtelierMainWindow::rebuildYearFilter()
{
    const QVariant prev = ui->fDate->currentData();
    QSignalBlocker b(ui->fDate);
    ui->fDate->clear();
    ui->fDate->addItem("Toutes", 0);
    QSet<int> years;
    for (const Atelier &a : std::as_const(m_all))
        if (a.date.isValid()) years.insert(a.date.date().year());
    QList<int> ys = years.values();
    std::sort(ys.begin(), ys.end(), std::greater<int>());
    for (int y : std::as_const(ys)) ui->fDate->addItem(QString::number(y), y);
    const int i = ui->fDate->findData(prev);
    if (i >= 0) ui->fDate->setCurrentIndex(i);
}

void AtelierMainWindow::applyFilters()
{
    const QString q = ui->search->text().trimmed();
    const int comp = ui->fCompetition->currentData().toInt();
    const int st = ui->fStatut->currentData().isValid() ? ui->fStatut->currentData().toInt() : -1;
    const int year = ui->fDate->currentData().toInt();

    m_filtered.clear();
    for (const Atelier &a : std::as_const(m_all)) {
        if (comp > 0 && a.idCompetition != comp) continue;
        if (st >= 0 && int(a.statut()) != st) continue;
        if (year > 0 && a.date.date().year() != year) continue;
        if (!q.isEmpty()
            && !a.titre.contains(q, Qt::CaseInsensitive)
            && !a.salle.contains(q, Qt::CaseInsensitive)
            && !a.description.contains(q, Qt::CaseInsensitive)
            && !a.competitionNom.contains(q, Qt::CaseInsensitive)
            && !a.code().contains(q, Qt::CaseInsensitive))
            continue;
        m_filtered << a;
    }

    switch (ui->fSort->currentIndex()) {
    case 0: std::sort(m_filtered.begin(), m_filtered.end(), [](const Atelier &x, const Atelier &y) { return x.date > y.date; }); break;
    case 1: std::sort(m_filtered.begin(), m_filtered.end(), [](const Atelier &x, const Atelier &y) { return x.date < y.date; }); break;
    case 2: std::sort(m_filtered.begin(), m_filtered.end(), [](const Atelier &x, const Atelier &y) { return x.titre.localeAwareCompare(y.titre) < 0; }); break;
    case 3: std::sort(m_filtered.begin(), m_filtered.end(), [](const Atelier &x, const Atelier &y) { return x.capacite > y.capacite; }); break;
    case 4: std::sort(m_filtered.begin(), m_filtered.end(), [](const Atelier &x, const Atelier &y) { return x.competitionNom.localeAwareCompare(y.competitionNom) < 0; }); break;
    }

    m_page = 0;
    fillTable();
    updateStats();
}

void AtelierMainWindow::fillTable()
{
    const int selectedId = selectedAtelier() ? selectedAtelier()->id : 0;

    const int total = m_filtered.size();
    const int pages = std::max(1, (total + PAGE_SIZE - 1) / PAGE_SIZE);
    m_page = std::clamp(m_page, 0, pages - 1);
    const int start = m_page * PAGE_SIZE;
    const int end = std::min(total, start + PAGE_SIZE);

    QSignalBlocker blocker(ui->table);
    ui->table->clearContents();
    ui->table->setRowCount(end - start);

    for (int r = 0; r < end - start; ++r) {
        const Atelier &a = m_filtered.at(start + r);
        const Atelier::Statut st = a.statut();

        auto mk = [](const QString &t, Qt::Alignment al = Qt::AlignVCenter | Qt::AlignLeft) {
            auto *it = new QTableWidgetItem(t);
            it->setTextAlignment(al);
            return it;
        };
        QTableWidgetItem *idIt = mk(a.code());
        idIt->setData(Qt::UserRole, a.id);
        idIt->setForeground(QColor("#6B7A99"));
        QTableWidgetItem *titleIt = mk(a.titre);
        QFont bold = titleIt->font(); bold.setBold(true); titleIt->setFont(bold);
        QTableWidgetItem *stIt = mk(Atelier::statutText(st), Qt::AlignCenter);
        stIt->setForeground(statutColor(st));
        QFont sf = stIt->font(); sf.setBold(true); stIt->setFont(sf);

        ui->table->setItem(r, 0, idIt);
        ui->table->setItem(r, 1, titleIt);
        ui->table->setItem(r, 2, mk(a.competitionNom));
        ui->table->setItem(r, 3, mk(a.date.toString("dd/MM/yyyy HH:mm")));
        ui->table->setItem(r, 4, mk(Atelier::dureeText(a.duree)));
        ui->table->setItem(r, 5, mk(a.salle));
        ui->table->setItem(r, 6, mk(QString::number(a.capacite), Qt::AlignCenter));
        ui->table->setItem(r, 7, stIt);
    }
    blocker.unblock();

    ui->footer->setText(total == 0
        ? QString("Aucun atelier à afficher")
        : QString("Affichage de %1 à %2 sur %3 ateliers").arg(start + 1).arg(end).arg(total));
    ui->lblPage->setText(QString("Page %1 / %2").arg(m_page + 1).arg(pages));
    ui->btnPrev->setEnabled(m_page > 0);
    ui->btnNext->setEnabled(m_page < pages - 1);

    if (selectedId > 0) selectRowById(selectedId);
    else onSelectionChanged();
}

void AtelierMainWindow::updateKpis()
{
    int cap = 0, avenir = 0;
    QSet<int> comps;
    for (const Atelier &a : std::as_const(m_all)) {
        cap += a.capacite;
        comps.insert(a.idCompetition);
        if (a.statut() == Atelier::AVenir) ++avenir;
    }
    ui->kpiTotal->setText(QString::number(m_all.size()));
    ui->kpiCapacite->setText(QString::number(cap));
    ui->kpiComps->setText(QString::number(comps.size()));
    ui->kpiAvenir->setText(QString::number(avenir));
}

void AtelierMainWindow::updateStats()
{
    int year = ui->fDate->currentData().toInt();
    if (year <= 0) year = QDate::currentDate().year();

    QVector<int> months(12, 0);
    int nA = 0, nC = 0, nT = 0, cap = 0;
    for (const Atelier &a : std::as_const(m_filtered)) {
        if (a.date.isValid() && a.date.date().year() == year)
            months[a.date.date().month() - 1]++;
        switch (a.statut()) {
        case Atelier::AVenir:  ++nA; break;
        case Atelier::EnCours: ++nC; break;
        case Atelier::Termine: ++nT; break;
        }
        cap += a.capacite;
    }
    ui->statsChart->setValues(months);
    ui->lblStats->setText(QString("📈  Ateliers par mois – %1").arg(year));
    ui->legAvenir->setText(QString::number(nA));
    ui->legCours->setText(QString::number(nC));
    ui->legTermine->setText(QString::number(nT));
    ui->lblMoyenne->setText(m_filtered.isEmpty()
        ? QString("Capacité moyenne : -")
        : QString("Capacité moyenne : %1 places").arg(qRound(double(cap) / m_filtered.size())));
}

// ------------------------------------------------------------------ selection / details
const Atelier *AtelierMainWindow::selectedAtelier() const
{
    const int row = ui->table->currentRow();
    if (row < 0 || !ui->table->item(row, 0) || !ui->table->isEnabled()) return nullptr;
    const int id = ui->table->item(row, 0)->data(Qt::UserRole).toInt();
    for (const Atelier &a : m_all)
        if (a.id == id) return &a;
    return nullptr;
}

void AtelierMainWindow::selectRowById(int id)
{
    for (int r = 0; r < ui->table->rowCount(); ++r) {
        if (ui->table->item(r, 0) && ui->table->item(r, 0)->data(Qt::UserRole).toInt() == id) {
            ui->table->selectRow(r);
            ui->table->scrollToItem(ui->table->item(r, 0));
            return;
        }
    }
    onSelectionChanged();
}

void AtelierMainWindow::onSelectionChanged()
{
    const bool has = ui->table->selectionModel() && ui->table->selectionModel()->hasSelection();
    updateDetails(has ? selectedAtelier() : nullptr);
}

void AtelierMainWindow::updateDetails(const Atelier *a)
{
    const bool ok = a != nullptr;
    ui->btnEdit->setEnabled(ok);
    ui->btnDelete->setEnabled(ok);
    ui->btnDetails->setEnabled(ok);
    ui->btnDuplicate->setEnabled(ok);

    if (!ok) {
        for (QLabel *l : {ui->dNom, ui->dId, ui->dBadge, ui->dComp, ui->dDate, ui->dDuree,
                          ui->dSalle, ui->dCap, ui->dDesc})
            l->setText("-");
        ui->dBadge->setStyleSheet(QString());
        return;
    }
    const Atelier::Statut st = a->statut();
    ui->dNom->setText(a->titre);
    ui->dId->setText(QString("%1  •  %2").arg(a->code(), a->competitionNom));
    ui->dBadge->setText(Atelier::statutText(st));
    ui->dBadge->setStyleSheet(badgeStyle(st));
    ui->dComp->setText(a->competitionNom.isEmpty() ? "-" : a->competitionNom);
    ui->dDate->setText(a->date.toString("dd/MM/yyyy à HH:mm"));
    ui->dDuree->setText(Atelier::dureeText(a->duree));
    ui->dSalle->setText(a->salle.isEmpty() ? "-" : a->salle);
    ui->dCap->setText(QString("%1 places").arg(a->capacite));
    ui->dDesc->setText(a->description.isEmpty() ? "-" : a->description);
}

void AtelierMainWindow::onDetails()
{
    const Atelier *a = selectedAtelier();
    if (!a) return;
    QMessageBox::information(this, a->titre,
        QString("<b>%1</b> (%2)<br><br>"
                "<b>Compétition :</b> %3<br>"
                "<b>Début :</b> %4<br>"
                "<b>Fin :</b> %5<br>"
                "<b>Durée :</b> %6<br>"
                "<b>Salle :</b> %7<br>"
                "<b>Capacité :</b> %8 places<br>"
                "<b>Statut :</b> %9<br><br>%10")
            .arg(a->titre.toHtmlEscaped(), a->code(), a->competitionNom.toHtmlEscaped(),
                 a->date.toString("dd/MM/yyyy HH:mm"), a->fin().toString("dd/MM/yyyy HH:mm"),
                 Atelier::dureeText(a->duree), a->salle.toHtmlEscaped())
            .arg(a->capacite)
            .arg(Atelier::statutText(a->statut()), a->description.toHtmlEscaped()));
}

// ------------------------------------------------------------------ form
void AtelierMainWindow::fillForm(const Atelier &a, bool editing)
{
    m_editingId = editing ? a.id : 0;
    ui->aErr->clear();
    ui->aConflict->clear();

    ui->lblAdd->setText(editing ? "✏️  Modifier l'atelier" : "＋  Ajouter un atelier");
    ui->btnSave->setText(editing ? "Mettre à jour" : "Enregistrer");
    ui->aId->setText(editing ? a.code() : QString("A-%1").arg(AtelierDao::nextId(), 3, 10, QChar('0')));

    ui->aTitre->setText(a.titre);
    const int i = ui->aComp->findData(a.idCompetition);
    ui->aComp->setCurrentIndex(i >= 0 ? i : 0);
    ui->aDate->setDateTime(a.date.isValid() ? a.date : QDateTime(QDate::currentDate(), QTime(10, 0)));
    ui->aDuree->setValue(a.duree > 0 ? a.duree : 60);
    ui->aCap->setValue(a.capacite > 0 ? a.capacite : 30);
    ui->aSalle->setText(a.salle);
    ui->aDesc->setPlainText(a.description);
}

Atelier AtelierMainWindow::readForm() const
{
    Atelier a;
    a.id            = m_editingId;
    a.titre         = ui->aTitre->text().trimmed();
    a.idCompetition = ui->aComp->currentData().toInt();
    a.date          = ui->aDate->dateTime();
    a.duree         = ui->aDuree->value();
    a.capacite      = ui->aCap->value();
    a.salle         = ui->aSalle->text().trimmed();
    a.description   = ui->aDesc->toPlainText().trimmed();
    return a;
}

QString AtelierMainWindow::validate(const Atelier &a, QString *conflict) const
{
    if (a.titre.isEmpty())        return "Le titre est obligatoire.";
    if (a.titre.size() > 100)     return "Le titre est trop long (100 caractères max).";
    if (a.idCompetition <= 0)     return "Choisissez une compétition (créez-en une d'abord si la liste est vide).";
    if (a.salle.isEmpty())        return "La salle est obligatoire.";
    if (a.duree <= 0)             return "La durée doit être positive.";
    if (a.capacite <= 0)          return "La capacité doit être positive.";

    // Conflit de salle : même salle, créneaux qui se chevauchent
    for (const Atelier &o : m_all) {
        if (o.id == a.id) continue;
        if (o.salle.compare(a.salle, Qt::CaseInsensitive) != 0) continue;
        if (a.date < o.fin() && o.date < a.fin()) {
            if (conflict)
                *conflict = QString("⚠️ Conflit de salle : « %1 » occupe %2 de %3 à %4.")
                                .arg(o.titre, o.salle, o.date.toString("dd/MM HH:mm"), o.fin().toString("HH:mm"));
            return "Choisissez un autre créneau ou une autre salle.";
        }
    }
    return QString();
}

void AtelierMainWindow::onSave()
{
    ui->aErr->clear();
    ui->aConflict->clear();

    Atelier a = readForm();
    QString conflict;
    const QString err = validate(a, &conflict);
    if (!err.isEmpty()) {
        ui->aConflict->setText(conflict);
        ui->aErr->setText("⚠️ " + err);
        return;
    }

    QString dbErr;
    const bool editing = m_editingId > 0;
    const bool ok = editing ? AtelierDao::update(a, &dbErr) : AtelierDao::insert(a, &dbErr);
    if (!ok) {
        ui->aErr->setText("Erreur base de données : " + dbErr);
        return;
    }
    statusBar()->showMessage(editing ? "Atelier mis à jour." : "Atelier ajouté.", 4000);
    reload(a.id);
    fillForm(Atelier(), false);
}

void AtelierMainWindow::onClear() { fillForm(Atelier(), false); }

void AtelierMainWindow::onEdit()
{
    if (const Atelier *a = selectedAtelier()) {
        fillForm(*a, true);
        ui->aTitre->setFocus();
    }
}

void AtelierMainWindow::onDuplicate()
{
    if (const Atelier *a = selectedAtelier()) {
        Atelier c = *a;
        c.id = 0;
        c.titre += " (copie)";
        fillForm(c, false);
        ui->aTitre->setFocus();
        statusBar()->showMessage("Copie chargée dans le formulaire : ajustez la date puis enregistrez.", 5000);
    }
}

void AtelierMainWindow::onDelete()
{
    const Atelier *a = selectedAtelier();
    if (!a) return;
    const int id = a->id;
    if (QMessageBox::question(this, "Supprimer",
            QString("Supprimer définitivement l'atelier « %1 » ?").arg(a->titre)) != QMessageBox::Yes)
        return;
    QString err;
    if (!AtelierDao::remove(id, &err)) {
        QMessageBox::warning(this, "Suppression impossible", err);
        return;
    }
    if (m_editingId == id) fillForm(Atelier(), false);
    statusBar()->showMessage("Atelier supprimé.", 4000);
    reload();
}

// ------------------------------------------------------------------ quick actions / paging
void AtelierMainWindow::onProchain()
{
    const QDateTime now = QDateTime::currentDateTime();
    const Atelier *best = nullptr;
    for (const Atelier &a : std::as_const(m_all))
        if (a.fin() >= now && (!best || a.date < best->date)) best = &a;
    if (!best) {
        statusBar()->showMessage("Aucun atelier à venir.", 4000);
        return;
    }
    const int id = best->id;
    onResetFilters();
    const int idx = [&] {
        for (int i = 0; i < m_filtered.size(); ++i) if (m_filtered[i].id == id) return i;
        return 0;
    }();
    m_page = idx / PAGE_SIZE;
    fillTable();
    selectRowById(id);
}

void AtelierMainWindow::onResetFilters()
{
    {
        QSignalBlocker b1(ui->search), b2(ui->topSearch), b3(ui->fCompetition),
                       b4(ui->fStatut), b5(ui->fDate), b6(ui->fSort);
        ui->search->clear();
        ui->topSearch->clear();
        ui->fCompetition->setCurrentIndex(0);
        ui->fStatut->setCurrentIndex(0);
        ui->fDate->setCurrentIndex(0);
        ui->fSort->setCurrentIndex(0);
    }
    applyFilters();
}

void AtelierMainWindow::onPrevPage() { --m_page; fillTable(); }
void AtelierMainWindow::onNextPage() { ++m_page; fillTable(); }

void AtelierMainWindow::onOtherModule()
{
    statusBar()->showMessage("Ce module est développé par un autre membre de l'équipe.", 4000);
}

// ------------------------------------------------------------------ exports
void AtelierMainWindow::onExportPdf()
{
    if (m_filtered.isEmpty()) {
        QMessageBox::information(this, "Export PDF", "Aucun atelier à exporter.");
        return;
    }
    const QString path = QFileDialog::getSaveFileName(this, "Exporter en PDF", "ateliers.pdf", "PDF (*.pdf)");
    if (path.isEmpty()) return;

    QString html =
        "<h1 style='color:#0B1630;'>HackTime – Liste des ateliers</h1>"
        "<p style='color:#6B7A99;'>Généré le " + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm") +
        " – " + QString::number(m_filtered.size()) + " atelier(s)</p>"
        "<table width='100%' cellspacing='0' cellpadding='6' style='border-collapse:collapse;'>"
        "<tr style='background:#1665E8;color:white;'>"
        "<th>ID</th><th align='left'>Titre</th><th align='left'>Compétition</th><th>Date</th>"
        "<th>Durée</th><th>Salle</th><th>Capacité</th><th>Statut</th></tr>";
    bool alt = false;
    for (const Atelier &a : std::as_const(m_filtered)) {
        html += QString("<tr style='background:%1;'><td align='center'>%2</td><td>%3</td><td>%4</td>"
                        "<td align='center'>%5</td><td align='center'>%6</td><td align='center'>%7</td>"
                        "<td align='center'>%8</td><td align='center'>%9</td></tr>")
                    .arg(alt ? "#F3F6FB" : "#FFFFFF", a.code(), a.titre.toHtmlEscaped(),
                         a.competitionNom.toHtmlEscaped(), a.date.toString("dd/MM/yyyy HH:mm"),
                         Atelier::dureeText(a.duree), a.salle.toHtmlEscaped())
                    .arg(a.capacite)
                    .arg(Atelier::statutText(a.statut()));
        alt = !alt;
    }
    html += "</table>";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    printer.setPageLayout(QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Landscape, QMarginsF(12, 12, 12, 12)));

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);
    statusBar()->showMessage("PDF exporté : " + path, 5000);
}

void AtelierMainWindow::onExportCsv()
{
    if (m_filtered.isEmpty()) {
        QMessageBox::information(this, "Export Excel", "Aucun atelier à exporter.");
        return;
    }
    const QString path = QFileDialog::getSaveFileName(this, "Exporter pour Excel", "ateliers.csv", "CSV (*.csv)");
    if (path.isEmpty()) return;

    auto esc = [](QString s) { s.replace('"', "\"\""); return "\"" + s + "\""; };
    QString out = "ID;Titre;Compétition;Date;Durée (min);Salle;Capacité;Statut;Description\n";
    for (const Atelier &a : std::as_const(m_filtered)) {
        out += QStringList{esc(a.code()), esc(a.titre), esc(a.competitionNom),
                           esc(a.date.toString("dd/MM/yyyy HH:mm")), QString::number(a.duree),
                           esc(a.salle), QString::number(a.capacite),
                           esc(Atelier::statutText(a.statut())), esc(a.description)}.join(';') + "\n";
    }

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Export", "Impossible d'écrire le fichier.");
        return;
    }
    f.write("\xEF\xBB\xBF");            // BOM UTF-8 : Excel affiche correctement les accents
    f.write(out.toUtf8());
    f.close();
    statusBar()->showMessage("Fichier exporté : " + path, 5000);
}
