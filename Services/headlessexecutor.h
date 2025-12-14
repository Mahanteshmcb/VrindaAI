#ifndef HEADLESSEXECUTOR_H
#define HEADLESSEXECUTOR_H

#include <QString>
#include <QObject>

/**
 * @class HeadlessExecutor
 * @brief Executes VrindaAI tasks without GUI
 * 
 * This allows VrindaAI to run as a background service or be called
 * from other applications (like Vryndara) via command line.
 */
class HeadlessExecutor : public QObject {
    Q_OBJECT

public:
    HeadlessExecutor();
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

private slots:
    void onJobComplete();
    void onJobError(const QString& errorMessage);

private:
    bool m_isRunning;
    QString m_currentJobId;
};

#endif // HEADLESSEXECUTOR_H
