#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "paneldialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

    loadPanels();
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

