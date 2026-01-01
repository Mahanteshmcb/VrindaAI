#ifndef PROJECTWORKFLOW_H
#define PROJECTWORKFLOW_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QSet>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

// A struct to hold all information about a single task in the plan
struct WorkflowTask
{
    QString id;
    QString role;
    QString description;
    QSet<QString> dependencies;
    enum Status { Pending, Ready, Running, Complete, Failed };
    Status status = Pending;
};

class ProjectWorkflow : public QObject
{
    Q_OBJECT
public:
    explicit ProjectWorkflow(const QString &basePath, QMap<QString, QString> &roleBuffers, QObject *parent = nullptr);

    // Public functions to be called from MainWindow
    QString getGeneratedScriptFile();
    bool isRunning() const;
    QJsonObject getCurrentPlanState() const { 
        return m_currentPlan; 
    }

public slots:
    void startWorkflowFromPlan(const QString &managerResponse);
    void taskFinished(const QString &taskId);
    void taskFailed(const QString &taskId, const QString &reason);
    void applyPlanModification(const QJsonObject &modification);

signals:
    void assignTaskToAgent(const QString &taskId, const QString &role, const QString &task);
    void workflowMessage(const QString &message);
    void workflowFinished(const QString &finalMessage);
    void escalateToManager(const QString &failedTaskId, const QString &reason, const QJsonArray &currentPlanState);

private:
    void parsePlan(const QJsonArray &planArray);
    void evaluateWorkflow();

    QJsonArray getPlanStateAsJson() const;
    QJsonObject m_currentPlan;

    QString m_basePath;
    QMap<QString, QString> &m_roleBuffers;

    // The new plan structure that holds all tasks
    QMap<QString, WorkflowTask> m_tasks;
    bool m_isRunning = false;
};

#endif // PROJECTWORKFLOW_H
