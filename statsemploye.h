#ifndef STATSEMPLOYE_H
#define STATSEMPLOYE_H

#include <QList>
#include <QPair>
#include <QString>
#include <QWidget>

// Module Employés : graphique des statistiques (barres + anneau), dessiné avec QPainter
class StatsWidget : public QWidget
{
public:
    explicit StatsWidget(QWidget *parent = nullptr);
    void setDonnees(const QList<QPair<QString, int> > &donnees);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<QPair<QString, int> > m_donnees;
};

#endif // STATSEMPLOYE_H
