#include "headlessexecutor.h"
#include "jobmanifestmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QDateTime>
#include <QCoreApplication> // Added for Universal Link
#include <QProcess>         // Added for Universal Link
#include <iostream>
#include <fstream>
#include <cstring>

// Simple logging function that writes to both console and file
static std::ofstream g_logFile;

void initLogging() {
    QString logPath = QDir::currentPath() + "/headless_execution.log";
    g_logFile.open(logPath.toStdString(), std::ios::app);
}

void logMessage(const std::string& message) {
    std::string timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
    std::string logLine = "[" + timestamp + "] " + message;
    
    // Write to console
    std::cout << logLine << std::endl;
    std::cout.flush();
    
    // Write to file
    if (g_logFile.is_open()) {
        g_logFile << logLine << std::endl;
        g_logFile.flush();
    }
}

HeadlessExecutor::HeadlessExecutor(QObject *parent) : QObject(parent), m_isRunning(false) {
    // Constructor
}

HeadlessExecutor::~HeadlessExecutor() {
    // Destructor
}

// --- EXISTING METHODS PRESERVED BELOW ---

int HeadlessExecutor::executeJobFile(const QString& jobFilePath) {
    logMessage("Executing job file: " + jobFilePath.toStdString());
    
    if (!QFile::exists(jobFilePath)) {
        logMessage("ERROR: Job file not found: " + jobFilePath.toStdString());
        return 1;
    }
    
    try {
        // Load job manifest
        JobManifestManager manager;
        QJsonObject manifest = manager.loadManifest(jobFilePath);
        
        // Validate
        if (!manager.validateManifest(manifest)) {
            logMessage("ERROR: Invalid job manifest: " + jobFilePath.toStdString());
            return 1;
        }
        
        // Determine engine and execute
        QString engineStr = manifest["engine"].toString().toLower();
        JobManifestManager::Engine engine;
        
        if (engineStr == "blender") {
            engine = JobManifestManager::Engine::Blender;
        } else if (engineStr == "unreal") {
            engine = JobManifestManager::Engine::Unreal;
        } else if (engineStr == "ffmpeg" || engineStr == "davinci") { // <<< FIXED: Support new FFmpeg and legacy DaVinci strings
            engine = JobManifestManager::Engine::FFmpeg;
        } else {
            logMessage("ERROR: Unknown engine: " + engineStr.toStdString());
            return 1;
        }
        
        // Execute
        m_isRunning = true;
        m_currentJobId = manifest["job_id"].toString();
        
        logMessage("Starting job: " + m_currentJobId.toStdString());
        logMessage("Calling manager.executeJob()...");
        
        if (!manager.executeJob(jobFilePath, engine)) {
            logMessage("ERROR: manager.executeJob returned false");
            logMessage("This may indicate: Blender/Unreal/Python/FFmpeg not in PATH, or process failed to start");
            return 1;
        }
        
        logMessage("Job execution initiated successfully");
        return 0;
    } catch (const std::exception& e) {
        logMessage(std::string("ERROR: Exception during job execution: ") + e.what());
        return 1;
    }
}

int HeadlessExecutor::executeWorkflow(const QString& workflowFilePath) {
    std::cout << "[HEADLESS] Executing workflow: " << workflowFilePath.toStdString() << std::endl;
    
    // Resolve absolute path if relative path given
    QString resolvedPath = workflowFilePath;
    if (QDir::isRelativePath(workflowFilePath)) {
        resolvedPath = QDir::currentPath() + "/" + workflowFilePath;
        std::cout << "[HEADLESS] Resolved relative path to: " << resolvedPath.toStdString() << std::endl;
    }
    
    if (!QFile::exists(resolvedPath)) {
        std::cerr << "[ERROR] Workflow file not found: " << resolvedPath.toStdString() << std::endl;
        std::cerr << "[ERROR] Current working directory: " << QDir::currentPath().toStdString() << std::endl;
        return 1;
    }
    
    try {
        // Load workflow descriptor
        QFile file(resolvedPath);
        if (!file.open(QIODevice::ReadOnly)) {
            std::cerr << "[ERROR] Cannot open workflow file: " << resolvedPath.toStdString() << std::endl;
            return 1;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        
        if (!doc.isObject()) {
            std::cerr << "[ERROR] Workflow file is not valid JSON object" << std::endl;
            return 1;
        }
        
        QJsonObject workflowObj = doc.object();
        QJsonArray jobs = workflowObj["jobs"].toArray();
        
        std::cout << "[HEADLESS] Workflow contains " << jobs.size() << " jobs" << std::endl;
        
        if (jobs.size() == 0) {
            std::cerr << "[ERROR] No jobs found in workflow" << std::endl;
            return 1;
        }
        
        // Execute each job in sequence
        int failedCount = 0;
        for (int i = 0; i < jobs.size(); ++i) {
            QString jobPath = jobs[i].toString();
            std::cout << "[HEADLESS] Executing job " << (i + 1) << " of " << jobs.size() << ": " << jobPath.toStdString() << std::endl;
            
            // Resolve job path relative to workflow directory
            QString resolvedJobPath = jobPath;
            if (QDir::isRelativePath(jobPath)) {
                QFileInfo workflowFileInfo(resolvedPath);
                resolvedJobPath = workflowFileInfo.absolutePath() + "/" + jobPath;
            }
            
            if (executeJobFile(resolvedJobPath) != 0) {
                failedCount++;
                std::cerr << "[WARNING] Job failed at index " << i << ": " << resolvedJobPath.toStdString() << std::endl;
                // Continue or fail based on workflow config
                bool continueOnError = workflowObj["continue_on_error"].toBool(false);
                if (!continueOnError) {
                    return 1;
                }
            } else {
                std::cout << "[HEADLESS] Job " << (i + 1) << " completed successfully" << std::endl;
            }
            
            // Small delay between jobs
            QThread::msleep(1000);
        }
        
        std::cout << "[HEADLESS] Workflow execution complete (" << failedCount << " jobs failed)" << std::endl;
        return (failedCount == 0) ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception during workflow execution: " << e.what() << std::endl;
        return 1;
    }
}

int HeadlessExecutor::generateAndExecuteJob(const QString& taskDescription,
                                            const QString& outputPath) {
    std::cout << "[HEADLESS] Generating job from description: " << taskDescription.toStdString() << std::endl;
    std::cout << "[HEADLESS] Output path: " << outputPath.toStdString() << std::endl;
    
    try {
        // TODO: Use LLM to generate job manifest from description
        // For now, this is a placeholder
        
        std::cerr << "[WARNING] Job generation not yet implemented" << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception during job generation: " << e.what() << std::endl;
        return 1;
    }
}

void HeadlessExecutor::onJobComplete() {
    std::cout << "[HEADLESS] Job completed: " << m_currentJobId.toStdString() << std::endl;
    m_isRunning = false;
}

void HeadlessExecutor::onJobError(const QString& errorMessage) {
    std::cerr << "[ERROR] Job error: " << errorMessage.toStdString() << std::endl;
    m_isRunning = false;
}

// --- NEW PHASE 2: UNIVERSAL NEURAL LINK IMPLEMENTATION ---

void HeadlessExecutor::executeUniversalTask(const QString &method, const QJsonObject &params)
{
    qDebug() << "🧠 Neural Link: Preparing to execute" << method;

    // 1. Construct the JSON Payload
    QJsonObject payload;
    payload["method"] = method;
    payload["params"] = params;

    QJsonDocument doc(payload);
    QString jsonString = doc.toJson(QJsonDocument::Compact);

    // 2. Locate the Python CLI
    // Look in standard deployment locations
    QString scriptPath = QCoreApplication::applicationDirPath() + "/src/vrindaai_cli.py";
    
    // Fallback for development environment
    if (!QFile::exists(scriptPath)) {
        // Try looking up two levels (standard Qt Creator build dir structure)
        scriptPath = QCoreApplication::applicationDirPath() + "/../../VrindaAI/vrindaai_cli.py";
    }
    
    if (!QFile::exists(scriptPath)) {
        scriptPath = "vrindaai_cli.py"; // Last resort
    }

    // 3. Configure Python Command
    QString program = "python"; 
    QStringList arguments;
    
    // QProcess handles quoting automatically
    arguments << scriptPath << "--json" << jsonString;

    qDebug() << "🚀 Launching Python Script:" << scriptPath;
    
    // 4. Run the Process
    runProcess(program, arguments);
}

void HeadlessExecutor::runProcess(const QString &program, const QStringList &arguments)
{
    QProcess *process = new QProcess(this);
    
    // --- FIX: Use SeparateChannels so Logs (Stderr) don't corrupt JSON (Stdout) ---
    process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &HeadlessExecutor::onProcessFinished);
    
    connect(process, &QProcess::errorOccurred,
            this, &HeadlessExecutor::onProcessError);

    // Optional: Connect readyReadStandardError if you want to stream logs in real-time
    // connect(process, &QProcess::readyReadStandardError, [process](){
    //     qDebug() << "PY_LOG:" << process->readAllStandardError();
    // });

    emit executionStarted(program + " " + arguments.join(" "));
    
    process->start(program, arguments);
}

void HeadlessExecutor::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    // --- FIX: Read ONLY Standard Output for JSON ---
    QString stdOut = QString::fromUtf8(process->readAllStandardOutput()).trimmed();
    QString stdErr = QString::fromUtf8(process->readAllStandardError()).trimmed();

    if (!stdErr.isEmpty()) {
        qDebug() << "⚠️ Python Logs:" << stdErr;
    }
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        qDebug() << "✅ Python Task Finished";
        
        // Parse the JSON result from Standard Output
        QJsonDocument responseDoc = QJsonDocument::fromJson(stdOut.toUtf8());
        QJsonObject responseObj = responseDoc.object();
        
        if (!responseObj.isEmpty()) {
            emit neuralLinkFinished(responseObj);
        } else {
            // Fallback: If output is empty, maybe the error is in stdErr
            QJsonObject error;
            error["status"] = "error";
            error["message"] = "Empty response from Python";
            error["error"] = stdErr; // Pass the log as the error detail
            emit neuralLinkFinished(error);
        }
    } else {
        qDebug() << "❌ Python Task Failed (Code" << exitCode << ")";
        QJsonObject error;
        error["status"] = "error";
        error["message"] = "Process crashed";
        error["error"] = stdErr;
        emit neuralLinkFinished(error);
    }

    process->deleteLater();
}

void HeadlessExecutor::onProcessError(QProcess::ProcessError error)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    QString errorMsg = "Unknown error";
    
    if (process) {
        errorMsg = process->errorString();
    }
    
    qDebug() << "⚠️ Process Error:" << errorMsg;
    emit executionError(errorMsg);
}