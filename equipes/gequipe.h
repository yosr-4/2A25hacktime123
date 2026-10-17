#ifndef GEQUIPE_H
#define GEQUIPE_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QList>

struct Equipe {
    QString nom;
    QString competition;
    QString chef;
    QString membres;
    QString date;
    QString statut;
};

QT_BEGIN_NAMESPACE
namespace Ui { class Gequipe; }
QT_END_NAMESPACE

class Gequipe : public QMainWindow
{
    Q_OBJECT

public:
    Gequipe(QWidget *parent = nullptr);
    ~Gequipe();

private slots:
    void chargerPage(int page);
    void onTeamSelected(int row, int column);
    void on_btnAjouter_clicked();
    void filtrerEquipes(const QString &text);
    void filtrerParComboBox();
    void on_btnStatistiques_clicked(); // <- Ajouté ici

private:
    Ui::Gequipe *ui;
    QList<Equipe> toutesLesEquipes;
    QList<Equipe> equipesAffichees;
    int currentPage = 1;
    void initialiserToutesLesEquipes();
};

#endif // GEQUIPE_H