#ifndef COMMON_H
#define COMMON_H

#include <QWidget>
#include <QLayout>
#include <QLayoutItem>
#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

/**
 * 这里保存了一些全局的申明以及通用的函数
 *
*/

// 枚举类型，用于修改qml light的颜色
enum LightColor
{
    SUCCESS = 0,
    RUNNING,
    FAIL,
};

// 枚举类型，用于表示当前图形界面显示的是第几步
enum Step
{
    STEP1 = 0,
    STEP2,
    STEP3,
    STEP4,
    STEP5,
    STEP6,
};


// 保存全局配置
extern QSettings globalSettings;

extern void printGlobalSettings();

// 图片标题中专业名词的 中英文对照 map
extern const QMap<QString, QString> imgTitleMap;


// 每一步分析的中文名
extern const QMap<Step, QString> titleMap;


// 递归删除布局内的控件
extern void deleteLayout(QLayout* layout);


// 递归删除QWidget内的控件
extern void clearWidget(QWidget* widget);


// 将英文文件名换为中文
extern QString convertImgTitle(QString& title);


// 删除文件夹内容
extern bool deleteFolderContent(const QString &folderPath);

#endif // COMMON_H
