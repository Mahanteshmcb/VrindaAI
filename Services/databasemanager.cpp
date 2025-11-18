#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>
#include <QCryptographicHash> // Required for generating a unique hash

DatabaseManager::DatabaseManager(const QString &path, QObject *parent)
    : QObject(parent),
    m_dbPath(path)
{
    // --- FIX: Create a unique connection name from the path ---
    // This allows multiple database connections (one global, one project) to exist simultaneously.
    m_connectionName = "vrinda_db_" + QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Md5).toHex();
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    // Remove the named connection so it can be re-added later if needed
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool DatabaseManager::setupDatabase()
{
    // Now we correctly use the unique connection name
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    QString dbPath = m_dbPath + "/db/tasks.sqlite";
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qDebug() << "❌ DB Error:" << m_db.lastError().text() << "for" << dbPath;
        return false;
    }

    QSqlQuery query(m_db);
    bool success = query.exec(R"(CREATE TABLE IF NOT EXISTS tasks (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        role TEXT,
        task TEXT,
        response TEXT,
        memory_notes TEXT,
        version TEXT,
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
    ))");

    if (!success) {
        qDebug() << "❌ Table creation failed:" << query.lastError().text();
        return false;
    }

    qDebug() << "✅ Database initialized at:" << dbPath;
    return true;
}


void DatabaseManager::logTask(const QString &role, const QString &task, const QString &response, const QString &memory, const QString &version)
{
    if (!m_db.isOpen()) {
        qDebug() << "❌ Database is not open. Cannot log task for role:" << role;
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO tasks (role, task, response, memory_notes, version) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(role);
    query.addBindValue(task);
    query.addBindValue(response);
    query.addBindValue(memory);
    query.addBindValue(version);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    if (!query.exec()) {
        qDebug() << "Insert error:" << query.lastError().text();
    } else {
        qDebug() << "🧠 Task + response + memory logged to DB for role:" << role;
        emit taskLogged(role, task, response, memory, timestamp);
    }
}

QStringList DatabaseManager::loadAgentLogs(const QString &role)
{
    QStringList logs;
    if (!m_db.isOpen()) {
        qDebug() << "❌ Database is not open. Cannot load logs for role:" << role;
        logs.append("❌ Database is not open.");
        return logs;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT task, response, memory_notes, timestamp FROM tasks WHERE role = ? ORDER BY timestamp ASC");
    query.addBindValue(role);

    if (!query.exec()) {
        qDebug() << "❌ Failed to load logs for role:" << role << query.lastError().text();
        logs.append("❌ Failed to load logs for role: " + role);
        return logs;
    }

    int rowCount = 0;
    while (query.next()) {
        QString task = query.value("task").toString();
        QString response = query.value("response").toString();
        QString memory = query.value("memory_notes").toString();
        QString time = query.value("timestamp").toString();

        if (!task.isEmpty())
            logs.append("[" + time + "] You: " + task);
        if (!response.isEmpty())
            logs.append("[" + time + "] 🧑 " + role + ": " + response);
        if (!memory.isEmpty())
            logs.append("[" + time + "] 💡 Memory: " + memory);
        rowCount++;
    }

    if (rowCount == 0) {
        logs.append("📁 No previous logs found.\n");
    } else {
        logs.prepend("🔁 Loaded previous chat history.\n");
    }
    return logs;
}

// Definition for the getDb() function needed by ReportGenerator
const QSqlDatabase &DatabaseManager::getDb() const
{
    return m_db;
}
