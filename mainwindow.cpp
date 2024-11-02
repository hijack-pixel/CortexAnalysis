#pragma execution_character_set("utf-8")
#include "mainwindow.h"
#include "ui_mainwindow.h"

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



// 静态static成员必需在类外初始化，类的静态成员变量需要在类外分配内存空间
Step MainWindow::m_currentDisplay = Step::STEP1;
// ！！！！！！！！！！！！！！！必须在cpp中初始化，h中初始化报错！！！！

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // this->setWindowFlags(windowFlags()  | Qt::FramelessWindowHint);//无边框
    // this->setAttribute(Qt::WA_TranslucentBackground, true);//窗体背景全透明

    m_config[STEP1]["module"] = "Registration2ConnectMatrix";
    m_config[STEP2]["module"] = "cMatrix2NetGraph";
    m_config[STEP3]["module"] = "Timecourse2spectrum";
    m_config[STEP4]["module"] = "TimeCorrMap";
    m_config[STEP5]["module"] = "Quant4SNR";
    m_config[STEP6]["module"] = "SpatialCorrMap";

    m_config[STEP1]["output_directory"] = "step1";
    m_config[STEP2]["output_directory"] = "step2";
    m_config[STEP3]["output_directory"] = "step3";
    m_config[STEP4]["output_directory"] = "step4";
    m_config[STEP5]["output_directory"] = "step5";
    m_config[STEP6]["output_directory"] = "step6";


    // 状态栏设置
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


    addNewThread("octave", {"step1.m"}, "E:\\CODE\\Qt\\MouseAnalysis\\build\\Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\\data");
    addNewThread("octave", {"step2.m"}, "E:\\CODE\\Qt\\MouseAnalysis\\build\\Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\\data");
    addNewThread("octave", {"step3.m"}, "E:\\CODE\\Qt\\MouseAnalysis\\build\\Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\\data");
    addNewThread("octave", {"time_consume4.m"}, "E:\\CODE\\Qt\\MouseAnalysis-Octave");
    addNewThread("octave", {"time_consume5.m"}, "E:\\CODE\\Qt\\MouseAnalysis-Octave");
    addNewThread("octave", {"time_consume6.m"}, "E:\\CODE\\Qt\\MouseAnalysis-Octave");


    // 开始按钮槽函数连接
    connect(ui->pushButtonRun, &QPushButton::clicked, this, &MainWindow::on_processStart);

    // 初始化图片列表
    m_imageList = new ImageList();
    ui->verticalLayoutImgList->addWidget(m_imageList);
    connect(m_imageList, &ImageList::itemCurrent, this, [&](const QString& path){
        ui->graphicsView->setImgByPath(path);
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

    initDataSettingPage1();
    initDataSettingPage2();
    initDataSettingPage3();
    initDataSettingPage4();
    initDataSettingPage5();
    initDataSettingPage6();


    // ui->graphicsView->setImgByPath(QDir::cleanPath(QCoreApplication::applicationDirPath()+"/data/step1/Registration2ConnectMatrix_Connectivity matrix_count1.png"));

    // 初始化显示第一个步骤
    on_widgetContainerStep_1_clicked();

    // QString filePath = "E:\\CODE\\Qt\\MouseAnalysis\\build\\Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\\debug\\data\\points.txt"; // 替换为实际文件路径
    // if (m_csvParser.parse(filePath))
    // {
    //     const QVector<QStringList>& csvData = m_csvParser.getData();
    //     for (const QStringList& row : csvData)
    //     {
    //         qDebug() << row;
    //     }
    // }
}


MainWindow::~MainWindow()
{
    delete ui;

    // 先删除CommandProcess对象
    foreach (const CommandProcess* constCmdProcess, m_cmdProcessList) {
        constCmdProcess->terminate();

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
    delete m_imageList;
}


void MainWindow::paintEvent(QPaintEvent *event)
{

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

void MainWindow::on_runButtonEndFinish()
{
    if(!m_cmdProcessList[m_currentDisplay]->isRunning())
    {
        ui->pushButtonRun->setText(tr("开始"));
        qDebug() << "Cmd Finish, change button state: ";
    }
}

void MainWindow::on_runButtonEndError(QProcess::ProcessError err)
{
    if(!m_cmdProcessList[m_currentDisplay]->isRunning())
    {
        ui->pushButtonRun->setText(tr("开始"));
        qDebug() << "Cmd Error, change button state: " ;
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
    disconnect(this, &MainWindow::cmdTerminate, m_cmdProcessList[preDisplay], &CommandProcess::terminate);
    connect(this, &MainWindow::cmdTerminate, m_cmdProcessList[m_currentDisplay], &CommandProcess::terminate);

    // 开始运行按钮槽函数的解绑与绑定,页面切换时初始化文字
    disconnect(m_cmdProcessList[preDisplay], &CommandProcess::cmdFinish, this, &MainWindow::on_runButtonEndFinish);
    connect(m_cmdProcessList[m_currentDisplay], &CommandProcess::cmdFinish, this, &MainWindow::on_runButtonEndFinish);

    disconnect(m_cmdProcessList[preDisplay], &CommandProcess::cmdError, this, &MainWindow::on_runButtonEndError);
    connect(m_cmdProcessList[m_currentDisplay], &CommandProcess::cmdError, this, &MainWindow::on_runButtonEndError);

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

    ui->labelList->setText(QString("Previous: %1  Current: %2").arg(preDisplay).arg(m_currentDisplay));

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
    }
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


    // 小鼠数量展示
    QLabel *labelMouseNum = new QLabel(tr("小鼠数量"));
    QSpinBox *spinBoxMouseNum = new QSpinBox();
    spinBoxMouseNum->setRange(1, 1000);
    spinBoxMouseNum->setReadOnly(true);
    // spinBoxMouseNum->setValue(10);


    // 每只小鼠 tif 文件数量展示
    QLabel *labelTifNum = new QLabel(tr("每只小鼠文件数量"));
    QSpinBox *spinBoxTifNum = new QSpinBox();
    spinBoxTifNum->setRange(1, 1000);
    spinBoxTifNum->setReadOnly(true);
    // spinBoxTifNum->setValue(3);


    // 每只小鼠的配准坐标
    QHBoxLayout *hLayoutForPoint = new QHBoxLayout();

    QLabel *labelPoint = new QLabel(tr("配准坐标"));

    QPushButton *pushButtonPointTXTImport = new QPushButton(tr("TXT 快速导入"));
    pushButtonPointTXTImport->setToolTip(tr(
        "一行对应一只个小鼠的坐标，以英文逗号隔开，行尾不要有标点符号。\n"
        "格式：小鼠文件前缀名,X1,Y1,X2,Y2\n"
        "示例（下面展示了 3 只小鼠的配准坐标）：\n"
        "1-CON F 1,122,100,126,228\n"
        "1-CON F 2,122,112,132,226\n"
        "2-CON F 1,126,88,130,212\n"));

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

    QLabel *labelPonintY1 = new QLabel(tr("Y1:"));
    labelPonintY1->setAlignment(Qt::AlignRight);
    labelPonintY1->setMargin(5);
    QSpinBox *spinBoxPointY1 = new QSpinBox();
    spinBoxPointY1->setRange(0, 10000);
    spinBoxPointY1->setValue(0);

    QLabel *labelPonintX2 = new QLabel(tr("X2:"));
    labelPonintX2->setAlignment(Qt::AlignRight);
    labelPonintX2->setMargin(5);
    QSpinBox *spinBoxPointX2 = new QSpinBox();
    spinBoxPointX2->setRange(0, 10000);
    spinBoxPointX2->setValue(0);

    QLabel *labelPonintY2 = new QLabel(tr("Y2:"));
    labelPonintY2->setAlignment(Qt::AlignRight);
    labelPonintY2->setMargin(5);
    QSpinBox *spinBoxPointY2 = new QSpinBox();
    spinBoxPointY2->setRange(0, 10000);
    spinBoxPointY2->setValue(0);

    // 添加坐标输入框
    QGridLayout *gridPoint = new QGridLayout();
    gridPoint->addWidget(comboBoxSelectMouse, 0, 0, 1, 6);
    gridPoint->addWidget(pushButtonPointTXTImport, 0, 6, 1, 3);

    gridPoint->addWidget(labelPonintX1, 1, 0, 1, 1);
    gridPoint->addWidget(spinBoxPointX1, 1, 1, 1, 1);

    gridPoint->addWidget(labelPonintY1, 1, 2, 1, 1);
    gridPoint->addWidget(spinBoxPointY1, 1, 3, 1, 1);

    gridPoint->addWidget(labelPonintX2, 1, 4, 1, 1);
    gridPoint->addWidget(spinBoxPointX2, 1, 5, 1, 1);

    gridPoint->addWidget(labelPonintY2, 1, 6, 1, 1);
    gridPoint->addWidget(spinBoxPointY2, 1, 7, 1, 1);

    hLayoutForPoint->addLayout(gridPoint);


    comboBoxSelectMouse->setDisabled(true);
    spinBoxMouseNum->setDisabled(true);
    spinBoxTifNum->setDisabled(true);
    spinBoxPointX1->setDisabled(true);
    spinBoxPointX2->setDisabled(true);
    spinBoxPointY1->setDisabled(true);
    spinBoxPointY2->setDisabled(true);
    pushButtonPointTXTImport->setDisabled(true);

    // QPushButton *pushButtonProSelect = new QPushButton(tr("高级选择"));
    // pushButtonProSelect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // grid->addWidget(pushButtonProSelect, 5, 1, 1, 2);

    // 选择文件目录，按照前缀来自动识别文件并分组
    auto on_selectDirectory = [&, comboBoxSelectMouse, lineEditDirectory, spinBoxMouseNum, spinBoxTifNum, spinBoxPointX1, spinBoxPointX2, spinBoxPointY1, spinBoxPointY2, pushButtonPointTXTImport](){
        QString directory = QFileDialog::getExistingDirectory(
            nullptr,
            "Select Directory",
            "E:\\CODE\\Qt\\MouseAnalysis-Octave\\Control",
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (!directory.isEmpty())
        {
            qDebug() << "Selected directory:" << directory;

            // 只读tif文件
            QDir dir(directory);
            QFileInfoList fileList = dir.entryInfoList(QStringList() << "*.tif", QDir::Files);
            QList<QString> fileNameList;

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

            // 获取不带扩展名的文件名
            foreach (const QFileInfo &fileInfo, fileList)
            {
                fileNameList.append(fileInfo.baseName());
            }

            // 文件名排序
            fileNameList.sort();

            // 文件名分组
            foreach(const auto fileName, fileNameList)
            {
                if(m_groupedFiles.contains(fileName))  // 前缀键存在，跳过此次
                    continue;

                foreach(const auto f, fileNameList)  // 寻找相同的前缀加入groupedFiles
                {
                    if(f.startsWith(fileName) && !m_groupedFiles[fileName].contains(f))
                    {
                        m_groupedFiles[fileName].append(f);
                    }
                }
            }

            // 文件名分组中的文件名修改为绝对路径
            foreach (auto key, m_groupedFiles.keys())
            {
                for(int i = 0; i < m_groupedFiles[key].length(); i++)
                {
                    foreach (const QFileInfo &fileInfo, fileList) {
                        if(m_groupedFiles[key][i] == fileInfo.baseName())
                        {
                            m_groupedFiles[key][i] = fileInfo.absoluteFilePath();
                        }
                    }
                }

                if(m_groupedFiles[key].length() == 1)  // 删除值只有一个的，并不是想要分组的对象
                {
                    m_groupedFiles.remove(key);
                }
            }

            /********************************************************************************************/
            /********************************************************************************************/
            /********************************************************************************************/
            foreach (auto key, m_groupedFiles.keys())  // 目前仅支持一个小鼠3个tif
                while(m_groupedFiles[key].length()>3)  // 目前仅支持一个小鼠3个tif
                {                                      // 目前仅支持一个小鼠3个tif
                    m_groupedFiles[key].pop_back();    // 目前仅支持一个小鼠3个tif
                }                                      // 目前仅支持一个小鼠3个tif
            /********************************************************************************************/
            /********************************************************************************************/
            /********************************************************************************************/
            // qDebug() << "groupedFiles" << groupedFiles;


            // 设置目录选择框
            lineEditDirectory->setText(directory);

            // 设置小鼠数目选择框
            spinBoxMouseNum->setValue(m_groupedFiles.count());

            // 设置tif数量选择框
            QSet<int> num;
            num.insert(m_groupedFiles.first().length());
            foreach (auto key, m_groupedFiles.keys())
            {
                num.insert(m_groupedFiles[key].length());
                comboBoxSelectMouse->addItem(key, QVariant(false)); // 设置小鼠配准坐标ComBox
            }
            if(num.count() == 1)
            {
                spinBoxTifNum->setValue(*num.begin());
            }

            // 将QMap<QString, QList<QString>> 转换为 QVariantMap
            // 方便存入input_files point
            QVariantMap variantMapInputFiles;
            QVariantMap variantMapPoint;
            for (auto it = m_groupedFiles.constBegin(); it != m_groupedFiles.constEnd(); ++it) {
                QVariantList list;
                for (const QString &item : it.value()) {
                    list.append(item);
                }
                variantMapInputFiles.insert(it.key(), list);

                QVector<QVector<int>> points(2, QVector<int>(2, -1));
                m_groupedFilesPoint.insert(it.key(), points);
                variantMapPoint.insert(it.key(), QVariant::fromValue(points));
            }

            m_config[STEP1]["input_files"] = variantMapInputFiles;
            m_config[STEP1]["point"] = variantMapPoint;
            m_config[STEP1]["input_file_directory"] = directory;
            m_config[STEP1]["mouse_count"] = m_groupedFiles.count();
            m_config[STEP1]["files_per_mouse"] = *num.begin();

        } else {
            qDebug() << "No directory selected.";
            lineEditDirectory->setText(tr("文件夹选择错误！"));
            comboBoxSelectMouse->setDisabled(true);
            spinBoxMouseNum->setDisabled(true);
            spinBoxTifNum->setDisabled(true);
            spinBoxPointX1->setDisabled(true);
            spinBoxPointX2->setDisabled(true);
            spinBoxPointY1->setDisabled(true);
            spinBoxPointY2->setDisabled(true);
            pushButtonPointTXTImport->setDisabled(true);
        }
    };

    auto on_pointX1Change = [&, comboBoxSelectMouse](const int x){
        QString currentMouse = comboBoxSelectMouse->currentText();
        if(m_groupedFilesPoint.contains(currentMouse))
        {
            m_groupedFilesPoint[currentMouse][0][0] = x;

            // 检查4个点是否都设置了
            for (int i = 0; i < m_groupedFilesPoint[currentMouse].size(); ++i) {
                for (int j = 0; j < m_groupedFilesPoint[currentMouse][i].size(); ++j) {
                    if(m_groupedFilesPoint[currentMouse][i][j] == -1)
                        return;
                }
            }
            int index = comboBoxSelectMouse->currentIndex();
            comboBoxSelectMouse->setItemData(index, true);
        }
    };

    auto on_pointY1Change = [&, comboBoxSelectMouse](const int x){
        QString currentMouse = comboBoxSelectMouse->currentText();
        if(m_groupedFilesPoint.contains(currentMouse))
        {
            m_groupedFilesPoint[currentMouse][0][1] = x;

            // 检查4个点是否都设置了
            for (int i = 0; i < m_groupedFilesPoint[currentMouse].size(); ++i) {
                for (int j = 0; j < m_groupedFilesPoint[currentMouse][i].size(); ++j) {
                    if(m_groupedFilesPoint[currentMouse][i][j] == -1)
                        return;
                }
            }
            int index = comboBoxSelectMouse->currentIndex();
            comboBoxSelectMouse->setItemData(index, true);
        }
    };

    auto on_pointX2Change = [&, comboBoxSelectMouse](const int x){
        QString currentMouse = comboBoxSelectMouse->currentText();
        if(m_groupedFilesPoint.contains(currentMouse))
        {
            m_groupedFilesPoint[currentMouse][1][0] = x;

            // 检查4个点是否都设置了
            for (int i = 0; i < m_groupedFilesPoint[currentMouse].size(); ++i) {
                for (int j = 0; j < m_groupedFilesPoint[currentMouse][i].size(); ++j) {
                    if(m_groupedFilesPoint[currentMouse][i][j] == -1)
                        return;
                }
            }
            int index = comboBoxSelectMouse->currentIndex();
            comboBoxSelectMouse->setItemData(index, true);
        }
    };

    auto on_pointY2Change = [&, comboBoxSelectMouse](const int x){
        QString currentMouse = comboBoxSelectMouse->currentText();
        if(m_groupedFilesPoint.contains(currentMouse))
        {
            m_groupedFilesPoint[currentMouse][1][1] = x;

            // 检查4个点是否都设置了
            for (int i = 0; i < m_groupedFilesPoint[currentMouse].size(); ++i) {
                for (int j = 0; j < m_groupedFilesPoint[currentMouse][i].size(); ++j) {
                    if(m_groupedFilesPoint[currentMouse][i][j] == -1)
                        return;
                }
            }

            int index = comboBoxSelectMouse->currentIndex();
            comboBoxSelectMouse->setItemData(index, true);
        }
    };

    auto on_comboxSelectChange = [&, spinBoxPointX1, spinBoxPointX2, spinBoxPointY1, spinBoxPointY2](const QString &text){
        if(m_groupedFilesPoint.contains(text))
        {
            // 阻止发送一次 spinBox 值改变信号，因为会导致 comBox设置完所有点后显示绿色的样式异常
            // 因为对于新的combox selectedItem 四个 piont 又重新设置值，这样就相当于设置完了要改变样式了
            spinBoxPointX1->blockSignals(true);
            spinBoxPointY1->blockSignals(true);
            spinBoxPointX2->blockSignals(true);
            spinBoxPointY2->blockSignals(true);

            spinBoxPointX1->setValue(m_groupedFilesPoint[text][0][0]);
            spinBoxPointY1->setValue(m_groupedFilesPoint[text][0][1]);
            spinBoxPointX2->setValue(m_groupedFilesPoint[text][1][0]);
            spinBoxPointY2->setValue(m_groupedFilesPoint[text][1][1]);

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
            "E:\\CODE\\Qt\\MouseAnalysis\\build\\Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\\data",
            "Text Files (*.txt);;All Files (*)"
            );

        if (fullPath.isEmpty())
        {
            QMessageBox::critical(nullptr, "错误", QString("未选择文件！"));
            return;
        }

        if(m_csvParser.parse(fullPath))
        {
            const QVector<QStringList> &csvData = m_csvParser.getData();
            foreach(const auto &list, csvData)
            {
                auto key = list[0];
                if(!m_groupedFilesPoint.contains(key))  // 没有这个键，退出解析
                {
                    QMessageBox::critical(nullptr, "错误", QString("文件名错误: %1\n%2").arg(key, list.join(',')));
                    return;
                }
                if(list.size() != 5)   // 一行应该有文件名加四个数字，不满足，退出解析
                {
                    QMessageBox::critical(nullptr, "错误", QString("数据量错误: %1\n应为：文件名,X1,Y1,X2,Y2").arg(list.join(',')));
                    return;
                }
                bool ok;
                m_groupedFilesPoint[key][0][0] = list[1].toInt(&ok);
                if(!ok)
                {
                    m_groupedFilesPoint[key][0][0] = -1;
                    QMessageBox::critical(nullptr, "错误", QString("坐标格式错误，无法转换为数字: %1\n%2").arg(list[1], list.join(',')));
                    return;
                }
                m_groupedFilesPoint[key][0][1] = list[2].toInt(&ok);
                if(!ok)
                {
                    m_groupedFilesPoint[key][0][1] = -1;
                    QMessageBox::critical(nullptr, "错误", QString("坐标格式错误，无法转换为数字: %1\n%2").arg(list[2], list.join(',')));
                    return;
                }
                m_groupedFilesPoint[key][1][0] = list[3].toInt(&ok);
                if(!ok)
                {
                    m_groupedFilesPoint[key][1][0] = -1;
                    QMessageBox::critical(nullptr, "错误", QString("坐标格式错误，无法转换为数字: %1\n%2").arg(list[3], list.join(',')));
                    return;
                }
                m_groupedFilesPoint[key][1][1] = list[4].toInt(&ok);
                if(!ok)
                {
                    m_groupedFilesPoint[key][1][1] = -1;
                    QMessageBox::critical(nullptr, "错误", QString("坐标格式错误，无法转换为数字: %1\n%2").arg(list[4], list.join(',')));
                    return;
                }

                // 检查4个点是否都设置了，更新combox选择框
                int index = comboBoxSelectMouse->findText(key);
                if(m_groupedFilesPoint[key][0][0]+m_groupedFilesPoint[key][0][1]+m_groupedFilesPoint[key][1][0]+m_groupedFilesPoint[key][1][1]!= -4)
                {
                    comboBoxSelectMouse->setItemData(index, true);
                    comboBoxSelectMouse->setCurrentIndex(index);
                }
                else
                {
                    comboBoxSelectMouse->setItemData(index, false);
                }
            }
        }

    };

    connect(pushButtonDirectory, &QPushButton::clicked, this, on_selectDirectory);
    connect(spinBoxPointX1, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointX1Change);
    connect(spinBoxPointY1, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointY1Change);
    connect(spinBoxPointX2, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointX2Change);
    connect(spinBoxPointY2, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointY2Change);
    connect(comboBoxSelectMouse, &QComboBox::currentTextChanged, this, on_comboxSelectChange);
    connect(pushButtonPointTXTImport, &QPushButton::clicked, this, on_buttonTXTImport);

    // 数据选择框添加控件
    grid->addWidget(labelDirectory, 1, 1);
    grid->addLayout(hLayoutForDirectory, 1, 2);

    grid->addWidget(labelMouseNum, 2, 1);
    grid->addWidget(spinBoxMouseNum, 2, 2);

    grid->addWidget(labelTifNum, 3, 1);
    grid->addWidget(spinBoxTifNum, 3, 2);

    grid->addWidget(labelPoint, 4, 1);
    grid->addLayout(hLayoutForPoint, 4, 2);
    m_widgetDataSetting_1->setLayout(grid);
}


void MainWindow::initDataSettingPage2()
{
    QGridLayout *grid = new QGridLayout();

    // 对照组
    QLabel *labelCON = new QLabel(tr("对照组 mat 文件"));
    QLineEdit *lineEditCON = new QLineEdit();
    lineEditCON->setReadOnly(true);
    QPushButton *pushButtonSelectCON = new QPushButton(tr("选择"));

    // 突变组
    QLabel *labelMUT = new QLabel(tr("突变组 mat 文件"));
    QLineEdit *lineEditMUT = new QLineEdit();
    lineEditMUT->setReadOnly(true);
    QPushButton *pushButtonSelectMUT = new QPushButton(tr("选择"));

    m_config[STEP2]["folder_path"] = "./";

    auto on_selectCONFile = [&, lineEditCON](){
        QString fullPath = QFileDialog::getOpenFileName(
            nullptr,
            tr("选择导入TXT配准坐标"),
            "E:\\CODE\\Qt\\MouseAnalysis\\build\\Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\\data",
            "Text Files (*.mat);;All Files (*)"
            );

        if (fullPath.isEmpty())
        {
            QMessageBox::critical(nullptr, "错误", QString("未选择文件！"));
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
            "E:\\CODE\\Qt\\MouseAnalysis\\build\\Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\\data",
            "Text Files (*.mat);;All Files (*)"
            );

        if (fullPath.isEmpty())
        {
            QMessageBox::critical(nullptr, "错误", QString("未选择文件！"));
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

    connect(pushButtonSelectCON, &QPushButton::clicked, this, on_selectCONFile);
    connect(pushButtonSelectMUT, &QPushButton::clicked, this, on_selectMUTFile);

    grid->addWidget(labelCON, 0, 0);
    grid->addWidget(lineEditCON, 0, 1);
    grid->addWidget(pushButtonSelectCON, 0, 2);

    grid->addWidget(labelMUT, 1, 0);
    grid->addWidget(lineEditMUT, 1, 1);
    grid->addWidget(pushButtonSelectMUT, 1, 2);

    grid->setAlignment(Qt::AlignTop);
    m_widgetDataSetting_2->setLayout(grid);
}


void MainWindow::initDataSettingPage3()
{
    QGridLayout *grid = new QGridLayout();

    // 对照组
    QLabel *labelCON = new QLabel(tr(" mat 文件"));
    QLineEdit *lineEditCON = new QLineEdit();
    lineEditCON->setReadOnly(true);
    QPushButton *pushButtonSelectCON = new QPushButton(tr("选择"));


    auto on_selectCONFile = [&, lineEditCON](){
        QString fullPath = QFileDialog::getOpenFileName(
            nullptr,
            tr("选择导入 mat 文件"),
            "E:\\CODE\\Qt\\MouseAnalysis\\build\\Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\\data",
            "Text Files (*.mat);;All Files (*)"
            );

        if (fullPath.isEmpty())
        {
            QMessageBox::critical(nullptr, "错误", QString("未选择文件！"));
            return;
        }

        lineEditCON->setText(fullPath);

        QMap<QString, QString> map;
        if(m_config[STEP3].contains("input_files"))  // 存在，以前写入过，更新
            map = m_config[STEP3]["input_files"].value<QMap<QString, QString>>();
        else                                         // 不存在，准备插入
            map.clear();

        map["CON"] = fullPath;
        m_config[STEP3]["input_files"] = QVariant::fromValue(map);
    };

    connect(pushButtonSelectCON, &QPushButton::clicked, this, on_selectCONFile);

    grid->addWidget(labelCON, 0, 0);
    grid->addWidget(lineEditCON, 0, 1);
    grid->addWidget(pushButtonSelectCON, 0, 2);

    grid->setAlignment(Qt::AlignTop);
    m_widgetDataSetting_3->setLayout(grid);
}


void MainWindow::initDataSettingPage4()
{

}


void MainWindow::initDataSettingPage5()
{

}


void MainWindow::initDataSettingPage6()
{

}




void MainWindow::on_widgetContainerStep_1_clicked()
{
    slotStepChange(Step::STEP1);

    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);

}


void MainWindow::on_widgetContainerStep_2_clicked()
{
    slotStepChange(Step::STEP2);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);
}


void MainWindow::on_widgetContainerStep_3_clicked()
{
    slotStepChange(Step::STEP3);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);
}


void MainWindow::on_widgetContainerStep_4_clicked()
{
    slotStepChange(Step::STEP4);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);
}


void MainWindow::on_widgetContainerStep_5_clicked()
{
    slotStepChange(Step::STEP5);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);
}


void MainWindow::on_widgetContainerStep_6_clicked()
{
    slotStepChange(Step::STEP6);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);
}


void MainWindow::on_processStart()
{
    QString configSavePath = QDir::cleanPath(QDir::currentPath() + "/data/config.json");

    qDebug() << QString("Process isRunning:(%1)").arg(m_cmdProcessList[m_currentDisplay]->isRunning());
    if(m_cmdProcessList[m_currentDisplay]->isRunning())  // 正在跑,此时功能是终止
    {
        emit cmdTerminate();
        qDebug() << "Send terminate singal to process";
        ui->pushButtonRun->setText(tr("开始"));
        return;
    }
    // 没有跑,此时功能是开始,开始下面的检查输入

    switch (m_currentDisplay)
    {
        case Step::STEP1:  // 检查 mouse_count files_per_mouse input_files point output_directory 是否已经设置
        {
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
            if(!m_config[STEP1].contains("files_per_mouse"))
            {
                QMessageBox::warning(nullptr, "错误", "未设置每只小鼠TIF文件数量！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }
            foreach(const auto key, m_groupedFilesPoint.keys())
            {
                for(int i = 0; i < m_groupedFilesPoint[key].size(); ++i)
                    for(int j = 0; j < m_groupedFilesPoint[key][i].size(); ++j)
                    {
                        if(m_groupedFilesPoint[key][i][j] == -1)
                        {
                            QMessageBox::warning(nullptr, "错误", "每只小鼠的配准坐标都需要设置！",  QMessageBox::Ok,QMessageBox::Ok);
                            return;
                        }
                    }
            }

            m_config[STEP1]["point"] = QVariant::fromValue(m_groupedFilesPoint);

            m_configSaver.saveConfig(m_config, configSavePath);

            emit cmdStartRun();
            ui->pushButtonRun->setText(tr("暂停"));
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

            m_configSaver.saveConfig(m_config, configSavePath);

            emit cmdStartRun();
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
            if(!map.contains("CON"))
            {
                QMessageBox::warning(nullptr, "错误", "未选择 mat 文件！",  QMessageBox::Ok,QMessageBox::Ok);
                return;
            }

            m_configSaver.saveConfig(m_config, configSavePath);

            emit cmdStartRun();
            ui->pushButtonRun->setText(tr("暂停"));
            break;
        }

        case Step::STEP4:
        {

            emit cmdStartRun();
            ui->pushButtonRun->setText(tr("暂停"));
            break;
        }

        case Step::STEP5:
        {

            emit cmdStartRun();
            ui->pushButtonRun->setText(tr("暂停"));
            break;
        }

        case Step::STEP6:
        {

            emit cmdStartRun();
            ui->pushButtonRun->setText(tr("暂停"));
            break;
        }
    }
}



void MainWindow::on_updateImageList()
{
    qDebug() << "cmdFinish, updateImageList" << m_resultPath[m_currentDisplay];

    switch (m_currentDisplay) {
    case Step::STEP1:

        m_imageList->setImgPath(m_resultPath[STEP1]);
        break;

    case Step::STEP2:

        m_imageList->setImgPath(m_resultPath[STEP2]);
        break;

    case Step::STEP3:

        m_imageList->setImgPath(m_resultPath[STEP3]);
        break;

    case Step::STEP4:

        m_imageList->setImgPath(m_resultPath[STEP4]);
        break;

    case Step::STEP5:

        m_imageList->setImgPath(m_resultPath[STEP5]);
        break;

    case Step::STEP6:

        m_imageList->setImgPath(m_resultPath[STEP6]);
        break;

    default:
        break;
    }
}



