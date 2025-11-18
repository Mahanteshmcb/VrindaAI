#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <QObject>
#include <QMap>
#include <QVariantMap>
#include <QString>
#include <QQueue>
#include <functional> // For std::function

class QNetworkAccessManager;
class QNetworkReply;
class LlamaServerController;

class ModelManager : public QObject
{
    Q_OBJECT
public:
    explicit ModelManager(QObject *parent = nullptr);

    // --- CRITICAL: Setter for the LlamaServerController ---
    void setServerController(LlamaServerController *controller);

public slots:
    void sendRequest(const QString &taskId, const QString &role, const QString &prompt);
    QString getModelForRole(const QString &role) const;

signals:
    void responseReady(const QString &taskId, const QString &role, const QString &response, const QString &modelUsed);
    void requestFailed(const QString &role, const QString &errorString);
    // New signal to initiate the polling mechanism (used by ModelManager)
    void startHealthCheck(int port, std::function<void()> onReadyCallback);

private slots:
    void onCompletionReplyFinished(QNetworkReply *reply);
    void processPendingRequestQueue(); // New slot to process queued tasks

private:
    QNetworkAccessManager *m_networkManager;

    // LINKAGE AND STATE
    LlamaServerController *m_serverController = nullptr;
    QMap<QString, QString> m_roleToModelMap;
    QString m_defaultModel;

    // Tracks the currently loaded model and port (essential for swap)
    QString m_activeModelName = "";
    int m_activeModelPort = 0;

    QString m_baseModelPath;
    QMap<QString, int> m_modelToPortMap;

    void sendNetworkRequest(const QString &taskId, const QString &role, const QString &prompt, const QString &modelName, int port);

    // Stores the request details while waiting for the model to load
    struct PendingRequest {
        QString taskId;
        QString role;
        QString prompt;
        QString model;
        int port;
    };
    QQueue<PendingRequest> m_pendingRequests; // Queue for tasks awaiting server load

    // NOTE: m_currentModelLoaded is unused/obsolete.
};

#endif // MODELMANAGER_H
