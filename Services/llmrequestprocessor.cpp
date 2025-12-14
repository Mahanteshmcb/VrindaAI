#include "Services/llmrequestprocessor.h"
#include <QJsonDocument>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>
#include <QEventLoop>
#include <QTimer>

LLMRequestProcessor::LLMRequestProcessor(const QString& llmEndpoint)
    : m_llmEndpoint(llmEndpoint)
{
    m_networkManager = std::make_unique<QNetworkAccessManager>();
}

LLMRequestProcessor::~LLMRequestProcessor()
{
}

bool LLMRequestProcessor::processRequest(const QString& userRequest, ProcessingResult& outResult)
{
    emit processingStarted(userRequest);
    
    // Step 1: Classify the request
    outResult.type = classifyRequest(userRequest);
    
    // Step 2: Generate appropriate job based on type
    bool success = false;
    
    switch (outResult.type) {
        case RequestType::CreateScene:
            success = generateSceneFromDescription(userRequest, outResult.job);
            outResult.engine = "blender";
            break;
            
        case RequestType::CreateCharacter:
            success = generateCharacterJob(userRequest, {}, outResult.job);
            outResult.engine = "blender";
            break;
            
        case RequestType::CreateAnimation:
            success = generateAnimationJob("Generic Character", userRequest, outResult.job);
            outResult.engine = "blender";
            break;
            
        case RequestType::CreateGame:
            success = generateGameJob(userRequest, "Action", {}, outResult.job);
            outResult.engine = "unreal";
            break;
            
        case RequestType::CreateVideo:
            success = generateVideoJob(userRequest, "cinematic", 30, outResult.job);
            outResult.engine = "blender";
            break;
            
        case RequestType::CreateAssets:
            success = generateCharacterJob(userRequest, {}, outResult.job);
            outResult.engine = "blender";
            break;
            
        case RequestType::Composite:
            success = generateVideoJob(userRequest, "cinematic", 30, outResult.job);
            outResult.engine = "davinci";
            break;
            
        default:
            outResult.errorMessage = "Could not classify request type";
            emit processingFailed(outResult.errorMessage);
            return false;
    }
    
    outResult.isValid = success;
    outResult.description = userRequest;
    
    if (success) {
        emit jobGenerated(outResult.job);
    } else {
        emit processingFailed("Failed to generate job manifest");
    }
    
    return success;
}

bool LLMRequestProcessor::processMultiStageRequest(const QStringList& requests,
                                                   QJsonArray& outJobs)
{
    outJobs = QJsonArray();
    
    for (const QString& request : requests) {
        ProcessingResult result;
        if (!processRequest(request, result)) {
            return false;
        }
        outJobs.append(result.job);
    }
    
    return true;
}

LLMRequestProcessor::RequestType LLMRequestProcessor::classifyRequest(const QString& request)
{
    QString lower = request.toLower();
    
    if (lower.contains("scene") || lower.contains("environment") || lower.contains("render")) {
        return RequestType::CreateScene;
    }
    else if (lower.contains("character") && lower.contains("create")) {
        return RequestType::CreateCharacter;
    }
    else if (lower.contains("animate") || lower.contains("animation")) {
        return RequestType::CreateAnimation;
    }
    else if (lower.contains("game") || lower.contains("gameplay")) {
        return RequestType::CreateGame;
    }
    else if (lower.contains("video") || lower.contains("cinematic") || lower.contains("movie")) {
        return RequestType::CreateVideo;
    }
    else if (lower.contains("asset") && (lower.contains("generate") || lower.contains("create"))) {
        return RequestType::CreateAssets;
    }
    else if (lower.contains("composite") || lower.contains("edit") || lower.contains("color")) {
        return RequestType::Composite;
    }
    
    return RequestType::Unknown;
}

bool LLMRequestProcessor::generateSceneFromDescription(const QString& description,
                                                      QJsonObject& outJob)
{
    qDebug() << "[LLMProcessor] Generating Blender scene from:" << description;
    
    // Extract key information from description
    QJsonObject scene;
    scene["name"] = "Scene";
    scene["description"] = description;
    scene["type"] = "environment";
    
    // Parse for style/theme
    QString style = "realistic";
    if (description.toLower().contains("sci-fi")) style = "sci-fi";
    if (description.toLower().contains("fantasy")) style = "fantasy";
    if (description.toLower().contains("cartoon")) style = "cartoon";
    scene["style"] = style;
    
    // Default render settings
    QJsonObject render;
    render["resolution_x"] = 1920;
    render["resolution_y"] = 1080;
    render["samples"] = 256;
    render["engine"] = "cycles";
    render["device"] = "gpu";
    scene["render"] = render;
    
    // Build job manifest
    outJob["job_id"] = "SCENE_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outJob["engine"] = "blender";
    outJob["job_type"] = "render_scene";
    outJob["description"] = description;
    outJob["scene"] = scene;
    
    QJsonObject output;
    output["path"] = "./output/renders";
    output["format"] = "exr_sequence";
    outJob["output"] = output;
    
    return true;
}

bool LLMRequestProcessor::generateCharacterJob(const QString& description,
                                              const QStringList& traits,
                                              QJsonObject& outJob)
{
    qDebug() << "[LLMProcessor] Generating character from:" << description;
    
    QJsonObject character;
    character["name"] = "Character";
    character["description"] = description;
    
    // Parse traits
    if (!traits.isEmpty()) {
        character["traits"] = QJsonArray::fromStringList(traits);
    }
    
    // Extract character type from description
    QString type = "human";
    if (description.toLower().contains("robot") || description.toLower().contains("android")) {
        type = "robot";
    }
    if (description.toLower().contains("alien")) {
        type = "alien";
    }
    if (description.toLower().contains("creature") || description.toLower().contains("animal")) {
        type = "creature";
    }
    character["type"] = type;
    
    outJob["job_id"] = "CHAR_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outJob["engine"] = "blender";
    outJob["job_type"] = "create_character";
    outJob["description"] = description;
    outJob["character"] = character;
    
    QJsonObject output;
    output["path"] = "./output/assets";
    output["format"] = "fbx";
    outJob["output"] = output;
    
    return true;
}

bool LLMRequestProcessor::generateAnimationJob(const QString& characterDesc,
                                              const QString& animationDesc,
                                              QJsonObject& outJob)
{
    qDebug() << "[LLMProcessor] Generating animation:" << animationDesc;
    
    QJsonObject animation;
    animation["character"] = characterDesc;
    animation["description"] = animationDesc;
    
    // Classify animation type
    QString animType = "idle";
    if (animationDesc.toLower().contains("walk")) animType = "walk";
    if (animationDesc.toLower().contains("run")) animType = "run";
    if (animationDesc.toLower().contains("jump")) animType = "jump";
    if (animationDesc.toLower().contains("attack")) animType = "attack";
    if (animationDesc.toLower().contains("dance")) animType = "dance";
    animation["type"] = animType;
    
    animation["frame_start"] = 1;
    animation["frame_end"] = 120;  // 5 seconds at 24 fps
    
    outJob["job_id"] = "ANIM_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outJob["engine"] = "blender";
    outJob["job_type"] = "create_animation";
    outJob["description"] = animationDesc;
    outJob["animation"] = animation;
    
    QJsonObject output;
    output["path"] = "./output/animations";
    output["format"] = "fbx";
    outJob["output"] = output;
    
    return true;
}

bool LLMRequestProcessor::generateGameJob(const QString& gameDescription,
                                         const QString& gameGenre,
                                         const QStringList& mechanics,
                                         QJsonObject& outJob)
{
    qDebug() << "[LLMProcessor] Generating game project from:" << gameDescription;
    
    QJsonObject game;
    game["name"] = "Game";
    game["description"] = gameDescription;
    
    // Determine genre from description if not provided
    QString genre = gameGenre;
    if (genre.isEmpty()) {
        if (gameDescription.toLower().contains("fps")) genre = "FPS";
        if (gameDescription.toLower().contains("rpg")) genre = "RPG";
        if (gameDescription.toLower().contains("strategy")) genre = "Strategy";
        if (gameDescription.toLower().contains("puzzle")) genre = "Puzzle";
        if (genre.isEmpty()) genre = "Action";
    }
    game["genre"] = genre;
    
    // Extract mechanics
    QJsonArray gameM;
    if (!mechanics.isEmpty()) {
        gameM = QJsonArray::fromStringList(mechanics);
    } else {
        // Infer from description
        if (gameDescription.toLower().contains("combat") || gameDescription.toLower().contains("fight")) {
            gameM.append("combat");
        }
        if (gameDescription.toLower().contains("explore")) {
            gameM.append("exploration");
        }
        if (gameDescription.toLower().contains("puzzle")) {
            gameM.append("puzzle_solving");
        }
        if (gameDescription.toLower().contains("level")) {
            gameM.append("level_progression");
        }
    }
    game["mechanics"] = gameM;
    
    outJob["job_id"] = "GAME_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outJob["engine"] = "unreal";
    outJob["job_type"] = "create_game";
    outJob["description"] = gameDescription;
    outJob["game"] = game;
    
    QJsonObject project;
    project["engine_version"] = "5.3";
    project["target_platform"] = "Windows";
    outJob["project"] = project;
    
    QJsonObject output;
    output["path"] = "./output/game";
    output["format"] = "executable";
    outJob["output"] = output;
    
    return true;
}

bool LLMRequestProcessor::generateVideoJob(const QString& sceneDescription,
                                          const QString& style,
                                          int durationSeconds,
                                          QJsonObject& outJob)
{
    qDebug() << "[LLMProcessor] Generating video from:" << sceneDescription;
    
    QJsonObject video;
    video["name"] = "Cinematic";
    video["description"] = sceneDescription;
    video["style"] = style;
    video["duration_seconds"] = durationSeconds;
    video["frame_rate"] = 24;
    video["resolution_x"] = 1920;
    video["resolution_y"] = 1080;
    
    outJob["job_id"] = "VIDEO_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outJob["engine"] = "blender";
    outJob["job_type"] = "render_video";
    outJob["description"] = sceneDescription;
    outJob["video"] = video;
    
    QJsonObject output;
    output["path"] = "./output/video";
    output["format"] = "mp4";
    outJob["output"] = output;
    
    return true;
}

bool LLMRequestProcessor::queryLLM(const QString& prompt, QString& outResponse)
{
    // Placeholder for actual LLM communication
    // In production, would make HTTP request to llama.cpp server
    qDebug() << "[LLMProcessor] Would query LLM with prompt:" << prompt;
    outResponse = "Generated response";
    return true;
}

bool LLMRequestProcessor::parseJobFromLLMResponse(const QString& llmResponse, QJsonObject& outJob)
{
    // Placeholder for parsing LLM response into job manifest
    qDebug() << "[LLMProcessor] Parsing LLM response:" << llmResponse;
    return true;
}

QString LLMRequestProcessor::buildPromptForSceneGeneration(const QString& description)
{
    return QString(
        "You are a professional 3D scene designer. Create detailed Blender scene specifications for:\n%1\n"
        "Provide: materials, lighting, camera position, objects, and render settings in JSON format."
    ).arg(description);
}

QString LLMRequestProcessor::buildPromptForCharacterGeneration(const QString& description,
                                                              const QStringList& traits)
{
    QString traitStr = traits.isEmpty() ? "" : "Traits: " + traits.join(", ");
    return QString(
        "You are a professional character designer. Create detailed character specifications for:\n%1\n%2\n"
        "Provide: anatomy, materials, clothing, rigging specifications in JSON format."
    ).arg(description, traitStr);
}

QString LLMRequestProcessor::buildPromptForAnimationGeneration(const QString& character,
                                                              const QString& animation)
{
    return QString(
        "You are a professional animator. Create animation keyframe specifications for:\n"
        "Character: %1\n"
        "Animation: %2\n"
        "Provide: keyframe positions, timing, easing functions in JSON format."
    ).arg(character, animation);
}

QString LLMRequestProcessor::buildPromptForGameGeneration(const QString& description,
                                                         const QString& genre,
                                                         const QStringList& mechanics)
{
    QString mechStr = mechanics.isEmpty() ? "" : "Mechanics: " + mechanics.join(", ");
    return QString(
        "You are a professional game designer. Create game specifications for:\n%1\n"
        "Genre: %2\n%3\n"
        "Provide: level design, gameplay systems, player progression in JSON format."
    ).arg(description, genre, mechStr);
}

QString LLMRequestProcessor::buildPromptForVideoGeneration(const QString& description,
                                                          const QString& style,
                                                          int duration)
{
    return QString(
        "You are a professional cinematographer. Create cinematic video specifications for:\n%1\n"
        "Style: %2\n"
        "Duration: %3 seconds\n"
        "Provide: shot composition, camera movement, lighting, effects in JSON format."
    ).arg(description, style, QString::number(duration));
}

QJsonObject LLMRequestProcessor::createBlenderJobFromLLM(const QString& response)
{
    QJsonObject job;
    // Parse response and create Blender job
    return job;
}

QJsonObject LLMRequestProcessor::createUnrealJobFromLLM(const QString& response)
{
    QJsonObject job;
    // Parse response and create Unreal job
    return job;
}

QJsonObject LLMRequestProcessor::createDaVinciJobFromLLM(const QString& response)
{
    QJsonObject job;
    // Parse response and create DaVinci job
    return job;
}
