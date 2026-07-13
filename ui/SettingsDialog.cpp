#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QMessageBox>

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
    // 保存当前选择
    QString currentPort = ui->comboPort->currentText();
    
    ui->comboPort->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto &port : ports) {
        ui->comboPort->addItem(port.portName());
    }
    if (ports.isEmpty()) {
        ui->comboPort->addItem("未检测到串口");
    }
    
    // 恢复之前的选择（如果还存在）
    int index = ui->comboPort->findText(currentPort);
    if (index >= 0) {
        ui->comboPort->setCurrentIndex(index);
    }
}

void SettingsDialog::on_btnRefresh_clicked()
{
    refreshPortList();
}

void SettingsDialog::on_tabWidget_currentChanged(int index)
{
    m_isTcpMode = (index == 1);
    ui->btnRefresh->setEnabled(!m_isTcpMode);
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
        QString portName = ui->comboPort->currentText();
        if (portName.isEmpty() || portName == "未检测到串口") {
            QMessageBox::warning(this, "提示", "请先选择有效的串口");
            return;
        }
        emit requestSerialConnect(portName, ui->comboBaud->currentText().toInt());
    }
}
