#include "Services/gameproductionworkflow.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QDateTime>

GameProductionWorkflow::GameProductionWorkflow()
{
}

GameProductionWorkflow::~GameProductionWorkflow()
{
}

bool GameProductionWorkflow::createGameSpecification(const QString& gameName,
                                                    const QString& description,
                                                    const QString& genre,
                                                    GameSpecification& outSpec)
{
    outSpec.gameId = "GAME_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outSpec.gameName = gameName;
    outSpec.description = description;
    outSpec.genre = genre;
    
    // Parse description to determine required assets and mechanics
    if (!parseGameDescription(description, outSpec)) {
        return false;
    }
    
    // Validate specification
    if (!validateGameSpec(outSpec)) {
        return false;
    }
    
    // Calculate estimated time
    outSpec.estimatedTotalMinutes = 0;
    for (const GameAssetSpec& asset : outSpec.requiredAssets) {
        outSpec.estimatedTotalMinutes += estimateAssetCreationTime(asset);
    }
    for (const GameLevel& level : outSpec.levels) {
        outSpec.estimatedTotalMinutes += estimateLevelCreationTime(level);
    }
    // Add compilation time
    outSpec.estimatedTotalMinutes += 60;
    
    return true;
}

QJsonArray GameProductionWorkflow::generateAssetCreationJobs(const GameSpecification& spec)
{
    QJsonArray jobs;
    
    // Generate Blender job for each asset
    for (const GameAssetSpec& asset : spec.requiredAssets) {
        QJsonObject job = generateBlenderAssetJob(asset);
        if (!job.isEmpty()) {
            jobs.append(job);
        }
    }
    
    return jobs;
}

QJsonObject GameProductionWorkflow::generateBlenderAssetJob(const GameAssetSpec& asset)
{
    QJsonObject job;
    
    job["job_id"] = "ASSET_" + asset.id;
    job["engine"] = "blender";
    job["job_type"] = "create_asset";
    job["description"] = "Create asset: " + asset.name;
    
    QJsonObject assetObj;
    assetObj["id"] = asset.id;
    assetObj["name"] = asset.name;
    assetObj["type"] = (int)asset.type;
    assetObj["description"] = asset.description;
    assetObj["poly_target"] = asset.polyCount;
    assetObj["animated"] = asset.isAnimated;
    
    if (asset.isAnimated && !asset.requiredAnimations.isEmpty()) {
        assetObj["animations"] = QJsonArray::fromStringList(asset.requiredAnimations);
    }
    
    job["asset"] = assetObj;
    
    QJsonObject output;
    output["format"] = "fbx";
    output["path"] = "./output/game_assets/" + getAssetTypeFolder(asset.type);
    job["output"] = output;
    
    return job;
}

QJsonObject GameProductionWorkflow::generateCharacterCreationJob(const QString& characterName,
                                                                const QString& description,
                                                                const QStringList& animations)
{
    QJsonObject job;
    
    job["job_id"] = "CHAR_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    job["engine"] = "blender";
    job["job_type"] = "create_character";
    job["description"] = "Create game character: " + characterName;
    
    QJsonObject character;
    character["name"] = characterName;
    character["description"] = description;
    character["with_rig"] = true;
    character["with_materials"] = true;
    
    if (!animations.isEmpty()) {
        character["animations"] = QJsonArray::fromStringList(animations);
    }
    
    job["character"] = character;
    
    QJsonObject output;
    output["format"] = "fbx";
    output["path"] = "./output/game_assets/characters";
    job["output"] = output;
    
    return job;
}

QJsonObject GameProductionWorkflow::generateEnvironmentCreationJob(const QString& envName,
                                                                  const QString& description,
                                                                  int sizeX, int sizeY, int sizeZ)
{
    QJsonObject job;
    
    job["job_id"] = "ENV_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    job["engine"] = "blender";
    job["job_type"] = "create_environment";
    job["description"] = "Create game environment: " + envName;
    
    QJsonObject environment;
    environment["name"] = envName;
    environment["description"] = description;
    environment["size_x"] = sizeX;
    environment["size_y"] = sizeY;
    environment["size_z"] = sizeZ;
    environment["optimize_for_realtime"] = true;
    environment["generate_collision"] = true;
    
    job["environment"] = environment;
    
    QJsonObject output;
    output["format"] = "fbx";
    output["path"] = "./output/game_assets/environments";
    job["output"] = output;
    
    return job;
}

QJsonArray GameProductionWorkflow::generateGameDevelopmentJobs(const GameSpecification& spec)
{
    QJsonArray jobs;
    
    // Job 1: Project Setup
    QJsonObject setupJob = generateGameProjectSetupJob(spec);
    jobs.append(setupJob);
    
    // Job 2: Import Assets
    QJsonObject importJob;
    importJob["job_id"] = "IMPORT_" + spec.gameId;
    importJob["engine"] = "unreal";
    importJob["job_type"] = "import_assets";
    importJob["description"] = "Import game assets from Blender into Unreal";
    QJsonObject imports;
    imports["asset_source_path"] = "./output/game_assets";
    imports["import_characters"] = true;
    imports["import_environments"] = true;
    imports["auto_create_materials"] = true;
    importJob["imports"] = imports;
    jobs.append(importJob);
    
    // Job 3: Implement Mechanics
    QJsonObject mechJob = generateMechanicsImplementationJob(spec);
    jobs.append(mechJob);
    
    // Job 4: Create Levels
    for (const GameLevel& level : spec.levels) {
        QJsonObject levelJob = generateLevelCreationJob(level, spec);
        jobs.append(levelJob);
    }
    
    // Job 5: Build Game
    QJsonObject buildJob = generateBuildJob(spec);
    jobs.append(buildJob);
    
    return jobs;
}

QJsonObject GameProductionWorkflow::generateLevelCreationJob(const GameLevel& level,
                                                           const GameSpecification& spec)
{
    QJsonObject job;
    
    job["job_id"] = "LEVEL_" + level.id;
    job["engine"] = "unreal";
    job["job_type"] = "create_level";
    job["description"] = "Create level: " + level.name;
    
    QJsonObject levelObj;
    levelObj["id"] = level.id;
    levelObj["name"] = level.name;
    levelObj["description"] = level.description;
    levelObj["size_x"] = level.sizeX;
    levelObj["size_y"] = level.sizeY;
    levelObj["size_z"] = level.sizeZ;
    
    if (!level.requiredAssets.isEmpty()) {
        levelObj["assets"] = QJsonArray::fromStringList(level.requiredAssets);
    }
    
    if (!level.requiredMechanics.isEmpty()) {
        levelObj["mechanics"] = QJsonArray::fromStringList(level.requiredMechanics);
    }
    
    if (!level.environmentAsset.isEmpty()) {
        levelObj["environment"] = level.environmentAsset;
    }
    
    job["level"] = levelObj;
    
    QJsonObject output;
    output["format"] = "umap";
    output["path"] = "./output/game/Content/Levels";
    job["output"] = output;
    
    return job;
}

QJsonObject GameProductionWorkflow::generateGameProjectSetupJob(const GameSpecification& spec)
{
    QJsonObject job;
    
    job["job_id"] = "SETUP_" + spec.gameId;
    job["engine"] = "unreal";
    job["job_type"] = "project_setup";
    job["description"] = "Setup Unreal game project";
    
    QJsonObject project;
    project["name"] = spec.gameName;
    project["description"] = spec.description;
    project["engine_version"] = "5.3";
    project["target_platform"] = spec.targetPlatform;
    project["target_fps"] = spec.targetFPS;
    project["genre"] = spec.genre;
    
    QJsonArray mechanics;
    for (GameMechanics mech : spec.mechanics) {
        mechanics.append(getMechanicsCode(mech));
    }
    project["mechanics"] = mechanics;
    
    job["project"] = project;
    
    QJsonObject output;
    output["path"] = "./output/game";
    job["output"] = output;
    
    return job;
}

QJsonObject GameProductionWorkflow::generateMechanicsImplementationJob(const GameSpecification& spec)
{
    QJsonObject job;
    
    job["job_id"] = "MECH_" + spec.gameId;
    job["engine"] = "unreal";
    job["job_type"] = "implement_mechanics";
    job["description"] = "Implement game mechanics";
    
    QJsonObject mechanics;
    for (GameMechanics mech : spec.mechanics) {
        mechanics[getMechanicsCode(mech)] = true;
    }
    
    job["mechanics"] = mechanics;
    
    return job;
}

QJsonObject GameProductionWorkflow::generateBuildJob(const GameSpecification& spec)
{
    QJsonObject job;
    
    job["job_id"] = "BUILD_" + spec.gameId;
    job["engine"] = "unreal";
    job["job_type"] = "build_game";
    job["description"] = "Build and package game";
    
    QJsonObject build;
    build["target"] = "Game";
    build["configuration"] = "Shipping";
    build["platform"] = spec.targetPlatform;
    build["package"] = true;
    
    job["build"] = build;
    
    QJsonObject output;
    output["format"] = "executable";
    output["path"] = "./output/game/Binaries";
    job["output"] = output;
    
    return job;
}

QJsonArray GameProductionWorkflow::generateCompleteWorkflow(const GameSpecification& spec)
{
    QJsonArray workflow;
    
    // Stage 1: Asset creation
    QJsonArray assetJobs = generateAssetCreationJobs(spec);
    for (const QJsonValue& job : assetJobs) {
        workflow.append(job);
    }
    
    // Stage 2: Game development
    QJsonArray gameJobs = generateGameDevelopmentJobs(spec);
    for (const QJsonValue& job : gameJobs) {
        workflow.append(job);
    }
    
    return workflow;
}

bool GameProductionWorkflow::parseGameDescription(const QString& description,
                                                 GameSpecification& outSpec)
{
    // Default assets and mechanics based on description
    
    // Check for characters
    if (description.toLower().contains("character") || description.toLower().contains("player")) {
        GameAssetSpec character;
        character.id = "main_character";
        character.type = GameAssetType::Character;
        character.name = "Main Character";
        character.description = "Primary player character";
        character.isAnimated = true;
        character.requiredAnimations = {"idle", "walk", "run", "jump"};
        outSpec.requiredAssets.append(character);
    }
    
    // Check for environments
    if (description.toLower().contains("world") || description.toLower().contains("level")) {
        GameAssetSpec env;
        env.id = "main_environment";
        env.type = GameAssetType::Environment;
        env.name = "Main Environment";
        env.description = "Primary game world";
        outSpec.requiredAssets.append(env);
    }
    
    // Check for level count
    if (description.toLower().contains("single level")) {
        GameLevel level;
        level.id = "level_1";
        level.name = "Level 1";
        level.description = description;
        outSpec.levels.append(level);
    } else if (description.toLower().contains("multi-level") || description.toLower().contains("multiple levels")) {
        for (int i = 1; i <= 5; ++i) {
            GameLevel level;
            level.id = "level_" + QString::number(i);
            level.name = "Level " + QString::number(i);
            level.description = "Game level " + QString::number(i);
            outSpec.levels.append(level);
        }
    } else {
        GameLevel level;
        level.id = "level_1";
        level.name = "Main Level";
        level.description = description;
        outSpec.levels.append(level);
    }
    
    // Infer mechanics from description
    if (description.toLower().contains("combat") || description.toLower().contains("fight")) {
        outSpec.mechanics.append(GameMechanics::Combat);
    }
    if (description.toLower().contains("inventory")) {
        outSpec.mechanics.append(GameMechanics::Inventory);
    }
    if (description.toLower().contains("puzzle")) {
        outSpec.mechanics.append(GameMechanics::Puzzle);
    }
    if (description.toLower().contains("shoot")) {
        outSpec.mechanics.append(GameMechanics::Shooting);
    }
    if (description.toLower().contains("platform")) {
        outSpec.mechanics.append(GameMechanics::Platforming);
    }
    if (description.toLower().contains("dialogue")) {
        outSpec.mechanics.append(GameMechanics::Dialogue);
    }
    if (outSpec.mechanics.isEmpty()) {
        outSpec.mechanics.append(GameMechanics::Movement);
    }
    
    return true;
}

bool GameProductionWorkflow::validateGameSpec(const GameSpecification& spec)
{
    if (spec.gameName.isEmpty() || spec.description.isEmpty()) {
        return false;
    }
    
    if (spec.levels.isEmpty()) {
        return false;
    }
    
    if (spec.mechanics.isEmpty()) {
        return false;
    }
    
    return true;
}

int GameProductionWorkflow::estimateAssetCreationTime(const GameAssetSpec& asset)
{
    int baseTime = 30;  // Base 30 minutes per asset
    
    // Add time based on complexity
    if (asset.isAnimated) {
        baseTime += 30;
    }
    
    if (asset.polyCount > 100000) {
        baseTime += 30;
    }
    
    return baseTime;
}

int GameProductionWorkflow::estimateLevelCreationTime(const GameLevel& level)
{
    int baseTime = 60;  // Base 60 minutes per level
    
    // Add time based on assets needed
    baseTime += level.requiredAssets.count() * 20;
    
    // Add time based on mechanics
    baseTime += level.requiredMechanics.count() * 15;
    
    return baseTime;
}

QString GameProductionWorkflow::getMechanicsCode(GameMechanics mechanic)
{
    switch (mechanic) {
        case GameMechanics::Movement: return "movement";
        case GameMechanics::Combat: return "combat";
        case GameMechanics::Inventory: return "inventory";
        case GameMechanics::Dialogue: return "dialogue";
        case GameMechanics::Puzzle: return "puzzle";
        case GameMechanics::Shooting: return "shooting";
        case GameMechanics::Platforming: return "platforming";
        case GameMechanics::RTS: return "rts";
        case GameMechanics::Stealth: return "stealth";
        default: return "unknown";
    }
}

QString GameProductionWorkflow::getAssetTypeFolder(GameAssetType type)
{
    switch (type) {
        case GameAssetType::Character: return "characters";
        case GameAssetType::Environment: return "environments";
        case GameAssetType::Prop: return "props";
        case GameAssetType::VFX: return "vfx";
        case GameAssetType::Audio: return "audio";
        case GameAssetType::UI: return "ui";
        case GameAssetType::Animation: return "animations";
        default: return "assets";
    }
}

QJsonObject GameProductionWorkflow::createBlenderTemplateJob()
{
    QJsonObject job;
    // Template for Blender job
    return job;
}

QJsonObject GameProductionWorkflow::createUnrealTemplateJob()
{
    QJsonObject job;
    // Template for Unreal job
    return job;
}
