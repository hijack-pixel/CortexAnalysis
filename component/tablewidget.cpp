#include "tablewidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <xlnt/xlnt.hpp>
#include <QScrollBar>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QScrollArea>
#include <codecvt>
#include <QDebug>
#include <QProcess>

TableWidget::TableWidget(QWidget *parent)
    : QWidget(parent),
    m_currentSheetIndex(0)
{
    // 初始化布局
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(QMargins(0,0,0,0));
    m_layout->setSpacing(0);
    m_layout->setAlignment(Qt::AlignCenter);

    m_tableWidget = new QTableWidget(this);
    m_layout->addWidget(m_tableWidget);

    // 初始化m_sheetButtonGroup属性，设置单选
    m_sheetButtonGroup = new QButtonGroup(this);
    m_sheetButtonGroup->setExclusive(true);

    // 按钮用于加载Sheet内容
    QWidget *sheetButtonContainerWidget = new QWidget(this);
    sheetButtonContainerWidget->setContentsMargins((QMargins(0,0,0,0)));
    m_sheetButtonLayout = new QHBoxLayout();
    m_sheetButtonLayout->setAlignment(Qt::AlignLeft);
    m_sheetButtonLayout->setContentsMargins(QMargins(0,0,0,0));
    m_sheetButtonLayout->setSpacing(3);
    sheetButtonContainerWidget->setLayout(m_sheetButtonLayout);

    // sheet按钮放到横向滚动区域
    m_scrollArea = new QScrollArea();
    m_scrollArea->setContentsMargins(QMargins(0,0,0,0));
    m_scrollArea->setStyleSheet("QScrollArea {"
                              "    border: none;" // 移除边框
                              "    background: transparent;" // 设置背景透明
                              "}"
                              "QScrollArea > QWidget > QWidget {"
                              "    margin: 0px;" // 设置内边距为0
                              "}");
    m_scrollArea->setWidgetResizable(true); // 设置滚动区域内的控件可调整大小
    m_scrollArea->setWidget(sheetButtonContainerWidget);

    m_layout->addWidget(m_scrollArea);

    // 按钮用于打开文件
    m_openButton = new QPushButton("打开文件", this);
    m_openButton->setFixedSize(m_openButton->sizeHint()+QSize(10, 10));
    connect(m_openButton, &QPushButton::clicked, this, &TableWidget::openFile);
    m_layout->addWidget(m_openButton);

    m_scrollArea->setFixedHeight(m_openButton->sizeHint().height()+20);
    setLayout(m_layout);

    m_openButton->setHidden(true);
}

TableWidget::~TableWidget()
{
    // 析构函数清理
}

void TableWidget::clearContent()
{
    // 清空现有内容，展示控件
    m_tableWidget->clear();
    m_tableWidget->setHidden(true);
    m_scrollArea->setHidden(true);

    // 删除sheetButtonGroup中的所有按钮，并从布局中移除
    QList<QAbstractButton*> buttons = m_sheetButtonGroup->buttons();
    if (!buttons.isEmpty()) {
        foreach (QAbstractButton *button, buttons) {
            m_sheetButtonLayout->removeWidget(button);
            m_sheetButtonGroup->removeButton(button);
            button->disconnect();
            button->deleteLater();
        }
    }
}

void TableWidget::openFile()
{
    // loadExcel("E:\\TEMP\\MouseAnalysisData\\反馈建议\\导出表格数据范例\\STEP 2 联通网络\\Average原始矩阵数据.xlsx");return;
    QString filePath = QFileDialog::getOpenFileName(this, "选择Excel文件", "", "Excel Files (*.xlsx)");
    if (!filePath.isEmpty()) {
        loadExcel(filePath);
    }
}

void TableWidget::loadExcel(const QString &filePath)
{
    if(filePath.isEmpty()) {
        qDebug() << "LoadExcel path is NULL!";
        return;
    }
    m_filePath = filePath;

    /* ---------- 新增：文件大小限制 ---------- */
    QFileInfo fi(m_filePath);
    if (fi.size() > MAX_FILE_SIZE)
    {
        showFileOpenButton();           // 直接显示“用 Excel 打开”按钮
        return;                         // 不再走 xlnt 解析
    }
    /* --------------------------------------- */


    // 清空现有内容，展示控件
    m_tableWidget->clear();
    m_tableWidget->setHidden(false);
    m_scrollArea->setHidden(false);

    try {
        xlnt::workbook wb;
        // std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;     //解决中文路径问题
        // std::string str = convert.to_bytes(filePath.toStdWString());
        wb.load(filePath.toStdString());  // 载入xlsx文件

        // 获取所有Sheet
        m_sheets.clear();
        for (const auto& sheet : wb) {
            m_sheets.push_back(QString::fromStdString((sheet.title())));
        }

        // 删除sheetButtonGroup中的所有按钮，并从布局中移除
        QList<QAbstractButton*> buttons = m_sheetButtonGroup->buttons();
        if (!buttons.isEmpty()) {
            foreach (QAbstractButton *button, buttons) {
                m_sheetButtonLayout->removeWidget(button);
                m_sheetButtonGroup->removeButton(button);
                button->disconnect();
                button->deleteLater();
            }
        }

        // 创建按钮切换不同的Sheet
        for (int i = 0; i < m_sheets.size(); ++i) {
            QPushButton *sheetButton = new QPushButton(m_sheets[i], this);
            sheetButton->setCheckable(true);
            sheetButton->setFixedWidth(sheetButton->sizeHint().width());  // 依据内容固定宽度
            connect(sheetButton, &QPushButton::clicked, this, [this, i]() {
                showSheet(i);
            });
            m_sheetButtonLayout->addWidget(sheetButton);
            m_sheetButtonGroup->addButton(sheetButton);
            if(i == 0) sheetButton->click();  // 默认显示第一个Sheet
        }

    } catch (const std::exception &e) {
        QMessageBox::critical(this, "错误", "无法读取文件: " + QString(e.what()));
    }
}

void TableWidget::showSheet(int sheetIndex)
{
    if (sheetIndex < 0 || sheetIndex >= m_sheets.size()) return;

    m_currentSheetIndex = sheetIndex;

    xlnt::workbook wb;
    wb.load(m_filePath.toStdString());
    xlnt::worksheet ws = wb.sheet_by_title(m_sheets[m_currentSheetIndex].toStdString());

    // 清空现有内容
    m_tableWidget->clear();
    m_tableWidget->setRowCount(ws.highest_row());
    m_tableWidget->setColumnCount(ws.highest_column().index);

    // 设置表格内容
    for (int row = 1; row <= ws.highest_row(); ++row) {
        for (int col = 1; col <= ws.highest_column(); ++col) {
            auto cell = ws.cell(xlnt::cell_reference(col, row));
            if(xlnt::cell_type::number == cell.data_type())
            {
                // 如果是数字就转成double显示，默认的to_string会损失精度
                m_tableWidget->setItem(row - 1, col - 1, new QTableWidgetItem(QString::number(cell.value<double>())));
            }
            else
            {
                m_tableWidget->setItem(row - 1, col - 1, new QTableWidgetItem(QString::fromStdString(cell.to_string())));
            }
        }
    }

    // 如果表格太大，显示文件信息按钮
    if (ws.highest_row() > MAX_ROW || ws.highest_column() > MAX_COL) {
        showFileOpenButton();
    } else {
        hideFileOpenButton();
    }
}

void TableWidget::showFileOpenButton()
{
    m_tableWidget->setHidden(true);
    m_scrollArea->setHidden(true);
    m_openButton->setText("表格太大不支持预览，点击打开文件");
    m_openButton->setFixedSize(m_openButton->sizeHint()+QSize(10, 10));
    m_openButton->setHidden(false);
    m_openButton->disconnect();
    connect(m_openButton, &QPushButton::clicked, this, &TableWidget::openInSystemApp);
}

void TableWidget::hideFileOpenButton()
{
    m_tableWidget->setHidden(false);
    m_scrollArea->setHidden(false);
    m_openButton->setHidden(true);
    // m_openButton->setText("打开文件");
    // m_openButton->setFixedSize(m_openButton->sizeHint()+QSize(10, 10));
    // m_openButton->disconnect();
    // connect(m_openButton, &QPushButton::clicked, this, &TableWidget::openFile);
}

void TableWidget::openInSystemApp()
{
    QFileInfo fileInfo(m_filePath);
    if (QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()))) {
        qDebug() << "文件已通过系统应用打开！";
    }
    else {
        qDebug() << "无法通过系统应用打开文件，尝试在资源管理器中显示文件所在位置。";

        QString folderPath = fileInfo.absolutePath();
#ifdef Q_OS_WIN
        // Windows系统使用explorer命令
        bool opened = QProcess::startDetached("explorer", QStringList() << "/select," << QDir::toNativeSeparators(fileInfo.absoluteFilePath()));
#elif defined(Q_OS_MAC)
        // macOS系统使用open命令
        bool opened = QProcess::startDetached("open", QStringList() << "-R" << fileInfo.absoluteFilePath());
#elif defined(Q_OS_LINUX)
        // Linux系统可能需要根据具体的文件管理器来编写代码，这里以Nautilus为例
        bool opened = QProcess::startDetached("nautilus", QStringList() << "--select" << fileInfo.absoluteFilePath());
#endif

        if (opened) {
            qDebug() << "文件位置已在资源管理器中显示！";
        } else {
            qDebug() << "无法在资源管理器中显示文件位置！";
        }
    }
}
