#include "workflowexecutor.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QDebug>
#include <QDateTime>

WorkflowExecutor::WorkflowExecutor() {
    // Constructor
}

WorkflowExecutor::~WorkflowExecutor() {
    // Destructor
}

bool WorkflowExecutor::loadWorkflowConfig(const QString& configPath) {
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open workflow config:" << configPath;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        qWarning() << "Invalid workflow config format";
        return false;
    }
    
    qDebug() << "Loaded workflow config:" << configPath;
    return true;
}

WorkflowExecutor::WorkflowConfig WorkflowExecutor::createWorkflow(const QString& projectName,
                                                                  const QString& outputPath) {
    WorkflowConfig config;
    config.projectName = projectName;
    config.outputPath = outputPath;
    return config;
}

void WorkflowExecutor::addScene(WorkflowConfig& workflow, const Scene& scene) {
    workflow.scenes.append(scene);
    qDebug() << "Added scene:" << scene.name << "targeting" 
            << (scene.targetEngine == WorkflowStage::Blender ? "Blender" :
                scene.targetEngine == WorkflowStage::Unreal ? "Unreal" : "FFmpeg"); // <<< FIXED
}

QJsonObject WorkflowExecutor::createBlenderJob(const Scene& scene, const WorkflowConfig& config) {
    JobManifestManager::JobConfig jobConfig;
    jobConfig.projectName = config.projectName;
    jobConfig.sceneName = scene.name;
    jobConfig.engine = JobManifestManager::Engine::Blender;
    jobConfig.resolutionWidth = config.resolutionWidth;
    jobConfig.resolutionHeight = config.resolutionHeight;
    jobConfig.fps = config.fps;
    jobConfig.outputPath = QString("%1/%2_blender_%3.exr")
        .arg(config.outputPath, config.projectName, scene.id);
    
    QJsonObject manifest = m_manifestManager.createJobManifest(jobConfig);
    
    // Add assets
    if (!scene.backgroundAsset.isEmpty()) {
        m_manifestManager.addAsset(manifest, "environments", "env_0", scene.name, 
                                  scene.backgroundAsset, "glb");
    }
    
    if (!scene.characterAsset.isEmpty()) {
        m_manifestManager.addAsset(manifest, "characters", "char_0", "Character",
                                  scene.characterAsset, "fbx");
    }
    
    // Add render action
    QJsonObject renderParams;
    renderParams["start_frame"] = scene.frameStart;
    renderParams["end_frame"] = scene.frameEnd;
    renderParams["output_path"] = jobConfig.outputPath;
    m_manifestManager.addAction(manifest, "render", "", renderParams);
    
    return manifest;
}

QJsonObject WorkflowExecutor::createUnrealJob(const Scene& scene, const WorkflowConfig& config) {
    JobManifestManager::JobConfig jobConfig;
    jobConfig.projectName = config.projectName;
    jobConfig.sceneName = scene.name;
    jobConfig.engine = JobManifestManager::Engine::Unreal;
    jobConfig.resolutionWidth = config.resolutionWidth;
    jobConfig.resolutionHeight = config.resolutionHeight;
    jobConfig.fps = config.fps;
    jobConfig.outputPath = QString("%1/%2_unreal_%3.exr")
        .arg(config.outputPath, config.projectName, scene.id);
    
    QJsonObject manifest = m_manifestManager.createJobManifest(jobConfig);
    
    // Add cinematic sequence creation action
    QJsonObject seqParams;
    seqParams["sequence_name"] = scene.name;
    seqParams["frame_count"] = scene.frameEnd - scene.frameStart;
    m_manifestManager.addAction(manifest, "create_sequence", "", seqParams);
    
    // Add character animation action
    if (!scene.animation.isEmpty()) {
        QJsonObject animParams;
        animParams["character"] = "char_0";
        animParams["animation"] = scene.animation;
        animParams["start_frame"] = scene.frameStart;
        animParams["duration"] = scene.frameEnd - scene.frameStart;
        m_manifestManager.addAction(manifest, "add_animation", "", animParams);
    }
    
    // Add render action
    QJsonObject renderParams;
    renderParams["format"] = "exr";
    m_manifestManager.addAction(manifest, "render", "", renderParams);
    
    return manifest;
}

QJsonObject WorkflowExecutor::createFfmpegAssemblyJob(const QStringList& renderPaths, // <<< RENAMED function
                                                      const WorkflowConfig& config) {
    JobManifestManager::JobConfig jobConfig;
    jobConfig.projectName = config.projectName;
    jobConfig.sceneName = "FinalAssembly";
    jobConfig.engine = JobManifestManager::Engine::FFmpeg; // <<< FIXED
    jobConfig.outputPath = config.outputPath + "/" + config.projectName + "_final.mp4";
    
    QJsonObject manifest = m_manifestManager.createJobManifest(jobConfig);
    
    // Create timeline
    QJsonObject timelineParams;
    timelineParams["timeline_name"] = config.projectName;
    timelineParams["resolution_width"] = config.resolutionWidth;
    timelineParams["resolution_height"] = config.resolutionHeight;
    timelineParams["fps"] = config.fps;
    m_manifestManager.addAction(manifest, "create_timeline", "", timelineParams);
    
    // Add clips action
    QJsonObject clipsParams;
    QJsonArray clipsArray;
    for (int i = 0; i < renderPaths.size(); ++i) {
        QJsonObject clip;
        clip["path"] = renderPaths[i];
        clip["start_frame"] = 0;
        clipsArray.append(clip);
    }
    clipsParams["clips"] = clipsArray;
    m_manifestManager.addAction(manifest, "add_clips", "", clipsParams);
    
    // Export action
    QJsonObject exportParams;
    exportParams["format"] = "mp4";
    exportParams["codec"] = "h264";
    exportParams["bitrate"] = "25M";
    m_manifestManager.addAction(manifest, "export", "", exportParams);
    
    return manifest;
}

QJsonObject WorkflowExecutor::generateSceneJob(const Scene& scene, const WorkflowConfig& config) {
    switch (scene.targetEngine) {
        case WorkflowStage::Blender:
            return createBlenderJob(scene, config);
        case WorkflowStage::Unreal:
            return createUnrealJob(scene, config);
        case WorkflowStage::FFmpeg: // <<< CHANGE THIS from WorkflowStage::DaVinci
            return createFfmpegAssemblyJob(QStringList(), config);
        default:
            return QJsonObject();
    }
}

QJsonArray WorkflowExecutor::generateAllJobs(const WorkflowConfig& config, const QString& jobsDirectory) {
    QJsonArray jobArray;
    
    // Create jobs directory if it doesn't exist
    QDir().mkpath(jobsDirectory);
    
    for (const Scene& scene : config.scenes) {
        QJsonObject jobManifest = generateSceneJob(scene, config);
        
        // Save job to file
        QString jobPath = jobsDirectory + "/" + scene.id + ".json";
        if (m_manifestManager.saveManifest(jobManifest, jobPath)) {
            jobArray.append(jobPath);
            qDebug() << "Generated job:" << jobPath;
        }
    }
    
    m_totalJobs = jobArray.size();
    return jobArray;
}

bool WorkflowExecutor::executeStage(WorkflowStage stage, const QJsonArray& jobs) {
    QString stageName = (stage == WorkflowStage::Blender ? "Blender" :
                        stage == WorkflowStage::Unreal ? "Unreal" : "FFmpeg"); // <<< FIXED
    
    updateStatus("Executing " + stageName + " stage with " + QString::number(jobs.size()) + " jobs");
    
    JobManifestManager::Engine engine = (stage == WorkflowStage::Blender ? JobManifestManager::Engine::Blender :
                                        stage == WorkflowStage::Unreal ? JobManifestManager::Engine::Unreal :
                                        JobManifestManager::Engine::FFmpeg); // <<< FIXED
    
    for (int i = 0; i < jobs.size(); ++i) {
        QString jobPath = jobs[i].toString();
        qDebug() << "Executing" << stageName << "job:" << jobPath;
        
        if (!m_manifestManager.executeJob(jobPath, engine)) {
            qWarning() << "Failed to execute job:" << jobPath;
            return false;
        }
        
        m_completedJobs++;
    }
    
    return true;
}

bool WorkflowExecutor::waitForJobCompletion(const QJsonArray& jobs) {
    // This would normally wait for async job completion
    // For now, it's a placeholder
    qDebug() << "Waiting for" << jobs.size() << "jobs to complete";
    return true;
}

bool WorkflowExecutor::executeWorkflow(const WorkflowConfig& config) {
    try {
        updateStatus("Starting workflow execution");
        
        // Create output directory
        QDir().mkpath(config.outputPath);
        
        // Separate scenes by target engine
        QList<Scene> blenderScenes, unrealScenes;
        
        for (const Scene& scene : config.scenes) {
            if (scene.targetEngine == WorkflowStage::Blender) {
                blenderScenes.append(scene);
            } else if (scene.targetEngine == WorkflowStage::Unreal) {
                unrealScenes.append(scene);
            }
        }
        
        // Generate all jobs
        QString jobsDir = config.outputPath + "/jobs";
        QJsonArray allJobs = generateAllJobs(config, jobsDir);
        
        // Execute Blender jobs
        if (!blenderScenes.isEmpty()) {
            updateStatus("Executing Blender renders");
            QJsonArray blenderJobs;
            for (const Scene& scene : blenderScenes) {
                blenderJobs.append(jobsDir + "/" + scene.id + ".json");
            }
            if (!executeStage(WorkflowStage::Blender, blenderJobs)) {
                updateStatus("Blender stage failed");
                return false;
            }
        }
        
        // Execute Unreal jobs
        if (!unrealScenes.isEmpty()) {
            updateStatus("Executing Unreal cinematics");
            QJsonArray unrealJobs;
            for (const Scene& scene : unrealScenes) {
                unrealJobs.append(jobsDir + "/" + scene.id + ".json");
            }
            if (!executeStage(WorkflowStage::Unreal, unrealJobs)) {
                updateStatus("Unreal stage failed");
                return false;
            }
        }
        
        // Execute FFmpeg assembly
        updateStatus("Assembling final video using FFmpeg"); // <<< FIXED
        
        // Collect all render outputs
        QStringList renderOutputs;
        for (const Scene& scene : config.scenes) {
            QString output = QString("%1/%2_%3_*.exr")
                .arg(config.outputPath, config.projectName, scene.id);
            renderOutputs.append(output);
        }
        
        if (!assembleVideo(config, renderOutputs)) {
            updateStatus("Video assembly failed");
            return false;
        }
        
        updateStatus("Workflow execution complete");
        qDebug() << "Workflow completed successfully";
        return true;
    } catch (const std::exception& e) {
        updateStatus("Workflow error: " + QString::fromStdString(std::string(e.what())));
        return false;
    }
}

bool WorkflowExecutor::assembleVideo(const WorkflowConfig& config, const QStringList& renderOutputs) {
    // Create FFmpeg assembly job
    QJsonObject ffmpegJob = createFfmpegAssemblyJob(renderOutputs, config); // <<< FIXED
    
    // Save and execute
    QString ffmpegJobPath = config.outputPath + "/jobs/ffmpeg_assembly.json"; // <<< FIXED
    if (!m_manifestManager.saveManifest(ffmpegJob, ffmpegJobPath)) {
        qWarning() << "Failed to save FFmpeg job"; // <<< FIXED
        return false;
    }
    
    return m_manifestManager.executeJob(ffmpegJobPath, JobManifestManager::Engine::FFmpeg); // <<< FIXED
}

void WorkflowExecutor::updateStatus(const QString& message) {
    m_statusMessage = "[" + QDateTime::currentDateTime().toString("hh:mm:ss") + "] " + message;
    qDebug() << m_statusMessage;
}