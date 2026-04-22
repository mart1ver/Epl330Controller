#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <udpclient.h>
#include <ledbar.h>
#include "panelconfig.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString getFont();
    QString getInAnim();
    QString getOutAnim();
    QString getSpeed();
    QString getTimeStop();
    QString getEffect();
    QString getMsgBank();
    void updateCurrentMsgNum();

    QString composeMessage();

private slots:

    void on_display_clicked();
    void on_save_clicked();
    void on_connect_clicked();
    void on_freezeMode_clicked();
    void on_runMode_clicked();
    void on_clearDevice_clicked();
    void on_rebootDevice_clicked();
    void on_ledIntensity_sliderMoved(int position);
    void on_checkBox_stateChanged(int arg1);
    void udpDatagramReceived(QString datagram);
    void eplConnected();
    void eplDisconnected();
    void on_clrBank_clicked();

    void on_authorizeAll_clicked();

    void on_blank_clicked();

    void on_clearSchedule_clicked();

    void on_saveIP_clicked();

    void on_authorizeAll_2_clicked();
    void on_managePanels_clicked();

private:
    void loadPanels();

    Ui::MainWindow  *ui;
    UDPClient       *udpReceiver;
    ledBar          *epl;
    QList<Panel>     _panels;
};

#endif // MAINWINDOW_H
