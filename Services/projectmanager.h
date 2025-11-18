#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QString>

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectManager(const QString &basePath, QObject *parent = nullptr);

    QString createNewProject(const QString &goal);
    QString findExistingProject(const QString &projectName);
    QString getActiveProjectPath() const;
    QString getBaseProjectsPath() const;
    bool isProjectLoaded() const;
    void loadProject(const QString &projectPath);

private:
    QString m_baseProjectsDir;
    QString m_activeProjectPath;
};

#endif // PROJECTMANAGER_H
