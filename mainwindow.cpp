#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "paneldialog.h"
#include "orfeoclient.h"

#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QScrollArea>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(1150, 780);

    epl = new ledBar(this);
    udpReceiver = new UDPClient(this);
    connect(udpReceiver, SIGNAL(sigUdpDataReceived(QString)), this, SLOT(udpDatagramReceived(QString)));
    connect(epl, SIGNAL(sigLedConnected()), this, SLOT(eplConnected()));
    connect(epl, SIGNAL(sigLedDisconnected()), this, SLOT(eplDisconnected()));

    connect(ui->monthSchedule, SIGNAL(toggled(bool)), ui->fromMonth, SLOT(setDisabled(bool)));
    connect(ui->monthSchedule, SIGNAL(toggled(bool)), ui->toMonth, SLOT(setDisabled(bool)));
    connect(ui->daySchedule, SIGNAL(toggled(bool)), ui->fromDay, SLOT(setDisabled(bool)));
    connect(ui->daySchedule, SIGNAL(toggled(bool)), ui->toDay, SLOT(setDisabled(bool)));
    connect(ui->timeSchedule, SIGNAL(toggled(bool)), ui->fromHTime, SLOT(setDisabled(bool)));
    connect(ui->timeSchedule, SIGNAL(toggled(bool)), ui->fromMTime, SLOT(setDisabled(bool)));
    connect(ui->timeSchedule, SIGNAL(toggled(bool)), ui->toHTime, SLOT(setDisabled(bool)));
    connect(ui->timeSchedule, SIGNAL(toggled(bool)), ui->toMTime, SLOT(setDisabled(bool)));

    _orfeoClient = new OrfeoClient(this);
    _orfeoTimer  = new QTimer(this);
    connect(_orfeoClient, &OrfeoClient::eventsReceived, this, &MainWindow::orfeoEventsReceived);
    connect(_orfeoClient, &OrfeoClient::errorOccurred,  this, &MainWindow::orfeoError);
    connect(_orfeoTimer,  &QTimer::timeout,             this, &MainWindow::orfeoAutoSyncTick);

    _orfeoClient->setApiUrl(OrfeoConfig::apiUrl());
    _orfeoClient->setApiKey(OrfeoConfig::apiKey());

    loadPanels();
    setupOrfeoTab();
}

void MainWindow::loadPanels()
{
    _panels = PanelConfig::load();
    ui->ip->clear();
    for (const Panel &p : _panels)
        ui->ip->addItem(p.name, p.ip);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::getFont()
{
    QString font;
    switch (ui->font->currentIndex())
    {
    case 0: font = "\\g0"; break;
    case 1: font = "\\g1"; break;
    case 2: font = "\\g2"; break;
    case 3: font = "\\g3"; break;
    case 4: font = "\\g4"; break;
    case 5: font = "\\g5"; break;
    case 6: font = "\\g6"; break;
    case 7: font = "\\g7"; break;
    case 8: font = "\\g8"; break;
    case 9: font = "\\g9"; break;
    }
    return font;
}

QString MainWindow::getInAnim()
{
    QString inAnim;
    switch (ui->inAnim->currentIndex())
    {
    case 0: inAnim = "\\edpc"; break;
    case 1: inAnim = "\\egpc"; break;
    case 2: inAnim = "\\ehpc"; break;
    case 3: inAnim = "\\ebpc"; break;
    case 4: inAnim = "\\ecpc"; break;
    case 5: inAnim = "\\eepc"; break;
    case 6: inAnim = "\\erpc"; break;
    case 7: inAnim = "\\eipc"; break;
    }
    return inAnim;
}

QString MainWindow::getOutAnim()
{
    QString outAnim;
    switch (ui->outAnim->currentIndex())
    {
    case 0: outAnim = ""; break;
    case 1: outAnim = "\\sd"; break;
    case 2: outAnim = "\\sg"; break;
    case 3: outAnim = "\\sh"; break;
    case 4: outAnim = "\\sb"; break;
    case 5: outAnim = "\\sc"; break;
    case 6: outAnim = "\\se"; break;
    case 7: outAnim = "\\sr"; break;
    case 8: outAnim = "\\si"; break;

    }
    return outAnim;
}

QString MainWindow::getSpeed()
{
    QString speed;
    speed = "\\k" + QString::number(ui->animSpeed->value());
    return speed;
}

QString MainWindow::getTimeStop()
{
    QString stopTime;
    if (ui->stopTime->value() == 0)
    {
        stopTime = "";
    }
    else if (ui->stopTime->value() < 10)
    {
        stopTime = "\\a" + QString::number(ui->stopTime->value());
    }
    else
    {
        stopTime = "\\a0" + QString::number(ui->stopTime->value());
    }

    return stopTime;
}

QString MainWindow::getEffect()
{
    QString effect;
    switch (ui->effect->currentIndex())
    {
    case 0: effect = ""; break;
    case 1: effect = "\\w"; break;
    case 2: effect = "\\`-"; break;
    }
    return effect;
}

QString MainWindow::getMsgBank()
{
    int row = ui->msgTable->currentRow() + 1;
    return (row < 10 ? QString("MSG0") : QString("MSG")) + QString::number(row) + " |";
}

QString MainWindow::getZone()
{
    int zone = ui->displayZoneNumber->value() - 1;
    if (zone == 0) return "";
    return "\\m" + QString::number(zone) + "t";
}

QString MainWindow::getRepeatStart()
{
    int r = ui->repeat->value();
    if (r < 2) return "";
    return r <= 9 ? "\\r" + QString::number(r) : "\\r0" + QString::number(r);
}

QString MainWindow::getRepeatEnd()
{
    return ui->repeat->value() >= 2 ? QString("\\f") : QString();
}

QString MainWindow::getBlink()
{
    int b = qMin(ui->blinkTime->value(), 9);
    if (b == 0) return "";
    return "\\c" + QString::number(b);
}

QString MainWindow::composeMessage()
{
    return getMsgBank() + getZone() + getFont() + getEffect() + getSpeed() +
           getRepeatStart() + getInAnim() +
           "   " + ui->msgTable->currentItem()->text() + "   " +
           getTimeStop() + getBlink() + getRepeatEnd() + getOutAnim();
}


// ******************* SLOTS ********************* //

void MainWindow::on_display_clicked()
{
    if (epl->isConnected())
    {
        if (!ui->msgTable->selectedItems().isEmpty())
        {
            epl->authorize(ui->msgTable->currentRow() + 1);
            this->updateCurrentMsgNum();
            ui->statusBar->showMessage("Transmission OK !", 5000);
        }
        else
        {
            ui->statusBar->showMessage("Invalid command : select a bank number first !", 5000);
        }
    }
    else
    {
        ui->statusBar->showMessage("Invalid command : select a bank number first !", 5000);
    }
}

void MainWindow::on_save_clicked()
{
    if (epl->isConnected())
    {
        if (!ui->msgTable->selectedItems().isEmpty())
        {
            epl->sendMessage(composeMessage());
            ui->statusBar->showMessage("Transmission OK !", 5000);
        }
        else
        {
            ui->statusBar->showMessage("Action impossible : sélectionnez une mémoire !", 5000);
        }
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }
}

void MainWindow::on_connect_clicked()
{
    if (ui->connect->isChecked())
    {
        QString ip = ui->custom_ip->text().isEmpty()
                     ? ui->ip->currentData().toString()
                     : ui->custom_ip->text();

        if (ip.isEmpty()) {
            ui->connect->setChecked(false);
            ui->statusBar->showMessage("Aucun panneau sélectionné.", 5000);
            return;
        }

        epl->connection(ip, 23);

        if (!epl->isConnected())
        {
            ui->connect->setChecked(false);
            ui->statusBar->showMessage("Impossible de se connecter... vérifiez vos paramètres et votre connexion réseau.", 5000);
        }
    }
    else
    {
        epl->closeConnection();
    }
}

void MainWindow::on_managePanels_clicked()
{
    PanelDialog dlg(_panels, this);
    if (dlg.exec() == QDialog::Accepted) {
        _panels = dlg.panels();
        PanelConfig::save(_panels);
        loadPanels();
    }
}

void MainWindow::on_freezeMode_clicked()
{
    if (epl->isConnected())
    {
        epl->freeze();
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }
}

void MainWindow::on_runMode_clicked()
{
    if (epl->isConnected())
    {
        epl->run();
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }
}

void MainWindow::on_clearDevice_clicked()
{
    if (epl->isConnected())
    {
        epl->clear();
        ui->msgTable->clearContents();
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }
}

void MainWindow::on_rebootDevice_clicked()
{
    if (epl->isConnected())
    {
        epl->reboot();
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }
}

void MainWindow::on_ledIntensity_sliderMoved(int position)
{
    if (epl->isConnected())
    {
        epl->brightness(position);
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    if (arg1 == Qt::Unchecked)
    {
        ui->groupTriggersConfig->setEnabled(false);
        udpReceiver->unbind();
    }
    else if (arg1 == Qt::Checked)
    {
        ui->groupTriggersConfig->setEnabled(true);
        QString port = ui->triggerPort->text();
        udpReceiver->bind(port.toInt());
    }
}

void MainWindow::updateCurrentMsgNum()
{
  QString msgNum;
  msgNum = epl->getCurrentMsgNumber();

  int msgIndex;
  msgIndex = msgNum.toInt();

  for (int i = 0; i < 10 ; i++)
  {
      ui->msgTable->setVerticalHeaderItem(i,new QTableWidgetItem(QString::number(i + 1)));
  }

  ui->msgTable->setVerticalHeaderItem(msgIndex - 1,new QTableWidgetItem(QString::number(msgIndex) + " *"));
}

// ****************** SLOTS ********************* //

void MainWindow::udpDatagramReceived(QString datagram)
{
    int value = datagram.remove(0,ui->triggerDisplay->text().size()).toInt();
    epl->authorize(value);
}

void MainWindow::eplConnected()
{
    for (int i=0 ; i<10 ; i++)
    {
        QString resp;
        resp = epl->getStoredMessage(i+1);
        resp.remove(0, resp.indexOf("   ") + 3);
        resp.truncate(resp.indexOf("   "));
        ui->msgTable->setItem(i, 0, new QTableWidgetItem(resp));
    }
    ui->ip->setEnabled(false);
    ui->custom_ip->setEnabled(false);
    ui->connect->setChecked(true);
    ui->connect->setText("Déconnecter");
    ui->groupIP->setEnabled(true);
    ui->groupPlanning->setEnabled(true);
    ui->groupZone->setEnabled(true);
    ui->statusBar->showMessage("Connecté au journal !", 5000);
    this->updateCurrentMsgNum();
}

void MainWindow::eplDisconnected()
{
    for (int i = 0; i < 10 ; i++)
    {
        ui->msgTable->setVerticalHeaderItem(i,new QTableWidgetItem(QString::number(i + 1)));
    }
    ui->ip->setEnabled(true);
    ui->custom_ip->setEnabled(true);
    ui->connect->setChecked(false);
    ui->connect->setText("Connecter");
    ui->groupIP->setEnabled(false);
    ui->groupPlanning->setEnabled(false);
    ui->groupZone->setEnabled(false);
    ui->msgTable->clearContents();
    ui->statusBar->showMessage("Journal déconnecté !", 5000);
}

void MainWindow::on_clrBank_clicked()
{
    if (epl->isConnected())
    {
        if (!ui->msgTable->selectedItems().isEmpty())
        {
            epl->clearBank(ui->msgTable->currentRow() + 1);
            ui->msgTable->currentItem()->setText("");
            epl->run();
        }
        else
        {
            ui->statusBar->showMessage("Action impossible : sélectionnez une mémoire !", 5000);
        }
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }
}


void MainWindow::on_authorizeAll_clicked()
{
    if (epl->isConnected())
    {
        epl->authorizeAll();
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }
}

void MainWindow::on_authorizeAll_2_clicked()
{
    if (epl->isConnected())
    {
        epl->authorizeAll();
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }

}

void MainWindow::on_blank_clicked()
{
    if (epl->isConnected())
    {
        epl->blank();
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }

}

void MainWindow::on_clearSchedule_clicked()
{
    if (epl->isConnected())
    {

        epl->clearSchedule(ui->msgTable->currentRow() + 1);
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }
}

void MainWindow::on_saveIP_clicked()
{
    if (epl->isConnected())
    {
        epl->setIpAddress(ui->deviceIP->text(), ui->deviceMask->text(), ui->deviceGateway->text());
    }
    else
    {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
    }
}

QString MainWindow::buildScheduleStr()
{
    static const QStringList MONTHS = {"JAN","FEV","MAR","AVR","MAI","JUN","JUL","AOU","SEP","OCT","NOV","DEC"};
    static const QStringList DAYS   = {"LU","MA","ME","JE","VE","SA","DI"};

    auto fmtTime = [](int h, int m) {
        return QString("%1:%2").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0'));
    };

    QStringList parts;

    if (!ui->monthSchedule->isChecked())
        parts << MONTHS[ui->fromMonth->currentIndex()] + "/" + MONTHS[ui->toMonth->currentIndex()];

    if (!ui->daySchedule->isChecked()) {
        QString tFrom = ui->timeSchedule->isChecked() ? "00:00" : fmtTime(ui->fromHTime->value(), ui->fromMTime->value());
        QString tTo   = ui->timeSchedule->isChecked() ? "24:00" : fmtTime(ui->toHTime->value(),   ui->toMTime->value());
        parts << DAYS[ui->fromDay->currentIndex()] + " " + tFrom + "/" +
                 DAYS[ui->toDay->currentIndex()]   + " " + tTo;
    } else if (!ui->timeSchedule->isChecked()) {
        parts << fmtTime(ui->fromHTime->value(), ui->fromMTime->value()) + "/" +
                 fmtTime(ui->toHTime->value(),   ui->toMTime->value());
    }

    return parts.join(",");
}

void MainWindow::on_addSchedule_clicked()
{
    if (!epl->isConnected()) {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
        return;
    }
    if (ui->msgTable->selectedItems().isEmpty()) {
        ui->statusBar->showMessage("Action impossible : sélectionnez une mémoire !", 5000);
        return;
    }
    QString plage = buildScheduleStr();
    if (plage.isEmpty()) {
        ui->statusBar->showMessage("Aucune restriction définie (tout coché).", 5000);
        return;
    }
    epl->addSchedule(ui->msgTable->currentRow() + 1, plage);
    ui->statusBar->showMessage("Plage horaire ajoutée !", 5000);
}

void MainWindow::on_delSchedule_clicked()
{
    if (!epl->isConnected()) {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
        return;
    }
    if (ui->msgTable->selectedItems().isEmpty()) {
        ui->statusBar->showMessage("Action impossible : sélectionnez une mémoire !", 5000);
        return;
    }
    QString plage = buildScheduleStr();
    if (plage.isEmpty()) {
        ui->statusBar->showMessage("Aucune plage à supprimer.", 5000);
        return;
    }
    epl->delSchedule(ui->msgTable->currentRow() + 1, plage);
    ui->statusBar->showMessage("Plage horaire supprimée !", 5000);
}

void MainWindow::on_zoneSet_clicked()
{
    if (!epl->isConnected()) {
        ui->statusBar->showMessage("Action impossible : veuillez d'abord vous connecter !", 5000);
        return;
    }
    int n = ui->zoneNumber->value();
    if (n == 0) {
        ui->statusBar->showMessage("Configurez au moins 1 zone.", 5000);
        return;
    }
    struct { QSpinBox *from; QSpinBox *to; } z[] = {
        {ui->fromZone1, ui->toZone1},
        {ui->fromZone2, ui->toZone2},
        {ui->fromZone3, ui->toZone3},
        {ui->fromZone4, ui->toZone4}
    };
    QStringList zones;
    for (int i = 0; i < n; i++)
        zones << QString("%1:%2-%3").arg(i).arg(z[i].from->value()).arg(z[i].to->value());
    epl->setZones(zones.join(","));
    ui->statusBar->showMessage("Zones configurées !", 5000);
}

// ==================== API ORFEO TAB ====================

static QWidget *centeredCheckBox(QCheckBox *cb)
{
    QWidget *w = new QWidget;
    QHBoxLayout *l = new QHBoxLayout(w);
    l->addWidget(cb);
    l->setAlignment(Qt::AlignCenter);
    l->setContentsMargins(0, 0, 0, 0);
    return w;
}

void MainWindow::setupOrfeoTab()
{
    QWidget *tab = ui->tabs->findChild<QWidget*>("orfeoTab");
    if (!tab) return;

    QVBoxLayout *root = new QVBoxLayout(tab);
    root->setSpacing(10);
    root->setContentsMargins(10, 10, 10, 10);

    // ── Group: Configuration API ─────────────────────────────────────────────
    QGroupBox *apiGroup = new QGroupBox("Configuration API Orfeo");
    QVBoxLayout *apiVBox = new QVBoxLayout(apiGroup);
    apiVBox->setSpacing(6);

    QFormLayout *apiForm = new QFormLayout;
    apiForm->setLabelAlignment(Qt::AlignRight);
    apiForm->setHorizontalSpacing(12);

    _orfeoApiUrlEdit = new QLineEdit(OrfeoConfig::apiUrl());
    _orfeoApiUrlEdit->setPlaceholderText("https://votre-instance.orfeo.pro");
    _orfeoApiKeyEdit = new QLineEdit(OrfeoConfig::apiKey());
    _orfeoApiKeyEdit->setEchoMode(QLineEdit::Password);
    _orfeoApiKeyEdit->setPlaceholderText("Clé API ou token d'authentification");

    apiForm->addRow("URL de l'instance :", _orfeoApiUrlEdit);
    apiForm->addRow("Clé API :", _orfeoApiKeyEdit);
    apiVBox->addLayout(apiForm);

    QHBoxLayout *apiButtons = new QHBoxLayout;
    QPushButton *testBtn    = new QPushButton("Tester la connexion");
    QPushButton *saveApiBtn = new QPushButton("Enregistrer");
    testBtn->setFixedWidth(160);
    saveApiBtn->setFixedWidth(120);
    _orfeoStatusLabel = new QLabel("—");
    _orfeoStatusLabel->setStyleSheet("color: gray;");
    apiButtons->addWidget(testBtn);
    apiButtons->addWidget(saveApiBtn);
    apiButtons->addSpacing(16);
    apiButtons->addWidget(_orfeoStatusLabel);
    apiButtons->addStretch();
    apiVBox->addLayout(apiButtons);

    // fixed height: API group never needs to grow
    apiGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    root->addWidget(apiGroup);

    // ── Group: Événements à venir ─────────────────────────────────────────────
    QGroupBox *eventsGroup = new QGroupBox("Événements à venir");
    QVBoxLayout *eventsVBox = new QVBoxLayout(eventsGroup);
    eventsVBox->setSpacing(6);

    _orfeoEventsTable = new QTableWidget(0, 5);
    _orfeoEventsTable->setHorizontalHeaderLabels({"Date", "Heure", "Titre", "Salle", "Personnel"});
    _orfeoEventsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _orfeoEventsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    _orfeoEventsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    _orfeoEventsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _orfeoEventsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    _orfeoEventsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    eventsVBox->addWidget(_orfeoEventsTable, 1);   // stretch=1 → fills remaining group height

    QHBoxLayout *eventsBar = new QHBoxLayout;
    QPushButton *refreshBtn = new QPushButton("Actualiser");
    refreshBtn->setFixedWidth(120);
    _orfeoAutoSyncCheck = new QCheckBox("Sync automatique toutes les");
    _orfeoIntervalSpin  = new QSpinBox;
    _orfeoIntervalSpin->setRange(1, 1440);
    _orfeoIntervalSpin->setValue(30);
    _orfeoIntervalSpin->setSuffix(" min");
    _orfeoIntervalSpin->setFixedWidth(90);
    eventsBar->addWidget(refreshBtn);
    eventsBar->addSpacing(20);
    eventsBar->addWidget(_orfeoAutoSyncCheck);
    eventsBar->addWidget(_orfeoIntervalSpin);
    eventsBar->addStretch();
    eventsVBox->addLayout(eventsBar);  // toolbar sits below table, inside group

    root->addWidget(eventsGroup, 2);   // gets 2/5 of remaining space

    // ── Group: Configuration par panneau ─────────────────────────────────────
    QGroupBox *panelGroup = new QGroupBox("Configuration par panneau");
    QVBoxLayout *panelVBox = new QVBoxLayout(panelGroup);
    panelVBox->setSpacing(6);

    const QStringList panelCols = {
        "Panneau", "Mémoire", "Titre", "Date", "Heure",
        "Salle", "Personnel", "Filtre salle", "Intervalle", "Actif"
    };
    _orfeoPanelTable = new QTableWidget(0, panelCols.size());
    _orfeoPanelTable->setHorizontalHeaderLabels(panelCols);
    _orfeoPanelTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _orfeoPanelTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    _orfeoPanelTable->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);
    _orfeoPanelTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Load saved configs; seed from panel list if none saved yet
    _orfeoConfigs = OrfeoConfig::load();
    if (_orfeoConfigs.isEmpty()) {
        for (const Panel &p : _panels) {
            PanelOrfeoConfig c;
            c.panelName = p.name;
            c.panelIp   = p.ip;
            _orfeoConfigs.append(c);
        }
    }

    _orfeoPanelTable->setRowCount(_orfeoConfigs.size());
    for (int row = 0; row < _orfeoConfigs.size(); ++row) {
        const PanelOrfeoConfig &c = _orfeoConfigs[row];

        auto nameItem = new QTableWidgetItem(c.panelName + " (" + c.panelIp + ")");
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        _orfeoPanelTable->setItem(row, 0, nameItem);

        QSpinBox *bankSpin = new QSpinBox;
        bankSpin->setRange(1, 10);
        bankSpin->setValue(c.bank);
        _orfeoPanelTable->setCellWidget(row, 1, bankSpin);

        auto mkCb = [](bool checked) {
            QCheckBox *cb = new QCheckBox;
            cb->setChecked(checked);
            return cb;
        };
        _orfeoPanelTable->setCellWidget(row, 2, centeredCheckBox(mkCb(c.showTitle)));
        _orfeoPanelTable->setCellWidget(row, 3, centeredCheckBox(mkCb(c.showDate)));
        _orfeoPanelTable->setCellWidget(row, 4, centeredCheckBox(mkCb(c.showTime)));
        _orfeoPanelTable->setCellWidget(row, 5, centeredCheckBox(mkCb(c.showSalle)));
        _orfeoPanelTable->setCellWidget(row, 6, centeredCheckBox(mkCb(c.showPersonnel)));

        QLineEdit *salleEdit = new QLineEdit(c.salleFilter);
        salleEdit->setPlaceholderText("(toutes)");
        _orfeoPanelTable->setCellWidget(row, 7, salleEdit);

        QSpinBox *intervalSpin = new QSpinBox;
        intervalSpin->setRange(1, 1440);
        intervalSpin->setValue(c.intervalMin);
        intervalSpin->setSuffix(" min");
        _orfeoPanelTable->setCellWidget(row, 8, intervalSpin);

        _orfeoPanelTable->setCellWidget(row, 9, centeredCheckBox(mkCb(c.active)));
    }

    panelVBox->addWidget(_orfeoPanelTable, 1);   // table fills group

    QHBoxLayout *panelBar = new QHBoxLayout;
    QPushButton *saveConfigBtn = new QPushButton("Enregistrer la configuration");
    saveConfigBtn->setFixedWidth(220);
    panelBar->addWidget(saveConfigBtn);
    panelBar->addStretch();
    panelVBox->addLayout(panelBar);              // button sits below table

    root->addWidget(panelGroup, 3);   // gets 3/5 of remaining space

    // ── Connections ───────────────────────────────────────────────────────────
    connect(testBtn,       &QPushButton::clicked, this, &MainWindow::on_orfeoTestApi_clicked);
    connect(saveApiBtn,    &QPushButton::clicked, this, &MainWindow::on_orfeoSaveApi_clicked);
    connect(refreshBtn,    &QPushButton::clicked, this, &MainWindow::on_orfeoRefresh_clicked);
    connect(saveConfigBtn, &QPushButton::clicked, this, &MainWindow::on_orfeoSaveConfig_clicked);
    connect(_orfeoAutoSyncCheck, &QCheckBox::toggled, this, &MainWindow::on_orfeoAutoSync_toggled);
}

void MainWindow::on_orfeoTestApi_clicked()
{
    _orfeoClient->setApiUrl(_orfeoApiUrlEdit->text().trimmed());
    _orfeoClient->setApiKey(_orfeoApiKeyEdit->text().trimmed());
    _orfeoStatusLabel->setText("Connexion en cours…");
    _orfeoStatusLabel->setStyleSheet("color: orange;");
    _orfeoClient->fetchEvents();
}

void MainWindow::on_orfeoSaveApi_clicked()
{
    OrfeoConfig::saveApiSettings(_orfeoApiUrlEdit->text().trimmed(),
                                 _orfeoApiKeyEdit->text().trimmed());
    _orfeoClient->setApiUrl(_orfeoApiUrlEdit->text().trimmed());
    _orfeoClient->setApiKey(_orfeoApiKeyEdit->text().trimmed());
    _orfeoStatusLabel->setText("Paramètres enregistrés.");
    _orfeoStatusLabel->setStyleSheet("color: green;");
}

void MainWindow::on_orfeoRefresh_clicked()
{
    _orfeoStatusLabel->setText("Actualisation…");
    _orfeoStatusLabel->setStyleSheet("color: orange;");
    _orfeoClient->fetchEvents();
}

void MainWindow::on_orfeoSaveConfig_clicked()
{
    readOrfeoPanelTable();
    OrfeoConfig::save(_orfeoConfigs);
    _orfeoStatusLabel->setText("Configuration panneaux enregistrée.");
    _orfeoStatusLabel->setStyleSheet("color: green;");
}

void MainWindow::on_orfeoAutoSync_toggled(bool checked)
{
    if (checked) {
        int ms = _orfeoIntervalSpin->value() * 60 * 1000;
        _orfeoTimer->start(ms);
        _orfeoStatusLabel->setText("Sync auto activée.");
        _orfeoStatusLabel->setStyleSheet("color: green;");
    } else {
        _orfeoTimer->stop();
        _orfeoStatusLabel->setText("Sync auto désactivée.");
        _orfeoStatusLabel->setStyleSheet("color: gray;");
    }
}

void MainWindow::orfeoEventsReceived(QList<OrfeoEvent> events)
{
    _orfeoEvents = events;
    populateOrfeoTable();
    _orfeoStatusLabel->setText(QString("%1 événement(s) reçu(s).").arg(events.size()));
    _orfeoStatusLabel->setStyleSheet("color: green;");

    if (_orfeoAutoSyncCheck && _orfeoAutoSyncCheck->isChecked())
        syncOrfeoToAllPanels();
}

void MainWindow::orfeoError(QString message)
{
    _orfeoStatusLabel->setText("Erreur : " + message);
    _orfeoStatusLabel->setStyleSheet("color: red;");
}

void MainWindow::orfeoAutoSyncTick()
{
    // Restart timer with current interval in case it changed
    _orfeoTimer->setInterval(_orfeoIntervalSpin->value() * 60 * 1000);
    _orfeoClient->fetchEvents();
}

void MainWindow::populateOrfeoTable()
{
    _orfeoEventsTable->setRowCount(_orfeoEvents.size());
    for (int i = 0; i < _orfeoEvents.size(); ++i) {
        const OrfeoEvent &e = _orfeoEvents[i];
        _orfeoEventsTable->setItem(i, 0, new QTableWidgetItem(e.date));
        _orfeoEventsTable->setItem(i, 1, new QTableWidgetItem(e.time));
        _orfeoEventsTable->setItem(i, 2, new QTableWidgetItem(e.title));
        _orfeoEventsTable->setItem(i, 3, new QTableWidgetItem(e.salle));
        _orfeoEventsTable->setItem(i, 4, new QTableWidgetItem(e.personnel));
    }
}

void MainWindow::readOrfeoPanelTable()
{
    for (int row = 0; row < _orfeoConfigs.size() && row < _orfeoPanelTable->rowCount(); ++row) {
        PanelOrfeoConfig &c = _orfeoConfigs[row];

        if (auto *spin = qobject_cast<QSpinBox*>(_orfeoPanelTable->cellWidget(row, 1)))
            c.bank = spin->value();

        auto cbAt = [&](int col) -> bool {
            QWidget *w = _orfeoPanelTable->cellWidget(row, col);
            if (!w) return false;
            QCheckBox *cb = w->findChild<QCheckBox*>();
            return cb ? cb->isChecked() : false;
        };
        c.showTitle     = cbAt(2);
        c.showDate      = cbAt(3);
        c.showTime      = cbAt(4);
        c.showSalle     = cbAt(5);
        c.showPersonnel = cbAt(6);

        if (auto *le = qobject_cast<QLineEdit*>(_orfeoPanelTable->cellWidget(row, 7)))
            c.salleFilter = le->text();

        if (auto *spin = qobject_cast<QSpinBox*>(_orfeoPanelTable->cellWidget(row, 8)))
            c.intervalMin = spin->value();

        c.active = cbAt(9);
    }
}

QString MainWindow::buildOrfeoMessage(const PanelOrfeoConfig &cfg, const QList<OrfeoEvent> &events)
{
    QStringList lines;
    for (const OrfeoEvent &e : events) {
        if (!cfg.salleFilter.isEmpty() &&
            !e.salle.contains(cfg.salleFilter, Qt::CaseInsensitive))
            continue;

        QStringList parts;
        if (cfg.showDate)      parts << e.date;
        if (cfg.showTime)      parts << e.time;
        if (cfg.showTitle)     parts << e.title;
        if (cfg.showSalle)     parts << e.salle;
        if (cfg.showPersonnel) parts << e.personnel;

        if (!parts.isEmpty())
            lines << parts.join("  ");
    }
    if (lines.isEmpty()) return "";

    QString bank = (cfg.bank < 10 ? QString("MSG0") : QString("MSG")) + QString::number(cfg.bank);
    return bank + "   " + lines.join(" | ") + "   ";
}

void MainWindow::syncOrfeoToAllPanels()
{
    readOrfeoPanelTable();
    for (const PanelOrfeoConfig &cfg : _orfeoConfigs) {
        if (!cfg.active) continue;
        QString msg = buildOrfeoMessage(cfg, _orfeoEvents);
        if (msg.isEmpty()) continue;

        ledBar *bar = new ledBar(this);
        bar->connection(cfg.panelIp, 23);
        if (bar->isConnected()) {
            bar->sendMessage(msg);
            bar->closeConnection();
        }
        bar->deleteLater();
    }
}
