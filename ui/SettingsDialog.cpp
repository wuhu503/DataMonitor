#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->comboBaud->addItems({"9600", "19200", "38400", "57600", "115200", "230400"});
    ui->comboBaud->setCurrentText("115200");
    ui->spinBoxPort->setValue(502);

    refreshPortList();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setConnected(bool connected)
{
    m_connected = connected;
    ui->btnConnect->setText(connected ? (m_isTcpMode ? "断开TCP" : "断开串口")
                                      : (m_isTcpMode ? "连接TCP" : "连接串口"));
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

void SettingsDialog::on_tabWidget_currentChanged(int index)
{
    m_isTcpMode = (index == 1);
    if (!m_connected) {
        ui->btnConnect->setText(m_isTcpMode ? "连接TCP" : "连接串口");
    } else {
        ui->btnConnect->setText(m_isTcpMode ? "断开TCP" : "断开串口");
    }
}

void SettingsDialog::on_btnConnect_clicked()
{
    if (m_connected) {
        emit requestDisconnect();
        return;
    }

    if (m_isTcpMode) {
        QString host = ui->lineEditHost->text().trimmed();
        if (host.isEmpty()) return;
        emit requestTcpConnect(host, static_cast<quint16>(ui->spinBoxPort->value()));
    } else {
        emit requestSerialConnect(ui->comboPort->currentText(),
                                   ui->comboBaud->currentText().toInt());
    }
}
