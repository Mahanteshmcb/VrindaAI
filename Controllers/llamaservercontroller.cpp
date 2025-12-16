#include "llamaservercontroller.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>

LlamaServerController::LlamaServerController(const QString &basePath, QObject *parent)
    : QObject(parent), m_basePath(basePath) {
    m_serverPath = basePath + "/llama.cpp/build/bin/Release/llama-server.exe";
}

void LlamaServerController::stopServer(int port) {
    if (m_activeServers.contains(port)) {
        QProcess *process = m_activeServers.value(port);
        if (process->state() != QProcess::NotRunning) {
            process->kill(); // Force VRAM release
            process->waitForFinished(3000);
        }
        process->deleteLater();
        m_activeServers.remove(port);
        emit serverStatus(QString("🛑 SWAP: Server on port %1 stopped.").arg(port));
    }
}

void LlamaServerController::startServer(const QString &modelName, int port) {
    if (m_activeServers.contains(port)) return;

    QProcess *serverProcess = new QProcess(this);
    m_activeServers.insert(port, serverProcess);

    QStringList args;
    // --- VISION MODEL HANDLING ---
    if (modelName.contains("--")) {
        QStringList parts = modelName.split("--");
        QString mainModel = m_basePath + "/llama.cpp/build/bin/Release/" + parts[0].trimmed();
        QString mmproj = m_basePath + "/llama.cpp/build/bin/Release/" + parts[1].trimmed();
        args << "--model" << mainModel << "--mmproj" << mmproj;
    } else {
        args << "--model" << m_basePath + "/llama.cpp/build/bin/Release/" + modelName;
    }

    args << "--port" << QString::number(port) << "--host" << "127.0.0.1" << "--ctx-size" << "4096" << "-ngl" << "40";

    serverProcess->setWorkingDirectory(QFileInfo(m_serverPath).dir().path());
    
    connect(serverProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = serverProcess->readAllStandardOutput().trimmed();
        if (!output.isEmpty()) emit serverStatus(QString("🟢 Server %1: %2").arg(port).arg(output.left(50)));
    });

    serverProcess->start(m_serverPath, args);
    emit serverStatus(QString("🚀 SWAPPING: Loading %1...").arg(modelName));
}

void LlamaServerController::stopAllServers() {
    for (int port : m_activeServers.keys()) stopServer(port);
}

void LlamaServerController::startAllServers() {
    startServer("Phimini.gguf", 8080);
}