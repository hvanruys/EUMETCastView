#include "processmanager.h"

ProcessManager::ProcessManager(QStringList datelist, int maxConcurrent, QString shortname, QObject *parent)
    : QObject(parent),  maxConcurrentProcesses(maxConcurrent)
{

    bool isMTG = (shortname == "MET_12" ? true : false);
    if(datelist.length() == 0)
        return;

    if(isMTG)
    {
        ProcessTask task;
        task.taskId = 49;

        task.program = "./EUMETCastVideo";
        task.arguments = {QString("%1").arg(49), datelist.at(49)};

        taskQueue.enqueue(task);

    }
    else
    {
        // Create a queue of tasks to execute
        for (int i = 0; i < datelist.length(); i++) {
            ProcessTask task;
            task.taskId = i;

            task.program = "./EUMETCastVideo";
            task.arguments = {QString("%1").arg(i), datelist.at(i)};

            taskQueue.enqueue(task);
        }
    }

    totalTasks = taskQueue.size();
    qDebug() << "Created" << totalTasks << "tasks in queue";
    qDebug() << "Maximum concurrent processes:" << maxConcurrentProcesses << "\n";

}

void ProcessManager::start() {
    qDebug() << "Starting process manager...\n";
    // Fill up to max concurrent processes
    while (activeProcesses.size() < maxConcurrentProcesses && !taskQueue.isEmpty()) {
        startNextTask();
    }

}

void ProcessManager::startNextTask() {
    if (taskQueue.isEmpty()) {
        return;
    }

    ProcessTask task = taskQueue.dequeue();

    QProcess *proc = new QProcess(this);

    // Store task ID as property for identification
    proc->setProperty("taskId", task.taskId);

    // Connect signals
    connect(proc, &QProcess::readyRead,
            this, &ProcessManager::onReadyRead);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ProcessManager::onFinished);
    connect(proc, &QProcess::errorOccurred,
            this, &ProcessManager::onErrorOccurred);

    // Set program and arguments
    proc->setProgram(task.program);
    proc->setArguments(task.arguments);

    activeProcesses.append(proc);

    qDebug() << QString("Starting Task %1: %2 %3")
                    .arg(task.taskId)
                    .arg(task.program)
                    .arg(task.arguments.join(" "));

    proc->start();
}

ProcessManager::~ProcessManager() {
    // Clean up processes
    for (QProcess *proc : activeProcesses) {
        if (proc->state() != QProcess::NotRunning) {
            proc->kill();
            proc->waitForFinished();
        }
        delete proc;
    }
}

void ProcessManager::onReadyRead() {
    QProcess *proc = qobject_cast<QProcess*>(sender());
    if (proc) {
        QByteArray output = proc->readAllStandardOutput();
        QByteArray error = proc->readAllStandardError();

        int taskId = proc->property("taskId").toInt();

        if (!output.isEmpty()) {
            qDebug() << QString("[Task %1 OUTPUT]:").arg(taskId)
            << output.trimmed();
        }
        if (!error.isEmpty()) {
            qDebug() << QString("[Task %1 ERROR]:").arg(taskId)
            << error.trimmed();
        }
    }

}

void ProcessManager::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QProcess *proc = qobject_cast<QProcess*>(sender());
    if (proc) {
        int taskId = proc->property("taskId").toInt();

        qDebug() << QString("[Task %1 FINISHED] Exit code: %2, Status: %3")
                        .arg(taskId)
                        .arg(exitCode)
                        .arg(exitStatus == QProcess::NormalExit ? "Normal" : "Crashed");

        completedTasks++;

        // Remove from active processes
        activeProcesses.removeOne(proc);
        proc->deleteLater();

        qDebug() << QString("Progress: %1/%2 completed, %3 running, %4 queued")
                        .arg(completedTasks)
                        .arg(totalTasks)
                        .arg(activeProcesses.size())
                        .arg(taskQueue.size());

        // Start next task if available
        if (!taskQueue.isEmpty()) {
            startNextTask();
        } else if (activeProcesses.isEmpty()) {
            // All done!
            qDebug() << "=== All tasks completed! ===";
            emit signalDeleteManager();
        }
    }
 }

void ProcessManager::onErrorOccurred(QProcess::ProcessError error) {
    QProcess *proc = qobject_cast<QProcess*>(sender());
    if (proc) {
        int taskId = proc->property("taskId").toInt();
        qDebug() << QString("[Task %1 ERROR]: Process error occurred - %2")
                        .arg(taskId)
                        .arg(error);
    }
}
