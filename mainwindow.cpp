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

    // QColor color(230, 230, 230,240);
    // QPalette pal(palette());
    // pal.setColor(QPalette::Window, color);
    // setAutoFillBackground(true);
    // setPalette(pal);

    // ui->widgetContainerStep_1->setProperty("widget", "stepContainer");
    // ui->widgetContainerStep_2->setProperty("widget", "stepContainer");
    // ui->widgetContainerStep_3->setProperty("widget", "stepContainer");
    // ui->widgetContainerStep_4->setProperty("widget", "stepContainer");
    // ui->widgetContainerStep_5->setProperty("widget", "stepContainer");
    // ui->widgetContainerStep_6->setProperty("widget", "stepContainer");

    // ClickableWidget *widget = new ClickableWidget();
    // widget->setContentsMargins(QMargins(10, 10, 10, 10));
    // // widget->setFixedSize(QSize(70, 70));
    // QHBoxLayout *layout_ = new QHBoxLayout();
    // layout_->setContentsMargins(0, 0, 10, 0);
    // QLabel *label1 = new QLabel("你好难啊");
    // layout_->addWidget(label1);
    // widget->setLayout(layout_);
    // ui->verticalLayout_4->addWidget(widget);

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
}


MainWindow::~MainWindow()
{
    delete ui;
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


void MainWindow::on_textBrowserLogShow(QString str)
{
    ui->textBrowser->append(str);
}


void MainWindow::slotChange(Step currentStep)
{
    Step preDisplay = m_currentDisplay;
    changeCurrentDisplay(currentStep);

    // 清除输出，添加当前运行的log
    ui->textBrowser->clear();
    ui->textBrowser->append(m_cmdLog[m_currentDisplay]->join("<br>"));

    // 日志框显示槽函数的解绑与绑定
    disconnect(m_cmdProcessList[preDisplay], &CommandProcess::resultReady, this, &MainWindow::on_textBrowserLogShow);
    connect(m_cmdProcessList[m_currentDisplay], &CommandProcess::resultReady, this, &MainWindow::on_textBrowserLogShow);

    // 开始运行按钮槽函数的解绑与绑定
    disconnect(this, &MainWindow::cmdStartRun, m_cmdProcessList[preDisplay], &CommandProcess::run);
    connect(this, &MainWindow::cmdStartRun, m_cmdProcessList[m_currentDisplay], &CommandProcess::run);

    // 图片列表显示的解绑与绑定
    disconnect(m_cmdProcessList[preDisplay], &CommandProcess::cmdFinish, this, &MainWindow::on_updateImageList);
    connect(m_cmdProcessList[m_currentDisplay], &CommandProcess::cmdFinish, this, &MainWindow::on_updateImageList);

    ui->labelList->setText(QString("Previous: %1  Current: %2").arg(preDisplay).arg(m_currentDisplay));

    // 移除旧的控件
    // clearWidget(qobject_cast<QWidget*>(ui->groupBoxData));
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
    QLabel *labelPoint = new QLabel(tr("配准坐标"));
    QHBoxLayout *hLayoutForPoint = new QHBoxLayout();
    QComboBox *comboBoxSelectMouse = new QComboBox();
    ComboxItemDelegate *delegate = new ComboxItemDelegate();
    comboBoxSelectMouse->setItemDelegate(delegate);

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

    hLayoutForPoint->addWidget(comboBoxSelectMouse, 4);
    QGridLayout *gridPoint = new QGridLayout();
    gridPoint->addWidget(labelPonintX1, 1, 1, 1, 1);
    gridPoint->addWidget(spinBoxPointX1, 1, 2, 1, 2);

    gridPoint->addWidget(labelPonintX2, 1, 4, 1, 1);
    gridPoint->addWidget(spinBoxPointX2, 1, 5, 1, 2);

    gridPoint->addWidget(labelPonintY1, 2, 1, 1, 1);
    gridPoint->addWidget(spinBoxPointY1, 2, 2, 1, 2);

    gridPoint->addWidget(labelPonintY2, 2, 4, 1, 1);
    gridPoint->addWidget(spinBoxPointY2, 2, 5, 1, 2);

    hLayoutForPoint->addLayout(gridPoint, 7);


    // 数据选择框添加控件
    grid->addWidget(labelDirectory, 1, 1);
    grid->addLayout(hLayoutForDirectory, 1, 2);

    grid->addWidget(labelMouseNum, 2, 1);
    grid->addWidget(spinBoxMouseNum, 2, 2);

    grid->addWidget(labelTifNum, 3, 1);
    grid->addWidget(spinBoxTifNum, 3, 2);

    grid->addWidget(labelPoint, 4, 1);
    grid->addLayout(hLayoutForPoint, 4, 2);

    comboBoxSelectMouse->setDisabled(true);
    spinBoxMouseNum->setDisabled(true);
    spinBoxTifNum->setDisabled(true);
    spinBoxPointX1->setDisabled(true);
    spinBoxPointX2->setDisabled(true);
    spinBoxPointY1->setDisabled(true);
    spinBoxPointY2->setDisabled(true);

    QPushButton *pushButtonProSelect = new QPushButton(tr("高级选择"));
    pushButtonProSelect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid->addWidget(pushButtonProSelect, 5, 1, 1, 2);

    // 添加控件到 groupBox
    // ui->groupBoxData->setLayout(grid);
    m_widgetDataSetting_1->setLayout(grid);

    // 选择文件目录，按照前缀来自动识别文件并分组
    auto on_selectDirectory = [&, comboBoxSelectMouse, lineEditDirectory, spinBoxMouseNum, spinBoxTifNum, spinBoxPointX1, spinBoxPointX2, spinBoxPointY1, spinBoxPointY2](){
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
                return;
            }
            comboBoxSelectMouse->setDisabled(false);
            spinBoxMouseNum->setDisabled(false);
            spinBoxTifNum->setDisabled(false);
            spinBoxPointX1->setDisabled(false);
            spinBoxPointX2->setDisabled(false);
            spinBoxPointY1->setDisabled(false);
            spinBoxPointY2->setDisabled(false);

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

            m_config[m_currentDisplay]["input_files"] = variantMapInputFiles;
            m_config[m_currentDisplay]["point"] = variantMapPoint;
            m_config[m_currentDisplay]["input_file_directory"] = directory;
            m_config[m_currentDisplay]["output_directory"] = ".\\data\\step1";
            m_config[m_currentDisplay]["mouse_count"] = m_groupedFiles.count();
            m_config[m_currentDisplay]["files_per_mouse"] = *num.begin();

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
        }
    };

    auto on_pointX1Change = [&, comboBoxSelectMouse](const int x){
        QString currentMouse = comboBoxSelectMouse->currentText();
        if(m_groupedFilesPoint.contains(currentMouse))
        {
            m_groupedFilesPoint[currentMouse][0][0] = x;
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

    connect(pushButtonDirectory, &QPushButton::clicked, this, on_selectDirectory);
    connect(spinBoxPointX1, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointX1Change);
    connect(spinBoxPointY1, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointY1Change);
    connect(spinBoxPointX2, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointX2Change);
    connect(spinBoxPointY2, QOverload<int>::of (&QSpinBox::valueChanged), this, on_pointY2Change);
    connect(comboBoxSelectMouse, &QComboBox::currentTextChanged, this, on_comboxSelectChange);
}


void MainWindow::initDataSettingPage2()
{

}


void MainWindow::initDataSettingPage3()
{

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
    slotChange(Step::STEP1);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);

}


void MainWindow::on_widgetContainerStep_2_clicked()
{
    slotChange(Step::STEP2);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);
}


void MainWindow::on_widgetContainerStep_3_clicked()
{
    slotChange(Step::STEP3);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);
}


void MainWindow::on_widgetContainerStep_4_clicked()
{
    slotChange(Step::STEP4);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);
}


void MainWindow::on_widgetContainerStep_5_clicked()
{
    slotChange(Step::STEP5);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);
}


void MainWindow::on_widgetContainerStep_6_clicked()
{
    slotChange(Step::STEP6);
    m_layoutDataSetting->setCurrentIndex(m_currentDisplay);
    m_imageList->setImgPath(m_resultPath[m_currentDisplay]);
}


void MainWindow::on_processStart()
{
    QString savePath = QDir::cleanPath(QDir::currentPath() + '/' + "/debug/data" + "/config11111.json");
    m_configSaver.saveConfig(m_config, savePath);

    switch (m_currentDisplay) {
    case Step::STEP1:  // 检查 mouse_count files_per_mouse input_files point output_directory 是否已经设置

        if(!m_config[m_currentDisplay].contains("input_file_directory"))
        {
            QMessageBox::warning(nullptr, "错误", "未设置小鼠TIF文件路径！",  QMessageBox::Ok,QMessageBox::Ok);
            return;
        }
        if(!m_config[m_currentDisplay].contains("mouse_count"))
        {
            QMessageBox::warning(nullptr, "错误", "未设置小鼠数量！",  QMessageBox::Ok,QMessageBox::Ok);
            return;
        }
        if(!m_config[m_currentDisplay].contains("files_per_mouse"))
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

        foreach (auto key, m_groupedFilesPoint.keys()) {
            m_groupedFilesPoint[key].pop_back();
        }
        m_config[m_currentDisplay]["point"] = QVariant::fromValue(m_groupedFilesPoint);

        m_configSaver.saveConfig(m_config, QDir::cleanPath(QDir::currentPath() + '/' + "/debug/data" + "/config11111.json"));

        emit cmdStartRun();
        break;

    case Step::STEP2:

        emit cmdStartRun();
        break;

    case Step::STEP3:

        emit cmdStartRun();
        break;

    case Step::STEP4:

        emit cmdStartRun();
        break;

    case Step::STEP5:

        emit cmdStartRun();
        break;

    case Step::STEP6:

        emit cmdStartRun();
        break;

    default:
        break;
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



