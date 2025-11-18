#include "blendercontroller.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextEdit>
#include <QDateTime>
#include <QDebug>
#include <QTemporaryDir>
#include <QRegularExpression>
#include <QCoreApplication>

BlenderController::BlenderController(const QString &basePath, QObject *parent)
    : QObject(parent), m_basePath(basePath),
    // Set a default path, which can be overridden by the setter
    m_blenderPath("C:/Program Files/Blender Foundation/Blender 4.3/blender.exe")
{
}

// --- NEW: Implementation of the public setter ---
void BlenderController::setBlenderPath(const QString &path)
{
    if (!path.isEmpty()) {
        m_blenderPath = path;
    }
}


void BlenderController::triggerScript(const QString &scriptContent)
{
    if (!QFile::exists(m_blenderPath)) {
        emit blenderError("❌ Blender executable not found at: " + m_blenderPath);
        return;
    }
    if (m_activeProjectPath.isEmpty()) {
        emit blenderError("❌ Cannot run Blender script: No active project path is set.");
        return;
    }

    // --- START: NEW LOGIC TO SAVE SCRIPT TO A TEMPORARY FILE ---
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        emit blenderError("❌ Could not create temporary directory for Blender script.");
        return;
    }
    QString scriptPath = tempDir.path() + "/temp_blender_script.py";
    QFile scriptFile(scriptPath);
    if (!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit blenderError("❌ Could not create temporary script file: " + scriptPath);
        return;
    }
    QTextStream out(&scriptFile);
    out << scriptContent;
    scriptFile.close();
    // --- END: NEW LOGIC ---


    // The rest of the function now uses this temporary scriptPath
    QDir projectDir(m_activeProjectPath);
    projectDir.mkpath("blend");
    projectDir.mkpath("assets");

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString blendPath = m_activeProjectPath + "/blend/scene_" + timestamp + ".blend";
    QString fbxPath = m_activeProjectPath + "/assets/export_" + timestamp + ".fbx"; // Export FBX to assets

    QProcess *blenderProcess = new QProcess(this);
    blenderProcess->setProcessChannelMode(QProcess::SeparateChannels);
    blenderProcess->setWorkingDirectory(m_activeProjectPath);

    QStringList args;
    args << "--background" << "--python" << scriptPath << "--" << fbxPath << blendPath;

    emit blenderOutput("🎬 Running Blender script: " + scriptPath);

    connect(blenderProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = QString::fromUtf8(blenderProcess->readAllStandardOutput()).trimmed();
        if (!output.isEmpty()) emit blenderOutput("🟢 Blender: " + output);
    });

    connect(blenderProcess, &QProcess::readyReadStandardError, this, [=]() {
        QString err = QString::fromUtf8(blenderProcess->readAllStandardError()).trimmed();
        if (!err.isEmpty()) emit blenderError("🔴 Blender Error: " + err);
    });

    connect(blenderProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [=](int exitCode, QProcess::ExitStatus) {
                emit blenderFinished(exitCode);
                if (exitCode == 0) {
                    emit assetReadyForEngine(fbxPath);
                }
                blenderProcess->deleteLater();
            });

    qDebug() << "Running blender:" << m_blenderPath << args;
    blenderProcess->start(m_blenderPath, args);
}

// You will need to add a public slot or method to set the active project path
void BlenderController::setActiveProjectPath(const QString &path)
{
    m_activeProjectPath = path;
}
