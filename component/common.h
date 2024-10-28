#ifndef COMMON_H
#define COMMON_H

#include <QWidget>
#include <QLayout>
#include <QLayoutItem>

/**
 * 这里保存了一些全局的申明以及通用的函数
 *
*/

// 枚举类型，用于修改qml light的颜色
enum LightColor{
    SUCCESS = 0,
    RUNNING,
    FAIL,
};

// 枚举类型，用于表示当前图形界面显示的是第几步
enum Step{
    STEP1 = 0,
    STEP2,
    STEP3,
    STEP4,
    STEP5,
    STEP6,
};


// 递归删除布局内的控件
void deleteLayout(QLayout* layout) {
    if (!layout)
    {
        return;
    }

    QLayoutItem* item;
    while ((item = layout->takeAt(0)))
    {
        if (item->widget())
        {
            delete item->widget();
        }
        else if (item->layout())
        {
            deleteLayout(item->layout());
        }
    }

    delete item;
    delete layout;
}


// 递归删除QWidget内的控件
void clearWidget(QWidget* widget)
{
    QLayout* layout = widget->layout();
    if (layout)
    {
        deleteLayout(layout);
        widget->setLayout(nullptr); // 将布局设置为nullptr，以确保不再使用该布局
    }
}



#endif // COMMON_H
