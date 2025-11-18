#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QStringList>
#include <QSqlDatabase>

class QTextEdit; // Forward declaration is enough here

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(const QString &projectPath, QObject *parent = nullptr);
    ~DatabaseManager();

    // Public method to set up the database connection
    bool setupDatabase();

    // Returns a list of formatted log strings for the UI to display
    QStringList loadAgentLogs(const QString &role);

    // --- FIX: Add this public getter function ---
    const QSqlDatabase &getDb() const;

public slots:
    // Public slots to be called from MainWindow
    void logTask(const QString &role, const QString &task, const QString &response, const QString &memory, const QString &version);

signals:
    // Emitted after a task is successfully logged to the database
    void taskLogged(const QString &role, const QString &task, const QString &response, const QString &memory, const QString &timestamp);

private:
    QString m_dbPath;
    QString m_connectionName;
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
