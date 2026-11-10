#include "statsemploye.h"

#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>

namespace {

const char *const COULEURS[] = {
    "#1a5ae8", "#ff8c1a", "#1fa35b", "#7a4be0", "#0fa3b1", "#e0508f",
    "#c9a400", "#6b7691", "#d33a3a", "#3fb0ff", "#8a5a2b", "#4a9b2f"
};
const int NB_COULEURS = 12;

QColor couleur(int i)
{
    return QColor(COULEURS[i % NB_COULEURS]);
}

} // namespace

StatsWidget::StatsWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(300);
}

void StatsWidget::setDonnees(const QList<QPair<QString, int> > &donnees)
{
    m_donnees = donnees;
    update();
}

void StatsWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QColor texte("#0f1a38");
    const QColor discret("#65708b");
    const QColor piste("#f3f6fc");

    if (m_donnees.isEmpty()) {
        p.setPen(discret);
        p.drawText(rect(), Qt::AlignCenter, "Ajoutez des employés pour afficher les statistiques.");
        return;
    }

    int total = 0;
    int maximum = 1;
    for (int i = 0; i < m_donnees.size(); ++i) {
        total += m_donnees.at(i).second;
        if (m_donnees.at(i).second > maximum)
            maximum = m_donnees.at(i).second;
    }

    const int largeur = width();
    const int hauteur = height();
    const int zoneAnneau = qMin(300, largeur / 3);
    const int zoneBarres = largeur - zoneAnneau - 24;

    // ---- Barres horizontales --------------------------------------
    const int n = m_donnees.size();
    const int hauteurLigne = qBound(22, (hauteur - 20) / n, 40);
    const int largeurLibelle = qMin(210, zoneBarres / 3);
    const int largeurValeur = 36;
    const int largeurPiste = qMax(40, zoneBarres - largeurLibelle - largeurValeur);
    int y = qMax(10, (hauteur - n * hauteurLigne) / 2);

    QFont normal = font();
    QFont gras = font();
    gras.setBold(true);
    const QFontMetrics fm(normal);

    for (int i = 0; i < n; ++i) {
        const QString nom = m_donnees.at(i).first;
        const int valeur = m_donnees.at(i).second;

        p.setFont(normal);
        p.setPen(texte);
        p.drawText(QRect(0, y, largeurLibelle - 10, hauteurLigne),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   fm.elidedText(nom, Qt::ElideRight, largeurLibelle - 10));

        const QRectF fond(largeurLibelle, y + (hauteurLigne - 18) / 2.0, largeurPiste, 18);
        p.setPen(Qt::NoPen);
        p.setBrush(piste);
        p.drawRoundedRect(fond, 5, 5);

        QRectF barre = fond;
        barre.setWidth(fond.width() * valeur / maximum);
        p.setBrush(couleur(i));
        p.drawRoundedRect(barre, 5, 5);

        p.setFont(gras);
        p.setPen(texte);
        p.drawText(QRect(largeurLibelle + largeurPiste, y, largeurValeur, hauteurLigne),
                   Qt::AlignVCenter | Qt::AlignRight, QString::number(valeur));

        y += hauteurLigne;
    }

    // ---- Anneau de répartition ------------------------------------
    const int epaisseur = 26;
    const int diametre = qMin(zoneAnneau, hauteur) - epaisseur - 30;
    if (diametre < 60 || total <= 0)
        return;

    const QRectF cercle(largeur - zoneAnneau + (zoneAnneau - diametre) / 2.0,
                        (hauteur - diametre) / 2.0, diametre, diametre);
    p.setBrush(Qt::NoBrush);
    double depart = 90.0;   // départ en haut, sens horaire
    for (int i = 0; i < n; ++i) {
        const double angle = 360.0 * m_donnees.at(i).second / total;
        QPen stylo(couleur(i));
        stylo.setWidth(epaisseur);
        stylo.setCapStyle(Qt::FlatCap);
        p.setPen(stylo);
        p.drawArc(cercle, int(depart * 16), -int(angle * 16 + 0.5));
        depart -= angle;
    }

    QFont grand = gras;
    grand.setPointSize(qMax(8, gras.pointSize()) + 10);
    p.setFont(grand);
    p.setPen(texte);
    p.drawText(cercle.adjusted(0, -10, 0, -10), Qt::AlignCenter, QString::number(total));
    p.setFont(normal);
    p.setPen(discret);
    p.drawText(cercle.adjusted(0, 24, 0, 24), Qt::AlignCenter, "employés");
}
