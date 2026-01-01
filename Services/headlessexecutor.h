#ifndef HEADLESSEXECUTOR_H
#define HEADLESSEXECUTOR_H

#include <QString>
#include <QObject>
#include <QJsonObject> // Added for Neural Link
#include <QProcess>    // Added for Neural Link

/**
 * @class HeadlessExecutor
 * @brief Executes VrindaAI tasks without GUI
 * * This allows VrindaAI to run as a background service or be called
 * from other applications (like Vryndara) via command line.
 */
class HeadlessExecutor : public QObject {
    Q_OBJECT

public:
    // Updated constructor to support parenting (standard Qt practice)
    explicit HeadlessExecutor(QObject *parent = nullptr);
    ~HeadlessExecutor();

    /**
     * Execute a job from manifest file
     * Returns exit code: 0 = success, 1 = failure
     */
    int executeJobFile(const QString& jobFilePath);

    /**
     * Execute workflow from descriptor
     */
    int executeWorkflow(const QString& workflowFilePath);

    /**
     * Generate a job from AI-written description
     */
    int generateAndExecuteJob(const QString& taskDescription,
                             const QString& outputPath);

    // --- NEW: Universal Neural Link Method (Phase 2) ---
    // This is the single entry point for all C++ -> Python communication
    Q_INVOKABLE void executeUniversalTask(const QString &method, const QJsonObject &params);

signals:
    // Signals for UI communication (Phase 2)
    void neuralLinkFinished(const QJsonObject &result);
    void executionStarted(const QString &command);
    void executionError(const QString &error);

private slots:
    // Existing slots
    void onJobComplete();
    void onJobError(const QString& errorMessage);

    // New slots for Neural Link
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    bool m_isRunning;
    QString m_currentJobId;

    // Helper for Neural Link
    void runProcess(const QString &program, const QStringList &arguments);
};

#endif // HEADLESSEXECUTOR_H