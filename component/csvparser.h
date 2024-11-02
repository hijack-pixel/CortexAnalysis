#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QString>
#include <QVector>

class CsvParser
{
public:
    CsvParser();
    ~CsvParser();

    bool parse(const QString &filePath);
    const QVector<QStringList>& getData() const;

private:
    QVector<QStringList> m_data; // 二维字符串数组保存CSV数据
};

#endif // CSVPARSER_H
