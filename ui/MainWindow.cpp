#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

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
    connect(ui->actionOpenConfig,&QAction::triggered,this,[this](){
        QMessageBox::information(this,"提示","配置功能将在v2版本中实现!");
    });
    connect(ui->actionExportData,&QAction::triggered,this,[this](){
        QMessageBox::information(this,"提示","导出功能将在v4版本中实现!");
    });
}