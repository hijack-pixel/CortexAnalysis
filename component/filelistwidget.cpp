#pragma execution_character_set("utf-8")

#include "common.h"
#include "filelistwidget.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include "copyworker.h"

extern QPixmap getEMFPixmap(const QString& filePath, bool zoomIn, int minSize);

FileListWidget::FileListWidget(QWidget *parent): QWidget{parent}
{
    init();
}

void FileListWidget::init()
{
    m_listWidget = new QListWidget();
    m_listWidget->setIconSize(QSize(32, 32));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(3);

    // 功能区
    QHBoxLayout *layoutFunction = new QHBoxLayout();
    m_pushButtonDownloadSelect = new QPushButton(tr("下载选中"));

    m_checkBox = new QCheckBox(tr("全选"));

    layoutFunction->addWidget(m_checkBox, 1);
    layoutFunction->addWidget(m_pushButtonDownloadSelect, 5);
    layoutFunction->setMargin(0);
    layoutFunction->setContentsMargins(QMargins(5, 0, 0, 0));
    layoutFunction->setAlignment(Qt::AlignBottom);

    layout->addWidget(m_listWidget);
    layout->addLayout(layoutFunction);

    // 当前行改变，发信号：当前图片路径
    connect(m_listWidget, &QListWidget::currentRowChanged, this, [&](int row)
    {
        // ListWidget clear 时 row 为 -1！！！
        if(row != -1)
        {
            QString absoultFilePath = m_listWidget->item(row)->data(Qt::UserRole).value<QString>();
            FileType fileType = m_listWidget->item(row)->data(Qt::UserRole+1).value<FileType>();
            emit itemCurrent(absoultFilePath, fileType, row);
        }
    });

    // 点击全选与否
    connect(m_checkBox, &QCheckBox::stateChanged, this, [&](int state){
        for (int i = 0; i < m_listWidget->count(); ++i) {
            m_listWidget->item(i)->setCheckState(static_cast<Qt::CheckState>(state));
        }

    });

    // 更新全选、未选状态
    connect(m_listWidget, &QListWidget::itemClicked, this, &FileListWidget::on_updateCheckState);

    // 点击下载选中图片
    connect(m_pushButtonDownloadSelect, &QPushButton::clicked, this, &FileListWidget::on_downloadSelect);
}

void FileListWidget::on_updateCheckState()
{
    bool allChecked = true;

    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (m_listWidget->item(i)->checkState() != Qt::Checked) {
            allChecked = false;
        }
    }

    m_checkBox->blockSignals(true);
    if (allChecked)
    {
        m_checkBox->setChecked(true);
    }
    else
    {
        m_checkBox->setChecked(false);
    }
    m_checkBox->blockSignals(false);
}

void FileListWidget::on_downloadSelect()
{
    int selectCount = 0;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (m_listWidget->item(i)->checkState() == Qt::Checked) {
            ++selectCount;
        }
    }

    if(selectCount == 0)
    {
        QMessageBox::critical(this, "错误", QString("未选择文件！"));
        return;
    }

    QString directory = QFileDialog::getExistingDirectory(
        nullptr,
        tr("选择保存目录"),
        nullptr,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(directory.isEmpty()) return;

    QList<QPair<QString, QString>> tasks;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            // 获取文件的绝对路径
            QString sourceFilePath = item->data(Qt::UserRole).toString();
            QFile sourceFile(sourceFilePath);

            // 获取文件名
            QFileInfo fileInfo(sourceFile);
            QString fileName = fileInfo.fileName();

            // 构建目标文件路径
            QString targetFilePath = QDir(directory).filePath(fileName);

            // 记录路径
            tasks.append(QPair(sourceFilePath, targetFilePath));
        }
    }

    // CopyWorker* copyWorker = new CopyWorker(this);
    // copyWorker->start(tasks);
    (new CopyWorker(this))->start(tasks);

    /*

    // 新版版导出逻辑
    QStringList sourceFilePathList;
    QStringList targetFilePathList;
    QStringList repeatFilePathList;
    QStringList repeatFilenameList;
    // 获取源文件列表、保存文件列表、重复文件列表
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            // 获取文件的绝对路径
            QString sourceFilePath = item->data(Qt::UserRole).toString();
            QFile sourceFile(sourceFilePath);

            // 获取文件名
            QFileInfo fileInfo(sourceFile);
            QString fileName = fileInfo.fileName();

            // 构建目标文件路径
            QString targetFilePath = QDir(directory).filePath(fileName);

            // 记录路径
            sourceFilePathList.append(sourceFilePath);
            targetFilePathList.append(targetFilePath);

            // 如果目标文件已存在，提示用户
            if (QFile::exists(targetFilePath)) {
                repeatFilePathList.append(targetFilePath);
                repeatFilenameList.append(fileName);
            }
        }
    }

    if(repeatFilePathList.isEmpty())    // 没有重复文件，直接保存
    {
        for(int i = 0; i < targetFilePathList.length(); i++)
        {
            QFile sourceFile(sourceFilePathList[i]);
            if (!sourceFile.copy(targetFilePathList[i]))
            {
                QMessageBox::critical(nullptr, tr("保存失败"), QString(tr("文件：%1")).arg(QFileInfo(targetFilePathList[i]).fileName()));
            }
        }
    }
    else                                // 有重复文件询问是否覆盖
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, tr("文件已存在"),
                                      QString(tr("覆盖以下文件?\n%1")).arg(repeatFilenameList.join("\n")),
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)   // 如果覆盖就删除重复的目标文件
        {
            foreach(auto& repeatFilePath, repeatFilePathList)
            {
                QFile existTargetFile(repeatFilePath);
                qDebug() << "delete" << repeatFilePath;
                if (!existTargetFile.remove()) {
                    QMessageBox::critical(nullptr, tr("保存失败"), QString(tr("无法覆盖目标文件%1！")).arg(repeatFilePath));
                    existTargetFile.close();
                }
            }
        }
        else                             // 不覆盖就从源文件列表、目标文件列表中删除重复文件
        {
            QStringList::iterator sourceIt = sourceFilePathList.begin();
            QStringList::iterator targetIt = targetFilePathList.begin();

            while (sourceIt != sourceFilePathList.end()) {
                bool shouldRemove = false;
                foreach (const QString& repeatFilePath, repeatFilePathList) {
                    if (*targetIt == repeatFilePath) {
                        shouldRemove = true;
                        break;
                    }
                }
                if (shouldRemove) {
                    sourceIt = sourceFilePathList.erase(sourceIt);// 删除sourceFilePathList中的元素
                    targetIt = targetFilePathList.erase(targetIt);// 删除targetFilePathList中的对应元素
                } else {
                    // 只有在不删除元素时才递增迭代器
                    ++sourceIt;
                    ++targetIt;
                }
            }
        }

        for(int i = 0; i < sourceFilePathList.length(); i++)   // copy文件
        {
            QFile sourceFile(sourceFilePathList[i]);
            if (!sourceFile.copy(targetFilePathList[i])) {
                QMessageBox::critical(nullptr, tr("保存失败"), QString(tr("文件：%1！")).arg(QFileInfo(targetFilePathList[i]).fileName()));
                sourceFile.close();
            }
        }

    }
*/
}

QPixmap loadPixmap(const QString &filePath, QListWidgetItem *item) {
    QPixmap pixmap(filePath);
    if (pixmap.isNull() && filePath.endsWith("emf")) {
        pixmap = getEMFPixmap(filePath);
    }
    return pixmap;
}

void FileListWidget::setFilePath(const QString& path, FileType fileType)
{
    clearFileList();

    QDir directory(path);
    QStringList filters;

    switch(fileType)
    {
    case FileType::IMAGE:
    {
        filters.clear();
        filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.emf";
        m_fileInfoList = directory.entryInfoList(filters, QDir::Files);

        m_loadingIconsCount = m_fileInfoList.size();  // 获取要加载的图标数量
        if(m_fileInfoList.size() == 0)
        {
            emit iconLoadFinish(); // 图标加载完成发送信号
            qDebug() << "ICON LOAD FINISH";
        }
        foreach (const auto &imgInfo, m_fileInfoList) {
            QListWidgetItem *item = new QListWidgetItem(imgInfo.fileName());
            item->setSizeHint(QSize(100, 40));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);  // 添加复选框
            item->setCheckState(Qt::Unchecked);                       // 默认不选中
            m_listWidget->addItem(item);                              // 添加到 listWidget
            item->setData(Qt::UserRole, QVariant::fromValue(imgInfo.absoluteFilePath()));
            item->setData(Qt::UserRole + 1, QVariant::fromValue(fileType));

            // 创建 QFutureWatcher 来处理异步任务
            auto watcher = new QFutureWatcher<QPixmap>(this); // 使用 this 来避免内存泄漏

            // 连接 QFutureWatcher 的 finished 信号
            connect(watcher, &QFutureWatcher<QPixmap>::finished, this, [watcher, item, imgInfo, this]() {
                if (watcher->result().isNull()) {
                    item->setIcon(QIcon(":/default_icon.png"));  // 设置默认图标
                }
                else {
                    item->setIcon(QIcon(watcher->result()));   // 设置加载的图标
                    item->setToolTip(
                        "文件名  ：" + imgInfo.fileName() + '\n' +
                        "分辨率  ：" + QString::number(watcher->result().width()) + " X " + QString::number(watcher->result().height()) + '\n' +
                        "文件大小：" + QString::number(static_cast<double>(imgInfo.size()) / (1024 * 1024), 'g', 2) + " MB" + '\n' +
                        "存储路径：" + imgInfo.filePath() + '\n' +
                        "创建时间：" + imgInfo.fileTime(QFileDevice::FileBirthTime).toString("yyyy-MM-dd HH:mm:ss") + '\n' +
                        "修改时间：" + imgInfo.fileTime(QFileDevice::FileModificationTime).toString("yyyy-MM-dd HH:mm:ss")
                        );
                }
                watcher->disconnect();
                watcher->deleteLater();  // 删除 QFutureWatcher 对象
                int currentCount = --m_loadingIconsCount; // 原子地减少计数器
                if (currentCount == 0) {
                    emit iconLoadFinish(); // 图标加载完成发送信号
                    qDebug() << "ICON LOAD FINISH";
                }
            });

            // 异步加载图标
            QFuture<QPixmap> future = QtConcurrent::run(loadPixmap, imgInfo.absoluteFilePath(), item);
            watcher->setFuture(future);  // 将异步任务与 watcher 绑定

            {
                // 老式单线程加载图标
                // QPixmap pixmap(imgInfo.absoluteFilePath());
                // if(pixmap.isNull() && imgInfo.absoluteFilePath().endsWith("emf"))
                // {
                //     pixmap = getEMFPixmap(imgInfo.absoluteFilePath());
                // }
                // pixmap = pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                // QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap), imgInfo.fileName());
                // item->setSizeHint(QSize(100, 40));
                // item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // 添加复选框
                // item->setCheckState(Qt::Unchecked);                      // 默认不选中
                // m_listWidget->addItem(item);                             // 添加item到list
                // item->setData(Qt::UserRole, QVariant::fromValue(imgInfo.absoluteFilePath()));
                // item->setData(Qt::UserRole + 1, QVariant::fromValue(fileType));
                // item->setToolTip(
                //     "文件名  ：" + imgInfo.fileName() + '\n' +
                //     "分辨率  ：" + QString::number(pixmap.width()) + " X " + QString::number(pixmap.height()) + '\n' +
                //     "文件大小：" + QString::number(static_cast<double>(imgInfo.size()) / (1024 * 1024), 'g', 2) + " MB" + '\n' +
                //     "存储路径：" + imgInfo.filePath() + '\n' +
                //     "创建时间：" + imgInfo.fileTime(QFileDevice::FileBirthTime).toString("yyyy-MM-dd HH:mm:ss")
                //     );
            }
        }
    }break;
    case FileType::TABLE:
    {
        filters.clear();
        filters << "*.xlsx";
        m_fileInfoList = directory.entryInfoList(filters, QDir::Files);

        foreach (const auto &fileInfo, m_fileInfoList) {
            QListWidgetItem *item = new QListWidgetItem(fileInfo.fileName());
            item->setSizeHint(QSize(100, 40));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // 添加复选框
            item->setCheckState(Qt::Unchecked);                      // 默认不选中
            m_listWidget->addItem(item);                             // 添加item到list
            item->setData(Qt::UserRole, QVariant::fromValue(fileInfo.absoluteFilePath()));
            item->setData(Qt::UserRole + 1, QVariant::fromValue(fileType));
            item->setToolTip(
                "文件名  ：" + fileInfo.fileName() + '\n' +
                "文件大小：" + QString::number(static_cast<double>(fileInfo.size()) / (1024 * 1024), 'g', 2) + " MB" + '\n' +
                "存储路径：" + fileInfo.filePath() + '\n' +
                "创建时间：" + fileInfo.fileTime(QFileDevice::FileBirthTime).toString("yyyy-MM-dd HH:mm:ss") + '\n' +
                "修改时间：" + fileInfo.fileTime(QFileDevice::FileModificationTime).toString("yyyy-MM-dd HH:mm:ss")
                );
        }
    }break;
    case FileType::OTHERS:
    {
        // 过滤图片、excel、txt（log保存用的是txt）
        QStringList excludeSuffix;
        excludeSuffix << "png" << "jpg" << "jpeg" << "bmp" << "emf" << "xlsx" << "txt";
        m_fileInfoList = directory.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

        foreach (const auto &fileInfo, m_fileInfoList) {
            // 去除图片、表格，只显示OTHERS
            if(excludeSuffix.contains(fileInfo.suffix(), Qt::CaseInsensitive)) continue;

            QListWidgetItem *item = new QListWidgetItem();
            item->setSizeHint(QSize(100, 40));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // 添加复选框
            item->setCheckState(Qt::Unchecked);                      // 默认不选中
            m_listWidget->addItem(item);                             // 添加item到list
            item->setData(Qt::UserRole, QVariant::fromValue(fileInfo.absoluteFilePath()));
            item->setData(Qt::UserRole + 1, QVariant::fromValue(fileType));

            if (fileInfo.isFile())
            {
                item->setText("["+fileInfo.suffix()+"] " + fileInfo.fileName());
                item->setToolTip(
                    "<b>不支持预览该类型文件：</b>" + fileInfo.suffix()+ "<br><br>" +
                    "<b>文件名&nbsp;&nbsp;&nbsp;：</b>" + fileInfo.fileName() + "<br>" +
                    "<b>文件大小：</b>" + formatFileSize(fileInfo.size()) + "<br>" +
                    "<b>存储路径：</b>" + fileInfo.filePath() + "<br>" +
                    "<b>创建时间：</b>" + fileInfo.fileTime(QFileDevice::FileBirthTime).toString("yyyy-MM-dd HH:mm:ss") + "<br>" +
                    "<b>修改时间：</b>" + fileInfo.fileTime(QFileDevice::FileModificationTime).toString("yyyy-MM-dd HH:mm:ss")
                    );
            }
            else if(fileInfo.isDir())
            {
                item->setText("[文件夹] " + fileInfo.fileName());
                item->setToolTip(
                    "<b>不支持预览该类型文件：</b>" + tr("文件夹") + "<br><br>" +
                    "<b>目录名&nbsp;&nbsp;&nbsp;：</b>" + fileInfo.fileName() + "<br>" +
                    "<b>目录大小：</b>" + "计算中..." + "<br>" +
                    "<b>存储路径：</b>" + fileInfo.filePath() + "<br>" +
                    "<b>创建时间：</b>" + fileInfo.fileTime(QFileDevice::FileBirthTime).toString("yyyy-MM-dd HH:mm:ss") + "<br>" +
                    "<b>修改时间：</b>" + fileInfo.fileTime(QFileDevice::FileModificationTime).toString("yyyy-MM-dd HH:mm:ss")
                    );

                // 为每个目录创建独立 watcher（轻量，计算完自动 delete）
                auto *watcher = new QFutureWatcher<qint64>();
                watcher->setProperty("filePath", fileInfo.filePath()); // 存 path 用于回调定位

                // 连接 finished 信号
                connect(watcher, &QFutureWatcher<qint64>::finished, this, [this, watcher]() {
                    QString path = watcher->property("filePath").toString();
                    qint64 size = watcher->future().result();
                    watcher->deleteLater(); // 自动清理

                    // 查找对应 item
                    for (int i = 0; i < m_listWidget->count(); ++i)
                    {
                        QListWidgetItem *itm = m_listWidget->item(i);
                        QString itemPath = itm->data(Qt::UserRole).toString();
                        if (itemPath == path)
                        {
                            QString newTooltip = itm->toolTip();
                            newTooltip.replace("计算中...", formatFileSize(size));
                            itm->setToolTip(newTooltip);
                            break;
                        }
                    }
                });

                // 启动计算
                watcher->setFuture(QtConcurrent::run(calculateDirectorySize, fileInfo.filePath()));
            }
        }

    }break;
    default:
        qDebug() << QString("FileListWidget::setFilePath，File type %1 not support!").arg(fileType);
    }

    // 文件列表没有项就不允许点击全选、下载
    if(m_listWidget->count() == 0)
    {
        m_checkBox->setDisabled(true);
        m_pushButtonDownloadSelect->setDisabled(true);
        emit itemCurrent("", fileType);
    }
    else
    {
        m_checkBox->setDisabled(false);
        m_pushButtonDownloadSelect->setDisabled(false);
        m_listWidget->setCurrentRow(0);
    }

    // 更新选中状态
    on_updateCheckState();
}


void FileListWidget::clearFileList()
{
    m_listWidget->clear();
    m_fileInfoList.clear();
}

QListWidgetItem* FileListWidget::getItemAtRow(const int row)
{
    if(row == -1 || m_listWidget == nullptr) return nullptr;

    return m_listWidget->item(row);
}

