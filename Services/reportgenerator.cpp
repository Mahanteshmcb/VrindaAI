#include "reportgenerator.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QPrinter>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>

ReportGenerator::ReportGenerator(const QString &projectPath, DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), m_projectPath(projectPath), m_dbManager(dbManager)
{
}

void ReportGenerator::exportTasksToCSV(const QString &filename) {
    QString reportDir = m_projectPath + "/reports";
    QDir dir;
    if (!dir.exists(reportDir)) dir.mkpath(reportDir);

    QString fullPath = reportDir + "/" + filename;

    // --- FIX: Use the database from the stored dbManager pointer ---
    if (!m_dbManager || !m_dbManager->getDb().isOpen()) {
        qDebug() << "❌ Report Generator Error: Database connection is not available.";
        return;
    }
    // The query now correctly uses the database connection provided by the manager
    QSqlQuery query(m_dbManager->getDb());

    query.exec("SELECT id, role, task, response, memory_notes, version, timestamp FROM tasks ORDER BY timestamp ASC");

    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "❌ Could not open CSV for writing:" << fullPath;
        return;
    }

    QTextStream out(&file);
    out << "ID,Role,Task,Response,Memory Notes,Version,Timestamp\n";

    while (query.next()) {
        QStringList row;
        for (int i = 0; i < query.record().count(); ++i) {
            QString value = query.value(i).toString();
            value.replace("\"", "\"\""); // escape quotes
            value.replace("\n", " ");    // remove newline
            row << "\"" + value + "\"";
        }
        out << row.join(",") << "\n";
    }

    file.close();
    qDebug() << "✅ Task history exported to:" << fullPath;
}

void ReportGenerator::exportTaskReportAsPDF(const QString &filename) {
    // --- FIX: Use the database from the stored dbManager pointer ---
    if (!m_dbManager || !m_dbManager->getDb().isOpen()) {
        qDebug() << "❌ Report Generator Error: Database connection is not available.";
        return;
    }
    // The query now correctly uses the database connection provided by the manager
    QSqlQuery query(m_dbManager->getDb());

    query.exec("SELECT role, task, response, memory_notes, version, timestamp FROM tasks");

    QTextDocument doc;
    QTextCursor cursor(&doc);

    cursor.insertHtml("<h1 align='center'>📝 VrindaAI Task History Report</h1><hr><br>");

    QTextCharFormat labelFormat;
    labelFormat.setFontWeight(QFont::DemiBold);

    QTextCharFormat contentFormat;
    contentFormat.setFontPointSize(10);

    while (query.next()) {
        QString role = query.value(0).toString();
        QString task = query.value(1).toString();
        QString response = query.value(2).toString();
        QString memory = query.value(3).toString();
        QString version = query.value(4).toString();
        QString timestamp = query.value(5).toString();

        cursor.insertText("👤 Role: ", labelFormat); cursor.insertText(role + "\n", contentFormat);
        cursor.insertText("🧠 Task: ", labelFormat); cursor.insertText(task + "\n", contentFormat);
        cursor.insertText("💡 Memory: ", labelFormat); cursor.insertText(memory + "\n", contentFormat);
        cursor.insertText("🔢 Version: ", labelFormat); cursor.insertText(version + "\n", contentFormat);
        cursor.insertText("🕒 Timestamp: ", labelFormat); cursor.insertText(timestamp + "\n", contentFormat);

        cursor.insertText("\n", contentFormat);
        cursor.insertText("🎯 Response:\n", labelFormat);
        cursor.insertText(response + "\n", contentFormat);
        cursor.insertHtml("<br><hr><br>");
    }

    QString fullPath = m_projectPath + "/reports/" + filename;

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fullPath);

    doc.print(&printer);
    QMessageBox::information(nullptr, "Export Complete", "📄 PDF report saved to:\n" + fullPath);
}

void ReportGenerator::saveLogToFile(const QString &role, const QString &task, const QString &response, const QString &memory, const QString &timestamp) {
    // This function does not access the database, so no changes were needed here.
    QString logDir = m_projectPath + "/logs/";
    QDir dir(logDir);
    if (!dir.exists()) dir.mkpath(".");

    QString filename = logDir + role.toLower() + "_log.txt";

    QFile file(filename);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << "[" << timestamp << "] You (" << role << "): " << task << "\n";
        if (!response.isEmpty())
            out << "[" << timestamp << "] 🧑 " << role << ": " << response << "\n";
        if (!memory.isEmpty())
            out << "[" << timestamp << "] 💡 Memory: " << memory << "\n";
        out << "----------------------------------------\n";
        file.close();
    } else {
        qDebug() << "❌ Could not open log file for writing:" << filename;
    }
}
