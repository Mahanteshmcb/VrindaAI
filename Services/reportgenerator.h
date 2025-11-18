#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <QObject>
#include <QString>
#include "databasemanager.h"

class ReportGenerator : public QObject
{
    Q_OBJECT
public:
    explicit ReportGenerator(const QString &projectPath, DatabaseManager *dbManager, QObject *parent = nullptr);

public slots:
    void exportTasksToCSV(const QString &filename);
    void exportTaskReportAsPDF(const QString &filename);
    void saveLogToFile(const QString &role, const QString &task, const QString &response, const QString &memory, const QString &timestamp);

private:
    QString m_projectPath;
    // --- FIX: Add a pointer to the db manager ---
    DatabaseManager *m_dbManager;
};

#endif // REPORTGENERATOR_H
