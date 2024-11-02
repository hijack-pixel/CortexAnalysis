#pragma execution_character_set("utf-8")

#include "imagelist.h"
#include <QDebug>
#include <QDateTime>

ImageList::ImageList(QWidget *parent): QWidget{parent}
{
    init();
}

void ImageList::init()
{
    m_listWidget = new QListWidget();
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    // 功能区
    QHBoxLayout *layoutFunction = new QHBoxLayout();
    pushButtonDownloadSelect = new QPushButton(tr("下载选中"));
    pushButtonDownloadSelect->setDisabled(true);
    layoutFunction->addWidget(pushButtonDownloadSelect);

    layout->addLayout(layoutFunction);
    layout->addWidget(m_listWidget);

    connect(m_listWidget, &QListWidget::currentRowChanged, this, [&](int row)
    {
        // ListWidget clear 时 row 为 -1！！！
        if(row != -1)
            emit itemCurrent(m_imgInfoList[row].absoluteFilePath());
    });
}


void ImageList::setImgPath(const QString& path)
{
    clearImgList();

    QDir directory(path);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";
    m_imgInfoList = directory.entryInfoList(filters, QDir::Files);

    foreach (const auto &imgInfo, m_imgInfoList) {
        QPixmap pixmap(imgInfo.absoluteFilePath());

        QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap), imgInfo.fileName());
        // item->setSizeHint(QSize(100, 80));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // 添加复选框
        item->setCheckState(Qt::Unchecked);                      // 默认不选中
        m_listWidget->addItem(item);
        item->setToolTip(         "文件名  ：" + imgInfo.fileName()+
                    "分辨率  ：" + QString::number(pixmap.width()) + " X " + QString::number(pixmap.height()) + '\n' +
                    "文件大小：" + QString::number(static_cast<double>(imgInfo.size()) / (1024 * 1024), 'g', 2) + " MB" + '\n' +
                    "存储路径：" + imgInfo.filePath() + '\n' +
                    "创建时间：" + imgInfo.fileTime(QFileDevice::FileBirthTime).toString("yyyy-MM-dd HH:mm:ss"));
    }

    if(m_imgInfoList.empty())
    {
        emit itemCurrent("");
    }
    else
    {
        m_listWidget->setCurrentRow(0);
        emit itemCurrent(m_imgInfoList[0].absoluteFilePath());
    }
}


void ImageList::clearImgList()
{
    m_listWidget->clear();
    m_imgInfoList.clear();
}

