#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QtCharts>
#include <QDateTime>
#include "ui/SettingsDialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initMenuBar();
    initWorker();
    initChart();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (m_workerThread && m_workerThread->isRunning()) {
        m_worker->stop();
        m_workerThread->wait(3000);
    }
}

void MainWindow::initMenuBar()
{
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
        QMessageBox::information(this,"提示","导出功能将在v4版本中实现");
    });
}

void MainWindow::initWorker()
{
    m_workerThread = new QThread(this);
    m_worker = new CommWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_worker, &CommWorker::frameReceived, this, [this](const ParsedFrame &frame) {
        if (!m_series) return;
        m_series->append(++m_pointCount, frame.value);

        // 插入表格行
        QList<QStandardItem *> row;
        row << new QStandardItem(QString::number(m_pointCount))
            << new QStandardItem("0x" + QString::number(frame.address, 16))
            << new QStandardItem(QString::number(frame.value))
            << new QStandardItem(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"));
        m_tableModel->insertRow(0, row);

        // 最多保留100行
        while (m_tableModel->rowCount() > 100) {
            m_tableModel->removeRow(m_tableModel->rowCount() - 1);
        }

        if (m_pointCount > 200) {
            auto xAxes = m_chart->axes(Qt::Horizontal);
            if (!xAxes.isEmpty()) {
                auto *xAxis = qobject_cast<QValueAxis *>(xAxes.first());
                if (xAxis) xAxis->setRange(m_pointCount - 200, m_pointCount);
            }
        }
    });

    connect(m_worker, &CommWorker::errorOccurred, this, [this](const QString &msg) {
        ui->statusbar->showMessage(msg, 5000);
    });

    connect(m_workerThread, &QThread::started, m_worker, &CommWorker::start);
    connect(m_worker, &CommWorker::finished, m_workerThread, &QThread::quit);

    m_workerThread->start();
}

void MainWindow::initChart()
{
    m_series = new QLineSeries();
    m_series->setName("通道1");

    m_chart = new QChart();
    m_chart->addSeries(m_series);
    m_chart->setTitle("实时数据曲线");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    auto *axisX = new QValueAxis();
    axisX->setTitleText("采样点");
    axisX->setRange(0, 100);
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_series->attachAxis(axisX);

    auto *axisY = new QValueAxis();
    axisY->setTitleText("数值");
    axisY->setRange(0, 1000);
    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_series->attachAxis(axisY);

    auto *chartView = new QChartView(m_chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    auto *tab1 = ui->tabWidget->widget(0);
    auto *layout = new QVBoxLayout(tab1);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);
}

void MainWindow::initTable()
{
    m_tableModel = new QStandardItemModel(0, 4, this);
    m_tableModel->setHorizontalHeaderLabels({"序号", "地址", "数值", "时间"});
    ui->tableView->setModel(m_tableModel);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
}