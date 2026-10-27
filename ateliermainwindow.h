#ifndef ATELIERMAINWINDOW_H
#define ATELIERMAINWINDOW_H

#include "atelier.h"

#include <QList>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class AtelierMainWindow; }
QT_END_NAMESPACE

class AtelierMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AtelierMainWindow(QWidget *parent = nullptr);
    ~AtelierMainWindow() override;

private slots:
    void applyFilters();
    void onSelectionChanged();
    void onSave();
    void onClear();
    void onEdit();
    void onDelete();
    void onDuplicate();
    void onProchain();
    void onResetFilters();
    void onPrevPage();
    void onNextPage();
    void onExportPdf();
    void onExportCsv();
    void onOtherModule();
    void onDetails();

private:
    void loadCompetitions();
    void reload(int selectId = 0);
    void rebuildYearFilter();
    void fillTable();
    void updateKpis();
    void updateStats();
    void updateDetails(const Atelier *a);
    void fillForm(const Atelier &a, bool editing);
    Atelier readForm() const;
    QString validate(const Atelier &a, QString *conflict) const;
    const Atelier *selectedAtelier() const;
    void selectRowById(int id);

    Ui::AtelierMainWindow *ui;
    QList<Atelier> m_all;
    QList<Atelier> m_filtered;
    int m_page = 0;
    int m_editingId = 0;
    static constexpr int PAGE_SIZE = 8;
};

#endif // ATELIERMAINWINDOW_H
