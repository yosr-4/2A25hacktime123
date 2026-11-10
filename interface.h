#ifndef INTERFACE_H
#define INTERFACE_H

#include <QMainWindow>
#include <QList>
#include <QString>
#include <QStringList>

class QPushButton;
class QTextEdit;
class QFrame;
class QLabel;
class QLineEdit;
class QComboBox;
class QResizeEvent;

struct Equipe
{
    int idEquipe = 0;
    int idCompetition = 0;
    int idChef = 0;
    int maxMembres = 0;
    QString nom;
    QString competition;
    QString chef;
    QString membres;
    QString date;
    QString statut;
    QStringList nomsMembres;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void chargerPage(int page);
    void onTeamSelected(int row, int column);
    void on_btnAjouter_clicked();
    void filtrerEquipes(const QString &text);
    void filtrerParComboBox();
    void on_btnStatistiques_clicked();

private:
    Ui::MainWindow *ui;
    QList<Equipe> toutesLesEquipes;
    QList<Equipe> equipesAffichees;
    int currentPage = 1;
    bool layoutReady = false;

    QFrame *panneauDroit = nullptr;
    QLineEdit *editNomEquipe = nullptr;
    QLineEdit *editDateEquipe = nullptr;
    QComboBox *comboNouvelleCompetition = nullptr;
    QLabel *lblStatTotal = nullptr;
    QLabel *lblStatActives = nullptr;
    QLabel *lblStatCours = nullptr;
    QLabel *lblStatIncompletes = nullptr;
    QLabel *lblDetailNom = nullptr;
    QLabel *lblDetailCompetition = nullptr;
    QLabel *lblDetailChef = nullptr;
    QLabel *lblDetailMembres = nullptr;
    QLabel *lblDetailDate = nullptr;
    QLabel *lblDetailStatut = nullptr;

    void initialiserPanneauDroit();
    void actualiserStatistiques();
    void afficherDetailsEquipe(const Equipe &equipe);
    bool chargerEquipesDepuisOracle();
    void ouvrirGestionChef(int idEquipe);
    void ouvrirGestionMembres(int idEquipe, int maxMembres, QPushButton *labelNombre, QTextEdit *listeMembres);
};

#endif // INTERFACE_H