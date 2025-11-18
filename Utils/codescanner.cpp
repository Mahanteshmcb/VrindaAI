#include "codescanner.h"
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSet>

CodeScanner::CodeScanner() {}

QString CodeScanner::summarizeProject(const QString &projectPath)
{
    QString summary = "Project Analysis Report for: " + projectPath + "\n\n";
    QSet<QString> ignoredFolders = {".git", "build", ".vscode", ".idea", "debug", "release"};
    QSet<QString> textFileExtensions = {"txt", "md", "cpp", "h", "py", "js", "html", "css", "qml", "json"};

    QDirIterator it(projectPath, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString currentPath = it.next();
        QFileInfo fileInfo = it.fileInfo();

        // --- FIX: Correct logic to skip ignored directories ---
        // Check if the current path is inside one of the ignored folders.
        bool isInIgnoredFolder = false;
        QString relativePath = QDir(projectPath).relativeFilePath(currentPath);
        for (const QString &ignored : ignoredFolders) {
            if (QDir(projectPath).relativeFilePath(currentPath).startsWith(ignored + "/")) {
                isInIgnoredFolder = true;
                break;
            }
        }
        if (isInIgnoredFolder) {
            continue; // Skip this item and move to the next
        }
        // --- End Correction  ---


        if (fileInfo.isDir()) {
            if (ignoredFolders.contains(fileInfo.fileName())) {
                // This handles top-level ignored folders
                continue;
            }
            summary += "Directory: " + QDir(projectPath).relativeFilePath(fileInfo.absoluteFilePath()) + "\n";

        } else if (fileInfo.isFile()) {
            if (textFileExtensions.contains(fileInfo.suffix().toLower())) {
                summary += "  File: " + QDir(projectPath).relativeFilePath(fileInfo.absoluteFilePath()) + "\n";
                summary += "  ---\n";
                QFile file(fileInfo.absoluteFilePath());
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    // Limit reading to a reasonable size to avoid huge summaries
                    QTextStream in(&file);
                    summary += in.read(8192); // Read up to 8KB of the file
                    if (!in.atEnd()) {
                        summary += "\n... [file truncated] ...";
                    }
                    file.close();
                }
                summary += "\n  ---\n";
            }
        }
    }
    return summary;
}
