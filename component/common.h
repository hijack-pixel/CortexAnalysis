#ifndef COMMON_H
#define COMMON_H

#include <QWidget>
#include <QLayout>
#include <QLayoutItem>
#include <QMap>

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


// 图片标题中专业名词的 中英文对照 map
extern const QMap<QString, QString> titleMap;


// 递归删除布局内的控件
extern void deleteLayout(QLayout* layout);


// 递归删除QWidget内的控件
extern void clearWidget(QWidget* widget);


// 将英文文件名换为中文
extern QString convertImgTitle(QString& title);


#endif // COMMON_H
