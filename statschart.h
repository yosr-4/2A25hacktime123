#ifndef STATSCHART_H
#define STATSCHART_H

#include <QVector>
#include <QWidget>

// Petit histogramme (12 mois) dessiné à la main, sans dépendance QtCharts.
class StatsChart : public QWidget
{
    Q_OBJECT
public:
    explicit StatsChart(QWidget *parent = nullptr);
    void setValues(const QVector<int> &monthly);   // 12 valeurs

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QVector<int> m_values;
};

#endif // STATSCHART_H
