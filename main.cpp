#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#pragma execution_character_set("utf-8")


int main(int argc, char *argv[])
{
    //设置输出信息的格式
    QString pattern="[%{file}] [%{function}] [%{line}] %{message}";
    qSetMessagePattern(pattern);

    QApplication a(argc, argv);

    // 加载 qss
    QFile styleFile(":/qss/main.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        a.setStyleSheet(styleSheet);
    }

    // 主窗口运行
    MainWindow w;
    w.setWindowIcon(QIcon(":/icon/logo.svg"));
    w.setWindowTitle(QString("大脑皮层成像分析软件"));
    w.show();
    return a.exec();
}
