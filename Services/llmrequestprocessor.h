#ifndef LLMREQUESTPROCESSOR_H
#define LLMREQUESTPROCESSOR_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <memory>

/**
 * @class LLMRequestProcessor
 * @brief Converts natural language requests to structured job manifests
 * 
 * This bridges natural language understanding with structured automation:
 * - "Create a sci-fi space scene" → Blender job manifest
 * - "Build a first-person game" → Unreal job manifest
 * - "Make a cinematic video" → Complete workflow with Blender + DaVinci
 */
class LLMRequestProcessor : public QObject {
    Q_OBJECT

public:
    enum class RequestType {
        CreateScene,        // "Create a sci-fi city scene"
        CreateCharacter,    // "Create an animated character"
        CreateAnimation,    // "Animate a character walking"
        CreateGame,         // "Build a game with these mechanics"
        CreateVideo,        // "Make a cinematic video of..."
        CreateAssets,       // "Generate game assets: characters, items, etc"
        Composite,          // "Combine video clips and add effects"
        Unknown
    };

    struct ProcessingResult {
        RequestType type;
        QJsonObject job;
        QString engine;  // "blender", "unreal", "davinci"
        QString description;
        bool isValid = false;
        QString errorMessage;
    };

    LLMRequestProcessor(const QString& llmEndpoint = "http://localhost:8080");
    ~LLMRequestProcessor();

    /**
     * Process natural language request and generate job manifest
     */
    bool processRequest(const QString& userRequest, ProcessingResult& outResult);

    /**
     * Process multiple requests in sequence (for complex projects)
     */
    bool processMultiStageRequest(const QStringList& requests, 
                                 QJsonArray& outJobs);

    /**
     * Classify request type
     */
    static RequestType classifyRequest(const QString& request);

    /**
     * Generate scene from text description
     */
    bool generateSceneFromDescription(const QString& description, 
                                     QJsonObject& outJob);

    /**
     * Generate character with specific traits
     */
    bool generateCharacterJob(const QString& description,
                             const QStringList& traits,
                             QJsonObject& outJob);

    /**
     * Generate animation specification
     */
    bool generateAnimationJob(const QString& characterDesc,
                             const QString& animationDesc,
                             QJsonObject& outJob);

    /**
     * Generate game specification
     */
    bool generateGameJob(const QString& gameDescription,
                        const QString& gameGenre,
                        const QStringList& mechanics,
                        QJsonObject& outJob);

    /**
     * Generate video/cinematic specification
     */
    bool generateVideoJob(const QString& sceneDescription,
                         const QString& style,
                         int durationSeconds,
                         QJsonObject& outJob);

    /**
     * Get LLM interpretation of request
     */
    bool queryLLM(const QString& prompt, QString& outResponse);

    /**
     * Parse LLM response into structured data
     */
    bool parseJobFromLLMResponse(const QString& llmResponse, QJsonObject& outJob);

signals:
    void processingStarted(const QString& request);
    void jobGenerated(const QJsonObject& job);
    void processingFailed(const QString& error);

private:
    QString m_llmEndpoint;
    std::unique_ptr<QNetworkAccessManager> m_networkManager;

    // Helper methods
    QString buildPromptForSceneGeneration(const QString& description);
    QString buildPromptForCharacterGeneration(const QString& description, 
                                            const QStringList& traits);
    QString buildPromptForAnimationGeneration(const QString& character,
                                            const QString& animation);
    QString buildPromptForGameGeneration(const QString& description,
                                        const QString& genre,
                                        const QStringList& mechanics);
    QString buildPromptForVideoGeneration(const QString& description,
                                         const QString& style,
                                         int duration);

    // Post-processing
    QJsonObject createBlenderJobFromLLM(const QString& response);
    QJsonObject createUnrealJobFromLLM(const QString& response);
    QJsonObject createDaVinciJobFromLLM(const QString& response);
};

#endif // LLMREQUESTPROCESSOR_H
