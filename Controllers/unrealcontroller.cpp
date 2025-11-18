/*Prompt for a design creation for unreal

Create a project plan to generate a simple, animated, interactive sci-fi crate for an Unreal Engine game. The plan must include these steps: First, the Researcher needs to find a simple, royalty-free sci-fi texture for a metal crate. Simultaneously, the Coder needs to write a Blender Python script that models a futuristic-looking crate with a separate lid. After the Coder is done, the Designer needs to take that script and add a simple open-and-close animation to the lid. Finally, after the texture is found and the animated model is ready, the Engine agent must generate the commands to import the crate and its animation into my Unreal Engine project and apply the texture to it.

create a project to produce a one-minute, cinematic video of a futuristic drone exploring a desolate alien canyon.
Key requirements for the project:
- The visual style should be cinematic, with smooth camera movements and a focus on scale and atmosphere.
- The project requires a 3D animated scene of a sci-fi drone flying through a rocky, Mars-like canyon.
- The scene should include some glowing, crystalline energy sources that the drone investigates.
- The project requires royalty-free, cinematic ambient music suitable for a sci-fi exploration theme.
- The project requires subtle, royalty-free sound effects for the drone's movement.
- The final output must be a single video file, approximately 60 seconds long, rendered at 1080p resolution.

Create a small, interactive game scene in an Unreal Engine project.
Key requirements for the project:
- The Designer agent must create a 3D model of an ancient, ornate treasure chest with a separate lid, along with a 2-second lid-opening animation.
- The Researcher agent must find a seamless, royalty-free stone texture for the chest and a "stone grinding" sound effect for the opening animation.
- The Coder agent must write a simple C++ Actor Component for Unreal Engine that can be attached to the chest. This component should trigger the chest's opening animation when an event is called.
- The Engine agent must then execute a sequence of commands to:
  1. Create a new level named "Treasure_Chamber".
  2. Import the chest model, its animation, the texture, and the sound effect.
  3. Create a Material from the texture and apply it to the chest.
  4. Place the chest in the level.
  5. Create a simple trigger volume in front of the chest that calls the C++ event to open it when the player overlaps it.
- The Integrator must compile the project's C++ code.

Create a project to produce a cinematic, 15-second video advertisement for a fictional smartwatch called the "Aura Sphere."
Key requirements for the project:
- The visual style should be sleek, minimalist, and futuristic.
- The Designer agent must create a 3D model of the "Aura Sphere" smartwatch in Blender. It should have a clean, circular face and a modern strap.
- The Designer agent must also create a simple animation of the watch rotating slowly and the screen lighting up with a glowing particle effect.
- The Researcher agent must find royalty-free, minimalist electronic background music.
- The Editing agent must assemble the rendered animation shots into a final 1080p video, add the music, and display the product name "Aura Sphere" at the end.

create a project so that i can know the full capability of all the workflow and final output i will get from this application.

Create a complete and functional desktop weather application using C++ and QML.
Key requirements for the project:
- The UI must be designed in a single QML file and should display the city name, current temperature in Celsius, a short weather description (e.g., "Clear Sky"), and an appropriate weather icon.
- The C++ backend must be able to fetch live weather data from a public API.
- The Researcher agent must first find a suitable free weather API that does not require a complex authentication key. The Coder should then use this API.
- The final application must be buildable with CMake.
- The project should conclude with the Integrator creating a README.md file.

Create a complete and functional desktop Pomodoro Timer application using C++. The UI should be designed first by the Designer agent, producing QML code for the user interface. The backend logic for the timers (work, break, state management) should be written by the Coder agent. Finally, the Integrator must combine the UI and the backend logic into a final, runnable application and create a README.md file.
*/
#include "unrealcontroller.h"
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QTemporaryDir>
#include <QJsonObject>

UnrealController::UnrealController(QObject *parent)
    : QObject(parent),
    // TODO: Move this path to a settings/config file
    //m_unrealEditorPath("C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor.exe")
    m_unrealEditorPath("")
{

}

void UnrealController::setUnrealEditorPath(const QString &path)
{
    if (!path.isEmpty()) {
        m_unrealEditorPath = path;
    }
}

void UnrealController::executeInstructionSequence(const QString &projectUProjectPath, const QJsonArray &instructions)
{
    // Step 1: Generate a Python script from the instruction sequence
    QStringList scriptLines;
    scriptLines << "import unreal"
                << "import os"
                << ""
                << "EUL = unreal.EditorUtilityLibrary"
                << "EAL = unreal.EditorAssetLibrary"
                << "ELL = unreal.EditorLevelLibrary"
                << ""
                << "def import_asset(source, dest):"
                << "    task = unreal.AssetImportTask()"
                << "    task.set_editor_property('automated', True)"
                << "    task.set_editor_property('filename', source)"
                << "    task.set_editor_property('destination_path', dest)"
                << "    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()"
                << "    asset_tools.import_asset_tasks([task])"
                << "";

    for (const QJsonValue &value : instructions) {
        QJsonObject instruction = value.toObject();
        QString command = instruction["command"].toString();
        QJsonObject payload = instruction["payload"].toObject();

        scriptLines << "# --- Command: " + command + " ---";
        if (command == "create_level") {
            QString name = payload["name"].toString();
            scriptLines << QString("new_level = EAL.create_asset('%1', unreal.World, unreal.WorldFactory())").arg(name);
        } else if (command == "import_asset") {
            QString source = QDir::toNativeSeparators(payload["source_path"].toString());
            QString dest = payload["destination_path"].toString();
            scriptLines << QString("import_asset(r'%1', '%2')").arg(source, dest);
        } else if (command == "place_in_level") {
            QString assetPath = payload["asset_path"].toString();
            QJsonArray loc = payload["location"].toArray();
            QJsonArray rot = payload["rotation"].toArray();
            QString locationStr = QString("unreal.Vector(%1, %2, %3)").arg(loc[0].toDouble()).arg(loc[1].toDouble()).arg(loc[2].toDouble());
            QString rotationStr = QString("unreal.Rotator(%1, %2, %3)").arg(rot[0].toDouble()).arg(rot[1].toDouble()).arg(rot[2].toDouble());
            scriptLines << QString("ELL.spawn_actor_from_asset('%1', %2, %3)").arg(assetPath, locationStr, rotationStr);
        } else if (command == "save_level") {
            scriptLines << "ELL.save_current_level()";
        }
    }

    // Step 2: Save the generated script to a temporary file
    QTemporaryDir tempDir;
    if (tempDir.isValid()) {
        QString scriptPath = tempDir.path() + "/vrinda_ue_script.py";
        QFile scriptFile(scriptPath);
        if (scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&scriptFile);
            out << scriptLines.join("\n");
            scriptFile.close();

            emit unrealOutput("✅ Generated Unreal script with " + QString::number(instructions.count()) + " steps.");
            runUnrealProcess(projectUProjectPath, scriptPath);
        } else {
            emit unrealError("❌ Could not create temporary Unreal script file.");
        }
    }
}

void UnrealController::runUnrealProcess(const QString &projectUProjectPath, const QString &scriptPath)
{
    // This function remains the same, but we need to find the .uproject file
    QString uprojectFile = projectUProjectPath;
    if(!uprojectFile.endsWith(".uproject")) {
        QDir dir(projectUProjectPath);
        QStringList filters;
        filters << "*.uproject";
        QStringList files = dir.entryList(filters);
        if(files.isEmpty()){
            emit unrealError("❌ Could not find a .uproject file in the project path: " + projectUProjectPath);
            return;
        }
        uprojectFile = dir.filePath(files[0]);
    }

    if (!QFile::exists(m_unrealEditorPath)) { emit unrealError("❌ Unreal Editor not found at: " + m_unrealEditorPath); return; }
    if (!QFile::exists(uprojectFile)) { emit unrealError("❌ Unreal Project file not found at: " + uprojectFile); return; }
    if (!QFile::exists(scriptPath)) { emit unrealError("❌ Python script to execute not found at: " + scriptPath); return; }

    QProcess *unrealProcess = new QProcess(this);
    QStringList args;
    args << uprojectFile << "-ExecutePythonScript=" + QDir::toNativeSeparators(scriptPath);

    emit unrealOutput("🚀 Launching Unreal Engine to execute script...");
    qDebug() << "Running Unreal:" << m_unrealEditorPath << args;

    connect(unrealProcess, &QProcess::readyReadStandardOutput, this, [=](){ emit unrealOutput(QString::fromLocal8Bit(unrealProcess->readAllStandardOutput())); });
    connect(unrealProcess, &QProcess::readyReadStandardError, this, [=](){ emit unrealError(QString::fromLocal8Bit(unrealProcess->readAllStandardError())); });
    connect(unrealProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode){
        emit unrealOutput(QString("✅ Unreal process finished with exit code %1.").arg(exitCode));
        unrealProcess->deleteLater();
    });

    unrealProcess->start(m_unrealEditorPath, args);
}
