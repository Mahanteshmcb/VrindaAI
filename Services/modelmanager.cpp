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
#include <QTimer>

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
        emit requestFailed(taskId, role, "Server controller not initialized.");
        return;
    }

    QString requiredModel = getModelForRole(role);
    int requiredPort = m_modelToPortMap.value(requiredModel, 8080);

    // 1. If correct model is ALREADY loaded and server is idle, send immediately
    if (m_activeModelName == requiredModel && !m_isCurrentlySwapping) {
        sendNetworkRequest(taskId, role, prompt, requiredModel, requiredPort);
        return;
    }

    // 2. Queue the request regardless - treat it as a source of truth
    m_pendingRequests.enqueue({taskId, role, prompt, requiredModel, requiredPort});

    // 3. If we are already mid-swap, STOP HERE. 
    // The callback in the existing swap will eventually call processPendingRequestQueue()
    if (m_isCurrentlySwapping) {
        qDebug() << "SCHEDULER: Swap in progress. Request for" << role << "queued.";
        return;
    }

    // 4. Initiate the Serial Swap
    m_isCurrentlySwapping = true;
    
    // Only stop the specific port if you have multi-port capability, 
    // otherwise, stop only the active one to avoid "Connection Refused" on other requests.
    m_serverController->stopServerOnPort(m_activeModelPort); 

    m_activeModelName = requiredModel;
    m_activeModelPort = requiredPort;

    qDebug() << "SCHEDULER: Initiating physical swap to" << requiredModel << "on port" << requiredPort;
    m_serverController->startServer(requiredModel, requiredPort);

    // 5. The "Gatekeeper" Health Check
    // This Lambda is the only way m_isCurrentlySwapping becomes false.
    emit startHealthCheck(requiredPort, [this]() {
        qDebug() << "SCHEDULER: Port" << this->m_activeModelPort << "confirmed READY.";
        this->m_isCurrentlySwapping = false; 
        this->processPendingRequestQueue(); 
    });
}

void ModelManager::processPendingRequestQueue() {
    if (m_pendingRequests.isEmpty()) {
        m_isCurrentlySwapping = false; // Release lock if nothing left
        return;
    }

    QQueue<PendingRequest> keepInQueue;
    int staggerDelay = 100; // Start with a small buffer

    // 1. Dispatch all tasks matching the CURRENTLY LOADED model
    while (!m_pendingRequests.isEmpty()) {
        PendingRequest req = m_pendingRequests.dequeue();
        
        if (req.model == m_activeModelName) {
            QTimer::singleShot(staggerDelay, this, [=]() {
                this->sendNetworkRequest(req.taskId, req.role, req.prompt, req.model, req.port);
            });
            staggerDelay += 1500; // Increased to 1.5s to ensure llama-server context is clear
        } else {
            keepInQueue.enqueue(req);
        }
    }
    m_pendingRequests = keepInQueue;

    // 2. Schedule the next swap ONLY after all current requests are safely out the door
    if (!m_pendingRequests.isEmpty()) {
        QTimer::singleShot(staggerDelay + 1000, this, [this]() {
            // Check again if a new task finished and cleared the queue
            if (!this->m_pendingRequests.isEmpty()) {
                PendingRequest next = this->m_pendingRequests.head();
                this->m_isCurrentlySwapping = false; // Explicitly release to allow sendRequest to trigger swap
                this->sendRequest(next.taskId, next.role, next.prompt);
            }
        });
    } else {
        m_isCurrentlySwapping = false;
    }
}

void ModelManager::sendNetworkRequest(const QString &taskId, const QString &role, const QString &prompt, const QString &modelName, int port) {
    // 1. Add a 500ms safety delay. 
    // This gives the Windows Socket layer and the LLM engine time to "settle" after a swap.
    QTimer::singleShot(500, this, [=]() {
        QString urlString = QString("http://127.0.0.1:%1/v1/chat/completions").arg(port);
        QNetworkRequest request(urlString);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        // Set a longer timeout for these complex agent requests
        request.setTransferTimeout(60000); // 60 seconds

        QJsonObject payload;
        payload["model"] = modelName;
        QJsonArray messages;
        messages.append(QJsonObject{{"role", "system"}, {"content", "You are the " + role + " agent."}});
        messages.append(QJsonObject{{"role", "user"}, {"content", prompt}});
        payload["messages"] = messages;
        payload["temperature"] = 0.7;

        QVariantMap requestData;
        requestData["role"] = role;
        requestData["taskId"] = taskId;
        requestData["modelUsed"] = modelName;
        // Keep track of retry count to prevent infinite loops
        int retries = requestData.value("retryCount", 0).toInt();
        request.setAttribute(QNetworkRequest::User, requestData);

        QByteArray jsonData = QJsonDocument(payload).toJson();
        QNetworkReply* reply = m_networkManager->post(request, jsonData);

        // 2. Enhanced Error Handling for the Reply
        connect(reply, &QNetworkReply::finished, this, [this, reply, taskId, role, prompt, modelName, port, retries]() {
            if (reply->error() == QNetworkReply::ConnectionRefusedError || 
                reply->error() == QNetworkReply::RemoteHostClosedError) {
                
                if (retries < 3) {
                    qDebug() << "⚠️ Network retry" << (retries + 1) << "for role" << role << "on port" << port;
                    reply->deleteLater();
                    
                    // Wait 2 seconds before retrying to allow server to stabilize
                    QTimer::singleShot(2000, this, [=]() {
                        // Re-trigger the request but increment retry count manually
                        this->sendNetworkRequest(taskId, role, prompt, modelName, port);
                    });
                    return;
                }
            }
            // If no connection error, the normal finished signal (onCompletionReplyFinished) 
            // will handle the data or report a permanent failure.
        });
    });
}

void ModelManager::onCompletionReplyFinished(QNetworkReply *reply) {
    QVariantMap requestData = reply->request().attribute(QNetworkRequest::User).toMap();
    const QString role = requestData["role"].toString();
    const QString taskId = requestData["taskId"].toString();
    const QString modelUsed = requestData["modelUsed"].toString();

    if (reply->error() != QNetworkReply::NoError) {
        emit requestFailed(taskId, role, reply->errorString());
        return;
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