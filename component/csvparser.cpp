#include "CsvParser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>

CsvParser::CsvParser() {}

CsvParser::~CsvParser() {}

bool CsvParser::parse(const QString &filePath) {
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "错误", "无法打开文件!");
        qDebug() << "无法打开文件";
        return false;
    }

    QTextStream in(&file);
    m_data.clear();
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',', Qt::SkipEmptyParts); // 使用逗号分割

        if(!fields.isEmpty())
            m_data.append(fields);
    }
    if(m_data.isEmpty())
    {
        QMessageBox::critical(nullptr, "错误", "文件为空！");
        return false;
    }


    int size = m_data[0].size();
    foreach (const auto& list, m_data)
    {
        if(list.size() != size)
        {
            QMessageBox::critical(nullptr, "错误", "每行数据量不相等！");
            return false;
        }
    }

    file.close();
    return true;
}

const QVector<QStringList>& CsvParser::getData() const {
    return m_data;
}
