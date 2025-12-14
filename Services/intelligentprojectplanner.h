#ifndef INTELLIGENTPROJECTPLANNER_H
#define INTELLIGENTPROJECTPLANNER_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <memory>

/**
 * @class IntelligentProjectPlanner
 * @brief AI-powered project planning that understands complex creative requests
 * 
 * Uses LLM to:
 * 1. Understand natural language project descriptions
 * 2. Break down complex projects into manageable tasks
 * 3. Estimate timeline and resource requirements
 * 4. Generate detailed production schedules
 * 5. Identify potential issues and suggest solutions
 * 
 * Capabilities:
 * - "Create a sci-fi cinematic" → Full production workflow
 * - "Build an action game" → Complete game pipeline
 * - "Make a product commercial" → Optimized ad production
 * - "Animate a character walk cycle" → Focused animation job
 */
class IntelligentProjectPlanner {
public:
    enum class ProjectComplexity {
        Simple,      // Single shot, single asset
        Moderate,    // Multiple scenes, 2-3 assets
        Complex,     // Multi-scene, many assets, special effects
        Enterprise   // Production-scale project
    };

    enum class ProductionPhase {
        Concept,
        Storyboard,
        AssetCreation,
        Production,
        Rendering,
        PostProduction,
        Distribution
    };

    struct ProjectAnalysis {
        QString projectId;
        ProjectComplexity complexity;
        int estimatedDurationDays = 0;
        int estimatedResourcesNeeded = 0;
        QStringList requiredTools;  // "blender", "unreal", "davinci", etc.
        QStringList requiredAssets;
        QStringList productionPhases;
        QString criticalPath;  // The longest phase
        QJsonArray detailedBreakdown;
    };

    struct TaskBreakdown {
        QString taskId;
        QString name;
        QString description;
        ProductionPhase phase;
        int estimatedHours = 0;
        int priority = 1;
        QStringList dependencies;
        QString assignedEngine;  // "blender", "unreal", "davinci"
        QJsonObject jobManifest;
    };

    struct ProductionSchedule {
        QString projectId;
        QString projectName;
        QList<TaskBreakdown> tasks;
        int totalEstimatedHours = 0;
        int parallelizablePhases = 0;
        QString criticalPathSummary;
        QJsonArray timeline;
    };

    IntelligentProjectPlanner(const QString& llmEndpoint = "http://localhost:8080");
    ~IntelligentProjectPlanner();

    /**
     * Analyze project description with LLM
     */
    bool analyzeProject(const QString& projectDescription, ProjectAnalysis& outAnalysis);

    /**
     * Break down project into detailed tasks
     */
    bool breakdownProjectIntoTasks(const ProjectAnalysis& analysis,
                                  QList<TaskBreakdown>& outTasks);

    /**
     * Generate production schedule
     */
    bool generateProductionSchedule(const ProjectAnalysis& analysis,
                                   const QList<TaskBreakdown>& tasks,
                                   ProductionSchedule& outSchedule);

    /**
     * Estimate complexity level
     */
    ProjectComplexity estimateComplexity(const QString& description);

    /**
     * Identify required tools
     */
    QStringList identifyRequiredTools(const QString& description);

    /**
     * Estimate project duration
     */
    int estimateProjectDuration(const QString& description, ProjectComplexity complexity);

    /**
     * Generate task from phase and requirements
     */
    bool generateTask(const QString& phaseName,
                     const QStringList& requirements,
                     TaskBreakdown& outTask);

    /**
     * Optimize schedule for parallelization
     */
    bool optimizeScheduleForParallelization(ProductionSchedule& schedule);

    /**
     * Identify dependencies between tasks
     */
    bool identifyTaskDependencies(QList<TaskBreakdown>& tasks);

    /**
     * Generate resource requirements
     */
    QJsonObject generateResourceRequirements(const ProjectAnalysis& analysis);

    /**
     * Create job manifests for all tasks
     */
    QJsonArray createJobManifestsForAllTasks(const QList<TaskBreakdown>& tasks);

    /**
     * Get LLM analysis of project
     */
    bool queryLLMForProjectAnalysis(const QString& description, QJsonObject& outAnalysis);

private:
    QString m_llmEndpoint;

    // Helper methods for LLM queries
    QString buildProjectAnalysisPrompt(const QString& description);
    QString buildTaskBreakdownPrompt(const ProjectAnalysis& analysis);
    QString buildScheduleOptimizationPrompt(const ProductionSchedule& schedule);

    // Parse LLM responses
    bool parseProjectAnalysisFromLLM(const QString& response, ProjectAnalysis& outAnalysis);
    bool parseTaskBreakdownFromLLM(const QString& response, QList<TaskBreakdown>& outTasks);
    bool parseScheduleFromLLM(const QString& response, ProductionSchedule& outSchedule);

    // Task estimation
    int estimateTaskDuration(const TaskBreakdown& task);
    int estimateRenderTime(const QString& description);
    int estimateAssetCreationTime(const QString& assetType);
    int estimatePostProductionTime(const QString& description);
};

#endif // INTELLIGENTPROJECTPLANNER_H
