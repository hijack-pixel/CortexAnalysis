#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "./component/commandprocess.h"
#include "./component/common.h"
#include "./component/configsaver.h"
#include "./component/imagelist.h"
#include "./component/comboxitemdelegate.h"
#include "./component/clickablewidget.h"
#include "./component/csvparser.h"

#include <QMainWindow>
#include <QQuickWidget>
#include <QQueue>
#include <QString>
#include <QList>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include <QVariant>
#include <QCoreApplication>
#include <QStackedLayout>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /**
     * @brief addNewThread    添加新的进程到设定的进程列表中
     * @param program         程序名，cmd、octave、explore....
     * @param args            参数列表
     * @param workDirectory   运行目录
     */
    void addNewThread(QString program, QStringList args, QString workDirectory);

    /**
     * @brief slotStepChange   因为只有一个图片展示列表和日志显示框，所以在界面切换时信号槽需要换绑
     * @param currentStep      当前新的页面，信号连接到对应的cmdProcess
     */
    void slotStepChange(Step currentStep);

    /**
     * @brief updateWidgetStepCheck  点击不同的分析步骤,按钮有选中效果,移除其他未选中的效果
     * @param stepPre                之前显示的
     * @param stepCur                当前显示的
     */
    void updateWidgetStepCheck(Step stepPre, Step stepCur);

    void initDataSettingPage1();
    void initDataSettingPage2();
    void initDataSettingPage3();
    void initDataSettingPage4();
    void initDataSettingPage5();
    void initDataSettingPage6();


protected:
    virtual void paintEvent(QPaintEvent *event) override;

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
     * @brief on_textBrowserLogShow 槽函数 在 QTextBrowser 中追加显示 log
     * @param str                   追加的 log
     */
    void on_textBrowserLogShow(QString str);

    /**
     * @brief on_processStart 在开始按钮按下时触发的函数，目的是在之前根据当前页面做一些检查和保存配置，决定是否运行分析
     */
    void on_processStart();

    /**
     * @brief on_updateImageList 在cmd命令运行结束后更新图片显示列表
     */
    void on_updateImageList();

    /**
     * @brief on_runButtonEndFinish 开始按钮随着程序运行状态改变功能,后台正常结束
     */
    void on_runButtonEndFinish();

    /**
     * @brief on_runButtonEndError 开始按钮随着程序运行状态改变功能,后台异常结束
     * @param err                  错误类型
     */
    void on_runButtonEndError(QProcess::ProcessError err);

signals:
    /**
     * @brief cmdStartRun 发出此信号，即说明通知后台开始运行命令行了
     */
    void cmdStartRun();

    /**
     * @brief cmdTerminate 发出此信号，即说明通知后台需要终止命令行了
     */
    void cmdTerminate();


private:
    Ui::MainWindow *ui;

    QMutex mutex;                                     // 创建互斥锁对象

    QQuickWidget  *m_qmlWidget = nullptr;             // qml控件，能放入widget交互
    QObject       *m_qmlRoot = nullptr;               // 用于调用qml内函数

    QList<const QThread*>          m_thdList;         // 保存进程列表
    QList<const CommandProcess*>   m_cmdProcessList;  // 保存 cmd 程序运行进程列表
    QList<const QQueue<QString>*>  m_cmdLog;          // 保存 cmd 程序运行的输出信息

    const int MAX_LOG_LENGHT = 1000;                  // m_cmdLog 保存的最大 log 数目
    const int MAX_THREAD_NUM = 7;                     // m_thdList 保存的最大线程数目

    static Step m_currentDisplay;                     // 当前显示的是哪一步的页面
    QStackedLayout* m_layoutDataSetting;              // 根据页面展示不同的数据准备选择设置
    QWidget* m_widgetDataSetting_1;
    QWidget* m_widgetDataSetting_2;
    QWidget* m_widgetDataSetting_3;
    QWidget* m_widgetDataSetting_4;
    QWidget* m_widgetDataSetting_5;
    QWidget* m_widgetDataSetting_6;


    QMap<Step, QMap<QString, QVariant>> m_config;             // 用于存储全局配置，包含每一步的数据设置和软件设置
    QMap<QString, QList<QString>> m_groupedFiles;             // 用于存储STEP1小鼠文件名分组后的文件，每只小鼠对应多个文件
    QMap<QString, QVector<QVector<int>>> m_groupedFilesPoint; // 用于存储每只小鼠对应的配准点坐标

    ConfigSaver m_configSaver;          // 用于保存配置至json文件
    CsvParser   m_csvParser;            // 用于解析csv文件

    ImageList *m_imageList;             // 图片列表显示

    // QMap<Step, QString> m_resultPath =  // 分析结果保存列表
    // {
    //     {STEP1, QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data/step1")},
    //     {STEP2, QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data/step2")},
    //     {STEP3, QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data/step3")},
    //     {STEP4, QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data/step4")},
    //     {STEP5, QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data/step5")},
    //     {STEP6, QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data/step6")}
    // };

    QMap<Step, QString> m_resultPath =  // 分析结果保存列表
    {
        {STEP1, QDir::cleanPath(QDir::currentPath() + "/data/step1")},
        {STEP2, QDir::cleanPath(QDir::currentPath() + "/data/step2")},
        {STEP3, QDir::cleanPath(QDir::currentPath() + "/data/step3")},
        {STEP4, QDir::cleanPath(QDir::currentPath() + "/data/step4")},
        {STEP5, QDir::cleanPath(QDir::currentPath() + "/data/step5")},
        {STEP6, QDir::cleanPath(QDir::currentPath() + "/data/step6")}
    };

};


#endif // MAINWINDOW_H
