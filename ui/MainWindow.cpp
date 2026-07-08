#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "ui/SettingsDialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initMenuBar();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (m_workerThread->isRunning()) {
        m_worker->stop();
        m_workerThread->wait(3000);
    }
}

void MainWindow::initMenuBar()
{
    // 创建通信线程
    m_workerThread = new QThread(this);
    m_worker = new CommWorker();
    m_worker->moveToThread(m_workerThread);

    connect(ui->actionExit,&QAction::triggered,qApp, &QApplication::quit);
    connect(ui->actionAbout,&QAction::triggered,this,[this](){
        QMessageBox::about(this,"关于 DataMonitor","工业数据采集与监控系统 v1.0");
    });
    connect(ui->actionToggleLeftDock,&QAction::triggered,this,[this](){
        ui->dockChannelPanel->toggleViewAction()->trigger();
    });
    connect(ui->actionToggleRightDock,&QAction::triggered,this,[this](){
        ui->dockDataPanel->toggleViewAction()->trigger();
    });
    connect(ui->actionOpenConfig, &QAction::triggered, this, [this]() {
        SettingsDialog *dialog = new SettingsDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        connect(dialog, &SettingsDialog::requestConnect, this, [this](const QString &port, qint32 baud) {
            QMetaObject::invokeMethod(m_worker, [this, port, baud]() {
                m_worker->openPort(port, baud);
            });
        });

        connect(dialog, &SettingsDialog::requestDisconnect, this, [this]() {
            QMetaObject::invokeMethod(m_worker, [this]() {
                m_worker->closePort();
            });
        });

        dialog->show();
    });
    connect(ui->actionExportData,&QAction::triggered,this,[this](){
        QMessageBox::information(this,"提示","导出功能将在v4版本中实现!");
    });

    // 接收跨线程信号
    connect(m_worker, &CommWorker::frameReceived, this, [this](const ParsedFrame &frame) {
        // 收到一帧数据，后续这里更新 UI
    });

    connect(m_worker, &CommWorker::errorOccurred, this, [this](const QString &msg) {
        // 串口错误时显示到状态栏
    });

    connect(m_workerThread, &QThread::started, m_worker, &CommWorker::start);
    connect(m_worker, &CommWorker::finished, m_workerThread, &QThread::quit);

    m_workerThread->start();
}