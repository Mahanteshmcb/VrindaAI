#ifndef WORKFLOWEXECUTOR_H
#define WORKFLOWEXECUTOR_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <memory>
#include "Services/jobmanifestmanager.h"

/**
 * @class WorkflowExecutor
 * @brief Coordinates multi-stage workflows (Blender → Unreal → FFmpeg)
 * * This is the core orchestration engine that:
 * 1. Takes a project description
 * 2. Breaks it into scenes
 * 3. Generates job tickets for each scene (Blender, Unreal, FFmpeg)
 * 4. Executes them in sequence or parallel
 * 5. Collects outputs and assembles final video
 */
class WorkflowExecutor {
public:
    enum class WorkflowStage {
        Blender,    // 3D rendering
        Unreal,     // Cinematic sequences
        FFmpeg      // Video editing & assembly (Replaced DaVinci)
    };

    struct Scene {
        QString id;
        QString name;
        QString description;
        QString backgroundAsset;
        QString characterAsset;
        QString animation;
        int frameStart;
        int frameEnd;
        WorkflowStage targetEngine;
    };

    struct WorkflowConfig {
        QString projectName;
        QString outputPath;
        int resolutionWidth = 1920;
        int resolutionHeight = 1080;
        int fps = 24;
        QList<Scene> scenes;
        bool parallelExecution = false;
        bool continueOnError = false;
    };

    WorkflowExecutor();
    ~WorkflowExecutor();

    /**
     * Load workflow configuration from JSON file
     */
    bool loadWorkflowConfig(const QString& configPath);

    /**
     * Create workflow configuration programmatically
     */
    WorkflowConfig createWorkflow(const QString& projectName,
                                 const QString& outputPath);

    /**
     * Add scene to workflow
     */
    void addScene(WorkflowConfig& workflow, const Scene& scene);

    /**
     * Generate job ticket for a scene
     */
    QJsonObject generateSceneJob(const Scene& scene, const WorkflowConfig& config);

    /**
     * Generate all job tickets for workflow
     */
    QJsonArray generateAllJobs(const WorkflowConfig& config, const QString& jobsDirectory);

    /**
     * Execute complete workflow
     */
    bool executeWorkflow(const WorkflowConfig& config);

    /**
     * Execute workflow stage (Blender, Unreal, or FFmpeg)
     */
    bool executeStage(WorkflowStage stage, const QJsonArray& jobs);

    /**
     * Assemble final video from all renders
     */
    bool assembleVideo(const WorkflowConfig& config, const QStringList& renderOutputs);

    /**
     * Get workflow status/progress
     */
    QString getWorkflowStatus() const { return m_statusMessage; }

private:
    JobManifestManager m_manifestManager;
    QString m_statusMessage;
    int m_completedJobs = 0;
    int m_totalJobs = 0;

    /**
     * Create Blender job for scene
     */
    QJsonObject createBlenderJob(const Scene& scene, const WorkflowConfig& config);

    /**
     * Create Unreal job for scene
     */
    QJsonObject createUnrealJob(const Scene& scene, const WorkflowConfig& config);

    /**
     * Create FFmpeg assembly job
     */
    QJsonObject createFfmpegAssemblyJob(const QStringList& renderPaths,
                                        const WorkflowConfig& config);

    /**
     * Wait for all jobs in a stage to complete
     */
    bool waitForJobCompletion(const QJsonArray& jobs);

    /**
     * Update status message
     */
    void updateStatus(const QString& message);
};

#endif // WORKFLOWEXECUTOR_H