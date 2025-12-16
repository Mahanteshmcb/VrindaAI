#ifndef JOBMANIFESTMANAGER_H
#define JOBMANIFESTMANAGER_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QList>
#include <QVector3D>
#include <memory>

/**
 * @class JobManifestManager
 * @brief Manages the creation, parsing, and execution of VrindaAI Job Manifests (JSON)
 * * This class handles:
 * - Creation of job tickets from task descriptions
 * - Validation of job manifests against schema
 * - Serialization to JSON files
 * - Execution of jobs via external engines (Blender, Unreal, FFmpeg) // MODIFIED
 */
class JobManifestManager {
public:
    enum class Engine {
        Blender,
        Unreal,
        FFmpeg // <<< MODIFIED: Replaced DaVinci
    };

    struct JobConfig {
        QString jobId;
        QString projectName;
        QString sceneName;
        Engine engine;
        int resolutionWidth = 1920;
        int resolutionHeight = 1080;
        int fps = 24;
        QString outputPath;
        QJsonObject metadata;
    };

    JobManifestManager();
    ~JobManifestManager();

    /**
     * Create a new job manifest
     */
    QJsonObject createJobManifest(const JobConfig& config);

    /**
     * Add asset to job manifest
     */
    void addAsset(QJsonObject& manifest, const QString& assetType, 
                 const QString& assetId, const QString& assetName, 
                 const QString& assetPath, const QString& format = "glb");

    /**
     * Add action to job manifest
     */
    void addAction(QJsonObject& manifest, const QString& actionType,
                  const QString& target, const QJsonObject& parameters);

    /**
     * Set camera configuration in manifest
     */
    void setCameraConfig(QJsonObject& manifest, 
                        const QVector3D& position,
                        const QVector3D& rotation = QVector3D(0, 0, 0),
                        float focalLength = 50.0f);

    /**
     * Set lighting configuration in manifest
     */
    void setLightingConfig(QJsonObject& manifest, const QJsonObject& lightingConfig);

    /**
     * Save manifest to JSON file
     */
    bool saveManifest(const QJsonObject& manifest, const QString& filePath);

    /**
     * Load manifest from JSON file
     */
    QJsonObject loadManifest(const QString& filePath);

    /**
     * Validate manifest against schema
     */
    bool validateManifest(const QJsonObject& manifest);

    /**
     * Execute job in specified engine
     * Returns true if execution started successfully
     */
    bool executeJob(const QString& jobPath, Engine engine);

    /**
     * Execute job from a JSON object manifest
     * Returns true if execution started successfully
     */
    bool executeJobFromObject(const QJsonObject& jobManifest); // <<< ADDED to fix compilation

    /**
     * Get unique job ID
     */
    static QString generateJobId();

    /**
     * Get engine script path
     */
    QString getEngineScriptPath(Engine engine) const;

private:
    /**
     * Initialize default manifest structure
     */
    QJsonObject initializeManifest(const JobConfig& config);

    /**
     * Get engine script filename
     */
    QString getEngineScriptName(Engine engine) const;

    /**
     * Get application resources directory
     */
    QString getResourcesDir() const;
};

#endif // JOBMANIFESTMANAGER_H