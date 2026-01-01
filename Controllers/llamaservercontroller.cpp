#include "llamaservercontroller.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>

LlamaServerController::LlamaServerController(const QString &basePath, QObject *parent)
    : QObject(parent), m_basePath(basePath) {
    // Standard path for your build
    m_serverPath = basePath + "/llama.cpp/build/bin/Release/llama-server.exe";
}

// FIX: Renamed to match the call in ModelManager
void LlamaServerController::stopServerOnPort(int port) {
    if (m_activeServers.contains(port)) {
        QProcess *process = m_activeServers.value(port);
        if (process && process->state() != QProcess::NotRunning) {
            process->kill(); // Force VRAM release for rapid swapping
            process->waitForFinished(3000);
        }
        process->deleteLater();
        m_activeServers.remove(port);
        emit serverStatus(QString("🛑 SWAP: Server on port %1 stopped.").arg(port));
    }
}

void LlamaServerController::startServer(const QString &modelName, int port) {
    // Ensure the port is clean before starting
    stopServerOnPort(port);

    QProcess *serverProcess = new QProcess(this);
    m_activeServers.insert(port, serverProcess);

    QStringList args;
    QString modelBinDir = m_basePath + "/llama.cpp/build/bin/Release/";

    // --- VISION MODEL HANDLING (e.g., LLaVA) ---
    if (modelName.contains("--")) {
        QStringList parts = modelName.split("--");
        QString mainModel = modelBinDir + parts[0].trimmed();
        QString mmproj = modelBinDir + parts[1].trimmed();
        args << "--model" << mainModel << "--mmproj" << mmproj;
    } else {
        args << "--model" << modelBinDir + modelName;
    }

    // Standard AAA Production Flags
    args << "--port" << QString::number(port) 
         << "--host" << "127.0.0.1" 
         << "--ctx-size" << "4096" 
         << "-ngl" << "99"; // Set to 99 to force all layers to GPU if available

    serverProcess->setWorkingDirectory(QFileInfo(m_serverPath).dir().path());
    
    // Log minimal output to avoid UI lag
    connect(serverProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = serverProcess->readAllStandardOutput().trimmed();
        if (!output.isEmpty() && output.contains("HTTP server listening")) {
            emit serverStatus(QString("🟢 Server %1 is listening...").arg(port));
        }
    });

    serverProcess->start(m_serverPath, args);
    emit serverStatus(QString("🚀 SWAPPING: Loading %1 on port %2...").arg(modelName).arg(port));
}

void LlamaServerController::stopAllServers() {
    QList<int> ports = m_activeServers.keys();
    for (int port : ports) {
        stopServerOnPort(port);
    }
}

void LlamaServerController::startAllServers() {
    // Default startup if needed
    startServer("Phimini.gguf", 8080);
}