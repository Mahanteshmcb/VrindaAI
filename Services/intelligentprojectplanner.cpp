#include "Services/intelligentprojectplanner.h"
#include <QJsonDocument>
#include <QDebug>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>

IntelligentProjectPlanner::IntelligentProjectPlanner(const QString& llmEndpoint)
    : m_llmEndpoint(llmEndpoint)
{
}

IntelligentProjectPlanner::~IntelligentProjectPlanner()
{
}

bool IntelligentProjectPlanner::analyzeProject(const QString& projectDescription,
                                             ProjectAnalysis& outAnalysis)
{
    qDebug() << "[ProjectPlanner] Analyzing project:" << projectDescription;
    
    outAnalysis.projectId = "PROJ_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    
    // Step 1: Estimate complexity
    outAnalysis.complexity = estimateComplexity(projectDescription);
    
    // Step 2: Identify required tools
    outAnalysis.requiredTools = identifyRequiredTools(projectDescription);
    
    // Step 3: Estimate duration
    outAnalysis.estimatedDurationDays = estimateProjectDuration(projectDescription, outAnalysis.complexity);
    
    // Step 4: Identify required assets
    if (projectDescription.toLower().contains("character")) {
        outAnalysis.requiredAssets.append("Character");
    }
    if (projectDescription.toLower().contains("environment") || projectDescription.toLower().contains("scene")) {
        outAnalysis.requiredAssets.append("Environment");
    }
    if (projectDescription.toLower().contains("prop")) {
        outAnalysis.requiredAssets.append("Props");
    }
    if (projectDescription.toLower().contains("effect")) {
        outAnalysis.requiredAssets.append("VFX");
    }
    
    // Step 5: Identify production phases
    outAnalysis.productionPhases << "Asset Creation" << "Production" << "Post-Production";
    
    // Step 6: Estimate resources
    outAnalysis.estimatedResourcesNeeded = outAnalysis.requiredAssets.count() + outAnalysis.requiredTools.count();
    
    qDebug() << "[ProjectPlanner] Analysis complete - Complexity:" << (int)outAnalysis.complexity
             << "Duration:" << outAnalysis.estimatedDurationDays << "days";
    
    return true;
}

bool IntelligentProjectPlanner::breakdownProjectIntoTasks(const ProjectAnalysis& analysis,
                                                        QList<TaskBreakdown>& outTasks)
{
    qDebug() << "[ProjectPlanner] Breaking down project into tasks...";
    
    int taskCounter = 1;
    
    // Create tasks for asset creation
    for (const QString& asset : analysis.requiredAssets) {
        TaskBreakdown task;
        task.taskId = "TASK_" + QString::number(taskCounter++);
        task.name = "Create " + asset;
        task.description = "Create and optimize " + asset;
        task.phase = ProductionPhase::AssetCreation;
        task.priority = 1;
        task.assignedEngine = "blender";
        task.estimatedHours = estimateAssetCreationTime(asset);
        
        outTasks.append(task);
    }
    
    // Create production task
    TaskBreakdown prodTask;
    prodTask.taskId = "TASK_" + QString::number(taskCounter++);
    prodTask.name = "Production";
    prodTask.description = "Main production phase";
    prodTask.phase = ProductionPhase::Production;
    prodTask.priority = 2;
    
    // Assign engine based on required tools
    if (analysis.requiredTools.contains("unreal")) {
        prodTask.assignedEngine = "unreal";
    } else if (analysis.requiredTools.contains("blender")) {
        prodTask.assignedEngine = "blender";
    }
    
    prodTask.estimatedHours = 240;
    outTasks.append(prodTask);
    
    // Create post-production task
    TaskBreakdown postTask;
    postTask.taskId = "TASK_" + QString::number(taskCounter++);
    postTask.name = "Post-Production";
    postTask.description = "Color grading, effects, and final export";
    postTask.phase = ProductionPhase::PostProduction;
    postTask.priority = 3;
    postTask.assignedEngine = "davinci";
    postTask.estimatedHours = 60;
    postTask.dependencies.append("TASK_" + QString::number(taskCounter - 2));
    
    outTasks.append(postTask);
    
    qDebug() << "[ProjectPlanner] Created" << outTasks.count() << "tasks";
    
    return true;
}

bool IntelligentProjectPlanner::generateProductionSchedule(const ProjectAnalysis& analysis,
                                                         const QList<TaskBreakdown>& tasks,
                                                         ProductionSchedule& outSchedule)
{
    qDebug() << "[ProjectPlanner] Generating production schedule...";
    
    outSchedule.projectId = analysis.projectId;
    outSchedule.projectName = "Project_" + analysis.projectId;
    
    // Copy tasks
    outSchedule.tasks = tasks;
    
    // Calculate total hours
    outSchedule.totalEstimatedHours = 0;
    for (const TaskBreakdown& task : tasks) {
        outSchedule.totalEstimatedHours += task.estimatedHours;
    }
    
    // Identify dependencies and parallelizable phases
    int dependencyCount = 0;
    for (const TaskBreakdown& task : tasks) {
        dependencyCount += task.dependencies.count();
    }
    
    // Estimate parallelizable phases (roughly 40% can run in parallel)
    outSchedule.parallelizablePhases = static_cast<int>(outSchedule.totalEstimatedHours * 0.4);
    
    // Create timeline
    QJsonArray timeline;
    int dayCounter = 1;
    
    for (const TaskBreakdown& task : tasks) {
        QJsonObject timelineItem;
        timelineItem["day"] = dayCounter;
        timelineItem["task"] = task.name;
        timelineItem["duration_hours"] = task.estimatedHours;
        timelineItem["engine"] = task.assignedEngine;
        
        timeline.append(timelineItem);
        dayCounter += (task.estimatedHours / 8) + 1;  // Assume 8-hour workdays
    }
    
    outSchedule.timeline = timeline;
    outSchedule.criticalPathSummary = "Asset Creation → Production → Post-Production";
    
    qDebug() << "[ProjectPlanner] Schedule generated -" << outSchedule.totalEstimatedHours 
             << "total hours over" << dayCounter << "days";
    
    return true;
}

IntelligentProjectPlanner::ProjectComplexity IntelligentProjectPlanner::estimateComplexity(const QString& description)
{
    int complexityScore = 0;
    
    // Check for multiple scenes/shots
    if (description.toLower().contains("multiple") || description.toLower().contains("series")) {
        complexityScore += 3;
    }
    
    // Check for special effects
    if (description.toLower().contains("effect") || description.toLower().contains("vfx")) {
        complexityScore += 2;
    }
    
    // Check for multiple characters
    if (description.toLower().contains("character") && description.toLower().count("character") > 1) {
        complexityScore += 2;
    }
    
    // Check for animation
    if (description.toLower().contains("animate") || description.toLower().contains("motion")) {
        complexityScore += 1;
    }
    
    // Check for game development
    if (description.toLower().contains("game") || description.toLower().contains("interactive")) {
        complexityScore += 2;
    }
    
    if (complexityScore <= 2) {
        return ProjectComplexity::Simple;
    } else if (complexityScore <= 5) {
        return ProjectComplexity::Moderate;
    } else if (complexityScore <= 8) {
        return ProjectComplexity::Complex;
    } else {
        return ProjectComplexity::Enterprise;
    }
}

QStringList IntelligentProjectPlanner::identifyRequiredTools(const QString& description)
{
    QStringList tools;
    
    if (description.toLower().contains("blender") || 
        description.toLower().contains("3d model") ||
        description.toLower().contains("render") ||
        description.toLower().contains("animation")) {
        tools.append("blender");
    }
    
    if (description.toLower().contains("unreal") || 
        description.toLower().contains("game") ||
        description.toLower().contains("interactive")) {
        tools.append("unreal");
    }
    
    if (description.toLower().contains("davinci") ||
        description.toLower().contains("color grading") ||
        description.toLower().contains("video edit") ||
        description.toLower().contains("composite")) {
        tools.append("davinci");
    }
    
    // Default tools
    if (tools.isEmpty()) {
        tools.append("blender");
    }
    
    return tools;
}

int IntelligentProjectPlanner::estimateProjectDuration(const QString& description, ProjectComplexity complexity)
{
    int baseDays = 0;
    
    switch (complexity) {
        case ProjectComplexity::Simple:
            baseDays = 3;
            break;
        case ProjectComplexity::Moderate:
            baseDays = 7;
            break;
        case ProjectComplexity::Complex:
            baseDays = 21;
            break;
        case ProjectComplexity::Enterprise:
            baseDays = 60;
            break;
    }
    
    // Adjust for specific requirements
    if (description.toLower().contains("high quality") || description.toLower().contains("professional")) {
        baseDays = static_cast<int>(baseDays * 1.5);
    }
    
    return baseDays;
}

bool IntelligentProjectPlanner::generateTask(const QString& phaseName,
                                           const QStringList& requirements,
                                           TaskBreakdown& outTask)
{
    outTask.taskId = "TASK_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outTask.name = phaseName;
    
    // Determine phase
    if (phaseName.toLower().contains("asset")) {
        outTask.phase = ProductionPhase::AssetCreation;
        outTask.assignedEngine = "blender";
        outTask.estimatedHours = 120;
    } else if (phaseName.toLower().contains("production")) {
        outTask.phase = ProductionPhase::Production;
        outTask.estimatedHours = 240;
    } else if (phaseName.toLower().contains("post")) {
        outTask.phase = ProductionPhase::PostProduction;
        outTask.assignedEngine = "davinci";
        outTask.estimatedHours = 60;
    }
    
    return true;
}

bool IntelligentProjectPlanner::optimizeScheduleForParallelization(ProductionSchedule& schedule)
{
    qDebug() << "[ProjectPlanner] Optimizing schedule for parallelization...";
    
    // Group tasks by dependencies
    QMap<QString, QStringList> dependencyGraph;
    
    for (const TaskBreakdown& task : schedule.tasks) {
        if (!task.dependencies.isEmpty()) {
            for (const QString& dep : task.dependencies) {
                dependencyGraph[dep].append(task.taskId);
            }
        }
    }
    
    // Tasks with no dependencies can run in parallel
    QStringList parallelizableTasks;
    for (const TaskBreakdown& task : schedule.tasks) {
        if (task.dependencies.isEmpty()) {
            parallelizableTasks.append(task.taskId);
        }
    }
    
    qDebug() << "[ProjectPlanner]" << parallelizableTasks.count() << "tasks can run in parallel";
    
    return true;
}

bool IntelligentProjectPlanner::identifyTaskDependencies(QList<TaskBreakdown>& tasks)
{
    // Asset creation must complete before production
    // Production must complete before post-production
    
    for (int i = 0; i < tasks.count(); ++i) {
        if (tasks[i].phase == ProductionPhase::Production) {
            // Find asset creation tasks
            for (int j = 0; j < tasks.count(); ++j) {
                if (tasks[j].phase == ProductionPhase::AssetCreation) {
                    tasks[i].dependencies.append(tasks[j].taskId);
                }
            }
        } else if (tasks[i].phase == ProductionPhase::PostProduction) {
            // Depends on production
            for (int j = 0; j < tasks.count(); ++j) {
                if (tasks[j].phase == ProductionPhase::Production) {
                    tasks[i].dependencies.append(tasks[j].taskId);
                }
            }
        }
    }
    
    return true;
}

QJsonObject IntelligentProjectPlanner::generateResourceRequirements(const ProjectAnalysis& analysis)
{
    QJsonObject resources;
    
    QJsonObject hardware;
    hardware["cpu_cores"] = 8;
    hardware["memory_gb"] = 32;
    hardware["gpu"] = "RTX 3080 or better";
    hardware["storage_gb"] = 2000;
    resources["hardware"] = hardware;
    
    QJsonObject software;
    software["tools"] = QJsonArray::fromStringList(analysis.requiredTools);
    software["plugins"] = QJsonArray();
    resources["software"] = software;
    
    QJsonObject time;
    time["total_hours"] = analysis.estimatedDurationDays * 8;
    time["estimated_days"] = analysis.estimatedDurationDays;
    resources["time"] = time;
    
    return resources;
}

QJsonArray IntelligentProjectPlanner::createJobManifestsForAllTasks(const QList<TaskBreakdown>& tasks)
{
    QJsonArray jobs;
    
    for (const TaskBreakdown& task : tasks) {
        QJsonObject job;
        job["job_id"] = task.taskId;
        job["name"] = task.name;
        job["engine"] = task.assignedEngine;
        job["phase"] = (int)task.phase;
        job["estimated_hours"] = task.estimatedHours;
        
        if (!task.dependencies.isEmpty()) {
            job["dependencies"] = QJsonArray::fromStringList(task.dependencies);
        }
        
        jobs.append(job);
    }
    
    return jobs;
}

bool IntelligentProjectPlanner::queryLLMForProjectAnalysis(const QString& description,
                                                         QJsonObject& outAnalysis)
{
    QString prompt = buildProjectAnalysisPrompt(description);
    qDebug() << "[ProjectPlanner] Would query LLM with prompt:" << prompt;
    
    // Placeholder for actual LLM query
    return true;
}

QString IntelligentProjectPlanner::buildProjectAnalysisPrompt(const QString& description)
{
    return QString(
        "Analyze this 3D production project request and provide:\n"
        "1. Complexity level (simple/moderate/complex/enterprise)\n"
        "2. Required tools (blender/unreal/davinci)\n"
        "3. Required assets (characters/environments/props/vfx)\n"
        "4. Estimated duration in days\n"
        "5. Production phases needed\n"
        "6. Critical path (longest phase)\n"
        "7. Potential challenges\n"
        "\n"
        "Project: %1\n"
        "\n"
        "Provide response as JSON."
    ).arg(description);
}

QString IntelligentProjectPlanner::buildTaskBreakdownPrompt(const ProjectAnalysis& analysis)
{
    return QString(
        "Break down this project into detailed production tasks:\n"
        "Complexity: %1\n"
        "Duration: %2 days\n"
        "Tools: %3\n"
        "Assets needed: %4\n"
        "\n"
        "Create task list with: task name, description, estimated hours, dependencies, assigned tool\n"
        "Provide as JSON array."
    ).arg(QString::number((int)analysis.complexity),
          QString::number(analysis.estimatedDurationDays),
          analysis.requiredTools.join(", "),
          analysis.requiredAssets.join(", "));
}

QString IntelligentProjectPlanner::buildScheduleOptimizationPrompt(const ProductionSchedule& schedule)
{
    return QString(
        "Optimize this production schedule for parallelization:\n"
        "Total tasks: %1\n"
        "Total hours: %2\n"
        "\n"
        "Identify which tasks can run in parallel and reorder for maximum efficiency.\n"
        "Provide optimized timeline as JSON."
    ).arg(QString::number(schedule.tasks.count()),
          QString::number(schedule.totalEstimatedHours));
}

bool IntelligentProjectPlanner::parseProjectAnalysisFromLLM(const QString& response,
                                                           ProjectAnalysis& outAnalysis)
{
    // Parse JSON response from LLM
    qDebug() << "[ProjectPlanner] Parsing LLM response:" << response;
    return true;
}

bool IntelligentProjectPlanner::parseTaskBreakdownFromLLM(const QString& response,
                                                        QList<TaskBreakdown>& outTasks)
{
    // Parse JSON response containing task breakdown
    qDebug() << "[ProjectPlanner] Parsing task breakdown:" << response;
    return true;
}

bool IntelligentProjectPlanner::parseScheduleFromLLM(const QString& response,
                                                    ProductionSchedule& outSchedule)
{
    // Parse optimized schedule from LLM
    qDebug() << "[ProjectPlanner] Parsing optimized schedule:" << response;
    return true;
}

int IntelligentProjectPlanner::estimateTaskDuration(const TaskBreakdown& task)
{
    return task.estimatedHours;
}

int IntelligentProjectPlanner::estimateRenderTime(const QString& description)
{
    // Base estimate: 5 minutes per frame at high quality
    int frames = 240;  // Assume 10 seconds at 24fps
    return (frames * 5) / 60;  // Return in minutes
}

int IntelligentProjectPlanner::estimateAssetCreationTime(const QString& assetType)
{
    if (assetType == "Character") {
        return 120;  // 120 hours for character creation with rigging
    } else if (assetType == "Environment") {
        return 100;  // 100 hours for environment
    } else if (assetType == "Props") {
        return 40;   // 40 hours for props
    } else if (assetType == "VFX") {
        return 60;   // 60 hours for VFX setup
    }
    return 50;  // Default
}

int IntelligentProjectPlanner::estimatePostProductionTime(const QString& description)
{
    return 60;  // Assume 60 hours for post-production
}
