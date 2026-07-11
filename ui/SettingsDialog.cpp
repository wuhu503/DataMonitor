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

void SettingsDialog::setConnected(bool connected)
{
    m_connected = connected;
    ui->btnConnect->setText(connected ? "断开串口" : "连接串口");
}

void SettingsDialog::on_btnRefresh_clicked()
{
    refreshPortList();
}

void SettingsDialog::on_btnConnect_clicked()
{
    if (!m_connected) {
        emit requestConnect(ui->comboPort->currentText(), ui->comboBaud->currentText().toInt());
    } else {
        emit requestDisconnect();
        setConnected(false);
    }
}
