#include "./configsaver.h"
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDir>

// 将枚举值转换为字符串
QString ConfigSaver::stepToString(Step step) {
    switch (step) {
    case STEP1: return "step1";
    case STEP2: return "step2";
    case STEP3: return "step3";
    case STEP4: return "step4";
    case STEP5: return "step5";
    case STEP6: return "step6";
    default: return "unknown";
    }
}

// 递归函数将QVariant转换为QJsonValue
QJsonValue ConfigSaver::variantToJsonValue(const QVariant &variant) {
    switch (variant.type()) {
    case QVariant::Map: {
        QJsonObject jsonObject;
        QVariantMap map = variant.toMap();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            jsonObject[it.key()] = variantToJsonValue(it.value());
        }
        return QJsonValue(jsonObject);
    }
    case QVariant::List: {
        QJsonArray jsonArray;
        QVariantList list = variant.toList();
        for (const QVariant &item : list) {
            jsonArray.append(variantToJsonValue(item));
        }
        return QJsonValue(jsonArray);
    }
    default:
        return QJsonValue::fromVariant(variant);
    }
}

// 构造函数
ConfigSaver::ConfigSaver() {}

// 成员函数实现
void ConfigSaver::saveConfig(const QMap<Step, QMap<QString, QVariant>>& config,const QString& savePath) {

    QFile file(savePath);
    qDebug() << "Save to: " << savePath;

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open file for writing:" << file.errorString() << file.fileName();
        return;
    }

    QJsonObject rootObject;
    for (auto it = config.constBegin(); it != config.constEnd(); ++it) {
        const Step& step = it.key();
        const QMap<QString, QVariant>& settings = it.value();
        QJsonObject stepObject;
        for (auto sit = settings.constBegin(); sit != settings.constEnd(); ++sit) {
            stepObject[sit.key()] = variantToJsonValue(sit.value());
        }
        rootObject[stepToString(step)] = stepObject;
    }

    QJsonDocument doc(rootObject);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    file.write(jsonData);
    file.close();
}
