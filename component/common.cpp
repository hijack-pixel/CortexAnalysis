#pragma execution_character_set("utf-8")
#include "common.h"
#include <QDebug>


QSettings globalSettings(QSettings::IniFormat, QSettings::UserScope, "BigData Lab", "Mouse Brain Analysis");

const QMap<QString, QString> imgTitleMap = {
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
    {"mouse",                   "小鼠"},
    {"ROI",                     "感兴趣区域"},
    {"ROI1 to other",           "感兴趣区域一至其余区域"},
    {"ROI2 to other",           "感兴趣区域二至其余区域"},

    {"Registration2ConnectMatrix",  "仿射变换的大脑配准和构建连通性矩阵"},
    {"cMatrix2NetGraph",            "连通矩阵数据显示网络图"},
    {"Timecourse2Spectrum",         "时间序列数据转换功率谱"},
    {"SpatialCorrMap",              "空间相关性图"},
    {"TimeCorrMap",                 "时间相关性图"},
    {"Quant4SNR",                   "信噪比成像"},
};


const QMap<Step, QString> titleMap = {
    {Step::STEP1, "大脑配准和ROI时序分析"},
    {Step::STEP2, "连通性矩阵和网络图谱"},
    {Step::STEP3, "不同频率的功率谱分析"},
    {Step::STEP4, "稳定相关图分析"},
    {Step::STEP5, "特定ROI的连通性分析"},
    {Step::STEP6, "成像的信噪比分析"},
};


extern void printGlobalSettings()
{
    qDebug() << "====================Settings=======================";
    QStringList keys = globalSettings.childKeys();
    foreach (const QString &key, keys)
    {
        qDebug() << key << ":" << globalSettings.value(key).toString();
    }
    qDebug() << "===================================================";
}


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
        // qDebug() << list[i] << titleMap[list[i]];
        if(imgTitleMap.contains(list[i]))
        {
            result << imgTitleMap[list[i]] + " ";
        }
        else if(list[i].contains("mouse count"))
        {
            QString s = list[i].replace("mouse count", "").split(' ', Qt::SkipEmptyParts).back();
            result << imgTitleMap["mouse count"] + s + " ";
        }
        else
        {
            result << list[i] + " ";
        }
    }
    return result.join("");
}


extern bool deleteFolderContent(const QString &folderPath) {
    QDir dir(folderPath);
    if (!dir.exists()) {
        qWarning() << "Directory does not exist:" << folderPath;
        return false;
    }

    // 获取文件夹内所有文件和子文件夹的信息列表
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

    // 遍历文件信息列表，先删除文件和子文件夹的内容，然后删除子文件夹
    for (const QFileInfo &fileInfo : fileInfoList) {
        if (fileInfo.isDir()) {
            // 如果是文件夹，递归删除其内容
            if (!deleteFolderContent(fileInfo.absoluteFilePath())) {
                return false;
            }
            // 删除空文件夹
            if (!dir.rmdir(fileInfo.fileName())) {
                qWarning() << "Failed to remove directory:" << fileInfo.fileName();
                return false;
            }
        } else {
            // 如果是文件，直接删除
            if (!dir.remove(fileInfo.fileName())) {
                qWarning() << "Failed to remove file:" << fileInfo.fileName();
                return false;
            }
        }
    }
    return true;
}

