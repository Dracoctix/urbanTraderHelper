#include "fenprincipale.h"

FenPrincipale::FenPrincipale() : QMainWindow()
{
    // ÉCRAN DE CHARGEMENT
    QPixmap *imageChargement = new QPixmap("photo.jpg","JPG");
    QSplashScreen *ecranChargement = new QSplashScreen(this, *imageChargement,Qt::SplashScreen);

    ecranChargement->show();

    connecte = true; // Pour savoir si on est connecté correctement à la BDD, et si on doit fermer le programme.
    // MANAGEMENT BDD
    bdd = QSqlDatabase::addDatabase("QSQLITE");
    bdd.setDatabaseName("storage.sqlite");

    if(bdd.open()) { // La BDD est-elle accessible ?

        int repDialog; // Variable qui servira à stocker la réponse de l'utilisateur à la boîte de dialogue.

        // On paramètre la BDD si c'est le premier lancement
        QSqlQuery requeteExecution(bdd);
        QSqlQuery requeteVerification(bdd);
        std::vector<QString> tabReq(1);
        std::vector<QString> tabVerif(1); // Vector qui vérifie si la table existe.
        tabReq[0] = "CREATE TABLE Transact(idTransact INTEGER PRIMARY KEY NOT NULL, nomCarte VARCHAR(75) NOT NULL, nbCartes INTEGER DEFAULT 1, prixU INTEGER NOT NULL DEFAULT 50, dateTransact DATETIME DEFAULT CURRENT_TIMESTAMP, typeTransact BYTE NOT NULL);";
        tabVerif[0] = "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = 'Transact';";

        for(int i = 0; i < tabReq.size(); i++) {
            if(requeteVerification.exec(tabVerif[i])) {
                requeteVerification.next();
                if(requeteVerification.value(0) == 0) {
                    if(!requeteExecution.exec(tabReq[i])) {
                        repDialog = QMessageBox::critical(this, tr("Exécution de la requête impossible !"), tr("Une requête de démarrage n'a pu être effectuée. Cela peut signifier qu'il y a un problème de connexion avec la base de données. Des erreurs peuvent donc survenir. Souhaitez-vous continuer ?<br>(Erreur : <em>") + requeteExecution.lastError().driverText() + "</em>).", QMessageBox::Yes | QMessageBox::No);
                        if (repDialog == QMessageBox::No) {
                            connecte = false;
                        }
                    }
                }

            } else {
                QMessageBox::critical(this, tr("Impossible de lire les informations de la base de données."), tr("La base de données est illisible. Vérifiez que vous avez les droits de lecture sur tous les fichiers de votre dossier."));
                connecte = false;
            }
        }

        // Paramètrage des métadonnées de la fenêtre
        this->setWindowTitle(tr("Urban Trader Helper"));
        this->resize(800,600);

        // Gestion des actions du menu
        QAction *actionAbout = new QAction(tr("À &propos"), this);
        actionAbout->setEnabled(false);
        QAction *actionQuit = new QAction(tr("&Quitter"), this);
        actionQuit->setShortcut(QKeySequence(tr("Ctrl+Q", "Raccourci clavier pour la fermeture du programme.")));
        QAction *actionRefresh = new QAction(tr("&Actualiser"), this);
        actionRefresh->setShortcut(QKeySequence(tr("Ctrl+R", "Raccourci clavier pour l'actualisation.")));

        // Création des menus
        QMenu *menuFichier = menuBar()->addMenu(tr("&Fichier"));
        menuBar()->addAction(actionRefresh);
        menuBar()->addAction(actionAbout);


        menuFichier->addAction(actionQuit);

        // Création de la barre de statut
        barreDeStatut = new QStatusBar(this);
        this->setStatusBar(barreDeStatut);

        // On définit les Widgets parents, utilisés à différents endroits
        QWidget *zoneCentrale = new QWidget;

        QWidget *zoneTabAchat = new QWidget;
        QWidget *zoneTabVente = new QWidget;

//        QWidget *zoneFormAjout = new QWidget;
        QGroupBox *boxFormAjout = new QGroupBox;
        boxFormAjout->setTitle("Ajouter une transaction");

        // On créé les widgets figurant dans la zone centrale
        QGridLayout *layout = new QGridLayout;
        QTabWidget *onglets = new QTabWidget;
        QFormLayout *formAjout = new QFormLayout;
        boxFormAjout->setLayout(formAjout);

        // On créé les onglets
        onglets->addTab(zoneTabAchat,"Achats");
        onglets->addTab(zoneTabVente,"Ventes");

        QGridLayout *layoutAchats = new QGridLayout;
        zoneTabAchat->setLayout(layoutAchats);
        QGridLayout *layoutVentes = new QGridLayout;
        zoneTabVente->setLayout(layoutVentes);

        // On créé les champs de formulaire
        nomCarte = new QLineEdit;
        nomCarte->setPlaceholderText(tr("Nom de la carte"));
        nbEx = new QSpinBox;
        nbEx->setValue(1);
        prix = new QSpinBox;
        prix->setMinimum(50);
        prix->setMaximum(50000000);
        venteOrAchat = new QComboBox;
        venteOrAchat->addItem(tr("Achat"));
        venteOrAchat->addItem(tr("Vente"));
        dateTransaction = new QDateTimeEdit;
        dateTransaction->setDateTime(QDateTime::currentDateTime());
        ajouter = new QPushButton;
        ajouter->setText(tr("Enregistrer l'opération"));

        // On définit le formulaire d'ajout.
        formAjout->addRow(tr("Carte : "), nomCarte);
        formAjout->addRow(tr("Nombre d'exemplaires : "), nbEx);
        formAjout->addRow(tr("Prix à l'unité : "), prix);
        formAjout->addRow(tr("Date d'achat : "), dateTransaction);
        formAjout->addRow(venteOrAchat);
        formAjout->addRow(ajouter);

        // On ajoute les onglets au layout de la zone centrale.
        layout->addWidget(onglets,0,0,1,2);
        layout->addWidget(boxFormAjout,0,2);

        // On définit le layout comme layout de la zone centrale, puis la zone centrale comme widget central de la fenêtre.
        zoneCentrale->setLayout(layout);
        setCentralWidget(zoneCentrale);

        // Partie modèle de l'affichage
        requeteAchats = new QSqlQueryModel;
        QSqlQuery selectAchats(bdd);
        if(!selectAchats.exec("SELECT nomCarte, nbCartes, prixU, prixU*nbCartes AS prixTotal, dateTransact FROM Transact WHERE typeTransact = 0 ORDER BY dateTransact DESC;")) {
            repDialog = QMessageBox::critical(this, tr("Impossible de lire la base de données."), tr("Il nous est impossible de lire les données. Voulez-vous continuer ?"), QMessageBox::Yes | QMessageBox::No);
            if (repDialog == QMessageBox::No) {
                connecte = false;
            }
        }
        requeteAchats->setQuery(selectAchats);
        requeteAchats->setHeaderData(0,Qt::Horizontal,tr("Nom de la carte"));
        requeteAchats->setHeaderData(1, Qt::Horizontal, tr("Nombre de cartes"));
        requeteAchats->setHeaderData(2,Qt::Horizontal, tr("Prix unitaire"));
        requeteAchats->setHeaderData(3, Qt::Horizontal, tr("Prix total"));
        requeteAchats->setHeaderData(4,Qt::Horizontal, tr("Date d'achat"));

        // Pour les ventes
        requeteVentes = new QSqlQueryModel;
        QSqlQuery selectVentes(bdd);
        if(!selectVentes.exec("SELECT nomCarte, nbCartes, prixU, prixU*nbCartes AS prixTotal, dateTransact FROM Transact WHERE typeTransact = 1 ORDER BY dateTransact DESC;")) {
            repDialog = QMessageBox::critical(this, tr("Impossible de lire la base de données."), tr("Il nous est impossible de lire les données. Voulez-vous continuer ?"), QMessageBox::Yes | QMessageBox::No);
            if (repDialog == QMessageBox::No) {
                connecte = false;
            }
        }
        requeteVentes->setQuery(selectAchats);
        requeteVentes->setHeaderData(0,Qt::Horizontal,tr("Nom de la carte"));
        requeteVentes->setHeaderData(1, Qt::Horizontal, tr("Nombre de cartes"));
        requeteVentes->setHeaderData(2,Qt::Horizontal, tr("Prix unitaire"));
        requeteVentes->setHeaderData(3, Qt::Horizontal, tr("Prix total"));
        requeteVentes->setHeaderData(4,Qt::Horizontal, tr("Date de vente"));

        // PARTIE VUE DE L'AFFICHAGE
        tableauAchats = new QTableView;
        tableauVentes = new QTableView;
        tableauAchats->setModel(requeteAchats);
        layoutAchats->addWidget(tableauAchats,0,0);
        tableauAchats->verticalHeader()->setVisible(false);
        tableauVentes->setModel(requeteVentes);
        layoutVentes->addWidget(tableauVentes,0,0);
        tableauAchats->verticalHeader()->setVisible(false);

        // On fait la police de taille
        QSizePolicy sizePolicyOnglets(QSizePolicy::Expanding, QSizePolicy::Expanding);
        onglets->setSizePolicy(sizePolicyOnglets);
        QSizePolicy sizePolicyFormulaire(QSizePolicy::Fixed, QSizePolicy::Expanding);
        boxFormAjout->setSizePolicy(sizePolicyFormulaire);

        // Connexion signaux et slots
        QObject::connect(actionAbout, SIGNAL(triggered()), this, SLOT(aPropos()));
        QObject::connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
        QObject::connect(ajouter, SIGNAL(clicked()), this, SLOT(ajoutEntree()));

        QObject::connect(nomCarte, SIGNAL(returnPressed()), ajouter, SIGNAL(clicked()));
        QObject::connect(actionRefresh, SIGNAL(triggered()), this, SLOT(refreshViews()));

        prix->installEventFilter(this);
        venteOrAchat->installEventFilter(this);
        dateTransaction->installEventFilter(this);
        ajouter->installEventFilter(this);
        nbEx->installEventFilter(this);

        bdd.close();
    }
    else {
        connecte = false;
        QMessageBox::critical(this, tr("Problème de stockage", "La base de données ne peut être jointe"), tr("Impossible d'accéder à la base de données. Vérifiez que vous ayiez les droits d'écriture sur le dossier d'installation."));
    }
    ecranChargement->close();
}

void FenPrincipale::aPropos() {
    QMessageBox::about(this, tr("À propos"), "");
}

bool FenPrincipale::getConnecte() {
    return connecte;
}

void FenPrincipale::ajoutEntree() {
    barreDeStatut->showMessage(tr("Insertion en cours..."));
    if (bdd.open()) {
        QSqlQuery ajout(bdd);
        ajout.prepare("INSERT INTO Transact(nomCarte, nbCartes, prixU, dateTransact, typeTransact) VALUES(:nomCarte, :nbCartes, :prixU, :date, :type);");

        ajout.bindValue(":type", venteOrAchat->currentIndex());
        ajout.bindValue(":nomCarte", nomCarte->text());
        ajout.bindValue(":nbCartes", nbEx->value());
        ajout.bindValue(":prixU", prix->value());
        ajout.bindValue(":date", dateTransaction->dateTime().toString("yyyy-MM-dd hh:mm:ss"));

//        ajout.prepare("INSERT INTO Transact(nomCarte, typeTransact) VALUES('Caca', 1);");

//        QMessageBox::information(this, "Debug info", ajout.executedQuery());

        if(ajout.exec()) {
            QMessageBox::information(this, tr("Succès !"), tr("L'enregistrement a fonctionné avec succès !"));
            nomCarte->clear();
            nbEx->setValue(1);
            prix->setValue(50);
            dateTransaction->setDateTime(QDateTime::currentDateTime());
        } else {
            QMessageBox::critical(this, tr("Échec de l'enregistrement."), tr("L'enregistrement de la transaction dans la base de données a échoué. Erreur : ") + ajout.lastError().driverText());
        }

        bdd.close();

        refreshViews();

    } else {
        QMessageBox::critical(this, tr("Problème de stockage", "La base de données ne peut être jointe"), tr("Impossible d'accéder à la base de données. Vérifiez que vous ayiez les droits d'écriture sur le dossier d'installation."));
        qApp->quit();
    }

    barreDeStatut->clearMessage();
}

void FenPrincipale::setDefaultFocus() {
    nomCarte->setFocus();
}

bool FenPrincipale::eventFilter(QObject *obj, QEvent *ev) {
    if(obj == prix || obj == nbEx || obj == venteOrAchat || obj == ajouter || obj == dateTransaction) {
        if(ev->type() == QEvent::KeyPress) {
            QKeyEvent *event = static_cast<QKeyEvent *>(ev);
            if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
                ajouter->click();
                return true;
            }
        }
    }
    return false;
}

void FenPrincipale::refreshViews() {
    barreDeStatut->showMessage(tr("Actualisation..."));
    bdd.open();
    int repDialog;
    QSqlQuery refreshAchats(bdd);
    QSqlQuery refreshVentes(bdd);

    if(!refreshAchats.exec("SELECT nomCarte, nbCartes, prixU, prixU*nbCartes AS prixTotal, dateTransact FROM Transact WHERE typeTransact = 0 ORDER BY dateTransact DESC;")) {
        repDialog = QMessageBox::critical(this, tr("Impossible de lire la base de données."), tr("Il nous est impossible de lire les données. Voulez-vous continuer ?"), QMessageBox::Yes | QMessageBox::No);
        if (repDialog == QMessageBox::No) {
            qApp->quit();
        }
    }
    requeteAchats->setQuery(refreshAchats);
    requeteAchats->setHeaderData(0,Qt::Horizontal,tr("Nom de la carte"));
    requeteAchats->setHeaderData(1, Qt::Horizontal, tr("Nombre de cartes"));
    requeteAchats->setHeaderData(2,Qt::Horizontal, tr("Prix unitaire"));
    requeteAchats->setHeaderData(3, Qt::Horizontal, tr("Prix total"));
    requeteAchats->setHeaderData(4,Qt::Horizontal, tr("Date d'achat"));

    if(!refreshVentes.exec("SELECT nomCarte, nbCartes, prixU, prixU*nbCartes AS prixTotal, dateTransact FROM Transact WHERE typeTransact = 1 ORDER BY dateTransact DESC;")) {
        repDialog = QMessageBox::critical(this, tr("Impossible de lire la base de données."), tr("Il nous est impossible de lire les données. Voulez-vous continuer ?"), QMessageBox::Yes | QMessageBox::No);
        if (repDialog == QMessageBox::No) {
            qApp->quit();
        }
    }
    requeteVentes->setQuery(refreshVentes);
    requeteVentes->setHeaderData(0,Qt::Horizontal,tr("Nom de la carte"));
    requeteVentes->setHeaderData(1, Qt::Horizontal, tr("Nombre de cartes"));
    requeteVentes->setHeaderData(2,Qt::Horizontal, tr("Prix unitaire"));
    requeteVentes->setHeaderData(3, Qt::Horizontal, tr("Prix total"));
    requeteVentes->setHeaderData(4,Qt::Horizontal, tr("Date de vente"));

    bdd.close();

    barreDeStatut->clearMessage();
}
