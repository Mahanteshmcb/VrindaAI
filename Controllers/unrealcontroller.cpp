#include "unrealcontroller.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QTemporaryDir>
#include <QJsonObject>
#include <QJsonArray>

UnrealController::UnrealController(QObject *parent)
    : QObject(parent),
      m_unrealEditorPath(""),
      m_projectStateController(nullptr)
{
}

void UnrealController::setProjectStateController(ProjectStateController* controller)
{
    m_projectStateController = controller;
}

void UnrealController::setUnrealEditorPath(const QString &path)
{
    if (!path.isEmpty()) {
        m_unrealEditorPath = path;
    }
}

void UnrealController::executeInstructionSequence(const QString &projectPath, const QJsonArray &instructions)
{
    if (!m_projectStateController) {
        emit unrealError("❌ Manifest Controller not linked. Cannot resolve Asset IDs.");
        return;
    }

    QStringList scriptLines;
    scriptLines << "import unreal"
                << "import os"
                << "import json"
                << ""
                << "def resolve_path(asset_id, project_path):"
                << "    manifest_path = os.path.join(project_path, 'project_assets.json')"
                << "    if not os.path.exists(manifest_path): return None"
                << "    with open(manifest_path, 'r') as f:"
                << "        manifest = json.load(f)"
                << "    for asset in manifest.get('assets', []):"
                << "        if asset['id'].upper() == asset_id.upper():"
                << "            return os.path.join(project_path, asset['path'])"
                << "    return None"
                << "";

    for (const QJsonValue &value : instructions) {
        QJsonObject instruction = value.toObject();
        QString command = instruction["command"].toString();
        QJsonObject payload = instruction["payload"].toObject();

        scriptLines << "# --- Command: " + command + " ---";

        if (command == "spawn_asset") {
            QString assetId = payload["asset_id"].toString();
            QJsonArray loc = payload["location"].toArray();
            QJsonArray rot = payload["rotation"].toArray();

            scriptLines << QString("path = resolve_path('%1', r'%2')").arg(assetId, QDir::toNativeSeparators(projectPath));
            scriptLines << "if path:";
            scriptLines << "    cls = unreal.EditorAssetLibrary.load_blueprint_class(path)";
            scriptLines << QString("    loc = unreal.Vector(%1, %2, %3)").arg(loc[0].toDouble()).arg(loc[1].toDouble()).arg(loc[2].toDouble());
            scriptLines << QString("    rot = unreal.Rotator(%1, %2, %3)").arg(rot[0].toDouble()).arg(rot[1].toDouble()).arg(rot[2].toDouble());
            scriptLines << "    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(cls, loc, rot)";
            scriptLines << QString("    actor.set_actor_label('%1_Actor')").arg(assetId);
        } 
        else if (command == "create_sequence") {
            QString name = payload["name"].toString();
            scriptLines << "asset_tools = unreal.AssetToolsHelpers.get_asset_tools()";
            scriptLines << QString("sequence = asset_tools.create_asset('%1', '/Game/Cinematics', unreal.LevelSequence, unreal.LevelSequenceFactoryNew())").arg(name);
        }
    }

    QTemporaryDir tempDir;
    if (tempDir.isValid()) {
        tempDir.setAutoRemove(false);
        QString scriptPath = tempDir.path() + "/unreal_director_script.py";
        QFile file(scriptPath);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            out << scriptLines.join("\n");
            file.close();
            runUnrealProcess(projectPath, scriptPath);
        }
    }
}

void UnrealController::runUnrealProcess(const QString &projectPath, const QString &scriptPath)
{
    QDir dir(projectPath);
    QStringList filters; filters << "*.uproject";
    QStringList files = dir.entryList(filters);
    
    if(files.isEmpty()) {
        emit unrealError("❌ No .uproject found in " + projectPath);
        return;
    }

    QString uprojectFile = QDir::toNativeSeparators(dir.filePath(files[0]));
    QProcess *process = new QProcess(this);
    
    QStringList args;
    args << uprojectFile 
         << "-run=PythonScript" 
         << "-Script=" + QDir::toNativeSeparators(scriptPath)
         << "-stdout" << "-Unattended" << "-NoSplash" << "-NullRHI";

    connect(process, &QProcess::readyReadStandardOutput, this, [=](){ emit unrealOutput(process->readAllStandardOutput()); });
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int code){
        emit unrealOutput(QString("✅ Phase 2: Unreal Stage Assembly finished (Code %1)").arg(code));
        process->deleteLater();
    });

    process->start(m_unrealEditorPath, args);
}