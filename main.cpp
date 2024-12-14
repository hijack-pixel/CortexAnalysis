#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QQmlApplicationEngine>
#include <QDateTime>
#pragma execution_character_set("utf-8")


QFile debugFile("log.txt");

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg){
    QTextStream out(&debugFile);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << msg << "\n";
};


int main(int argc, char *argv[])
{
    //设置输出信息的格式
    QString pattern="[%{file}] [%{function}] [%{line}] %{message}";
    qSetMessagePattern(pattern);

    debugFile.open(QIODevice::WriteOnly | QIODevice::Text);

    // 在程序开始时设置输出重定向
    #ifdef REDIRECT_DEBUG_OUTPUT
        qInstallMessageHandler(customMessageHandler);
    #endif


    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    // 加载 qss
    QFile styleFile(":/qss/main.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        a.setStyleSheet(styleSheet);
    }

    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:/qml/breathinglight.qml");

    // 主窗口运行
    MainWindow w;
    w.setWindowIcon(QIcon(":/icon/logo.svg"));
    w.setWindowTitle(QString("大脑皮层成像分析软件 V0.1"));
    w.show();


    return a.exec();
}
