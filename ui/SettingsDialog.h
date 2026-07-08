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

signals:
    void requestConnect(const QString &portName, qint32 baudRate);
    void requestDisconnect();

private slots:
    void on_btnRefresh_clicked();
    void on_btnConnect_clicked();

private:
    void refreshPortList();

    Ui::SettingsDialog *ui;
    bool m_connected = false;
};

#endif // SETTINGSDIALOG_H