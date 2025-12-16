#include "modelmanager.h"
#include "Controllers/llamaservercontroller.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QVariantMap>

ModelManager::ModelManager(QObject *parent)
    : QObject(parent), m_activeModelPort(0), m_isCurrentlySwapping(false)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &ModelManager::onCompletionReplyFinished);

    m_defaultModel = "mistral.gguf";

    // Role Assignments
    m_roleToModelMap["Manager"] = "qwencoder.gguf";
    m_roleToModelMap["Coder"] = "qwencoder.gguf";
    m_roleToModelMap["Planner"] = "mistral.gguf";
    m_roleToModelMap["Researcher"] = "Phimini.gguf";
    // ... add others as needed

    // Port Mapping
    m_modelToPortMap["Phimini.gguf"] = 8080;
    m_modelToPortMap["qwencoder.gguf"] = 8081;
    m_modelToPortMap["llava.gguf -- mmproj-model-f16.gguf"] = 8082;
    m_modelToPortMap["mistral.gguf"] = 8083;
}

void ModelManager::setServerController(LlamaServerController *controller) {
    m_serverController = controller;
}

void ModelManager::sendRequest(const QString &taskId, const QString &role, const QString &prompt) {
    if (!m_serverController) {
        emit requestFailed(role, "Server controller not initialized.");
        return;
    }

    QString requiredModel = getModelForRole(role);
    int requiredPort = m_modelToPortMap.value(requiredModel, 8080);

    // 1. If correct model is loaded and NOT swapping, send now
    if (m_activeModelName == requiredModel && !m_isCurrentlySwapping) {
        sendNetworkRequest(taskId, role, prompt, requiredModel, requiredPort);
        return;
    }

    // 2. Queue the request
    m_pendingRequests.enqueue({taskId, role, prompt, requiredModel, requiredPort});

    // 3. Prevent multiple swap triggers
    if (m_isCurrentlySwapping) {
        qDebug() << "SCHEDULER: Already swapping. Request for" << role << "is in queue.";
        return;
    }

    // 4. Trigger Physical Swap
    qDebug() << "SCHEDULER: Initiating swap:" << m_activeModelName << "-> " << requiredModel;
    m_isCurrentlySwapping = true;

    // Use stopAllServers to be absolutely sure port 8080-8083 are free
    m_serverController->stopAllServers();

    m_activeModelName = requiredModel;
    m_activeModelPort = requiredPort;

    // Start the process
    m_serverController->startServer(requiredModel, requiredPort);

    // 5. Emit signal for MainWindow to handle health checks
    emit startHealthCheck(requiredPort, [this]() {
        qDebug() << "SCHEDULER: Swap successful. " << this->m_activeModelName << "is ready.";
        this->m_isCurrentlySwapping = false; 
        this->processPendingRequestQueue();
    });
}

void ModelManager::processPendingRequestQueue() {
    if (m_pendingRequests.isEmpty()) return;

    QQueue<PendingRequest> failedQueue;
    while (!m_pendingRequests.isEmpty()) {
        PendingRequest req = m_pendingRequests.dequeue();
        if (req.model == m_activeModelName) {
            sendNetworkRequest(req.taskId, req.role, req.prompt, req.model, req.port);
        } else {
            failedQueue.enqueue(req);
        }
    }
    m_pendingRequests = failedQueue;

    // If there's another model transition needed, start it
    if (!m_pendingRequests.isEmpty()) {
        PendingRequest next = m_pendingRequests.head();
        sendRequest(next.taskId, next.role, next.prompt);
    }
}

void ModelManager::sendNetworkRequest(const QString &taskId, const QString &role, const QString &prompt, const QString &modelName, int port) {
    QString urlString = QString("http://127.0.0.1:%1/v1/chat/completions").arg(port);
    QNetworkRequest request(urlString);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject payload;
    payload["model"] = modelName;
    QJsonArray messages;
    messages.append(QJsonObject{{"role", "system"}, {"content", "You are the " + role + " agent."}});
    messages.append(QJsonObject{{"role", "user"}, {"content", prompt}});
    payload["messages"] = messages;

    QVariantMap requestData;
    requestData["role"] = role;
    requestData["taskId"] = taskId;
    requestData["modelUsed"] = modelName;
    request.setAttribute(QNetworkRequest::User, requestData);

    m_networkManager->post(request, QJsonDocument(payload).toJson());
}

void ModelManager::onCompletionReplyFinished(QNetworkReply *reply) {
    QVariantMap requestData = reply->request().attribute(QNetworkRequest::User).toMap();
    const QString role = requestData["role"].toString();
    const QString taskId = requestData["taskId"].toString();
    const QString modelUsed = requestData["modelUsed"].toString();

    if (reply->error() != QNetworkReply::NoError) {
        emit requestFailed(role, reply->errorString());
    } else {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
        QString responseText;
        if (jsonResponse.isObject()) {
            QJsonObject obj = jsonResponse.object();
            QJsonArray choices = obj["choices"].toArray();
            if (!choices.isEmpty()) {
                responseText = choices.first().toObject()["message"].toObject()["content"].toString().trimmed();
            }
        }
        emit responseReady(taskId, role, responseText, modelUsed);
    }
    reply->deleteLater();
}

QString ModelManager::getModelForRole(const QString &role) const {
    return m_roleToModelMap.value(role, m_defaultModel);
}