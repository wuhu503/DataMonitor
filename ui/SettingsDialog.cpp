#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->comboBaud->addItems({
        "9600", "19200", "38400", "57600", "115200", "230400"
    });
    ui->comboBaud->setCurrentText("115200");

    refreshPortList();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::refreshPortList()
{
    ui->comboPort->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto &port : ports) {
        ui->comboPort->addItem(port.portName());
    }
}

void SettingsDialog::on_btnRefresh_clicked()
{
    refreshPortList();
}

void SettingsDialog::on_btnConnect_clicked()
{
    if (!m_connected) {
        QString portName = ui->comboPort->currentText();
        qint32 baudRate = ui->comboBaud->currentText().toInt();
        emit requestConnect(portName, baudRate);
        ui->btnConnect->setText("断开串口");
        m_connected = true;
    } else {
        emit requestDisconnect();
        ui->btnConnect->setText("连接串口");
        m_connected = false;
    }
}