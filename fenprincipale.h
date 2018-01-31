#ifndef FENPRINCIPALE_H
#define FENPRINCIPALE_H

#include <QtWidgets>
#include <QtSql>
#include <vector>
#include <QStringBuilder>

class FenPrincipale : public QMainWindow
{
    Q_OBJECT
private:
    bool connecte;

    QSqlDatabase bdd;

    QStatusBar *barreDeStatut;

    QLineEdit *nomCarte;
    QSpinBox  *nbEx;
    QSpinBox  *prix;
    QComboBox *venteOrAchat;
    QDateTimeEdit *dateTransaction;

    QPushButton *ajouter;

    QSqlQueryModel *requeteAchats;
    QSqlQueryModel *requeteVentes;
    QTableView *tableauAchats;
    QTableView *tableauVentes;

    bool eventFilter(QObject *obj, QEvent *ev);

public:
    FenPrincipale();
    bool getConnecte();
    void setDefaultFocus();

signals:

public slots:
    void aPropos();
    void ajoutEntree();
    void refreshViews();
};

#endif // FENPRINCIPALE_H
