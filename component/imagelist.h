#ifndef IMAGELIST_H
#define IMAGELIST_H

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
 * @brief The ImageList class 用于显示图片列表，带多选框和配对的按钮功能布局，在当前显示item变化时发出包含图片路径信号
 */
class ImageList : public QWidget
{
    Q_OBJECT
public:
    // explicit ImageList();

    explicit ImageList(QWidget *parent = nullptr);

    void init();

    void setImgPath(const QString& path);

    void clearImgList();

signals:
    void itemCurrent(const QString& path);

private:
    QListWidget *m_listWidget;
    QPushButton *pushButtonDownloadSelect;

    QFileInfoList m_imgInfoList;
};

#endif // IMAGELIST_H
