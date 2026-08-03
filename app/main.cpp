#include "ui/MainWindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 加载 QSS 样式（通过 .qrc 内嵌，不依赖工作目录）
    QFile file(":/styles/style.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        a.setStyleSheet(file.readAll());
        file.close();
    }

    MainWindow w;
    w.show();
    return QApplication::exec();
}
