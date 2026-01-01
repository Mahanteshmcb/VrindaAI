#ifndef LLAMASERVERCONTROLLER_H
#define LLAMASERVERCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QProcess>

// --- RESOURCE MANAGEMENT CONSTANT ---
// CRITICAL: Change this value (1, 2, 3, or 4) to manually cap system load.
const int MAX_ACTIVE_LLM_SERVERS = 2;

class LlamaServerController : public QObject
{
    Q_OBJECT
public:
    explicit LlamaServerController(const QString &basePath, QObject *parent = nullptr);
    // Requests the controller to stop the running server on the specified port.
    void stopServerOnPort(int port);
    // Requests the controller to launch a specific model on the specified port.
    void startServer(const QString &modelName, int port);
    void stopAllServers();
    void startAllServers();


signals:
    void serverStatus(const QString &message);
    void serverError(const QString &message);

private:
    QString m_serverPath;
    QString m_basePath;
    // This map now tracks WHICH model is running on which port.
    QMap<int, QProcess*> m_activeServers;
    QMap<int, QString> m_currentModelOnPort; // Added to track current model

    // Configuration Map: Model Filename -> Port
    const QMap<QString, int> c_serverConfig = {
        {"Phimini.gguf", 8080},          // Planner, Vrinda
        {"qwencoder.gguf", 8081},   // Manager, Coder, Validator
        {"llava.gguf", 8082},     // Heavy tasks: Modeling, Animation
        {"mistral.gguf", 8083}     // Vision tasks: Texturing
    };
    QString m_activeModelName = "";
    int m_activeModelPort = 0;
};

#endif // LLAMASERVERCONTROLLER_H
