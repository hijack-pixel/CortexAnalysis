#ifndef COPYWORKER_H
#define COPYWORKER_H

#include <QObject>
#include <QPair>
#include <QStringList>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QPointer>

/*
注意：start 之后复制任务在后台运行，靠前台 dialog 模态框撑着！【所以 start 后面的代码是会继续执行的！】

收集 tasks： QList<QPair<QString, QString>>

启动导出（自动弹进度框、支持取消、完成后自清理）
new CopyWorker(this)->start(tasks);

提供链接信号槽
connect(worker, &CopyWorker::finished, this, [](bool success, const QStringList &failed) {
    if (success && failed.isEmpty()) {
        QMessageBox::information(nullptr, "成功", "导出完成！");
    } else if (!failed.isEmpty()) {
        QMessageBox::warning(nullptr, "部分失败", "失败项：\n" + failed.join("\n"));
    } else {
        // 被取消
        qDebug() << "Export canceled by user.";
    }
});
*/
class CopyWorker : public QObject
{
    Q_OBJECT

    struct CopyTaskParams {
        QList<QPair<QString, QString>> tasks;
        QPointer<CopyWorker> receiver = nullptr;
        QStringList excludeFileName;
        QStringList excludeFolderName;
        bool allowCover = false;
        volatile bool *cancelFlag = nullptr; // 或改用 future 取消
    };

public:
    explicit CopyWorker(QWidget *parent = nullptr);
    ~CopyWorker() override;

    // 启动任务（调用后本对象会 self-delete）
    void start(
        const QList<QPair<QString, QString>> &tasks,
        const QStringList& excludeFileName = {},
        const QStringList& excludeFolderName = {}
        );

    static QStringList detectConflictingTargets(const QList<QPair<QString, QString>> &tasks);

Q_INVOKABLE
    void setFailedItems(const QStringList &list);

signals:
    void progressUpdate(int current, int total, const QString item);
    void finished(bool success, const QStringList failedItems);

private slots:
    void onProgressUpdate(int current, int total, const QString item);
    void onCopyFinished();
    void onCanceled();

private:
    // 后台复制任务（静态，线程安全）
    static void copyTasksImpl(const CopyTaskParams &params);

    QWidget *m_parentWidget = nullptr;
    QProgressDialog *m_dialog = nullptr;
    QFutureWatcher<void> *m_watcher = nullptr;
    volatile bool m_cancelRequested = false;
    QStringList m_failedItems;
};

#endif // COPYWORKER_H
