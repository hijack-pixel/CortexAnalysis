#pragma execution_character_set("utf-8")
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "./component/comboxitemdelegate.h"
#include "./component/clickablewidget.h"
#include "./component/settingsdialog.h"
#include "./component/dualslider.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QQuickWidget>
#include <QProcess>
#include <QFileInfo>
#include <QFileDialog>
#include <QImage>
#include <QPainterPath>
#include <QScreen>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSpinBox>
#include <QlineEdit>
#include <QComboBox>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QTextStream>
#include <QDate>
#include <QtQuickControls2>
#include <QProgressBar>
#include <QFileSystemWatcher>
#include <QDoubleSpinBox>
#include <QScrollBar>

// #include "FramelessHelper.h"


extern QSettings globalSettings;
extern const QMap<QString, QString> imgTitleMap;
extern const QMap<Step, QString> titleMap;
extern bool deleteFolderContent(const QString &folderPath);
extern void printGlobalSettings();
extern bool recursiveCopy(const QString& source, const QString& destination, const QStringList& excludeFileNames, const QStringList& excludeDirNames, bool allowCover);




// 静态static成员必需在类外初始化，类的静态成员变量需要在类外分配内存空间
Step MainWindow::m_currentDisplay = Step::STEP1;
// ！！！！！！！！！！！！！！！必须在cpp中初始化，h中初始化报错！！！！


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 初始化类型
    qRegisterMetaType<FileType>();

    pixmapTitleLogo = new QPixmap(":icon/title.png");
    QPixmap scaledPixmap = pixmapTitleLogo->scaled(ui->labelLogoText->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelLogoText->setPixmap(scaledPixmap);

    m_configSaver.loadConfig(m_config, m_configSavePath);

    m_config[SETTINGS]["history_analysis_directory"] = m_analysisHistoryPath;

    // 设置分析步骤名和分析结果保存目录
    m_config[STEP1]["module"] = "Registration2ConnectMatrix";
    m_config[STEP2]["module"] = "cMatrix2NetGraph";
    m_config[STEP3]["module"] = "Timecourse2Spectrum";
    m_config[STEP4]["module"] = "TimeCorrMap";
    m_config[STEP5]["module"] = "Quant4SNR";
    m_config[STEP6]["module"] = "SpatialCorrMap";

    m_config[STEP1]["output_directory"] = "step1";
    m_config[STEP2]["output_directory"] = "step2";
    m_config[STEP3]["output_directory"] = "step3";
    m_config[STEP4]["output_directory"] = "step4";
    m_config[STEP5]["output_directory"] = "step5";
    m_config[STEP6]["output_directory"] = "step6";

    m_config[STEP2]["folder_path"] = QDir::cleanPath(QApplication::applicationDirPath() + QDir::separator() + "data" + QDir::separator() + "Toolbox4PYCM");

    qDebug() << "m_config[STEP2][\"folder_path\"]" << m_config[STEP2]["folder_path"];

    // 删除历史分析结果
    qDebug() << QDir::currentPath() << QCoreApplication::applicationDirPath();
    foreach (const auto& path, m_resultPath) {
        #ifdef REDIRECT_DEBUG_OUTPUT
            qDebug() << "Attempt to delete directory content: " << path;
            deleteFolderContent(path);
        #endif
        qDebug() << "DEBUG Do not Attempt to delete directory content: " << path;
    }


    // 状态栏设置
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 10, 0);

    // 状态栏 label
    QLabel *labelStatus = new QLabel(tr("运行状态"));
    layout->addWidget(labelStatus);

    //QML呼吸灯初始化
    m_qmlWidget = new QQuickWidget(QUrl("qrc:/qml/breathinglight.qml"), this);
    m_qmlRoot = (QObject *)m_qmlWidget->rootObject();
    m_qmlWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_qmlWidget->setClearColor(Qt::transparent);
    m_qmlWidget->setResizeMode(QQuickWidget::SizeViewToRootObject);
    layout->addWidget(m_qmlWidget);

    //添加呼吸灯到状态栏
    QWidget *widgetStatus = new QWidget();
    widgetStatus->setLayout(layout);
    ui->statusbar->addPermanentWidget(widgetStatus);




    // 菜单栏初始设置
    QMenu *menuFile = new QMenu(tr("开始"), this);
    QMenu *menuHelp = new QMenu(tr("帮助"), this);

    QAction *actionStep1 = new QAction(tr("成像配准"), this);
    QAction *actionStep2 = new QAction(tr("连通网络"), this);
    QAction *actionStep3 = new QAction(tr("功率谱图"), this);
    QAction *actionStep4 = new QAction(tr("稳定相关"), this);
    QAction *actionStep5 = new QAction(tr("ROI连通"), this);
    QAction *actionStep6 = new QAction(tr("信噪分析"), this);
    QAction *actionExit = new QAction(tr("退出"), this);

    QAction *actionSetting = new QAction(tr("配置"), this);
    QAction *actionAbout = new QAction(tr("关于我们"), this);

    menuFile->addAction(actionStep1);
    menuFile->addAction(actionStep2);
    menuFile->addAction(actionStep3);
    menuFile->addAction(actionStep4);
    menuFile->addAction(actionStep5);
    menuFile->addAction(actionStep6);
    menuFile->addSeparator(); // 添加分隔线
    menuFile->addAction(actionExit);

    menuHelp->addAction(actionSetting);
    menuHelp->addAction(actionAbout);

    ui->menubar->addMenu(menuFile);
    ui->menubar->addMenu(menuHelp);

    connect(actionStep1, &QAction::triggered, this, &MainWindow::on_widgetContainerStep_1_clicked);
    connect(actionStep2, &QAction::triggered, this, &MainWindow::on_widgetContainerStep_2_clicked);
    connect(actionStep3, &QAction::triggered, this, &MainWindow::on_widgetContainerStep_3_clicked);
    connect(actionStep4, &QAction::triggered, this, &MainWindow::on_widgetContainerStep_4_clicked);
    connect(actionStep5, &QAction::triggered, this, &MainWindow::on_widgetContainerStep_5_clicked);
    connect(actionStep6, &QAction::triggered, this, &MainWindow::on_widgetContainerStep_6_clicked);
    connect(actionSetting, &QAction::triggered, this, &MainWindow::on_settingAction);
    connect(actionAbout, &QAction::triggered, this, &MainWindow::on_aboutAction);
    connect(actionExit, &QAction::triggered, this, &MainWindow::on_exitAction);




    // 运行octave初始化环境配置
    QProcess octaveInitProcess;
    octaveInitProcess.setWorkingDirectory(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/octave"));
    octaveInitProcess.start("cmd", QStringList() << "/C" << "post-install.bat");
    qDebug() << "Start post-install.bat at: " << octaveInitProcess.workingDirectory();
    bool ret = octaveInitProcess.waitForFinished(3000);
    qDebug() << QString("post-install.bat exec return: %1").arg(ret);
    // if(!ret)
    // {
    //     QMessageBox::critical(nullptr, "错误", QString(tr("未检测到Octave软件分析包！")));
    //     qDebug() << "Failed to initial the octave:" << octaveInitProcess.errorString();
    //     changeQmlLightColor(LightColor::FAIL);
    // }
    // else
    // {
    //     qDebug() << "Octave initial successful";
    //     changeQmlLightColor(LightColor::SUCCESS);
    // }
    QDir octaveDir(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/octave"));
    if(!octaveDir.exists())
    {
        QMessageBox::critical(nullptr, "错误", QString(tr("未检测到Octave软件分析包！")));
        changeQmlLightColor(LightColor::FAIL);
    }
    else
    {
        qDebug() << "Octave directory exist...";
        changeQmlLightColor(LightColor::SUCCESS);
    }


    // 初始化octave 6步的分析代码
    // QString octavePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/octave/bin/octave.exe");
    QString octavePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data/exec.bat");
    QString octaveRunningPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data");
    QDir octaveRunningDir(octaveRunningPath);
    if (!octaveRunningDir.exists()) {
        if (!octaveRunningDir.mkpath(octaveRunningDir.absolutePath())) {
            qDebug() << "Failed to create directory:" << octaveRunningDir.absolutePath();
            return;
        }
    }
    addNewThread(octavePath, {"1"}, octaveRunningPath);
    addNewThread(octavePath, {"2"}, octaveRunningPath);
    addNewThread(octavePath, {"3"}, octaveRunningPath);
    addNewThread(octavePath, {"4"}, octaveRunningPath);
    addNewThread(octavePath, {"5"}, octaveRunningPath);
    addNewThread(octavePath, {"6"}, octaveRunningPath);
    // addNewThread(octavePath, {"step1.m"}, octaveRunningPath);
    // addNewThread(octavePath, {"step2.m"}, octaveRunningPath);
    // addNewThread(octavePath, {"step3.m"}, octaveRunningPath);
    // addNewThread(octavePath, {"step4.m"}, octaveRunningPath);
    // addNewThread(octavePath, {"step5.m"}, octaveRunningPath);
    // addNewThread(octavePath, {"step6.m"}, octaveRunningPath);


    // 开始运行分析按钮槽函数连接
    connect(ui->pushButtonRun, &QPushButton::clicked, this, &MainWindow::on_processStart);


    // 日志功能槽函数连接
    connect(ui->pushButtonLogClear, &QPushButton::clicked, this, &MainWindow::on_textBrowserLogClear);
    connect(ui->pushButtonLogExport, &QPushButton::clicked, this, &MainWindow::on_textBrowserLogExport);


    // 初始化监控进度文件progress.txt，用于进度条更新
    for (auto it = m_progressPath.constBegin(); it != m_progressPath.constEnd(); ++it)
    {
        QFile file(it.value());
        if (!file.exists()) // 尝试创建文件
        {
            if (!file.open(QIODevice::WriteOnly))
            {
                qDebug() << "Failed to create file:" << m_progressPath[it.key()];
            } else
            {
                file.close();
                qDebug() << "File created:" << m_progressPath[it.key()];
            }
        }
        if (!m_fileWatcher.addPath(m_progressPath[it.key()]))
        {
            qDebug() << "Failed to add path to file watcher:" << m_progressPath[it.key()];
        }
        else
        {
            qDebug() << "Add path to file watcher:" << m_progressPath[it.key()];
        }
    }

    ui->progressBar->setValue(0);
    ui->progressBar->setRange(0, 100);
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::on_updateProgressBar);
    // ui->progressBar->setVisible(false);


    // 初始化文件列表控件
    m_fileListWidget = new FileListWidget();
    ui->verticalLayoutFileList->addWidget(m_fileListWidget);
    // ui->graphicsView->enableFreeDraw();
    // ui->graphicsView->enableRectDraw();
    connect(m_fileListWidget, &FileListWidget::itemCurrent, this, [&](const QString& path, const FileType fileType){
        switch (fileType) {
        case FileType::IMAGE:
            ui->graphicsView->setImgByPath(path);
            break;
        case FileType::TABLE:
            ui->tablePreview->loadExcel(path);
            break;
        case FileType::OTHERS:
            if(path.isEmpty()){
                ui->othersPreviewLabel->setText("");
            }
            else{
                QFileInfo fileInfo(path);
                ui->othersPreviewLabel->setText(
                    QString("<b>不支持预览该类型文件:</b> %1").arg(
                        fileInfo.suffix() + "<br><br>" +
                        "<b>文件名&nbsp;&nbsp;&nbsp;：</b>" + fileInfo.fileName() + "<br>" +
                        "<b>文件大小：</b>" + QString::number(static_cast<double>(fileInfo.size()) / (1024 * 1024), 'g', 2) + " MB" + "<br>" +
                        "<b>存储路径：</b>" + fileInfo.filePath() + "<br>" +
                        "<b>创建时间：</b>" + fileInfo.fileTime(QFileDevice::FileBirthTime).toString("yyyy-MM-dd HH:mm:ss")  + "<br>" +
                        "<b>修改时间：</b>" + fileInfo.fileTime(QFileDevice::FileModificationTime).toString("yyyy-MM-dd HH:mm:ss")
                    )
                );
            }
            break;
        }
    });

    // 初始化文件列表tab页按钮
    // 图片tab，图标加载耗时，加载过程disable按钮
    connect(ui->pushButtonImgTab, &QPushButton::clicked, this, [&](){
        QPushButton *button = qobject_cast<QPushButton*>(sender());
        if(button) button->setDisabled(true);
        m_fileListWidget->setFilePath(m_resultPath[m_currentDisplay], FileType::IMAGE);
        ui->graphicsView->setHidden(false);
        ui->tablePreview->setHidden(true);
        ui->othersPreview->setHidden(true);
    });
    connect(m_fileListWidget, &FileListWidget::iconLoadFinish, ui->pushButtonImgTab, [&](){
        ui->pushButtonImgTab->setDisabled(false);
    });

    // 表格tab
    connect(ui->pushButtonTableTab, &QPushButton::clicked, this, [&](){
        m_fileListWidget->setFilePath(m_resultPath[m_currentDisplay], FileType::TABLE);
        ui->graphicsView->setHidden(true);
        ui->tablePreview->setHidden(false);
        ui->othersPreview->setHidden(true);
    });

    // 其他tab
    connect(ui->pushButtonOtherTab, &QPushButton::clicked, this, [&](){
        m_fileListWidget->setFilePath(m_resultPath[m_currentDisplay], FileType::OTHERS);
        ui->graphicsView->setHidden(true);
        ui->tablePreview->setHidden(true);
        ui->othersPreview->setHidden(false);
    });


    // 初始化数据设置部分
    m_layoutDataSetting = new QStackedLayout(ui->groupBoxData);
    m_widgetDataSetting_1 = new QWidget();
    m_widgetDataSetting_2 = new QWidget();
    m_widgetDataSetting_3 = new QWidget();
    m_widgetDataSetting_4 = new QWidget();
    m_widgetDataSetting_5 = new QWidget();
    m_widgetDataSetting_6 = new QWidget();
    m_layoutDataSetting->addWidget(m_widgetDataSetting_1);
    m_layoutDataSetting->addWidget(m_widgetDataSetting_2);
    m_layoutDataSetting->addWidget(m_widgetDataSetting_3);
    m_layoutDataSetting->addWidget(m_widgetDataSetting_4);
    m_layoutDataSetting->addWidget(m_widgetDataSetting_5);
    m_layoutDataSetting->addWidget(m_widgetDataSetting_6);



    // 初始化数据设置界面、槽函数、逻辑功能...
    initDataSettingPage1();
    initDataSettingPage2();
    initDataSettingPage3();
    initDataSettingPage4();
    initDataSettingPage5();
    initDataSettingPage6();


    // 初始化显示第一个步骤
    on_widgetContainerStep_1_clicked();



    // 更新开始运行按钮位置，因为他是浮动显示的
    ui->horizontalLayoutLog->removeWidget(ui->pushButtonRun);
    ui->pushButtonRun->setParent(ui->groupBoxData);
    ui->pushButtonRun->raise();

    QTimer::singleShot(400, this, [this]()
    {
        QRect settingRect = m_widgetDataSetting_1->geometry();
        QSize btnSize = ui->pushButtonRun->size();
        QRect btnRect;
        int padding = 5;
        btnRect.setLeft(settingRect.width() - btnSize.width() - padding*2 + settingRect.left() );
        btnRect.setTop(settingRect.height() - btnSize.height() - padding*2 + settingRect.top() );
        btnRect.setWidth(btnSize.width() + padding*2);
        btnRect.setHeight(btnSize.height() + padding*2);
        ui->pushButtonRun->setGeometry(btnRect);
        qDebug() << settingRect << btnRect;
    });

    // 默认显示当前进度也就是0
    on_updateProgressBar(m_progressPath[m_currentDisplay]);
}


MainWindow::~MainWindow()
{
    delete ui;

    // 先删除CommandProcess对象
    foreach (const CommandProcess* constCmdProcess, m_cmdProcessList) {
        constCmdProcess->abort();

        CommandProcess* cmdProcess = const_cast<CommandProcess*>(constCmdProcess); // 指针是不变的，但是CommandProcess只是普通new出来的，可以删
        delete cmdProcess;
    }


    // 再停止并删除线程
    foreach (const QThread* constThread, m_thdList) {
        QThread* thread = const_cast<QThread*>(constThread); // 指针是不变的，但是thread只是普通new出来的，可以删
        thread->quit(); // 请求线程退出
        thread->wait(); // 等待线程真正退出
        delete thread;
    }


    // 删除日志队列
    foreach (const QQueue<QString>* logQueue, m_cmdLog) {
        delete logQueue;
    }

    // 删除QWidget对象
    delete m_widgetDataSetting_1;
    delete m_widgetDataSetting_2;
    delete m_widgetDataSetting_3;
    delete m_widgetDataSetting_4;
    delete m_widgetDataSetting_5;
    delete m_widgetDataSetting_6;

    // 删除QQuickWidget对象
    delete m_qmlWidget;

    // 删除ImageList对象
    delete m_fileListWidget;

    delete pixmapTitleLogo;
}


void MainWindow::changeQmlLightColor(LightColor color)
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


void MainWindow::addNewThread(QString program, QStringList args, QString workDirectory)
{
    //创建进程用于运行分析代码
    QThread *thread = new QThread();
    CommandProcess *process = new CommandProcess(program, args, workDirectory);
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


void MainWindow::on_textBrowserLogShow(QString str)
{
    ui->textBrowser->append(str);
}



void MainWindow::on_textBrowserLogClear()
{
    ui->textBrowser->clear();
}



void MainWindow::on_textBrowserLogExport()
{
    const QQueue<QString>* currentLog = m_cmdLog.at(static_cast<int>(m_currentDisplay));

    QString directory = QFileDialog::getExistingDirectory(this, "选择日志保存目录", "", QFileDialog::ShowDirsOnly);

    if (directory.isEmpty()) return;

    QString dateString = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
    QString fileName =  QString(tr("日志_%1_%2.txt")).arg(dateString, titleMap[m_currentDisplay]);
    QString savePath = directory + QDir::separator() + fileName;
    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QString htmlText;
        QTextStream out(&file);
        static QRegularExpression htmlTagRx("<[^>]*>");
        for (QQueue<QString>::const_iterator it = currentLog->begin(); it != currentLog->end(); ++it)
        {
            htmlText = *it;
            QString plainText = htmlText;
            plainText.remove(htmlTagRx);
            out << plainText << "\n";
        }
        file.close();
    }
    else
    {
        QMessageBox::warning(this, tr("错误"), QString(tr("文件保存失败：%1")).arg(qUtf8Printable(file.errorString())));
    }
}



void MainWindow::on_updateProgressBar(const QString &path)
{
    if(!m_fileWatcher.files().contains(path))
    {
        qDebug() << QString("m_fileWatcher does not contains: %1").arg(path);
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file:" << path;
        return;
    }

    // 文件为空，退出
    qint64 fileSize = file.size();
    if (fileSize == 0)
    {
        file.close();
        return;
    }



    // 在Windows系统中，通常使用 CR+LF（即 0D 0A）来表示新行。
    // 末尾是空

    // 移动到文件末尾前的位置
    qint64 pos = fileSize - 3;
    file.seek(pos);

    // 向前查找第一个换行符
    while (pos > 0 && file.read(1) != "\n")
    {
        --pos;
        file.seek(pos);
    }

    // 读取最后一行
    QTextStream stream(&file);
    QString lastLine = stream.readAll();
    lastLine = lastLine.trimmed();
    file.close();

    bool ok;
    int progressValue = lastLine.toInt(&ok);

    qDebug() << path << lastLine << progressValue;

    if (ok)
    {
        for (auto it = m_progressPath.constBegin(); it != m_progressPath.constEnd(); ++it)
        {
            if (path == it.value())
            {
                m_progressHistory[it.key()] = progressValue;   //更新保存每一步的历史进度
                if(it.key() == m_currentDisplay)                // 但是只显示当前步的进度
                    ui->progressBar->setValue(m_progressHistory[it.key()]);
            }
        }
    }
    else
    {
        qDebug() << "Failed to convert progress value to integer";
    }
}



void MainWindow::on_runButtonEndFinish()
{
    if(!m_cmdProcessList[m_currentDisplay]->isRunning())
    {
        ui->progressBar->setValue(100);
        // ui->progressBar->setStyleSheet("QProgressBar {"
        //                                 "    background-color: white; /* 背景色 */"
        //                                 "    border-radius: 2px; "
        //                                 "    border: 0px solid #F0F0F0;"
        //                                 "}"
        //                                );
        ui->pushButtonRun->setDisabled(false);
        ui->pushButtonRun->setText(tr("开始"));
        qDebug() << "Cmd Finish, change button state: ";
    }
}



void MainWindow::on_runButtonEndError(QProcess::ProcessError err)
{
    if(!m_cmdProcessList[m_currentDisplay]->isRunning())
    {
        m_progressHistory[m_currentDisplay] = 0;
        ui->progressBar->setValue(0);
        // ui->progressBar->setStyleSheet("QProgressBar::chunk {"
        //                                 "    background-color: red; /* 进度条填充色 */"
        //                                 "    border-radius: 9px;"
        //                                 "} "
        // );
        ui->pushButtonRun->setDisabled(false);
        ui->pushButtonRun->setText(tr("开始"));
        qDebug() << "Cmd Error, change button state: " ;
    }
}



void MainWindow::on_settingAction()
{
    SettingsDialog settingsDialog(nullptr);
    settingsDialog.exec();
    printGlobalSettings();
}



void MainWindow::on_aboutAction()
{
    QMessageBox aboutBox;

    aboutBox.setWindowTitle(tr("关于我们 - 大脑皮层成像分析软件"));

    aboutBox.setText(tr("<h2>欢迎来到大脑皮层成像分析软件！</h2>"
                        "<p>我们的团队由一群充满激情的医疗专家、软件开发者和数据科学家组成，"
                        "致力于为医疗行业提供先进的影像分析和数据处理解决方案。</p>"
                        "<h3>我们的使命</h3>"
                        "<p>我们的目标是利用最前沿的技术帮助医生更准确地诊断和治疗脑部疾病，"
                        "提高患者的生存率和生活质量。</p>"
                        "<h3>我们的产品</h3>"
                        "<ul>"
                        "<li>成像配准：确保不同时间点的影像能够精确对齐，便于比较和分析。</li>"
                        "<li>连通网络分析：深入理解大脑内部复杂的神经网络结构。</li>"
                        "<li>功率图谱绘制：可视化显示大脑活动的强度和分布情况。</li>"
                        "<li>稳定相关分析：评估大脑区域之间的关联性。</li>"
                        "<li>ROI连通性分析：研究特定感兴趣区域的连接模式。</li>"
                        "<li>信噪比分析：优化影像质量，减少噪声干扰。</li>"
                        "</ul>"
                        "<h3>为什么选择我们？</h3>"
                        "<ul>"
                        "<li>先进技术：采用最新的算法和技术，保证分析的准确性和可靠性。</li>"
                        "<li>易用界面：直观的用户界面设计，即使是初学者也能快速上手。</li>"
                        "<li>定制化服务：根据您的具体需求进行个性化设置和服务支持。</li>"
                        "<li>持续更新：定期发布新版本和更新，不断改进产品性能和用户体验。</li>"
                        "</ul>"
                        "<h3>联系我们</h3>"
                        "<p>电子邮件: support@brainimagingsoftware.com</p>"
                        "<p>电话: +1234567890</p>"
                        "<p>网站: <a href=\"http://www.brainimagingsoftware.com\">www.brainimagingsoftware.com</a></p>"
                        "<p>感谢您选择大脑皮层成像分析软件，让我们一起为人类的健康事业贡献力量！</p>"));


    aboutBox.exec();
}


void MainWindow::on_exitAction()
{
    QMessageBox msgBox;
    msgBox.setText("你想在退出前保存文件吗？");
    QPushButton *saveDefaultButton = msgBox.addButton(tr("默认保存"), QMessageBox::AcceptRole);
    QPushButton *saveCustomButton = msgBox.addButton(tr("自选保存"), QMessageBox::AcceptRole);
    QPushButton *discardButton = msgBox.addButton(tr("不保存"), QMessageBox::AcceptRole);
    QPushButton *cancelButton = msgBox.addButton(tr("取消"), QMessageBox::RejectRole);
    msgBox.setDefaultButton(saveDefaultButton);

    auto saveFileToDefaultLocation = [this](){
        QDir dir(m_config[SETTINGS]["history_analysis_directory"].toString());

        // 如果目录不存在，则创建
        if (!dir.exists()) {
            if (!dir.mkpath(dir.absolutePath())) {
                qDebug() << "Failed to create directory:" << dir.absolutePath();
                return;
            }
        }

        // 创建时间文件夹
        QString timeFolder = dir.absolutePath() + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
        QDir timeDir(timeFolder);
        if (!timeDir.exists() && !timeDir.mkpath(timeFolder)) {
            qDebug() << "Failed to create time folder:" << timeFolder;
            return;
        }

        bool ret;
        ret = recursiveCopy(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data"),
                            timeFolder,
                            QStringList({"progress.txt", "config.json", "MyExecutable.exe", "exec.bat"}),
                            QStringList({"Toolbox4PYCM"}),
                            true);
        if(!ret) qDebug() << QString("Copy %1 to %2, some mistankens happend....")
                            .arg(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data" ), timeFolder);
    };

    auto saveFileToCustomLocation = [this](){
        QString directory = QFileDialog::getExistingDirectory(this,
                                                              "选择保存目录",
                                                              QCoreApplication::applicationDirPath(),
                                                              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (directory.isEmpty()) {
            qDebug() << "用户取消了目录选择。";
            return;
        }
        qDebug() << "选择的保存目录是：" << directory;

        bool ret;
        ret = recursiveCopy(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data"),
                            directory,
                            QStringList({"progress.txt", "config.json", "MyExecutable.exe", "exec.bat"}),
                            QStringList({"Toolbox4PYCM"}),
                            true);
        if(!ret) qDebug() << QString("Copy %1 to %2, some mistankens happend....")
                            .arg(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data" ), directory);
    };

    int ret = msgBox.exec();
    qDebug() << ret;

    if(msgBox.clickedButton() == saveDefaultButton)
    {
        saveFileToDefaultLocation();
        qApp->quit();
    }
    else if(msgBox.clickedButton() == saveCustomButton)
    {
        saveFileToCustomLocation();
        qApp->quit();
    }
    else if(msgBox.clickedButton() == discardButton)
    {
        qApp->quit();
    }
    else if(msgBox.clickedButton() == cancelButton)
    {
        return;
    }
}


void MainWindow::slotStepChange(Step currentStep)
{
    Step preDisplay = m_currentDisplay;
    changeCurrentDisplay(currentStep);
    updateWidgetStepCheck(preDisplay, currentStep);

    // 清除输出，添加当前运行的log
    ui->textBrowser->clear();
    ui->textBrowser->append(m_cmdLog[m_currentDisplay]->join("<br>"));

    // 日志框显示槽函数的解绑与绑定
    disconnect(m_cmdProcessList[preDisplay], &CommandProcess::resultReady, this, &MainWindow::on_textBrowserLogShow);
    connect(m_cmdProcessList[m_currentDisplay], &CommandProcess::resultReady, this, &MainWindow::on_textBrowserLogShow);

    // 开始信号与后台程序解绑与绑定
    disconnect(this, &MainWindow::cmdStartRun, m_cmdProcessList[preDisplay], &CommandProcess::run);
    connect(this, &MainWindow::cmdStartRun, m_cmdProcessList[m_currentDisplay], &CommandProcess::run);

    // 终止信号与后台程序解绑与绑定
    disconnect(this, &MainWindow::cmdTerminate, m_cmdProcessList[preDisplay], &CommandProcess::abort);
    connect(this, &MainWindow::cmdTerminate, m_cmdProcessList[m_currentDisplay], &CommandProcess::abort);

    // 开始运行按钮槽函数的解绑与绑定,页面切换时初始化文字
    disconnect(m_cmdProcessList[preDisplay], &CommandProcess::cmdFinish, this, &MainWindow::on_runButtonEndFinish);
    connect(m_cmdProcessList[m_currentDisplay], &CommandProcess::cmdFinish, this, &MainWindow::on_runButtonEndFinish);

    disconnect(m_cmdProcessList[preDisplay], &CommandProcess::cmdError, this, &MainWindow::on_runButtonEndError);
    connect(m_cmdProcessList[m_currentDisplay], &CommandProcess::cmdError, this, &MainWindow::on_runButtonEndError);

    // // 更新m_fileWatcher文件，只监控当前步骤progress.txt，解除其他的监控
    // m_fileWatcher.removePaths(m_fileWatcher.files());
    // QFile file(m_progressPath[m_currentDisplay]);
    // if (!file.exists()) // 尝试创建文件
    // {
    //     if (!file.open(QIODevice::WriteOnly))
    //     {
    //         qDebug() << "Failed to create file:" << m_progressPath[m_currentDisplay];
    //     } else
    //     {
    //         file.close();
    //         qDebug() << "File created:" << m_progressPath[m_currentDisplay];
    //     }
    // }
    // if (!m_fileWatcher.addPath(m_progressPath[m_currentDisplay]))
    // {
    //     qDebug() << "Failed to add path to file watcher:" << m_progressPath[m_currentDisplay];
    // }

    ui->progressBar->setValue(m_progressHistory[m_currentDisplay]);


    // 更新开始按钮
    ui->pushButtonRun->setDisabled(false);
    if(m_cmdProcessList[m_currentDisplay]->isRunning())
    {
        ui->pushButtonRun->setProperty("run", true);
        ui->pushButtonRun->setText(tr("暂停"));
    }
    else
    {
        ui->pushButtonRun->setProperty("run", false);
        ui->pushButtonRun->setText(tr("开始"));
    }

    // 图片列表显示的解绑与绑定
    disconnect(m_cmdProcessList[preDisplay], &CommandProcess::cmdFinish, this, &MainWindow::on_updateImageList);
    connect(m_cmdProcessList[m_currentDisplay], &CommandProcess::cmdFinish, this, &MainWindow::on_updateImageList);

    // 更新图片列表标题
    ui->labelList->setText(titleMap[m_currentDisplay]);


    // 移除旧的控件
    // clearWidget(qobject_cast<QWidget*>(ui->groupBoxData));
}


void MainWindow::updateWidgetStepCheck(Step stepPre, Step stepCur)
{
    // 关闭之前的选中效果
    switch(stepPre)
    {
    case STEP1: ui->widgetContainerStep_1->setChecked(false);break;
    case STEP2: ui->widgetContainerStep_2->setChecked(false);break;
    case STEP3: ui->widgetContainerStep_3->setChecked(false);break;
    case STEP4: ui->widgetContainerStep_4->setChecked(false);break;
    case STEP5: ui->widgetContainerStep_5->setChecked(false);break;
    case STEP6: ui->widgetContainerStep_6->setChecked(false);break;
    default: qDebug()<<"Invalid Step: stepPre";
    }

    // 打开现在的选中效果
    switch(stepCur)
    {
    case STEP1: ui->widgetContainerStep_1->setChecked(true);break;
    case STEP2: ui->widgetContainerStep_2->setChecked(true);break;
    case STEP3: ui->widgetContainerStep_3->setChecked(true);break;
    case STEP4: ui->widgetContainerStep_4->setChecked(true);break;
    case STEP5: ui->widgetContainerStep_5->setChecked(true);break;
    case STEP6: ui->widgetContainerStep_6->setChecked(true);break;
    default: qDebug()<<"Invalid Step: stepPre";
    }
}


bool MainWindow::getGroupFiles(QFileInfoList fileList, QMap<QString, QList<QString>>& groupFiles)
{
    // 获取不带扩展名的文件名
    QList<QString> fileNameList;
    foreach (const QFileInfo &fileInfo, fileList)
        fileNameList.append(fileInfo.baseName());

    // 文件名排序
    fileNameList.sort();

    // 文件名分组
    foreach(const auto fileName, fileNameList)
    {
        if(groupFiles.contains(fileName))  // 前缀键存在，跳过此次
            continue;

        QChar prefixBack = fileName.back();
        foreach(const auto f, fileNameList)  // 寻找相同的前缀加入groupedFiles
        {
            if(f==fileName && f.startsWith(fileName) && !groupFiles[fileName].contains(f))  // CON 1\CON 1@123\...，处理碰到CON 1
            {
                groupFiles[fileName].append(f);
            }

            if(f!=fileName && f.startsWith(fileName) && !groupFiles[fileName].contains(f))  // CON 1\CON 1@123\...，处理碰到CON 1@xxxx
            {
                QChar prefixNext = f.at(fileName.length());
                if(prefixBack.isDigit() && prefixNext.isDigit()) // 假如前缀刚好分到数字，比如CON 1\CON 10，这就不是一组
                    continue;

                groupFiles[fileName].append(f);
            }
        }
    }

    // 文件名分组中的文件名修改为绝对路径
    foreach (auto key, groupFiles.keys())
    {
        for(int i = 0; i < groupFiles[key].length(); i++)
        {
            foreach (const QFileInfo &fileInfo, fileList) {
                if(groupFiles[key][i] == fileInfo.baseName())
                {
                    groupFiles[key][i] = fileInfo.absoluteFilePath();
                }
            }
        }

        if(groupFiles[key].length() == 1)  // 删除值只有一个的，并不是想要分组的对象
        {
            groupFiles.remove(key);
        }
    }

    QSet<int> num;
    foreach (auto key, groupFiles.keys())
    {
        num.insert(groupFiles[key].length());
    }
    if(num.count() == 1)   // 每个组名key对应的小鼠文件数量相同
        return true;
    else
    {
        QString str;
        foreach (auto key, groupFiles.keys()) {
            str.append(key + ": \n" + groupFiles[key].join(", ") + "\n\n");
        }

        QString nums;
        foreach (auto value, num.values()) {
            nums.append(QString::number(value)+" ");
        }

        qDebug() << QString("The values' count of each key should be equaled(got %1):\n").arg(nums) << groupFiles;

        QMessageBox::critical(nullptr, "错误", QString("每组小鼠文件数量应该相等(每组数量有：%1): \n%2").arg(nums, str));
        groupFiles.clear();
        return false;
    }
}


bool MainWindow::isGroupFilesCompleted(const QMap<QString, QList<QString>>& groupFiles)
{
    QSet<int> numSet;
    foreach (auto key, groupFiles.keys())
    {
        numSet.insert(groupFiles[key].length());
    }
    if(numSet.count() == 1)   // 每个组名key对应的小鼠文件数量相同
        return true;
    else if(numSet.contains(0))
    {
        QMessageBox::critical(nullptr, "错误", QString("每组小鼠文件数量不应为0！"));
        return false;
    }
    else
    {
        QString str;
        foreach (auto key, groupFiles.keys()) {
            str.append(key + ": \n" + groupFiles[key].join(", ") + "\n\n");
        }
        qDebug() << "The values' count of each key should be equaled:\n" << groupFiles;

        QString nums;
        foreach (auto value, numSet.values()) {
            nums.append(QString::number(value)+" ");
        }

        QMessageBox::critical(nullptr, "错误", QString("每组小鼠文件数量应该相等(每组数量有：%1): \n%2").arg(nums).arg(str));
        return false;
    }
}


bool MainWindow::getGroupFilesPoints(CsvParser& csvParser, QString path, QMap<QString, QVector<QVector<int>>>& groupPoints)
{
    if(!m_csvParser.parse(path))
    {
        qDebug() << "CsvParser error.";
        return false;
    }

    const QVector<QStringList> &csvData = m_csvParser.getData();
    foreach(const auto &list, csvData)
    {
        auto key = list[0];
        if(!groupPoints.contains(key))  // 没有这个键，退出解析
        {
            QMessageBox::critical(nullptr, "错误", QString("文件名错误: %1\n%2").arg(key, list.join(',')));
            return false;
        }
        if(list.size() != 5)   // 一行应该有文件名加四个数字，不满足，退出解析
        {
            QMessageBox::critical(nullptr, "错误", QString("数据量错误: %1\n应为：文件名,X1,Y1,X2,Y2").arg(list.join(',')));
            return false;
        }
        bool ok;
        groupPoints[key][0][0] = list[1].toInt(&ok);
        if(!ok)
        {
            groupPoints[key][0][0] = -1;
            QMessageBox::critical(nullptr, "错误", QString("坐标格式错误，无法转换为数字: %1\n%2").arg(list[1], list.join(',')));
            return false;
        }
        groupPoints[key][0][1] = list[2].toInt(&ok);
        if(!ok)
        {
            groupPoints[key][0][1] = -1;
            QMessageBox::critical(nullptr, "错误", QString("坐标格式错误，无法转换为数字: %1\n%2").arg(list[2], list.join(',')));
            return false;
        }
        groupPoints[key][1][0] = list[3].toInt(&ok);
        if(!ok)
        {
            groupPoints[key][1][0] = -1;
            QMessageBox::critical(nullptr, "错误", QString("坐标格式错误，无法转换为数字: %1\n%2").arg(list[3], list.join(',')));
            return false;
        }
        groupPoints[key][1][1] = list[4].toInt(&ok);
        if(!ok)
        {
            groupPoints[key][1][1] = -1;
            QMessageBox::critical(nullptr, "错误", QString("坐标格式错误，无法转换为数字: %1\n%2").arg(list[4], list.join(',')));
            return false;
        }
    }
    return true;
}

bool MainWindow::isGroupFilesPointsCompleted(const QMap<QString, QVector<QVector<int>>>& groupPoints)
{
    foreach(const auto key, groupPoints.keys())
    {
        for(int i = 0; i < groupPoints[key].size(); ++i)
            for(int j = 0; j < groupPoints[key][i].size(); ++j)
            {
                if(groupPoints[key][i][j] == -1)
                {
                    QMessageBox::warning(nullptr, "错误", "每只小鼠的配准坐标都需要设置！",  QMessageBox::Ok,QMessageBox::Ok);
                    return false;
                }
            }
    }
    return true;
}


QString MainWindow::getGroupDataDetail(const QMap<QString, QList<QString>>& groupFiles, const QMap<QString, QVector<QVector<int>>>& groupPoints)
{
    QString detail;
    if(groupFiles.empty())
        return QString("还未选择文件！");

    foreach (auto key, groupFiles.keys()) {
        QString point = "配准坐标:  ";
        if(groupPoints.contains(key))
        {
            for(int i = 0; i < groupPoints[key].size(); ++i)
                for(int j = 0; j < groupPoints[key][i].size(); ++j)
                {
                    if(groupPoints[key][i][j] == -1)
                        point.append("未设置,");
                    else
                        point.append(QString::number(groupPoints[key][i][j])+",");
                }
        }
        detail.append(key + ": " + "\n" + point + "\n");
        detail.append(groupFiles[key].join("\n"));
        detail.append("\n\n");
    }

    return detail;
}


void MainWindow::initDataSettingPage1()
{
    QGridLayout *grid = new QGridLayout();

    // 数据文件位置选择
    QLabel *labelDirectory = new QLabel(tr("文件位置"));
    QHBoxLayout *hLayoutForDirectory = new QHBoxLayout();

    QLineEdit *lineEditDirectory = new QLineEdit();
    lineEditDirectory->setReadOnly(true);
    QPushButton *pushButtonDirectory = new QPushButton(tr("选择"));

    hLayoutForDirectory->addWidget(lineEditDirectory);
    hLayoutForDirectory->addWidget(pushButtonDirectory);

    // 小鼠数量信息
    QLabel *labelData = new QLabel(tr("数据概览"));

    // 小鼠数量展示
    QSpinBox *spinBoxMouseNum = new QSpinBox();
    spinBoxMouseNum->setRange(0, 1000);
    spinBoxMouseNum->setReadOnly(true);
    spinBoxMouseNum->setPrefix(tr("小鼠数量 : "));
    spinBoxMouseNum->setStyleSheet(
        "QSpinBox QAbstractSpinBox::prefix {"
        "    color: #888888;" // 设置前缀颜色为淡灰色
        "}"
        );

    // 每只小鼠 tif 文件数量展示
    QSpinBox *spinBoxTifNum = new QSpinBox();
    spinBoxTifNum->setRange(0, 1000);
    spinBoxTifNum->setReadOnly(true);
    spinBoxTifNum->setPrefix(tr("每只小鼠文件数量 : "));
    spinBoxTifNum->setStyleSheet(
        "QSpinBox QAbstractSpinBox::prefix {"
        "    color: #888888;" // 设置前缀颜色为淡灰色
        "}"
        );

    QPushButton *pushButtonDetail = new QPushButton(tr("数据详情"));

    // 每只小鼠的配准坐标
    QLabel *labelPoint = new QLabel(tr("配准坐标"));

    QPushButton *pushButtonPointTXTImport = new QPushButton(tr("坐标快速导入"));
    pushButtonPointTXTImport->setToolTip(m_pointTxtImportHelp);

    QComboBox *comboBoxSelectMouse = new QComboBox();
    ComboxItemDelegate *delegate = new ComboxItemDelegate();
    comboBoxSelectMouse->setItemDelegate(delegate);
    comboBoxSelectMouse->setPlaceholderText(tr("选择小鼠..."));

    // 创建一个QLineEdit并设置占位符文本
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setPlaceholderText("选择小鼠...");
    lineEdit->setReadOnly(true);
    comboBoxSelectMouse->setLineEdit(lineEdit);

    QLabel *labelPonintX1 = new QLabel(tr("X1:"));
    labelPonintX1->setAlignment(Qt::AlignRight);
    labelPonintX1->setMargin(5);
    QSpinBox *spinBoxPointX1 = new QSpinBox();
    spinBoxPointX1->setRange(0, 10000);
    spinBoxPointX1->setValue(0);
    spinBoxPointX1->setButtonSymbols(QSpinBox::NoButtons);

    QLabel *labelPonintY1 = new QLabel(tr("Y1:"));
    labelPonintY1->setAlignment(Qt::AlignRight);
    labelPonintY1->setMargin(5);
    QSpinBox *spinBoxPointY1 = new QSpinBox();
    spinBoxPointY1->setRange(0, 10000);
    spinBoxPointY1->setValue(0);
    spinBoxPointY1->setButtonSymbols(QSpinBox::NoButtons);

    QLabel *labelPonintX2 = new QLabel(tr("X2:"));
    labelPonintX2->setAlignment(Qt::AlignRight);
    labelPonintX2->setMargin(5);
    QSpinBox *spinBoxPointX2 = new QSpinBox();
    spinBoxPointX2->setRange(0, 10000);
    spinBoxPointX2->setValue(0);
    spinBoxPointX2->setButtonSymbols(QSpinBox::NoButtons);

    QLabel *labelPonintY2 = new QLabel(tr("Y2:"));
    labelPonintY2->setAlignment(Qt::AlignRight);
    labelPonintY2->setMargin(5);
    QSpinBox *spinBoxPointY2 = new QSpinBox();
    spinBoxPointY2->setRange(0, 10000);
    spinBoxPointY2->setValue(0);
    spinBoxPointY2->setButtonSymbols(QSpinBox::NoButtons);

    // 添加坐标输入框
    QGridLayout *gridPoint = new QGridLayout();
    gridPoint->addWidget(comboBoxSelectMouse, 0, 0, 1, 6);
    gridPoint->addWidget(pushButtonPointTXTImport, 0, 6, 1, 3);

    gridPoint->addWidget(new QLabel(tr("坐标1 (B): ")), 2, 0, 1, 1);
    QHBoxLayout *hLayoutForPoint1 = new QHBoxLayout();
    hLayoutForPoint1->addWidget(new QLabel(tr("(")), 1);
    hLayoutForPoint1->addWidget(spinBoxPointX1, 7);
    hLayoutForPoint1->addWidget(new QLabel(tr(",")), 1);
    hLayoutForPoint1->addWidget(spinBoxPointY1, 7);
    hLayoutForPoint1->addWidget(new QLabel(tr(")")));
    gridPoint->addLayout(hLayoutForPoint1, 2, 1, 1, 4);

    gridPoint->addWidget(new QLabel(tr("坐标2 (Λ): ")), 3, 0, 1, 1);
    QHBoxLayout *hLayoutForPoint2 = new QHBoxLayout();
    hLayoutForPoint2->addWidget(new QLabel(tr("(")), 1);
    hLayoutForPoint2->addWidget(spinBoxPointX2, 7);
    hLayoutForPoint2->addWidget(new QLabel(tr(",")), 1);
    hLayoutForPoint2->addWidget(spinBoxPointY2, 7);
    hLayoutForPoint2->addWidget(new QLabel(tr(")")));
    gridPoint->addLayout(hLayoutForPoint2, 3, 1, 1, 4);

    comboBoxSelectMouse->setDisabled(true);
    spinBoxMouseNum->setDisabled(true);
    spinBoxTifNum->setDisabled(true);
    spinBoxPointX1->setDisabled(true);
    spinBoxPointX2->setDisabled(true);
    spinBoxPointY1->setDisabled(true);
    spinBoxPointY2->setDisabled(true);
    pushButtonPointTXTImport->setDisabled(true);

    // 选择文件目录，按照前缀来自动识别文件并分组
    auto on_selectDirectory = [&, comboBoxSelectMouse, lineEditDirectory, spinBoxMouseNum, spinBoxTifNum, spinBoxPointX1, spinBoxPointX2, spinBoxPointY1, spinBoxPointY2, pushButtonPointTXTImport](){
        QString directory = QFileDialog::getExistingDirectory(
            nullptr,
            "选择数据目录",
            QCoreApplication::applicationDirPath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        qDebug() << "Selected directory:" << directory;
        if (directory.isEmpty())
        {
            qDebug() << "No directory selected.";
            comboBoxSelectMouse->setDisabled(true);
            spinBoxMouseNum->setDisabled(true);
            spinBoxTifNum->setDisabled(true);
            spinBoxPointX1->setDisabled(true);
            spinBoxPointX2->setDisabled(true);
            spinBoxPointY1->setDisabled(true);
            spinBoxPointY2->setDisabled(true);
            pushButtonPointTXTImport->setDisabled(true);
            // QMessageBox::critical(nullptr, "错误", QString("未选择文件！"));
            return;
        }

        // 清除原先内容防止累加
        m_groupedFilesStep1.clear();

        // 只读tif文件
        QDir dir(directory);
        QFileInfoList fileList = dir.entryInfoList(QStringList() << "*.tif", QDir::Files);

        if(fileList.length() == 0)
        {
            qDebug() << "Directory has no files.";
            lineEditDirectory->setText(tr("文件夹为空！"));
            comboBoxSelectMouse->setDisabled(true);
            spinBoxMouseNum->setDisabled(true);
            spinBoxTifNum->setDisabled(true);
            spinBoxPointX1->setDisabled(true);
            spinBoxPointX2->setDisabled(true);
            spinBoxPointY1->setDisabled(true);
            spinBoxPointY2->setDisabled(true);
            pushButtonPointTXTImport->setDisabled(true);
            return;
        }
        comboBoxSelectMouse->setDisabled(false);
        spinBoxMouseNum->setDisabled(false);
        spinBoxTifNum->setDisabled(false);
        spinBoxPointX1->setDisabled(false);
        spinBoxPointX2->setDisabled(false);
        spinBoxPointY1->setDisabled(false);
        spinBoxPointY2->setDisabled(false);
        pushButtonPointTXTImport->setDisabled(false);

        bool ok = getGroupFiles(fileList, m_groupedFilesStep1);
        if(!ok) return;

        /********************************************************************************************/
        foreach (auto key, m_groupedFilesStep1.keys())  // 目前仅支持一个小鼠3个tif
            while(m_groupedFilesStep1[key].length()>3)  // 目前仅支持一个小鼠3个tif
            {                                      // 目前仅支持一个小鼠3个tif
                m_groupedFilesStep1[key].pop_back();    // 目前仅支持一个小鼠3个tif
            }                                      // 目前仅支持一个小鼠3个tif
        /********************************************************************************************/

        // 初始化配准坐标
        m_groupedFilesPointStep1.clear();
        for (auto it = m_groupedFilesStep1.constBegin(); it != m_groupedFilesStep1.constEnd(); ++it) {
            QVector<QVector<int>> points(2, QVector<int>(2, -1));
            m_groupedFilesPointStep1.insert(it.key(), points);
        }

        // 根据 key 内容判断现在是对照组还是突变组
        bool isMUT = true;
        bool isCON = true;
        foreach (auto key, m_groupedFilesStep1.keys())
        {
            isMUT = isMUT && key.contains("MUT");
            isCON = isCON && key.contains("CON");
        }

        if(isMUT ^ isCON)  // 只能是con mut其中一个
        {
            if(isMUT) m_config[STEP1]["group_name"] = "MUT";
            if(isCON) m_config[STEP1]["group_name"] = "CON";
        }
        else
        {
            m_config[STEP1]["group_name"] = "UNKNOW";
        }

        // qDebug() << "m_groupedFilesStep1" << m_groupedFilesStep1;


        // 设置目录选择框
        lineEditDirectory->setText(directory);

        // 设置小鼠数目选择框
        spinBoxMouseNum->setValue(m_groupedFilesStep1.count());

        // 设置tif数量选择框，设置下拉框
        QSet<int> num;
        comboBoxSelectMouse->clear();
        foreach (auto key, m_groupedFilesStep1.keys())
        {
            num.insert(m_groupedFilesStep1[key].length());
            comboBoxSelectMouse->addItem(key, QVariant(false)); // 设置小鼠配准坐标ComBox
        }
        if(num.count() == 1)   // 每个组名key对应的小鼠文件数量相同
        {
            spinBoxTifNum->setValue(*num.begin());
        }

        m_config[STEP1]["input_files"] = QVariant::fromValue(m_groupedFilesStep1);
        m_config[STEP1]["input_file_directory"] = directory;
        m_config[STEP1]["mouse_count"] = m_groupedFilesStep1.count();
        m_config[STEP1]["files_per_mouse"] = *num.begin();
    };

    auto on_pointX1Change = [&, comboBoxSelectMouse](const int x){
        QString currentMouse = comboBoxSelectMouse->currentText();
        if(m_groupedFilesPointStep1.contains(currentMouse))
        {
            m_groupedFilesPointStep1[currentMouse][0][0] = x;

            // 检查4个点是否都设置了
            for (int i = 0; i < m_groupedFilesPointStep1[currentMouse].size(); ++i) {
                for (int j = 0; j < m_groupedFilesPointStep1[currentMouse][i].size(); ++j) {
                    if(m_groupedFilesPointStep1[currentMouse][i][j] == -1)
                        return;
                }
            }
            int index = comboBoxSelectMouse->currentIndex();
            comboBoxSelectMouse->setItemData(index, true);
        }
    };

    auto on_pointY1Change = [&, comboBoxSelectMouse](const int x){
        QString currentMouse = comboBoxSelectMouse->currentText();
        if(m_groupedFilesPointStep1.contains(currentMouse))
        {
            m_groupedFilesPointStep1[currentMouse][0][1] = x;

            // 检查4个点是否都设置了
            for (int i = 0; i < m_groupedFilesPointStep1[currentMouse].size(); ++i) {
                for (int j = 0; j < m_groupedFilesPointStep1[currentMouse][i].size(); ++j) {
                    if(m_groupedFilesPointStep1[currentMouse][i][j] == -1)
                        return;
                }
            }
            int index = comboBoxSelectMouse->currentIndex();
            comboBoxSelectMouse->setItemData(index, true);
        }
    };

    auto on_pointX2Change = [&, comboBoxSelectMouse](const int x){
        QString currentMouse = comboBoxSelectMouse->currentText();
        if(m_groupedFilesPointStep1.contains(currentMouse))
        {
            m_groupedFilesPointStep1[currentMouse][1][0] = x;

            // 检查4个点是否都设置了
            for (int i = 0; i < m_groupedFilesPointStep1[currentMouse].size(); ++i) {
                for (int j = 0; j < m_groupedFilesPointStep1[currentMouse][i].size(); ++j) {
                    if(m_groupedFilesPointStep1[currentMouse][i][j] == -1)
                        return;
                }
            }
            int index = comboBoxSelectMouse->currentIndex();
            comboBoxSelectMouse->setItemData(index, true);
        }
    };

    auto on_pointY2Change = [&, comboBoxSelectMouse](const int x){
        QString currentMouse = comboBoxSelectMouse->currentText();
        if(m_groupedFilesPointStep1.contains(currentMouse))
        {
            m_groupedFilesPointStep1[currentMouse][1][1] = x;

            // 检查4个点是否都设置了
            for (int i = 0; i < m_groupedFilesPointStep1[currentMouse].size(); ++i) {
                for (int j = 0; j < m_groupedFilesPointStep1[currentMouse][i].size(); ++j) {
                    if(m_groupedFilesPointStep1[currentMouse][i][j] == -1)
                        return;
                }
            }

            int index = comboBoxSelectMouse->currentIndex();
            comboBoxSelectMouse->setItemData(index, true);
        }
    };

    auto on_comboxSelectChange = [&, spinBoxPointX1, spinBoxPointX2, spinBoxPointY1, spinBoxPointY2](const QString &text){
        if(m_groupedFilesPointStep1.contains(text))
        {
            // 阻止发送一次 spinBox 值改变信号，因为会导致 comBox设置完所有点后显示绿色的样式异常
            // 因为对于新的combox selectedItem 四个 piont 又重新设置值，这样就相当于设置完了要改变样式了
            spinBoxPointX1->blockSignals(true);
            spinBoxPointY1->blockSignals(true);
            spinBoxPointX2->blockSignals(true);
            spinBoxPointY2->blockSignals(true);

            spinBoxPointX1->setValue(m_groupedFilesPointStep1[text][0][0]);
            spinBoxPointY1->setValue(m_groupedFilesPointStep1[text][0][1]);
            spinBoxPointX2->setValue(m_groupedFilesPointStep1[text][1][0]);
            spinBoxPointY2->setValue(m_groupedFilesPointStep1[text][1][1]);

            spinBoxPointX1->blockSignals(false);
            spinBoxPointY1->blockSignals(false);
            spinBoxPointX2->blockSignals(false);
            spinBoxPointY2->blockSignals(false);
        }
    };

    auto on_buttonTXTImport = [&, comboBoxSelectMouse]()
    {
        QString fullPath = QFileDialog::getOpenFileName(
            nullptr,
            tr("选择导入TXT配准坐标"),
            QCoreApplication::applicationDirPath(),
            "Text Files (*.txt);;All Files (*)"
            );

        if (fullPath.isEmpty())
            return;

        bool ok = getGroupFilesPoints(m_csvParser, fullPath, m_groupedFilesPointStep1);
        if(!ok) return;
        qDebug() << m_groupedFilesPointStep1;

        // 检查4个点是否都设置了，更新combox选择框
        foreach(auto key, m_groupedFilesPointStep1.keys())
        {
            int index = comboBoxSelectMouse->findText(key);
            if(m_groupedFilesPointStep1[key][0][0]+m_groupedFilesPointStep1[key][0][1]+m_groupedFilesPointStep1[key][1][0]+m_groupedFilesPointStep1[key][1][1]!= -4)
            {
                comboBoxSelectMouse->setItemData(index, true);
                comboBoxSelectMouse->setCurrentIndex(index);
            }
            else
            {
                comboBoxSelectMouse->setItemData(index, false);
            }
        }
    };

    auto on_showDataDetail = [&]() {
        QString detailA = getGroupDataDetail(m_groupedFilesStep1, m_groupedFilesPointStep1);

        // 创建对话框
        QDialog dialog;
        dialog.setWindowTitle("数据详情");
        dialog.resize(600, 400);

        QTextEdit *textEdit = new QTextEdit(&dialog);
        textEdit->setPlainText(detailA);

        QPushButton *closeButton = new QPushButton("知道了", &dialog);
        QObject::connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

        // 布局
        QHBoxLayout *textLayout = new QHBoxLayout();
        textLayout->addWidget(textEdit);
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QSpacerItem *horizontalSpacer = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
        buttonLayout->addItem(horizontalSpacer);
        buttonLayout->addWidget(closeButton);

        QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
        mainLayout->addLayout(textLayout);
        mainLayout->addLayout(buttonLayout);

        // 显示对话框
        dialog.exec();
    };


    connect(pushButtonDirectory, &QPushButton::clicked, this, on_selectDirectory);
    connect(spinBoxPointX1, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointX1Change);
    connect(spinBoxPointY1, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointY1Change);
    connect(spinBoxPointX2, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointX2Change);
    connect(spinBoxPointY2, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointY2Change);
    connect(comboBoxSelectMouse, &QComboBox::currentTextChanged, this, on_comboxSelectChange);
    connect(pushButtonPointTXTImport, &QPushButton::clicked, this, on_buttonTXTImport);
    connect(pushButtonDetail, &QPushButton::clicked, this, on_showDataDetail);

    // 数据选择框添加控件
    grid->addWidget(labelDirectory, 0, 0, 1, 1);
    grid->addLayout(hLayoutForDirectory, 0, 1, 1, 5);

    grid->addWidget(labelData, 1, 0, 1, 1);
    grid->addWidget(spinBoxMouseNum, 1, 1, 1, 2);
    grid->addWidget(spinBoxTifNum, 1, 3, 1, 2);
    grid->addWidget(pushButtonDetail, 1, 5, 1, 1);

    grid->addWidget(labelPoint, 2, 0, 1, 1);
    grid->addLayout(gridPoint, 2, 1, 1, 5);
    m_widgetDataSetting_1->setLayout(grid);
}


void MainWindow::initDataSettingPage2()
{
    QGridLayout *grid = new QGridLayout();

    // 对照组
    QLabel *labelCON = new QLabel(tr("对照组数据文件"));
    QLineEdit *lineEditCON = new QLineEdit();
    lineEditCON->setReadOnly(true);
    QPushButton *pushButtonSelectCON = new QPushButton(tr("选择"));

    // 突变组
    QLabel *labelMUT = new QLabel(tr("突变组数据文件"));
    QLineEdit *lineEditMUT = new QLineEdit();
    lineEditMUT->setReadOnly(true);
    QPushButton *pushButtonSelectMUT = new QPushButton(tr("选择"));

    // 频率范围选取
    QLabel *labelFrequency = new QLabel(tr("频率范围选取"));
    QDoubleSpinBox *spinBoxLeft = new QDoubleSpinBox();
    spinBoxLeft->setRange(0, 100);
    spinBoxLeft->setValue(0);
    spinBoxLeft->setDecimals(2);
    spinBoxLeft->setSingleStep(0.01);
    spinBoxLeft->setButtonSymbols(QSpinBox::NoButtons);
    QDoubleSpinBox *spinBoxRight = new QDoubleSpinBox();
    spinBoxRight->setRange(0, 100);
    spinBoxRight->setValue(13);
    spinBoxRight->setDecimals(2);
    spinBoxRight->setSingleStep(0.01);
    spinBoxRight->setButtonSymbols(QSpinBox::NoButtons);  

    QLabel *labelHzLeft = new QLabel("Hz       ~ ");
    QLabel *labelHzRight = new QLabel("Hz");

    QHBoxLayout *hLayoutForFrequency = new QHBoxLayout();
    hLayoutForFrequency->setAlignment(Qt::AlignLeft);
    hLayoutForFrequency->setContentsMargins(QMargins(0, 0, 0, 0));
    hLayoutForFrequency->setSpacing(0);
    hLayoutForFrequency->addWidget(spinBoxLeft);
    hLayoutForFrequency->addWidget(labelHzLeft);
    hLayoutForFrequency->addWidget(spinBoxRight);
    hLayoutForFrequency->addWidget(labelHzRight);

    // 设置默认值
    m_config[STEP2]["begin"] = spinBoxLeft->value();
    m_config[STEP2]["last"] = spinBoxRight->value();

    auto on_selectCONFile = [&, lineEditCON](){
        QString fullPath = QFileDialog::getOpenFileName(
            nullptr,
            tr("选择导入 mat 文件"),
            QCoreApplication::applicationDirPath(),
            "Text Files (*.mat);;All Files (*)"
            );

        if (fullPath.isEmpty())
        {
            // QMessageBox::critical(nullptr, "错误", QString("未选择文件！"));
            return;
        }

        lineEditCON->setText(fullPath);

        QMap<QString, QString> map;
        if(m_config[STEP2].contains("input_files"))  // 存在，以前写入过，更新
            map = m_config[STEP2]["input_files"].value<QMap<QString, QString>>();
        else                                         // 不存在，准备插入
            map.clear();

        map["CON"] = fullPath;
        m_config[STEP2]["input_files"] = QVariant::fromValue(map);
    };

    auto on_selectMUTFile = [&, lineEditMUT](){
        QString fullPath = QFileDialog::getOpenFileName(
            nullptr,
            tr("选择导入 mat 文件"),
            QCoreApplication::applicationDirPath(),
            "Text Files (*.mat);;All Files (*)"
            );

        if (fullPath.isEmpty())
        {
            // QMessageBox::critical(nullptr, "错误", QString("未选择文件！"));
            return;
        }

        lineEditMUT->setText(fullPath);

        QMap<QString, QString> map;
        if(m_config[STEP2].contains("input_files"))  // 存在，以前写入过，更新
            map = m_config[STEP2]["input_files"].value<QMap<QString, QString>>();
        else                                         // 不存在，准备插入
            map.clear();

        map["MUT"] = fullPath;
        m_config[STEP2]["input_files"] = QVariant::fromValue(map);
    };

    auto on_frequencyLeftChange = [&](double value){
        m_config[STEP2]["begin"] = value;
    };

    auto on_frequencyRightChange = [&](double value){
            m_config[STEP2]["last"] = value;
    };

    connect(pushButtonSelectCON, &QPushButton::clicked, this, on_selectCONFile);
    connect(pushButtonSelectMUT, &QPushButton::clicked, this, on_selectMUTFile);
    connect(spinBoxLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, on_frequencyLeftChange);
    connect(spinBoxRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, on_frequencyRightChange);

    grid->addWidget(labelCON, 0, 0);
    grid->addWidget(lineEditCON, 0, 1);
    grid->addWidget(pushButtonSelectCON, 0, 2);

    grid->addWidget(labelMUT, 1, 0);
    grid->addWidget(lineEditMUT, 1, 1);
    grid->addWidget(pushButtonSelectMUT, 1, 2);

    grid->addWidget(labelFrequency, 2, 0);
    grid->addLayout(hLayoutForFrequency, 2, 1);

    grid->setAlignment(Qt::AlignTop);
    m_widgetDataSetting_2->setLayout(grid);
}


void MainWindow::initDataSettingPage3()
{
    QGridLayout *grid = new QGridLayout();

    // 对照组
    QLabel *labelCON1 = new QLabel(tr("对照组数据文件"));
    QLineEdit *lineEditAliasCON1 = new QLineEdit();
    lineEditAliasCON1->setPlaceholderText("默认组名：Control 1");
    lineEditAliasCON1->setMaxLength(100);
    QLineEdit *lineEditCON1 = new QLineEdit();
    lineEditCON1->setReadOnly(true);
    lineEditCON1->setMinimumWidth(200);
    QPushButton *pushButtonSelectCON1 = new QPushButton(tr("选择"));

    // 突变组
    QLabel *labelMUT1 = new QLabel(tr("突变组数据文件"));
    QLineEdit *lineEditAliasMUT1 = new QLineEdit();
    lineEditAliasMUT1->setPlaceholderText("默认组名：Mutant 1");
    lineEditAliasMUT1->setMaxLength(100);
    QLineEdit *lineEditMUT1 = new QLineEdit();
    lineEditMUT1->setReadOnly(true);
    lineEditMUT1->setMinimumWidth(200);
    QPushButton *pushButtonSelectMUT1 = new QPushButton(tr("选择"));

    // 对照组
    QLabel *labelCON2 = new QLabel(tr("对照组2（可选）"));
    QLineEdit *lineEditAliasCON2 = new QLineEdit();
    lineEditAliasCON2->setPlaceholderText("默认组名：Control 2");
    lineEditAliasCON2->setMaxLength(100);
    QPushButton *pushButtonSelectCON2 = new QPushButton(tr("点击选择文件..."));
    pushButtonSelectCON2->setStyleSheet(QString("color:gray; text-align:left"));
    pushButtonSelectCON2->setMaximumWidth(150);

    // 突变组
    QLabel *labelMUT2 = new QLabel(tr("突变组2（可选）"));
    QLineEdit *lineEditAliasMUT2 = new QLineEdit();
    lineEditAliasMUT2->setPlaceholderText("默认组名：Mutant 2");
    lineEditAliasMUT2->setMaxLength(100);
    QPushButton *pushButtonSelectMUT2 = new QPushButton(tr("点击选择文件..."));
    pushButtonSelectMUT2->setStyleSheet(QString("color:gray; text-align:left"));
    pushButtonSelectMUT2->setMaximumWidth(150);

    // 频率范围选取
    QLabel *labelFrequency = new QLabel(tr("频率范围选取"));
    QDoubleSpinBox *spinBoxLeft = new QDoubleSpinBox();
    spinBoxLeft->setRange(0, 100);
    spinBoxLeft->setValue(0);
    spinBoxLeft->setDecimals(2);
    spinBoxLeft->setSingleStep(0.01);
    spinBoxLeft->setButtonSymbols(QSpinBox::NoButtons);
    QDoubleSpinBox *spinBoxRight = new QDoubleSpinBox();
    spinBoxRight->setRange(0, 100);
    spinBoxRight->setValue(13);
    spinBoxRight->setDecimals(2);
    spinBoxRight->setSingleStep(0.01);
    spinBoxRight->setButtonSymbols(QSpinBox::NoButtons);

    QLabel *labelHzLeft = new QLabel("Hz       ~ ");
    QLabel *labelHzRight = new QLabel("Hz");

    QHBoxLayout *hLayoutForFrequency = new QHBoxLayout();
    hLayoutForFrequency->setAlignment(Qt::AlignLeft);
    hLayoutForFrequency->setContentsMargins(QMargins(0, 0, 0, 0));
    hLayoutForFrequency->setSpacing(0);
    hLayoutForFrequency->addWidget(spinBoxLeft);
    hLayoutForFrequency->addWidget(labelHzLeft);
    hLayoutForFrequency->addWidget(spinBoxRight);
    hLayoutForFrequency->addWidget(labelHzRight);

    // 设置默认值
    m_config[STEP3]["begin"] = spinBoxLeft->value();
    m_config[STEP3]["last"] = spinBoxRight->value();
    QList<QString> stringList;
    stringList << "CON1" << "MUT1";
    m_config[STEP3]["selected_inputs"].setValue(stringList);

    QMap<QString, QString> map;
    map["CON1"] = "Control 1";
    map["CON2"] = "Control 2";
    map["MUT1"] = "Mutant 1";
    map["MUT2"] = "Mutant 2";
    m_config[STEP3]["group_names"] = QVariant::fromValue(map);

    auto on_selectCON1File = [&, lineEditCON1](){
        QString fullPath = QFileDialog::getOpenFileName(
            nullptr,
            tr("选择导入 mat 文件"),
            QCoreApplication::applicationDirPath(),
            "Text Files (*.mat);;All Files (*)"
            );

        if (fullPath.isEmpty())
            return;

        lineEditCON1->setText(fullPath);

        QMap<QString, QString> map;
        if(m_config[STEP3].contains("input_files"))  // 存在，以前写入过，更新
            map = m_config[STEP3]["input_files"].value<QMap<QString, QString>>();
        else                                         // 不存在，准备插入
            map.clear();

        map["CON1"] = fullPath;
        m_config[STEP3]["input_files"] = QVariant::fromValue(map);
    };


    auto on_selectMUT1File = [&, lineEditMUT1](){
        QString fullPath = QFileDialog::getOpenFileName(
            nullptr,
            tr("选择导入 mat 文件"),
            QCoreApplication::applicationDirPath(),
            "Text Files (*.mat);;All Files (*)"
            );

        if (fullPath.isEmpty())
            return;

        lineEditMUT1->setText(fullPath);

        QMap<QString, QString> map;
        if(m_config[STEP3].contains("input_files"))  // 存在，以前写入过，更新
            map = m_config[STEP3]["input_files"].value<QMap<QString, QString>>();
        else                                         // 不存在，准备插入
            map.clear();

        map["MUT1"] = fullPath;
        m_config[STEP3]["input_files"] = QVariant::fromValue(map);
    };

    auto on_selectCON2File = [&, pushButtonSelectCON2](){
        QString fullPath = QFileDialog::getOpenFileName(
            nullptr,
            tr("选择导入 mat 文件"),
            QCoreApplication::applicationDirPath(),
            "Text Files (*.mat);;All Files (*)"
            );

        if (fullPath.isEmpty())
            return;

        pushButtonSelectCON2->setText(tr("点击选择文件..."));

        QMap<QString, QString> map;
        if(m_config[STEP3].contains("input_files"))  // 存在，以前写入过，更新
            map = m_config[STEP3]["input_files"].value<QMap<QString, QString>>();
        else                                         // 不存在，准备插入
            map.clear();

        if(map.contains("CON1") && map.contains("MUT1"))
        {
            map["CON2"] = fullPath;
            m_config[STEP3]["input_files"] = QVariant::fromValue(map);
            QList<QString> stringList = m_config[STEP3]["selected_inputs"].value<QList<QString>>();
            if(!stringList.contains("CON2"))
                stringList.append("CON2");
            m_config[STEP3]["selected_inputs"].setValue(stringList);
            pushButtonSelectCON2->setText(fullPath);
        }
        else
        {
            QMessageBox::information(nullptr, "提示", "请先选择【对照组数据文件】【突变组数据文件】", QMessageBox::Ok);
            return;
        }
    };


    auto on_selectMUT2File = [&, pushButtonSelectMUT2](){
        QString fullPath = QFileDialog::getOpenFileName(
            nullptr,
            tr("选择导入 mat 文件"),
            QCoreApplication::applicationDirPath(),
            "Text Files (*.mat);;All Files (*)"
            );

        if (fullPath.isEmpty())
            return;

        pushButtonSelectMUT2->setText(tr("点击选择文件..."));

        QMap<QString, QString> map;
        if(m_config[STEP3].contains("input_files"))  // 存在，以前写入过，更新
            map = m_config[STEP3]["input_files"].value<QMap<QString, QString>>();
        else                                         // 不存在，准备插入
            map.clear();

        if(map.contains("CON1") && map.contains("MUT1") && map.contains("CON2"))
        {
            map["MUT2"] = fullPath;
            m_config[STEP3]["input_files"] = QVariant::fromValue(map);
            QList<QString> stringList = m_config[STEP3]["selected_inputs"].value<QList<QString>>();
            if(!stringList.contains("MUT2"))
                stringList.append("MUT2");
            m_config[STEP3]["selected_inputs"].setValue(stringList);
            pushButtonSelectMUT2->setText(fullPath);
        }
        else
        {
            QMessageBox::information(nullptr, "提示", "请先选择【对照组数据文件】【突变组数据文件】【对照组2（可选）】", QMessageBox::Ok);
            return;
        }
    };

    auto on_aliasCON1Changed = [&](const QString &text){
        QMap<QString, QString> map;
        if(m_config[STEP3].contains("group_names"))  // 存在，以前写入过，更新
            map = m_config[STEP3]["group_names"].value<QMap<QString, QString>>();
        else                                         // 不存在，准备插入
            map.clear();

        map["CON1"] = text;
        m_config[STEP3]["group_names"] = QVariant::fromValue(map);
    };

    auto on_aliasCON2Changed = [&](const QString &text){
        QMap<QString, QString> map;
        if(m_config[STEP3].contains("group_names"))  // 存在，以前写入过，更新
            map = m_config[STEP3]["group_names"].value<QMap<QString, QString>>();
        else                                         // 不存在，准备插入
            map.clear();

        map["CON2"] = text;
        m_config[STEP3]["group_names"] = QVariant::fromValue(map);
    };

    auto on_aliasMUT1Changed = [&](const QString &text){
        QMap<QString, QString> map;
        if(m_config[STEP3].contains("group_names"))  // 存在，以前写入过，更新
            map = m_config[STEP3]["group_names"].value<QMap<QString, QString>>();
        else                                         // 不存在，准备插入
            map.clear();

        map["MUT1"] = text;
        m_config[STEP3]["group_names"] = QVariant::fromValue(map);
    };

    auto on_aliasMUT2Changed = [&](const QString &text){
        QMap<QString, QString> map;
        if(m_config[STEP3].contains("group_names"))  // 存在，以前写入过，更新
            map = m_config[STEP3]["group_names"].value<QMap<QString, QString>>();
        else                                         // 不存在，准备插入
            map.clear();

        map["MUT2"] = text;
        m_config[STEP3]["group_names"] = QVariant::fromValue(map);
    };


    auto on_spinBoxLeftValueChanged = [&](double left) {
        m_config[STEP3]["begin"] = left;
    };

    auto on_spinBoxRightValueChanged = [&](double right) {
        m_config[STEP3]["last"] = right;
    };

    connect(spinBoxLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, on_spinBoxLeftValueChanged);
    connect(spinBoxRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, on_spinBoxRightValueChanged);

    connect(pushButtonSelectCON1, &QPushButton::clicked, this, on_selectCON1File);
    connect(pushButtonSelectMUT1, &QPushButton::clicked, this, on_selectMUT1File);
    connect(pushButtonSelectCON2, &QPushButton::clicked, this, on_selectCON2File);
    connect(pushButtonSelectMUT2, &QPushButton::clicked, this, on_selectMUT2File);

    connect(lineEditAliasCON1, &QLineEdit::textChanged, this, on_aliasCON1Changed);
    connect(lineEditAliasCON2, &QLineEdit::textChanged, this, on_aliasCON2Changed);
    connect(lineEditAliasMUT1, &QLineEdit::textChanged, this, on_aliasMUT1Changed);
    connect(lineEditAliasMUT2, &QLineEdit::textChanged, this, on_aliasMUT2Changed);


    grid->addWidget(labelCON1, 0, 0, 1, 1);
    grid->addWidget(lineEditAliasCON1, 0, 1, 1, 1);
    grid->addWidget(lineEditCON1, 0, 2, 1, 3);
    grid->addWidget(pushButtonSelectCON1, 0, 5, 1, 1);

    grid->addWidget(labelMUT1, 1, 0, 1, 1);
    grid->addWidget(lineEditAliasMUT1, 1, 1, 1, 1);
    grid->addWidget(lineEditMUT1, 1, 2, 1, 3);
    grid->addWidget(pushButtonSelectMUT1, 1, 5, 1, 1);

    grid->addWidget(labelFrequency, 3, 0, 1, 1);
    grid->addLayout(hLayoutForFrequency, 3, 1, 1, 5);

    grid->addWidget(labelCON2, 4, 0, 1, 1);
    grid->addWidget(lineEditAliasCON2, 4, 1, 1, 1);
    grid->addWidget(pushButtonSelectCON2, 4, 2, 1, 2);

    grid->addWidget(labelMUT2, 5, 0, 1, 1);
    grid->addWidget(lineEditAliasMUT2, 5, 1, 1, 1);
    grid->addWidget(pushButtonSelectMUT2, 5, 2, 1, 2);

    grid->setAlignment(Qt::AlignTop);
    m_widgetDataSetting_3->setLayout(grid);
}


void MainWindow::initDataSettingPage4()
{
    QGridLayout *grid = new QGridLayout();

    QLabel *labelTimeTif = new QLabel(tr("时序数据文件"));
    QLineEdit *lineEditDir = new QLineEdit();
    lineEditDir->setReadOnly(true);
    QPushButton *pushButtonSelectDir = new QPushButton(tr("选择"));

    QLabel *labelPoint = new QLabel(tr("配准点坐标"));
    // QLabel *labelPointX1 = new QLabel(tr("X1:"));
    // QLabel *labelPointX2 = new QLabel(tr("X2:"));
    // QLabel *labelPointY1 = new QLabel(tr("Y1:"));
    // QLabel *labelPointY2 = new QLabel(tr("Y2:"));
    // labelPointX1->setAlignment(Qt::AlignRight);
    // labelPointX1->setMargin(5);
    // labelPointX2->setAlignment(Qt::AlignRight);
    // labelPointX2->setMargin(5);
    // labelPointY1->setAlignment(Qt::AlignRight);
    // labelPointY1->setMargin(5);
    // labelPointY2->setAlignment(Qt::AlignRight);
    // labelPointY2->setMargin(5);
    QSpinBox *spinBoxPointX1 = new QSpinBox();
    QSpinBox *spinBoxPointX2 = new QSpinBox();
    QSpinBox *spinBoxPointY1 = new QSpinBox();
    QSpinBox *spinBoxPointY2 = new QSpinBox();
    spinBoxPointX1->setRange(0, 10000);
    spinBoxPointX2->setRange(0, 10000);
    spinBoxPointY1->setRange(0, 10000);
    spinBoxPointY2->setRange(0, 10000);
    spinBoxPointX1->setValue(0);
    spinBoxPointX2->setValue(0);
    spinBoxPointY1->setValue(0);
    spinBoxPointY2->setValue(0);
    spinBoxPointX1->setButtonSymbols(QSpinBox::NoButtons);
    spinBoxPointX2->setButtonSymbols(QSpinBox::NoButtons);
    spinBoxPointY1->setButtonSymbols(QSpinBox::NoButtons);
    spinBoxPointY2->setButtonSymbols(QSpinBox::NoButtons);

    spinBoxPointX1->setDisabled(true);
    spinBoxPointX2->setDisabled(true);
    spinBoxPointY1->setDisabled(true);
    spinBoxPointY2->setDisabled(true);

    auto on_selectDirectory = [&, lineEditDir, spinBoxPointX1, spinBoxPointX2, spinBoxPointY1, spinBoxPointY2](){
        QString directory = QFileDialog::getExistingDirectory(
            nullptr,
            tr("选择文件夹"),
            QCoreApplication::applicationDirPath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if(directory.isEmpty())
        {
            // QMessageBox::critical(nullptr, "错误", QString("未选择文件！"));
            return;
        }

        spinBoxPointX1->setDisabled(false);
        spinBoxPointX2->setDisabled(false);
        spinBoxPointY1->setDisabled(false);
        spinBoxPointY2->setDisabled(false);

        lineEditDir->setText(directory);
        QDir dir(directory);
        QFileInfoList fileList = dir.entryInfoList(QStringList() << "*.tif", QDir::Files);
        foreach (auto const& file, fileList)
        {
            m_groupedFilesStep4[dir.dirName()].append(file.absoluteFilePath());
            // m_groupedFilesStep4["1"].append(file.absoluteFilePath());
        }

        // 初始化配准点键
        for (auto it = m_groupedFilesStep4.constBegin(); it != m_groupedFilesStep4.constEnd(); ++it)
        {
            QVector<QVector<int>> points(2, QVector<int>(2, -1));
            m_groupedFilesPointStep4.insert(it.key(), points);
        }

        m_config[STEP4]["input_files"] = QVariant::fromValue(m_groupedFilesStep4);
    };

    auto on_pointX1Change = [&](const int x){
        auto it = m_groupedFilesStep4.constBegin();
        m_groupedFilesPointStep4[it.key()][0][0] = x;
    };

    auto on_pointY1Change = [&](const int x){
        auto it = m_groupedFilesStep4.constBegin();
        m_groupedFilesPointStep4[it.key()][0][1] = x;
    };

    auto on_pointX2Change = [&](const int x){
        auto it = m_groupedFilesStep4.constBegin();
        m_groupedFilesPointStep4[it.key()][1][0] = x;
    };

    auto on_pointY2Change = [&](const int x){
        auto it = m_groupedFilesStep4.constBegin();
        m_groupedFilesPointStep4[it.key()][1][1] = x;
    };

    connect(pushButtonSelectDir, &QPushButton::clicked, this, on_selectDirectory);
    connect(spinBoxPointX1, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointX1Change);
    connect(spinBoxPointY1, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointY1Change);
    connect(spinBoxPointX2, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointX2Change);
    connect(spinBoxPointY2, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointY2Change);

    grid->addWidget(labelTimeTif, 0, 0, 1, 1);
    grid->addWidget(lineEditDir, 0, 1, 1, 6);
    grid->addWidget(pushButtonSelectDir, 0, 7, 1, 2);

    grid->addWidget(labelPoint, 1, 0, 1, 1);

    // grid->addWidget(labelPointX1, 1, 1);
    // grid->addWidget(spinBoxPointX1, 1, 2);
    // grid->addWidget(labelPointY1, 1, 3);
    // grid->addWidget(spinBoxPointY1, 1, 4);
    // grid->addWidget(labelPointX2, 1, 5);
    // grid->addWidget(spinBoxPointX2, 1, 6);
    // grid->addWidget(labelPointY2, 1, 7);
    // grid->addWidget(spinBoxPointY2, 1, 8);

    grid->addWidget(new QLabel(tr("坐标1 (B): ")), 1, 1, 1, 1);
    QHBoxLayout *hLayoutForPoint1 = new QHBoxLayout();
    hLayoutForPoint1->addWidget(new QLabel(tr("(")), 1);
    hLayoutForPoint1->addWidget(spinBoxPointX1, 7);
    hLayoutForPoint1->addWidget(new QLabel(tr(",")), 1);
    hLayoutForPoint1->addWidget(spinBoxPointY1, 7);
    hLayoutForPoint1->addWidget(new QLabel(tr(")")));
    grid->addLayout(hLayoutForPoint1, 1, 2, 1, 4);

    grid->addWidget(new QLabel(tr("坐标2 (Λ): ")), 2, 1, 1, 1);
    QHBoxLayout *hLayoutForPoint2 = new QHBoxLayout();
    hLayoutForPoint2->addWidget(new QLabel(tr("(")), 1);
    hLayoutForPoint2->addWidget(spinBoxPointX2, 7);
    hLayoutForPoint2->addWidget(new QLabel(tr(",")), 1);
    hLayoutForPoint2->addWidget(spinBoxPointY2, 7);
    hLayoutForPoint2->addWidget(new QLabel(tr(")")));
    grid->addLayout(hLayoutForPoint2, 2, 2, 1, 4);


    grid->setAlignment(Qt::AlignTop);
    m_widgetDataSetting_4->setLayout(grid);
}


void MainWindow::initDataSettingPage5()
{
    QGridLayout *grid = new QGridLayout();

    QLabel *labelDirA = new QLabel(tr("组A数据文件"));
    QLineEdit *lineEditDirA = new QLineEdit();
    lineEditDirA->setReadOnly(true);
    QPushButton *pushButtonSelectDirA = new QPushButton(tr("选择"));
    QPushButton *pushButtonSelectPointA = new QPushButton(tr("坐标快速导入"));
    pushButtonSelectPointA->setToolTip(m_pointTxtImportHelp);

    QLabel *labelDirB = new QLabel(tr("组B数据文件"));
    QLineEdit *lineEditDirB = new QLineEdit();
    lineEditDirB->setReadOnly(true);
    QPushButton *pushButtonSelectDirB = new QPushButton(tr("选择"));
    QPushButton *pushButtonSelectPointB = new QPushButton(tr("坐标快速导入"));
    pushButtonSelectPointB->setToolTip(m_pointTxtImportHelp);

    QLabel *labelBar = new QLabel(tr("阈值设置"));
    DualSlider *dualSlider = new DualSlider();
    dualSlider->setRange(0, 100);

    QDoubleSpinBox *spinBoxLeft = new QDoubleSpinBox();
    spinBoxLeft->setRange(0, 1);
    spinBoxLeft->setValue(0);
    spinBoxLeft->setDecimals(2);
    spinBoxLeft->setSingleStep(0.05);
    spinBoxLeft->setButtonSymbols(QSpinBox::NoButtons);
    QDoubleSpinBox *spinBoxRight = new QDoubleSpinBox();
    spinBoxRight->setRange(0, 1);
    spinBoxRight->setValue(1);
    spinBoxRight->setDecimals(2);
    spinBoxRight->setSingleStep(0.05);
    spinBoxRight->setButtonSymbols(QSpinBox::NoButtons);

    // 设置默认值
    m_config[STEP5]["bar_begin"] = spinBoxLeft->value();
    m_config[STEP5]["bar_end"] = spinBoxRight->value();

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(dualSlider, 7);
    hLayout->addWidget(spinBoxLeft, 2);
    // QLabel *splitLabel = new QLabel("123456789");
    // hLayout->addWidget(splitLabel, 2);
    hLayout->addWidget(spinBoxRight, 2);

    QLabel *labelMore = new QLabel(tr("更多设置"));
    QPushButton *pushButtonDetail = new QPushButton(tr("数据详情"));
    // QPushButton *pushButtonAdvance = new QPushButton(tr("高级设置"));

    auto on_selectDirA = [&, lineEditDirA](){
        QString directory = QFileDialog::getExistingDirectory(
            nullptr,
            "选择数据目录",
            QCoreApplication::applicationDirPath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        qDebug() << "Selected directory:" << directory;
        if (directory.isEmpty())
        {
            qDebug() << "No directory selected.";
            return;
        }

        // 清除原先内容防止累加
        m_groupedFilesStep5A.clear();

        // 只读tif文件
        QDir dir(directory);
        QFileInfoList fileList = dir.entryInfoList(QStringList() << "*.tif", QDir::Files);

        if(fileList.length() == 0)
        {
            qDebug() << "Directory has no files.";
            lineEditDirA->setText(tr("文件夹为空！"));
            return;
        }

        lineEditDirA->setText(directory);

        // 获取分组文件并保存至config
        bool ok = getGroupFiles(fileList, m_groupedFilesStep5A);
        if(!ok) return;

        QMap<QString, QVariant> groupFiles;
        m_groupedFilesStep5["groupA"] = QVariant::fromValue(m_groupedFilesStep5A);
        m_config[STEP5]["input_files"] = QVariant::fromValue(m_groupedFilesStep5);

        // 初始化配准坐标
        m_groupedFilesPointStep5A.clear();
        for (auto it = m_groupedFilesStep5A.constBegin(); it != m_groupedFilesStep5A.constEnd(); ++it) {
            QVector<QVector<int>> points(2, QVector<int>(2, -1));
            m_groupedFilesPointStep5A.insert(it.key(), points);
        }
    };

    auto on_selectDirB = [&, lineEditDirB](){
        QString directory = QFileDialog::getExistingDirectory(
            nullptr,
            "选择数据目录",
            QCoreApplication::applicationDirPath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        qDebug() << "Selected directory:" << directory;
        if (directory.isEmpty())
        {
            qDebug() << "No directory selected.";
            return;
        }

        // 清除原先内容防止累加
        m_groupedFilesStep5B.clear();

        // 只读tif文件
        QDir dir(directory);
        QFileInfoList fileList = dir.entryInfoList(QStringList() << "*.tif", QDir::Files);

        if(fileList.length() == 0)
        {
            qDebug() << "Directory has no files.";
            lineEditDirB->setText(tr("文件夹为空！"));
            return;
        }

        lineEditDirB->setText(directory);

        // 获取分组文件并保存至config
        bool ok = getGroupFiles(fileList, m_groupedFilesStep5B);
        if(!ok) return;

        m_groupedFilesStep5["groupB"] = QVariant::fromValue(m_groupedFilesStep5B);
        m_config[STEP5]["input_files"] = QVariant::fromValue(m_groupedFilesStep5);

        // 初始化配准坐标
        m_groupedFilesPointStep5B.clear();
        for (auto it = m_groupedFilesStep5B.constBegin(); it != m_groupedFilesStep5B.constEnd(); ++it) {
            QVector<QVector<int>> points(2, QVector<int>(2, -1));
            m_groupedFilesPointStep5B.insert(it.key(), points);
        }
    };

    auto on_selectPointA = [&]() {
        QString fullPath = QFileDialog::getOpenFileName(
            nullptr,
            tr("选择导入TXT配准坐标"),
            QCoreApplication::applicationDirPath(),
            "Text Files (*.txt);;All Files (*)"
            );

        if (fullPath.isEmpty())
            return;

        // 获取分组文件
        bool ok = getGroupFilesPoints(m_csvParser, fullPath, m_groupedFilesPointStep5A);
        if(!ok) return;
        m_groupedFilesPointStep5["groupA"] =  QVariant::fromValue(m_groupedFilesPointStep5A);
    };

    auto on_selectPointB = [&]() {
        QString fullPath = QFileDialog::getOpenFileName(
            nullptr,
            tr("选择导入TXT配准坐标"),
            QCoreApplication::applicationDirPath(),
            "Text Files (*.txt);;All Files (*)"
            );

        if (fullPath.isEmpty())
            return;

        // 获取分组文件并保存至config
        bool ok = getGroupFilesPoints(m_csvParser, fullPath, m_groupedFilesPointStep5B);
        if(!ok) return;
        m_groupedFilesPointStep5["groupB"] =  QVariant::fromValue(m_groupedFilesPointStep5B);
    };

    auto on_slideValueChanged = [&, spinBoxLeft, spinBoxRight](int left, int right) {
        spinBoxLeft->setValue(left/100.0);
        spinBoxRight->setValue(right/100.0);

        m_config[STEP5]["bar_begin"] = spinBoxLeft->value();
        m_config[STEP5]["bar_end"] = spinBoxRight->value();
        // qDebug("Left: %d, Right: %d", left, right);
    };

    auto on_spinBoxLeftValueChanged = [&, dualSlider](double left) {
        dualSlider->setLeftValue(left*100);
    };

    auto on_spinBoxRightValueChanged = [&, dualSlider](double right) {
        dualSlider->setRightValue(right*100);
    };

    auto on_showDataDetail = [&]() {
        QString detailA = getGroupDataDetail(m_groupedFilesStep5A, m_groupedFilesPointStep5A);
        QString detailB = getGroupDataDetail(m_groupedFilesStep5B, m_groupedFilesPointStep5B);
        detailA.prepend("组别A：\n");
        detailB.prepend("组别B：\n");

        // 创建对话框
        QDialog dialog;
        dialog.setWindowTitle("数据详情");
        dialog.resize(600, 400);

        // 创建左右两个文本框
        QTextEdit *leftTextEdit = new QTextEdit(&dialog);
        QTextEdit *rightTextEdit = new QTextEdit(&dialog);

        // 设置只读
        leftTextEdit->setReadOnly(true);
        rightTextEdit->setReadOnly(true);

        // 填充一些示例文本
        leftTextEdit->setPlainText(detailA);
        rightTextEdit->setPlainText(detailB);

        // 同步滚动条
        QObject::connect(leftTextEdit->verticalScrollBar(), &QScrollBar::valueChanged,
                         rightTextEdit->verticalScrollBar(), &QScrollBar::setValue);
        QObject::connect(rightTextEdit->verticalScrollBar(), &QScrollBar::valueChanged,
                         leftTextEdit->verticalScrollBar(), &QScrollBar::setValue);

        QPushButton *closeButton = new QPushButton("知道了", &dialog);
        QObject::connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

        // 布局
        QHBoxLayout *textLayout = new QHBoxLayout();
        textLayout->addWidget(leftTextEdit);
        textLayout->addWidget(rightTextEdit);
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QSpacerItem *horizontalSpacer = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
        buttonLayout->addItem(horizontalSpacer);
        buttonLayout->addWidget(closeButton);

        QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
        mainLayout->addLayout(textLayout);
        mainLayout->addLayout(buttonLayout);

        // 显示对话框
        dialog.exec();
    };

    connect(pushButtonSelectDirA, &QPushButton::clicked, this, on_selectDirA);
    connect(pushButtonSelectDirB, &QPushButton::clicked, this, on_selectDirB);
    connect(pushButtonSelectPointA, &QPushButton::clicked, this, on_selectPointA);
    connect(pushButtonSelectPointB, &QPushButton::clicked, this, on_selectPointB);
    connect(pushButtonDetail, &QPushButton::clicked, this, on_showDataDetail);
    connect(dualSlider, &DualSlider::valuesChanged, this, on_slideValueChanged);
    connect(spinBoxLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, on_spinBoxLeftValueChanged);
    connect(spinBoxRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, on_spinBoxRightValueChanged);

    grid->addWidget(labelDirA, 0, 0);
    grid->addWidget(lineEditDirA, 0, 1);
    grid->addWidget(pushButtonSelectDirA, 0, 2);
    grid->addWidget(pushButtonSelectPointA, 0, 3);

    grid->addWidget(labelDirB, 2, 0);
    grid->addWidget(lineEditDirB, 2, 1);
    grid->addWidget(pushButtonSelectDirB, 2, 2);
    grid->addWidget(pushButtonSelectPointB, 2, 3);

    grid->addWidget(labelBar, 3, 0);
    grid->addLayout(hLayout, 3, 1, 1, 3);

    grid->addWidget(labelMore, 4, 0);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(pushButtonDetail);
    QSpacerItem *horizontalSpacer = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
    layout->addItem(horizontalSpacer);
    // layout->addWidget(pushButtonAdvance);
    grid->addLayout(layout, 4, 1, 1, 1);
    // grid->addWidget(pushButtonDetail, 4, 1);
    // grid->addWidget(pushButtonAdvance, 4, 2);

    grid->setAlignment(Qt::AlignTop);
    m_widgetDataSetting_5->setLayout(grid);
}


void MainWindow::initDataSettingPage6()
{
    QGridLayout *grid = new QGridLayout();

    QLabel *labelTif = new QLabel(tr("数据文件夹"));
    QLineEdit *lineEditTif = new QLineEdit();
    lineEditTif->setReadOnly(true);
    QPushButton *pushButtonSelectTif = new QPushButton(tr("选择"));

    auto on_selectTif = [&, lineEditTif](){    
        QString directory = QFileDialog::getExistingDirectory(
            nullptr,
            "选择数据目录",
            QCoreApplication::applicationDirPath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        qDebug() << "Selected directory:" << directory;
        if (directory.isEmpty())
        {
            qDebug() << "No directory selected.";
            return;
        }

        // 只读tif文件
        QDir dir(directory);
        QFileInfoList fileList = dir.entryInfoList(QStringList() << "*.tif", QDir::Files);

        if(fileList.length() == 0)
        {
            qDebug() << "Directory has no files.";
            lineEditTif->setText(tr("文件夹为空！"));
            return;
        }

        lineEditTif->setText(directory);

        QList<QString> tifPathList;
        foreach (auto file, fileList) {
            tifPathList.append(file.absoluteFilePath());
        }


        QMap<QString, QList<QString>> map;
        if(m_config[STEP6].contains("input_files"))  // 存在，以前写入过，更新
            map = m_config[STEP6]["input_files"].value<QMap<QString, QList<QString>>>();
        else                                         // 不存在，准备插入
            map.clear();

        map["mouse"] = tifPathList;
        m_config[STEP6]["input_files"] = QVariant::fromValue(map);
    };

    connect(pushButtonSelectTif, &QPushButton::clicked, this, on_selectTif);

    grid->addWidget(labelTif, 0, 0);
    grid->addWidget(lineEditTif, 0, 1);
    grid->addWidget(pushButtonSelectTif, 0, 2);

    grid->setAlignment(Qt::AlignTop);
    m_widgetDataSetting_6->setLayout(grid);
}




void MainWindow::on_widgetContainerStep_1_clicked()
{
    slotStepChange(Step::STEP1);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    ui->pushButtonImgTab->click();  // 默认显示图片分页
    ui->pushButtonRun->raise();
}


void MainWindow::on_widgetContainerStep_2_clicked()
{
    slotStepChange(Step::STEP2);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    ui->pushButtonImgTab->click();  // 默认显示图片分页
    ui->pushButtonRun->raise();
}


void MainWindow::on_widgetContainerStep_3_clicked()
{
    slotStepChange(Step::STEP3);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    ui->pushButtonImgTab->click();  // 默认显示图片分页
    ui->pushButtonRun->raise();
}


void MainWindow::on_widgetContainerStep_4_clicked()
{
    slotStepChange(Step::STEP4);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    ui->pushButtonImgTab->click();  // 默认显示图片分页
    ui->pushButtonRun->raise();
}


void MainWindow::on_widgetContainerStep_5_clicked()
{
    slotStepChange(Step::STEP5);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    ui->pushButtonImgTab->click();  // 默认显示图片分页
    ui->pushButtonRun->raise();
}


void MainWindow::on_widgetContainerStep_6_clicked()
{
    slotStepChange(Step::STEP6);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    ui->pushButtonImgTab->click();  // 默认显示图片分页
    ui->pushButtonRun->raise();
}


void MainWindow::on_processStart()
{
    qDebug() << QString("Process isRunning:(%1)").arg(m_cmdProcessList[m_currentDisplay]->isRunning());
    if(m_cmdProcessList[m_currentDisplay]->isRunning())  // 正在跑,此时功能是终止
    {
        emit cmdTerminate();
        qDebug() << "Send terminate singal to process";
        ui->pushButtonRun->setText(tr("暂停中..."));
        ui->pushButtonRun->setDisabled(true);
        return;
    }
    // 没有跑,此时功能是开始,开始下面的检查输入

    switch (m_currentDisplay)
    {
        case Step::STEP1:  // 检查 mouse_count files_per_mouse input_files point output_directory 是否已经设置
        {
            if(!m_config[STEP1].contains("input_files"))
            {
                QMessageBox::warning(nullptr, "错误", "未选择 tif 数据文件！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }
            if(!m_config[STEP1].contains("input_file_directory"))
            {
                QMessageBox::warning(nullptr, "错误", "未设置小鼠TIF文件路径！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }
            if(!m_config[STEP1].contains("mouse_count"))
            {
                QMessageBox::warning(nullptr, "错误", "未设置小鼠数量！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }
            else if(m_config[STEP1]["mouse_count"] == 0)
            {
                QMessageBox::warning(nullptr, "错误", "文件夹内没有小鼠！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }
            if(!m_config[STEP1].contains("files_per_mouse"))
            {
                QMessageBox::warning(nullptr, "错误", "未设置每只小鼠TIF文件数量！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }
            else if(m_config[STEP1]["files_per_mouse"] == 0)
            {
                QMessageBox::warning(nullptr, "错误", "文件夹内没有小鼠！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }
            bool ok;
            ok = isGroupFilesCompleted(m_groupedFilesStep1);
            if(!ok) return;

            ok = isGroupFilesPointsCompleted(m_groupedFilesPointStep1);
            if(!ok) return;

            m_config[STEP1]["point"] = QVariant::fromValue(m_groupedFilesPointStep1);

            m_configSaver.saveConfig(m_config, m_configSavePath);

            emit cmdStartRun();
            ui->pushButtonRun->setText(tr("暂停"));
            ui->progressBar->setVisible(true);
            ui->progressBar->setValue(0);
            break;
        }

        case Step::STEP2:
        {
            if(!m_config[STEP2].contains("input_files"))
            {
                QMessageBox::warning(nullptr, "错误", "未选择 mat 文件！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }

            QMap<QString, QString> map = m_config[STEP2]["input_files"].value<QMap<QString, QString>>();
            if(!map.contains("CON"))
            {
                QMessageBox::warning(nullptr, "错误", "未选择对照组 mat 文件！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }
            if(!map.contains("MUT"))
            {
                QMessageBox::warning(nullptr, "错误", "未选择突变组 mat 文件！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }

            m_configSaver.saveConfig(m_config, m_configSavePath);

            emit cmdStartRun();
            ui->pushButtonRun->setText(tr("暂停"));
            ui->progressBar->setVisible(true);
            ui->progressBar->setValue(0);
            break;
        }

        case Step::STEP3:
        {
            if(!m_config[STEP3].contains("input_files"))
            {
                QMessageBox::warning(nullptr, "错误", "未选择 mat 文件！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }

            QMap<QString, QString> map = m_config[STEP3]["input_files"].value<QMap<QString, QString>>();
            if(!map.contains("CON1"))
            {
                QMessageBox::warning(nullptr, "错误", "未选择对照组 mat 文件！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }
            if(!map.contains("MUT1"))
            {
                QMessageBox::warning(nullptr, "错误", "未选择突变组 mat 文件！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }

            m_configSaver.saveConfig(m_config, m_configSavePath);

            emit cmdStartRun();
            ui->pushButtonRun->setText(tr("暂停"));
            ui->progressBar->setVisible(true);
            ui->progressBar->setValue(0);
            break;
        }

        case Step::STEP4:
        {
            if(!m_config[STEP4].contains("input_files"))
            {
                QMessageBox::warning(nullptr, "错误", "未选择小鼠时间段文件夹！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }

            bool ok = isGroupFilesPointsCompleted(m_groupedFilesPointStep4);
            if(!ok) return;

            m_config[STEP4]["point"] = QVariant::fromValue(m_groupedFilesPointStep4);
            m_configSaver.saveConfig(m_config, m_configSavePath);

            emit cmdStartRun();
            ui->pushButtonRun->setText(tr("暂停"));
            ui->progressBar->setVisible(true);
            ui->progressBar->setValue(0);
            break;
        }

        case Step::STEP5:
        {
            if(!m_config[STEP5].contains("input_files"))
            {
                QMessageBox::warning(nullptr, "错误", "未选择文件！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }

            // 检查 A B 组数据是否已经规范准备好
            bool ok;
            ok = isGroupFilesCompleted(m_groupedFilesStep5A);
            if(!ok) return;
            ok = isGroupFilesCompleted(m_groupedFilesStep5B);
            if(!ok) return;

            ok = isGroupFilesPointsCompleted(m_groupedFilesPointStep5A);
            if(!ok) return;
            ok = isGroupFilesPointsCompleted(m_groupedFilesPointStep5B);
            if(!ok) return;


            m_config[STEP5]["point"] = QVariant::fromValue(m_groupedFilesPointStep5);
            m_configSaver.saveConfig(m_config, m_configSavePath);

            emit cmdStartRun();
            ui->pushButtonRun->setText(tr("暂停"));
            ui->progressBar->setVisible(true);
            ui->progressBar->setValue(0);
            break;
        }

        case Step::STEP6:
        {
            if(!m_config[STEP6].contains("input_files"))
            {
                QMessageBox::warning(nullptr, "错误", "未选择文件！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }

            if(m_config[STEP6]["input_files"].canConvert<QMap<QString, QList<QString>>>())
            {
                QMap<QString, QList<QString>> map = m_config[STEP6]["input_files"].value<QMap<QString, QList<QString>>>();
                if(map.contains("mouse"))
                {
                    if(map["mouse"].length() < 2)
                    {
                        QMessageBox::warning(nullptr, "错误", QString("至少输入该小鼠的 3 个 TIF 文件，获取输入【%1】个").arg(map["mouse"].length()), QMessageBox::Ok,QMessageBox::Ok);
                        return;
                    }
                }
                else // 没有 mouse 键，报错
                {
                    QMessageBox::warning(
                        nullptr,
                        "错误",
                        QString("配置文件错误！\n%1\n")
                            .arg("m_config[STEP6][\"input_files\"].contains(\"mouse\") FALSE",
                                 "\nm_config[STEP6][\"input_files\"]:\n",
                                 m_config[STEP6]["input_files"].toString()),
                        QMessageBox::Ok,QMessageBox::Ok
                    );
                    return;
                }
            }
            else
            {
                QMessageBox::warning(
                    nullptr,
                    "错误",
                    QString("配置文件错误！\n%1\n")
                        .arg("m_config[STEP6][\"input_files\"].canConvert<QMap<QString, QList<QString>>> FALSE",
                             "\nm_config[STEP6][\"input_files\"]:\n",
                             m_config[STEP6]["input_files"].toString()),
                    QMessageBox::Ok,QMessageBox::Ok
                );
                return;
            }

            m_configSaver.saveConfig(m_config, m_configSavePath);

            emit cmdStartRun();
            ui->pushButtonRun->setText(tr("暂停"));
            ui->progressBar->setVisible(true);
            ui->progressBar->setValue(0);
            break;
        }
    }
}



void MainWindow::on_updateImageList()
{
    QString resultPath = m_resultPath[m_currentDisplay];
    qDebug() << "cmdFinish, updateImageList" << resultPath;

    // 默认显示图片列表
    ui->pushButtonImgTab->click();
}



void MainWindow::paintEvent(QPaintEvent *event)
{

}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    // if (pixmapTitleLogo != nullptr)
    // {
    //     qDebug() << ui->labelLogoText->size();
    //     QPixmap scaledPixmap = pixmapTitleLogo->scaled(ui->labelLogoText->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    //     ui->labelLogoText->setPixmap(scaledPixmap);
    // }

    // 更新开始运行按钮位置，因为他是浮动显示的
    QRect settingRect = ui->groupBoxData->geometry();
    QSize btnSize = ui->pushButtonRun->size();
    QRect btnRect;
    int padding = 5;
    btnRect.setLeft(settingRect.width() - btnSize.width() - padding*2 + settingRect.left() );
    btnRect.setTop(settingRect.height() - btnSize.height() - padding*2 + settingRect.top() );
    btnRect.setWidth(btnSize.width() + padding*2);
    btnRect.setHeight(btnSize.height() + padding*2);
    ui->pushButtonRun->setGeometry(btnRect);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    on_exitAction();   // 如果退出在这个里面就退出了
    event->ignore();   // 还是再想想不退出吧
}



