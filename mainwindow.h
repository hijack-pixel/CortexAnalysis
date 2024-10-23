#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "./component/commandprocess.h"

#include <QMainWindow>
#include <QQuickWidget>
#include <QQueue>
#include <QString>
#include <QList>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


// 枚举类型，用于修改qml light的颜色
enum LightColor{
    SUCCESS = 0,
    RUNNING,
    FAIL,
};

// 枚举类型，用于表示当前图形界面显示的是第几步
enum Step{
    STEP1 = 0,
    STEP2,
    STEP3,
    STEP4,
    STEP5,
    STEP6,
};



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /**
     * @brief addNewThread 添加新的进程到设定的进程列表中
     * @param program      程序名，cmd、octave、explore....
     * @param args         参数列表
     */
    void addNewThread(QString program, QStringList args);

    void slotChange(Step currentStep);

public slots:
    /**
     * @brief changeLightColor  槽函数，用于改变qml指示灯的颜色
     * @param color             枚举类型，默认0
     */
    void changeLightColor(LightColor color=LightColor::SUCCESS);

    /**
     * @brief changeCurrentDisplay 槽函数，改变当前显示哪一步
     * @param step                 枚举类型，默认0
     */
    void changeCurrentDisplay(Step step=Step::STEP1);

    /**
     * @brief on_widgetContainerStep_1_clicked 槽函数 模块分析按钮1点击事件
     */
    void on_widgetContainerStep_1_clicked();

    /**
     * @brief on_widgetContainerStep_2_clicked 槽函数 模块分析按钮2点击事件
     */
    void on_widgetContainerStep_2_clicked();

    /**
     * @brief on_widgetContainerStep_3_clicked 槽函数 模块分析按钮3点击事件
     */
    void on_widgetContainerStep_3_clicked();

    /**
     * @brief on_widgetContainerStep_4_clicked 槽函数 模块分析按钮4点击事件
     */
    void on_widgetContainerStep_4_clicked();

    /**
     * @brief on_widgetContainerStep_5_clicked 槽函数 模块分析按钮5点击事件
     */
    void on_widgetContainerStep_5_clicked();

    /**
     * @brief on_widgetContainerStep_6_clicked 槽函数 模块分析按钮6点击事件
     */
    void on_widgetContainerStep_6_clicked();

    /**
     * @brief onTextBrowserLogShow 槽函数 在 QTextBrowser 中追加显示 log
     * @param str                  追加的 log
     */
    void onTextBrowserLogShow(QString str);

private:
    Ui::MainWindow *ui;

    QMutex mutex; // 创建互斥锁对象

    QQuickWidget  *m_qmlWidget = nullptr;  // qml控件，能放入widget交互
    QObject       *m_qmlRoot = nullptr;    // 用于调用qml内函数

    QList<const QThread*>          m_thdList;         // 保存进程列表
    QList<const CommandProcess*>   m_cmdProcessList;  // 保存 cmd 程序运行进程列表
    QList<const QQueue<QString>*>  m_cmdLog;          // 保存 cmd 程序运行的输出信息

    const int MAX_LOG_LENGHT = 1000;       // m_cmdLog 保存的最大 log 数目
    const int MAX_THREAD_NUM = 7;          // m_thdList 保存的最大线程数目

    static Step m_currentDisplay;           // 当前显示的是哪一步的页面
};


#endif // MAINWINDOW_H
