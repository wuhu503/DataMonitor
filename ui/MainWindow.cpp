#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QtCharts>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include "ui/SettingsDialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initMenuBar();
    initWorker();
    initChart();
    initTable();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (m_workerThread && m_workerThread->isRunning()) {
        m_worker->stop();
        m_workerThread->wait(3000);
    }
    if (m_dbThread && m_dbThread->isRunning()) {
        m_dbThread->quit();
        m_dbThread->wait(3000);
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
    // 通信线程
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

    // 数据库线程
    m_dbThread = new QThread(this);
    m_dbWriter = new DatabaseWriter("data.db");
    m_dbWriter->moveToThread(m_dbThread);

    connect(m_worker, &CommWorker::frameReceived, m_dbWriter, &DatabaseWriter::saveRecord);
    connect(m_dbWriter, &DatabaseWriter::errorOccurred, this, [this](const QString &msg) {
        ui->statusbar->showMessage(msg, 5000);
    });
    m_dbThread->start();

    // Tab 切换时加载历史
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 1) loadHistory();
    });
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

void MainWindow::loadHistory()
{
    QSqlDatabase readDb = QSqlDatabase::addDatabase("QSQLITE", "read_connection");
    readDb.setDatabaseName("data.db");
    if (!readDb.open()) return;

    QSqlQuery query(readDb);
    query.exec("SELECT address, value FROM data_records ORDER BY id DESC LIMIT 100");

    QList<ParsedFrame> records;
    while (query.next()) {
        ParsedFrame f;
        f.address = query.value(0).toUInt();
        f.value = query.value(1).toUInt();
        records.append(f);
    }
    readDb.close();
    QSqlDatabase::removeDatabase("read_connection");

    auto *model = new QStandardItemModel(records.size(), 3, this);
    model->setHorizontalHeaderLabels({"地址", "数值", "序号"});
    for (int i = 0; i < records.size(); ++i) {
        model->setItem(i, 0, new QStandardItem("0x" + QString::number(records[i].address, 16)));
        model->setItem(i, 1, new QStandardItem(QString::number(records[i].value)));
        model->setItem(i, 2, new QStandardItem(QString::number(i + 1)));
    }
    ui->historyTable->setModel(model);
    ui->historyTable->horizontalHeader()->setStretchLastSection(true);
}
