#include "llamaservercontroller.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>

LlamaServerController::LlamaServerController(const QString &basePath, QObject *parent)
    : QObject(parent),
    m_basePath(basePath)
{
    // Assume llama-server.exe is in the same directory as the models
    m_serverPath = basePath + "/llama.cpp/build/bin/Release/llama-server.exe";
}

void LlamaServerController::stopServer(int port)
{
    if (m_activeServers.contains(port)) {
        QProcess *process = m_activeServers.value(port);
        if (process->state() != QProcess::NotRunning) {
            // CRITICAL: Use kill() for immediate VRAM release
            process->kill();
            process->waitForFinished(3000);
        }
        process->deleteLater();
        m_activeServers.remove(port);
        emit serverStatus(QString("🛑 SWAP: Server on port %1 stopped successfully.").arg(port));
    }
}


void LlamaServerController::startServer(const QString &modelName, int port)
{
    // FIX: startServer implementation
    if (m_activeServers.contains(port)) {
        emit serverError(QString("❌ Cannot start %1: Port %2 is already in use.").arg(modelName).arg(port));
        return;
    }

    // Set current active model state
    m_activeModelName = modelName;
    m_activeModelPort = port;

    QProcess *serverProcess = new QProcess(this);
    m_activeServers.insert(port, serverProcess);

    // Arguments to start the server (Using fixed ngl=40 for RTX 4050)
    QString modelFullPath = m_basePath + "/llama.cpp/build/bin/Release/" + modelName;

    QStringList args;
    args << "--model" << modelFullPath
         << "--port" << QString::number(port)
         << "--host" << "127.0.0.1"
         << "--ctx-size" << "4096"
         << "-ngl" << "40"; // Use 40 layers for max GPU offload

    // Set the working directory to the model folder for easy file access
    QDir modelDir = QFileInfo(m_serverPath).dir();
    serverProcess->setWorkingDirectory(modelDir.path());

    // Connect output signals for logging (same as before)
    connect(serverProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = QString::fromUtf8(serverProcess->readAllStandardOutput()).trimmed();
        if (!output.isEmpty()) emit serverStatus(QString("🟢 Server %1 (%2): %3").arg(port).arg(modelName).arg(output.left(80) + "..."));
    });
    connect(serverProcess, &QProcess::readyReadStandardError, this, [=]() {
        QString err = QString::fromUtf8(serverProcess->readAllStandardError()).trimmed();
        if (!err.isEmpty()) emit serverError(QString("🔴 Server %1 (%2) ERROR: %3").arg(port).arg(modelName).arg(err));
    });

    serverProcess->start(m_serverPath, args);
    emit serverStatus(QString("🚀 SWAPPING: Loading %1 on port %2...").arg(modelName).arg(port));

    // Connect crash signal (Crucial: removes the process from the map if it fails)
    connect(serverProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::CrashExit || exitCode != 0) {
            emit serverError(QString("🚨 Model %1 CRASHED on Port %2.").arg(modelName).arg(port));
        }
        // Always remove the process from the map whether it crashed or was killed cleanly.
        m_activeServers.remove(port);
        if (m_activeModelPort == port) {
            m_activeModelPort = 0;
            m_activeModelName = "";
        }
    });
}

// --- Static Wrappers for MainWindow Compatibility (Required to fix errors) ---

void LlamaServerController::stopAllServers()
{
    // Kills all active servers (including the Vector DB if it was added to the map)
    for (int port : m_activeServers.keys()) {
        stopServer(port);
    }
    emit serverStatus("🛑 All Llama server processes stopped.");
    m_activeModelPort = 0;
    m_activeModelName = "";
}

void LlamaServerController::startAllServers()
{
    // This is now used to launch the Vector DB and the initial Planner model.

    // NOTE: This initial static launch is only to make the system available immediately.
    // The ModelManager handles all subsequent starts.

    // 1. Launch Vector DB (If not manual)
    // startVectorDBOnly();

    // 2. Launch the MOST CRITICAL LLM (Planner/Phi-3.5)
    // We launch only one server to prove the concept works before relying on swapping.
    startServer("Phimini", 8080);
}

