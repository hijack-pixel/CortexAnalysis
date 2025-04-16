#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include "common.h"
#include <QWidget>
#include <QListWidget>
#include <QCheckBox>
#include <QIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QPushButton>


/**
 * @brief The FileListWidget class
 * 用于显示文件列表，带多选框和配对的按钮功能布局，在当前显示item变化时发出包含文件路径以及路径信号（item UserRole中）
 */
class FileListWidget : public QWidget
{
    Q_OBJECT
public:
    // explicit FileListWidget();

    explicit FileListWidget(QWidget *parent = nullptr);

    void init();

    void setFilePath(const QString& path, FileType fileType = FileType::IMAGE);

    void clearFileList();

signals:
    // path，fileType保存在item的userRole与userRole+1，listWidget当前项改变时发送信号
    void itemCurrent(const QString& path, const FileType fileType);

    // 图标加载完成发送，没有图标加载也发送，这样加载时disable避免异步加载出错
    void iconLoadFinish();

public slots:
    void on_updateCheckState();

    void on_downloadSelect();

private:
    QListWidget *m_listWidget = nullptr;
    QPushButton *m_pushButtonDownloadSelect = nullptr;
    QCheckBox *m_checkBox = nullptr;

    QFileInfoList m_fileInfoList;

    std::atomic<int> m_loadingIconsCount;
};

#endif // FILELISTWIDGET_H
