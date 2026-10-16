#include "participant.h"

Participant::Participant() {
    m_id = "";
    m_name = "";
    m_email = "";
    m_phone = "";
    m_role = "";
    m_status = "";
}

Participant::Participant(QString id, QString name, QString email, QString phone, QString role, QString status) {
    this->m_id = id;
    this->m_name = name;
    this->m_email = email;
    this->m_phone = phone;
    this->m_role = role;
    this->m_status = status;
}

bool Participant::ajouter() {
    QSqlQuery query;
    query.prepare("INSERT INTO PARTICIPANT (ID, NOM, EMAIL, PHONE, ROLE, STATUT) "
                  "VALUES (:id, :name, :email, :phone, :role, :status)");

    query.bindValue(":id", m_id);
    query.bindValue(":name", m_name);
    query.bindValue(":email", m_email);
    query.bindValue(":phone", m_phone);
    query.bindValue(":role", m_role);
    query.bindValue(":status", m_status);

    return query.exec();
}

QSqlQueryModel* Participant::afficher() {
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM PARTICIPANT");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Rôle"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Statut"));

    return model;
}

bool Participant::supprimer(QString id) {
    QSqlQuery query;
    query.prepare("DELETE FROM PARTICIPANT WHERE ID = :id");
    query.bindValue(":id", id);

    return query.exec();
}

bool Participant::modifier(QString id) {
    QSqlQuery query;
    query.prepare("UPDATE PARTICIPANT SET NOM = :name, EMAIL = :email, PHONE = :phone, "
                  "ROLE = :role, STATUT = :status WHERE ID = :id");

    query.bindValue(":id", id);
    query.bindValue(":name", m_name);
    query.bindValue(":email", m_email);
    query.bindValue(":phone", m_phone);
    query.bindValue(":role", m_role);
    query.bindValue(":status", m_status);

    return query.exec();
}