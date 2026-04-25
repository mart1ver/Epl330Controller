#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <udpclient.h>
#include <ledbar.h>
#include "panelconfig.h"
#include "orfeoconfig.h"
#include "orfeoevent.h"

class OrfeoClient;
class QTimer;
class QTableWidget;
class QLineEdit;
class QLabel;
class QCheckBox;
class QSpinBox;

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
    QString getZone();
    QString getBlink();
    QString getRepeatStart();
    QString getRepeatEnd();
    void updateCurrentMsgNum();

    QString composeMessage();
    QString buildScheduleStr();

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
    void on_addSchedule_clicked();
    void on_delSchedule_clicked();
    void on_zoneSet_clicked();

    void on_saveIP_clicked();

    void on_authorizeAll_2_clicked();
    void on_managePanels_clicked();

    void on_orfeoTestApi_clicked();
    void on_orfeoSaveApi_clicked();
    void on_orfeoRefresh_clicked();
    void on_orfeoSaveConfig_clicked();
    void on_orfeoAutoSync_toggled(bool checked);
    void orfeoEventsReceived(QList<OrfeoEvent> events);
    void orfeoError(QString message);
    void orfeoAutoSyncTick();

private:
    void loadPanels();
    void setupOrfeoTab();
    void populateOrfeoTable();
    void readOrfeoPanelTable();
    void syncOrfeoToAllPanels();
    QString buildOrfeoMessage(const PanelOrfeoConfig &cfg, const QList<OrfeoEvent> &events);

    Ui::MainWindow  *ui;
    UDPClient       *udpReceiver;
    ledBar          *epl;
    QList<Panel>     _panels;

    OrfeoClient             *_orfeoClient;
    QTimer                  *_orfeoTimer;
    QList<PanelOrfeoConfig>  _orfeoConfigs;
    QList<OrfeoEvent>        _orfeoEvents;

    QLineEdit    *_orfeoApiUrlEdit;
    QLineEdit    *_orfeoApiKeyEdit;
    QLabel       *_orfeoStatusLabel;
    QTableWidget *_orfeoEventsTable;
    QTableWidget *_orfeoPanelTable;
    QCheckBox    *_orfeoAutoSyncCheck;
    QSpinBox     *_orfeoIntervalSpin;
};

#endif // MAINWINDOW_H
