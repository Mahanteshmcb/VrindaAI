#include "blendercontroller.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QTemporaryDir>
#include <QRegularExpression>
#include <QCoreApplication>

BlenderController::BlenderController(const QString &basePath, QObject *parent)
    : QObject(parent), m_basePath(basePath),
    // Default path (can be overridden by settings)
    m_blenderPath("C:/Program Files/Blender Foundation/Blender 4.3/blender.exe")
{
}

void BlenderController::setBlenderPath(const QString &path)
{
    if (!path.isEmpty()) {
        m_blenderPath = path;
    }
}

void BlenderController::setActiveProjectPath(const QString &path)
{
    m_activeProjectPath = path;
}

void BlenderController::triggerScript(const QString &scriptContent)
{
    // 1. Validation
    if (!QFile::exists(m_blenderPath)) {
        emit blenderError("❌ Blender executable not found at: " + m_blenderPath);
        return;
    }
    if (m_activeProjectPath.isEmpty()) {
        emit blenderError("❌ Cannot run Blender script: No active project path is set.");
        return;
    }

    // 2. Setup Temporary Directory & Script
    // FIX: Allocate on heap so it survives after this function returns
    QTemporaryDir *tempDir = new QTemporaryDir();
    if (!tempDir->isValid()) {
        emit blenderError("❌ Could not create temporary directory.");
        delete tempDir;
        return;
    }

    // Turn off auto-remove so we can control deletion time
    tempDir->setAutoRemove(false); 

    QString scriptPath = tempDir->path() + "/temp_blender_script.py";
    QFile scriptFile(scriptPath);
    if (!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit blenderError("❌ Could not create script file: " + scriptPath);
        delete tempDir;
        return;
    }

    QTextStream out(&scriptFile);
    out << scriptContent;
    scriptFile.close();

    // 3. Prepare Paths
    QDir projectDir(m_activeProjectPath);
    if (!projectDir.exists()) {
        projectDir.mkpath(".");
    }
    projectDir.mkpath("blend");
    projectDir.mkpath("assets");

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString blendPath = projectDir.filePath("blend/scene_" + timestamp + ".blend");
    QString fbxPath = projectDir.filePath("assets/export_" + timestamp + ".fbx");

    // 4. Configure Process
    QProcess *blenderProcess = new QProcess(this);
    blenderProcess->setProcessChannelMode(QProcess::SeparateChannels);
    blenderProcess->setWorkingDirectory(m_activeProjectPath);

    QStringList args;
    // --background: Run headless
    // --python: Execute the script
    // --: arguments passed to the python script itself
    args << "--background" << "--python" << scriptPath << "--" << fbxPath << blendPath;

    emit blenderOutput("🎬 Running Blender script: " + scriptPath);
    qDebug() << "Executing:" << m_blenderPath << args;

    // 5. Connect Signals
    connect(blenderProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = QString::fromUtf8(blenderProcess->readAllStandardOutput()).trimmed();
        if (!output.isEmpty()) emit blenderOutput("🟢 Blender: " + output);
    });

    connect(blenderProcess, &QProcess::readyReadStandardError, this, [=]() {
        QString err = QString::fromUtf8(blenderProcess->readAllStandardError()).trimmed();
        if (!err.isEmpty()) emit blenderError("🔴 Blender Log: " + err);
    });

    // Handle Cleanup when process finishes
    connect(blenderProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [=](int exitCode, QProcess::ExitStatus status) {
        
        emit blenderFinished(exitCode);
        
        if (status == QProcess::NormalExit && exitCode == 0) {
            emit blenderOutput("✅ Blender process completed successfully.");
            emit assetReadyForEngine(fbxPath);
        } else {
            emit blenderError("❌ Blender process failed with code: " + QString::number(exitCode));
        }

        // Cleanup resources
        // Now it is safe to delete the temporary directory and the script file inside it
        tempDir->remove(); 
        delete tempDir; 
        
        blenderProcess->deleteLater();
    });

    // 6. Start
    blenderProcess->start(m_blenderPath, args);
}