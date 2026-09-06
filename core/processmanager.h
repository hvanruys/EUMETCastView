#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QList>
#include <QQueue>
#include <QVariant>

struct ProcessTask {
    QString program;
    QStringList arguments;
    int taskId;
};

class ProcessManager : public QObject
{
    Q_OBJECT
public:
    // singleimage < 0 renders the whole datelist. Anything else renders that one
    // entry only, keeping its position in the list as its image number, so a
    // test image lands on the frame a full run would have written there.
    explicit ProcessManager(QStringList datelist, int maxConcurrent = 10, QString shortname = "MET_12", int singleimage = -1, QObject *parent = nullptr);
    void start();
    void stopAll();
    bool wasAborted() const { return aborted; }

    ~ProcessManager();

private:
    // void createProcess(const QString &program, const QStringList &args);
    void startNextTask();
    void retireProcess(QProcess *proc);

    //QVarLengthArray<QProcess*, 256> processes;
    //QVarLengthArray<int, 256> processtatus;
    int finishedCount = 0;
    bool aborted = false;
    QQueue<ProcessTask> taskQueue;
    QList<QProcess*> activeProcesses;

    QString videoprogram;
    int maxConcurrentProcesses;
    int totalTasks = 0;
    int completedTasks = 0;

private slots:
    void onReadyRead();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(QProcess::ProcessError error);

signals:
    void signalDeleteManager();
    void signalMessage(QString msg);
};

#endif // PROCESSMANAGER_H
