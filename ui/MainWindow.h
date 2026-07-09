#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QStandardItemModel>

#include "thread/CommWorker.h"

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
    void initTable();

    QLineSeries *m_series = nullptr;
    QChart *m_chart = nullptr;
    int m_pointCount = 0;
    QStandardItemModel *m_tableModel = nullptr;

    QThread *m_workerThread = nullptr;
    CommWorker *m_worker = nullptr;
};
#endif // MAINWINDOW_H
