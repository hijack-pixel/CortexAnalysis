#pragma execution_character_set("utf-8")
#include "settingsdialog.h"
#include "common.h"

#include <QDebug>

extern QSettings globalSettings;

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setMinimumSize(QSize(300, 223));
    setWindowTitle(tr("软件设置"));

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 添加语言选择
    QHBoxLayout *languageLayout = new QHBoxLayout();
    QLabel *languageLabel = new QLabel(tr("语言:"));
    QComboBox *languageComboBox = new QComboBox();
    languageComboBox->addItem(tr("英语"));
    languageComboBox->addItem(tr("中文"));
    // ... 可以添加更多语言
    languageLayout->addWidget(languageLabel);
    languageLayout->addWidget(languageComboBox);
    mainLayout->addLayout(languageLayout);

    // 添加主题选择
    QHBoxLayout *themeLayout = new QHBoxLayout();
    QLabel *themeLabel = new QLabel(tr("主题:"));
    QComboBox *themeComboBox = new QComboBox();
    themeComboBox->addItem(tr("浅色"));
    themeComboBox->addItem(tr("深色"));
    // ... 可以添加更多主题
    themeLayout->addWidget(themeLabel);
    themeComboBox->setCurrentIndex(0); // 设置默认主题
    themeLayout->addWidget(themeComboBox);
    mainLayout->addLayout(themeLayout);

    // 添加一个复选框来表示是否启用某些功能
    QCheckBox *enableFeatureCheckBox = new QCheckBox(tr("启用高级功能"));
    mainLayout->addWidget(enableFeatureCheckBox);

    // 添加按钮布局
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("确定"));
    QPushButton *cancelButton = new QPushButton(tr("取消"));
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonsLayout);

    // 连接按钮信号
    connect(okButton, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &SettingsDialog::reject);

    // 加载现有设置
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{

}


void SettingsDialog::loadSettings()
{

}

void SettingsDialog::saveSettings()
{
    globalSettings.setValue("languageIndex", 0);
    globalSettings.setValue("themeIndex", 0);
    globalSettings.setValue("enableFeature", 0);

}


void SettingsDialog::accept()
{
    saveSettings();
    done(QDialog::Accepted);
}


