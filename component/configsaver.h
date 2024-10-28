#ifndef CONFIGSAVER_H
#define CONFIGSAVER_H

#include "./component/common.h"

#include <QMap>
#include <QVariant>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

/**
 * @brief The ConfigSaver class 用于递归保存自定义的config，类型为QMap<Step, QMap<QString, QVariant>>
 */
class ConfigSaver {
public:

    ConfigSaver();

    void saveConfig(const QMap<Step, QMap<QString, QVariant>>& config, const QString& savePath);

private:
    static QString stepToString(Step step);
    static QJsonValue variantToJsonValue(const QVariant &variant);
};

#endif // CONFIGSAVER_H
