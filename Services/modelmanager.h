#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QQueue>
#include <functional>

class LlamaServerController;
class QNetworkAccessManager;
class QNetworkReply;

class ModelManager : public QObject {
    Q_OBJECT
public:
    explicit ModelManager(QObject *parent = nullptr);
    QString getModelForRole(const QString &role) const;
    void setServerController(LlamaServerController *controller);
    void sendRequest(const QString &taskId, const QString &role, const QString &prompt);

signals:
    void responseReady(const QString &taskId, const QString &role, const QString &response, const QString &model);
    void requestFailed(const QString &taskId, const QString &role, const QString &errorString);    // Crucial: port to check, and code to run when it's alive
    void startHealthCheck(int port, std::function<void()> onReady);

private slots:
    void onCompletionReplyFinished(QNetworkReply *reply);
    void processPendingRequestQueue();

private:
    struct PendingRequest {
        QString taskId;
        QString role;
        QString prompt;
        QString model;
        int port;
    };

    QNetworkAccessManager *m_networkManager;
    LlamaServerController *m_serverController = nullptr;
    QMap<QString, QString> m_roleToModelMap;
    QMap<QString, int> m_modelToPortMap;
    QQueue<PendingRequest> m_pendingRequests;

    QString m_activeModelName;
    int m_activeModelPort;
    QString m_defaultModel;
    QString m_baseModelPath;
    bool m_isCurrentlySwapping;
    void sendNetworkRequest(const QString &taskId, const QString &role, const QString &prompt, const QString &model, int port);
};

#endif