#include "modelmanager.h"
#include "Controllers/llamaservercontroller.h" // <<< Must be included
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QVariantMap>
#include <QDir>
#include <QQueue>
#include <functional>

// NOTE: m_currentModelLoaded is unused in this parallel architecture and should be removed from modelmanager.h

ModelManager::ModelManager(QObject *parent)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &ModelManager::onCompletionReplyFinished);

    // --- Configuration for Model Selection ---
    m_baseModelPath = ""; // Set to empty as servers run in model dir

    // Set the default model for any agent not specified below
    m_defaultModel = "mistral.gguf";
    // --- 1. MODEL ASSIGNMENT (Maps Role -> Model) ---
    m_roleToModelMap["Vrinda"] = "Phimini.gguf";
    m_roleToModelMap["Researcher"] = "Phimini.gguf";
    m_roleToModelMap["Assistant"] = "Phimini.gguf";

    m_roleToModelMap["Manager"] = "qwencoder.gguf";
    m_roleToModelMap["Coder"] = "qwencoder.gguf";
    m_roleToModelMap["Scripter"] = "qwencoder.gguf";
    m_roleToModelMap["Integrator"] = "qwencoder.gguf";
    m_roleToModelMap["Validator"] = "qwencoder.gguf";
    m_roleToModelMap["Corrector"] = "qwencoder.gguf";

    m_roleToModelMap["Texturing"] = "llava.gguf -- mmproj-model-f16.gguf";
    m_roleToModelMap["Designer"] = "llava.gguf -- mmproj-model-f16.gguf";

    m_roleToModelMap["Planner"] = "mistral.gguf";
    m_roleToModelMap["Architect"] = "mistral.gguf";
    m_roleToModelMap["Engine"] = "mistral.gguf";
    m_roleToModelMap["Editing"] = "mistral.gguf";
    //m_roleToModelMap["Modeling"] = "stable-diffusion-3d.gguf";
    //m_roleToModelMap["Animation"] = "motion-generator.gguf";
    //m_roleToModelMap["Compiler"] = "mistral.gguf";

    // --- 2. MODEL-TO-PORT MAPPING (Maps Model -> Port) ---
    // Ensure the models listed here match the models you start on the specified port.
    m_modelToPortMap["Phimini.gguf"] = 8080;
    m_modelToPortMap["qwencoder.gguf"] = 8081;
    m_modelToPortMap["llava.gguf -- mmproj-model-f16.gguf"] = 8082;
    m_modelToPortMap["mistral.gguf"] = 8083;

    //m_roleToModelMap.clear();

}

// Implementation for setting the controller pointer
void ModelManager::setServerController(LlamaServerController *controller)
{
    m_serverController = controller;
}


// --- Internal method used AFTER the server is verified ready ---
void ModelManager::sendNetworkRequest(const QString &taskId, const QString &role, const QString &prompt, const QString &modelName, int port)
{
    QString urlString = QString("http://127.0.0.1:%1/v1/chat/completions").arg(port);
    QNetworkRequest request(urlString);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject payload;
    payload["model"] = modelName;

    QJsonArray messages;
    messages.append(QJsonObject{{"role", "system"}, {"content", "You are the " + role + " agent."}});
    messages.append(QJsonObject{{"role", "user"}, {"content", prompt}});
    payload["messages"] = messages;
    payload["temperature"] = 0.7;
    payload["max_tokens"] = 8192;

    QVariantMap requestData;
    requestData["role"] = role;
    requestData["taskId"] = taskId;
    requestData["modelUsed"] = modelName;
    request.setAttribute(QNetworkRequest::User, requestData);

    QByteArray jsonData = QJsonDocument(payload).toJson();
    m_networkManager->post(request, jsonData);
}


// --- THE CORE SWAPPING LOGIC: SCHEDULER ---
void ModelManager::sendRequest(const QString &taskId, const QString &role, const QString &prompt)
{
    if (!m_serverController) {
        emit requestFailed(role, "Server controller not initialized. Cannot run models.");
        return;
    }

    QString requiredModel = getModelForRole(role);
    int requiredPort = m_modelToPortMap.value(requiredModel, 8080);

    // 1. Check if required model is already running
    if (m_activeModelName == requiredModel) {
        // CASE 1: Model is already loaded. Dispatch immediately.
        sendNetworkRequest(taskId, role, prompt, requiredModel, requiredPort);
        return;
    }

    // 2. SWAP REQUIRED: Queue the request and manage servers

    // A. Queue the request first
    m_pendingRequests.enqueue({taskId, role, prompt, requiredModel, requiredPort});

    // If a swap operation is already in progress, just queue and wait.
    if (m_activeModelPort != 0 && m_activeModelName != "") {
        qDebug() << "SCHEDULER: Model swap already in progress (" << m_activeModelName << "). Queued task for" << requiredModel;
        return;
    }

    // B. Stop the currently active server (if one exists)
    if (m_activeModelPort != 0 && m_serverController) {
        m_serverController->stopServer(m_activeModelPort);
    }

    // C. Update the active state
    m_activeModelName = requiredModel;
    m_activeModelPort = requiredPort;

    // D. Launch the new server
    m_serverController->startServer(requiredModel, requiredPort);

    // E. Start Polling/Health Check
    qDebug() << "SCHEDULER: Initiating health check for new model:" << requiredModel;

    // This signal tells MainWindow's health poller to start checking the new port.
    emit startHealthCheck(requiredPort, [this]() {
        this->processPendingRequestQueue();
    });
}

// New slot to execute all waiting requests once the server is ready
void ModelManager::processPendingRequestQueue()
{
    qDebug() << "SCHEDULER: Server " << m_activeModelPort << " is ready. Processing queue...";

    // Only process requests that belong to the currently active model.
    QQueue<PendingRequest> failedQueue;

    while (!m_pendingRequests.isEmpty()) {
        PendingRequest req = m_pendingRequests.dequeue();

        // If the request requires the model that just loaded, dispatch it
        if (req.model == m_activeModelName) {
            sendNetworkRequest(req.taskId, req.role, req.prompt, req.model, req.port);
        } else {
            // If the active model has changed while waiting, push the request back to the failed queue
            failedQueue.enqueue(req);
        }
    }

    // Put back any tasks that were for a different model (or were skipped)
    m_pendingRequests = failedQueue;
}

// The following function handles the network response directly
void ModelManager::onCompletionReplyFinished(QNetworkReply *reply)
{
    // Implementation remains the same as your previous onCompletionReplyFinished
    QVariantMap requestData = reply->request().attribute(QNetworkRequest::User).toMap();
    const QString role = requestData["role"].toString();
    const QString taskId = requestData["taskId"].toString();
    const QString modelUsed = requestData["modelUsed"].toString();

    if (reply->error() != QNetworkReply::NoError) {
        emit requestFailed(role, reply->errorString());
    } else {
        QByteArray responseBytes = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseBytes);
        QString responseText;

        if (jsonResponse.isObject()) {
            QJsonObject obj = jsonResponse.object();
            if (obj.contains("choices") && obj["choices"].isArray()) {
                QJsonArray choices = obj["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject first = choices.first().toObject();
                    QJsonObject message = first["message"].toObject();
                    responseText = message["content"].toString().trimmed();
                }
            }
        }
        emit responseReady(taskId, role, responseText, modelUsed);
    }
    reply->deleteLater();
}

QString ModelManager::getModelForRole(const QString &role) const
{
    return m_roleToModelMap.value(role, m_defaultModel);
}
