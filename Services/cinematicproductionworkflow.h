#ifndef CINEMATICPRODUCTIONWORKFLOW_H
#define CINEMATICPRODUCTIONWORKFLOW_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>

/**
 * @class CinematicProductionWorkflow
 * @brief Specialized workflow for complete cinematic/movie production pipeline
 * 
 * Manages complete cinematic production:
 * 1. Pre-Production (Concept, Storyboarding, Asset Planning)
 * 2. Production (Blender 3D Rendering) - Scenes, camera work, lighting
 * 3. Post-Production (DaVinci) - Color grading, effects, sound design
 * 4. Distribution - Final output for multiple platforms
 */
class CinematicProductionWorkflow {
public:
    enum class CinematicStyle {
        Realistic,
        Stylized,
        Photorealistic,
        Cartoon,
        SciFi,
        Fantasy,
        Documentary
    };

    enum class ShotType {
        Wide,
        Medium,
        Close,
        Overhead,
        POV,
        Tracking,
        Dolly,
        Crane
    };

    enum class LightingType {
        ThreePoint,
        HighKey,
        LowKey,
        Backlit,
        Rim,
        Natural,
        Dramatic
    };

    struct CinematicShot {
        QString id;
        QString name;
        QString description;
        ShotType shotType;
        int frameStart;
        int frameEnd;
        QString cameraMovement;  // e.g., "pan left", "dolly forward"
        LightingType lighting;
        QString environment;
        QStringList characters;
        QString musicTempo;  // e.g., "slow", "moderate", "fast"
    };

    struct ProductionAct {
        QString id;
        QString name;
        QString description;
        QList<CinematicShot> shots;
        int estimatedFrames = 0;
        QString mood;  // e.g., "tense", "dramatic", "peaceful"
    };

    struct CinematicSpecification {
        QString cinematicId;
        QString title;
        QString description;
        CinematicStyle style;
        int totalFrames = 240;  // 10 seconds at 24fps default
        int frameRate = 24;
        int resolutionX = 1920;
        int resolutionY = 1080;
        QString aspectRatio = "16:9";
        
        QList<ProductionAct> acts;
        int estimatedRenderMinutes = 0;
        int estimatedPostMinutes = 0;
        
        // Assets needed
        QStringList requiredEnvironments;
        QStringList requiredCharacters;
        QStringList requiredProps;
        
        // Post-production
        bool needsColorGrading = true;
        bool needsVFX = false;
        bool needsSoundDesign = true;
        QString colorGradingStyle;
        QStringList vfxElements;
    };

    CinematicProductionWorkflow();
    ~CinematicProductionWorkflow();

    /**
     * Create cinematic specification from description
     */
    bool createCinematicSpecification(const QString& title,
                                     const QString& description,
                                     CinematicStyle style,
                                     CinematicSpecification& outSpec);

    /**
     * Generate shot from description
     */
    bool generateShotFromDescription(const QString& description,
                                    int frameStart,
                                    int frameEnd,
                                    CinematicShot& outShot);

    /**
     * Generate Blender rendering jobs for all shots
     */
    QJsonArray generateBlenderRenderingJobs(const CinematicSpecification& spec);

    /**
     * Generate Blender job for specific shot
     */
    QJsonObject generateShotRenderingJob(const CinematicShot& shot,
                                        const CinematicSpecification& spec);

    /**
     * Generate environment/scene setup job
     */
    QJsonObject generateEnvironmentSetupJob(const QString& environmentName,
                                           const QString& description,
                                           CinematicStyle style);

    /**
     * Generate character positioning job
     */
    QJsonObject generateCharacterPositioningJob(const QStringList& characters,
                                               const CinematicShot& shot);

    /**
     * Generate camera setup and keyframing job
     */
    QJsonObject generateCameraSetupJob(const CinematicShot& shot,
                                      const CinematicSpecification& spec);

    /**
     * Generate lighting setup job
     */
    QJsonObject generateLightingSetupJob(const CinematicShot& shot,
                                        CinematicStyle style);

    /**
     * Generate post-production (DaVinci) jobs
     */
    QJsonArray generatePostProductionJobs(const CinematicSpecification& spec);

    /**
     * Generate color grading job
     */
    QJsonObject generateColorGradingJob(const CinematicSpecification& spec);

    /**
     * Generate VFX and effects job
     */
    QJsonObject generateVFXJob(const CinematicSpecification& spec);

    /**
     * Generate sound design and audio mixing job
     */
    QJsonObject generateSoundDesignJob(const CinematicSpecification& spec);

    /**
     * Generate final export job
     */
    QJsonObject generateFinalExportJob(const CinematicSpecification& spec);

    /**
     * Generate complete cinematic workflow
     */
    QJsonArray generateCompleteWorkflow(const CinematicSpecification& spec);

    /**
     * Parse narrative description into shots and acts
     */
    bool parseNarrativeDescription(const QString& description,
                                  CinematicSpecification& outSpec);

    /**
     * Validate cinematic specification
     */
    bool validateCinematicSpec(const CinematicSpecification& spec);

private:
    // Helper methods
    QString getLightingPreset(LightingType type);
    QString getCameraMovementScript(const QString& movement);
    QString getColorGradingLUT(CinematicStyle style);
    int estimateShotRenderTime(const CinematicShot& shot);
    QString getShotTypeCamera(ShotType type);
};

#endif // CINEMATICPRODUCTIONWORKFLOW_H
