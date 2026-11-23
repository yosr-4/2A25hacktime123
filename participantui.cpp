// ============================================================================
//  participantui.cpp : interface du module Gestion des Participants (HackTime)
//  contrôle de saisie, composition d'équipes, engagement, statistiques, exports, certificat
//  (composition d'équipes, engagement, statistiques, exports PDF / Excel)
// ============================================================================
#include "participantui.h"
#include "participant.h"
#include "participantborne.h"
#include "ui_hacktime.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QFrame>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <QDialog>
#include <QLineEdit>
#include <QEvent>
#include <QUrl>
#include <QApplication>
#include <QWidget>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QTextDocument>
#include <QPdfWriter>
#include <QPageSize>
#include <QBuffer>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QRegularExpression>
#include <QComboBox>
#include <QInputDialog>
#include <QDesktopServices>
#include <QSpinBox>
#include <QTabWidget>
#include <QScrollArea>
#include <QHash>
#include <QListWidget>
#include <QMap>
#include <QSet>
#include <QList>
#include <QDate>
#include <QColor>
#include <QPolygonF>
#include <QPainterPath>
#include <QMarginsF>
#include <QByteArray>
#include <QVector>
#include <QDebug>
#include <QEvent>
#include <algorithm>
#include <functional>
#include <memory>
#include <cmath>

static QPixmap createHackTimeLogoPixmap();

namespace {

// ============================================================================
//  Graphique dessiné par le code (aucune dépendance QtCharts)
// ============================================================================
// Graphique simple dessiné par le code (aucune dépendance QtCharts).
// Mode barres (histogramme) ou courbe, alimenté par de vraies données SQL.
class ChartWidget : public QWidget {
public:
    enum Mode { Barres, Courbe };

    struct Point { QString etiquette; double valeur; QColor couleur; };

    explicit ChartWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setMinimumSize(380, 230);
        setAttribute(Qt::WA_OpaquePaintEvent, false);
    }

    void setDonnees(const QList<Point> &donnees, Mode mode, const QString &titre = QString()) {
        m_donnees = donnees;
        m_mode = mode;
        m_titre = titre;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#ffffff"));
        p.setPen(QPen(QColor("#cbd5e1"), 1));
        p.drawRect(rect().adjusted(0, 0, -1, -1));

        int top = 14;
        if (!m_titre.isEmpty()) {
            p.setPen(QColor("#0f172a"));
            p.setFont(QFont("Arial", 10, QFont::Bold));
            p.drawText(QRect(12, 8, width() - 24, 22), Qt::AlignLeft | Qt::AlignVCenter, m_titre);
            top = 40;
        }

        if (m_donnees.isEmpty()) {
            p.setPen(QColor("#64748b"));
            p.setFont(QFont("Arial", 10));
            p.drawText(rect(), Qt::AlignCenter, "Aucune donnée à afficher");
            return;
        }

        const int gauche = 46, droite = 18, bas = 38;
        const QRectF zone(gauche, top, width() - gauche - droite, height() - top - bas);

        double maxVal = 0;
        for (const Point &d : m_donnees) maxVal = std::max(maxVal, d.valeur);
        const double maxAxe = niceMax(maxVal);

        // grille + graduations
        const int graduations = 4;
        p.setFont(QFont("Arial", 8));
        for (int i = 0; i <= graduations; ++i) {
            const double y = zone.bottom() - zone.height() * i / graduations;
            p.setPen(QPen(QColor("#e2e8f0"), 1));
            p.drawLine(QPointF(zone.left(), y), QPointF(zone.right(), y));
            p.setPen(QColor("#475569"));
            const double v = maxAxe * i / graduations;
            p.drawText(QRectF(2, y - 8, gauche - 8, 16), Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(v, 'f', (std::fabs(v - std::round(v)) < 1e-9) ? 0 : 1));
        }

        const int n = m_donnees.size();
        const double pas = zone.width() / n;

        if (m_mode == Barres) {
            const double largeur = std::min(60.0, pas * 0.6);
            for (int i = 0; i < n; ++i) {
                const Point &d = m_donnees[i];
                const double h = (maxAxe > 0) ? zone.height() * d.valeur / maxAxe : 0;
                const QRectF barre(zone.left() + pas * i + (pas - largeur) / 2, zone.bottom() - h, largeur, h);
                p.setPen(Qt::NoPen);
                p.setBrush(d.couleur);
                p.drawRoundedRect(barre, 3, 3);

                p.setPen(QColor("#0f172a"));
                p.setFont(QFont("Arial", 9, QFont::Bold));
                p.drawText(QRectF(barre.left() - 10, barre.top() - 18, barre.width() + 20, 16), Qt::AlignCenter,
                           QString::number(d.valeur, 'f', (std::fabs(d.valeur - std::round(d.valeur)) < 1e-9) ? 0 : 1));

                p.setPen(QColor("#334155"));
                p.setFont(QFont("Arial", 8, QFont::Bold));
                p.drawText(QRectF(zone.left() + pas * i, zone.bottom() + 4, pas, bas - 6),
                           Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, d.etiquette);
            }
        } else {
            QPolygonF poly;
            for (int i = 0; i < n; ++i) {
                const double x = zone.left() + pas * i + pas / 2;
                const double y = zone.bottom() - ((maxAxe > 0) ? zone.height() * m_donnees[i].valeur / maxAxe : 0);
                poly << QPointF(x, y);
            }
            p.setPen(QPen(QColor("#2563eb"), 2.5));
            p.drawPolyline(poly);
            for (int i = 0; i < n; ++i) {
                p.setPen(Qt::NoPen);
                p.setBrush(QColor("#ea580c"));
                p.drawEllipse(poly[i], 4, 4);
                p.setPen(QColor("#0f172a"));
                p.setFont(QFont("Arial", 8, QFont::Bold));
                p.drawText(QRectF(poly[i].x() - 16, poly[i].y() - 20, 32, 14), Qt::AlignCenter,
                           QString::number(m_donnees[i].valeur, 'f', 0));
                p.setPen(QColor("#334155"));
                p.setFont(QFont("Arial", 8));
                p.drawText(QRectF(zone.left() + pas * i, zone.bottom() + 4, pas, bas - 6),
                           Qt::AlignHCenter | Qt::AlignTop, m_donnees[i].etiquette);
            }
        }
    }

private:
    static double niceMax(double v) {
        if (v <= 4) return 4;
        const double exp10 = std::pow(10.0, std::floor(std::log10(v)));
        const double f = v / exp10;
        double nf = (f <= 1) ? 1 : (f <= 2) ? 2 : (f <= 4) ? 4 : (f <= 8) ? 8 : 10;
        return nf * exp10;
    }

    QList<Point> m_donnees;
    Mode m_mode = Barres;
    QString m_titre;
};


// ============================================================================
//  Écriture d'un vrai fichier Excel (.xlsx) sans bibliothèque externe
// ============================================================================
struct FeuilleXlsx {
    QString nom;
    QList<QVariantList> lignes;     // lignes[0] = en-têtes
    QList<int> largeurs;            // largeur des colonnes (en caractères), optionnel
    bool filtre = true;             // filtre automatique sur la ligne d'en-têtes
};


// ---------- CRC32 ----------
quint32 crc32Zip(const QByteArray &data)
{
    static quint32 table[256];
    static bool init = false;
    if (!init) {
        for (quint32 i = 0; i < 256; ++i) {
            quint32 c = i;
            for (int k = 0; k < 8; ++k)
                c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            table[i] = c;
        }
        init = true;
    }
    quint32 crc = 0xFFFFFFFFu;
    for (char ch : data)
        crc = table[(crc ^ quint8(ch)) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

void ecrire16(QByteArray &b, quint16 v) { b.append(char(v & 0xFF)); b.append(char((v >> 8) & 0xFF)); }
void ecrire32(QByteArray &b, quint32 v) { for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xFF)); }

struct EntreeZip { QString nom; QByteArray donnees; quint32 crc; quint32 offset; };

QByteArray construireZip(const QList<QPair<QString, QByteArray>> &fichiers)
{
    QByteArray zip;
    QList<EntreeZip> entrees;

    // Date/heure DOS fixe (évite toute dépendance à l'horloge)
    const quint16 dosTime = 0;
    const quint16 dosDate = (quint16)(((2026 - 1980) << 9) | (1 << 5) | 1);

    for (const auto &f : fichiers) {
        EntreeZip e;
        e.nom = f.first;
        e.donnees = f.second;
        e.crc = crc32Zip(f.second);
        e.offset = quint32(zip.size());
        const QByteArray nom = e.nom.toUtf8();

        ecrire32(zip, 0x04034b50);
        ecrire16(zip, 20);                  // version
        ecrire16(zip, 0x0800);              // UTF-8
        ecrire16(zip, 0);                   // méthode : stockage sans compression
        ecrire16(zip, dosTime);
        ecrire16(zip, dosDate);
        ecrire32(zip, e.crc);
        ecrire32(zip, quint32(e.donnees.size()));
        ecrire32(zip, quint32(e.donnees.size()));
        ecrire16(zip, quint16(nom.size()));
        ecrire16(zip, 0);
        zip.append(nom);
        zip.append(e.donnees);
        entrees.append(e);
    }

    const quint32 debutCentral = quint32(zip.size());
    for (const EntreeZip &e : entrees) {
        const QByteArray nom = e.nom.toUtf8();
        ecrire32(zip, 0x02014b50);
        ecrire16(zip, 20);
        ecrire16(zip, 20);
        ecrire16(zip, 0x0800);
        ecrire16(zip, 0);
        ecrire16(zip, dosTime);
        ecrire16(zip, dosDate);
        ecrire32(zip, e.crc);
        ecrire32(zip, quint32(e.donnees.size()));
        ecrire32(zip, quint32(e.donnees.size()));
        ecrire16(zip, quint16(nom.size()));
        ecrire16(zip, 0);
        ecrire16(zip, 0);
        ecrire16(zip, 0);
        ecrire16(zip, 0);
        ecrire32(zip, 0);
        ecrire32(zip, e.offset);
        zip.append(nom);
    }
    const quint32 tailleCentral = quint32(zip.size()) - debutCentral;

    ecrire32(zip, 0x06054b50);
    ecrire16(zip, 0);
    ecrire16(zip, 0);
    ecrire16(zip, quint16(entrees.size()));
    ecrire16(zip, quint16(entrees.size()));
    ecrire32(zip, tailleCentral);
    ecrire32(zip, debutCentral);
    ecrire16(zip, 0);
    return zip;
}

QString echapper(const QString &s)
{
    QString r;
    r.reserve(s.size());
    for (QChar c : s) {
        const ushort u = c.unicode();
        if (c == '&') r += "&amp;";
        else if (c == '<') r += "&lt;";
        else if (c == '>') r += "&gt;";
        else if (c == '"') r += "&quot;";
        else if (u < 0x20 && u != 9 && u != 10 && u != 13) continue;   // caractères interdits en XML
        else r += c;
    }
    return r;
}

QString nomColonne(int index)   // 0 -> A, 25 -> Z, 26 -> AA
{
    QString s;
    int n = index + 1;
    while (n > 0) {
        const int r = (n - 1) % 26;
        s.prepend(QChar('A' + r));
        n = (n - 1) / 26;
    }
    return s;
}

QString xmlFeuille(const FeuilleXlsx &f)
{
    int nbCol = 0;
    for (const QVariantList &l : f.lignes) nbCol = std::max(nbCol, int(l.size()));

    QString x;
    x += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    x += "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">";
    x += "<sheetViews><sheetView workbookViewId=\"0\">"
         "<pane ySplit=\"1\" topLeftCell=\"A2\" activePane=\"bottomLeft\" state=\"frozen\"/>"
         "</sheetView></sheetViews>";
    x += "<sheetFormatPr defaultRowHeight=\"18\"/>";
    if (!f.largeurs.isEmpty()) {
        x += "<cols>";
        for (int i = 0; i < f.largeurs.size(); ++i)
            x += QString("<col min=\"%1\" max=\"%1\" width=\"%2\" customWidth=\"1\"/>").arg(i + 1).arg(f.largeurs[i]);
        x += "</cols>";
    }
    x += "<sheetData>";
    for (int r = 0; r < f.lignes.size(); ++r) {
        x += QString("<row r=\"%1\">").arg(r + 1);
        const QVariantList &ligne = f.lignes[r];
        for (int c = 0; c < ligne.size(); ++c) {
            const QString ref = nomColonne(c) + QString::number(r + 1);
            const QVariant &v = ligne[c];
            const int style = (r == 0) ? 1 : 0;
            const int t = v.typeId();
            if (t == QMetaType::Int || t == QMetaType::LongLong || t == QMetaType::Double ||
                t == QMetaType::UInt || t == QMetaType::ULongLong) {
                x += QString("<c r=\"%1\" s=\"%2\"><v>%3</v></c>").arg(ref).arg(style).arg(v.toString());
            } else {
                x += QString("<c r=\"%1\" s=\"%2\" t=\"inlineStr\"><is><t xml:space=\"preserve\">%3</t></is></c>")
                         .arg(ref).arg(style).arg(echapper(v.toString()));
            }
        }
        x += "</row>";
    }
    x += "</sheetData>";
    if (f.filtre && !f.lignes.isEmpty() && nbCol > 0)
        x += QString("<autoFilter ref=\"A1:%1%2\"/>").arg(nomColonne(nbCol - 1)).arg(f.lignes.size());
    x += "</worksheet>";
    return x;
}

bool ecrireXlsx(const QString &fichier, const QList<FeuilleXlsx> &feuilles, QString *erreur)
{
    if (feuilles.isEmpty()) {
        if (erreur) *erreur = "Aucune feuille à écrire";
        return false;
    }

    QList<QPair<QString, QByteArray>> f;

    // [Content_Types].xml
    QString ct = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                 "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                 "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
                 "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                 "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
                 "<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>";
    for (int i = 0; i < feuilles.size(); ++i)
        ct += QString("<Override PartName=\"/xl/worksheets/sheet%1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>").arg(i + 1);
    ct += "</Types>";
    f.append({"[Content_Types].xml", ct.toUtf8()});

    // _rels/.rels
    f.append({"_rels/.rels",
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                         "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                         "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
                         "</Relationships>")});

    // xl/workbook.xml
    QString wb = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                 "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
                 "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><sheets>";
    for (int i = 0; i < feuilles.size(); ++i) {
        QString nom = feuilles[i].nom;
        nom.remove(QRegularExpression("[\\\\/?*\\[\\]:]"));
        nom = nom.left(31);
        wb += QString("<sheet name=\"%1\" sheetId=\"%2\" r:id=\"rId%2\"/>").arg(echapper(nom)).arg(i + 1);
    }
    wb += "</sheets></workbook>";
    f.append({"xl/workbook.xml", wb.toUtf8()});

    // xl/_rels/workbook.xml.rels
    QString rels = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                   "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
    for (int i = 0; i < feuilles.size(); ++i)
        rels += QString("<Relationship Id=\"rId%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet%1.xml\"/>").arg(i + 1);
    rels += QString("<Relationship Id=\"rId%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>").arg(feuilles.size() + 1);
    rels += "</Relationships>";
    f.append({"xl/_rels/workbook.xml.rels", rels.toUtf8()});

    // xl/styles.xml : style 0 = normal, style 1 = en-tête (gras blanc sur fond bleu)
    f.append({"xl/styles.xml",
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                         "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
                         "<fonts count=\"2\">"
                         "<font><sz val=\"11\"/><name val=\"Calibri\"/></font>"
                         "<font><b/><sz val=\"11\"/><color rgb=\"FFFFFFFF\"/><name val=\"Calibri\"/></font>"
                         "</fonts>"
                         "<fills count=\"3\">"
                         "<fill><patternFill patternType=\"none\"/></fill>"
                         "<fill><patternFill patternType=\"gray125\"/></fill>"
                         "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF2563EB\"/><bgColor indexed=\"64\"/></patternFill></fill>"
                         "</fills>"
                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                         "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
                         "<cellXfs count=\"2\">"
                         "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
                         "<xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"0\" xfId=\"0\" applyFont=\"1\" applyFill=\"1\" applyAlignment=\"1\"><alignment horizontal=\"center\" vertical=\"center\"/></xf>"
                         "</cellXfs>"
                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>"
                         "</styleSheet>")});

    for (int i = 0; i < feuilles.size(); ++i)
        f.append({QString("xl/worksheets/sheet%1.xml").arg(i + 1), xmlFeuille(feuilles[i]).toUtf8()});

    QFile out(fichier);
    if (!out.open(QIODevice::WriteOnly)) {
        if (erreur) *erreur = out.errorString();
        return false;
    }
    out.write(construireZip(f));
    out.close();
    return true;
}


// =====================  CONTRÔLE DE SAISIE  =====================
const QString kEditOk  = "QLineEdit { border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px 8px; background-color: #ffffff; color: #0f172a; }";
const QString kEditErr = "QLineEdit { border: 2px solid #dc2626; border-radius: 6px; padding: 5px 7px; background-color: #fee2e2; color: #0f172a; }";

// Lettres uniquement (accents autorisés), espaces simples entre les mots
bool nomValide(const QString &t) {
    static const QRegularExpression rx("^\\p{L}+( \\p{L}+)*$");
    return rx.match(t.trimmed()).hasMatch();
}

// E-mail strict : partie locale + @ + domaine + "." + extension (2 lettres minimum)
bool emailValide(const QString &t) {
    static const QRegularExpression rx(
        "^[A-Za-z0-9]+([._%+-][A-Za-z0-9]+)*@([A-Za-z0-9]+(-[A-Za-z0-9]+)*\\.)+[A-Za-z]{2,}$");
    return rx.match(t).hasMatch();
}

// Téléphone : exactement 8 chiffres
bool telephoneValide(const QString &t) {
    static const QRegularExpression rx("^[0-9]{8}$");
    return rx.match(t).hasMatch();
}

// ID équipe : entier strictement positif
bool equipeValide(const QString &t) {
    static const QRegularExpression rx("^[1-9][0-9]{0,8}$");
    return rx.match(t).hasMatch();
}

// Identifiants : chiffres uniquement (1 à 9 chiffres)
bool idValide(const QString &t) {
    static const QRegularExpression rx("^[0-9]{1,9}$");
    return rx.match(t).hasMatch();
}

struct ChampSaisie {
    QLineEdit *edit = nullptr;
    QLabel *erreur = nullptr;
    QString messageVide;
    QString messageInvalide;
    bool optionnel = false;
    std::function<bool(const QString&)> valide;

    void montrer(const QString &m) {
        edit->setStyleSheet(kEditErr);
        erreur->setText("⚠ " + m);
        erreur->show();
    }
    void effacer() {
        edit->setStyleSheet(kEditOk);
        erreur->hide();
    }
    // retourne true si le champ est valide
    bool verifier(bool afficherSiVide) {
        const QString t = edit->text();
        if (t.trimmed().isEmpty()) {
            if (optionnel) { effacer(); return true; }
            if (afficherSiVide) montrer(messageVide); else effacer();
            return false;
        }
        if (!valide(t)) {
            montrer(messageInvalide);
            return false;
        }
        effacer();
        return true;
    }
};

void creerChamp(QDialog *dlg, QVBoxLayout *layout, const QString &titre, ChampSaisie &c,
                const QString &placeholder, const QString &msgVide, const QString &msgInvalide,
                std::function<bool(const QString&)> valide, bool optionnel = false)
{
    QLabel *lbl = new QLabel(titre, dlg);
    lbl->setStyleSheet("color: #334155; font-size: 12px; font-weight: 600;");

    c.edit = new QLineEdit(dlg);
    c.edit->setPlaceholderText(placeholder);
    c.edit->setStyleSheet(kEditOk);

    c.erreur = new QLabel(dlg);
    c.erreur->setStyleSheet("color: #dc2626; font-size: 11px; font-weight: bold;");
    c.erreur->hide();

    c.messageVide = msgVide;
    c.messageInvalide = msgInvalide;
    c.valide = valide;
    c.optionnel = optionnel;

    layout->addWidget(lbl);
    layout->addWidget(c.edit);
    layout->addWidget(c.erreur);

    ChampSaisie *pc = &c;
    QObject::connect(c.edit, &QLineEdit::textChanged, dlg, [pc]() { pc->verifier(false); });
}

const QString kDialogStyle =
    "QDialog { background-color: #ffffff; }"
    "QPushButton#dlgOk { background-color: #2563eb; color: white; padding: 9px 18px; border-radius: 8px; font-weight: bold; border: none; }"
    "QPushButton#dlgOk:hover { background-color: #1d4ed8; }"
    "QPushButton#dlgCancel { background-color: #ffffff; color: #0f172a; padding: 9px 18px; border-radius: 8px; font-weight: 600; border: 1px solid #cbd5e1; }"
    "QPushButton#dlgCancel:hover { background-color: #f1f5f9; }";


const QString kStyleCombo =
    "QComboBox { border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px 8px; background-color: #ffffff; color: #0f172a; }"
    "QComboBox QAbstractItemView { background-color: #ffffff; color: #0f172a; selection-background-color: #2563eb; selection-color: #ffffff; }";

QComboBox *creerCombo(QDialog *dlg, QVBoxLayout *layout, const QString &titre)
{
    QLabel *lbl = new QLabel(titre, dlg);
    lbl->setStyleSheet("color: #334155; font-size: 12px; font-weight: 600;");
    QComboBox *cb = new QComboBox(dlg);
    cb->setStyleSheet(kStyleCombo);
    layout->addWidget(lbl);
    layout->addWidget(cb);
    return cb;
}

// Formulaire unique pour "Ajouter" et "Modifier" (contrôle de saisie complet).
// `enregistrer` écrit dans la base ; s'il retourne false, la fenêtre reste ouverte avec le message d'erreur.
bool afficherFormulaire(QWidget *parent, bool modification, const InfoParticipant &init,
                        const std::function<bool(const InfoParticipant &, QString &)> &enregistrer)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(modification ? "Modifier le Participant" : "Ajouter un Participant");
    dlg.setMinimumWidth(440);
    dlg.setStyleSheet(kDialogStyle);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(25, 20, 25, 20);
    layout->setSpacing(4);

    ChampSaisie fId, fNom, fPrenom, fEmail, fTel, fEquipe;

    if (!modification) {
        creerChamp(&dlg, layout, "ID Participant :", fId, "Chiffres uniquement",
                   "L'ID est obligatoire !", "L'ID doit contenir uniquement des chiffres !", idValide);
        fId.edit->setEchoMode(QLineEdit::Password);          // ID masqué à l'écran
    }
    creerChamp(&dlg, layout, "Nom :", fNom, "Lettres uniquement",
               "Le nom est obligatoire !", "Le nom ne doit contenir que des lettres !", nomValide);
    creerChamp(&dlg, layout, "Prénom :", fPrenom, "Lettres uniquement",
               "Le prénom est obligatoire !", "Le prénom ne doit contenir que des lettres !", nomValide);
    creerChamp(&dlg, layout, "E-mail :", fEmail, "exemple@gmail.com",
               "L'e-mail est obligatoire !", "E-mail invalide (ex : nom@domaine.com) !", emailValide);
    creerChamp(&dlg, layout, "Téléphone :", fTel, "8 chiffres",
               "Le téléphone est obligatoire !", "Le téléphone doit contenir exactement 8 chiffres !", telephoneValide);
    creerChamp(&dlg, layout, "ID Équipe (facultatif) :", fEquipe, "Vide = pas d'équipe (4 à 6 membres par équipe)",
               "", "L'ID équipe doit être un nombre positif !", equipeValide, true);

    QComboBox *cbCompetence = creerCombo(&dlg, layout, "Compétence principale :");
    for (const QString &code : Metier::codesCompetences())
        cbCompetence->addItem(Metier::libelleCompetence(code), code);
    QComboBox *cbNiveau = creerCombo(&dlg, layout, "Niveau :");
    for (int n = 1; n <= 3; ++n)
        cbNiveau->addItem(Metier::libelleNiveau(n), n);

    if (modification) {
        fNom.edit->setText(init.nom);
        fPrenom.edit->setText(init.prenom);
        fEmail.edit->setText(init.email);
        fTel.edit->setText(init.telephone);
        fEquipe.edit->setText(init.aEquipe() ? QString::number(init.idEquipe) : QString());
        const int ic = cbCompetence->findData(init.competence);
        if (ic >= 0) cbCompetence->setCurrentIndex(ic);
        const int in = cbNiveau->findData(init.niveau);
        if (in >= 0) cbNiveau->setCurrentIndex(in);
    }

    layout->addSpacing(10);
    QHBoxLayout *btns = new QHBoxLayout();
    btns->addStretch();
    QPushButton *btnAnnuler = new QPushButton("Annuler", &dlg);
    btnAnnuler->setObjectName("dlgCancel");
    QPushButton *btnOk = new QPushButton(modification ? "Modifier" : "Ajouter", &dlg);
    btnOk->setObjectName("dlgOk");
    btns->addWidget(btnAnnuler);
    btns->addWidget(btnOk);
    layout->addLayout(btns);

    QObject::connect(btnAnnuler, &QPushButton::clicked, &dlg, &QDialog::reject);

    bool resultat = false;
    QObject::connect(btnOk, &QPushButton::clicked, &dlg, [&]() {
        QList<ChampSaisie*> champs;
        if (!modification) champs << &fId;
        champs << &fNom << &fPrenom << &fEmail << &fTel << &fEquipe;

        QLineEdit *premierInvalide = nullptr;
        for (ChampSaisie *ch : champs) {
            if (!ch->verifier(true) && !premierInvalide)
                premierInvalide = ch->edit;
        }
        if (premierInvalide) {
            premierInvalide->setFocus();
            return;
        }

        if (!modification) {
            QSqlQuery q;
            q.prepare("SELECT COUNT(*) FROM participant WHERE id_participant = :id");
            q.bindValue(":id", fId.edit->text().toInt());
            if (q.exec() && q.next() && q.value(0).toInt() > 0) {
                fId.montrer("Cet ID existe déjà !");
                fId.edit->setFocus();
                return;
            }
        }

        InfoParticipant r = init;
        if (!modification) r.id = fId.edit->text();
        r.nom = fNom.edit->text().trimmed();
        r.prenom = fPrenom.edit->text().trimmed();
        r.email = fEmail.edit->text();
        r.telephone = fTel.edit->text();
        r.idEquipe = fEquipe.edit->text().trimmed().isEmpty() ? 0 : fEquipe.edit->text().toInt();
        r.competence = cbCompetence->currentData().toString();
        r.niveau = cbNiveau->currentData().toInt();

        if (r.aEquipe()) {                                   // une équipe ne dépasse pas kMaxEquipe membres
            int effectif = Participant::effectifEquipe(r.idEquipe);
            if (modification && init.idEquipe == r.idEquipe) effectif -= 1;
            if (effectif >= kMaxEquipe) {
                fEquipe.montrer(QString("L'équipe %1 est complète (%2 membres maximum) !").arg(r.idEquipe).arg(kMaxEquipe));
                fEquipe.edit->setFocus();
                return;
            }
        }

        QString erreur;
        if (enregistrer(r, erreur)) {
            QMessageBox::information(&dlg, "Succès", modification ? "Participant modifié avec succès !"
                                                                  : "Participant ajouté avec succès !");
            resultat = true;
            dlg.accept();
        } else {
            QMessageBox::critical(&dlg, "Erreur", QString(modification ? "Échec de la modification !" : "Échec d'ajout dans la base SQL !")
                                                      + "\n\n" + erreur);
        }
    });

    dlg.exec();
    return resultat;
}

// Petite "pastille" colorée centrée dans une cellule du tableau
QWidget *creerPastille(const QString &texte, const QString &fond, const QString &couleur, int largeur, const QString &infobulle = QString())
{
    QLabel *l = new QLabel(texte);
    l->setAlignment(Qt::AlignCenter);
    l->setStyleSheet(QString("QLabel { background-color: %1; color: %2; border-radius: 12px; font-weight: bold; font-size: 10px; padding: 0px 2px; }")
                         .arg(fond, couleur));
    l->setFixedSize(largeur, 24);
    if (!infobulle.isEmpty()) l->setToolTip(infobulle);

    QWidget *conteneur = new QWidget();
    QHBoxLayout *lay = new QHBoxLayout(conteneur);
    lay->setContentsMargins(2, 2, 2, 2);
    lay->setAlignment(Qt::AlignCenter);
    lay->addWidget(l);
    return conteneur;
}



// ============================================================================
//  THÈME COMMUN DES FENÊTRES MÉTIERS (bleu nuit du logo + accent orange)
// ============================================================================
const QString kStyleDialogue =
    "QDialog { background-color: #f1f5f9; }"
    "QLabel { color: #0f172a; background: transparent; }"
    "QScrollArea { border: none; background: transparent; }"
    "QScrollArea > QWidget > QWidget { background: transparent; }"
    "QTableWidget { background: #ffffff; border: 1px solid #e2e8f0; gridline-color: #f1f5f9; color: #1e293b; "
    "  alternate-background-color: #f8fafc; selection-background-color: #dbeafe; selection-color: #0f172a; }"
    "QHeaderView::section { background-color: #0d1b2a; color: #f8fafc; border: none; padding: 8px 6px; font-weight: bold; }"
    "QSpinBox, QComboBox, QLineEdit { border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px 8px; background: #ffffff; color: #0f172a; }"
    "QComboBox QAbstractItemView { background: #ffffff; color: #0f172a; selection-background-color: #2563eb; selection-color: #ffffff; }"
    "QListWidget { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 8px; color: #0f172a; padding: 4px; }"
    "QListWidget::item { padding: 9px 10px; border-radius: 6px; }"
    "QListWidget::item:selected { background: #dbeafe; color: #0f172a; }"
    "QListWidget::item:hover { background: #f1f5f9; }"
    "QTabWidget::pane { border: none; background: transparent; }"
    "QTabBar::tab { background: transparent; color: #64748b; padding: 9px 16px; border: none; border-bottom: 3px solid transparent; font-weight: 600; }"
    "QTabBar::tab:selected { color: #0d1b2a; border-bottom: 3px solid #f26b0f; }"
    "QTabBar::tab:hover { color: #2563eb; }";

struct Fenetre {
    std::unique_ptr<QDialog> dlg;
    QVBoxLayout *corps = nullptr;
};

// Fenêtre avec bandeau bleu nuit (titre + sous-titre) et corps blanc cassé
Fenetre creerFenetre(QWidget *parent, const QString &titre, const QString &sousTitre, int largeur, int hauteur)
{
    Fenetre f;
    f.dlg.reset(new QDialog(parent));
    QDialog *dlg = f.dlg.get();
    dlg->setWindowTitle(titre);
    dlg->resize(largeur, hauteur);
    dlg->setStyleSheet(kStyleDialogue);

    QVBoxLayout *root = new QVBoxLayout(dlg);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    QFrame *entete = new QFrame(dlg);
    entete->setObjectName("entete");
    entete->setStyleSheet("QFrame#entete { background-color: #0d1b2a; border-bottom: 3px solid #f26b0f; }");
    QVBoxLayout *el = new QVBoxLayout(entete);
    el->setContentsMargins(24, 16, 24, 14);
    el->setSpacing(3);
    QLabel *t = new QLabel(titre, entete);
    t->setStyleSheet("color: #ffffff; font-size: 19px; font-weight: bold; background: transparent;");
    el->addWidget(t);
    if (!sousTitre.isEmpty()) {
        QLabel *s = new QLabel(sousTitre, entete);
        s->setWordWrap(true);
        s->setStyleSheet("color: #94a3b8; font-size: 12px; background: transparent;");
        el->addWidget(s);
    }
    root->addWidget(entete);

    QWidget *corps = new QWidget(dlg);
    f.corps = new QVBoxLayout(corps);
    f.corps->setContentsMargins(24, 16, 24, 18);
    f.corps->setSpacing(12);
    root->addWidget(corps, 1);
    return f;
}

QPushButton *bouton(const QString &texte, bool primaire, QWidget *parent = nullptr)
{
    QPushButton *b = new QPushButton(texte, parent);
    b->setCursor(Qt::PointingHandCursor);
    if (primaire)
        b->setStyleSheet("QPushButton { background-color: #2563eb; color: white; padding: 9px 20px; border-radius: 8px; font-weight: bold; border: none; }"
                         "QPushButton:hover { background-color: #1d4ed8; }"
                         "QPushButton:disabled { background-color: #cbd5e1; color: #f8fafc; }");
    else
        b->setStyleSheet("QPushButton { background-color: #ffffff; color: #0f172a; padding: 9px 20px; border-radius: 8px; font-weight: 600; border: 1px solid #cbd5e1; }"
                         "QPushButton:hover { background-color: #e2e8f0; }"
                         "QPushButton:disabled { color: #94a3b8; background-color: #f1f5f9; }");
    return b;
}

QLabel *badge(const QString &texte, const QString &fond, const QString &couleur)
{
    QLabel *l = new QLabel(texte);
    l->setAlignment(Qt::AlignCenter);
    l->setStyleSheet(QString("QLabel { background-color: %1; color: %2; border-radius: 9px; padding: 2px 10px; font-size: 11px; font-weight: bold; }")
                         .arg(fond, couleur));
    return l;
}

QFrame *carteKpi(const QString &titre, const QString &valeur, const QString &couleur)
{
    QFrame *f = new QFrame();
    f->setObjectName("kpi");
    f->setStyleSheet("QFrame#kpi { background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 10px; }");
    QVBoxLayout *l = new QVBoxLayout(f);
    l->setContentsMargins(16, 10, 16, 10);
    l->setSpacing(0);
    QLabel *t = new QLabel(titre);
    t->setStyleSheet("color: #64748b; font-size: 11px; font-weight: 600;");
    QLabel *v = new QLabel(valeur);
    v->setStyleSheet(QString("color: %1; font-size: 24px; font-weight: bold;").arg(couleur));
    l->addWidget(t);
    l->addWidget(v);
    return f;
}

void viderLayout(QLayout *l)
{
    while (QLayoutItem *it = l->takeAt(0)) {
        if (QWidget *w = it->widget()) w->deleteLater();
        if (QLayout *sub = it->layout()) { viderLayout(sub); }
        delete it;
    }
}

QTableWidgetItem *cellule(const QString &texte, Qt::Alignment al = Qt::AlignVCenter | Qt::AlignLeft)
{
    QTableWidgetItem *it = new QTableWidgetItem(texte);
    it->setTextAlignment(al);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    return it;
}

void configurerTable(QTableWidget *t)
{
    t->verticalHeader()->setVisible(false);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->setShowGrid(false);
    t->verticalHeader()->setDefaultSectionSize(36);
    t->horizontalHeader()->setHighlightSections(false);
}

QString desktop() { return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation); }

QString mailto(const QStringList &bcc, const QString &sujet, const QString &corps)
{
    QString url = "mailto:?subject=" + QString::fromUtf8(QUrl::toPercentEncoding(sujet));
    if (!bcc.isEmpty())
        url += "&bcc=" + QString::fromUtf8(QUrl::toPercentEncoding(bcc.join(","), "@,"));
    url += "&body=" + QString::fromUtf8(QUrl::toPercentEncoding(corps));
    return url;
}

QString nomCompetenceAffiche(const InfoParticipant &p)
{
    return p.competence.isEmpty() ? QString("-") : Metier::libelleCompetence(p.competence);
}

void couleursStatut(const QString &statut, QString &fond, QString &texte)
{
    fond = "#e2e8f0"; texte = "#475569";
    if (statut == "AFFECTE")      { fond = "#dbeafe"; texte = "#1e40af"; }
    else if (statut == "VALIDE")  { fond = "#dcfce7"; texte = "#166534"; }
    else if (statut == "INACTIF") { fond = "#fee2e2"; texte = "#991b1b"; }
}

QString dateTexte(const QDate &d) { return d.isValid() ? d.toString("dd/MM/yyyy") : QString("-"); }

// Participants d'une équipe (données SQL réelles)
QList<InfoParticipant> membresEquipe(int idEquipe)
{
    QList<InfoParticipant> res;
    if (idEquipe <= 0) return res;
    for (const InfoParticipant &p : Participant::lister())
        if (p.idEquipe == idEquipe) res.append(p);
    return res;
}

QString etatEquipe(int effectif)
{
    if (effectif >= kMinEquipe) return "Complète";
    return QString("Incomplète (%1 membre%2 manquant%2)")
        .arg(kMinEquipe - effectif).arg(kMinEquipe - effectif > 1 ? "s" : "");
}

QString pastilleInitiales(const InfoParticipant &p)
{
    return (p.prenom.left(1) + p.nom.left(1)).toUpper();
}

QLabel *avatar(const InfoParticipant &p, const QString &fond = "#0d1b2a")
{
    QLabel *a = new QLabel(pastilleInitiales(p));
    a->setFixedSize(30, 30);
    a->setAlignment(Qt::AlignCenter);
    a->setStyleSheet(QString("QLabel { background-color: %1; color: #ffffff; border-radius: 15px; font-weight: bold; font-size: 11px; }").arg(fond));
    return a;
}

// ============================================================================
//  MÉTIER 1 : composition intelligente des équipes (minimum 4 membres)
// ============================================================================
QFrame *carteEquipe(const Metier::EquipeProposee &eq)
{
    QFrame *f = new QFrame();
    f->setObjectName("carte");
    f->setStyleSheet("QFrame#carte { background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 12px; }");
    QVBoxLayout *l = new QVBoxLayout(f);
    l->setContentsMargins(16, 12, 16, 12);
    l->setSpacing(8);

    QHBoxLayout *haut = new QHBoxLayout();
    QLabel *titre = new QLabel(QString("Équipe %1").arg(eq.idEquipe));
    titre->setStyleSheet("font-size: 15px; font-weight: bold; color: #0d1b2a;");
    haut->addWidget(titre);

    QString fond = "#dbeafe", texte = "#1e40af", libelle = "Nouvelle équipe";
    if (eq.etat == "Complétée")      { fond = "#dcfce7"; texte = "#166534"; libelle = "Équipe complétée"; }
    else if (eq.etat == "Incomplète") { fond = "#ffedd5"; texte = "#9a3412"; libelle = "Reste incomplète"; }
    haut->addWidget(badge(libelle, fond, texte));
    haut->addStretch();
    QLabel *score = new QLabel(QString("Compatibilité <b>%1</b> / 100").arg(eq.score));
    score->setStyleSheet("color: #475569; font-size: 12px;");
    haut->addWidget(score);
    l->addLayout(haut);

    QLabel *info = new QLabel(QString("%1 membre(s)  ·  niveau moyen %2  ·  %3 compétence(s) différente(s)")
                                  .arg(eq.membres.size()).arg(QString::number(eq.niveauMoyen, 'f', 1)).arg(eq.nbCompetences));
    info->setStyleSheet("color: #64748b; font-size: 11px;");
    l->addWidget(info);

    QFrame *ligne = new QFrame();
    ligne->setFixedHeight(1);
    ligne->setStyleSheet("background-color: #e2e8f0;");
    l->addWidget(ligne);

    for (int i = 0; i < eq.membres.size(); ++i) {
        const InfoParticipant &m = eq.membres[i];
        const bool nouveau = i < eq.nouveau.size() && eq.nouveau[i];
        QHBoxLayout *r = new QHBoxLayout();
        r->setSpacing(10);
        r->addWidget(avatar(m, nouveau ? "#2563eb" : "#64748b"));
        QLabel *nom = new QLabel(m.nomComplet());
        nom->setStyleSheet("font-weight: 600; color: #0f172a;");
        r->addWidget(nom);
        r->addStretch();
        r->addWidget(badge(nomCompetenceAffiche(m), "#f1f5f9", "#334155"));
        r->addWidget(badge(Metier::libelleNiveau(m.niveau), "#f1f5f9", "#334155"));
        QLabel *tag = new QLabel(nouveau ? "à affecter" : "déjà membre");
        tag->setFixedWidth(80);
        tag->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tag->setStyleSheet(nouveau ? "color: #2563eb; font-size: 11px; font-weight: bold;" : "color: #94a3b8; font-size: 11px;");
        r->addWidget(tag);
        l->addLayout(r);
    }
    return f;
}

bool compositionEquipes(QWidget *parent)
{
    const QList<InfoParticipant> tous = Participant::lister();
    int sans = 0;
    QMap<int, int> effectifs;
    for (const InfoParticipant &p : tous) {
        if (p.statut == "INACTIF") continue;
        if (p.aEquipe()) effectifs[p.idEquipe]++; else sans++;
    }
    int incompletes = 0;
    for (auto it = effectifs.constBegin(); it != effectifs.constEnd(); ++it)
        if (it.value() < kMinEquipe) incompletes++;

    if (sans == 0) {
        QMessageBox::information(parent, "Composition d'équipes",
                                 incompletes > 0
                                     ? QString("%1 équipe(s) ont moins de %2 membres, mais aucun participant sans équipe n'est disponible pour les compléter.\n"
                                               "Ajoutez de nouveaux participants (sans ID équipe).").arg(incompletes).arg(kMinEquipe)
                                     : QString("Tous les participants actifs ont déjà une équipe de %1 membres ou plus.").arg(kMinEquipe));
        return false;
    }

    bool modifie = false;
    Fenetre fen = creerFenetre(parent, "Composition intelligente des équipes",
        QString("Règle : chaque équipe doit avoir au minimum %1 participants (maximum %2). "
                "On complète d'abord les équipes incomplètes, puis on crée de nouvelles équipes équilibrées.")
            .arg(kMinEquipe).arg(kMaxEquipe), 860, 660);
    QDialog *dlg = fen.dlg.get();
    QVBoxLayout *corps = fen.corps;

    QHBoxLayout *controles = new QHBoxLayout();
    controles->addWidget(new QLabel("ID de la première nouvelle équipe :"));
    QSpinBox *spPremier = new QSpinBox();
    spPremier->setRange(1, 999999);
    spPremier->setValue(Participant::prochainIdEquipe());
    spPremier->setFixedWidth(100);
    controles->addWidget(spPremier);
    controles->addStretch();
    corps->addLayout(controles);

    QLabel *resume = new QLabel();
    resume->setTextFormat(Qt::RichText);
    resume->setStyleSheet("color: #334155; font-size: 12px;");
    corps->addWidget(resume);

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    QWidget *contenu = new QWidget();
    QVBoxLayout *zone = new QVBoxLayout(contenu);
    zone->setContentsMargins(0, 0, 6, 0);
    zone->setSpacing(10);
    scroll->setWidget(contenu);
    corps->addWidget(scroll, 1);

    QHBoxLayout *boutons = new QHBoxLayout();
    boutons->addStretch();
    QPushButton *btnFermer = bouton("Fermer", false);
    QPushButton *btnAppliquer = bouton("Appliquer cette composition", true);
    boutons->addWidget(btnFermer);
    boutons->addWidget(btnAppliquer);
    corps->addLayout(boutons);

    Metier::ResultatComposition res;

    auto reconstruire = [&]() {
        res = Metier::composerEquipes(tous, kMinEquipe, spPremier->value());
        viderLayout(zone);
        int aAffecter = 0, nouvelles = 0, completees = 0;
        for (const Metier::EquipeProposee &eq : res.equipes) {
            zone->addWidget(carteEquipe(eq));
            aAffecter += eq.nbNouveaux();
            if (eq.existante) completees++; else nouvelles++;
        }
        if (!res.enAttente.isEmpty()) {
            QFrame *f = new QFrame();
            f->setObjectName("attente");
            f->setStyleSheet("QFrame#attente { background-color: #fff7ed; border: 1px dashed #fb923c; border-radius: 12px; }");
            QVBoxLayout *l = new QVBoxLayout(f);
            l->setContentsMargins(16, 12, 16, 12);
            QLabel *t = new QLabel(QString("<b>En attente (%1)</b>").arg(res.enAttente.size()));
            t->setStyleSheet("color: #9a3412; font-size: 14px;");
            l->addWidget(t);
            QStringList noms;
            for (const InfoParticipant &p : res.enAttente) noms << p.nomComplet();
            QLabel *n = new QLabel(noms.join("  ·  "));
            n->setWordWrap(true);
            n->setStyleSheet("color: #7c2d12;");
            l->addWidget(n);
            QLabel *m = new QLabel(QString("Une équipe doit avoir au moins %1 membres : il manque %2 participant(s) pour former une nouvelle équipe.")
                                       .arg(kMinEquipe).arg(kMinEquipe - res.enAttente.size()));
            m->setWordWrap(true);
            m->setStyleSheet("color: #9a3412; font-size: 11px;");
            l->addWidget(m);
            zone->addWidget(f);
        }
        zone->addStretch();
        resume->setText(QString("<b>%1</b> participant(s) à affecter  ·  <b>%2</b> équipe(s) existante(s) complétée(s)  ·  "
                                "<b>%3</b> nouvelle(s) équipe(s)  ·  <b>%4</b> en attente")
                            .arg(aAffecter).arg(completees).arg(nouvelles).arg(res.enAttente.size()));
        btnAppliquer->setEnabled(aAffecter > 0);
    };
    QObject::connect(spPremier, QOverload<int>::of(&QSpinBox::valueChanged), dlg, [&](int) { reconstruire(); });
    reconstruire();

    QObject::connect(btnAppliquer, &QPushButton::clicked, dlg, [&]() {
        QList<QPair<QString, int>> affectations;
        for (const Metier::EquipeProposee &eq : res.equipes) {
            if (!eq.existante && Participant::effectifEquipe(eq.idEquipe) > 0) {
                QMessageBox::warning(dlg, "ID déjà utilisé",
                    QString("L'équipe %1 existe déjà. Choisissez un autre « ID de la première nouvelle équipe ».").arg(eq.idEquipe));
                return;
            }
            for (int i = 0; i < eq.membres.size(); ++i)
                if (i < eq.nouveau.size() && eq.nouveau[i])
                    affectations.append({eq.membres[i].id, eq.idEquipe});
        }
        if (affectations.isEmpty()) return;

        QSqlDatabase db = QSqlDatabase::database();
        const bool tx = db.transaction();
        bool ok = true;
        for (const auto &a : affectations)
            if (!Participant::affecterEquipe(a.first, a.second)) { ok = false; break; }
        if (tx) { if (ok) db.commit(); else db.rollback(); }

        if (ok) {
            modifie = true;
            QMessageBox::information(dlg, "Succès",
                QString("%1 participant(s) affecté(s). Ils passent au statut « Affecté ».").arg(affectations.size()));
            dlg->accept();
        } else {
            QMessageBox::critical(dlg, "Erreur", "L'affectation a échoué, aucune modification n'a été enregistrée.");
        }
    });
    QObject::connect(btnFermer, &QPushButton::clicked, dlg, &QDialog::reject);
    dlg->exec();
    return modifie;
}

// Affectation manuelle d'un participant (une équipe ne dépasse pas kMaxEquipe)
bool affecterManuellement(QWidget *parent, const InfoParticipant &p)
{
    bool ok = false;
    const int actuel = p.aEquipe() ? p.idEquipe : Participant::prochainIdEquipe();
    const int equipe = QInputDialog::getInt(parent, "Affecter à une équipe",
                                            QString("ID de l'équipe pour %1 :").arg(p.nomComplet()),
                                            actuel, 1, 999999, 1, &ok);
    if (!ok) return false;

    int effectif = Participant::effectifEquipe(equipe);
    if (p.idEquipe == equipe) effectif -= 1;
    if (effectif >= kMaxEquipe) {
        QMessageBox::warning(parent, "Équipe complète",
                             QString("L'équipe %1 a déjà %2 membres (maximum).\nChoisissez une autre équipe.").arg(equipe).arg(kMaxEquipe));
        return false;
    }
    if (!Participant::affecterEquipe(p.id, equipe)) {
        QMessageBox::critical(parent, "Erreur", "L'affectation a échoué.");
        return false;
    }
    const int nouveau = effectif + 1;
    QString msg = QString("%1 est affecté(e) à l'équipe %2 (%3 membre(s)).").arg(p.nomComplet()).arg(equipe).arg(nouveau);
    if (nouveau < kMinEquipe)
        msg += QString("\n\nAttention : une équipe doit avoir au minimum %1 membres (il en manque encore %2).")
                   .arg(kMinEquipe).arg(kMinEquipe - nouveau);
    QMessageBox::information(parent, "Succès", msg);
    return true;
}

// ============================================================================
//  MÉTIER 2 : engagement, participants à risque, relances, escalade
// ============================================================================
bool relances(QWidget *parent)
{
    bool modifie = false;
    Fenetre fen = creerFenetre(parent, "Engagement des participants",
        "Score /100 = profil (20) + équipe (25) + statut (25) + réactivité (20) + relances (10).  "
        "Actif ≥ 70  ·  À surveiller 40-69  ·  À risque < 40.  Après 2 relances sans réponse : escalade possible.", 1000, 640);
    QDialog *dlg = fen.dlg.get();
    QVBoxLayout *corps = fen.corps;

    QHBoxLayout *kpis = new QHBoxLayout();
    kpis->setSpacing(10);
    corps->addLayout(kpis);

    QTableWidget *table = new QTableWidget(0, 6);
    table->setHorizontalHeaderLabels({"Participant", "E-mail", "Score", "Catégorie", "Relances", "Motif"});
    configurerTable(table);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    corps->addWidget(table, 1);

    QLabel *aide = new QLabel("Les participants à risque sont présélectionnés. Utilisez Ctrl / Maj pour modifier la sélection.");
    aide->setStyleSheet("color: #64748b; font-size: 11px;");
    corps->addWidget(aide);

    QHBoxLayout *boutons = new QHBoxLayout();
    QPushButton *btnRelance = bouton("Envoyer une relance (e-mail)", true);
    QPushButton *btnEscalade = bouton("Escalade : marquer Inactif", false);
    QPushButton *btnFermer = bouton("Fermer", false);
    boutons->addWidget(btnRelance);
    boutons->addWidget(btnEscalade);
    boutons->addStretch();
    boutons->addWidget(btnFermer);
    corps->addLayout(boutons);

    QList<InfoParticipant> affiches;

    auto remplir = [&]() {
        affiches.clear();
        table->setRowCount(0);
        viderLayout(kpis);
        int nbRisque = 0, nbSurveiller = 0, nbNouveau = 0, nbEscalade = 0;

        struct Ligne { InfoParticipant p; Metier::Engagement e; };
        QList<Ligne> lignes;
        for (const InfoParticipant &p : Participant::lister()) {
            const Metier::Engagement e = Metier::calculerEngagement(p);
            if (e.categorie == "Inactif") continue;
            if (e.categorie == "Nouveau") { nbNouveau++; continue; }
            if (e.categorie == "À risque") { nbRisque++; lignes.append({p, e}); }
            else if (e.categorie == "À surveiller") { nbSurveiller++; lignes.append({p, e}); }
            if (e.categorie != "Actif" && p.nbRelances >= 2) nbEscalade++;
        }
        std::sort(lignes.begin(), lignes.end(), [](const Ligne &a, const Ligne &b) { return a.e.score < b.e.score; });

        for (const Ligne &l : lignes) {
            const int r = table->rowCount();
            table->insertRow(r);
            affiches.append(l.p);
            table->setItem(r, 0, cellule(l.p.nomComplet()));
            table->setItem(r, 1, cellule(l.p.email));
            table->setItem(r, 2, cellule(QString::number(l.e.score), Qt::AlignCenter));
            QTableWidgetItem *cat = cellule(l.e.categorie, Qt::AlignCenter);
            cat->setForeground(QColor(l.e.couleurTexte));
            QFont f = cat->font(); f.setBold(true); cat->setFont(f);
            table->setItem(r, 3, cat);
            table->setItem(r, 4, cellule(QString::number(l.p.nbRelances), Qt::AlignCenter));
            table->setItem(r, 5, cellule(Metier::motifRisque(l.p)));
            if (l.e.categorie == "À risque")
                table->selectionModel()->select(table->model()->index(r, 0),
                                                QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        kpis->addWidget(carteKpi("À risque", QString::number(nbRisque), "#dc2626"));
        kpis->addWidget(carteKpi("À surveiller", QString::number(nbSurveiller), "#ea580c"));
        kpis->addWidget(carteKpi("Nouveaux (période de grâce)", QString::number(nbNouveau), "#2563eb"));
        kpis->addWidget(carteKpi("Escalade possible", QString::number(nbEscalade), "#0d1b2a"));
        btnRelance->setEnabled(!lignes.isEmpty());
        btnEscalade->setEnabled(!lignes.isEmpty());
    };
    remplir();

    auto selection = [&]() {
        QList<InfoParticipant> sel;
        QSet<int> lignes;
        for (const QModelIndex &i : table->selectionModel()->selectedRows())
            lignes.insert(i.row());
        for (int r : lignes) sel.append(affiches[r]);
        return sel;
    };

    QObject::connect(btnRelance, &QPushButton::clicked, dlg, [&]() {
        const QList<InfoParticipant> sel = selection();
        if (sel.isEmpty()) {
            QMessageBox::information(dlg, "Relance", "Sélectionnez au moins un participant dans la liste.");
            return;
        }
        QStringList emails;
        for (const InfoParticipant &p : sel)
            if (emailValide(p.email)) emails << p.email;
        if (emails.isEmpty()) {
            QMessageBox::warning(dlg, "Relance", "Aucun e-mail valide parmi les participants sélectionnés.");
            return;
        }
        const QString sujet = "HackTime - Rappel : finalisez votre participation au hackathon";
        const QString texte = "Bonjour,\n\n"
                              "Nous avons remarqué que votre inscription au hackathon n'est pas finalisée "
                              "(équipe non attribuée, profil incomplet ou statut en attente).\n"
                              "Merci de nous répondre ou de compléter votre profil dans les plus brefs délais afin de conserver votre place.\n\n"
                              "Cordialement,\nL'équipe d'organisation HackTime";
        QDesktopServices::openUrl(QUrl(mailto(emails, sujet, texte)));

        const auto rep = QMessageBox::question(dlg, "Enregistrer la relance",
            QString("Le brouillon d'e-mail a été ouvert pour %1 participant(s).\n\nEnregistrer cette relance (compteur +1) ?").arg(emails.size()));
        if (rep == QMessageBox::Yes) {
            for (const InfoParticipant &p : sel)
                if (emailValide(p.email)) Participant::enregistrerRelance(p.id);
            modifie = true;
            remplir();
        }
    });

    QObject::connect(btnEscalade, &QPushButton::clicked, dlg, [&]() {
        QList<InfoParticipant> eligibles;
        for (const InfoParticipant &p : selection())
            if (p.nbRelances >= 2) eligibles.append(p);
        if (eligibles.isEmpty()) {
            QMessageBox::information(dlg, "Escalade", "Aucun des participants sélectionnés n'a atteint 2 relances sans réponse.");
            return;
        }
        QStringList noms;
        for (const InfoParticipant &p : eligibles) noms << p.nomComplet();
        const auto rep = QMessageBox::question(dlg, "Escalade",
            QString("Marquer %1 participant(s) comme Inactif(s) ?\n\n%2\n\nLeur place pourra être proposée à la liste d'attente.")
                .arg(eligibles.size()).arg(noms.join("\n")));
        if (rep != QMessageBox::Yes) return;
        for (const InfoParticipant &p : eligibles)
            Participant::changerStatut(p.id, "INACTIF");
        modifie = true;
        remplir();
    });

    QObject::connect(btnFermer, &QPushButton::clicked, dlg, &QDialog::accept);
    dlg->exec();
    return modifie;
}

// ============================================================================
//  Workflow de statut : Inscrit → Affecté → Validé (→ Inactif)
//  Règle : « Validé » exige une équipe d'au moins 4 membres.
// ============================================================================
bool changerStatut(QWidget *parent, const InfoParticipant &p)
{
    const int effectif = p.aEquipe() ? Participant::effectifEquipe(p.idEquipe) : 0;
    const bool equipeOk = p.aEquipe() && effectif >= kMinEquipe;

    QStringList codes;
    QString aide;
    if (p.statut == "INSCRIT") {
        if (p.aEquipe()) codes << "AFFECTE";
        else aide = "Affectez d'abord ce participant à une équipe pour pouvoir le passer « Affecté » puis « Validé ».";
        codes << "INACTIF";
    } else if (p.statut == "AFFECTE") {
        if (equipeOk) codes << "VALIDE";
        else aide = QString("Validation impossible : l'équipe %1 n'a que %2 membre(s). Une équipe doit avoir au minimum %3 membres.")
                        .arg(p.idEquipe).arg(effectif).arg(kMinEquipe);
        codes << "INACTIF";
    } else if (p.statut == "VALIDE") {
        codes << "AFFECTE" << "INACTIF";
    } else {
        codes << "INSCRIT";
    }

    Fenetre fen = creerFenetre(parent, "Valider le statut", "Workflow : Inscrit → Affecté → Validé", 480, 380);
    QDialog *dlg = fen.dlg.get();
    QVBoxLayout *corps = fen.corps;

    QHBoxLayout *id = new QHBoxLayout();
    id->addWidget(avatar(p));
    QLabel *nom = new QLabel(QString("<b>%1</b><br><span style='color:#64748b'>%2</span>").arg(p.nomComplet(), p.email));
    id->addWidget(nom);
    id->addStretch();
    corps->addLayout(id);

    QHBoxLayout *actuel = new QHBoxLayout();
    actuel->addWidget(new QLabel("Statut actuel :"));
    QString fond, texte;
    couleursStatut(p.statut, fond, texte);
    actuel->addWidget(badge(Metier::libelleStatut(p.statut), fond, texte));
    actuel->addStretch();
    corps->addLayout(actuel);

    QLabel *lbl = new QLabel("Nouveau statut :");
    lbl->setStyleSheet("color: #334155; font-weight: 600;");
    corps->addWidget(lbl);
    QComboBox *cb = new QComboBox();
    for (const QString &c : codes) cb->addItem(Metier::libelleStatut(c), c);
    corps->addWidget(cb);

    if (!aide.isEmpty()) {
        QLabel *a = new QLabel("⚠ " + aide);
        a->setWordWrap(true);
        a->setStyleSheet("color: #9a3412; background-color: #fff7ed; border: 1px solid #fed7aa; border-radius: 8px; padding: 8px; font-size: 12px;");
        corps->addWidget(a);
    }
    corps->addStretch();

    bool modifie = false;
    QHBoxLayout *boutons = new QHBoxLayout();
    boutons->addStretch();
    QPushButton *btnAnnuler = bouton("Annuler", false);
    QPushButton *btnOk = bouton("Enregistrer", true);
    boutons->addWidget(btnAnnuler);
    boutons->addWidget(btnOk);
    corps->addLayout(boutons);
    QObject::connect(btnAnnuler, &QPushButton::clicked, dlg, &QDialog::reject);
    QObject::connect(btnOk, &QPushButton::clicked, dlg, [&]() {
        if (!Participant::changerStatut(p.id, cb->currentData().toString())) {
            QMessageBox::critical(dlg, "Erreur", "Le changement de statut a échoué.");
            return;
        }
        modifie = true;
        dlg->accept();
    });
    dlg->exec();
    return modifie;
}

// ============================================================================
//  Statistiques réelles (données SQL)
// ============================================================================
void statistiques(QWidget *parent)
{
    const QList<InfoParticipant> liste = Participant::lister();
    const int total = liste.size();

    int avecEquipe = 0, valides = 0, esprit = 0, gmail = 0;
    double sommeScore = 0;
    QMap<int, int> parEquipe;
    QMap<QString, int> parStatut, parCategorie, parCompetence;
    int niv[4] = {0, 0, 0, 0};
    QMap<QDate, int> parJour;

    for (const InfoParticipant &p : liste) {
        if (p.aEquipe()) { avecEquipe++; parEquipe[p.idEquipe]++; }
        if (p.statut == "VALIDE") valides++;
        parStatut[p.statut]++;
        const Metier::Engagement e = Metier::calculerEngagement(p);
        sommeScore += e.score;
        parCategorie[e.categorie]++;
        parCompetence[p.competence.isEmpty() ? QString("-") : p.competence]++;
        niv[std::min(3, std::max(1, p.niveau))]++;
        if (p.dateInscription.isValid()) parJour[p.dateInscription]++;
        const QString mail = p.email.toLower();
        if (mail.endsWith("@esprit.tn")) esprit++;
        else if (mail.endsWith("@gmail.com")) gmail++;
    }
    const int autres = std::max(0, total - esprit - gmail);
    const int sansEquipe = total - avecEquipe;
    const double scoreMoyen = total > 0 ? sommeScore / total : 0;

    Fenetre fen = creerFenetre(parent, "Statistiques des participants", "Données en temps réel issues de la base SQL", 900, 660);
    QDialog *dlg = fen.dlg.get();
    QVBoxLayout *root = fen.corps;

    QTabWidget *tabs = new QTabWidget();
    root->addWidget(tabs, 1);

    auto nouvelOnglet = [&](const QString &titre) {
        QWidget *w = new QWidget();
        QVBoxLayout *l = new QVBoxLayout(w);
        l->setContentsMargins(0, 12, 0, 0);
        l->setSpacing(10);
        tabs->addTab(w, titre);
        return l;
    };

    {   // Vue d'ensemble
        QVBoxLayout *l = nouvelOnglet("Vue d'ensemble");
        QHBoxLayout *cartes = new QHBoxLayout();
        cartes->addWidget(carteKpi("Participants", QString::number(total), "#0d1b2a"));
        cartes->addWidget(carteKpi("Avec équipe", QString::number(avecEquipe), "#2563eb"));
        cartes->addWidget(carteKpi("Sans équipe", QString::number(sansEquipe), "#ea580c"));
        cartes->addWidget(carteKpi("Validés", QString::number(valides), "#16a34a"));
        cartes->addWidget(carteKpi("Engagement moyen", QString::number(scoreMoyen, 'f', 0) + " / 100", "#7c3aed"));
        l->addLayout(cartes);
        ChartWidget *c = new ChartWidget();
        c->setDonnees({{"Inscrit", double(parStatut["INSCRIT"]), QColor("#64748b")},
                       {"Affecté", double(parStatut["AFFECTE"]), QColor("#2563eb")},
                       {"Validé", double(parStatut["VALIDE"]), QColor("#16a34a")},
                       {"Inactif", double(parStatut["INACTIF"]), QColor("#dc2626")}},
                      ChartWidget::Barres, "Répartition par statut (workflow)");
        l->addWidget(c, 1);
    }
    {   // Équipes (minimum 4)
        QVBoxLayout *l = nouvelOnglet("Équipes");
        QList<ChartWidget::Point> pts;
        int completes = 0, incompletes = 0;
        for (auto it = parEquipe.constBegin(); it != parEquipe.constEnd(); ++it) {
            const bool complete = it.value() >= kMinEquipe;
            if (complete) completes++; else incompletes++;
            pts.append({QString("Éq. %1").arg(it.key()), double(it.value()), complete ? QColor("#16a34a") : QColor("#f59e0b")});
        }
        ChartWidget *c = new ChartWidget();
        c->setDonnees(pts, ChartWidget::Barres, QString("Participants par équipe (minimum requis : %1)").arg(kMinEquipe));
        l->addWidget(c, 1);
        QLabel *r = new QLabel(QString("%1 équipe(s)  ·  <span style='color:#16a34a'><b>%2 complète(s) (≥ %3)</b></span>  ·  "
                                       "<span style='color:#d97706'><b>%4 incomplète(s)</b></span>  ·  "
                                       "<span style='color:#ea580c'><b>%5 participant(s) sans équipe</b></span>")
                                   .arg(parEquipe.size()).arg(completes).arg(kMinEquipe).arg(incompletes).arg(sansEquipe));
        r->setTextFormat(Qt::RichText);
        l->addWidget(r);
    }
    {   // Engagement
        QVBoxLayout *l = nouvelOnglet("Engagement");
        ChartWidget *c = new ChartWidget();
        c->setDonnees({{"Actif", double(parCategorie["Actif"]), QColor("#16a34a")},
                       {"Nouveau", double(parCategorie["Nouveau"]), QColor("#2563eb")},
                       {"À surveiller", double(parCategorie["À surveiller"]), QColor("#f59e0b")},
                       {"À risque", double(parCategorie["À risque"]), QColor("#dc2626")},
                       {"Inactif", double(parCategorie["Inactif"]), QColor("#64748b")}},
                      ChartWidget::Barres, "Répartition par niveau d'engagement");
        l->addWidget(c, 1);
        l->addWidget(new QLabel(QString("Score d'engagement moyen : <b>%1 / 100</b>").arg(QString::number(scoreMoyen, 'f', 1))));
    }
    {   // Inscriptions
        QVBoxLayout *l = nouvelOnglet("Inscriptions");
        QList<ChartWidget::Point> pts;
        const QDate aujourdhui = QDate::currentDate();
        for (int i = 13; i >= 0; --i) {
            const QDate d = aujourdhui.addDays(-i);
            pts.append({d.toString("dd/MM"), double(parJour.value(d, 0)), QColor("#2563eb")});
        }
        ChartWidget *c = new ChartWidget();
        c->setDonnees(pts, ChartWidget::Courbe, "Inscriptions par jour (14 derniers jours)");
        l->addWidget(c, 1);
    }
    {   // Compétences
        QVBoxLayout *l = nouvelOnglet("Compétences");
        QList<ChartWidget::Point> pts;
        const QStringList couleurs = {"#2563eb", "#7c3aed", "#0891b2", "#ea580c", "#16a34a"};
        int i = 0;
        for (const QString &code : Metier::codesCompetences())
            pts.append({Metier::libelleCompetence(code), double(parCompetence.value(code, 0)), QColor(couleurs[i++ % couleurs.size()])});
        pts.append({"Non renseigné", double(parCompetence.value("-", 0)), QColor("#94a3b8")});
        ChartWidget *c = new ChartWidget();
        c->setDonnees(pts, ChartWidget::Barres, "Répartition par compétence");
        l->addWidget(c, 1);
        l->addWidget(new QLabel(QString("Niveaux : <b>%1</b> débutant(s)  ·  <b>%2</b> intermédiaire(s)  ·  <b>%3</b> avancé(s)")
                                    .arg(niv[1]).arg(niv[2]).arg(niv[3])));
    }
    {   // E-mails
        QVBoxLayout *l = nouvelOnglet("E-mails");
        ChartWidget *c = new ChartWidget();
        c->setDonnees({{"ESPRIT", double(esprit), QColor("#dc2626")},
                       {"Gmail", double(gmail), QColor("#2563eb")},
                       {"Autres", double(autres), QColor("#16a34a")}},
                      ChartWidget::Barres, "Répartition par domaine e-mail");
        l->addWidget(c, 1);
        const double pct = total > 0 ? esprit * 100.0 / total : 0.0;
        l->addWidget(new QLabel(QString("Étudiants ESPRIT (@esprit.tn) : <b>%1</b> (%2 %)").arg(esprit).arg(QString::number(pct, 'f', 1))));
    }

    QHBoxLayout *boutons = new QHBoxLayout();
    QPushButton *btnPng = bouton("Exporter l'onglet en PNG", false);
    QPushButton *btnFermer = bouton("Fermer", true);
    boutons->addWidget(btnPng);
    boutons->addStretch();
    boutons->addWidget(btnFermer);
    root->addLayout(boutons);

    QObject::connect(btnPng, &QPushButton::clicked, dlg, [&]() {
        const QString f = QFileDialog::getSaveFileName(dlg, "Exporter en PNG", desktop() + "/statistiques_participants.png", "Images PNG (*.png)");
        if (f.isEmpty()) return;
        if (tabs->currentWidget()->grab().save(f, "PNG"))
            QMessageBox::information(dlg, "Succès", "Image exportée avec succès !");
        else
            QMessageBox::critical(dlg, "Erreur", "Impossible d'enregistrer l'image.");
    });
    QObject::connect(btnFermer, &QPushButton::clicked, dlg, &QDialog::accept);
    dlg->exec();
}

QString logoBase64()
{
    QByteArray logo;
    QBuffer buf(&logo);
    buf.open(QIODevice::WriteOnly);
    createHackTimeLogoPixmap().save(&buf, "PNG");
    return QString::fromLatin1(logo.toBase64());
}

void imprimerHtml(const QString &html, const QString &fichier, QPageLayout::Orientation orientation, int resolution)
{
    QPdfWriter pdf(fichier);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageOrientation(orientation);
    pdf.setResolution(resolution);
    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&pdf);
}

// ============================================================================
//  Rapport de progression (PDF)
// ============================================================================
void rapportProgression(QWidget *parent)
{
    const QList<InfoParticipant> liste = Participant::lister();
    if (liste.isEmpty()) {
        QMessageBox::information(parent, "Rapport", "Aucun participant à analyser.");
        return;
    }
    const QString fichier = QFileDialog::getSaveFileName(parent, "Générer le rapport de progression",
                                desktop() + "/rapport_progression.pdf", "Documents PDF (*.pdf)");
    if (fichier.isEmpty()) return;

    const int total = liste.size();
    QMap<QString, int> parStatut, parCat;
    QMap<int, QList<InfoParticipant>> equipes;
    double somme = 0;
    int sansEquipe = 0;
    struct Risque { InfoParticipant p; int score; };
    QList<Risque> risques;
    for (const InfoParticipant &p : liste) {
        const Metier::Engagement e = Metier::calculerEngagement(p);
        somme += e.score;
        parStatut[p.statut]++;
        parCat[e.categorie]++;
        if (p.aEquipe()) equipes[p.idEquipe].append(p); else sansEquipe++;
        if (e.categorie == "À risque") risques.append({p, e.score});
    }
    std::sort(risques.begin(), risques.end(), [](const Risque &a, const Risque &b) { return a.score < b.score; });

    auto pct = [&](int n) { return QString::number(total > 0 ? n * 100.0 / total : 0.0, 'f', 1) + " %"; };

    int incompletes = 0;
    for (auto it = equipes.constBegin(); it != equipes.constEnd(); ++it)
        if (it.value().size() < kMinEquipe) incompletes++;

    QString html =
        "<html><head><style>"
        "body { font-family: Helvetica, Arial, sans-serif; color: #0f172a; }"
        "h1 { color: #0d1b2a; font-size: 22px; margin: 6px 0 2px 0; }"
        "h2 { color: #2563eb; font-size: 15px; margin: 18px 0 6px 0; border-bottom: 1px solid #cbd5e1; }"
        "p { color: #475569; font-size: 11px; }"
        "table { border-collapse: collapse; width: 100%; }"
        "th { background-color: #0d1b2a; color: #ffffff; font-size: 11px; padding: 6px; text-align: left; border: 1px solid #0d1b2a; }"
        "td { font-size: 11px; padding: 6px; border: 1px solid #cbd5e1; }"
        "</style></head><body>";
    html += "<table width='100%'><tr><td style='border:none; background-color:#0d1b2a;' align='center'><img src='data:image/png;base64," +
            logoBase64() + "' width='180' height='49'/></td></tr></table>";
    html += "<h1>Rapport de progression - Participants</h1>";
    html += "<p>Généré le " + QDate::currentDate().toString("dd/MM/yyyy") + " - Plateforme HackTime</p>";

    html += "<h2>1. Chiffres clés</h2><table width='100%'><tr><th>Indicateur</th><th>Valeur</th></tr>";
    html += QString("<tr><td>Participants inscrits</td><td>%1</td></tr>").arg(total);
    html += QString("<tr><td>Participants avec équipe</td><td>%1 (%2)</td></tr>").arg(total - sansEquipe).arg(pct(total - sansEquipe));
    html += QString("<tr><td>Participants sans équipe</td><td>%1 (%2)</td></tr>").arg(sansEquipe).arg(pct(sansEquipe));
    html += QString("<tr><td>Nombre d'équipes</td><td>%1 (dont %2 incomplète(s), minimum %3 membres)</td></tr>")
                .arg(equipes.size()).arg(incompletes).arg(kMinEquipe);
    html += QString("<tr><td>Score d'engagement moyen</td><td>%1 / 100</td></tr></table>").arg(QString::number(somme / total, 'f', 1));

    html += "<h2>2. Avancement par statut</h2><table width='100%'><tr><th>Statut</th><th>Participants</th><th>Part</th></tr>";
    for (const QString &s : QStringList{"INSCRIT", "AFFECTE", "VALIDE", "INACTIF"})
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(Metier::libelleStatut(s)).arg(parStatut[s]).arg(pct(parStatut[s]));
    html += "</table>";

    html += "<h2>3. Niveau d'engagement</h2><table width='100%'><tr><th>Catégorie</th><th>Participants</th><th>Part</th></tr>";
    for (const QString &c : QStringList{"Actif", "Nouveau", "À surveiller", "À risque", "Inactif"})
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(c).arg(parCat[c]).arg(pct(parCat[c]));
    html += "</table>";

    html += "<h2>4. Équipes</h2><table width='100%'><tr><th>Équipe</th><th>Membres</th><th>Effectif</th><th>État</th><th>Niveau moyen</th></tr>";
    for (auto it = equipes.constBegin(); it != equipes.constEnd(); ++it) {
        QStringList noms;
        double sn = 0;
        for (const InfoParticipant &p : it.value()) { noms << p.nomComplet(); sn += p.niveau; }
        html += QString("<tr><td>Équipe %1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
                    .arg(it.key()).arg(noms.join(", ")).arg(it.value().size())
                    .arg(etatEquipe(it.value().size()))
                    .arg(QString::number(sn / it.value().size(), 'f', 1));
    }
    html += "</table>";

    html += "<h2>5. Participants à risque</h2>";
    if (risques.isEmpty()) {
        html += "<p>Aucun participant à risque.</p>";
    } else {
        html += "<table width='100%'><tr><th>Participant</th><th>Score</th><th>Motif</th></tr>";
        for (const Risque &r : risques)
            html += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(r.p.nomComplet()).arg(r.score).arg(Metier::motifRisque(r.p));
        html += "</table>";
    }
    html += "</body></html>";

    imprimerHtml(html, fichier, QPageLayout::Portrait, 96);
    QMessageBox::information(parent, "Succès", "Rapport de progression généré avec succès !");
    QDesktopServices::openUrl(QUrl::fromLocalFile(fichier));
}

// ============================================================================
//  EXPORT : choix du participant puis fiche individuelle (PDF / Excel)
// ============================================================================
enum class ChoixExport { Annule, Fiche, ListeComplete };

ChoixExport choisirParticipant(QWidget *parent, const QString &titre, InfoParticipant &choisi, bool avecListe = true)
{
    const QList<InfoParticipant> tous = Participant::lister("", 0);
    Fenetre fen = creerFenetre(parent, titre,
        "Choisissez un participant : sa fiche complète (contact, équipe, activité, engagement) sera exportée.", 560, 560);
    QDialog *dlg = fen.dlg.get();
    QVBoxLayout *corps = fen.corps;

    QLineEdit *recherche = new QLineEdit();
    recherche->setPlaceholderText("🔍  Rechercher par nom, prénom, e-mail ou téléphone");
    corps->addWidget(recherche);

    QListWidget *liste = new QListWidget();
    for (int i = 0; i < tous.size(); ++i) {
        const InfoParticipant &p = tous[i];
        QListWidgetItem *it = new QListWidgetItem(QString("%1   ·   %2   ·   %3")
            .arg(p.nomComplet(), p.email, p.aEquipe() ? QString("Équipe %1").arg(p.idEquipe) : QString("sans équipe")));
        it->setData(Qt::UserRole, i);
        it->setData(Qt::UserRole + 1, (p.nom + " " + p.prenom + " " + p.email + " " + p.telephone).toLower());
        liste->addItem(it);
    }
    corps->addWidget(liste, 1);

    QLabel *compteur = new QLabel();
    compteur->setStyleSheet("color: #64748b; font-size: 11px;");
    corps->addWidget(compteur);

    QHBoxLayout *boutons = new QHBoxLayout();
    QPushButton *btnListe = bouton("Toute la liste", false);
    QPushButton *btnAnnuler = bouton("Annuler", false);
    QPushButton *btnFiche = bouton("Exporter la fiche", true);
    btnFiche->setEnabled(false);
    boutons->addWidget(btnListe);
    btnListe->setVisible(avecListe);
    boutons->addStretch();
    boutons->addWidget(btnAnnuler);
    boutons->addWidget(btnFiche);
    corps->addLayout(boutons);

    auto filtrer = [&](const QString &t) {
        const QString q = t.trimmed().toLower();
        int visibles = 0;
        for (int i = 0; i < liste->count(); ++i) {
            const bool ok = q.isEmpty() || liste->item(i)->data(Qt::UserRole + 1).toString().contains(q);
            liste->item(i)->setHidden(!ok);
            if (ok) visibles++;
        }
        compteur->setText(QString("%1 participant(s)").arg(visibles));
    };
    filtrer(QString());
    QObject::connect(recherche, &QLineEdit::textChanged, dlg, filtrer);
    QObject::connect(liste, &QListWidget::currentItemChanged, dlg, [&]() { btnFiche->setEnabled(liste->currentItem() != nullptr); });

    ChoixExport resultat = ChoixExport::Annule;
    auto valider = [&]() {
        if (!liste->currentItem()) return;
        choisi = tous[liste->currentItem()->data(Qt::UserRole).toInt()];
        resultat = ChoixExport::Fiche;
        dlg->accept();
    };
    QObject::connect(btnFiche, &QPushButton::clicked, dlg, valider);
    QObject::connect(liste, &QListWidget::itemDoubleClicked, dlg, valider);
    QObject::connect(btnListe, &QPushButton::clicked, dlg, [&]() { resultat = ChoixExport::ListeComplete; dlg->accept(); });
    QObject::connect(btnAnnuler, &QPushButton::clicked, dlg, &QDialog::reject);

    dlg->exec();
    return resultat;
}

// Historique réel + évènement d'inscription déduit de la date d'inscription (participants créés avant le journal)
QList<EvenementHistorique> historiqueComplet(const InfoParticipant &p)
{
    QList<EvenementHistorique> h = Participant::historique(p.id);
    bool aInscription = false;
    for (const EvenementHistorique &e : h) if (e.code == "INSCRIPTION") aInscription = true;
    if (!aInscription && p.dateInscription.isValid()) {
        EvenementHistorique e;
        e.date = QDateTime(p.dateInscription, QTime(0, 0));
        e.code = "INSCRIPTION";
        h.prepend(e);
    }
    return h;
}

QString htmlFiche(const InfoParticipant &p)
{
    const Metier::Engagement e = Metier::calculerEngagement(p);
    const QList<InfoParticipant> equipe = membresEquipe(p.idEquipe);
    const QList<EvenementHistorique> hist = historiqueComplet(p);

    QString fond, texte;
    couleursStatut(p.statut, fond, texte);

    auto ligne = [](const QString &k, const QString &v) {
        return QString("<tr><td width='32%' bgcolor='#f1f5f9' style='color:#475569; font-weight:bold;'>%1</td><td>%2</td></tr>").arg(k, v);
    };
    auto titre = [](const QString &t) {
        return QString("<h2>%1</h2>").arg(t);
    };

    QString h =
        "<html><head><style>"
        "body { font-family: Helvetica, Arial, sans-serif; color: #0f172a; }"
        "h2 { color: #0d1b2a; font-size: 14px; margin: 16px 0 6px 0; border-bottom: 2px solid #f26b0f; padding-bottom: 3px; }"
        "table { border-collapse: collapse; width: 100%; }"
        "th { background-color: #0d1b2a; color: #ffffff; font-size: 11px; padding: 6px; text-align: left; border: 1px solid #0d1b2a; }"
        "td { font-size: 11px; padding: 6px; border: 1px solid #cbd5e1; }"
        "p { font-size: 10px; color: #64748b; }"
        "</style></head><body>";

    h += "<table width='100%'><tr><td bgcolor='#0d1b2a' style='border:1px solid #0d1b2a;' width='34%' align='center'><img src='data:image/png;base64," +
         logoBase64() + "' width='160' height='44'/></td>"
         "<td bgcolor='#0d1b2a' style='border:1px solid #0d1b2a; color:#ffffff;'>"
         "<span style='font-size:19px; font-weight:bold; color:#ffffff;'>" + p.nomComplet().toHtmlEscaped() + "</span><br/>"
         "<span style='font-size:11px; color:#94a3b8;'>Fiche participant - HackTime · éditée le " +
         QDate::currentDate().toString("dd/MM/yyyy") + "</span></td></tr></table>";

    h += titre("1. Coordonnées");
    h += "<table width='100%'>";
    h += ligne("Identifiant", p.id.toHtmlEscaped());
    h += ligne("Nom", p.nom.toHtmlEscaped());
    h += ligne("Prénom", p.prenom.toHtmlEscaped());
    h += ligne("E-mail", "<span style='color:#2563eb;'>" + p.email.toHtmlEscaped() + "</span>");
    h += ligne("Téléphone", p.telephone.toHtmlEscaped());
    h += "</table>";

    h += titre("2. Profil et statut");
    h += "<table width='100%'>";
    h += ligne("Compétence principale", nomCompetenceAffiche(p).toHtmlEscaped());
    h += ligne("Niveau", Metier::libelleNiveau(p.niveau));
    h += ligne("Statut", QString("<span style='color:%1; font-weight:bold;'>%2</span>").arg(texte, Metier::libelleStatut(p.statut)));
    h += ligne("Date d'inscription", dateTexte(p.dateInscription));
    h += ligne("Relances reçues", QString::number(p.nbRelances));
    h += "</table>";

    h += titre("3. Équipe");
    if (!p.aEquipe()) {
        h += "<table width='100%'>" + ligne("Équipe", "Ce participant n'a pas encore d'équipe.") + "</table>";
    } else {
        h += "<table width='100%'>";
        h += ligne("Équipe", QString("n° %1").arg(p.idEquipe));
        h += ligne("Effectif", QString("%1 membre(s) (minimum %2) - %3").arg(equipe.size()).arg(kMinEquipe).arg(etatEquipe(equipe.size())));
        h += "</table><br/><table width='100%'><tr><th>Membre</th><th>E-mail</th><th>Téléphone</th><th>Compétence</th><th>Niveau</th></tr>";
        for (const InfoParticipant &m : equipe) {
            const bool moi = (m.id == p.id);
            h += QString("<tr%1><td>%2%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
                     .arg(moi ? " bgcolor='#dbeafe'" : "")
                     .arg(m.nomComplet().toHtmlEscaped(), moi ? " <b>(ce participant)</b>" : "")
                     .arg(m.email.toHtmlEscaped(), m.telephone, nomCompetenceAffiche(m), Metier::libelleNiveau(m.niveau));
        }
        h += "</table>";
    }

    h += titre(QString("4. Engagement : %1 / 100 (%2)").arg(e.score).arg(e.categorie));
    h += QString("<table width='100%'><tr><td bgcolor='%1' width='%2%' style='border:none;'>&nbsp;</td><td bgcolor='#e2e8f0' style='border:none;'>&nbsp;</td></tr></table><br/>")
             .arg(e.couleurTexte).arg(std::max(1, e.score));
    h += "<table width='100%'><tr><th>Critère</th><th width='16%'>Points</th></tr>";
    for (const Metier::CritereEngagement &c : e.details)
        h += QString("<tr><td>%1</td><td>%2 / %3</td></tr>").arg(c.libelle.toHtmlEscaped()).arg(c.points).arg(c.maximum);
    h += "</table>";

    h += titre("5. Historique : ce que le participant a fait");
    if (hist.isEmpty()) {
        h += "<p>Aucune activité enregistrée pour le moment.</p>";
    } else {
        h += "<table width='100%'><tr><th width='24%'>Date</th><th>Activité</th></tr>";
        for (const EvenementHistorique &ev : hist)
            h += QString("<tr><td>%1</td><td>%2</td></tr>")
                     .arg(ev.date.toString("dd/MM/yyyy HH:mm"), Metier::libelleEvenement(ev).toHtmlEscaped());
        h += "</table>";
    }
    h += "<p>HackTime Platform © 2026 - Document généré automatiquement.</p></body></html>";
    return h;
}

QList<FeuilleXlsx> feuillesFiche(const InfoParticipant &p)
{
    const Metier::Engagement e = Metier::calculerEngagement(p);
    const QList<InfoParticipant> equipe = membresEquipe(p.idEquipe);
    const QList<EvenementHistorique> hist = historiqueComplet(p);

    FeuilleXlsx fiche;
    fiche.nom = "Fiche";
    fiche.filtre = false;
    fiche.largeurs = {28, 46};
    fiche.lignes.append(QVariantList() << "Information" << "Valeur");
    fiche.lignes.append(QVariantList() << "Identifiant" << p.id);
    fiche.lignes.append(QVariantList() << "Nom" << p.nom);
    fiche.lignes.append(QVariantList() << "Prénom" << p.prenom);
    fiche.lignes.append(QVariantList() << "E-mail" << p.email);
    fiche.lignes.append(QVariantList() << "Téléphone" << p.telephone);
    fiche.lignes.append(QVariantList() << "Compétence principale" << nomCompetenceAffiche(p));
    fiche.lignes.append(QVariantList() << "Niveau" << Metier::libelleNiveau(p.niveau));
    fiche.lignes.append(QVariantList() << "Statut" << Metier::libelleStatut(p.statut));
    fiche.lignes.append(QVariantList() << "Date d'inscription" << dateTexte(p.dateInscription));
    fiche.lignes.append(QVariantList() << "Équipe" << (p.aEquipe() ? QString("Équipe %1").arg(p.idEquipe) : QString("Aucune")));
    fiche.lignes.append(QVariantList() << "Effectif de l'équipe" << (p.aEquipe() ? QString("%1 membre(s) - %2").arg(equipe.size()).arg(etatEquipe(equipe.size())) : QString("-")));
    fiche.lignes.append(QVariantList() << "Relances reçues" << p.nbRelances);
    fiche.lignes.append(QVariantList() << "Score d'engagement" << QString("%1 / 100 (%2)").arg(e.score).arg(e.categorie));

    FeuilleXlsx feqp;
    feqp.nom = "Équipe";
    feqp.largeurs = {26, 30, 14, 20, 16, 14};
    feqp.lignes.append(QVariantList() << "Membre" << "E-mail" << "Téléphone" << "Compétence" << "Niveau" << "Participant exporté");
    for (const InfoParticipant &m : equipe)
        feqp.lignes.append(QVariantList() << m.nomComplet() << m.email << m.telephone << nomCompetenceAffiche(m)
                                          << Metier::libelleNiveau(m.niveau) << (m.id == p.id ? "oui" : ""));
    if (equipe.isEmpty())
        feqp.lignes.append(QVariantList() << "Pas encore d'équipe");

    FeuilleXlsx feng;
    feng.nom = "Engagement";
    feng.filtre = false;
    feng.largeurs = {56, 10, 10};
    feng.lignes.append(QVariantList() << "Critère" << "Points" << "Maximum");
    for (const Metier::CritereEngagement &c : e.details)
        feng.lignes.append(QVariantList() << c.libelle << c.points << c.maximum);
    feng.lignes.append(QVariantList() << "TOTAL" << e.score << 100);

    FeuilleXlsx fhis;
    fhis.nom = "Historique";
    fhis.largeurs = {20, 56};
    fhis.lignes.append(QVariantList() << "Date" << "Activité");
    for (const EvenementHistorique &ev : hist)
        fhis.lignes.append(QVariantList() << ev.date.toString("dd/MM/yyyy HH:mm") << Metier::libelleEvenement(ev));
    if (hist.isEmpty())
        fhis.lignes.append(QVariantList() << "-" << "Aucune activité enregistrée");

    return {fiche, feqp, feng, fhis};
}

// ============================================================================
//  CERTIFICAT DE PARTICIPATION (PDF paysage dessiné par le code)
// ============================================================================
void dessinerVague(QPainter &p, const QPainterPath &chemin, const QColor &c)
{
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawPath(chemin);
}

void dessinerPoints(QPainter &p, double x, double y, int lignes, int colonnes, const QColor &c)
{
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    for (int i = 0; i < lignes; ++i)
        for (int j = 0; j < colonnes; ++j)
            p.drawEllipse(QPointF(x + j * 17, y + i * 17), 3.6, 3.6);
}

bool genererCertificat(const InfoParticipant &part, const QString &fichier)
{
    const int L = 1500, H = 1060;                       // repère de dessin (ratio A4 paysage)
    QPdfWriter pdf(fichier);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageOrientation(QPageLayout::Landscape);
    pdf.setResolution(300);
    pdf.setPageMargins(QMarginsF(0, 0, 0, 0));

    QPainter p(&pdf);
    if (!p.isActive()) return false;
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setWindow(0, 0, L, H);
    p.setViewport(0, 0, pdf.width(), pdf.height());

    const QColor marine("#0d1b2a"), teal("#1fa592"), tealClair("#8dd9cb"), tealMoyen("#3ab5a3"), orange("#f26b0f");

    p.fillRect(0, 0, L, H, QColor("#fbfdfd"));

    // vagues décoratives
    QPainterPath v1; v1.moveTo(0, 0); v1.lineTo(560, 0);
    v1.cubicTo(430, 90, 260, 70, 140, 150); v1.cubicTo(70, 195, 20, 230, 0, 250); v1.closeSubpath();
    dessinerVague(p, v1, tealClair);
    QPainterPath v2; v2.moveTo(640, 0); v2.lineTo(L, 0); v2.lineTo(L, 170);
    v2.cubicTo(1250, 190, 1100, 40, 900, 60); v2.cubicTo(790, 70, 700, 40, 640, 0); v2.closeSubpath();
    dessinerVague(p, v2, tealMoyen);
    QPainterPath v3; v3.moveTo(0, H); v3.lineTo(0, 740);
    v3.cubicTo(40, 800, 60, 960, 160, 990); v3.cubicTo(230, 1010, 270, 1040, 300, H); v3.closeSubpath();
    dessinerVague(p, v3, tealMoyen);
    QPainterPath v4; v4.moveTo(L, H); v4.lineTo(L, 880);
    v4.cubicTo(1400, 900, 1380, 980, 1300, 1000); v4.cubicTo(1230, 1015, 1200, 1040, 1180, H); v4.closeSubpath();
    dessinerVague(p, v4, tealClair);

    dessinerPoints(p, 38, 38, 6, 5, QColor(255, 255, 255, 235));
    dessinerPoints(p, L - 110, H - 270, 6, 5, tealMoyen);

    // courbe fine + pastilles
    p.setPen(QPen(marine, 2.2));
    p.setBrush(Qt::NoBrush);
    QPainterPath c1; c1.moveTo(1120, 0); c1.cubicTo(1230, 180, 1330, 230, L - 60, 215);
    p.drawPath(c1);
    p.setPen(Qt::NoPen); p.setBrush(teal);
    p.drawEllipse(QPointF(L - 70, 215), 24, 24);
    p.drawEllipse(QPointF(L - 28, 190), 11, 11);

    // logo HackTime sur pastille bleu nuit
    p.setBrush(marine);
    p.drawRoundedRect(QRectF(L / 2 - 135, 70, 270, 74), 37, 37);
    p.drawPixmap(QRectF(L / 2 - 105, 76, 210, 57), createHackTimeLogoPixmap(), QRectF(0, 0, 220, 60));

    // titre
    p.setPen(marine);
    QFont f("Helvetica", 1); f.setPixelSize(64); f.setBold(true);
    p.setFont(f);
    p.drawText(QRectF(0, 175, L, 80), Qt::AlignCenter, "CERTIFICAT DE PARTICIPATION");
    p.setPen(QPen(orange, 4));
    p.drawLine(L / 2 - 70, 268, L / 2 + 70, 268);

    p.setPen(marine);
    f.setPixelSize(24); f.setBold(true); p.setFont(f);
    p.drawText(QRectF(0, 300, L, 34), Qt::AlignCenter, "REMIS À");

    // nom
    QFont fn("Georgia", 1); fn.setPixelSize(104);
    fn.setStyleHint(QFont::Serif);
    p.setFont(fn);
    p.setPen(teal);
    QString nom = part.nomComplet();
    {
        QStringList mots = nom.split(' ', Qt::SkipEmptyParts);
        for (QString &m : mots) m = m.left(1).toUpper() + m.mid(1).toLower();
        nom = mots.join(' ');
    }
    p.drawText(QRectF(100, 335, L - 200, 140), Qt::AlignCenter, nom);
    p.setPen(QPen(teal, 3));
    p.drawLine(230, 490, L - 230, 490);

    // texte
    const QList<InfoParticipant> equipe = membresEquipe(part.idEquipe);
    QString texte = QString("a participé avec succès au hackathon HackTime au sein de l'équipe n° %1 (%2 membres), "
                            "dans le domaine « %3 » (niveau %4). Ce certificat atteste son engagement et sa contribution "
                            "tout au long de l'événement.")
                        .arg(part.idEquipe).arg(equipe.size())
                        .arg(nomCompetenceAffiche(part), Metier::libelleNiveau(part.niveau).toLower());
    f.setBold(false); f.setPixelSize(30); p.setFont(f);
    p.setPen(marine);
    p.drawText(QRectF(210, 520, L - 420, 140), Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, texte);

    // identifiant + date
    f.setBold(true); f.setPixelSize(27); p.setFont(f);
    p.drawText(QRectF(0, 690, L, 36), Qt::AlignCenter,
               QString("IDENTIFIANT : HT-%1-%2").arg(QDate::currentDate().year()).arg(part.id.rightJustified(4, '0')));
    p.drawText(QRectF(0, 735, L, 36), Qt::AlignCenter,
               QString("DATE D'ÉMISSION : %1").arg(QDate::currentDate().toString("dd/MM/yyyy")));

    // sceau central
    const QPointF cs(L / 2, 880);
    p.setPen(QPen(teal, 5)); p.setBrush(QColor("#ffffff"));
    p.drawEllipse(cs, 64, 64);
    p.setPen(QPen(orange, 2)); p.setBrush(Qt::NoBrush);
    p.drawEllipse(cs, 53, 53);
    f.setPixelSize(23); f.setBold(true); p.setFont(f);
    p.setPen(marine);
    p.drawText(QRectF(cs.x() - 60, cs.y() - 30, 120, 34), Qt::AlignCenter, "HACK");
    p.setPen(orange);
    p.drawText(QRectF(cs.x() - 60, cs.y() - 4, 120, 34), Qt::AlignCenter, "TIME");
    p.setPen(teal);
    f.setPixelSize(13); p.setFont(f);
    p.drawText(QRectF(cs.x() - 60, cs.y() + 24, 120, 20), Qt::AlignCenter, "OFFICIEL");

    // signatures
    p.setPen(QPen(teal, 3));
    p.drawLine(250, 880, 590, 880);
    p.drawLine(L - 590, 880, L - 250, 880);
    p.setPen(marine);
    f.setPixelSize(27); f.setBold(true); p.setFont(f);
    p.drawText(QRectF(250, 892, 340, 34), Qt::AlignCenter, "COMITÉ D'ORGANISATION");
    p.drawText(QRectF(L - 590, 892, 340, 34), Qt::AlignCenter, "DIRECTION HACKTIME");
    p.setPen(QColor("#475569"));
    f.setBold(false); f.setPixelSize(24); p.setFont(f);
    p.drawText(QRectF(250, 928, 340, 30), Qt::AlignCenter, "Responsable du hackathon");
    p.drawText(QRectF(L - 590, 928, 340, 30), Qt::AlignCenter, "Direction de la plateforme");

    p.end();
    return true;
}


// Certificat basé sur le modèle graphique (certificat_modele.png) : le code remplit les espaces vides
// (nom, identifiant, date, texte de l'équipe) à partir des données du participant.
bool genererCertificatModele(const InfoParticipant &part, const QString &fichier)
{
    const QPixmap modele(":/certificat_modele.png");
    if (modele.isNull()) return genererCertificat(part, fichier);     // secours : certificat dessiné par le code

    const int L = modele.width(), H = modele.height();
    QPdfWriter pdf(fichier);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setPageOrientation(QPageLayout::Landscape);
    pdf.setResolution(300);
    pdf.setPageMargins(QMarginsF(0, 0, 0, 0));

    QPainter p(&pdf);
    if (!p.isActive()) return false;
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setWindow(0, 0, L, H);
    p.setViewport(0, 0, pdf.width(), pdf.height());
    p.drawPixmap(0, 0, modele);

    const QColor marine("#0d1b2a"), teal("#1fa592");

    // nom du participant (entre « REMIS À » et la ligne)
    QString nom = part.nomComplet();
    {
        QStringList mots = nom.split(' ', Qt::SkipEmptyParts);
        for (QString &m : mots) m = m.left(1).toUpper() + m.mid(1).toLower();
        nom = mots.join(' ');
    }
    QFont fn("Georgia", 1);
    fn.setStyleHint(QFont::Serif);
    int taille = 108;
    fn.setPixelSize(taille);
    while (QFontMetrics(fn).horizontalAdvance(nom) > L - 360 && taille > 40) fn.setPixelSize(--taille);
    p.setFont(fn);
    p.setPen(teal);
    p.drawText(QRectF(100, 330, L - 200, 130), Qt::AlignCenter, nom);

    // texte de participation (remplace le texte d'exemple : fond blanc du modèle)
    const QList<InfoParticipant> equipe = membresEquipe(part.idEquipe);
    p.fillRect(QRectF(150, 482, L - 300, 150), QColor("#ffffff"));
    QFont f("Helvetica", 1);
    f.setPixelSize(31);
    p.setFont(f);
    p.setPen(marine);
    const QString texte = QString("a participé avec succès au hackathon HackTime au sein de l'équipe n° %1 (%2 membres), "
                                  "dans le domaine « %3 » (niveau %4). Ce certificat atteste son engagement.")
                              .arg(part.idEquipe).arg(equipe.size())
                              .arg(nomCompetenceAffiche(part), Metier::libelleNiveau(part.niveau).toLower());
    p.drawText(QRectF(195, 492, L - 390, 140), Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, texte);

    // identifiant et date, à droite des libellés « IDENTIFIANT : » et « DATE D'ÉMISSION : »
    f.setBold(true);
    f.setPixelSize(32);
    p.setFont(f);
    p.drawText(QRectF(778, 640, 600, 46), Qt::AlignLeft | Qt::AlignVCenter,
               QString("HT-%1-%2").arg(QDate::currentDate().year()).arg(part.id.rightJustified(4, '0')));
    p.drawText(QRectF(825, 687, 600, 46), Qt::AlignLeft | Qt::AlignVCenter,
               QDate::currentDate().toString("dd/MM/yyyy"));

    p.end();
    return true;
}

} // namespace
static QPixmap createHackTimeLogoPixmap() {
    QPixmap pixmap(220, 60);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF iconRect(5, 5, 50, 50);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(13, 27, 42));
    painter.drawRoundedRect(iconRect, 10, 10);

    QRectF clockRect(10, 10, 40, 40);

    QPen bluePen(QColor(0, 122, 255), 3);
    painter.setPen(bluePen);
    painter.drawArc(clockRect, 45 * 16, 270 * 16);

    QPen orangePen(QColor(242, 107, 15), 3);
    painter.setPen(orangePen);
    painter.drawArc(clockRect, 15 * 16, 60 * 16);

    QFont codeFont("Arial", 8, QFont::Bold);
    painter.setFont(codeFont);
    painter.setPen(QColor(242, 107, 15));
    painter.drawText(QRectF(20, 6, 20, 12), Qt::AlignCenter, "</>");

    painter.setPen(QPen(QColor(0, 122, 255), 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(30, 30, 22, 22);
    painter.drawLine(30, 30, 42, 24);

    painter.setPen(QPen(QColor(242, 107, 15), 2));
    painter.drawLine(14, 30, 17, 30);
    painter.drawLine(43, 30, 46, 30);
    painter.drawLine(30, 43, 30, 46);

    QFont titleFont("Arial", 18, QFont::Bold);
    painter.setFont(titleFont);

    painter.setPen(QColor(255, 255, 255));
    painter.drawText(65, 32, "Hack");

    painter.setPen(QColor(242, 107, 15));
    painter.drawText(122, 32, "Time");

    QFont subFont("Arial", 5, QFont::Bold);
    painter.setFont(subFont);
    painter.setPen(QColor(148, 163, 184));
    painter.drawText(66, 45, "GESTION INTELLIGENTE DE HACKATHONS");

    return pixmap;
}

QWidget* ModuleParticipants::createActionButtonsWidget(int row)
{
    QWidget *actionsWidget = new QWidget();
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(6);
    actionsLayout->setAlignment(Qt::AlignCenter);

    QPushButton *btnVoir = new QPushButton("👁");
    QPushButton *btnModif = new QPushButton("✏");
    QPushButton *btnSuppr = new QPushButton("🗑️");

    QString btnStyle = "QPushButton { border: none; background: transparent; font-size: 14px; }"
                       "QPushButton:hover { font-size: 16px; }";
    btnVoir->setStyleSheet(btnStyle);
    btnModif->setStyleSheet(btnStyle);
    btnSuppr->setStyleSheet(btnStyle);

    connect(btnVoir, &QPushButton::clicked, this, [this, row]() { onVoirParticipantClicked(row); });
    connect(btnModif, &QPushButton::clicked, this, [this, row]() { onModifierParticipantClicked(row); });
    connect(btnSuppr, &QPushButton::clicked, this, [this, row]() { onSupprimerParticipantClicked(row); });

    actionsLayout->addWidget(btnVoir);
    actionsLayout->addWidget(btnModif);
    actionsLayout->addWidget(btnSuppr);

    return actionsWidget;
}

void ModuleParticipants::updateCounters()
{
    Participant p;
    int totalSQL = p.calculerTotalParticipants();

    if (lblActifs && table) {
        lblActifs->setText(QString::number(totalSQL));
    }
    if (lblNouveaux) {
        lblNouveaux->setText(QString::number(nouveauxInscritsCount));
    }
}

int ModuleParticipants::ligneSelectionnee() const
{
    if (!table) return -1;
    const QList<QTableWidgetItem*> items = table->selectedItems();
    if (items.isEmpty()) return -1;
    return items.first()->row();
}

const InfoParticipant *ModuleParticipants::participantDeLigne(int row) const
{
    if (!table || row < 0 || row >= table->rowCount() || !table->item(row, 0)) return nullptr;
    const QString id = table->item(row, 0)->data(Qt::UserRole).toString();
    for (const InfoParticipant &p : m_liste)
        if (p.id == id) return &p;
    return nullptr;
}

// Nom de la compétition d'une équipe (table EQUIPE de la gestion Compétitions/Équipes).
// Résultat mis en cache ; si la table ou la colonne n'existe pas encore, on renvoie "".
static QHash<int, QString> g_cacheCompetition;
static bool g_competitionIndisponible = false;

static QString nomCompetitionEquipe(int idEquipe)
{
    QHash<int, QString> &cache = g_cacheCompetition;
    if (g_competitionIndisponible) return QString();
    if (cache.contains(idEquipe)) return cache.value(idEquipe);
    QSqlQuery q;
    q.prepare("SELECT C.NOM FROM EQUIPE E JOIN COMPETITION C ON C.ID_COMPETITION = E.ID_COMPETITION "
              "WHERE E.ID_EQUIPE = :e");
    q.bindValue(":e", idEquipe);
    QString nom;
    if (!q.exec()) { g_competitionIndisponible = true; return QString(); }
    if (q.next()) nom = q.value(0).toString();
    cache.insert(idEquipe, nom);
    return nom;
}

void ModuleParticipants::chargerDonneesSQL()
{
    if (!table) return;
    g_cacheCompetition.clear();   // refait à chaque rechargement (les équipes peuvent changer)
    g_competitionIndisponible = false;

    // Recherche (nom, prénom, e-mail, téléphone, équipe) + tri choisi : tout est fait en SQL
    m_liste = Participant::lister(ui->searchBoxPart->text(), ui->comboTriPart->currentIndex());
    table->setRowCount(0);
    const QSet<QString> presents = Participant::idsPointesAujourdhui();

    int row = 0;
    for (const InfoParticipant &p : m_liste) {
        table->insertRow(row);

        // L'ID n'est pas une colonne visible : il est stocké dans l'item "Participant" (Qt::UserRole)
        const bool present = presents.contains(p.id);   // pointé aujourd'hui à la barrière Arduino
        QTableWidgetItem *itemNom = new QTableWidgetItem((present ? QString("✔ ") : QString()) + p.nom + " " + p.prenom);
        if (present) {
            itemNom->setForeground(QColor("#15803d"));
            itemNom->setToolTip("Présent : pointé aujourd'hui à la borne d'accès");
        }
        itemNom->setData(Qt::UserRole, p.id);
        table->setItem(row, 0, itemNom);
        table->setItem(row, 1, new QTableWidgetItem(p.email));
        table->setItem(row, 2, new QTableWidgetItem(p.telephone));

        QTableWidgetItem *itemEquipe = new QTableWidgetItem(p.aEquipe() ? QString::number(p.idEquipe) : QString("-"));
        itemEquipe->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 3, itemEquipe);

        // Compétition de l'équipe (lien avec le module Compétitions : EQUIPE -> COMPETITION)
        const QString comp = p.aEquipe() ? nomCompetitionEquipe(p.idEquipe) : QString();
        QTableWidgetItem *itemComp = new QTableWidgetItem(comp.isEmpty() ? QString("-") : comp);
        itemComp->setToolTip(comp.isEmpty() ? QString("Pas de compétition (équipe non liée)") : comp);
        table->setItem(row, 4, itemComp);

        // Statut (workflow) : Inscrit / Affecté / Validé / Inactif
        QString fond, texte;
        couleursStatut(p.statut, fond, texte);
        table->setCellWidget(row, 5, creerPastille(Metier::libelleStatut(p.statut), fond, texte, 84));

        // Score d'engagement (métier 2)
        const Metier::Engagement e = Metier::calculerEngagement(p);
        QString court = e.categorie;
        if (court == "À surveiller") court = "Surveiller";
        else if (court == "À risque") court = "Risque";
        table->setCellWidget(row, 6, creerPastille(QString("%1 · %2").arg(e.score).arg(court),
                                                   e.couleurFond, e.couleurTexte, 96,
                                                   QString("Score d'engagement : %1 / 100 (%2)").arg(e.score).arg(e.categorie)));

        table->setCellWidget(row, 7, createActionButtonsWidget(row));
        row++;
    }
    updateCounters();
    showPage(currentPage);
    majCarteBorne();
}

void ModuleParticipants::setupCustomUi()
{
    // ---- Liaison avec les widgets définis dans mainwindow.ui ----
    table          = ui->tablePart;
    lblActifs      = ui->lblActifsPart;
    lblNouveaux    = ui->lblNouveauxPart;
    paginationLayout = ui->paginationLayoutPart;

    // Curseur "main" sur tous les boutons du .ui
    const auto buttons = m_page->findChildren<QPushButton*>();
    for (QPushButton *b : buttons)
        b->setCursor(Qt::PointingHandCursor);

    // ---- Configuration du tableau ----
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"Participant", "E-mail", "Téléphone", "Équipe", "Compétition", "Statut", "Engagement", "Actions"});

    QHeaderView *h = table->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::Interactive);
    h->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    h->setSectionResizeMode(1, QHeaderView::Stretch);
    h->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    h->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    h->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    h->setMinimumSectionSize(40);
    h->setSectionResizeMode(5, QHeaderView::Fixed);
    table->setColumnWidth(5, 96);
    h->setSectionResizeMode(6, QHeaderView::Fixed);
    table->setColumnWidth(6, 108);
    h->setSectionResizeMode(7, QHeaderView::Fixed);
    table->setColumnWidth(7, 96);
    h->setMinimumSectionSize(40);
    table->setMinimumWidth(0);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(42);
    table->setShowGrid(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setFrameShape(QFrame::NoFrame);

    // ---- Boutons principaux ----
    connect(ui->addBtnPart, &QPushButton::clicked, this, &ModuleParticipants::onAjouterParticipantClicked);
    connect(ui->btnPdfPart, &QPushButton::clicked, this, &ModuleParticipants::exporterPdf);
    connect(ui->btnExcelPart, &QPushButton::clicked, this, &ModuleParticipants::exporterExcel);
    connect(ui->btnStatsDialogPart, &QPushButton::clicked, this, [this]() { statistiques(m_page); });

    // ---- Actions rapides (chacune exécute un vrai traitement métier) ----
    // 1) Affecter à une équipe : composition intelligente (métier 1) ou affectation manuelle
    connect(ui->btnAction1Part, &QPushButton::clicked, this, [this]() {
        bool modifie = false;
        const InfoParticipant *p = participantDeLigne(ligneSelectionnee());
        if (p) {
            const QStringList choix = {"Composition intelligente des équipes (automatique)",
                                       QString("Affecter manuellement : %1").arg(p->nomComplet())};
            bool ok = false;
            const QString c = QInputDialog::getItem(m_page, "Affecter à une équipe", "Que voulez-vous faire ?",
                                                    choix, 0, false, &ok);
            if (!ok) return;
            const InfoParticipant copie = *p;
            modifie = (c == choix[1]) ? affecterManuellement(m_page, copie)
                                      : compositionEquipes(m_page);
        } else {
            modifie = compositionEquipes(m_page);
        }
        if (modifie) chargerDonneesSQL();
    });

    // 2) Rappeler aux retardataires : score d'engagement, relances, escalade (métier 2)
    connect(ui->btnAction3Part, &QPushButton::clicked, this, [this]() {
        if (relances(m_page)) chargerDonneesSQL();
    });

    // 3) Valider statut projet : workflow Inscrit → Affecté → Validé
    connect(ui->btnAction4Part, &QPushButton::clicked, this, [this]() {
        const InfoParticipant *p = participantDeLigne(ligneSelectionnee());
        if (!p) {
            QMessageBox::information(m_page, "Valider statut", "Cliquez d'abord sur la ligne d'un participant dans le tableau.");
            return;
        }
        const InfoParticipant copie = *p;
        if (changerStatut(m_page, copie)) chargerDonneesSQL();
    });

    // 4) Rapport de progression (PDF)
    connect(ui->btnAction5Part, &QPushButton::clicked, this, [this]() { rapportProgression(m_page); });

    // 5) Certificat de participation (PDF) : réservé aux participants « Validé »
    connect(ui->btnAction6Part, &QPushButton::clicked, this, [this]() {
        InfoParticipant choisi;
        if (choisirParticipant(m_page, "Certificat de participation", choisi, false) != ChoixExport::Fiche) return;
        if (choisi.statut != "VALIDE") {
            QMessageBox::warning(m_page, "Certificat de participation",
                QString("%1 n'a pas le statut « Validé » (statut actuel : %2).\n\n"
                        "Validez d'abord le participant avec « Valider statut projet » (il faut une équipe d'au moins %3 membres).")
                    .arg(choisi.nomComplet(), Metier::libelleStatut(choisi.statut)).arg(kMinEquipe));
            return;
        }
        const QString nomFichier = (choisi.nom + "_" + choisi.prenom).simplified().replace(' ', '_');
        const QString fichier = QFileDialog::getSaveFileName(m_page, "Enregistrer le certificat",
                                    desktop() + "/certificat_" + nomFichier + ".pdf", "Documents PDF (*.pdf)");
        if (fichier.isEmpty()) return;
        if (genererCertificatModele(choisi, fichier)) {
            Participant::journaliser(choisi.id, "CERTIFICAT");
            QMessageBox::information(m_page, "Succès", QString("Certificat de %1 généré avec succès !").arg(choisi.nomComplet()));
            QDesktopServices::openUrl(QUrl::fromLocalFile(fichier));
        } else {
            QMessageBox::critical(m_page, "Erreur", "Impossible de créer le fichier PDF.");
        }
    });

    // ---- Recherche (nom, prénom, e-mail, téléphone, équipe) : filtrage SQL ----
    connect(ui->searchBoxPart, &QLineEdit::textChanged, this, [this](const QString &) {
        currentPage = 1;
        chargerDonneesSQL();
    });

    // ---- Tri : l'utilisateur choisit le critère dans la liste ----
    connect(ui->comboTriPart, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        currentPage = 1;
        chargerDonneesSQL();
    });

    chargerDonneesSQL();
}

// ======================= Export PDF =======================
// 1) fenêtre : choisir un participant existant (ou toute la liste)
// 2) PDF : fiche complète du participant (contact, équipe, profil, engagement, historique)
void ModuleParticipants::exporterPdf()
{
    InfoParticipant choisi;
    const ChoixExport choix = choisirParticipant(m_page, "Exporter en PDF", choisi);
    if (choix == ChoixExport::Annule) return;

    if (choix == ChoixExport::Fiche) {
        QString nomFichier = (choisi.nom + "_" + choisi.prenom).simplified().replace(' ', '_');
        const QString fichier = QFileDialog::getSaveFileName(m_page, "Exporter la fiche en PDF",
                                    desktop() + "/fiche_" + nomFichier + ".pdf", "Documents PDF (*.pdf)");
        if (fichier.isEmpty()) return;
        imprimerHtml(htmlFiche(choisi), fichier, QPageLayout::Portrait, 112);
        QMessageBox::information(m_page, "Succès", QString("Fiche PDF de %1 exportée avec succès !").arg(choisi.nomComplet()));
        return;
    }

    // ---- Liste complète (participants affichés) ----
    const QString fichier = QFileDialog::getSaveFileName(m_page, "Exporter la liste en PDF", desktop() + "/participants.pdf", "Documents PDF (*.pdf)");
    if (fichier.isEmpty()) return;

    int avecEquipe = 0, valides = 0;
    double somme = 0;
    for (const InfoParticipant &p : m_liste) {
        if (p.aEquipe()) avecEquipe++;
        if (p.statut == "VALIDE") valides++;
        somme += Metier::calculerEngagement(p).score;
    }
    const int total = m_liste.size();

    QString html = QString("<html><head><style>") +
                   "body { font-family: Helvetica, Arial, sans-serif; color: #0f172a; }"
                   "h1 { color: #0d1b2a; font-size: 22px; margin: 8px 0 2px 0; }"
                   "p { color: #475569; font-size: 11px; }"
                   "table { width: 100%; border-collapse: collapse; }"
                   "th { background-color: #0d1b2a; color: #ffffff; font-size: 12px; padding: 9px; text-align: left; border: 1px solid #0d1b2a; }"
                   "td { font-size: 11px; padding: 8px; border: 1px solid #cbd5e1; }"
                   "</style></head><body>";
    html += "<table width='100%'><tr><td bgcolor='#0d1b2a' align='center' style='border:1px solid #0d1b2a;'><img src='data:image/png;base64," +
            logoBase64() + "' width='180' height='49'/></td></tr></table>";
    html += "<h1>Liste officielle des participants</h1>";
    html += QString("<p>%1 participant(s) · %2 avec équipe · %3 validé(s) · engagement moyen %4 / 100</p>")
                .arg(total).arg(avecEquipe).arg(valides).arg(total > 0 ? QString::number(somme / total, 'f', 0) : QString("0"));
    html += "<table width='100%'><tr><th>Nom</th><th>Prénom</th><th>E-mail</th><th>Téléphone</th><th>Équipe</th><th>Statut</th><th>Engagement</th></tr>";
    for (const InfoParticipant &p : m_liste) {
        const Metier::Engagement e = Metier::calculerEngagement(p);
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7 / 100 (%8)</td></tr>")
                    .arg(p.nom.toHtmlEscaped(), p.prenom.toHtmlEscaped(), p.email.toHtmlEscaped(), p.telephone,
                         p.aEquipe() ? QString::number(p.idEquipe) : QString("-"),
                         Metier::libelleStatut(p.statut))
                    .arg(e.score).arg(e.categorie);
    }
    html += "</table><p>HackTime Platform © 2026 - Tous droits réservés</p></body></html>";

    imprimerHtml(html, fichier, QPageLayout::Landscape, 96);
    QMessageBox::information(m_page, "Succès", "Fichier PDF exporté avec succès !");
}

// ======================= Export Excel (.xlsx) =======================
void ModuleParticipants::exporterExcel()
{
    InfoParticipant choisi;
    const ChoixExport choix = choisirParticipant(m_page, "Exporter en Excel", choisi);
    if (choix == ChoixExport::Annule) return;

    QList<FeuilleXlsx> feuilles;
    QString fichier;

    if (choix == ChoixExport::Fiche) {
        QString nomFichier = (choisi.nom + "_" + choisi.prenom).simplified().replace(' ', '_');
        fichier = QFileDialog::getSaveFileName(m_page, "Exporter la fiche en Excel",
                      desktop() + "/fiche_" + nomFichier + ".xlsx", "Classeur Excel (*.xlsx)");
        if (fichier.isEmpty()) return;
        feuilles = feuillesFiche(choisi);
    } else {
        fichier = QFileDialog::getSaveFileName(m_page, "Exporter la liste en Excel", desktop() + "/participants.xlsx", "Classeur Excel (*.xlsx)");
        if (fichier.isEmpty()) return;

        FeuilleXlsx feuille;
        feuille.nom = "Participants";
        feuille.largeurs = {18, 18, 30, 14, 20, 14, 10, 12, 10, 14, 16};
        feuille.lignes.append(QVariantList() << "Nom" << "Prénom" << "E-mail" << "Téléphone" << "Compétence" << "Niveau"
                                             << "Équipe" << "Statut" << "Score" << "Engagement" << "Date inscription");
        for (const InfoParticipant &p : m_liste) {
            const Metier::Engagement e = Metier::calculerEngagement(p);
            feuille.lignes.append(QVariantList() << p.nom << p.prenom << p.email << p.telephone
                                                 << nomCompetenceAffiche(p) << Metier::libelleNiveau(p.niveau)
                                                 << (p.aEquipe() ? QVariant(p.idEquipe) : QVariant(QString("-")))
                                                 << Metier::libelleStatut(p.statut)
                                                 << e.score << e.categorie << dateTexte(p.dateInscription));
        }
        feuilles = {feuille};
    }

    QString erreur;
    if (ecrireXlsx(fichier, feuilles, &erreur))
        QMessageBox::information(m_page, "Succès", "Fichier Excel (.xlsx) exporté avec succès !");
    else
        QMessageBox::critical(m_page, "Erreur", "Échec de l'export Excel : " + erreur);
}

void ModuleParticipants::showPage(int page)
{
    if (!table) return;

    int totalRows = table->rowCount();
    int totalPages = (totalRows + itemsPerPage - 1) / itemsPerPage;
    if (totalPages == 0) totalPages = 1;

    if (page < 1) page = 1;
    if (page > totalPages) page = totalPages;

    currentPage = page;

    int startRow = (currentPage - 1) * itemsPerPage;
    int endRow = startRow + itemsPerPage;

    for (int i = 0; i < totalRows; ++i) {
        if (i >= startRow && i < endRow) {
            table->setRowHidden(i, false);
        } else {
            table->setRowHidden(i, true);
        }
    }

    updatePagination();
}

void ModuleParticipants::updatePagination()
{
    if (!paginationLayout) return;

    QLayoutItem *child;
    while ((child = paginationLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    paginationLayout->addStretch();

    int totalRows = table ? table->rowCount() : 0;
    int totalPages = (totalRows + itemsPerPage - 1) / itemsPerPage;
    if (totalPages == 0) totalPages = 1;

    QPushButton *btnPrev = new QPushButton("<");
    btnPrev->setCursor(Qt::PointingHandCursor);
    btnPrev->setObjectName("pageBtn");
    btnPrev->setEnabled(currentPage > 1);
    connect(btnPrev, &QPushButton::clicked, this, &ModuleParticipants::onPrevPageClicked);
    paginationLayout->addWidget(btnPrev);

    for (int i = 1; i <= totalPages; ++i) {
        QPushButton *btn = new QPushButton(QString::number(i));
        btn->setCursor(Qt::PointingHandCursor);

        if (i == currentPage) {
            btn->setObjectName("pageActive");
        } else {
            btn->setObjectName("pageBtn");
        }

        connect(btn, &QPushButton::clicked, this, &ModuleParticipants::onPageClicked);
        paginationLayout->addWidget(btn);
    }

    QPushButton *btnNext = new QPushButton(">");
    btnNext->setCursor(Qt::PointingHandCursor);
    btnNext->setObjectName("pageBtn");
    btnNext->setEnabled(currentPage < totalPages);
    connect(btnNext, &QPushButton::clicked, this, &ModuleParticipants::onNextPageClicked);
    paginationLayout->addWidget(btnNext);

    applyStyleSheet();
}

void ModuleParticipants::onPageClicked()
{
    QPushButton *clickedBtn = qobject_cast<QPushButton*>(sender());
    if (!clickedBtn) return;

    bool ok;
    int page = clickedBtn->text().toInt(&ok);
    if (ok) {
        showPage(page);
    }
}

void ModuleParticipants::onPrevPageClicked()
{
    if (currentPage > 1) {
        showPage(currentPage - 1);
    }
}

void ModuleParticipants::onNextPageClicked()
{
    int totalRows = table ? table->rowCount() : 0;
    int totalPages = (totalRows + itemsPerPage - 1) / itemsPerPage;
    if (currentPage < totalPages) {
        showPage(currentPage + 1);
    }
}

void ModuleParticipants::onAjouterParticipantClicked()
{
    InfoParticipant vide;
    const bool ajoute = afficherFormulaire(m_page, false, vide, [this](const InfoParticipant &p, QString &erreur) {
        Participant part;
        const bool ok = part.ajouter(p.id, p.nom, p.prenom, p.email, p.telephone,
                                     p.aEquipe() ? QString::number(p.idEquipe) : QString(),
                                     p.competence, p.niveau);
        if (!ok) erreur = part.derniereErreur();
        return ok;
    });
    if (ajoute) {
        nouveauxInscritsCount++;
        chargerDonneesSQL();
    }
}

void ModuleParticipants::onModifierParticipantClicked(int row)
{
    const InfoParticipant *p = participantDeLigne(row);
    if (!p) return;
    const InfoParticipant courant = *p;

    const bool modifie = afficherFormulaire(m_page, true, courant, [](const InfoParticipant &n, QString &erreur) {
        Participant part;
        const bool ok = part.modifier(n.id, n.nom, n.prenom, n.email, n.telephone,
                                      n.aEquipe() ? QString::number(n.idEquipe) : QString(),
                                      n.competence, n.niveau);
        if (!ok) erreur = part.derniereErreur();
        return ok;
    });
    if (modifie) chargerDonneesSQL();
}

void ModuleParticipants::onVoirParticipantClicked(int row)
{
    const InfoParticipant *p = participantDeLigne(row);
    if (!p) return;
    const Metier::Engagement e = Metier::calculerEngagement(*p);

    QString details = QString("<b>Nom:</b> %1<br>"
                              "<b>Prénom:</b> %2<br>"
                              "<b>Email:</b> %3<br>"
                              "<b>Téléphone:</b> %4<br>"
                              "<b>Équipe:</b> %5<br>"
                              "<b>Compétence:</b> %6 (%7)<br>"
                              "<b>Statut:</b> %8<br>"
                              "<b>Inscrit le:</b> %9<br>"
                              "<b>Engagement:</b> %10 / 100 - %11")
                          .arg(p->nom, p->prenom, p->email, p->telephone,
                               p->aEquipe() ? QString::number(p->idEquipe) : QString("-"),
                               p->competence.isEmpty() ? QString("-") : Metier::libelleCompetence(p->competence),
                               Metier::libelleNiveau(p->niveau), Metier::libelleStatut(p->statut))
                          .arg(p->dateInscription.isValid() ? p->dateInscription.toString("dd/MM/yyyy") : QString("-"))
                          .arg(e.score).arg(e.categorie);

    QMessageBox::information(m_page, "Détails du Participant", details);
}

void ModuleParticipants::onSupprimerParticipantClicked(int row)
{
    if (!table || row < 0 || row >= table->rowCount()) return;

    QString id = table->item(row, 0) ? table->item(row, 0)->data(Qt::UserRole).toString() : "";
    QString nom = table->item(row, 0) ? table->item(row, 0)->text() : "";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(m_page, "Confirmation",
                                  QString("Voulez-vous vraiment supprimer %1 ?").arg(nom),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        Participant p;
        if (p.supprimer(id)) {
            chargerDonneesSQL();
            showPage(currentPage);
            QMessageBox::information(m_page, "Supprimé", "Participant supprimé avec succès de la base SQL !");
        } else {
            QMessageBox::critical(m_page, "Erreur", "Echec de suppression !");
        }
    }
}


// ======================================================================
//  Construction du module (appelé par HackTime::initParticipant)
// ======================================================================
ModuleParticipants::ModuleParticipants(Ui::HackTime *interface, QWidget *fenetre)
    : QObject(fenetre)
    , ui(interface)
    , m_page(interface->pageParticipants)
{
    Q_UNUSED(fenetre);
    Participant::assurerSchema();   // ajoute les colonnes / la table d'historique si besoin
    applyStyleSheet();
    setupCustomUi();
    creerCarteBorne();
    m_page->installEventFilter(this);   // rafraîchit la liste à chaque ouverture de la page
}

bool ModuleParticipants::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_page && event->type() == QEvent::Show) {
        if (borne) borne->connecter();      // scénario Arduino : (re)connexion de la borne à l'ouverture
        chargerDonneesSQL();
    }
    return QObject::eventFilter(watched, event);
}

void ModuleParticipants::applyStyleSheet()
{
    // Style des boutons de pagination (créés dynamiquement) et des boîtes de message du module.
    // La feuille de style de la page (hacktime.ui) est conservée.
    if (m_styleBase.isNull()) m_styleBase = m_page->styleSheet();
    m_page->setStyleSheet(m_styleBase +
        "\nQPushButton#pageBtn { background-color: #f1f5f9; border: 1px solid #e2e8f0; border-radius: 6px; padding: 5px 10px; color: #475569; }"
        "\nQPushButton#pageBtn:disabled { background-color: #f8fafc; color: #cbd5e1; border-color: #f1f5f9; }"
        "\nQPushButton#pageActive { background-color: #2563eb; color: white; border-radius: 6px; padding: 5px 10px; font-weight: bold; border: none; }"
        "\nQMessageBox, QInputDialog { background-color: #ffffff; }"
        "\nQMessageBox QLabel, QInputDialog QLabel { color: #0f172a; font-size: 13px; }"
        "\nQMessageBox QPushButton, QInputDialog QPushButton { background-color: #2563eb; color: #ffffff; border: none; border-radius: 6px; padding: 7px 18px; min-width: 72px; font-weight: bold; }"
        "\nQMessageBox QPushButton:hover, QInputDialog QPushButton:hover { background-color: #1d4ed8; }"
        "\nQInputDialog QLineEdit, QInputDialog QSpinBox, QInputDialog QComboBox { border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px; background: #ffffff; color: #0f172a; }");
}


// =====================================================================
// === SCÉNARIO ARDUINO : BARRIÈRE D'ACCÈS (pointage des participants) ==
// =====================================================================
//  Potentiomètre -> choisit la ligne du tableau ; bouton ENTRER -> pointerSelection()
//  Participant::pointer() décide dans Oracle ; la borne reçoit O / R / D.

void ModuleParticipants::creerCarteBorne()
{
    borne = new BornePointage(this);
    borne->connecter();
    connect(borne, &BornePointage::positionChangee, this, &ModuleParticipants::selectionnerFraction);
    connect(borne, &BornePointage::entrerDemande, this, [this]() { pointerSelection(true); });

    QFrame *carte = new QFrame(m_page);
    carte->setObjectName("carteBornePart");
    carte->setStyleSheet("QFrame#carteBornePart { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 8px; }"
                         "QFrame#carteBornePart QLabel { background: transparent; border: none; }");
    QVBoxLayout *v = new QVBoxLayout(carte);
    v->setContentsMargins(10, 8, 10, 8);
    v->setSpacing(4);

    QLabel *titre = new QLabel("🚪  Barrière d'accès (Arduino)", carte);
    titre->setStyleSheet("font-size: 12px; font-weight: bold; color: #0f172a;");
    lblBorneEtat = new QLabel(carte);
    lblBorneEtat->setStyleSheet("font-size: 11px;");
    lblBornePresents = new QLabel(carte);
    lblBornePresents->setStyleSheet("font-size: 12px; font-weight: bold; color: #15803d;");
    lblBorneMessage = new QLabel("Sélectionnez un participant puis pointez son entrée.", carte);
    lblBorneMessage->setStyleSheet("font-size: 11px; color: #64748b;");
    lblBorneMessage->setWordWrap(true);
    lblBorneMessage->setMinimumHeight(52);                       // 3 lignes de texte
    lblBorneMessage->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QPushButton *bouton = new QPushButton("✅  Pointer l'entrée", carte);
    bouton->setCursor(Qt::PointingHandCursor);
    bouton->setToolTip("Même traitement que le bouton ENTRER de la borne (utile sans carte Arduino)");
    bouton->setStyleSheet("QPushButton { background-color: #ffffff; border: 1px solid #e2e8f0; padding: 8px 10px; border-radius: 8px;"
                          " text-align: left; color: #334155; font-size: 12px; font-weight: 500; }"
                          "QPushButton:hover { background-color: #f1f5f9; border-color: #cbd5e1; }");
    connect(bouton, &QPushButton::clicked, this, [this]() { pointerSelection(false); });

    v->addWidget(titre);
    v->addWidget(lblBorneEtat);
    v->addWidget(lblBornePresents);
    v->addWidget(lblBorneMessage);
    v->addWidget(bouton);

    // on place la carte dans « Actions rapides », avant le ressort final
    if (QVBoxLayout *vl = qobject_cast<QVBoxLayout *>(ui->actionsCardPart->layout()))
        vl->insertWidget(qMax(0, vl->count() - 1), carte);
    majCarteBorne();
}

void ModuleParticipants::majCarteBorne()
{
    if (!borne || !lblBorneEtat) return;
    const bool ok = borne->estConnecte();
    lblBorneEtat->setText((ok ? "● " : "○ ") + borne->etat());
    lblBorneEtat->setStyleSheet(QString("font-size: 11px; color: %1;").arg(ok ? "#15803d" : "#94a3b8"));
    lblBornePresents->setText(QString("Présents aujourd'hui : %1").arg(Participant::nombrePresentsAujourdhui()));
}

// potentiomètre : fraction 0..1 de la course -> ligne du tableau (toute la liste, toutes pages)
void ModuleParticipants::selectionnerFraction(double fraction)
{
    if (!table || m_liste.isEmpty()) return;
    const int n = m_liste.size();
    const int index = qBound(0, int(fraction * n), n - 1);
    showPage(index / itemsPerPage + 1);
    table->selectRow(index);
    table->scrollToItem(table->item(index, 0));
}

void ModuleParticipants::selectionnerParId(const QString &id)
{
    for (int i = 0; i < m_liste.size(); ++i) {
        if (m_liste.at(i).id == id) {
            showPage(i / itemsPerPage + 1);
            table->selectRow(i);
            return;
        }
    }
}

void ModuleParticipants::pointerSelection(bool depuisBorne)
{
    const InfoParticipant *p = participantDeLigne(ligneSelectionnee());
    if (!p) {
        lblBorneMessage->setText("Aucun participant sélectionné.");
        lblBorneMessage->setStyleSheet("font-size: 11px; color: #b45309;");
        if (!depuisBorne)
            QMessageBox::information(m_page, "Barrière d'accès", "Cliquez d'abord sur la ligne d'un participant (ou tournez le potentiomètre).");
        borne->repondre(ResultatPointage::Introuvable);
        return;
    }
    const QString id = p->id;
    const InfoPointage r = Participant::pointer(id);   // décision prise dans Oracle
    borne->repondre(r.resultat);                        // la barrière s'ouvre ou reste fermée

    const bool ok = (r.resultat == ResultatPointage::Accepte);
    lblBorneMessage->setText(QString("%1 : %2").arg(r.nom, r.message));
    lblBorneMessage->setStyleSheet(QString("font-size: 11px; color: %1;")
        .arg(ok ? "#15803d" : (r.resultat == ResultatPointage::DejaPointe ? "#b45309" : "#b91c1c")));
    chargerDonneesSQL();
    selectionnerParId(id);
}
