#include "jobmanifestmanager.h"
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <QDateTime>
#include <QProcess>
#include <QCoreApplication>
#include <QDebug>
#include <QVector3D>

JobManifestManager::JobManifestManager() {
    // Constructor
}

JobManifestManager::~JobManifestManager() {
    // Destructor
}

QString JobManifestManager::getResourcesDir() const {
    return QCoreApplication::applicationDirPath() + "/Resources";
}

QString JobManifestManager::generateJobId() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces).left(12).toUpper();
}

QString JobManifestManager::getEngineScriptName(Engine engine) const {
    switch (engine) {
        case Engine::Blender:
            return "blender_master.py";
        case Engine::Unreal:
            return "unreal_master.py";
        case Engine::FFmpeg: // <<< FIX 1: Replaced DaVinci with FFmpeg
            return "ffmpeg_engine.py"; // <<< Updated script name
        default:
            return "";
    }
}

QString JobManifestManager::getEngineScriptPath(Engine engine) const {
    return getResourcesDir() + "/Scripts/" + getEngineScriptName(engine);
}

QJsonObject JobManifestManager::initializeManifest(const JobConfig& config) {
    QJsonObject manifest;
    
    // Job metadata
    manifest["job_id"] = config.jobId;
    manifest["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    
    // Header
    QJsonObject header;
    header["project_name"] = config.projectName;
    header["scene_name"] = config.sceneName;
    
    QJsonObject resolution;
    resolution["width"] = config.resolutionWidth;
    resolution["height"] = config.resolutionHeight;
    header["resolution"] = resolution;
    
    header["fps"] = config.fps;
    manifest["header"] = header;
    
    // Engine
    switch (config.engine) {
        case Engine::Blender:
            manifest["engine"] = "blender";
            break;
        case Engine::Unreal:
            manifest["engine"] = "unreal";
            break;
        case Engine::FFmpeg: // <<< FIX 2: Replaced DaVinci with FFmpeg
            manifest["engine"] = "ffmpeg";
            break;
    }
    
    // Assets (empty initially)
    QJsonObject assets;
    assets["characters"] = QJsonArray();
    assets["environments"] = QJsonArray();
    assets["props"] = QJsonArray();
    assets["animations"] = QJsonArray();
    manifest["assets"] = assets;
    
    // Actions (empty initially)
    manifest["actions"] = QJsonArray();
    
    // Output
    QJsonObject output;
    output["path"] = config.outputPath;
    output["format"] = "mp4";
    output["codec"] = "h264";
    output["bitrate"] = "25M";
    manifest["output"] = output;
    
    // Metadata
    manifest["metadata"] = config.metadata;
    
    return manifest;
}

QJsonObject JobManifestManager::createJobManifest(const JobConfig& config) {
    JobConfig mutableConfig = config;
    if (mutableConfig.jobId.isEmpty()) {
        mutableConfig.jobId = generateJobId();
    }
    return initializeManifest(mutableConfig);
}

void JobManifestManager::addAsset(QJsonObject& manifest, const QString& assetType,
                                   const QString& assetId, const QString& assetName,
                                   const QString& assetPath, const QString& format) {
    QJsonObject asset;
    asset["id"] = assetId;
    asset["name"] = assetName;
    asset["path"] = assetPath;
    asset["format"] = format;
    
    QJsonObject assetsObj = manifest["assets"].toObject();
    QJsonArray assetArray = assetsObj[assetType].toArray();
    assetArray.append(asset);
    assetsObj[assetType] = assetArray;
    
    manifest["assets"] = assetsObj;
}

void JobManifestManager::addAction(QJsonObject& manifest, const QString& actionType,
                                    const QString& target, const QJsonObject& parameters) {
    QJsonObject action;
    action["type"] = actionType;
    action["target"] = target;
    action["parameters"] = parameters;
    
    QJsonArray actions = manifest["actions"].toArray();
    actions.append(action);
    manifest["actions"] = actions;
}

void JobManifestManager::setCameraConfig(QJsonObject& manifest,
                                          const QVector3D& position,
                                          const QVector3D& rotation,
                                          float focalLength) {
    QJsonObject camera;
    
    QJsonArray posArray;
    posArray.append(position.x());
    posArray.append(position.y());
    posArray.append(position.z());
    camera["position"] = posArray;
    
    QJsonArray rotArray;
    rotArray.append(rotation.x());
    rotArray.append(rotation.y());
    rotArray.append(rotation.z());
    camera["rotation"] = rotArray;
    
    camera["focal_length"] = focalLength;
    camera["keyframes"] = QJsonArray();
    
    manifest["camera"] = camera;
}

void JobManifestManager::setLightingConfig(QJsonObject& manifest,
                                            const QJsonObject& lightingConfig) {
    manifest["lighting"] = lightingConfig;
}

bool JobManifestManager::saveManifest(const QJsonObject& manifest, const QString& filePath) {
    try {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Failed to open file for writing:" << filePath;
            return false;
        }
        
        QJsonDocument doc(manifest);
        file.write(doc.toJson());
        file.close();
        
        qDebug() << "Job manifest saved:" << filePath;
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Exception saving manifest:" << e.what();
        return false;
    }
}

QJsonObject JobManifestManager::loadManifest(const QString& filePath) {
    QJsonObject emptyObject;
    try {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open file:" << filePath;
            return emptyObject;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        
        if (!doc.isObject()) {
            qWarning() << "JSON document is not an object";
            return emptyObject;
        }
        
        return doc.object();
    } catch (const std::exception& e) {
        qWarning() << "Exception loading manifest:" << e.what();
        return emptyObject;
    }
}

bool JobManifestManager::validateManifest(const QJsonObject& manifest) {
    // Basic validation
    if (manifest["job_id"].toString().isEmpty()) {
        qWarning() << "Manifest missing job_id";
        return false;
    }
    
    if (manifest["header"].toObject()["project_name"].toString().isEmpty()) {
        qWarning() << "Manifest missing project_name";
        return false;
    }
    
    if (manifest["output"].toObject()["path"].toString().isEmpty()) {
        qWarning() << "Manifest missing output path";
        return false;
    }
    
    if (manifest["engine"].toString().isEmpty()) {
        qWarning() << "Manifest missing engine specification";
        return false;
    }
    
    return true;
}

bool JobManifestManager::executeJob(const QString& jobPath, Engine engine) {
    try {
        QString scriptPath = getEngineScriptPath(engine);
        
        if (!QFile::exists(scriptPath)) {
            qWarning() << "Script not found:" << scriptPath;
            return false;
        }
        
        QProcess process;
        QStringList arguments;
        
        switch (engine) {
            case Engine::Blender: {
                // Try to find Blender executable
                QString blenderExe = "blender";
                
                // Try common Blender installation paths on Windows
                QStringList blenderPaths = {
                    "blender",  // Try PATH first
                    "C:/Program Files/Blender Foundation/Blender 4.3/blender.exe",
                    "C:/Program Files/Blender Foundation/Blender 4.2/blender.exe",
                    "C:/Program Files/Blender Foundation/Blender 4.1/blender.exe",
                    "C:/Program Files (x86)/Blender Foundation/Blender/blender.exe"
                };
                
                for (const QString& path : blenderPaths) {
                    if (QFile::exists(path)) {
                        blenderExe = path;
                        break;
                    }
                }
                
                arguments << "-b" << "-P" << scriptPath << "--" << jobPath;
                process.start(blenderExe, arguments);
                break;
            }
            
            case Engine::Unreal: {
                // Try to find Unreal Editor executable
                QString unrealExe = "unrealed.exe";
                
                // Try common Unreal Engine installation paths
                QStringList unrealPaths = {
                    "UnrealEditor.exe",  // Try PATH first
                    "C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor.exe",
                    "C:/Program Files/Epic Games/UE_5.5/Engine/Binaries/Win64/UnrealEditor.exe",
                    "C:/Program Files/Epic Games/UE_5.4/Engine/Binaries/Win64/UnrealEditor.exe",
                    "C:/Program Files (x86)/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor.exe"
                };
                
                for (const QString& path : unrealPaths) {
                    if (QFile::exists(path)) {
                        unrealExe = path;
                        break;
                    }
                }
                
                // Launch Unreal with Python script
                arguments << scriptPath << "-" << jobPath;
                process.start(unrealExe, arguments);
                break;
            }
            
            case Engine::FFmpeg: { // <<< FFmpeg and updated logic
                // The FFmpeg execution is handled by running the Python script
                QString pythonExe = "python";
                // The Python script expects the job manifest path as an argument
                arguments << scriptPath << "--job_manifest" << jobPath; 
                process.start(pythonExe, arguments);
                break;
            }
        }
        
        if (!process.waitForStarted()) {
            qWarning() << "Failed to start process";
            return false;
        }
        
        qDebug() << "Job execution started:" << jobPath;
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Exception executing job:" << e.what();
        return false;
    }
}

bool JobManifestManager::executeJobFromObject(const QJsonObject& jobManifest) {
    try {
        // 1. Determine the engine to get the correct file extension/naming
        QString engineStr = jobManifest["engine"].toString();
        Engine engine;
        if (engineStr == "blender") engine = Engine::Blender;
        else if (engineStr == "unreal") engine = Engine::Unreal;
        else engine = Engine::FFmpeg; // Default to FFmpeg for post-production

        // 2. Create a temporary file path for the manifest
        QString tempDir = QDir::tempPath();
        QString jobId = jobManifest["job_id"].toString();
        if (jobId.isEmpty()) jobId = generateJobId();
        
        QString tempFilePath = tempDir + "/" + jobId + "_manifest.json";

        // 3. Save the JSON object to the temporary file
        if (!saveManifest(jobManifest, tempFilePath)) {
            qWarning() << "Failed to save temporary manifest for execution:" << tempFilePath;
            return false;
        }

        // 4. Execute the job using the existing file-based method
        bool success = executeJob(tempFilePath, engine);
        
        // Note: We don't delete the temp file immediately because 
        // QProcess starts asynchronously. The engine scripts will handle the manifest.
        
        return success;
    } catch (const std::exception& e) {
        qWarning() << "Exception in executeJobFromObject:" << e.what();
        return false;
    }
}