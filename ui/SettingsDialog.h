#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSerialPortInfo>
#include <QSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;
    void setConnected(bool connected);

signals:
    void requestSerialConnect(const QString &portName, qint32 baudRate);
    void requestTcpConnect(const QString &host, quint16 port);
    void requestDisconnect();

private slots:
    void on_btnRefresh_clicked();
    void on_btnConnect_clicked();
    void on_tabWidget_currentChanged(int index);

private:
    void refreshPortList();

    Ui::SettingsDialog *ui;
    bool m_connected = false;
    bool m_isTcpMode = false;
};

#endif // SETTINGSDIALOG_H
