#pragma execution_character_set("utf-8")
#include "commandprocess.h"

#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>

CommandProcess::CommandProcess(QObject *parent): QObject(parent) {};

CommandProcess::CommandProcess(const QString& program, const QStringList& args, QString workDirectory)
{
    m_process = new QProcess(this);
    m_program = program;
    m_args = args;
    m_workDirectory = workDirectory;

    // 通过信号队列（queued connection）传递需要注册
    qRegisterMetaType<QProcess::ProcessError>();

    connect(m_process, SIGNAL(readyRead()), this, SLOT(onProcessReadOutput()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(onProcessReadError()));

    connect(m_process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(onProcessStateChanged(QProcess::ProcessState)));
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onProcessError(QProcess::ProcessError)));

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CommandProcess::onProcessExitState);
}


CommandProcess::~CommandProcess()
{
    if(m_process->state() != QProcess::NotRunning)
    {
        m_process->kill();
        m_process->waitForFinished();  //等待程序关闭
        qDebug() << QString("Close %1 %2").arg(m_program, m_args.join(' '));
    }
}


QString& CommandProcess::getProgram()
{
    return m_program;
}


QStringList& CommandProcess::getArgs()
{
    return m_args;
}


QProcess* CommandProcess::getProcess()
{
    return m_process;
}

bool CommandProcess::isRunning() const
{
    if(m_process->state() == QProcess::NotRunning)
        return false;
    else
        return true;
}


QString CommandProcess::wrapperHTML(QString str, LOG log)
{
    switch(log)
    {
    case LOG::SUCCESS:
        return QString("<span style='font-size: 10pt; color: green;'>%1</span>").arg(str);
    case LOG::ERROR:
        return QString("<span style='font-size: 10pt; color: red; '>%1</span>").arg(str);
    case LOG::INFO:
        return QString("<span style='font-size: 10pt; color: black; '>%1</span>").arg(str);
    }
}


void CommandProcess::run() const
{
    if(!m_workDirectory.isEmpty())
    {
        m_process->setWorkingDirectory(m_workDirectory);
    }

    m_process->start(m_program, m_args);  //process在thread中阻塞运行

    m_process->waitForStarted();   //等待程序启动

    // m_process->waitForFinished();  //等待程序关闭
}


void CommandProcess::abort() const
{
    QString pid = QString::number(m_process->processId());
    qDebug() << QString("Attemptting to kill(running state:%1, pid:%2): ")
                    .arg(isRunning()?"Running":"Not Running", pid) << m_program << m_args;
    if(isRunning())
    {
        {
        // m_process->terminate();

        // if (!m_process->waitForFinished(500))
        // {
        //     qDebug() << "terminate failed, force kill: " << m_program << m_args;
        //     m_process->kill();
        // }

        // if(m_process->waitForFinished(500))
        //     qDebug() << "Kill successfull: " << m_program << m_args;

        // // 杀掉子进程，octave.exe会唤起 conhost.exe octave-gui.exe，杀掉octave-gui.exe，conhost.exe自动终止
        // QProcess processKillChild;
        // connect(&processKillChild, &QProcess::readyReadStandardOutput, this, [&](){
        //     QProcess *process = qobject_cast<QProcess *>(sender());
        //     if (process)
        //     {
        //         QProcess taskKillProcess;
        //         QString output = process->readAllStandardOutput();// 输出示例: "ProcessId\n9324\n"
        //         QStringList lines = output.split("\n");
        //         qDebug() << "SubProcess found: " << lines.join(" ");
        //         foreach (const QString &line, lines)
        //         {
        //             if (line.startsWith("ProcessId"))
        //                 continue; // 跳过标题行

        //             if (!line.isEmpty()) // 解析 PID
        //             {
        //                 bool ok;
        //                 int pid = line.toInt(&ok);
        //                 if (ok)
        //                 {
        //                     taskKillProcess.start("taskkill", {"/PID", QString::number(pid), "/F" });
        //                     taskKillProcess.waitForFinished();
        //                     qDebug() << QString(taskKillProcess.readAllStandardOutput());
        //                     qDebug() << " kill octave-gui.exe PID:" << pid;
        //                 }
        //             }
        //         }
        //     }});
        // QString program = "wmic";
        // QStringList arguments;
        // arguments << "process"
        //           << "where" << QString("ParentProcessId=%1 and Name='%2'").arg(pid).arg("octave-gui.exe")
        //           << "get" << "ProcessId";
        // processKillChild.start(program, arguments);
        // processKillChild.waitForFinished();
        }

        m_process->terminate();
        QProcess processTaskill;
        QString program = "taskkill";
        QStringList arguments;
        arguments << "/PID" << pid << "/T" << "/F";
        processTaskill.start(program, arguments);
        processTaskill.waitForFinished();
        qDebug() << QString::fromLocal8Bit(processTaskill.readAllStandardOutput());
        qDebug() << QString::fromLocal8Bit(processTaskill.readAllStandardError());
    }
    else
    {
        qDebug() << "NotRunning, Kill error: " << m_program << m_args;
    }
}


void CommandProcess::onProcessReadOutput()
{
    QByteArray data = m_process->readLine();

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QString output = codec->toUnicode(data);

    emit resultReady(wrapperHTML(output));
    // qDebug() << QString("%1 %2 output: %3").arg(m_program, m_args.join(' '), output);
}


void CommandProcess::onProcessReadError()
{
    QByteArray data = m_process->readAllStandardError();

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QString output = codec->toUnicode(data);
    output += "<br><br>";

    emit resultReady(wrapperHTML(output, LOG::ERROR));
    qDebug() << QString("%1 %2 output: %3").arg(m_program, m_args.join(' '), output);
}


void CommandProcess::onProcessStateChanged(QProcess::ProcessState state)
{
    switch(state)
    {
    case QProcess::NotRunning:
        qDebug() << QString("NotRunning %1 %2 (PID: %3)").arg(m_program, m_args.join(' '), QString::number(m_process->processId()));
        break;
    case QProcess::Starting:
        qDebug() << QString("Starting %1 %2 (PID: %3)").arg(m_program, m_args.join(' '), QString::number(m_process->processId()));
        break;
    case QProcess::Running:
        emit resultReady(wrapperHTML(QString("Running %1 %2").arg(m_program, m_args.join(' ')), LOG::INFO));
        qDebug() << QString("Running %1 %2 (PID: %3)").arg(m_program, m_args.join(' '), QString::number(m_process->processId()));
        break;
    }
}


void CommandProcess::onProcessError(QProcess::ProcessError error)
{
    emit cmdError(error);
    switch(error)
    {
    case QProcess::FailedToStart:
        qDebug() << "FailedToStart";
        break;
    case QProcess::Crashed:
        qDebug() << "Crashed";
        break;
    case QProcess::Timedout:
        qDebug() << "Timedout";
        break;
    case QProcess::WriteError:
        qDebug() << "WriteError";
        break;
    case QProcess::ReadError:
        qDebug() << "ReadError";
        break;
    case QProcess::UnknownError:
        qDebug() << "UnknownError";
        break;
    default:
        qDebug() << "UnknownError";
        break;
    }
}


void CommandProcess::onProcessExitState(int exitCode, QProcess::ExitStatus exitStatus)
{
    switch(exitStatus)
    {
    case QProcess::NormalExit:
        emit cmdFinish();
        emit resultReady(wrapperHTML(QString("NormalExit %1 %2 <br><br>").arg(m_program, m_args.join(' ')), LOG::INFO));
        qDebug() << QString("NormalExit %1 %2").arg(m_program, m_args.join(' '));
        break;
    case QProcess::CrashExit:
        emit cmdError(QProcess::Crashed);
        emit resultReady(wrapperHTML(QString("CrashExit %1 %2 <br><br>").arg(m_program, m_args.join(' ')), LOG::ERROR));
        qDebug() << QString("CrashExit %1 %2").arg(m_program, m_args.join(' '));
        break;
    }
}

