#include "Services/cinematicproductionworkflow.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QDateTime>
#include <QStringList>

CinematicProductionWorkflow::CinematicProductionWorkflow()
{
}

CinematicProductionWorkflow::~CinematicProductionWorkflow()
{
}

bool CinematicProductionWorkflow::createCinematicSpecification(const QString& title,
                                                              const QString& description,
                                                              CinematicStyle style,
                                                              CinematicSpecification& outSpec)
{
    outSpec.cinematicId = "CINEMA_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outSpec.title = title;
    outSpec.description = description;
    outSpec.style = style;
    
    // Parse description to determine structure
    if (!parseNarrativeDescription(description, outSpec)) {
        return false;
    }
    
    // Validate specification
    if (!validateCinematicSpec(outSpec)) {
        return false;
    }
    
    // Calculate estimated time
    outSpec.estimatedRenderMinutes = 0;
    for (const ProductionAct& act : outSpec.acts) {
        for (const CinematicShot& shot : act.shots) {
            outSpec.estimatedRenderMinutes += estimateShotRenderTime(shot);
        }
    }
    
    // Estimate post-production (roughly 30-50% of render time)
    outSpec.estimatedPostMinutes = static_cast<int>(outSpec.estimatedRenderMinutes * 0.4);
    
    return true;
}

bool CinematicProductionWorkflow::generateShotFromDescription(const QString& description,
                                                             int frameStart,
                                                             int frameEnd,
                                                             CinematicShot& outShot)
{
    outShot.id = "SHOT_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    outShot.name = "Shot";
    outShot.description = description;
    outShot.frameStart = frameStart;
    outShot.frameEnd = frameEnd;
    
    // Classify shot type
    if (description.toLower().contains("wide") || description.toLower().contains("establishing")) {
        outShot.shotType = ShotType::Wide;
    } else if (description.toLower().contains("close") || description.toLower().contains("closeup")) {
        outShot.shotType = ShotType::Close;
    } else if (description.toLower().contains("tracking") || description.toLower().contains("follow")) {
        outShot.shotType = ShotType::Tracking;
    } else if (description.toLower().contains("overhead") || description.toLower().contains("top-down")) {
        outShot.shotType = ShotType::Overhead;
    } else if (description.toLower().contains("pov") || description.toLower().contains("point of view")) {
        outShot.shotType = ShotType::POV;
    } else {
        outShot.shotType = ShotType::Medium;
    }
    
    // Determine camera movement
    if (description.toLower().contains("pan")) {
        outShot.cameraMovement = "pan";
    } else if (description.toLower().contains("dolly")) {
        outShot.cameraMovement = "dolly";
    } else if (description.toLower().contains("crane")) {
        outShot.cameraMovement = "crane";
    } else if (description.toLower().contains("tracking")) {
        outShot.cameraMovement = "tracking";
    } else {
        outShot.cameraMovement = "static";
    }
    
    // Determine lighting
    if (description.toLower().contains("dramatic")) {
        outShot.lighting = LightingType::Dramatic;
    } else if (description.toLower().contains("natural")) {
        outShot.lighting = LightingType::Natural;
    } else if (description.toLower().contains("bright")) {
        outShot.lighting = LightingType::HighKey;
    } else if (description.toLower().contains("dark")) {
        outShot.lighting = LightingType::LowKey;
    } else {
        outShot.lighting = LightingType::ThreePoint;
    }
    
    // Determine music tempo from mood
    if (description.toLower().contains("tense") || description.toLower().contains("fast")) {
        outShot.musicTempo = "fast";
    } else if (description.toLower().contains("slow") || description.toLower().contains("peaceful")) {
        outShot.musicTempo = "slow";
    } else {
        outShot.musicTempo = "moderate";
    }
    
    return true;
}

QJsonArray CinematicProductionWorkflow::generateBlenderRenderingJobs(const CinematicSpecification& spec)
{
    QJsonArray jobs;
    
    // Create setup jobs for each environment
    for (const QString& env : spec.requiredEnvironments) {
        QJsonObject envJob = generateEnvironmentSetupJob(env, "", spec.style);
        jobs.append(envJob);
    }
    
    // Create rendering jobs for each shot
    for (const ProductionAct& act : spec.acts) {
        for (const CinematicShot& shot : act.shots) {
            QJsonObject shotJob = generateShotRenderingJob(shot, spec);
            jobs.append(shotJob);
        }
    }
    
    return jobs;
}

QJsonObject CinematicProductionWorkflow::generateShotRenderingJob(const CinematicShot& shot,
                                                                  const CinematicSpecification& spec)
{
    QJsonObject job;
    
    job["job_id"] = "SHOT_" + shot.id;
    job["engine"] = "blender";
    job["job_type"] = "render_shot";
    job["description"] = "Render cinematic shot: " + shot.name;
    
    QJsonObject shotObj;
    shotObj["id"] = shot.id;
    shotObj["name"] = shot.name;
    shotObj["description"] = shot.description;
    shotObj["frame_start"] = shot.frameStart;
    shotObj["frame_end"] = shot.frameEnd;
    shotObj["shot_type"] = (int)shot.shotType;
    shotObj["camera_movement"] = shot.cameraMovement;
    shotObj["lighting"] = (int)shot.lighting;
    shotObj["environment"] = shot.environment;
    
    if (!shot.characters.isEmpty()) {
        shotObj["characters"] = QJsonArray::fromStringList(shot.characters);
    }
    
    job["shot"] = shotObj;
    
    // Render settings
    QJsonObject render;
    render["resolution_x"] = spec.resolutionX;
    render["resolution_y"] = spec.resolutionY;
    render["fps"] = spec.frameRate;
    render["samples"] = 512;  // High quality
    render["engine"] = "cycles";
    render["device"] = "gpu";
    render["denoise"] = true;
    render["output_format"] = "exr";
    job["render"] = render;
    
    // Camera setup
    QJsonObject camera;
    camera["type"] = getShotTypeCamera(shot.shotType);
    camera["lens"] = 50;
    job["camera"] = camera;
    
    // Lighting setup
    QJsonObject lighting;
    lighting["preset"] = getLightingPreset(shot.lighting);
    lighting["style"] = (int)spec.style;
    job["lighting"] = lighting;
    
    // Output
    QJsonObject output;
    output["path"] = "./output/cinematic_renders/" + shot.id;
    output["format"] = "exr_sequence";
    job["output"] = output;
    
    return job;
}

QJsonObject CinematicProductionWorkflow::generateEnvironmentSetupJob(const QString& environmentName,
                                                                    const QString& description,
                                                                    CinematicStyle style)
{
    QJsonObject job;
    
    job["job_id"] = "ENV_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    job["engine"] = "blender";
    job["job_type"] = "setup_environment";
    job["description"] = "Setup environment: " + environmentName;
    
    QJsonObject environment;
    environment["name"] = environmentName;
    environment["description"] = description;
    environment["style"] = (int)style;
    environment["optimize_viewport"] = false;  // Quality over performance
    
    job["environment"] = environment;
    
    return job;
}

QJsonObject CinematicProductionWorkflow::generateCharacterPositioningJob(const QStringList& characters,
                                                                        const CinematicShot& shot)
{
    QJsonObject job;
    
    job["job_id"] = "CHAR_POS_" + shot.id;
    job["engine"] = "blender";
    job["job_type"] = "position_characters";
    job["description"] = "Position characters for shot: " + shot.name;
    
    QJsonObject positioning;
    positioning["shot_id"] = shot.id;
    positioning["characters"] = QJsonArray::fromStringList(characters);
    positioning["camera_angle"] = getShotTypeCamera(shot.shotType);
    
    job["positioning"] = positioning;
    
    return job;
}

QJsonObject CinematicProductionWorkflow::generateCameraSetupJob(const CinematicShot& shot,
                                                               const CinematicSpecification& spec)
{
    QJsonObject job;
    
    job["job_id"] = "CAM_" + shot.id;
    job["engine"] = "blender";
    job["job_type"] = "setup_camera";
    job["description"] = "Setup camera for shot: " + shot.name;
    
    QJsonObject camera;
    camera["shot_type"] = (int)shot.shotType;
    camera["movement"] = shot.cameraMovement;
    camera["movement_script"] = getCameraMovementScript(shot.cameraMovement);
    camera["frame_start"] = shot.frameStart;
    camera["frame_end"] = shot.frameEnd;
    camera["lens"] = 50;
    
    job["camera"] = camera;
    
    return job;
}

QJsonObject CinematicProductionWorkflow::generateLightingSetupJob(const CinematicShot& shot,
                                                                 CinematicStyle style)
{
    QJsonObject job;
    
    job["job_id"] = "LIGHT_" + shot.id;
    job["engine"] = "blender";
    job["job_type"] = "setup_lighting";
    job["description"] = "Setup lighting for shot: " + shot.name;
    
    QJsonObject lighting;
    lighting["preset"] = getLightingPreset(shot.lighting);
    lighting["style"] = (int)style;
    lighting["intensity"] = 1.0;
    lighting["color_temperature"] = 5600;
    
    job["lighting"] = lighting;
    
    return job;
}

QJsonArray CinematicProductionWorkflow::generatePostProductionJobs(const CinematicSpecification& spec)
{
    QJsonArray jobs;
    
    // Color grading job
    if (spec.needsColorGrading) {
        QJsonObject colorJob = generateColorGradingJob(spec);
        jobs.append(colorJob);
    }
    
    // VFX job
    if (spec.needsVFX && !spec.vfxElements.isEmpty()) {
        QJsonObject vfxJob = generateVFXJob(spec);
        jobs.append(vfxJob);
    }
    
    // Sound design
    if (spec.needsSoundDesign) {
        QJsonObject soundJob = generateSoundDesignJob(spec);
        jobs.append(soundJob);
    }
    
    // Final export
    QJsonObject exportJob = generateFinalExportJob(spec);
    jobs.append(exportJob);
    
    return jobs;
}

QJsonObject CinematicProductionWorkflow::generateColorGradingJob(const CinematicSpecification& spec)
{
    QJsonObject job;
    
    job["job_id"] = "COLOR_" + spec.cinematicId;
    job["engine"] = "davinci";
    job["job_type"] = "color_grading";
    job["description"] = "Color grading for cinematic: " + spec.title;
    
    QJsonObject grading;
    grading["style"] = (int)spec.style;
    grading["lut"] = getColorGradingLUT(spec.style);
    grading["contrast"] = 1.1;
    grading["saturation"] = 1.15;
    grading["temperature"] = 5600;
    grading["highlights_lift"] = 0.1;
    grading["shadows_lift"] = 0.05;
    
    job["color_grading"] = grading;
    
    QJsonObject input;
    input["path"] = "./output/cinematic_renders";
    input["format"] = "exr_sequence";
    job["input"] = input;
    
    QJsonObject output;
    output["path"] = "./output/graded";
    output["format"] = "exr_sequence";
    job["output"] = output;
    
    return job;
}

QJsonObject CinematicProductionWorkflow::generateVFXJob(const CinematicSpecification& spec)
{
    QJsonObject job;
    
    job["job_id"] = "VFX_" + spec.cinematicId;
    job["engine"] = "davinci";
    job["job_type"] = "apply_effects";
    job["description"] = "Apply visual effects";
    
    QJsonObject effects;
    effects["elements"] = QJsonArray::fromStringList(spec.vfxElements);
    effects["motion_blur_strength"] = 0.3;
    effects["bloom_strength"] = 0.2;
    effects["color_aberration"] = 0.1;
    
    job["effects"] = effects;
    
    return job;
}

QJsonObject CinematicProductionWorkflow::generateSoundDesignJob(const CinematicSpecification& spec)
{
    QJsonObject job;
    
    job["job_id"] = "SOUND_" + spec.cinematicId;
    job["engine"] = "davinci";
    job["job_type"] = "sound_design";
    job["description"] = "Sound design and audio mixing";
    
    QJsonObject sound;
    sound["enable_music_sync"] = true;
    sound["enable_foley"] = true;
    sound["enable_dialogue"] = false;
    sound["target_loudness"] = -16;  // LUFS
    
    job["sound"] = sound;
    
    return job;
}

QJsonObject CinematicProductionWorkflow::generateFinalExportJob(const CinematicSpecification& spec)
{
    QJsonObject job;
    
    job["job_id"] = "EXPORT_" + spec.cinematicId;
    job["engine"] = "davinci";
    job["job_type"] = "export_video";
    job["description"] = "Export final cinematic video";
    
    QJsonObject export_settings;
    export_settings["format"] = "mp4";
    export_settings["codec"] = "h264";
    export_settings["bitrate"] = "25Mbps";
    export_settings["resolution_x"] = spec.resolutionX;
    export_settings["resolution_y"] = spec.resolutionY;
    export_settings["frame_rate"] = spec.frameRate;
    export_settings["color_space"] = "rec709";
    
    job["export"] = export_settings;
    
    QJsonObject output;
    output["path"] = "./output/final";
    output["format"] = "mp4";
    output["filename"] = spec.title.replace(" ", "_") + ".mp4";
    job["output"] = output;
    
    return job;
}

QJsonArray CinematicProductionWorkflow::generateCompleteWorkflow(const CinematicSpecification& spec)
{
    QJsonArray workflow;
    
    // Stage 1: Blender rendering
    QJsonArray blenderJobs = generateBlenderRenderingJobs(spec);
    for (const QJsonValue& job : blenderJobs) {
        workflow.append(job);
    }
    
    // Stage 2: Post-production (DaVinci)
    QJsonArray postJobs = generatePostProductionJobs(spec);
    for (const QJsonValue& job : postJobs) {
        workflow.append(job);
    }
    
    return workflow;
}

bool CinematicProductionWorkflow::parseNarrativeDescription(const QString& description,
                                                           CinematicSpecification& outSpec)
{
    // Create default act structure
    ProductionAct act;
    act.id = "ACT_1";
    act.name = "Opening";
    act.description = description;
    
    // Create a default shot
    CinematicShot shot;
    generateShotFromDescription(description, 1, outSpec.totalFrames, shot);
    
    act.shots.append(shot);
    act.estimatedFrames = outSpec.totalFrames;
    
    outSpec.acts.append(act);
    
    // Parse for required assets
    if (description.toLower().contains("character") || description.toLower().contains("person")) {
        outSpec.requiredCharacters.append("Main Character");
    }
    
    if (description.toLower().contains("environment") || description.toLower().contains("world") ||
        description.toLower().contains("scene")) {
        outSpec.requiredEnvironments.append("Main Environment");
    }
    
    // Infer color grading style from description
    if (description.toLower().contains("dark") || description.toLower().contains("noir")) {
        outSpec.colorGradingStyle = "noir";
    } else if (description.toLower().contains("warm")) {
        outSpec.colorGradingStyle = "warm";
    } else if (description.toLower().contains("cool") || description.toLower().contains("blue")) {
        outSpec.colorGradingStyle = "cool";
    } else {
        outSpec.colorGradingStyle = "neutral";
    }
    
    return true;
}

bool CinematicProductionWorkflow::validateCinematicSpec(const CinematicSpecification& spec)
{
    if (spec.title.isEmpty() || spec.description.isEmpty()) {
        return false;
    }
    
    if (spec.acts.isEmpty()) {
        return false;
    }
    
    int totalFrames = 0;
    for (const ProductionAct& act : spec.acts) {
        if (act.shots.isEmpty()) {
            return false;
        }
        totalFrames += act.estimatedFrames;
    }
    
    if (totalFrames <= 0) {
        return false;
    }
    
    return true;
}

QString CinematicProductionWorkflow::getLightingPreset(LightingType type)
{
    switch (type) {
        case LightingType::ThreePoint: return "three_point";
        case LightingType::HighKey: return "high_key";
        case LightingType::LowKey: return "low_key";
        case LightingType::Backlit: return "backlit";
        case LightingType::Rim: return "rim";
        case LightingType::Natural: return "natural";
        case LightingType::Dramatic: return "dramatic";
        default: return "three_point";
    }
}

QString CinematicProductionWorkflow::getCameraMovementScript(const QString& movement)
{
    if (movement == "pan") {
        return "camera.rotation_euler.z += 0.01";
    } else if (movement == "dolly") {
        return "camera.location.y -= 0.1";
    } else if (movement == "crane") {
        return "camera.location.z += 0.1";
    } else if (movement == "tracking") {
        return "camera.location.x += 0.1";
    }
    return "";
}

QString CinematicProductionWorkflow::getColorGradingLUT(CinematicStyle style)
{
    switch (style) {
        case CinematicStyle::SciFi: return "scifi_lut.cube";
        case CinematicStyle::Fantasy: return "fantasy_lut.cube";
        case CinematicStyle::Photorealistic: return "realistic_lut.cube";
        case CinematicStyle::Cartoon: return "cartoon_lut.cube";
        case CinematicStyle::Documentary: return "documentary_lut.cube";
        default: return "neutral_lut.cube";
    }
}

int CinematicProductionWorkflow::estimateShotRenderTime(const CinematicShot& shot)
{
    int frames = shot.frameEnd - shot.frameStart + 1;
    int minutesPerFrame = 5;  // Assume 5 minutes per frame at 256 samples
    
    return (frames * minutesPerFrame) / 60;  // Return in minutes
}

QString CinematicProductionWorkflow::getShotTypeCamera(ShotType type)
{
    switch (type) {
        case ShotType::Wide: return "wide_angle";
        case ShotType::Medium: return "standard";
        case ShotType::Close: return "telephoto";
        case ShotType::Overhead: return "overhead";
        case ShotType::POV: return "first_person";
        case ShotType::Tracking: return "tracking";
        case ShotType::Dolly: return "dolly";
        case ShotType::Crane: return "crane";
        default: return "standard";
    }
}
