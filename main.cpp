#include "ateliermainwindow.h"
#include "connexion.h"

#include <QApplication>
#include <QMessageBox>
#include <QStatusBar>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("HackTimeAtelier");
    a.setStyle("Fusion");   // rendu identique sur toutes les plateformes

    QString msg;
    if (!Connexion::open(&msg)) {
        QMessageBox::critical(nullptr, "Base de données", msg);
        return 1;
    }

    AtelierMainWindow w;
    w.statusBar()->showMessage(msg);
    w.show();
    return a.exec();
}
