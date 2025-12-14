#ifndef GAMEPRODUCTIONWORKFLOW_H
#define GAMEPRODUCTIONWORKFLOW_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>

/**
 * @class GameProductionWorkflow
 * @brief Specialized workflow for complete game development pipeline
 * 
 * Manages the complete game development cycle:
 * 1. Asset Creation (Blender) - Characters, environments, props
 * 2. Game Development (Unreal) - Import assets, add mechanics, build levels
 * 3. Testing & Optimization - Profiling, bug fixes
 * 4. Compilation & Packaging - Build final executable
 */
class GameProductionWorkflow {
public:
    enum class GameAssetType {
        Character,
        Environment,
        Prop,
        VFX,
        Audio,
        UI,
        Animation
    };

    enum class GameMechanics {
        Movement,
        Combat,
        Inventory,
        Dialogue,
        Puzzle,
        Shooting,
        Platforming,
        RTS,
        Stealth
    };

    struct GameAssetSpec {
        QString id;
        GameAssetType type;
        QString name;
        QString description;
        int polyCount = 0;
        bool isAnimated = false;
        QStringList requiredAnimations;
    };

    struct GameLevel {
        QString id;
        QString name;
        QString description;
        int sizeX = 1000;
        int sizeY = 1000;
        int sizeZ = 1000;
        QStringList requiredAssets;
        QStringList requiredMechanics;
        QString environmentAsset;
        int estimatedCompletionMinutes = 60;
    };

    struct GameSpecification {
        QString gameId;
        QString gameName;
        QString description;
        QString genre;
        QString targetPlatform = "Windows";
        int targetFPS = 60;
        QList<GameAssetSpec> requiredAssets;
        QList<GameLevel> levels;
        QList<GameMechanics> mechanics;
        int estimatedTotalMinutes = 0;
    };

    GameProductionWorkflow();
    ~GameProductionWorkflow();

    /**
     * Create game specification from description
     */
    bool createGameSpecification(const QString& gameName,
                               const QString& description,
                               const QString& genre,
                               GameSpecification& outSpec);

    /**
     * Generate asset creation jobs (Blender)
     */
    QJsonArray generateAssetCreationJobs(const GameSpecification& spec);

    /**
     * Generate Blender job for specific asset
     */
    QJsonObject generateBlenderAssetJob(const GameAssetSpec& asset);

    /**
     * Generate Blender character creation job with rigging
     */
    QJsonObject generateCharacterCreationJob(const QString& characterName,
                                            const QString& description,
                                            const QStringList& animations);

    /**
     * Generate Blender environment job
     */
    QJsonObject generateEnvironmentCreationJob(const QString& envName,
                                              const QString& description,
                                              int sizeX, int sizeY, int sizeZ);

    /**
     * Generate game development jobs (Unreal Engine)
     */
    QJsonArray generateGameDevelopmentJobs(const GameSpecification& spec);

    /**
     * Generate level creation job
     */
    QJsonObject generateLevelCreationJob(const GameLevel& level,
                                        const GameSpecification& spec);

    /**
     * Generate complete game project setup job
     */
    QJsonObject generateGameProjectSetupJob(const GameSpecification& spec);

    /**
     * Generate game mechanics implementation job
     */
    QJsonObject generateMechanicsImplementationJob(const GameSpecification& spec);

    /**
     * Generate game compilation & build job
     */
    QJsonObject generateBuildJob(const GameSpecification& spec);

    /**
     * Generate complete game production workflow
     */
    QJsonArray generateCompleteWorkflow(const GameSpecification& spec);

    /**
     * Parse game description into specification
     */
    bool parseGameDescription(const QString& description, GameSpecification& outSpec);

    /**
     * Validate game specification
     */
    bool validateGameSpec(const GameSpecification& spec);

private:
    // Helper methods
    int estimateAssetCreationTime(const GameAssetSpec& asset);
    int estimateLevelCreationTime(const GameLevel& level);
    QString getMechanicsCode(GameMechanics mechanic);
    QString getAssetTypeFolder(GameAssetType type);
    
    // Template generation
    QJsonObject createBlenderTemplateJob();
    QJsonObject createUnrealTemplateJob();
};

#endif // GAMEPRODUCTIONWORKFLOW_H
