#pragma execution_character_set("utf-8")
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "./component/commandprocess.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QQuickWidget>
#include <QProcess>
#include <QFileInfo>
#include <QFileDialog>
#include "./component/clickablewidget.h"


// 静态static成员必需在类外初始化，类的静态成员变量需要在类外分配内存空间
// ！！！！！！！！！！！！！！！必须在cpp中初始化，h中初始化报错！！！！
Step MainWindow::m_currentDisplay = Step::STEP1;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    ui->widgetContainerStep_1->setProperty("widget", "stepContainer");
    ui->widgetContainerStep_2->setProperty("widget", "stepContainer");
    ui->widgetContainerStep_3->setProperty("widget", "stepContainer");
    ui->widgetContainerStep_4->setProperty("widget", "stepContainer");
    ui->widgetContainerStep_5->setProperty("widget", "stepContainer");
    ui->widgetContainerStep_6->setProperty("widget", "stepContainer");

    ClickableWidget *widget = new ClickableWidget();
    widget->setContentsMargins(QMargins(10, 10, 10, 10));
    // widget->setFixedSize(QSize(70, 70));
    QHBoxLayout *layout_ = new QHBoxLayout();
    layout_->setContentsMargins(0, 0, 10, 0);
    QLabel *label1 = new QLabel("你好难啊");
    layout_->addWidget(label1);
    widget->setLayout(layout_);
    ui->verticalLayout_4->addWidget(widget);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 10, 0);

    // 状态栏 label
    QLabel *labelStatus = new QLabel(tr("运行状态"));
    layout->addWidget(labelStatus);

    //QML呼吸灯初始化
    m_qmlWidget = new QQuickWidget(QUrl("qrc:/breathinglight.qml"), this);
    m_qmlRoot = (QObject *)m_qmlWidget->rootObject();
    m_qmlWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_qmlWidget->setClearColor(Qt::transparent);
    m_qmlWidget->setResizeMode(QQuickWidget::SizeViewToRootObject);
    layout->addWidget(m_qmlWidget);

    //添加呼吸灯到状态栏
    QWidget *widgetStatus = new QWidget();
    widgetStatus->setLayout(layout);
    ui->statusbar->addPermanentWidget(widgetStatus);

    addNewThread("octave", {"E:\\CODE\\Qt\\MouseAnalysis-Octave\\time_consume1.m"});
    addNewThread("octave", {"E:\\CODE\\Qt\\MouseAnalysis-Octave\\time_consume2.m"});
    addNewThread("octave", {"E:\\CODE\\Qt\\MouseAnalysis-Octave\\time_consume3.m"});
    addNewThread("octave", {"E:\\CODE\\Qt\\MouseAnalysis-Octave\\time_consume4.m"});
    addNewThread("octave", {"E:\\CODE\\Qt\\MouseAnalysis-Octave\\time_consume5.m"});
    addNewThread("octave", {"E:\\CODE\\Qt\\MouseAnalysis-Octave\\time_consume6.m"});


    connect(ui->pushButtonOpenFile, &QPushButton::clicked, this, [&](){
        QString fileName = QFileDialog::getOpenFileName(
            this,
            tr("open a file."),
            "E:\\CODE\\Qt\\MouseAnalysis-Octave",
            tr("images(*.tif *jpeg *bmp);;All files(*.*)"));

        qDebug() << QString("Open file: %1").arg(fileName);
    });
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changeLightColor(LightColor color)
{
    switch(color)
    {
    case LightColor::SUCCESS:
        QMetaObject::invokeMethod(m_qmlRoot, "setColor", Q_ARG(QVariant, "green"));
        break;
    case LightColor::RUNNING:
        QMetaObject::invokeMethod(m_qmlRoot, "setColor", Q_ARG(QVariant, "cyan"));
        break;
    case LightColor::FAIL:
        QMetaObject::invokeMethod(m_qmlRoot, "setColor", Q_ARG(QVariant, "red"));
        break;
    }
}


void MainWindow::changeCurrentDisplay(Step step)
{
    QMutexLocker locker(&mutex); // 当离开这个代码块时，QMutexLocker的析构函数会自动释放互斥锁
    m_currentDisplay = step;
}


void MainWindow::addNewThread(QString program, QStringList args)
{
    //创建进程用于运行分析代码
    QThread *thread = new QThread();
    CommandProcess *process = new CommandProcess(program, args);
    QQueue<QString> *queue = new QQueue<QString>();

    qDebug() << thread << process << queue;

    m_thdList.append(thread);
    m_cmdProcessList.append(process);
    m_cmdLog.append(queue);

    process->moveToThread(thread);

    //释放堆空间资源
    connect(thread, &QThread::finished, process, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();

    connect(process, &CommandProcess::resultReady, this, [&, queue](QString result){
        if(queue->size() > MAX_LOG_LENGHT)
        {
            queue->dequeue();
        }

        // 队列为空 或 入队元素和队尾不同相同，入队
        if(queue->empty() || queue->constLast() != result)
        {
            queue->enqueue(result);
        }
    });
}


void MainWindow::onTextBrowserLogShow(QString str)
{
    ui->textBrowser->append(str);
}


void MainWindow::slotChange(Step currentStep)
{
    Step preDisplay = m_currentDisplay;
    changeCurrentDisplay(currentStep);

    // qDebug() << m_cmdLog[m_currentDisplay]->join('\n');

    ui->textBrowser->clear();
    ui->textBrowser->append(m_cmdLog[m_currentDisplay]->join("<br>"));

    // 日志框显示槽函数的解绑与绑定
    disconnect(m_cmdProcessList[preDisplay], &CommandProcess::resultReady, this, &MainWindow::onTextBrowserLogShow);
    connect(m_cmdProcessList[m_currentDisplay], &CommandProcess::resultReady, this, &MainWindow::onTextBrowserLogShow);

    // 开始运行按钮槽函数的解绑与绑定
    disconnect(ui->pushButtonRun, &QPushButton::clicked, m_cmdProcessList[preDisplay], &CommandProcess::run);
    connect(ui->pushButtonRun, &QPushButton::clicked, m_cmdProcessList[m_currentDisplay], &CommandProcess::run);

    ui->labelList->setText(QString("Previous: %1  Current: %2").arg(preDisplay).arg(m_currentDisplay));
}


void MainWindow::on_widgetContainerStep_1_clicked()
{
    slotChange(Step::STEP1);
}


void MainWindow::on_widgetContainerStep_2_clicked()
{
    slotChange(Step::STEP2);
}


void MainWindow::on_widgetContainerStep_3_clicked()
{
    slotChange(Step::STEP3);
}


void MainWindow::on_widgetContainerStep_4_clicked()
{
    slotChange(Step::STEP4);
}


void MainWindow::on_widgetContainerStep_5_clicked()
{
    slotChange(Step::STEP5);
}


void MainWindow::on_widgetContainerStep_6_clicked()
{
    slotChange(Step::STEP6);

}
