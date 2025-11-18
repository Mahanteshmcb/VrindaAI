#include "projectworkflow.h"
#include <QDebug>
#include <QRegularExpression>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ProjectWorkflow::ProjectWorkflow(const QString &basePath, QMap<QString, QString> &roleBuffers, QObject *parent)
    : QObject(parent),
    m_basePath(basePath),
    m_roleBuffers(roleBuffers)
{
}

bool ProjectWorkflow::isRunning() const
{
    return m_isRunning;
}

void ProjectWorkflow::startWorkflowFromPlan(const QString &managerResponse)
{
    emit workflowMessage("📋 Advanced plan received. Parsing JSON plan...");
    m_tasks.clear();

    // --- NEW: PARSE THE RESPONSE AS JSON ---
    QJsonDocument doc = QJsonDocument::fromJson(managerResponse.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        emit workflowMessage("❌ Manager's plan was not valid JSON. Workflow stopped.");
        return;
    }

    QJsonObject root = doc.object();
    if (root.contains("plan") && root["plan"].isArray()) {
        parsePlan(root["plan"].toArray());
    }

    if (!m_tasks.isEmpty()) {
        m_isRunning = true;
        evaluateWorkflow(); // Kick off the first evaluation
    } else {
        emit workflowMessage("❌ Manager's plan was empty or in an invalid format. Workflow stopped.");
    }
}

void ProjectWorkflow::taskFinished(const QString &taskId)
{
    if (!m_tasks.contains(taskId) || m_tasks[taskId].status == WorkflowTask::Complete) return;

    emit workflowMessage(QString("✅ Task %1 (%2) has finished.").arg(taskId, m_tasks[taskId].role));
    m_tasks[taskId].status = WorkflowTask::Complete;
    evaluateWorkflow();// Re-evaluate the workflow to see what's unblocked
}


void ProjectWorkflow::parsePlan(const QJsonArray& planArray)
{
    // This regex is no longer needed and can be deleted
    // const QRegularExpression taskRegex(...);

    for (const QJsonValue &value : planArray) {
        QJsonObject taskObj = value.toObject();
        if (taskObj.contains("id") && taskObj.contains("role") && taskObj.contains("description") && taskObj.contains("dependencies")) {
            WorkflowTask task;
            task.id = QString::number(taskObj["id"].toInt());
            task.role = taskObj["role"].toString();
            task.description = taskObj["description"].toString();

            QJsonArray deps = taskObj["dependencies"].toArray();
            if (deps.isEmpty()) {
                task.status = WorkflowTask::Ready;
            } else {
                task.status = WorkflowTask::Pending;
                for (const QJsonValue &dep : deps) {
                    task.dependencies.insert(QString::number(dep.toInt()));
                }
            }
            m_tasks.insert(task.id, task);
            qDebug() << "Parsed Task" << task.id << "for Role" << task.role << "with dependencies:" << task.dependencies;
        }
    }
}

void ProjectWorkflow::taskFailed(const QString &taskId, const QString &reason)
{
    if (!m_tasks.contains(taskId)) return;

    emit workflowMessage(QString("❌ Task %1 (%2) has failed. Reason: %3").arg(taskId, m_tasks[taskId].role, reason));
    m_tasks[taskId].status = WorkflowTask::Failed;

    // --- MODIFIED: Escalate with the current plan state ---
    emit escalateToManager(taskId, reason, getPlanStateAsJson());

    m_isRunning = false;
    emit workflowMessage("⏸️ Workflow paused pending manager correction.");
}

// --- ADD THIS NEW HELPER FUNCTION ---
void ProjectWorkflow::applyPlanModification(const QJsonObject &modification)
{
    if (modification.contains("add_tasks")) {
        parsePlan(modification["add_tasks"].toArray()); // We can reuse our parsePlan function!
    }
    if (modification.contains("retry_tasks")) {
        QJsonArray tasksToRetry = modification["retry_tasks"].toArray();
        for (const QJsonValue &val : tasksToRetry) {
            QString taskId = QString::number(val.toInt());
            if (m_tasks.contains(taskId)) {
                m_tasks[taskId].status = WorkflowTask::Pending; // Reset status to re-queue it
                emit workflowMessage(QString("🔄 Task %1 has been re-queued for another attempt.").arg(taskId));
            }
        }
    }
    // Note: abort_tasks logic can be added here if needed.

    // Resume the workflow with the modified plan
    m_isRunning = true;
    emit workflowMessage("✅ Plan modified by Corrector. Resuming workflow...");
    evaluateWorkflow();
}

// --- ADD THIS NEW HELPER FUNCTION ---
QJsonArray ProjectWorkflow::getPlanStateAsJson() const
{
    QJsonArray planState;
    for (const WorkflowTask &task : m_tasks.values()) {
        QJsonObject taskObj;
        taskObj["id"] = task.id.toInt();
        taskObj["role"] = task.role;
        taskObj["description"] = task.description;

        QString statusStr;
        switch(task.status) {
        case WorkflowTask::Pending: statusStr = "Pending"; break;
        case WorkflowTask::Ready: statusStr = "Ready"; break;
        case WorkflowTask::Running: statusStr = "Running"; break;
        case WorkflowTask::Complete: statusStr = "Complete"; break;
        case WorkflowTask::Failed: statusStr = "Failed"; break;
        }
        taskObj["status"] = statusStr;

        QJsonArray deps;
        for (const QString &dep : task.dependencies) {
            deps.append(dep.toInt());
        }
        taskObj["dependencies"] = deps;
        planState.append(taskObj);
    }
    return planState;
}

void ProjectWorkflow::evaluateWorkflow()
{
    // --- START OF NEW, MORE ROBUST LOGIC ---
    bool tasksWereDispatched;
    do {
        tasksWereDispatched = false;
        // Find and dispatch all tasks that are currently ready to run
        QList<QString> dispatchedTaskIds;

        for (auto it = m_tasks.begin(); it != m_tasks.end(); ++it) {
            WorkflowTask& task = it.value();

            if (task.status == WorkflowTask::Pending) {
                bool allDepsMet = true;
                for (const QString& depId : task.dependencies) {
                    // Check if dependency exists and is complete
                    if (!m_tasks.contains(depId) || m_tasks.value(depId).status != WorkflowTask::Complete) {
                        allDepsMet = false;
                        break;
                    }
                }
                if (allDepsMet) {
                    task.status = WorkflowTask::Ready;
                }
            }

            if (task.status == WorkflowTask::Ready) {
                tasksWereDispatched = true;
                task.status = WorkflowTask::Running;
                emit workflowMessage(QString("📤 Dispatching Task %1 to %2.").arg(task.id, task.role));
                emit assignTaskToAgent(task.id, task.role, task.description);
            }
        }
    } while (tasksWereDispatched); // Keep evaluating if we just unblocked and dispatched new tasks

    // Now, check if the project is finished ONLY after trying to dispatch all possible tasks.
    bool allComplete = true;
    for (const auto& task : m_tasks.values()) {
        if (task.status != WorkflowTask::Complete) {
            allComplete = false;
            break;
        }
    }

    if (allComplete && !m_tasks.values().isEmpty()) {
        m_isRunning = false;
        emit workflowFinished("✅ All tasks in the plan are complete. Project finished!");
        m_tasks.clear();
    }
}

QString ProjectWorkflow::getGeneratedScriptFile() {
    // This function needs to be smarter, looking in the active project path.
    // For now, this is a placeholder. We will improve this in Phase 2.
    if (m_roleBuffers.contains("Coder_output_path")) {
        return m_roleBuffers["Coder_output_path"];
    }
    return QString();
}
