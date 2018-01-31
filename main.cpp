#include <QApplication>
#include <QtWidgets>
#include "fenprincipale.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    FenPrincipale fenetre;
    if(fenetre.getConnecte()) {
        fenetre.show();
        fenetre.setDefaultFocus();
    } else {
        qApp->quit;
    }

    return app.exec();
}
