#include "Services/autonomousworkflowengine.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>
#include <QEventLoop>
#include <QTimer>

AutonomousWorkflowEngine::AutonomousWorkflowEngine() 
    : m_currentStageIndex(0), m_totalStages(0)
{
}

AutonomousWorkflowEngine::~AutonomousWorkflowEngine()
{
}

bool AutonomousWorkflowEngine::processProjectRequest(const ProjectRequest& request, 
                                                     WorkflowPlan& outPlan)
{
    updateStatus("Processing project request: " + request.projectName);
    
    // Step 1: Analyze and enhance request if enabled
    ProjectRequest enhancedRequest = request;
    if (request.autoImprove) {
        if (!analyzeAndEnhanceRequest(enhancedRequest)) {
            updateStatus("Warning: Could not enhance request, continuing with original");
            // Continue anyway
        }
    }
    
    // Step 2: Generate appropriate workflow based on project type
    bool success = false;
    switch (request.projectType) {
        case ProjectType::CinematicMovie:
        case ProjectType::VisualizationRender:
        case ProjectType::CommerialAd:
            success = generateCinematicWorkflow(enhancedRequest, outPlan);
            break;
            
        case ProjectType::GameProject:
            success = generateGameWorkflow(enhancedRequest, outPlan);
            break;
            
        case ProjectType::AnimationSequence:
            success = generateCinematicWorkflow(enhancedRequest, outPlan);
            break;
            
        default:
            updateStatus("Error: Unknown project type");
            return false;
    }
    
    if (!success) {
        updateStatus("Error: Could not generate workflow plan");
        return false;
    }
    
    // Step 3: Validate the workflow plan
    if (!validateWorkflowPlan(outPlan)) {
        updateStatus("Error: Generated workflow plan is invalid");
        return false;
    }
    
    m_currentProjectName = request.projectName;
    m_totalStages = outPlan.stages.count();
    
    updateStatus("Workflow plan generated successfully: " + QString::number(m_totalStages) + " stages");
    return true;
}

bool AutonomousWorkflowEngine::analyzeAndEnhanceRequest(ProjectRequest& request)
{
    updateStatus("Analyzing request with LLM...");
    
    QString prompt = QString(
        "As a professional 3D production director, analyze and enhance this project request:\n\n"
        "Project: %1\n"
        "Type: %2\n"
        "Description: %3\n"
        "Style: %4\n\n"
        "Provide:\n"
        "1. Enhanced description for realistic production\n"
        "2. Required assets list\n"
        "3. Recommended production stages\n"
        "4. Estimated total duration\n"
        "\n"
        "Format as JSON with keys: enhanced_description, assets, stages, duration_minutes"
    ).arg(request.projectName, 
          (request.projectType == ProjectType::CinematicMovie ? "Cinematic" : "Game"),
          request.description,
          request.creativeDirection);
    
    // Query LLM (would connect to localhost:8080/api/generate)
    // For now, we'll use the original descriptions
    // In production, this would call the actual LLM
    
    return true;
}

bool AutonomousWorkflowEngine::generateCinematicWorkflow(const ProjectRequest& request,
                                                         WorkflowPlan& outPlan)
{
    updateStatus("Generating cinematic workflow...");
    
    outPlan.workflowId = "WORKFLOW_CINEMA_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outPlan.projectName = request.projectName;
    outPlan.projectType = ProjectType::CinematicMovie;
    outPlan.description = "Professional cinematic production: Blender → DaVinci";
    
    QJsonArray stages;
    int totalMinutes = 0;
    
    // Stage 1: Blender 3D Rendering
    if (request.scope == ProductionScope::Complete || 
        request.scope == ProductionScope::Production) {
        
        QJsonObject blenderStage;
        blenderStage["stage_id"] = "BLEND_001";
        blenderStage["stage_name"] = "3D Environment Rendering";
        blenderStage["engine"] = "blender";
        blenderStage["description"] = "Render 3D cinematic scene with " + request.creativeDirection + " style";
        blenderStage["estimated_minutes"] = 120;
        blenderStage["priority"] = 1;
        
        // Create job manifest for Blender
        QJsonObject blenderJob = createBlenderSceneJob(request, request.description);
        blenderStage["job_manifest"] = blenderJob;
        
        stages.append(blenderStage);
        totalMinutes += 120;
    }
    
    // Stage 2: DaVinci Composition & Color Grading
    if (request.scope == ProductionScope::Complete || 
        request.scope == ProductionScope::PostProduction) {
        
        QJsonObject davinciStage;
        davinciStage["stage_id"] = "DAVINCI_001";
        davinciStage["stage_name"] = "Color Grading & Composition";
        davinciStage["engine"] = "davinci";
        davinciStage["description"] = "Apply color grading, effects, and compose final video";
        davinciStage["estimated_minutes"] = 45;
        davinciStage["priority"] = 2;
        davinciStage["depends_on"] = "BLEND_001";
        
        // Create job manifest for DaVinci
        QJsonObject davinciJob = createDaVinciPostJob(request, request.description);
        davinciStage["job_manifest"] = davinciJob;
        
        stages.append(davinciStage);
        totalMinutes += 45;
    }
    
    outPlan.stages = stages;
    outPlan.totalEstimatedMinutes = totalMinutes;
    
    updateStatus("Cinematic workflow generated: " + QString::number(stages.count()) + " stages");
    return true;
}

bool AutonomousWorkflowEngine::generateGameWorkflow(const ProjectRequest& request,
                                                    WorkflowPlan& outPlan)
{
    updateStatus("Generating game production workflow...");
    
    outPlan.workflowId = "WORKFLOW_GAME_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outPlan.projectName = request.projectName;
    outPlan.projectType = ProjectType::GameProject;
    outPlan.description = "Professional game production: Assets → Unreal Engine → Compilation";
    
    QJsonArray stages;
    int totalMinutes = 0;
    
    // Stage 1: Asset Creation (Blender)
    if (request.scope == ProductionScope::Complete || 
        request.scope == ProductionScope::PreProduction) {
        
        QJsonObject assetStage;
        assetStage["stage_id"] = "ASSET_001";
        assetStage["stage_name"] = "3D Asset Creation";
        assetStage["engine"] = "blender";
        assetStage["description"] = "Create and optimize game assets (characters, environments)";
        assetStage["estimated_minutes"] = 180;
        assetStage["priority"] = 1;
        
        QJsonObject blenderJob = createBlenderSceneJob(request, "Create game assets: " + request.description);
        assetStage["job_manifest"] = blenderJob;
        
        stages.append(assetStage);
        totalMinutes += 180;
    }
    
    // Stage 2: Game Development (Unreal)
    if (request.scope == ProductionScope::Complete || 
        request.scope == ProductionScope::Production) {
        
        QJsonObject gameStage;
        gameStage["stage_id"] = "UE_001";
        gameStage["stage_name"] = "Unreal Engine Development";
        gameStage["engine"] = "unreal";
        gameStage["description"] = "Develop game mechanics, import assets, build gameplay";
        gameStage["estimated_minutes"] = 240;
        gameStage["priority"] = 2;
        gameStage["depends_on"] = "ASSET_001";
        
        QJsonObject unrealJob = createUnrealGameJob(request, request.description);
        gameStage["job_manifest"] = unrealJob;
        
        stages.append(gameStage);
        totalMinutes += 240;
    }
    
    outPlan.stages = stages;
    outPlan.totalEstimatedMinutes = totalMinutes;
    
    updateStatus("Game workflow generated: " + QString::number(stages.count()) + " stages");
    return true;
}

bool AutonomousWorkflowEngine::generateCinematicTrailerWorkflow(const ProjectRequest& request,
                                                                WorkflowPlan& outPlan)
{
    updateStatus("Generating cinematic trailer workflow...");
    
    outPlan.workflowId = "WORKFLOW_TRAILER_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outPlan.projectName = request.projectName;
    outPlan.projectType = ProjectType::CinematicMovie;
    outPlan.description = "Game cinematic trailer: Blender/Unreal → DaVinci → Final Video";
    
    QJsonArray stages;
    int totalMinutes = 0;
    
    // Stage 1: Create cinematic in Blender or Unreal
    QJsonObject cinemaStage;
    cinemaStage["stage_id"] = "CINEMA_001";
    cinemaStage["stage_name"] = "Cinematic Sequence Creation";
    cinemaStage["engine"] = "blender";  // Could also be Unreal
    cinemaStage["description"] = "Create professional cinematic trailer for game";
    cinemaStage["estimated_minutes"] = 150;
    cinemaStage["priority"] = 1;
    
    QJsonObject cinemaJob = createBlenderSceneJob(request, "Create game trailer: " + request.description);
    cinemaStage["job_manifest"] = cinemaJob;
    
    stages.append(cinemaStage);
    totalMinutes += 150;
    
    // Stage 2: Post-production in DaVinci
    QJsonObject postStage;
    postStage["stage_id"] = "DAVINCI_002";
    postStage["stage_name"] = "Final Composition & Export";
    postStage["engine"] = "davinci";
    postStage["description"] = "Color grade, add effects, and export final trailer";
    postStage["estimated_minutes"] = 60;
    postStage["priority"] = 2;
    postStage["depends_on"] = "CINEMA_001";
    
    QJsonObject postJob = createDaVinciPostJob(request, "Finalize game trailer");
    postStage["job_manifest"] = postJob;
    
    stages.append(postStage);
    totalMinutes += 60;
    
    outPlan.stages = stages;
    outPlan.totalEstimatedMinutes = totalMinutes;
    
    return true;
}

bool AutonomousWorkflowEngine::executeWorkflowPlan(const WorkflowPlan& plan,
                                                   const QString& jobsDirectory)
{
    updateStatus("Starting workflow execution: " + plan.projectName);
    emit workflowStarted(plan.projectName);
    
    // Create jobs directory if it doesn't exist
    QDir dir;
    dir.mkpath(jobsDirectory);
    
    m_currentStageIndex = 0;
    m_totalStages = plan.stages.count();
    
    // Execute each stage in sequence
    for (int i = 0; i < plan.stages.count(); ++i) {
        if (!executeWorkflowStep(plan, i)) {
            updateStatus("Error: Stage " + QString::number(i) + " failed");
            emit errorOccurred("Stage failed: " + plan.stages[i].toObject()["stage_name"].toString());
            return false;
        }
    }
    
    updateStatus("Workflow completed successfully");
    emit workflowCompleted(plan.stages.last().toObject()["output_path"].toString("./output"));
    
    return true;
}

bool AutonomousWorkflowEngine::executeWorkflowStep(const WorkflowPlan& plan, int stageIndex)
{
    if (stageIndex < 0 || stageIndex >= plan.stages.count()) {
        return false;
    }
    
    QJsonObject stage = plan.stages[stageIndex].toObject();
    QString stageName = stage["stage_name"].toString();
    
    m_currentStageIndex = stageIndex;
    updateStatus("Executing stage " + QString::number(stageIndex + 1) + "/" + 
                 QString::number(m_totalStages) + ": " + stageName);
    emit stageStarted(stageName, stageIndex);
    
    // Get the job manifest
    QJsonObject jobManifest = stage["job_manifest"].toObject();
    
    if (!validateJobManifest(jobManifest)) {
        updateStatus("Error: Invalid job manifest for stage " + stageName);
        return false;
    }
    
    // Execute the job using JobManifestManager
    QString jobId = stage["stage_id"].toString();
    bool success = m_manifestManager.executeJobFromObject(jobManifest);
    
    if (success) {
        emit stageProgress(100);
        emit stageCompleted(stageName);
        return true;
    } else {
        return false;
    }
}

QString AutonomousWorkflowEngine::getExecutionStatus() const
{
    return m_statusMessage;
}

int AutonomousWorkflowEngine::getEstimatedTimeRemaining() const
{
    // Would calculate based on remaining stages
    return 0;
}

QJsonObject AutonomousWorkflowEngine::createBlenderSceneJob(const ProjectRequest& req,
                                                           const QString& sceneDescription)
{
    QJsonObject job;
    job["job_id"] = "BLENDER_" + req.projectName.replace(" ", "_");
    job["engine"] = "blender";
    job["job_type"] = "render_scene";
    job["description"] = sceneDescription;
    
    QJsonObject scene;
    scene["name"] = req.projectName;
    scene["description"] = sceneDescription;
    scene["creative_direction"] = req.creativeDirection;
    scene["frame_start"] = 1;
    scene["frame_end"] = (req.durationSeconds * req.fps);
    
    QJsonObject camera;
    camera["position"] = QJsonArray::fromStringList({"0", "0", "10"});
    camera["rotation"] = QJsonArray::fromStringList({"0", "0", "0"});
    camera["lens"] = 50;
    scene["camera"] = camera;
    
    QJsonObject render;
    render["resolution_x"] = req.resolutionWidth;
    render["resolution_y"] = req.resolutionHeight;
    render["fps"] = req.fps;
    render["samples"] = 256;
    render["engine"] = "cycles";
    render["device"] = "gpu";
    render["output_format"] = "exr";
    scene["render"] = render;
    
    job["scene"] = scene;
    
    QJsonObject output;
    output["path"] = req.outputPath + "/renders";
    output["format"] = "exr_sequence";
    job["output"] = output;
    
    return job;
}

QJsonObject AutonomousWorkflowEngine::createUnrealGameJob(const ProjectRequest& req,
                                                         const QString& gameDescription)
{
    QJsonObject job;
    job["job_id"] = "UNREAL_" + req.projectName.replace(" ", "_");
    job["engine"] = "unreal";
    job["job_type"] = "game_development";
    job["description"] = gameDescription;
    
    QJsonObject project;
    project["name"] = req.projectName;
    project["description"] = gameDescription;
    project["engine_version"] = "5.3";
    
    QJsonObject levels;
    levels["main_level"] = "MainGame";
    project["levels"] = levels;
    
    QJsonArray assets;
    for (const QString& asset : req.requiredAssets) {
        QJsonObject assetObj;
        assetObj["type"] = asset;
        assetObj["source"] = "./output/renders";
        assets.append(assetObj);
    }
    project["assets"] = assets;
    
    job["project"] = project;
    
    QJsonObject build;
    build["target"] = "Game";
    build["configuration"] = "Shipping";
    build["platform"] = "Windows";
    job["build"] = build;
    
    QJsonObject output;
    output["path"] = req.outputPath + "/game";
    output["format"] = "executable";
    job["output"] = output;
    
    return job;
}

QJsonObject AutonomousWorkflowEngine::createDaVinciPostJob(const ProjectRequest& req,
                                                          const QString& postDescription)
{
    QJsonObject job;
    job["job_id"] = "DAVINCI_" + req.projectName.replace(" ", "_");
    job["engine"] = "davinci";
    job["job_type"] = "color_grading_export";
    job["description"] = postDescription;
    
    QJsonObject timeline;
    timeline["name"] = req.projectName;
    timeline["resolution_x"] = req.resolutionWidth;
    timeline["resolution_y"] = req.resolutionHeight;
    timeline["frame_rate"] = req.fps;
    
    QJsonObject colorGrading;
    colorGrading["contrast"] = 1.1;
    colorGrading["saturation"] = 1.15;
    colorGrading["temperature"] = 5600;
    timeline["color_grading"] = colorGrading;
    
    job["timeline"] = timeline;
    
    QJsonObject output;
    output["path"] = req.outputPath + "/final";
    output["format"] = "mp4_h264";
    output["bitrate"] = "15Mbps";
    job["output"] = output;
    
    return job;
}

bool AutonomousWorkflowEngine::validateJobManifest(const QJsonObject& job)
{
    // Check required fields
    if (!job.contains("job_id") || !job.contains("engine") || !job.contains("job_type")) {
        return false;
    }
    return true;
}

bool AutonomousWorkflowEngine::validateWorkflowPlan(const WorkflowPlan& plan)
{
    if (plan.stages.isEmpty()) {
        return false;
    }
    
    for (const QJsonValue& stageVal : plan.stages) {
        QJsonObject stage = stageVal.toObject();
        if (!stage.contains("stage_id") || !stage.contains("engine")) {
            return false;
        }
    }
    
    return true;
}

void AutonomousWorkflowEngine::updateStatus(const QString& message)
{
    m_statusMessage = message;
    qDebug() << "[AutonomousWorkflow]" << message;
}

bool AutonomousWorkflowEngine::queryLLMForJobGeneration(const QString& description,
                                                        QJsonObject& outJob)
{
    // Placeholder for LLM integration
    // In production, would query llama.cpp server
    return true;
}

bool AutonomousWorkflowEngine::queryLLMForAssetList(const QString& description,
                                                    QStringList& outAssets)
{
    // Placeholder for LLM integration
    return true;
}

bool AutonomousWorkflowEngine::queryLLMForCreativeDirection(const QString& baseDescription,
                                                           const QString& style,
                                                           QString& outEnhanced)
{
    // Placeholder for LLM integration
    outEnhanced = baseDescription + " with " + style + " style";
    return true;
}
