#ifndef ATELIER_H
#define ATELIER_H

#include <QDateTime>
#include <QList>
#include <QPair>
#include <QString>

// Table Atelier (id_atelier, titre, description, date_atelier, duree, salle, capacite, #id_competition)
struct Atelier
{
    enum Statut { AVenir = 0, EnCours = 1, Termine = 2 };

    int       id            = 0;
    QString   titre;
    QString   description;
    QDateTime date;                 // date_atelier
    int       duree         = 60;   // minutes
    QString   salle;
    int       capacite      = 30;
    int       idCompetition = 0;
    QString   competitionNom;       // jointure (lecture seule)

    QDateTime fin() const { return date.addSecs(qint64(duree) * 60); }
    QString   code() const { return QString("A-%1").arg(id, 3, 10, QChar('0')); }
    Statut    statut(const QDateTime &now = QDateTime::currentDateTime()) const;
    static QString statutText(Statut s);
    static QString dureeText(int minutes);
};

// Accès base de données (CRUD)
class AtelierDao
{
public:
    static QList<Atelier> all(QString *err = nullptr);
    static bool insert(Atelier &a, QString *err = nullptr);
    static bool update(const Atelier &a, QString *err = nullptr);
    static bool remove(int id, QString *err = nullptr);
    static QList<QPair<int, QString>> competitions(QString *err = nullptr);
    static int nextId();
};

#endif // ATELIER_H
