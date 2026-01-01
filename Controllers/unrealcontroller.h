#ifndef UNREALCONTROLLER_H
#define UNREALCONTROLLER_H

#include <QObject>
#include <QJsonArray>
#include "Controllers/projectstatecontroller.h"

class UnrealController : public QObject
{
    Q_OBJECT
public:
    explicit UnrealController(QObject *parent = nullptr);
    virtual ~UnrealController() = default; // Explicit virtual destructor for QObject compatibility

    /**
     * @brief Set the path to UnrealEditor-Cmd.exe (Industry Standard for Headless Automation)
     */
    void setUnrealEditorPath(const QString &path);

    /**
     * @brief Links the manifest controller to resolve Asset IDs during execution
     */
    void setProjectStateController(ProjectStateController* controller);

public slots:
    /**
     * @brief Executes a sequence of AAA Director commands (Spawn, Sequence, Render)
     */
    void executeInstructionSequence(const QString &projectPath, const QJsonArray &instructions);

signals:
    void unrealOutput(const QString &output);
    void unrealError(const QString &error);

private:
    /**
     * @brief Launches Unreal Engine in headless mode (-NullRHI -Unattended)
     */
    void runUnrealProcess(const QString &projectPath, const QString &scriptPath);

    QString m_unrealEditorPath;
    ProjectStateController* m_projectStateController = nullptr;
};

#endif // UNREALCONTROLLER_H