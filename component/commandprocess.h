#ifndef COMMANDPROCESS_H
#define COMMANDPROCESS_H

#include <QThread>
#include <QProcess>

/**
 * @brief The CommandProcess class
 * 一个简单的运行命令行程序的类，基于QProcess实现
 */
class CommandProcess : public QObject
{
    Q_OBJECT

public:
    enum LOG{
        SUCCESS = 0,
        ERROR,
        INFO
    };

    explicit CommandProcess(QObject *parent = nullptr);


    /**
     * @brief CommandProcess 初始化程序名称、参数列表
     * @param program        程序名称
     * @param args           参数列表
     * @param workDirectory  运行目录
     */
    CommandProcess(const QString& program, const QStringList& args, QString workDirectory);

    ~CommandProcess();

    QString& getProgram();
    QStringList& getArgs();
    QProcess* getProcess();

    /**
     * @brief isRunning 当前程序结束,返回true
     * @return
     */
    bool isRunning() const;

    /**
     * @brief wrapperHTML  对输入数据使用HTML标签包裹
     * @param str          被包裹的字符串
     * @param success      rue表示成功是绿色，false表示失败是红色
     * @return             返回HTML包裹后的字符串
     */
    QString wrapperHTML(QString str, LOG log=LOG::SUCCESS);

signals:
    /**
     * @brief resultReady 发送程序输出的内容
     * @param result      程序的输出
     */
    void resultReady(QString result);

    /**
     * @brief cmdFinish 程序运行完成
     */
    void cmdFinish();

    /**
     * @brief cmdError 程序运行失败，传递失败信息
     */
    void cmdError(QProcess::ProcessError);

public slots:
    /**
     * @brief run 程序在这个函数里运行
     */
    void run() const;

    /**
     * @brief abort 终止程序
     */
    void abort() const;


private slots:

    //处理标准输出和错误输出
    void onProcessReadOutput();
    void onProcessReadError();

    //处理运行错误
    void onProcessError(QProcess::ProcessError);

    //展示运行状态变化
    void onProcessStateChanged(QProcess::ProcessState);

    //展示退出情况
    void onProcessExitState(int, QProcess::ExitStatus);

private:

    QProcess *m_process = nullptr;
    QString m_program;
    QStringList m_args;
    QString m_workDirectory;

};

#endif // COMMANDPROCESS_H
