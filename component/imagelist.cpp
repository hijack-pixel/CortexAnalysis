#pragma execution_character_set("utf-8")

#include "imagelist.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>
#include "common.h"

extern QPixmap getEMFPixmap(const QString& filePath, bool zoomIn, int minSize);

ImageList::ImageList(QWidget *parent): QWidget{parent}
{
    init();
}

void ImageList::init()
{
    m_listWidget = new QListWidget();
    m_listWidget->setIconSize(QSize(32, 32));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    // 功能区
    QHBoxLayout *layoutFunction = new QHBoxLayout();
    m_pushButtonDownloadSelect = new QPushButton(tr("下载选中"));

    m_checkBox = new QCheckBox(tr("全部选中"));

    layoutFunction->addWidget(m_checkBox, 1);
    layoutFunction->addWidget(m_pushButtonDownloadSelect, 5);

    layoutFunction->setMargin(5);
    layoutFunction->setAlignment(Qt::AlignBottom);
    layout->addLayout(layoutFunction);
    layout->addWidget(m_listWidget);

    // 当前行改变，发信号：当前图片路径
    connect(m_listWidget, &QListWidget::currentRowChanged, this, [&](int row)
    {
        // ListWidget clear 时 row 为 -1！！！
        if(row != -1)
            emit itemCurrent(m_imgInfoList[row].absoluteFilePath());
    });

    // 点击全选与否
    connect(m_checkBox, &QCheckBox::stateChanged, this, [&](int state){

        for (int i = 0; i < m_listWidget->count(); ++i) {
            m_listWidget->item(i)->setCheckState(static_cast<Qt::CheckState>(state));
        }

    });

    // 更新全选、未选状态
    connect(m_listWidget, &QListWidget::itemClicked, this, &ImageList::on_updateCheckState);

    // 点击下载选中图片
    connect(m_pushButtonDownloadSelect, &QPushButton::clicked, this, &ImageList::on_downloadSelect);
}

void ImageList::on_updateCheckState()
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

void ImageList::on_downloadSelect()
{
    int selectCount = 0;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (m_listWidget->item(i)->checkState() == Qt::Checked) {
            ++selectCount;
        }
    }

    if(selectCount == 0)
    {
        QMessageBox::critical(nullptr, "错误", QString("未选择文件！"));
        return;
    }

    QString directory = QFileDialog::getExistingDirectory(
        nullptr,
        tr("选择保存目录"),
        nullptr,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(directory.isEmpty()) return;

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

    // 老版导出逻辑
    // for (int i = 0; i < m_listWidget->count(); ++i) {
    //     QListWidgetItem *item = m_listWidget->item(i);
    //     if (item->checkState() == Qt::Checked) {
    //         // 获取文件的绝对路径
    //         QString sourceFilePath = item->data(Qt::UserRole).toString();
    //         QFile sourceFile(sourceFilePath);

    //         // 获取文件名
    //         QFileInfo fileInfo(sourceFile);
    //         QString fileName = fileInfo.fileName();

    //         // 构建目标文件路径
    //         QString targetFilePath = QDir(directory).filePath(fileName);

    //         // 如果目标文件已存在，提示用户
    //         if (QFile::exists(targetFilePath)) {
    //             QMessageBox::StandardButton reply;
    //             reply = QMessageBox::question(nullptr, tr("文件已存在"),
    //                                           QString(tr("文件已存在：%1，覆盖此文件?")).arg(fileName),
    //                                           QMessageBox::Yes | QMessageBox::No);
    //             if (reply == QMessageBox::No)        // 如果用户选择不覆盖，则跳过当前文件
    //             {
    //                 continue;
    //             }
    //         }

    //         // 复制文件到新位置
    //         if (sourceFile.copy(targetFilePath)) {
    //             // QMessageBox::information(nullptr, "File Saved", "File saved as " + targetFilePath);
    //         } else {
    //             QFile existTargetFile(targetFilePath);
    //             if (!existTargetFile.remove()) {
    //                 QMessageBox::critical(nullptr, tr("保存失败"), QString(tr("无法覆盖目标文件%1！")).arg(targetFilePath));
    //                 existTargetFile.close();
    //             }
    //         }
    //     }
    // }
}


void ImageList::setImgPath(const QString& path)
{
    clearImgList();

    QDir directory(path);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.emf";
    m_imgInfoList = directory.entryInfoList(filters, QDir::Files);

    foreach (const auto &imgInfo, m_imgInfoList) {
        QPixmap pixmap(imgInfo.absoluteFilePath());
        if(pixmap.isNull() && imgInfo.absoluteFilePath().endsWith("emf"))
        {
            pixmap = getEMFPixmap(imgInfo.absoluteFilePath());
        }

        QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap), imgInfo.fileName());
        item->setSizeHint(QSize(100, 40));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // 添加复选框
        item->setCheckState(Qt::Unchecked);                      // 默认不选中
        m_listWidget->addItem(item);                             // 添加item到list
        item->setData(Qt::UserRole, QVariant::fromValue(imgInfo.absoluteFilePath()));
        item->setToolTip(         "文件名  ：" + imgInfo.fileName() + '\n' +
                    "分辨率  ：" + QString::number(pixmap.width()) + " X " + QString::number(pixmap.height()) + '\n' +
                    "文件大小：" + QString::number(static_cast<double>(imgInfo.size()) / (1024 * 1024), 'g', 2) + " MB" + '\n' +
                    "存储路径：" + imgInfo.filePath() + '\n' +
                    "创建时间：" + imgInfo.fileTime(QFileDevice::FileBirthTime).toString("yyyy-MM-dd HH:mm:ss"));
    }

    if(m_imgInfoList.empty())
    {
        m_checkBox->setDisabled(true);
        m_pushButtonDownloadSelect->setDisabled(true);
        emit itemCurrent("");
    }
    else
    {
        m_checkBox->setDisabled(false);
        m_pushButtonDownloadSelect->setDisabled(false);
        m_listWidget->setCurrentRow(0);
        // emit itemCurrent(m_imgInfoList[0].absoluteFilePath());
    }

    // 更新选中状态
    on_updateCheckState();
}


void ImageList::clearImgList()
{
    m_listWidget->clear();
    m_imgInfoList.clear();
}

