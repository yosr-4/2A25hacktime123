#include "atelier.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

static const char *DT_FMT = "yyyy-MM-dd HH:mm:ss";

static QDateTime parseDate(const QVariant &v)
{
    if (v.userType() == QMetaType::QDateTime) return v.toDateTime();
    const QString s = v.toString();
    QDateTime d = QDateTime::fromString(s, DT_FMT);
    if (!d.isValid()) d = QDateTime::fromString(s, Qt::ISODate);
    if (!d.isValid()) d = QDateTime::fromString(s, "yyyy-MM-dd");
    return d;
}

Atelier::Statut Atelier::statut(const QDateTime &now) const
{
    if (!date.isValid()) return AVenir;
    if (now < date) return AVenir;
    if (now <= fin()) return EnCours;
    return Termine;
}

QString Atelier::statutText(Statut s)
{
    switch (s) {
    case AVenir:  return "À venir";
    case EnCours: return "En cours";
    case Termine: return "Terminé";
    }
    return QString();
}

QString Atelier::dureeText(int minutes)
{
    const int h = minutes / 60, m = minutes % 60;
    if (h == 0) return QString("%1 min").arg(m);
    if (m == 0) return QString("%1 h").arg(h);
    return QString("%1 h %2").arg(h).arg(m, 2, 10, QChar('0'));
}

QList<Atelier> AtelierDao::all(QString *err)
{
    QList<Atelier> list;
    QSqlQuery q;
    if (!q.exec("SELECT a.id_atelier, a.titre, a.description, a.date_atelier, a.duree, a.salle, "
                "a.capacite, a.id_competition, c.nom "
                "FROM Atelier a LEFT JOIN Competition c ON c.id_competition = a.id_competition "
                "ORDER BY a.date_atelier")) {
        if (err) *err = q.lastError().text();
        return list;
    }
    while (q.next()) {
        Atelier a;
        a.id             = q.value(0).toInt();
        a.titre          = q.value(1).toString();
        a.description    = q.value(2).toString();
        a.date           = parseDate(q.value(3));
        a.duree          = q.value(4).toInt();
        a.salle          = q.value(5).toString();
        a.capacite       = q.value(6).toInt();
        a.idCompetition  = q.value(7).toInt();
        a.competitionNom = q.value(8).toString();
        list << a;
    }
    return list;
}

bool AtelierDao::insert(Atelier &a, QString *err)
{
    QSqlQuery q;
    q.prepare("INSERT INTO Atelier (titre, description, date_atelier, duree, salle, capacite, id_competition) "
              "VALUES (?, ?, ?, ?, ?, ?, ?)");
    q.addBindValue(a.titre);
    q.addBindValue(a.description);
    q.addBindValue(a.date.toString(DT_FMT));
    q.addBindValue(a.duree);
    q.addBindValue(a.salle);
    q.addBindValue(a.capacite);
    q.addBindValue(a.idCompetition);
    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false;
    }
    a.id = q.lastInsertId().toInt();
    return true;
}

bool AtelierDao::update(const Atelier &a, QString *err)
{
    QSqlQuery q;
    q.prepare("UPDATE Atelier SET titre=?, description=?, date_atelier=?, duree=?, salle=?, capacite=?, "
              "id_competition=? WHERE id_atelier=?");
    q.addBindValue(a.titre);
    q.addBindValue(a.description);
    q.addBindValue(a.date.toString(DT_FMT));
    q.addBindValue(a.duree);
    q.addBindValue(a.salle);
    q.addBindValue(a.capacite);
    q.addBindValue(a.idCompetition);
    q.addBindValue(a.id);
    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false;
    }
    return true;
}

bool AtelierDao::remove(int id, QString *err)
{
    QSqlQuery q;
    q.prepare("DELETE FROM Atelier WHERE id_atelier=?");
    q.addBindValue(id);
    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false;
    }
    return true;
}

QList<QPair<int, QString>> AtelierDao::competitions(QString *err)
{
    QList<QPair<int, QString>> list;
    QSqlQuery q;
    if (!q.exec("SELECT id_competition, nom FROM Competition ORDER BY nom")) {
        if (err) *err = q.lastError().text();
        return list;
    }
    while (q.next())
        list << qMakePair(q.value(0).toInt(), q.value(1).toString());
    return list;
}

int AtelierDao::nextId()
{
    QSqlQuery q;
    if (q.exec("SELECT COALESCE(MAX(id_atelier), 0) + 1 FROM Atelier") && q.next())
        return q.value(0).toInt();
    return 1;
}
