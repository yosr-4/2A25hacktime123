#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>

class Participant {
private:
    QString m_id;
    QString m_name;
    QString m_email;
    QString m_phone;
    QString m_role;
    QString m_status;

public:
    Participant();
    Participant(QString id, QString name, QString email, QString phone, QString role, QString status);

    QString getId() const { return m_id; }
    QString getName() const { return m_name; }
    QString getEmail() const { return m_email; }
    QString getPhone() const { return m_phone; }
    QString getRole() const { return m_role; }
    QString getStatus() const { return m_status; }

    void setId(QString id) { m_id = id; }
    void setName(QString name) { m_name = name; }
    void setEmail(QString email) { m_email = email; }
    void setPhone(QString phone) { m_phone = phone; }
    void setRole(QString role) { m_role = role; }
    void setStatus(QString status) { m_status = status; }

    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(QString id);
    bool modifier(QString id);
};

#endif