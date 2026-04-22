#include "paneldialog.h"
#include "networkscanner.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QInputDialog>

PanelDialog::PanelDialog(const QList<Panel> &panels, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Gestion des panneaux");
    setMinimumSize(480, 320);

    _table = new QTableWidget(0, 2, this);
    _table->setHorizontalHeaderLabels({"Nom", "Adresse IP"});
    _table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    _table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);

    for (const Panel &p : panels) {
        int row = _table->rowCount();
        _table->insertRow(row);
        _table->setItem(row, 0, new QTableWidgetItem(p.name));
        _table->setItem(row, 1, new QTableWidgetItem(p.ip));
    }

    _scanBtn          = new QPushButton("Scan réseau",  this);
    QPushButton *addBtn    = new QPushButton("Ajouter",      this);
    QPushButton *removeBtn = new QPushButton("Supprimer",    this);
    QPushButton *okBtn     = new QPushButton("OK",           this);
    QPushButton *cancelBtn = new QPushButton("Annuler",      this);

    _statusLabel = new QLabel(this);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(_scanBtn);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_table);
    layout->addWidget(_statusLabel);
    layout->addLayout(btnLayout);

    _scanner = new NetworkScanner(this);

    connect(_scanBtn,  &QPushButton::clicked, this, &PanelDialog::startScan);
    connect(addBtn,    &QPushButton::clicked, this, &PanelDialog::addPanel);
    connect(removeBtn, &QPushButton::clicked, this, &PanelDialog::removePanel);
    connect(okBtn,     &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    connect(_scanner, &NetworkScanner::deviceFound, this, &PanelDialog::onDeviceFound);
    connect(_scanner, &NetworkScanner::progress,    this, &PanelDialog::onScanProgress);
    connect(_scanner, &NetworkScanner::finished,    this, &PanelDialog::onScanFinished);
}

QList<Panel> PanelDialog::panels() const
{
    QList<Panel> list;
    for (int i = 0; i < _table->rowCount(); ++i) {
        Panel p;
        p.name = _table->item(i, 0) ? _table->item(i, 0)->text() : "";
        p.ip   = _table->item(i, 1) ? _table->item(i, 1)->text() : "";
        if (!p.ip.isEmpty())
            list.append(p);
    }
    return list;
}

bool PanelDialog::hasIP(const QString &ip) const
{
    for (int i = 0; i < _table->rowCount(); ++i)
        if (_table->item(i, 1) && _table->item(i, 1)->text() == ip)
            return true;
    return false;
}

void PanelDialog::addPanel()
{
    bool ok;
    QString ip = QInputDialog::getText(this, "Ajouter", "Adresse IP :", QLineEdit::Normal, "", &ok);
    if (!ok || ip.isEmpty()) return;
    int row = _table->rowCount();
    _table->insertRow(row);
    _table->setItem(row, 0, new QTableWidgetItem(ip));
    _table->setItem(row, 1, new QTableWidgetItem(ip));
}

void PanelDialog::removePanel()
{
    int row = _table->currentRow();
    if (row >= 0)
        _table->removeRow(row);
}

void PanelDialog::startScan()
{
    _scanBtn->setEnabled(false);
    _statusLabel->setText("Scan en cours...");
    _scanner->scan(NetworkScanner::detectSubnet(), 23, 500);
}

void PanelDialog::onDeviceFound(const QString &ip)
{
    if (hasIP(ip)) return;
    int row = _table->rowCount();
    _table->insertRow(row);
    _table->setItem(row, 0, new QTableWidgetItem(ip));
    _table->setItem(row, 1, new QTableWidgetItem(ip));
}

void PanelDialog::onScanProgress(int done, int total)
{
    _statusLabel->setText(QString("Scan : %1 / %2").arg(done).arg(total));
}

void PanelDialog::onScanFinished()
{
    _scanBtn->setEnabled(true);
    _statusLabel->setText(QString("Scan terminé — %1 panneau(x) trouvé(s).").arg(_table->rowCount()));
}
