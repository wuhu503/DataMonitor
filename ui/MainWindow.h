#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMap>

#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>

#include "thread/CommWorker.h"
#include "thread/DatabaseWriter.h"
#include "core/FileLogger.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;

    void initMenuBar();
    void initWorker();
    void initChart();
    void loadHistory();

    QMap<uint8_t, QLineSeries *> m_seriesMap;
    QChart *m_chart = nullptr;
    int m_pointCount = 0;
    static constexpr int kMaxSeriesPoints = 200;
    int m_alarmThreshold = 800;

    QThread *m_workerThread = nullptr;
    CommWorker *m_worker = nullptr;

    QThread *m_dbThread = nullptr;
    DatabaseWriter *m_dbWriter = nullptr;
    FileLogger *m_logger = nullptr;
    bool m_isConnecting = false;
    
    // 信号连接管理
    QMetaObject::Connection m_serialConn;
    QMetaObject::Connection m_tcpConnOk;
    QMetaObject::Connection m_tcpConnFail;
};
#endif // MAINWINDOW_H
