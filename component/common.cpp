#pragma execution_character_set("utf-8")
#include "common.h"
#include <QDebug>



const QMap<QString, QString> titleMap = {
    {"mouse count",             "小鼠"},
    {"Connectivity matrix",     "连通性矩阵"},
    {"ptROI",                   "感兴趣区域点"},
    {"registration",            "配准"},
    {"Average",                 "平均性"},
    {"Diff",                    "差异性"},
    {"CON mice cMartix",        "对照组小鼠矩阵"},
    {"MUT mice cMartix",        "突变组小鼠矩阵"},
    {"MUT CON mice cMartix",    "对照突变组小鼠结合矩阵"},
    {"Power spectrum CON mice", "CON组小鼠功率谱"},
    {"Registration2ConnectMatrix", "仿射变换的大脑配准和构建连通性矩阵"},
    {"cMatrix2NetGraph",        "连通矩阵数据显示网络图"},
    {"Timecourse2Spectrum",     "时间序列数据转换功率谱"},
};


// 递归删除布局内的控件
extern void deleteLayout(QLayout* layout)
{
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
extern void clearWidget(QWidget* widget)
{
    QLayout* layout = widget->layout();
    if (layout)
    {
        deleteLayout(layout);
        widget->setLayout(nullptr); // 将布局设置为nullptr，以确保不再使用该布局
    }
}


// 将英文文件名换为中文
extern QString convertImgTitle(QString title)
{
    int lastDotIndex = title.lastIndexOf('.');
    if (lastDotIndex != -1) {
        title = title.left(lastDotIndex);
    }

    QStringList result;
    QStringList list = title.split('_', Qt::SkipEmptyParts);

    for(int i = 0; i < list.length(); ++i)
    {
        qDebug() << list[i] << titleMap[list[i]];
        if(titleMap.contains(list[i]))
        {
            result << titleMap[list[i]] + " ";
        }
        else if(list[i].contains("mouse count"))
        {
            QString s = list[i].replace("mouse count", "").split(' ', Qt::SkipEmptyParts).back();
            result << titleMap["mouse count"] + s + " ";
        }
        else
        {
            result << list[i] + " ";
        }
    }
    return result.join("");
}
