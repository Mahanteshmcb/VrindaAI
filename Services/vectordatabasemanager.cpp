#include "vectordatabasemanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QTimer>

namespace {
// Defines a function that is run exactly once on startup to register the type.
bool registerVectorDBMetaTypes() {
    qRegisterMetaType<QList<MemoryQueryResult>>("QList<MemoryQueryResult>");
    return true;
}
// Call the function to ensure it runs globally once.
static bool registered = registerVectorDBMetaTypes();
}
VectorDatabaseManager::VectorDatabaseManager(QObject *parent)
    : QObject(parent), m_baseUrl("http://127.0.0.1:5100")
{
    m_networkManager = new QNetworkAccessManager(this);
}


void VectorDatabaseManager::addMemory(const QString &text)
{
    QNetworkRequest request(m_baseUrl.resolved(QUrl("/add_memory")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject payload;
    payload["text"] = text;

    QByteArray jsonData = QJsonDocument(payload).toJson();

    QNetworkReply *reply = m_networkManager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit memoryAddedSuccessfully();
        } else {
            qDebug() << "❌ Vector DB Error (addMemory):" << reply->errorString();
            emit memoryError(reply->errorString());
        }
        reply->deleteLater();
    });
}

void VectorDatabaseManager::queryMemory(const QString &queryText, int numResults)
{
    QNetworkRequest request(m_baseUrl.resolved(QUrl("/query_memory")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject payload;
    payload["query"] = queryText;
    payload["n_results"] = numResults;

    QByteArray jsonData = QJsonDocument(payload).toJson();

    QNetworkReply *reply = m_networkManager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            if (jsonDoc.isObject() && jsonDoc.object().contains("results")) {
                QJsonArray jsonResults = jsonDoc.object()["results"].toArray();
                QList<MemoryQueryResult> results; // <-- Use the new struct
                for (const QJsonValue &value : jsonResults) {
                    QJsonObject obj = value.toObject();
                    MemoryQueryResult res;
                    res.document = obj["document"].toString();
                    res.distance = obj["distance"].toDouble();
                    results.append(res);
                }
                emit queryResult(results); // <-- Emit the list of structs
            }
        } else {
            qDebug() << "❌ Vector DB Error (queryMemory):" << reply->errorString();
            emit memoryError(reply->errorString());
        }
        reply->deleteLater();
    });
}

void VectorDatabaseManager::checkHealthAndProceed(std::function<void()> callback)
{
    m_readyCallback = callback;
    // We send a minimal request to query_memory to test if the server is alive.

    // Call the polling slot directly to start the check
    onHealthCheckFinished(nullptr);
}

void VectorDatabaseManager::onHealthCheckFinished(QNetworkReply *reply)
{
    // 1. If this is a response (not the initial call)
    if (reply && reply->error() == QNetworkReply::NoError) {
        qDebug() << "✅ Vector DB Health Check successful! Server is ready.";
        // Server is ready! Execute the callback function.
        m_readyCallback();
        if (reply) reply->deleteLater();
        return;
    }

    if (reply) {
        qDebug() << "❌ Vector DB Health Check failed:" << reply->errorString() << "Retrying in 5 seconds...";
        reply->deleteLater();
    } else {
        qDebug() << "🚀 Starting initial Vector DB Health Check...";
    }

    // 2. Schedule the next check (5-second interval)
    QTimer::singleShot(5000, this, [=]() {
        QNetworkRequest request(m_baseUrl.resolved(QUrl("/query_memory")));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject payload;
        payload["query"] = "health check"; // Dummy query
        payload["n_results"] = 1;

        QByteArray jsonData = QJsonDocument(payload).toJson();
        QNetworkReply *newReply = m_networkManager->post(request, jsonData);

        // --- FIX: Use a lambda to bridge the argument mismatch ---
        connect(newReply, &QNetworkReply::finished, this, [=]() {
            // Now, manually call the function with the newReply pointer
            onHealthCheckFinished(newReply);
        });

    });
}
