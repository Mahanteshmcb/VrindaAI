#include "projectmanager.h"
#include <QDir>
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>

ProjectManager::ProjectManager(const QString &basePath, QObject *parent)
    : QObject(parent)
{
    m_baseProjectsDir = basePath + "/projects";
    QDir dir(m_baseProjectsDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QString ProjectManager::createNewProject(const QString &goal)
{
    // Clean name generation from goal
    QStringList words = goal.split(' ', Qt::SkipEmptyParts);
    QString shortGoal = words.mid(0, 8).join(' '); 

    QString goalSlug = shortGoal.toLower()
                           .replace(QRegularExpression(R"([^a-z0-9\s_])"), "") 
                           .trimmed()
                           .replace(QRegularExpression(R"(\s+)"), "_") 
                           .left(50); 

    if (goalSlug.isEmpty()) {
        goalSlug = "new_project_" + QString::number(QDateTime::currentSecsSinceEpoch());
    }

    QString newProjectPath = m_baseProjectsDir + "/" + goalSlug;
    QDir projectDir(newProjectPath);
    int counter = 1;
    while (projectDir.exists()) {
        newProjectPath = m_baseProjectsDir + "/" + goalSlug + "_" + QString::number(counter++);
        projectDir.setPath(newProjectPath);
    }

    // --- PHASE 1: INDUSTRY STANDARD FOLDER STANDARDIZATION ---
    // Core Workflow Folders [cite: 10, 12, 27, 28]
    projectDir.mkpath(".");
    projectDir.mkpath("Raw_Downloads");  // For initial asset collection [cite: 12]
    projectDir.mkpath("Processed_FBX"); // For Blender Auto-Rigger output [cite: 12]
    projectDir.mkpath("Renders");       // For Blender/Unreal frame sequences [cite: 12]
    projectDir.mkpath("Final_Exports"); // For FFmpeg final movie output [cite: 28]
    
    // Support & Code Folders
    projectDir.mkpath("src");
    projectDir.mkpath("scripts");
    projectDir.mkpath("db");            // For project-specific tasks.sqlite
    projectDir.mkpath("logs");          // For Phase 5 feedback loop reports [cite: 26]
    projectDir.mkpath("reports");
    projectDir.mkpath("docs");
    projectDir.mkpath("r&d");           // Research documentation

    qDebug() << "✅ Phase 1 Complete: Standardized workspace established at:" << newProjectPath;
    return newProjectPath;
}



QString ProjectManager::findExistingProject(const QString &projectName)
{
    // For now, this is a simple check. It can be made more intelligent later.
    QString projectPath = m_baseProjectsDir + "/" + projectName;
    if (QDir(projectPath).exists()) {
        return projectPath;
    }
    return QString(); // Return empty string if not found
}

void ProjectManager::loadProject(const QString &projectPath)
{
    m_activeProjectPath = projectPath;
    qDebug() << "✅ Project loaded:" << m_activeProjectPath;
}

QString ProjectManager::getActiveProjectPath() const
{
    return m_activeProjectPath;
}

bool ProjectManager::isProjectLoaded() const
{
    return !m_activeProjectPath.isEmpty();
}
// get the path to the main "projects" folder.
QString ProjectManager::getBaseProjectsPath() const
{
    return m_baseProjectsDir;
}
