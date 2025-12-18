#include "./configsaver.h"
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDir>

// 构造函数
ConfigSaver::ConfigSaver() {}


// 将枚举值转换为字符串
QString ConfigSaver::stepToString(Step step) {
    switch (step) {
    case STEP1: return "step1";
    case STEP2: return "step2";
    case STEP3: return "step3";
    case STEP4: return "step4";
    case STEP5: return "step5";
    case STEP6: return "step6";
    case SETTINGS:  return "settings";
    default: return "unknown";
    }
}


// 将字符串转换为枚举值
Step ConfigSaver::stringToStep(const QString& stepString) {
    if (stepString == "step1") return STEP1;
    if (stepString == "step2") return STEP2;
    if (stepString == "step3") return STEP3;
    if (stepString == "step4") return STEP4;
    if (stepString == "step5") return STEP5;
    if (stepString == "step6") return STEP6;
    if (stepString == "settings") return SETTINGS;
    return UNKNOWN;
}


// 递归函数将QVariant转换为QJsonValue
QJsonValue ConfigSaver::variantToJsonValue(const QVariant &variant) {
    switch (variant.type())
    {
    case QVariant::Map:
    {
        QJsonObject jsonObject;
        QVariantMap map = variant.toMap();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it)
        {
            jsonObject[it.key()] = variantToJsonValue(it.value());
        }
        return QJsonValue(jsonObject);
    }
    case QVariant::List:
    {
        QJsonArray jsonArray;
        QVariantList list = variant.toList();
        for (const QVariant &item : list)
        {
            jsonArray.append(variantToJsonValue(item));
        }
        return QJsonValue(jsonArray);
    }
    case QVariant::UserType:
    {
        // 添加对QMap<QString, QList<QString>>的处理
        if (variant.canConvert<QMap<QString, QList<QString>>>())
        {
            qDebug() << "<QMap<QString, QList<QString>>>" << "<QMap<QString, QList<QString>>>";
            QJsonObject jsonObject;
            QMap<QString, QList<QString>> map = variant.value<QMap<QString, QList<QString>>>();
            for (auto it = map.constBegin(); it != map.constEnd(); ++it)
            {
                QJsonArray jsonArray;
                for (const QString &str : it.value())
                {
                    jsonArray.append(str);
                }
                jsonObject[it.key()] = jsonArray;
                qDebug() << it.key() << it.value();
            }
            return QJsonValue(jsonObject);
        }
        // 添加对QMap<QString, QVector<QVector<int>>>的处理
        else if (variant.canConvert<QMap<QString, QVector<QVector<int>>>>())
        {
            QJsonObject jsonObject;
            QMap<QString, QVector<QVector<int>>> map = variant.value<QMap<QString, QVector<QVector<int>>>>();
            for (auto it = map.constBegin(); it != map.constEnd(); ++it)
            {
                QJsonArray jsonArray;
                for (const QVector<int> &vector : it.value())
                {
                    QJsonArray intArray;
                    for (int i : vector)
                    {
                        // qDebug() << "正在保存" << i ;
                        intArray.append(i);
                    }
                    jsonArray.append(intArray);
                }
                jsonObject[it.key()] = jsonArray;
            }
            return QJsonValue(jsonObject);
        }
        // 添加对 QMap<QString, QVector<float>> 的处理
        else if (variant.canConvert<QMap<QString, QVector<float>>>())
        {
            QJsonObject jsonObject;
            QMap<QString, QVector<float>> map = variant.value<QMap<QString, QVector<float>>>();

            for (auto it = map.constBegin(); it != map.constEnd(); ++it)
            {
                QJsonArray jsonArray;

                // 遍历 QVector<float> 中的每个 float 值
                for (const float &value : it.value())
                {
                    jsonArray.append(value);
                    // qDebug() << "正在保存" << value;
                }

                jsonObject[it.key()] = jsonArray;
            }

            return QJsonValue(jsonObject);
        }
        // 添加对QMap<QString, QString>的处理
        else if (variant.canConvert<QMap<QString, QString>>())
        {
            QJsonObject jsonObject;
            QMap<QString, QString> map = variant.value<QMap<QString, QString>>();
            for (auto it = map.constBegin(); it != map.constEnd(); ++it)
            {
                jsonObject[it.key()] = variantToJsonValue(it.value());
                // qDebug() << it.key() << it.value();
            }
            return QJsonValue(jsonObject);
        }
        // 添加对QList<QString>的处理
        else if (variant.canConvert<QList<QString>>())
        {
            QJsonArray jsonArray;
            QList<QString> stringList = variant.value<QList<QString>>();
            for (const QString &str : stringList)
            {
                jsonArray.append(str);
            }
            return QJsonValue(jsonArray);
        }
        // 如果是其他自定义类型，继续递归转换
        else
        {
            return QJsonValue::fromVariant(variant);
        }
    }
    default:
        return QJsonValue::fromVariant(variant);
    }
}


// 成员函数实现
void ConfigSaver::saveConfig(const QMap<Step, QMap<QString, QVariant>>& config, const QString& savePath) {

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



// 从JSON文件加载配置
bool ConfigSaver::loadConfig(QMap<Step, QMap<QString, QVariant>>& config, const QString& loadPath) {
    QFile file(loadPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file for reading:" << file.errorString() << file.fileName();
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Invalid JSON document";
        return false;
    }

    QJsonObject rootObject = doc.object();
    for (auto it = rootObject.constBegin(); it != rootObject.constEnd(); ++it) {
        Step step = stringToStep(it.key());
        if (step == UNKNOWN) {
            qDebug() << "Unknown step:" << it.key();
            continue;
        }

        QMap<QString, QVariant> stepSettings;
        QJsonObject stepObject = it.value().toObject();
        for (auto sit = stepObject.constBegin(); sit != stepObject.constEnd(); ++sit) {
            stepSettings[sit.key()] = sit.value().toVariant();
        }
        config[step] = stepSettings;
    }

    return true;
}



