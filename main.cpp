#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QQmlApplicationEngine>
#include <QDateTime>
#pragma execution_character_set("utf-8")

QFile debugFile("log.txt");

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg){
    QTextStream out(&debugFile);

    // 从上下文中提取文件名、函数名和行号
    QString fileName = context.file ? context.file : "unknown file";
    QString functionName = context.function ? context.function : "unknown function";
    int lineNumber = context.line ? context.line : -1;

    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz ")
        << "[" << fileName << ":" << functionName << ":" << lineNumber << "] "
        << msg << '\n';

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
    w.setWindowTitle(QString("大脑皮层成像分析软件 V0.5"));
    w.show();

    // xlnt::workbook wb;
    // xlnt::worksheet ws = wb.active_sheet();
    // ws.cell("A1").value(5); // 写入数值
    // ws.cell("B2").value("string data"); // 写入字符串
    // ws.cell("C3").formula("RAND()"); // 写入公式
    // ws.merge_cells("C3:C4"); // 合并 C3:C4 单元格
    // ws.freeze_panes("B2"); // 冻结 B2
    // wb.save("example.xlsx");

    return a.exec();
}
