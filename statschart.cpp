#include "statschart.h"

#include <QPainter>
#include <algorithm>

StatsChart::StatsChart(QWidget *parent) : QWidget(parent), m_values(12, 0)
{
    setMinimumHeight(190);
}

void StatsChart::setValues(const QVector<int> &monthly)
{
    m_values = monthly;
    m_values.resize(12);
    update();
}

void StatsChart::paintEvent(QPaintEvent *)
{
    static const char *months[12] = {"J","F","M","A","M","J","J","A","S","O","N","D"};

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF r = QRectF(rect()).adjusted(6, 20, -6, -24);
    const int mx = std::max(1, *std::max_element(m_values.begin(), m_values.end()));
    const double slot = r.width() / 12.0;
    const double bw = slot * 0.56;

    QFont f = font();
    f.setPointSizeF(8.5);
    p.setFont(f);

    // ligne de base
    p.setPen(QColor("#E3EAF6"));
    p.drawLine(QPointF(r.left(), r.bottom()), QPointF(r.right(), r.bottom()));

    for (int i = 0; i < 12; ++i) {
        const double h = r.height() * m_values[i] / mx;
        const double x = r.left() + i * slot + (slot - bw) / 2.0;
        const QRectF bar(x, r.bottom() - std::max(h, 3.0), bw, std::max(h, 3.0));

        p.setPen(Qt::NoPen);
        p.setBrush(m_values[i] > 0 ? QColor("#1665E8") : QColor("#E3EAF6"));
        p.drawRoundedRect(bar, 4, 4);

        p.setPen(QColor("#6B7A99"));
        p.drawText(QRectF(r.left() + i * slot, r.bottom() + 5, slot, 16), Qt::AlignCenter, months[i]);

        if (m_values[i] > 0) {
            p.setPen(QColor("#0B1630"));
            p.drawText(QRectF(r.left() + i * slot, bar.top() - 17, slot, 15), Qt::AlignCenter,
                       QString::number(m_values[i]));
        }
    }
}
