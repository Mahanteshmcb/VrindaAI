#include "mainwindow.h"
#include "Services/headlessexecutor.h"

#include <QApplication>
#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include <iostream>

// Forward declare logging function from headlessexecutor.cpp
extern void initLogging();
extern void logMessage(const std::string& message);

int main(int argc, char *argv[])
{
    // Check for headless mode arguments
    bool headlessMode = false;
    QString jobFile;
    QString workflowFile;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLatin1(argv[i]);
        
        if (arg == "--headless") {
            headlessMode = true;
        } else if (arg == "--job" && i + 1 < argc) {
            jobFile = QString::fromLatin1(argv[++i]);
        } else if (arg == "--workflow" && i + 1 < argc) {
            workflowFile = QString::fromLatin1(argv[++i]);
        }
    }
    
    // Headless mode: execute job without GUI
    if (headlessMode) {
        std::cout << "========================================" << std::endl;
        std::cout << "VrindaAI - Headless Execution Mode" << std::endl;
        std::cout << "========================================" << std::endl;
        
        initLogging();
        logMessage("===== VrindaAI Headless Execution Started =====");
        
        QCoreApplication app(argc, argv);
        
        HeadlessExecutor executor;
        int exitCode = 0;
        
        if (!jobFile.isEmpty()) {
            logMessage("Headless mode: executing job file " + jobFile.toStdString());
            exitCode = executor.executeJobFile(jobFile);
        } else if (!workflowFile.isEmpty()) {
            logMessage("Headless mode: executing workflow " + workflowFile.toStdString());
            exitCode = executor.executeWorkflow(workflowFile);
        } else {
            logMessage("ERROR: Headless mode requires --job or --workflow argument");
            exitCode = 1;
        }
        
        logMessage("===== VrindaAI Headless Execution Finished (exit code: " + std::to_string(exitCode) + ") =====");
        
        std::cout << "========================================" << std::endl;
        std::cout << "Execution finished with exit code: " << exitCode << std::endl;
        std::cout << "Check headless_execution.log for details." << std::endl;
        std::cout << "========================================" << std::endl;
        
        return exitCode;
    }
    
    // GUI mode: normal application
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "VrindaAI_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}

