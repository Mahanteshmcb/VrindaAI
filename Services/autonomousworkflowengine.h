#ifndef AUTONOMOUSWORKFLOWENGINE_H
#define AUTONOMOUSWORKFLOWENGINE_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <memory>
#include "Services/jobmanifestmanager.h"
#include "Services/workflowexecutor.h"

/**
 * @class AutonomousWorkflowEngine
 * @brief Converts natural language requests to complete workflows
 * 
 * This is the "Puppet Master" that:
 * 1. Takes user requests (text or structured input)
 * 2. Uses LLM to understand intent and requirements
 * 3. Generates job manifests for Blender, Unreal, DaVinci
 * 4. Orchestrates execution across all tools
 * 5. Returns final outputs (movies or game assets)
 * 
 * Supported Workflows:
 * - Cinema Production: Concept → Blender Render → DaVinci Composition → Movie
 * - Game Production: Concept → Unreal Development → Game Assets → Executable
 * - Mixed: Blender → Unreal → DaVinci (cinematic trailers for games)
 */
class AutonomousWorkflowEngine : public QObject {
    Q_OBJECT

public:
    enum class ProjectType {
        CinematicMovie,
        GameProject,
        AnimationSequence,
        VisualizationRender,
        CommerialAd
    };

    enum class ProductionScope {
        Concept,
        PreProduction,
        Production,
        PostProduction,
        Complete  // All stages
    };

    struct ProjectRequest {
        QString projectName;
        QString description;
        ProjectType projectType;
        ProductionScope scope;
        QString outputPath;
        int resolutionWidth = 1920;
        int resolutionHeight = 1080;
        int fps = 24;
        int durationSeconds = 30;
        QString creativeDirection;  // e.g., "sci-fi", "fantasy", "realistic"
        QStringList requiredAssets;  // e.g., ["character", "environment"]
        bool autoImprove = true;    // Use LLM to enhance descriptions
        QString llmEndpoint = "http://localhost:8080";
    };

    struct WorkflowPlan {
        QString workflowId;
        QString projectName;
        ProjectType projectType;
        QJsonArray stages;  // Array of execution stages
        int totalEstimatedMinutes = 0;
        QString description;
    };

    AutonomousWorkflowEngine();
    ~AutonomousWorkflowEngine();

    /**
     * Main entry point: Convert request to complete workflow execution
     */
    bool processProjectRequest(const ProjectRequest& request, WorkflowPlan& outPlan);

    /**
     * Enhanced request with LLM analysis
     */
    bool analyzeAndEnhanceRequest(ProjectRequest& request);

    /**
     * Generate production workflow for cinema
     */
    bool generateCinematicWorkflow(const ProjectRequest& request, WorkflowPlan& outPlan);

    /**
     * Generate production workflow for game
     */
    bool generateGameWorkflow(const ProjectRequest& request, WorkflowPlan& outPlan);

    /**
     * Generate mixed workflow (cinematic for game trailer)
     */
    bool generateCinematicTrailerWorkflow(const ProjectRequest& request, WorkflowPlan& outPlan);

    /**
     * Execute the complete workflow
     */
    bool executeWorkflowPlan(const WorkflowPlan& plan, const QString& jobsDirectory);

    /**
     * Execute workflow step-by-step (for progress tracking)
     */
    bool executeWorkflowStep(const WorkflowPlan& plan, int stageIndex);

    /**
     * Get current execution status
     */
    QString getExecutionStatus() const;

    /**
     * Get estimated time to completion
     */
    int getEstimatedTimeRemaining() const;

signals:
    void workflowStarted(const QString& projectName);
    void stageStarted(const QString& stageName, int stageIndex);
    void stageProgress(int progress);  // 0-100
    void stageCompleted(const QString& stageName);
    void workflowCompleted(const QString& outputPath);
    void errorOccurred(const QString& errorMessage);

private:
    JobManifestManager m_manifestManager;
    WorkflowExecutor m_workflowExecutor;
    
    QString m_currentProjectName;
    int m_currentStageIndex = 0;
    int m_totalStages = 0;
    QString m_statusMessage;
    QMap<QString, QString> m_stageOutputs;  // Map stage ID to output path

    // LLM Integration
    bool queryLLMForJobGeneration(const QString& description, QJsonObject& outJob);
    bool queryLLMForAssetList(const QString& description, QStringList& outAssets);
    bool queryLLMForCreativeDirection(const QString& baseDescription, 
                                      const QString& style, QString& outEnhanced);

    // Workflow generation helpers
    QJsonObject createBlenderSceneJob(const ProjectRequest& req, const QString& sceneDescription);
    QJsonObject createUnrealGameJob(const ProjectRequest& req, const QString& gameDescription);
    QJsonObject createDaVinciPostJob(const ProjectRequest& req, const QString& postDescription);

    // Job validation
    bool validateJobManifest(const QJsonObject& job);
    bool validateWorkflowPlan(const WorkflowPlan& plan);

    // Status updates
    void updateStatus(const QString& message);
    void onStageComplete(const QString& stageId, const QString& outputPath);
};

#endif // AUTONOMOUSWORKFLOWENGINE_H
