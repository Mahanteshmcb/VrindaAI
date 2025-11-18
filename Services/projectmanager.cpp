#include "projectmanager.h"
#include <QDir>
#include <QDebug>
#include <QRegularExpression>

ProjectManager::ProjectManager(const QString &basePath, QObject *parent)
    : QObject(parent)
{
    // The main directory where all projects will be stored
    m_baseProjectsDir = basePath + "/projects";
    QDir dir(m_baseProjectsDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QString ProjectManager::createNewProject(const QString &goal)
{
    // --- NEW: Create a shorter, cleaner name from the goal ---
    QStringList words = goal.split(' ', Qt::SkipEmptyParts);
    QString shortGoal = words.mid(0, 8).join(' '); // Take the first 8 words

    QString goalSlug = shortGoal.toLower()
                           .replace(QRegularExpression(R"([^a-z0-9\s_])"), "") // Allow spaces and underscores initially
                           .trimmed()
                           .replace(QRegularExpression(R"(\s+)"), "_") // Replace whitespace with a single underscore
                           .left(50); // Truncate to a safe length

    if (goalSlug.isEmpty()) {
        goalSlug = "new_project_" + QString::number(QDateTime::currentSecsSinceEpoch());
    }

    QString newProjectPath = m_baseProjectsDir + "/" + goalSlug;
    QDir projectDir(newProjectPath);
    int counter = 1;
    while (projectDir.exists()) {
        // If project already exists, append a number
        newProjectPath = m_baseProjectsDir + "/" + goalSlug + "_" + QString::number(counter++);
        projectDir.setPath(newProjectPath);
    }

    // Create the project directory and standard sub-folders
    projectDir.mkpath(".");
    projectDir.mkpath("src");
    projectDir.mkpath("scripts");
    projectDir.mkpath("db");
    projectDir.mkpath("assets");
    projectDir.mkpath("logs");
    projectDir.mkpath("reports");
    projectDir.mkpath("frontend");
    projectDir.mkpath("backend");
    projectDir.mkpath("ui");
    projectDir.mkpath("docs");
    projectDir.mkpath("r&d");




    qDebug() << "✅ Created new project workspace at:" << newProjectPath;
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
