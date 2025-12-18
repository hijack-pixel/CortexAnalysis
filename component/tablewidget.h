#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringList>
#include <QButtonGroup>
#include <QScrollArea>


/**
 * @brief The TableWidget class
 * 使用xlnt库实现xlsx文件预览，支持多sheet预览，文件过大使用本地应用打开。
 */
class TableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableWidget(QWidget *parent = nullptr);
    ~TableWidget();

public slots:
    void openFile();                 // 打开Excel文件
    void loadExcel(const QString &filePath);  // 载入Excel文件
    void showSheet(int sheetIndex);  // 显示选定的Sheet
    void openInSystemApp();          // 打开文件通过系统应用
    void clearContent();             // 清除内容

private:
    void showFileOpenButton();       // 显示打开文件按钮
    void hideFileOpenButton();       // 隐藏打开文件按钮

private:
    QTableWidget *m_tableWidget;             // 表格控件
    QPushButton *m_openButton;               // 打开文件按钮
    QVBoxLayout *m_layout;                   // 主布局
    QScrollArea *m_scrollArea;              // Sheet按钮滑动区域
    QHBoxLayout *m_sheetButtonLayout;       // 存放Sheet按钮的布局
    QButtonGroup *m_sheetButtonGroup;       // 存放Sheet按钮
    QString m_filePath;                      // 当前文件路径
    QStringList m_sheets;                    // 所有Sheet的名称
    int m_currentSheetIndex;                // 当前显示的Sheet索引

    const int MAX_ROW = 5000;
    const int MAX_COL = 100;
    const int MAX_FILE_SIZE = 1 * 1024 * 1024;   // 1 MB
};

#endif // TABLEWIDGET_H
