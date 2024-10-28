#include "commandprocess.h"

#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>

CommandProcess::CommandProcess(QObject *parent): QObject(parent) {};

CommandProcess::CommandProcess(const QString& program, const QStringList& args)
{
    m_process = new QProcess(this);
    m_program = program;
    m_args = args;


    connect(m_process, SIGNAL(readyRead()), this, SLOT(onProcessReadOutput()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(onProcessReadError()));

    connect(m_process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(onProcessStateChanged(QProcess::ProcessState)));
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onProcessError(QProcess::ProcessError)));

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CommandProcess::onProcessExitState);
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

QString CommandProcess::wrapperHTML(QString str, LOG log)
{
    switch(log)
    {
    case LOG::SUCCESS:
        return QString("<span style='font-size: 10pt; color: green;'>%1</span>").arg(str);
    case LOG::ERROR:
        return QString("<span style='font-size: 10pt; color: red;'>%1</span>").arg(str);
    case LOG::INFO:
        return QString("<span style='font-size: 10pt; color: black;'>%1</span>").arg(str);
    }
}


void CommandProcess::run() const
{
    m_process->start(m_program, m_args);  //process在thread中阻塞运行

    m_process->waitForStarted();   //等待程序启动

    m_process->waitForFinished();  //等待程序关闭
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

    emit resultReady(wrapperHTML(output, LOG::ERROR));
    qDebug() << QString("%1 %2 output: %3").arg(m_program, m_args.join(' '), output);
}


void CommandProcess::onProcessStateChanged(QProcess::ProcessState state)
{
    switch(state)
    {
    case QProcess::NotRunning:
        qDebug() << QString("NotRunning %1 %2").arg(m_program, m_args.join(' '));
        break;
    case QProcess::Starting:
        qDebug() << QString("Starting %1 %2").arg(m_program, m_args.join(' '));
        break;
    case QProcess::Running:
        emit resultReady(wrapperHTML(QString("Running %1 %2").arg(m_program, m_args.join(' ')), LOG::INFO));
        qDebug() << QString("Running %1 %2").arg(m_program, m_args.join(' '));
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
        emit resultReady(wrapperHTML(QString("NormalExit %1 %2").arg(m_program, m_args.join(' ')), LOG::INFO));
        qDebug() << QString("NormalExit %1 %2").arg(m_program, m_args.join(' '));
        break;
    case QProcess::CrashExit:
        emit cmdError(QProcess::Crashed);
        emit resultReady(wrapperHTML(QString("CrashExit %1 %2").arg(m_program, m_args.join(' ')), LOG::ERROR));
        qDebug() << QString("CrashExit %1 %2").arg(m_program, m_args.join(' '));
        break;
    }
}

