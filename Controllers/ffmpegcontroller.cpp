#include "ffmpegcontroller.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>

FfmpegController::FfmpegController(const QString &basePath, QObject *parent)
    : QObject(parent), m_basePath(basePath)
{
    m_scriptPath = QDir(m_basePath).filePath("src/engines/ffmpeg_engine.py");
    qDebug() << "FFmpegController initialized. Script path:" << m_scriptPath;
}

void FfmpegController::executeEditingCommand(const QString &taskId, const QString &jsonPayload)
{
    QProcess *process = new QProcess(this);
    m_activeTaskIds[process] = taskId;

    // Use a temporary path to save the JSON manifest for the Python script
    QString tempManifestPath = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath(taskId + "_ffmpeg_job.json");
    
    QFile file(tempManifestPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit processError("Failed to open temporary manifest file for writing.");
        emit processFinished(taskId, "");
        delete process;
        return;
    }
    QTextStream out(&file);
    out << jsonPayload;
    file.close();

    // Command to execute the python script with the manifest
    QString program = "python";
    QStringList arguments;
    arguments << m_scriptPath << "--job_manifest" << tempManifestPath;

    connect(process, &QProcess::readyReadStandardOutput, this, &FfmpegController::readProcessOutput);
    connect(process, &QProcess::readyReadStandardError, this, &FfmpegController::readProcessOutput);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FfmpegController::handleProcessFinished);

    qDebug() << "Starting FFmpeg Python script for task:" << taskId << program << arguments.join(" ");

    process->start(program, arguments);
    if (!process->waitForStarted(5000)) {
        emit processError("FFmpeg process failed to start: " + process->errorString());
        emit processFinished(taskId, "");
        delete process;
        QFile::remove(tempManifestPath); // Cleanup
    }
    // Note: The manifest is cleaned up in handleProcessFinished
}

void FfmpegController::readProcessOutput()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    QString output = process->readAllStandardOutput().trimmed();
    QString error = process->readAllStandardError().trimmed();

    if (!output.isEmpty()) {
        emit processOutput(output);
    }
    if (!error.isEmpty()) {
        emit processOutput("[FFMPEG-ERROR] " + error);
    }
}

void FfmpegController::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    QString taskId = m_activeTaskIds.value(process, "UNKNOWN");
    m_activeTaskIds.remove(process);

    // Cleanup the temporary manifest file
    QString tempManifestPath = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath(taskId + "_ffmpeg_job.json");
    QFile::remove(tempManifestPath);

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        // Assume success and let the main window logic extract the final path if necessary
        emit processFinished(taskId, "SUCCESS");
    } else {
        // Output already captured via readProcessOutput, but emit a formal error signal
        emit processError(QString("FFmpeg script failed for task %1 (Exit Code: %2)").arg(taskId).arg(exitCode));
        emit processFinished(taskId, ""); // Signal failure
    }

    process->deleteLater();
}