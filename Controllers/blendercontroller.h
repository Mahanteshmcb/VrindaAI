#ifndef BLENDERCONTROLLER_H
#define BLENDERCONTROLLER_H

#include <QObject>
#include <QProcess>
#include <QFileInfo>
#include "Controllers/projectstatecontroller.h" // Needed for Manifest registration

class BlenderController : public QObject
{
    Q_OBJECT
public:
    explicit BlenderController(const QString &basePath, QObject *parent = nullptr);
    
    void setBlenderPath(const QString &path);
    void setActiveProjectPath(const QString &path);
    void setProjectStateController(ProjectStateController* controller) { m_projectStateController = controller; }

    // Phase 2 Action: Automated Casting
    void executeAutoRig(const QString &taskId, const QString &inputMeshPath, const QString &rigType = "basic_human");

signals:
    void blenderOutput(const QString &output);
    void blenderError(const QString &error);
    void blenderFinished(int exitCode);
    void assetReadyForEngine(const QString &assetPath);

private:
    QString m_basePath;
    QString m_blenderPath;
    QString m_activeProjectPath;
    QString m_engineScriptPath;
    ProjectStateController* m_projectStateController = nullptr;
};

#endif