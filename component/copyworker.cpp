#include "qpushbutton.h"
#pragma execution_character_set("utf-8")

#include "copyworker.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QtConcurrent>
#include <QMetaObject>
#include <QDebug>
#include <QMessageBox>
#include <QTextEdit>
#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QLabel>

#include "common.h"

CopyWorker::CopyWorker(QWidget *parent)
    : QObject(parent)
    , m_parentWidget(parent)
{
}

CopyWorker::~CopyWorker()
{
    if (m_dialog) {
        m_dialog->deleteLater();
        m_dialog = nullptr;
    }
    if (m_watcher) {
        m_watcher->deleteLater();
        m_watcher = nullptr;
    }

    deleteLater();
    qDebug() << "CopyWorker 析构";
}

// 静态工具函数（线程安全，纯计算）
QStringList CopyWorker::detectConflictingTargets(const QList<QPair<QString, QString>> &tasks)
{
    QStringList conflicts;

    for (const auto &task : tasks) {
        const QString &targetPath = task.second;
        QFileInfo targetInfo(targetPath);

        if (!targetInfo.exists()) {
            continue; // 不存在 → 无冲突
        }

        if (targetInfo.isFile()) {
            // 文件存在 → 冲突
            conflicts.append(task.first); // 记源路径，用于取 fileName
        } else if (targetInfo.isDir()) {
            // 目录存在 → 检查是否为空
            QDir dir(targetPath);
            // 排除 . 和 .. 后仍有项 → 非空
            QStringList entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
            if (!entries.isEmpty()) {
                conflicts.append(task.first); // 非空目录 → 冲突
            }
            // 若为空目录 → 不冲突，允许直接写入
        }
    }

    // 提取文件名用于显示（不暴露路径）
    QStringList conflictNames;
    for (const QString &srcPath : conflicts) {
        conflictNames.append(QFileInfo(srcPath).fileName());
    }
    return conflictNames;
}

void CopyWorker::start(const QList<QPair<QString, QString>> &tasks,
                       const QStringList& excludeFileName,
                       const QStringList& excludeFolderName)
{
    if (tasks.isEmpty()) {
        emit finished(false, {});
        deleteLater();
        return;
    }

    // 主线程同步检测冲突========================================
    QDialog waitDlg(m_parentWidget);
    waitDlg.setWindowTitle(tr("请稍候"));
    waitDlg.setModal(true);
    waitDlg.resize(500, 300);

    QLabel *label = new QLabel(tr("收集文件列表中..."), &waitDlg);
    label->setStyleSheet("background:transparent; font-size:14");
    label->setAlignment(Qt::AlignCenter);
    QVBoxLayout *lay = new QVBoxLayout(&waitDlg);
    lay->addWidget(label);

    waitDlg.show();
    qApp->processEvents();               // 立即刷新

    QStringList repeatNames = detectConflictingTargets(tasks);

    waitDlg.accept();                    // 关闭

    // 用户判断是否覆盖文件=========================================
    bool allowCover = false;
    if (!repeatNames.isEmpty()) {
        QDialog dialog(m_parentWidget);
        dialog.setWindowTitle(tr("冲突"));
        dialog.setModal(true);
        dialog.resize(500, 300);

        QVBoxLayout *vbox = new QVBoxLayout(&dialog);
        QString msg = tr("以下 %1 项已存在，是否覆盖？").arg(repeatNames.size());
        QLabel *label = new QLabel(msg);
        vbox->addWidget(label);

        QTextEdit *edit = new QTextEdit;
        edit->setPlainText(repeatNames.join("\n"));
        edit->setReadOnly(true);
        vbox->addWidget(edit);

        QDialogButtonBox *btns = new QDialogButtonBox(
            QDialogButtonBox::Yes | QDialogButtonBox::Ignore | QDialogButtonBox::Abort);
        btns->button(QDialogButtonBox::Yes)->setText(tr("覆盖"));
        btns->button(QDialogButtonBox::Ignore)->setText(tr("跳过"));
        btns->button(QDialogButtonBox::Abort)->setText(tr("取消"));
        vbox->addWidget(btns);
        int userChoice = -1;

        QObject::connect(btns, &QDialogButtonBox::clicked, [&](QAbstractButton *btn) {
            if (btn == btns->button(QDialogButtonBox::Yes))
            {
                userChoice = QDialogButtonBox::Yes;
                dialog.done(QDialogButtonBox::Yes);
            }
            else if (btn == btns->button(QDialogButtonBox::Ignore))
            {
                userChoice = QDialogButtonBox::Ignore;
                dialog.done(QDialogButtonBox::Ignore);
            }
            else if (btn == btns->button(QDialogButtonBox::Abort))
            {
                userChoice = QDialogButtonBox::Abort;
                dialog.done(QDialogButtonBox::Abort);
            }
        });

        dialog.exec();  // 阻塞等待用户选择
        if (userChoice == QDialogButtonBox::Yes)
        {
            allowCover = true;
        }
        else if (userChoice == QDialogButtonBox::Ignore)
        {
            allowCover = false;
        }
        else if (userChoice == QDialogButtonBox::Abort)
        {
            emit finished(false, {});
            deleteLater();
            return;
        }
    }

    // 创建模态进度对话框（主线程）==============================================================================
    m_dialog = new QProgressDialog(tr("正在导出..."), tr("取消"), 0, 0, m_parentWidget);
    m_dialog->setWindowTitle(tr("导出进度"));
    m_dialog->setWindowModality(Qt::WindowModal);
    m_dialog->setMinimumDuration(100);
    m_dialog->setMaximum(tasks.size());
    m_dialog->setValue(0);
    QCoreApplication::processEvents();        // 立刻画出来

    // 连接进度对话框取消信号
    connect(m_dialog, &QProgressDialog::canceled, this, &CopyWorker::onCanceled);

    // 连接进度更新槽（必须用 QueuedConnection 确保在主线程执行）
    connect(this, &CopyWorker::progressUpdate,
            this, &CopyWorker::onProgressUpdate, Qt::QueuedConnection);

    // 创建 watcher（用于监听异步任务）
    m_watcher = new QFutureWatcher<void>(this);
    connect(m_watcher, &QFutureWatcher<void>::finished, this, &CopyWorker::onCopyFinished);

    // 启动后台任务
    CopyTaskParams p;
    p.tasks = tasks;
    p.receiver = QPointer<CopyWorker>(this);;
    p.excludeFileName = excludeFileName;
    p.excludeFolderName = excludeFolderName;
    p.allowCover = allowCover;
    p.cancelFlag = &m_cancelRequested;
    m_watcher->setFuture(
        QtConcurrent::run(
            &CopyWorker::copyTasksImpl, p)
        );
}

// ========== 静态后台任务 ==========
void CopyWorker::copyTasksImpl(const CopyTaskParams &params)
{
    auto worker = params.receiver;          // 拿一份 QPointer
    if (!worker) {
        QMessageBox::critical(nullptr, tr("错误"), tr("CopyWorker 已销毁，放弃发送进度/结果"), QMessageBox::Close);
        return;
    }

    auto emitProgress = [worker](int cur, int total, const QString &item) {
        if (!worker) return;
        QMetaObject::invokeMethod(worker.data(),
                                  "progressUpdate",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, cur),
                                  Q_ARG(int, total),
                                  Q_ARG(QString, item));
    };

    auto emitFinished = [worker](bool success, const QStringList &failedItems) {
        if (!worker) return;
        QMetaObject::invokeMethod(worker.data(),
                                  "finished",
                                  Qt::QueuedConnection,
                                  Q_ARG(bool, success),
                                  Q_ARG(QStringList, failedItems));
    };

    QStringList failed;
    int total = params.tasks.size();

    for (int i = 0; i < total; ++i) {
        if (*params.cancelFlag) {
            qDebug() << "Copy canceled at" << i;
            break;
        }

        const auto &task = params.tasks[i];
        QString src = task.first;
        QString dst = task.second;

        QFileInfo srcInfo(src);
        QString name = srcInfo.fileName();

        qDebug() << tr("CopyWorker task 进行中 [%1/%2] %3 %4")
                        .arg(QString::number(i + 1))
                        .arg(QString::number(total))
                        .arg(src)
                        .arg(dst);

        emitProgress(i , total, name);

        if (!recursiveCopy(src, dst, params.excludeFileName, params.excludeFolderName, params.allowCover)) {
            failed.append(name);
        }

        emitProgress(i + 1, total, name);
    }

    // 保存失败列表（通过 invokeMethod 传回主线程）
    // 传回失败列表
    if (worker) {
        QMetaObject::invokeMethod(worker.data(),
                                  "setFailedItems",
                                  Qt::QueuedConnection,
                                  Q_ARG(QStringList, failed));
    }

    // 触发 finished
    qDebug() << "copyTasks 结束，发送 finish 信号";
    emitFinished(true, failed);
}

// ========== 主线程槽 ==========
void CopyWorker::onProgressUpdate(int current, int total, const QString item)
{
    if (m_dialog)
    {
        qDebug() << tr("更新复制进度： [%2/%3]：%1").arg(item).arg(current).arg(total);
        m_dialog->setValue(current);
        m_dialog->setLabelText(tr("正在导出 [%2/%3]：%1\n")
                                   .arg(item).arg(current).arg(total));
    }
}

void CopyWorker::onCanceled()
{
    m_cancelRequested = true;
    if (m_dialog) {
        qDebug() << "Export canceled by user.";
        m_dialog->setLabelText(tr("正在取消..."));
        m_dialog->setCancelButtonText(QString()); // 禁用多次点击
        m_dialog->setMinimum(0);
        m_dialog->setMaximum(0); // 转为忙状态
    }
}

// 弹框展示复制 成功与否
void CopyWorker::onCopyFinished()
{
    // 把所有 Queued 调用落盘
    QCoreApplication::sendPostedEvents(nullptr, QEvent::MetaCall);
    QCoreApplication::processEvents();

    m_dialog->hide();

    if (m_failedItems.isEmpty())
    {
        QMessageBox::information(nullptr, "成功", "导出完成！");
        qDebug() << tr("Export successfully");
    }
    else
    {
        QMessageBox::warning(nullptr, "部分失败", "失败项：\n" + m_failedItems.join("\n"));
        qDebug() << tr("Part of the export was successfully saved to the folder, fail items list below:\n")
                 << m_failedItems.join("\n");
    }

    qDebug() << "CopyWorker 复制完成";
}

// 供后台线程调用的槽（保存失败项）
void CopyWorker::setFailedItems(const QStringList &list)
{
    m_failedItems = list;
}
