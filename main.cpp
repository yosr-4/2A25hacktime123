#include "hacktime.h"
#include "connection.h"

#include <QApplication>
#include <QIcon>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("HackTime");
    a.setStyle("Fusion");   // même rendu pour tous les modules
    a.setWindowIcon(QIcon(":/images/icon.png"));

    // connexion commune à la base Oracle (classe Connection, Singleton)
    Connection &c = Connection::createInstance();
    if (c.createConnection())
        QMessageBox::information(nullptr, "Base de données", "Connexion réussie.");
    else
        QMessageBox::critical(nullptr, "Base de données", "Échec de la connexion :\n" + c.lastError());

    HackTime w;
    w.show();
    return a.exec();
}
