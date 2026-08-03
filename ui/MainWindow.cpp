#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QtCharts>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>
#include <QCoreApplication>
#include "ui/SettingsDialog.h"
#include "core/FileLogger.h"

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
    if (m_workerThread && m_workerThread->isRunning()) {
        // 在 worker 线程内执行 stop()，避免跨线程直接调用
        QMetaObject::invokeMethod(m_worker, &CommWorker::stop,
                                  Qt::BlockingQueuedConnection);
    }
    if (m_workerThread) {
        m_workerThread->quit();   // 线程安全，直接请求事件循环退出
        m_workerThread->wait(3000);
    }
    if (m_dbThread) {
        m_dbThread->quit();
        m_dbThread->wait(3000);
    }
    delete m_logger;
    delete ui;
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

        connect(dialog, &SettingsDialog::requestSerialConnect, this, [this, dialog](const QString &port, qint32 baud) {
            if (m_isConnecting) return;
            m_isConnecting = true;
            
            // 断开旧连接
            if (m_serialConn) {
                disconnect(m_serialConn);
                m_serialConn = QMetaObject::Connection();
            }
            
            // 建立新连接
            m_serialConn = connect(m_worker, &CommWorker::portOpened, this, [this, dialog](bool ok) {
                m_isConnecting = false;
                if (ok) {
                    dialog->setConnected(true);
                    ui->statusbar->showMessage("串口连接成功", 3000);
                } else {
                    QMessageBox::warning(dialog, "连接失败", "无法打开串口，请检查端口号");
                }
            });
            
            QMetaObject::invokeMethod(m_worker, [this, port, baud]() {
                m_worker->openPort(port, baud);
            });
        });

        connect(dialog, &SettingsDialog::requestDisconnect, this, [this, dialog]() {
            dialog->setConnected(false);
            QMetaObject::invokeMethod(m_worker, [this]() {
                m_worker->closePort();
            });
        });

        dialog->show();
        connect(dialog, &SettingsDialog::requestTcpConnect, this, [this, dialog](const QString &host, quint16 port) {
            if (m_isConnecting) return;
            m_isConnecting = true;

            // 1. 先断开旧连接（不调用 stop，避免线程退出）
            QMetaObject::invokeMethod(m_worker, [this]() {
                m_worker->closePort();
            });

            // 2. 设置新模式（queued 到 worker 线程，保证在 connectToHost 之前执行）
            QMetaObject::invokeMethod(m_worker, [this]() {
                m_worker->setMode(CommWorker::Tcp);
            });

            // 3. 断开旧连接
            if (m_tcpConnOk) {
                disconnect(m_tcpConnOk);
                m_tcpConnOk = QMetaObject::Connection();
            }
            if (m_tcpConnFail) {
                disconnect(m_tcpConnFail);
                m_tcpConnFail = QMetaObject::Connection();
            }
            
            // 4. 建立新连接
            m_tcpConnOk = connect(m_worker, &CommWorker::tcpConnected, this, [this, dialog]() {
                m_isConnecting = false;
                dialog->setConnected(true);
                ui->statusbar->showMessage("TCP 连接成功", 3000);
            });

            m_tcpConnFail = connect(m_worker, &CommWorker::tcpConnectFailed, this, [this, dialog](const QString &msg) {
                m_isConnecting = false;
                dialog->setConnected(false);
                // 失败提示走状态栏，避免自动重试时 3 秒一次的弹窗风暴
                ui->statusbar->showMessage("TCP 连接失败: " + msg, 5000);
            });

            // 4. 启动线程（如果未运行）并连接
            if (!m_workerThread->isRunning()) {
                connect(m_workerThread, &QThread::started, this, [this, host, port]() {
                    QMetaObject::invokeMethod(m_worker, [this, host, port]() {
                        m_worker->connectToHost(host, port);
                    });
                }, Qt::SingleShotConnection);
                m_workerThread->start();
            } else {
                QMetaObject::invokeMethod(m_worker, [this, host, port]() {
                    m_worker->connectToHost(host, port);
                });
            }
        });
    });
    connect(ui->actionExportData,&QAction::triggered,this,[this](){
        if (m_seriesMap.isEmpty()) {
            QMessageBox::information(this,"提示","没有数据可导出");
            return;
        }
        QString filePath = QFileDialog::getSaveFileName(this,"导出数据","","CSV 文件 (*.csv)");
        if (filePath.isEmpty()) return;

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this,"错误","无法创建文件");
            return;
        }

        QTextStream out(&file);
        out << "地址,序号,数值\n";
        for (auto it2 = m_seriesMap.begin(); it2 != m_seriesMap.end(); ++it2) {
            auto pts = it2.value()->points();
            for (int i = 0; i < pts.size(); ++i) {
                out << "0x" << QString::number(it2.key(), 16) << ","
                    << i + 1 << "," << pts[i].y() << "\n";
            }
        }
        file.close();
        QMessageBox::information(this,"完成","导出成功: " + filePath);
    });
}

void MainWindow::initWorker()
{
    // 通信线程
    m_workerThread = new QThread(this);
    m_worker = new CommWorker();
    m_worker->moveToThread(m_workerThread);

    // 信号流向：CommWorker::frameReceived -> MainWindow (UI 更新)
    connect(m_worker, &CommWorker::frameReceived, this, [this](const ParsedFrame &frame) {
        auto it = m_seriesMap.find(frame.address);
        if (it == m_seriesMap.end()) {
            qWarning() << "未知地址:" << frame.address;
            return;
        }
        it.value()->append(++m_pointCount, frame.value);

        // 裁剪窗口外旧点，避免长时间运行内存持续增长
        if (it.value()->count() > kMaxSeriesPoints) {
            it.value()->removePoints(0, it.value()->count() - kMaxSeriesPoints);
        }

        ui->listWidget->insertItem(0,
            QStringLiteral("%1 | 0x%2 | %3 | %4")
                .arg(m_pointCount)
                .arg(frame.address, 2, 16, QChar('0'))
                .arg(frame.value)
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz")));

        while (ui->listWidget->count() > 100) {
            delete ui->listWidget->takeItem(ui->listWidget->count() - 1);
        }

        if (frame.value > kAlarmThreshold) {
            ui->listWidget->item(0)->setForeground(Qt::red);
            ui->statusbar->showMessage(
                QStringLiteral("警报: 通道 0x%1 数值 %2 超过阈值 %3")
                    .arg(frame.address, 2, 16, QChar('0'))
                    .arg(frame.value)
                    .arg(kAlarmThreshold), 3000);
        }

        // Y 轴即时上调
        auto yAxes = m_chart->axes(Qt::Vertical);
        if (!yAxes.isEmpty()) {
            auto *yAxis = qobject_cast<QValueAxis *>(yAxes.first());
            if (yAxis && frame.value > yAxis->max()) {
                yAxis->setRange(0, frame.value * 1.2);
            }
        }

        // 每 50 帧按可视窗口内最大值回落 Y 轴
        if (m_pointCount % 50 == 0) {
            if (!yAxes.isEmpty()) {
                auto *yAxis = qobject_cast<QValueAxis *>(yAxes.first());
                if (yAxis) {
                    double windowStart = m_pointCount > kMaxSeriesPoints
                                             ? m_pointCount - kMaxSeriesPoints
                                             : 0;
                    double visibleMax = 0.0;
                    for (auto *s : m_seriesMap) {
                        const auto pts = s->points();
                        for (int i = 0; i < pts.size(); ++i) {
                            if (pts[i].x() >= windowStart && pts[i].y() > visibleMax) {
                                visibleMax = pts[i].y();
                            }
                        }
                    }
                    double newMax = qMax(visibleMax * 1.2, 10.0);
                    if (newMax < yAxis->max()) {
                        yAxis->setRange(0, newMax);
                    }
                }
            }
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
    // 线程结束后在各自线程内销毁，QSqlDatabase 连接归属线程安全
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_workerThread->start();

    // 数据库线程
    m_dbThread = new QThread(this);
    m_dbWriter = new DatabaseWriter(QCoreApplication::applicationDirPath() + "/data.db");
    m_dbWriter->moveToThread(m_dbThread);

    // 数据库在 moveToThread 之后于数据库线程内打开（QSqlDatabase 绑定创建它的线程）
    connect(m_dbThread, &QThread::started, m_dbWriter, &DatabaseWriter::init);

    // 信号流向：CommWorker::frameReceived -> DatabaseWriter (数据库写入)
    connect(m_worker, &CommWorker::frameReceived, m_dbWriter, &DatabaseWriter::saveRecord);
    connect(m_dbWriter, &DatabaseWriter::errorOccurred, this, [this](const QString &msg) {
        ui->statusbar->showMessage(msg, 5000);
    });
    connect(m_dbThread, &QThread::finished, m_dbWriter, &QObject::deleteLater);
    m_dbThread->start();

    m_logger = new FileLogger(QCoreApplication::applicationDirPath() + "/data_monitor.log");
    // 信号流向：CommWorker::frameReceived -> FileLogger (日志写入)
    connect(m_worker, &CommWorker::frameReceived, m_logger, &FileLogger::write);

    // Tab 切换时加载历史
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 1) loadHistory();
    });

    // 初始化时加载一次历史数据（显示表头）
    loadHistory();
}

void MainWindow::initChart()
{
    m_chart = new QChart();
    m_chart->setTitle("实时数据曲线");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    uint8_t addrs[] = {0x01, 0x02, 0x03, 0x04};

    for (int i = 0; i < 4; ++i) {
        auto *series = new QLineSeries();
        series->setName(QString("通道 0x%1").arg(addrs[i], 2, 16, QChar(0x30)));
        m_chart->addSeries(series);
        m_seriesMap[addrs[i]] = series;
    }

    auto *axisX = new QValueAxis();
    axisX->setTitleText("采样点");
    axisX->setRange(0, 100);
    m_chart->addAxis(axisX, Qt::AlignBottom);

    auto *axisY = new QValueAxis();
    axisY->setTitleText("数值");
    axisY->setRange(0, 1000);
    m_chart->addAxis(axisY, Qt::AlignLeft);

    for (auto *s : m_seriesMap) {
        s->attachAxis(axisX);
        s->attachAxis(axisY);
    }

    auto *chartView = new QChartView(m_chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    auto *tab1 = ui->tabWidget->widget(0);
    auto *layout = new QVBoxLayout(tab1);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);
}
void MainWindow::loadHistory()
{
    QSqlDatabase readDb = QSqlDatabase::addDatabase("QSQLITE", "read_connection");
    readDb.setDatabaseName(QCoreApplication::applicationDirPath() + "/data.db");
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

    ui->historyTable->setModel(nullptr);

    auto *model = new QStandardItemModel(records.size(), 3, this);
    model->setHorizontalHeaderLabels({"地址", "序号", "数值"});
    for (int i = 0; i < records.size(); ++i) {
        model->setItem(i, 0, new QStandardItem("0x" + QString::number(records[i].address, 16)));
        model->setItem(i, 1, new QStandardItem(QString::number(i + 1)));
        model->setItem(i, 2, new QStandardItem(QString::number(records[i].value)));
    }
    ui->historyTable->setModel(model);
    ui->historyTable->horizontalHeader()->setStretchLastSection(true);
}
