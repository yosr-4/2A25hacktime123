// =====================================================================
//  Classe Competition : l'entité gérée par le module « Gestion des compétitions ».
//  Elle correspond à la table COMPETITION de la base Oracle :
//    Competition (id_competition, nom, theme, date_debut, date_fin,
//                 nb_max_membres, statut, mode)
//    Gerer       (#id_employe, #id_competition)   <- association n,n
//  « mode » est un mot réservé d'Oracle : la colonne s'appelle MODE_COMP.
// =====================================================================
#ifndef COMPETITION_H
#define COMPETITION_H

#include <QDate>
#include <QSqlQueryModel>
#include <QString>
#include <QStringList>
#include <QList>
#include <QPair>
#include <QVector>

class Competition
{
public:
    Competition();
    Competition(const QString &nom, const QString &theme, const QDate &dateDebut,
                const QDate &dateFin, int nbMaxMembres, const QString &statut,
                const QString &mode);

    // ----- getters
    int getId() const { return id; }
    QString getNom() const { return nom; }
    QString getTheme() const { return theme; }
    QDate getDateDebut() const { return dateDebut; }
    QDate getDateFin() const { return dateFin; }
    int getNbMaxMembres() const { return nbMaxMembres; }
    QString getStatut() const { return statut; }   // « Planifiée », « En cours », « Terminée »
    QString getMode() const { return mode; }       // « Présentiel », « En ligne », « Hybride »
    int getIdJury() const { return idJury; }       // ID_JURY (employé de rôle JURY), 0 = aucun
    QString getNomJury() const { return nomJury; } // « Prénom Nom » du jury (lu avec lister())

    // ----- setters
    void setId(int v) { id = v; }
    void setNom(const QString &v) { nom = v; }
    void setTheme(const QString &v) { theme = v; }
    void setDateDebut(const QDate &v) { dateDebut = v; }
    void setDateFin(const QDate &v) { dateFin = v; }
    void setNbMaxMembres(int v) { nbMaxMembres = v; }
    void setStatut(const QString &v) { statut = v; }
    void setMode(const QString &v) { mode = v; }
    void setIdJury(int v) { idJury = v; }

    // ----- contrôle de saisie : message d'erreur, ou chaîne vide si tout est valide
    QString verifier() const;

    // ----- CRUD
    bool ajouter();                          // INSERT (COMPETITION + GERER)
    QSqlQueryModel *afficher();              // SELECT, pour un QTableView
    bool modifier();                         // UPDATE selon l'ID
    static bool supprimer(int id);           // DELETE
    static bool nomExiste(const QString &nom, int idExclu = 0);   // nom déjà utilisé ?
    static QVector<Competition> lister();    // SELECT, pour remplir le tableau de l'interface
    static QList<QPair<int, QString>> listerJurys();   // employés JURY : {ID_EMPLOYE, « Prénom Nom »}
    static QString derniereErreur();         // message Oracle de la dernière erreur

private:
    int id = 0;
    int idJury = 0;          // colonne ID_JURY (clé étrangère vers EMPLOYE)
    QString nomJury;
    QString nom, theme;
    QDate dateDebut, dateFin;
    int nbMaxMembres = 5;
    QString statut, mode;
};

#endif // COMPETITION_H
