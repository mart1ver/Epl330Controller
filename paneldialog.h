#ifndef PANELDIALOG_H
#define PANELDIALOG_H

#include <QDialog>
#include "panelconfig.h"

class QTableWidget;
class QLabel;
class QPushButton;
class NetworkScanner;

class PanelDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PanelDialog(const QList<Panel> &panels, QWidget *parent = nullptr);
    QList<Panel> panels() const;

private slots:
    void addPanel();
    void removePanel();
    void startScan();
    void onDeviceFound(const QString &ip);
    void onScanProgress(int done, int total);
    void onScanFinished();

private:
    bool hasIP(const QString &ip) const;

    QTableWidget   *_table;
    QPushButton    *_scanBtn;
    QLabel         *_statusLabel;
    NetworkScanner *_scanner;
};

#endif
