#include "processmanager.h"

#include <QCoreApplication>

ProcessManager::ProcessManager(QStringList datelist, int maxConcurrent, QString shortname, QObject *parent)
    : QObject(parent),  maxConcurrentProcesses(qMax(1, maxConcurrent))
{

    // Resolved against the directory this executable lives in, not against the
    // working directory. "./EUMETCastVideo" only worked because the application
    // happened to be started from its own directory: on Windows whatever starts
    // EUMETCastView - a shortcut, QtCreator - chooses the working directory, and
    // the relative name then points at a file that is not there. The process
    // failed to start, which nothing reported, and tempvideo/ stayed empty.
    // The .exe suffix is not needed; CreateProcess appends it itself.
    videoprogram = QCoreApplication::applicationDirPath() + "/EUMETCastVideo";

    bool isMTG = (shortname == "MET_12" ? true : false);
    if(datelist.length() == 0)
        return;

    if(isMTG)
    {
        for (int i = 0; i < datelist.length(); i++) {
            {
                ProcessTask task;
                task.taskId = i;

                task.program = videoprogram;
                task.arguments = {QString("%1").arg(i), datelist.at(i)};

                taskQueue.enqueue(task);
            }
        }
    }
    else
    {
        // Create a queue of tasks to execute
        for (int i = 0; i < datelist.length(); i++) {
            ProcessTask task;
            task.taskId = i;

            task.program = videoprogram;
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

    emit signalMessage(QString("Starting %1 task(s), %2 at a time : %3")
                           .arg(totalTasks).arg(maxConcurrentProcesses).arg(videoprogram));

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

        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            emit signalMessage(QString("Task %1 ended badly : exit code %2, %3")
                                   .arg(taskId)
                                   .arg(exitCode)
                                   .arg(exitStatus == QProcess::NormalExit ? "normal exit" : "crashed"));
        }

        retireProcess(proc);
    }
 }

void ProcessManager::onErrorOccurred(QProcess::ProcessError error) {
    QProcess *proc = qobject_cast<QProcess*>(sender());
    if (proc) {
        int taskId = proc->property("taskId").toInt();
        qDebug() << QString("[Task %1 ERROR]: Process error occurred - %2")
                        .arg(taskId)
                        .arg(error);

        emit signalMessage(QString("Task %1 : %2 (%3)")
                               .arg(taskId)
                               .arg(proc->errorString())
                               .arg(proc->program()));

        // A process that never started emits no finished(), so nothing else
        // would take it out of activeProcesses: the queue stalled here, with no
        // images written and nothing said about it. Retire it as if it had run.
        // Queued, because on Windows this arrives from inside proc->start() -
        // starting the next task from here would recurse once per task - while
        // on Unix it arrives from the event loop.
        if (error == QProcess::FailedToStart) {
            QMetaObject::invokeMethod(this, [this, proc]{ retireProcess(proc); },
                                      Qt::QueuedConnection);
        }
    }
}

// Called for a process that has run, and for one that never started. Both leave
// the queue to be advanced and both are the last use of the QProcess.
void ProcessManager::retireProcess(QProcess *proc) {
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
