#include "blendercontroller.h"
#include <QDir>
#include <QDebug>
#include <QDateTime>

BlenderController::BlenderController(const QString &basePath, QObject *parent)
    : QObject(parent), m_basePath(basePath)
{
    m_blenderPath = "C:/Program Files/Blender Foundation/Blender 4.2/blender.exe";
    // Point to the industry-standard engine script from your roadmap
    m_engineScriptPath = m_basePath + "/src/engines/blender_engine.py";
}

void BlenderController::setActiveProjectPath(const QString &path)
{
    m_activeProjectPath = path;
}

void BlenderController::executeAutoRig(const QString &taskId, const QString &inputMeshPath, const QString &rigType)
{
    if (m_activeProjectPath.isEmpty()) return;

    // ROADMAP FIX: Use Phase 1 standard folder structure
    QString outputDir = m_activeProjectPath + "/Processed_FBX";
    QDir().mkpath(outputDir);
    
    QString assetName = QFileInfo(inputMeshPath).baseName();
    QString outputFbx = outputDir + "/" + assetName + "_rigged.fbx";

    QProcess *process = new QProcess(this);
    
    // Command line for blender_engine.py AAA pipeline
    QStringList args;
    args << "-b" << "-P" << m_engineScriptPath << "--" 
         << "--action" << "auto_rig"
         << "--input" << inputMeshPath
         << "--output" << outputFbx
         << "--rig_type" << rigType;

    connect(process, &QProcess::readyReadStandardOutput, this, [=]() {
        emit blenderOutput(process->readAllStandardOutput().trimmed());
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode) {
        if (exitCode == 0) {
            emit blenderOutput("✅ Auto-Rig Complete: " + assetName);
            
            // PHASE 1 COMPLIANCE: Register with unified manifest including description
            if (m_projectStateController) {
                m_projectStateController->registerAsset(
                    "MESH", 
                    assetName, 
                    "AAA Rigged character ready for Unreal Engine spawning.", // Description for future fetching
                    "Processed_FBX/" + assetName + "_rigged.fbx"
                );
            }
            emit assetReadyForEngine(outputFbx);
        } else {
            emit blenderError("❌ Auto-Rig failed for " + assetName);
        }
        process->deleteLater();
    });

    process->start(m_blenderPath, args);
}