/* run the server before running the app with:
 * cd C:/Users/Mahantesh/DevelopmentProjects/VrindaAI/VrindaAI/llama.cpp/build/bin/Release
llama-server.exe --model mistral.gguf -ngl 99 --parallel 4 --threads 8 --host 127.0.0.1 --port 8080 --ctx-size 4096
*/
/*check health with:
http://127.0.0.1:8080/health
*/
// mainwindow.cpp
// Updated and fixed version for VrindaAI main window
// Key fixes:
//  - Robust prompt sanitization
//  - Do NOT pass prompt as argv; write prompt to stdin
//  - Clear process management per-role
//  - Preserve existing multi-agent logic and routing
//  - Improved debug + error messages

#include "mainwindow.h"
#if __has_include("ui_mainwindow.h")
#include "ui_mainwindow.h"
#else
// Minimal stub for situations where the uic-generated ui_mainwindow.h is not available
// This stub only provides the declarations needed for compilation and IDE diagnostics.
// It intentionally does not implement full UI wiring; the real generated header will be used at build time.
class QTextEdit;
class QPushButton;
class QScrollBar;
class QMainWindow;
class QPlainTextEdit;

namespace Ui {
class MainWindow {
public:
    // Chat areas
    QTextEdit *CoderChatArea = nullptr;
    QTextEdit *DesignerChatArea = nullptr;
    QTextEdit *ManagerChatArea = nullptr;
    QTextEdit *IntegratorChatArea = nullptr;
    QTextEdit *AssistantChatArea = nullptr;
    QTextEdit *EngineChatArea = nullptr;
    QTextEdit *EditingChatArea = nullptr;
    QTextEdit *ResearcherChatArea = nullptr;
    QTextEdit *ArchitectChatArea = nullptr;
    QTextEdit *VrindaChatArea = nullptr;
    QTextEdit *PlannerChatArea = nullptr;
    QTextEdit *CorrectorChatArea = nullptr;
    QTextEdit *ModelingChatArea = nullptr;
    QTextEdit *TexturingChatArea = nullptr;
    QTextEdit *AnimationChatArea = nullptr;
    QTextEdit *ValidatorChatArea = nullptr;
    QTextEdit *ScripterChatArea = nullptr;
    QTextEdit *CompilerChatArea = nullptr;

    // Input boxes
    QTextEdit *CoderTextEdit = nullptr;
    QTextEdit *DesignerTextEdit = nullptr;
    QTextEdit *ManagerTextEdit = nullptr;
    QTextEdit *IntegratorTextEdit = nullptr;
    QTextEdit *AssistantTextEdit = nullptr;
    QTextEdit *EngineTextEdit = nullptr;
    QTextEdit *EditingTextEdit = nullptr;
    QTextEdit *ResearcherTextEdit = nullptr;
    QTextEdit *ArchitectTextEdit = nullptr;
    QTextEdit *VrindaTextEdit = nullptr;
    QTextEdit *PlannerTextEdit = nullptr;
    QTextEdit *CorrectorTextEdit = nullptr;
    QTextEdit *ModelingTextEdit = nullptr;
    QTextEdit *TexturingTextEdit = nullptr;
    QTextEdit *AnimationTextEdit = nullptr;
    QTextEdit *ValidatorTextEdit = nullptr;
    QTextEdit *ScripterTextEdit = nullptr;
    QTextEdit *CompilerTextEdit = nullptr;

    // Buttons
    QPushButton *pdfExportButton = nullptr;
    QPushButton *ExportButton = nullptr;
    QPushButton *VrindaSendButton = nullptr;
    QPushButton *ArchitectSendButton = nullptr;
    QPushButton *CoderSendButton = nullptr;
    QPushButton *DesignerSendButton = nullptr;
    QPushButton *ManagerSendButton = nullptr;
    QPushButton *IntegratorSendButton = nullptr;
    QPushButton *AssistantSendButton = nullptr;
    QPushButton *EditingSendButton = nullptr;
    QPushButton *ResearcherSendButton = nullptr;
    QPushButton *EngineSendButton = nullptr;

    // Minimal setup hooks
    void setupUi(QMainWindow * /*MainWindow*/) {}
    void retranslateUi(QMainWindow * /*MainWindow*/) {}
};
} // namespace Ui
#endif

// General Qt Utilities
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

// File System
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QTextStream>
#include <QProcess>


// UI
#include <QMessageBox>
#include <QTextEdit>
#include <QSettings>
#include <QApplication>
#include <QPushButton>

#ifdef QT_NO_DEBUG
QString basePath = QCoreApplication::applicationDirPath();
#else
QString basePath = "C:/Users/Mahantesh/DevelopmentProjects/VrindaAI/VrindaAI";
#endif

// Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_dbManager(nullptr), // <-- Initialize service pointers to null
    m_reportGenerator(nullptr)
{
    ui->setupUi(this);


    // --- INSTANTIATE ONLY THE CORE CONTROLLERS/CLIENTS HERE ---
    m_projectManager = new ProjectManager(basePath, this);
    m_projectWorkflow = new ProjectWorkflow(basePath, roleBuffers, this);
    m_blenderController = new BlenderController(basePath, this);
    m_unrealController = new UnrealController(this);
    m_modelManager = new ModelManager(this);
    m_compilerController = new CompilerController(this);
    m_vectorDbManager = new VectorDatabaseManager(this);
    m_projectStateController = new ProjectStateController(this);
    m_modelingController = new ModelingController(basePath, this);
    m_texturingController = new TexturingController(basePath, this);
    m_animationController = new AnimationController(basePath, this);
    m_validatorController = new ValidatorController(basePath, this);
    m_ffmpegController = new FfmpegController(basePath, this);
    m_networkManager = new QNetworkAccessManager(this);

    // --- START: NEW Global Logger Initialization ---
    // These loggers are for agents that work outside of a project context, like 'Vrinda'.
    m_globalDbManager = new DatabaseManager(basePath, this);
    m_globalReportGenerator = new ReportGenerator(basePath, m_globalDbManager, this);
    if (m_globalDbManager->setupDatabase()) {
        // Connect the global logger to the global file saver
        connect(m_globalDbManager, &DatabaseManager::taskLogged,
                m_globalReportGenerator, &ReportGenerator::saveLogToFile);
    }

    // map chat boxes
    roleToChatBox["Coder"] = ui->CoderChatArea;
    roleToChatBox["Designer"] = ui->DesignerChatArea;
    roleToChatBox["Manager"] = ui->ManagerChatArea;
    roleToChatBox["Integrator"] = ui->IntegratorChatArea;
    roleToChatBox["Assistant"] = ui->AssistantChatArea;
    roleToChatBox["Engine"] = ui->EngineChatArea;
    roleToChatBox["Editing"] = ui->EditingChatArea;
    roleToChatBox["Researcher"] = ui->ResearcherChatArea;
    roleToChatBox["Architect"] = ui->ArchitectChatArea;
    roleToChatBox["Vrinda"] = ui->VrindaChatArea;
    roleToChatBox["Planner"] = ui->PlannerChatArea;
    roleToChatBox["Corrector"] = ui->CorrectorChatArea;
    roleToChatBox["Modeling"] = ui->ModelingChatArea;
    roleToChatBox["Texturing"] = ui->TexturingChatArea;
    roleToChatBox["Animation"] = ui->AnimationChatArea;
    roleToChatBox["Validator"] = ui->ValidatorChatArea;
    roleToChatBox["Scripter"] = ui->ScripterChatArea;
    roleToChatBox["Compiler"] = ui->CompilerChatArea;


    // map input boxes
    roleToInputBox["Coder"] = ui->CoderTextEdit;
    roleToInputBox["Designer"] = ui->DesignerTextEdit;
    roleToInputBox["Manager"] = ui->ManagerTextEdit;
    roleToInputBox["Integrator"] = ui->IntegratorTextEdit;
    roleToInputBox["Assistant"] = ui->AssistantTextEdit;
    roleToInputBox["Engine"] = ui->EngineTextEdit;
    roleToInputBox["Editing"] = ui->EditingTextEdit;
    roleToInputBox["Researcher"] = ui->ResearcherTextEdit;
    roleToInputBox["Architect"] = ui->ArchitectTextEdit;
    roleToInputBox["Vrinda"] = ui->VrindaTextEdit;
    roleToInputBox["Planner"] = ui->PlannerTextEdit;
    roleToInputBox["Corrector"] = ui->CorrectorTextEdit;
    roleToInputBox["Modeling"] = ui->ModelingTextEdit;
    roleToInputBox["Texturing"] = ui->TexturingTextEdit;
    roleToInputBox["Animation"] = ui->AnimationTextEdit;
    roleToInputBox["Validator"] = ui->ValidatorTextEdit;
    roleToInputBox["Scripter"] = ui->ScripterTextEdit;
    roleToInputBox["Compiler"] = ui->CompilerTextEdit;


    // Try apply theme
    QString stylePath = basePath + "/styles/darktheme.qss";
    QFile styleFile(stylePath);
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        qApp->setStyleSheet(styleSheet);
        qDebug() << "✅ Dark theme applied from:" << stylePath;
        styleFile.close();
    } else {
        qDebug() << "❌ Failed to load darktheme.qss from:" << stylePath;
    }


    // --- ADD NEW CONNECTIONS FOR MODELMANAGER ---
    connect(m_modelManager, &ModelManager::responseReady, this, &MainWindow::onLlamaResponse);
    connect(m_modelManager, &ModelManager::requestFailed, this, &MainWindow::onLlamaError);

    // --- NEW SERVER CONTROLLER INSTANTIATION ---
    m_llamaServerController = new LlamaServerController(basePath, this);

    // Connect server messages to a debug window or the Manager/System chat area
    connect(m_llamaServerController, &LlamaServerController::serverStatus, this, [=](const QString& msg){
        qDebug() << msg; // Log status to debug output
        // Optionally: ui->ManagerChatArea->append(msg);
    });
    connect(m_llamaServerController, &LlamaServerController::serverError, this, [=](const QString& msg){
        qDebug() << msg;
        // Optionally: ui->ManagerChatArea->append(msg);
    });


    // START ALL SERVERS AUTOMATICALLY
    //m_llamaServerController->startAllServers();

    // Pass the controller instance to the ModelManager so it can execute swaps
    m_modelManager->setServerController(m_llamaServerController);

    // --- HEALTH CHECK POLLER ---
    // Connect ModelManager's startHealthCheck signal to a poller that checks the
    // /health endpoint and invokes the provided callback when the server is ready.
    connect(m_modelManager, &ModelManager::startHealthCheck, this, [this](int port, std::function<void()> onReadyCallback) {
        qDebug() << "MAINWINDOW: Received Health Check request for port" << port;

        // Use a pointer to track if we've already finished to avoid double-calling
        auto alreadyTriggered = std::make_shared<bool>(false);

        QTimer* pollTimer = new QTimer(this);
        pollTimer->setInterval(2000); // Check every 2 seconds

        connect(pollTimer, &QTimer::timeout, this, [this, port, pollTimer, onReadyCallback, alreadyTriggered]() {
            if (*alreadyTriggered) return;

            QString url = QString("http://127.0.0.1:%1/health").arg(port);
            QNetworkRequest req((QUrl(url)));
            
            // Use the main network manager to ensure the request survives the lambda scope
            QNetworkReply *reply = m_networkManager->get(req);

            connect(reply, &QNetworkReply::finished, this, [this, reply, pollTimer, onReadyCallback, alreadyTriggered, port]() {
                if (*alreadyTriggered) {
                    reply->deleteLater();
                    return;
                }

                bool isReady = false;
                if (reply->error() == QNetworkReply::NoError) {
                    QByteArray resp = reply->readAll();
                    QString s = QString::fromUtf8(resp).toLower();
                    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

                    // Llama.cpp returns {"status": "ok"} or simply 200 OK
                    if (s.contains("ok") || statusCode == 200) {
                        isReady = true;
                    }
                }

                if (isReady) {
                    qDebug() << "HEALTH POLLER: Port" << port << "is READY. Stopping timer.";
                    *alreadyTriggered = true;
                    
                    pollTimer->stop();
                    pollTimer->deleteLater();
                    
                    // Execute the callback in ModelManager to reset m_isCurrentlySwapping
                    if (onReadyCallback) {
                        onReadyCallback();
                    }
                } else {
                    // Optional: Log that we are still waiting
                    qDebug() << "HEALTH POLLER: Port" << port << "not ready yet... retrying.";
                }

                reply->deleteLater();
            });
        });

        pollTimer->start();
    });

    m_vectorDbManager->checkHealthAndProceed([this]() {
        // This lambda contains the original logic that required the server to be running:
        if (!m_pendingGoal.isEmpty()) {
            m_vectorDbManager->queryMemory(m_pendingGoal, 5);
            m_pendingGoal.clear(); // Clear the goal after triggering the query
        }
    });


    // --- ADD NEW CONNECTIONS FOR ModelingController ---
    connect(m_modelingController, &ModelingController::processOutput, this, [=](const QString& output){
        ui->ModelingChatArea->append("⚙️ Model Process: " + output);
    });
    connect(m_modelingController, &ModelingController::processFinished, this, [=](const QString& taskId, const QString& assetPath){
        ui->ModelingChatArea->append("✅ 3D Model Asset Ready: " + assetPath);
        // CRITICAL: Must register the new asset with the ProjectStateController
        // NOTE: The asset ID must be passed back via the controller or the original LLM response
        // For simplicity, we just mark the task finished here. Asset registration should happen inside the controller.
        m_projectWorkflow->taskFinished(taskId);
    });

    // --- ADD NEW CONNECTIONS FOR TexturingController ---
    connect(m_texturingController, &TexturingController::processOutput, this, [=](const QString& output){
        ui->TexturingChatArea->append("⚙️ Texture Process: " + output);
    });
    connect(m_texturingController, &TexturingController::processFinished, this, [=](const QString& taskId, const QString& materialPath){
        ui->TexturingChatArea->append(QString("✅ PBR Material Manifest Ready: %1").arg(materialPath));
        // The successful creation of the material allows the workflow to proceed.
        m_projectWorkflow->taskFinished(taskId);
    });

    // --- ADD NEW CONNECTIONS FOR AnimationController ---
    connect(m_animationController, &AnimationController::processOutput, this, [=](const QString& output){
        ui->AnimationChatArea->append("⚙️ Animation Process: " + output);
    });
    connect(m_animationController, &AnimationController::processFinished, this, [=](const QString& taskId, const QString& assetPath){
        ui->AnimationChatArea->append(QString("✅ Animation Asset Ready: %1").arg(assetPath));
        m_projectWorkflow->taskFinished(taskId);
    });

    // --- ADD NEW CONNECTIONS FOR ValidatorController ---
    connect(m_validatorController, &ValidatorController::validationOutput, this, [=](const QString& output){
        ui->ValidatorChatArea->append("🔍 Validation Check: " + output);
    });
    connect(m_validatorController, &ValidatorController::validationFinished, this, [=](const QString& taskId, bool success, const QString& summary){
        if (success) {
            ui->ValidatorChatArea->append("✅ Validation Successful! Workflow continues.");
            m_projectWorkflow->taskFinished(taskId);
        } else {
            ui->ValidatorChatArea->append("❌ Validation Failed. Reason: " + summary);
            // CRITICAL: Fail the workflow task so the Manager/Corrector can intervene.
            m_projectWorkflow->taskFailed(taskId, "Validation failed: " + summary);
        }
    });

    // --- ADD NEW CONNECTIONS FOR FfmpegController (Editing Agent) ---
    connect(m_ffmpegController, &FfmpegController::processOutput, this, [=](const QString& output){
        ui->EditingChatArea->append("🎬 FFmpeg Process: " + output);
    });
    connect(m_ffmpegController, &FfmpegController::processFinished, this, [=](const QString& taskId, const QString& outputPath){
        if (outputPath == "SUCCESS") {
            ui->EditingChatArea->append("✅ Video Editing Complete (FFmpeg).");
            m_projectWorkflow->taskFinished(taskId);
        } else {
            ui->EditingChatArea->append("❌ Video Editing Failed (FFmpeg). Check console for error.");
            m_projectWorkflow->taskFailed(taskId, "Video editing/composition failed (FFmpeg).");
        }
    });

    // --- ADD NEW CONNECTIONS FOR BlenderController ---
    connect(m_blenderController, &BlenderController::blenderOutput, this, [=](const QString& output){
        ui->DesignerChatArea->append(output);
    });
    connect(m_blenderController, &BlenderController::blenderError, this, [=](const QString& error){
        ui->DesignerChatArea->append(error);
    });
    connect(m_blenderController, &BlenderController::blenderFinished, this, [=](int exitCode){
        ui->DesignerChatArea->append(QString("✅ Blender script finished with exit code %1").arg(exitCode));
    });
    connect(m_blenderController, &BlenderController::assetReadyForEngine, this, [=](const QString& assetPath){
        roleBuffers["Designer_output"] = assetPath;
        ui->EngineTextEdit->setPlainText("Import: " + assetPath);
    });

    // --- ADD NEW CONNECTIONS FOR ProjectWORKFLOW ---
    connect(m_projectWorkflow, &ProjectWorkflow::workflowMessage, this, [=](const QString& message){
        ui->ManagerChatArea->append(message);
    });

    connect(m_projectWorkflow, &ProjectWorkflow::workflowFinished, this, [=](const QString& finalMessage){
        ui->ManagerChatArea->append(finalMessage);
    });

    connect(m_projectWorkflow, &ProjectWorkflow::assignTaskToAgent, this, &MainWindow::onAssignTask);

    connect(m_projectWorkflow, &ProjectWorkflow::escalateToManager, this, &MainWindow::onEscalateToCorrector);

    // --- ADD NEW CONNECTIONS FOR UnrealController ---
    connect(m_unrealController, &UnrealController::unrealOutput, this, [=](const QString& output){
        ui->EngineChatArea->append(output);
    });
    connect(m_unrealController, &UnrealController::unrealError, this, [=](const QString& error){
        ui->EngineChatArea->append(error);
    });


    // --- ADD NEW CONNECTIONS FOR CompilerController ---
    connect(m_compilerController, &CompilerController::compilationOutput, this, [=](const QString& output){
        // We'll route compiler output to the Integrator's chat box
        ui->IntegratorChatArea->append(output.trimmed());
    });

    connect(m_compilerController, &CompilerController::compilationFinished, this, &MainWindow::onCompilationFinished);


    // --- ADD NEW CONNECTIONS FOR VectorDatabaseManager ---
    connect(m_vectorDbManager, &VectorDatabaseManager::queryResult, this, &MainWindow::onMemoryQueryResult);

    connect(m_vectorDbManager, &VectorDatabaseManager::memoryAddedSuccessfully, this, [=](){
        qDebug() << "🧠 Memory added successfully to Vector DB.";
    });

    connect(m_vectorDbManager, &VectorDatabaseManager::memoryError, this, [=](const QString &error){
        qDebug() << "❌ Vector DB Error:" << error;
        // Optionally, show a non-intrusive error to the user in a status bar or log
    });

    // Connect UI signals
    connect(ui->pdfExportButton, &QPushButton::clicked, this, [=]() {
        QString defaultPath = m_activeProjectPath + "/reports/tasks_report";   //make the path end with .pdf if error occurs in saving else perfect
        QString path = QFileDialog::getSaveFileName(this, "Save Report As PDF", defaultPath, "PDF Files (*.pdf)");
        if (!path.isEmpty()) {
            m_reportGenerator->exportTaskReportAsPDF(QFileInfo(path).fileName());
        }
    });

    connect(ui->ExportButton, &QPushButton::clicked, this, [=]() {
        QString filename = "task_history.csv";
        m_reportGenerator->exportTasksToCSV(filename);
        QString filePath = m_activeProjectPath + "/reports/" + filename;
        ui->AssistantChatArea->append("📥 Task history exported to: " + filePath);
    });

    connect(ui->VrindaSendButton, &QPushButton::clicked, this, [=]() {
        // This agent is standalone and doesn't interact with the project workflow.
        ui->VrindaChatArea->setReadOnly(true);
        ui->VrindaChatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        handleAgent("", "Vrinda", ui->VrindaTextEdit, ui->VrindaChatArea);
    });

    // --- You may not need a send button for the Architect, as it's triggered by the 'load' command ---
    // But if you want to be able to send it manual commands, you would add its connect block here:
    connect(ui->ArchitectSendButton, &QPushButton::clicked, this, [=]() {
        ui->ArchitectChatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        handleAgent("", "Architect", ui->ArchitectTextEdit, ui->ArchitectChatArea);
    });


    // role send buttons
    connect(ui->CoderSendButton, &QPushButton::clicked, this, [=]() {
        ui->CoderChatArea->setReadOnly(true);
        ui->CoderChatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        QStringList logs = m_dbManager->loadAgentLogs("Coder");
        ui->CoderChatArea->clear(); // Optional: clear the chat before loading history
        ui->CoderChatArea->append(logs.join("\n"));
        handleAgent("", "Coder", ui->CoderTextEdit, ui->CoderChatArea);
    });

    connect(ui->DesignerSendButton, &QPushButton::clicked, this, [=]() {
        ui->DesignerChatArea->setReadOnly(true);
        ui->DesignerChatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        QStringList logs = m_dbManager->loadAgentLogs("Designer");
        ui->DesignerChatArea->clear(); // Optional: clear the chat before loading history
        ui->DesignerChatArea->append(logs.join("\n"));
        handleAgent("", "Designer", ui->DesignerTextEdit, ui->DesignerChatArea);
    });

    connect(ui->ManagerSendButton, &QPushButton::clicked, this, [=]() {
        ui->ManagerChatArea->setReadOnly(true);
        ui->ManagerChatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        QStringList logs = m_dbManager->loadAgentLogs("Manager");
        ui->ManagerChatArea->clear(); // Optional: clear the chat before loading history
        ui->ManagerChatArea->append(logs.join("\n"));
        handleAgent("", "Manager", ui->ManagerTextEdit, ui->ManagerChatArea);
        ui->ManagerTextEdit->clear();
    });

    connect(ui->IntegratorSendButton, &QPushButton::clicked, this, [=]() {
        ui->IntegratorChatArea->setReadOnly(true);
        ui->IntegratorChatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        // gather today's logs from roles
        QString date = QDate::currentDate().toString("yyyy-MM-dd");

        QString coderLogPath = m_activeProjectPath + "/logs/coder_log" + ".txt";
        QString designerLogPath = m_activeProjectPath + "/logs/designer_log" + ".txt";
        QString engineLogPath = m_activeProjectPath + "/logs/engine_log" + ".txt";
        QString editingLogPath = m_activeProjectPath + "/logs/editing_log" + ".txt";
        QString researcherLogPath = m_activeProjectPath + "/logs/researcher_log" + ".txt";


        QString coderText = "[Coder logs not found for " + coderLogPath + "]";
        QString designerText = "[Designer logs not found for " + designerLogPath + "]";
        QString engineText = "[Engine logs not found for " + engineLogPath + "]";
        QString editingText = "[Editing logs not found for " + editingLogPath + "]";
        QString researcherText = "[Researcher logs not found for " + researcherLogPath + "]";

        QFile t1(coderLogPath);
        if (t1.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&t1);
            coderText = in.readAll();
            t1.close();
        }
        QFile t2(designerLogPath);
        if (t2.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&t2);
            designerText = in.readAll();
            t2.close();
        }
        QFile t3(engineLogPath);
        if (t3.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&t3);
            engineText = in.readAll();
            t3.close();
        }
        QFile t4(editingLogPath);
        if (t4.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&t4);
            editingText = in.readAll();
            t4.close();
        }

        QString integrated = "=== 👨‍💻 CODE ===\n" + coderText + "\n\n=== 🎨 DESIGN ===\n" + designerText
                             + "\n\n=== 👨‍💻 ENGINE ===\n" + engineText + "\n\n=== 🎨 EDITING ===\n" + editingText;
        ui->IntegratorChatArea->append(integrated);
        QStringList logs = m_dbManager->loadAgentLogs("Integrator");
        ui->IntegratorChatArea->clear(); // Optional: clear the chat before loading history
        ui->IntegratorChatArea->append(logs.join("\n"));
        handleAgent("", "Integrator", ui->IntegratorTextEdit, ui->IntegratorChatArea);
    });


    connect(ui->AssistantSendButton, &QPushButton::clicked, this, [=]() {
        QString input = ui->AssistantTextEdit->toPlainText().trimmed();
        if (input.isEmpty()) return;

        // If a workflow is running, send the input to the ASSISTANT for reformatting.
        if (m_projectWorkflow->isRunning()) {
            ui->AssistantChatArea->append("[Intervention] Asking Assistant to reformat new instructions...");

            // Set a flag so onLlamaResponse knows this is an intervention goal
            roleBuffers["InterventionInProgress"] = "true";

            // Create a prompt for the Assistant to do its job
            QString assistantPrompt = "The user has provided new instructions to change the current project's direction. "
                                      "Rephrase these instructions into a concise, high-level objective with key requirements. "
                                      "Your response MUST be ONLY a goal description enclosed within [GOAL] and [END GOAL] tags.\n\n"
                                      "New Instructions: " + input;

            QTextEdit* tempInput = new QTextEdit(assistantPrompt, this);
            handleAgent("", "Assistant", tempInput, ui->AssistantChatArea);
            ui->AssistantTextEdit->clear();
            return; // Stop further processing
        }

        // --- START: CORRECTED AND FINALIZED LOGIC ---
        if (input.startsWith("create", Qt::CaseInsensitive)) {
            // This is a new project. First, query the memory for relevant context.
            ui->AssistantChatArea->append("🔍 Searching long-term memory for relevant context...");
            m_pendingGoal = input; // Store the goal
            m_vectorDbManager->queryMemory(input); // This will trigger onMemoryQueryResult when done
            ui->AssistantTextEdit->clear();
            return;
        } else if (input.startsWith("load", Qt::CaseInsensitive)) {
            // User wants to load and analyze an existing project.
            QString projectName = input.mid(4).trimmed();
            QString existingProjectPath = m_projectManager->findExistingProject(projectName);

            if (!existingProjectPath.isEmpty()) {
                m_projectManager->loadProject(existingProjectPath);
                m_activeProjectPath = existingProjectPath;
                initializeServicesForProject(m_activeProjectPath);
                ui->AssistantChatArea->append("✅ Project '" + QFileInfo(existingProjectPath).baseName() + "' loaded. Starting analysis...");

                // 1. Scan the project directory
                CodeScanner scanner;
                QString projectContext = scanner.summarizeProject(m_activeProjectPath);

                // 2. Prepare and send the context to the Architect agent
                QTextEdit* architectInput = roleToInputBox.value("Architect");
                if(architectInput) {
                    architectInput->setPlainText("Analyze the following project codebase and provide a summary:\n" + projectContext);
                    handleAgent("", "Architect", architectInput, roleToChatBox.value("Architect"));
                } else {
                    ui->AssistantChatArea->append("❌ Architect agent UI not found. Cannot analyze project.");
                }
            } else {
                ui->AssistantChatArea->append("❌ Project '" + projectName + "' not found.");
            }
        } else {
            // This is a follow-up command for an already loaded project.
            if (!m_projectManager->isProjectLoaded()) {
                ui->AssistantChatArea->append("❌ No project loaded. Please start with 'create' or 'load'.");
                return;
            }

            // --- FIX: For any follow-up creative task, treat it as a new goal ---
            // Set the buffer that triggers the special GOAL formatting in handleAgent
            roleBuffers["PendingGoal"] = input;
            handleAgent("", "Assistant", ui->AssistantTextEdit, ui->AssistantChatArea);
        }
    });

    connect(ui->EditingSendButton, &QPushButton::clicked, this, [=]() {
        ui->EditingChatArea->setReadOnly(true);
        ui->EditingChatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        if (roleBuffers.contains("Engine_output")) {
            ui->EditingTextEdit->setPlainText("Edit: " + roleBuffers["Engine_output"]);
            roleBuffers.remove("Engine_output");
        }

        // Removed redundant log loading here, relying on handleAgent or logs being pre-loaded.
        // It's crucial to call handleAgent now:
        handleAgent("", "Editing", ui->EditingTextEdit, ui->EditingChatArea);
    });

    connect(ui->ResearcherSendButton, &QPushButton::clicked, this, [=]() {
        ui->ResearcherChatArea->setReadOnly(true);
        ui->ResearcherChatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        if (roleBuffers.contains("Researcher_task")) {
            ui->ResearcherTextEdit->setPlainText(roleBuffers["Researcher_task"]);
            roleBuffers.remove("Researcher_task");
        }

        QStringList logs = m_dbManager->loadAgentLogs("Researcher");
        ui->ResearcherChatArea->clear(); // Optional: clear the chat before loading history
        ui->ResearcherChatArea->append(logs.join("\n"));
        handleAgent("", "Researcher", ui->ResearcherTextEdit, ui->ResearcherChatArea);
    });

    connect(ui->EngineSendButton, &QPushButton::clicked, this, [=]() {
        ui->EngineChatArea->setReadOnly(true);
        ui->EngineChatArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        QString promptPath = basePath + "/prompts/Engine.txt";
        if (!QFile::exists(promptPath)) {
            createDefaultEnginePrompt();
        }

        QStringList logs = m_dbManager->loadAgentLogs("Engine");
        ui->EngineChatArea->clear(); // Optional: clear the chat before loading history
        ui->EngineChatArea->append(logs.join("\n"));
        handleAgent("", "Engine", ui->EngineTextEdit, ui->EngineChatArea);
    });

    // --- START: NEW Unreal Path Loading Logic ---
    QSettings settings("VrindaAI", "VrindaAI");
    QString unrealPath = settings.value("unrealEditorPath", "").toString();
    if (!unrealPath.isEmpty()) {
        m_unrealController->setUnrealEditorPath(unrealPath);
        qDebug() << "✅ Loaded Unreal Editor path from settings:" << unrealPath;
    } else {
        qDebug() << "⚠️ Unreal Editor path not set. Please configure it.";
        // Optionally, you could show a message to the user here
        ui->EngineChatArea->append("⚠️ Unreal Editor path not set. Please configure it in the application settings.");
    }

    qDebug() << "VrindaAI mainwindow started. basePath=" << basePath;
    // Clear the input box immediately after capturing the text
    ui->AssistantTextEdit->clear();
}

void MainWindow::onMemoryQueryResult(const QList<MemoryQueryResult> &results)
{
    if (m_pendingGoal.isEmpty()) return;

    // Step 1: Create the project workspace.
    QString newProjectPath = m_projectManager->createNewProject(m_pendingGoal);
    m_projectManager->loadProject(newProjectPath);
    m_activeProjectPath = newProjectPath;
    initializeServicesForProject(m_activeProjectPath);

    // --- THIS IS THE FIX: Announce the new project name in the UI ---
    QString newProjectName = QFileInfo(newProjectPath).baseName();
    ui->AssistantChatArea->append(QString("✅ Project '%1' created successfully in your projects folder.").arg(newProjectName));


    // Step 2: Build the prompt for the Planner agent
    QString plannerPrompt = "PRIMARY GOAL:\n";
    plannerPrompt += m_pendingGoal + "\n";

    QStringList relevantMemories;
    const double RELEVANCE_THRESHOLD = 1.0;

    for (const MemoryQueryResult &res : results) {
        if (res.distance < RELEVANCE_THRESHOLD) {
            relevantMemories.append(res.document);
        }
    }

    if (!relevantMemories.isEmpty()) {
        ui->AssistantChatArea->append("✅ Found relevant memories. Adding them to the Planner's context.");
        plannerPrompt += "\nREFERENCE MEMORIES (For context only, do not repeat these tasks):\n";
        plannerPrompt += "- " + relevantMemories.join("\n- ");
    } else {
        ui->AssistantChatArea->append("ℹ️ No sufficiently relevant memories found.");
    }
    plannerPrompt += "\n[END GOAL]";

    // Step 3: Send the prompt to the Planner agent
    ui->AssistantChatArea->append("[System] Goal sent to Planner agent for initial breakdown...");
    QTextEdit* plannerInput = roleToInputBox.value("Planner");
    if (plannerInput) {
        plannerInput->setPlainText(plannerPrompt);
        handleAgent("", "Planner", plannerInput, roleToChatBox.value("Planner"));
    } else {
        qDebug() << "❌ UI for Planner agent not found!";
    }

    m_pendingGoal.clear();
}

// --- Initialize Services for Projects ---
void MainWindow::initializeServicesForProject(const QString &projectPath)
{
    // Clean up old services if they exist
    if (m_dbManager) m_dbManager->deleteLater();
    if (m_reportGenerator) m_reportGenerator->deleteLater();

    // Create new instances with the correct project path
    m_dbManager = new DatabaseManager(projectPath, this);
    m_reportGenerator = new ReportGenerator(projectPath, m_dbManager, this);

    m_blenderController->setActiveProjectPath(projectPath);
    m_projectStateController->loadManifest(projectPath);

    // Initialize the new database
    if (m_dbManager->setupDatabase()) {
        // Connect the logging signal for the new instances
        connect(m_dbManager, &DatabaseManager::taskLogged, m_reportGenerator, &ReportGenerator::saveLogToFile);
    }

    setWindowTitle("VrindaAI - " + QFileInfo(projectPath).baseName());
}

// This function is a slot that you will connect to a "Set Unreal Path" button in your UI.
void MainWindow::on_setUnrealPathButton_clicked() // Name this to match your button objectName
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Select Unreal Editor Executable"), "", tr("Executable Files (UnrealEditor.exe)"));

    if (!filePath.isEmpty()) {
        m_unrealController->setUnrealEditorPath(filePath);

        QSettings settings("VrindaAI", "VrindaAI");
        settings.setValue("unrealEditorPath", filePath);

        qDebug() << "✅ Set and saved new Unreal Editor path:" << filePath;
        ui->EngineChatArea->append("✅ Unreal Editor path has been set to: " + filePath);
    }
}


/* -------------------------
   Core handler (prepares prompt and starts llama-server)
   ------------------------- */
void MainWindow::handleAgent(const QString &taskId, const QString &role, QTextEdit *inputBox, QTextEdit *chatBox)
{
    QString input = inputBox->toPlainText().trimmed();
    if (!input.isEmpty()) {
        inputBox->clear();
    } else {
        input = roleBuffers.value(role + "_task", "");
    }

    if (input.isEmpty()) return;

    roleBuffers[role + "_task"] = input;
    roleBuffers[role + "_taskId"] = taskId;

    // --- This is the context injection logic you already have, which is correct ---
    QString contextBlock;
    QString cleanedInput = input;
    QRegularExpression contextRegex(R"(\[CONTEXT:\s*([^\]]+)\])");
    auto it = contextRegex.globalMatch(input);
    while (it.hasNext()) {
        auto match = it.next();
        QString pathArg = match.captured(1).trimmed();
        QString fullPath;
        if (pathArg.contains('/')) {
            fullPath = m_projectManager->getBaseProjectsPath() + "/" + pathArg;
        } else {
            fullPath = m_activeProjectPath + "/" + pathArg;
        }
        QFile contextFile(fullPath);
        if (contextFile.exists() && contextFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&contextFile);
            contextBlock += QString("--- START OF CONTEXT FILE: %1 ---\n%2\n--- END OF CONTEXT FILE: %1 ---\n\n").arg(pathArg, in.readAll());
            cleanedInput.remove(match.captured(0));
        } else {
            chatBox->append("⚠️ Context file not found: " + pathArg);
        }
    }
    cleanedInput = cleanedInput.trimmed();

    // --- The obsolete 'if (role == "manager")' block that was here has been REMOVED ---
    // This is a key part of the fix. This function no longer needs to validate the Manager's input.

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    DatabaseManager* logger = (role == "Vrinda" || !m_projectManager->isProjectLoaded()) ? m_globalDbManager : m_dbManager;
    if (logger) {
        // This now correctly logs which model is ABOUT to be used for the task
        QString modelToUse = m_modelManager->getModelForRole(role);
        logger->logTask(role, input, "", "Task Dispatched", modelToUse);
    }
    chatBox->append("[" + timestamp + "] You: " + input);

    QString promptFile = QDir(basePath).filePath("prompts/" + role + ".txt");
    QFile file(promptFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        chatBox->append("❌ Prompt file not found: " + promptFile);
        qDebug() << "Prompt file missing for role:" << role << " at:" << promptFile;
        return;
    }
    QString basePrompt = QString::fromUtf8(file.readAll()).trimmed();
    file.close();

    // --- THIS IS THE SECOND KEY FIX ---
    // We now correctly construct the final prompt using the context we gathered.
    QString finalPrompt = basePrompt;
    if (!contextBlock.isEmpty()) {
        finalPrompt += "\n\n### Provided Context Files\n" + contextBlock;
    }
    // We use 'cleanedInput', which has the [CONTEXT] tags removed.
    finalPrompt += "\n\nUser: " + cleanedInput + "\nAssistant:";
    finalPrompt += "\n\n### END OF PROMPT ###\n";

    m_modelManager->sendRequest(taskId, role, finalPrompt);
}

void MainWindow::onLlamaError(const QString &role, const QString &errorString)
{
    QTextEdit* chatBox = roleToChatBox.value(role, nullptr);
    if (chatBox) {
        chatBox->append("❌ Network request failed for role " + role + ": " + errorString);
    }
    qDebug() << "MODELMANAGER Error for role" << role << ":" << errorString;
}

void MainWindow::onLlamaResponse(const QString &taskId, const QString &role, const QString &response, const QString &modelUsed)
{
    qDebug() << "========================================";
    qDebug() << "DEBUG: onLlamaResponse triggered for ROLE:" << role << "TASK_ID:" << taskId;
    QTextEdit* chatBox = roleToChatBox.value(role, nullptr);
    if (!chatBox) {
        qDebug() << "DEBUG: ERROR - Could not find chatBox for role:" << role;
        return;
    }
    qDebug() << "DEBUG: Response text starts with:" << response.left(120);
    qDebug() << "========================================";

    QString t = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    if (response.isEmpty()) {
        chatBox->append("[" + t + "] ⚠️ " + role + " produced no output.");
        m_projectWorkflow->taskFailed(taskId, "Agent produced an empty response.");
        return;
    }
    chatBox->append("[" + t + "] 🧑 " + role + ": " + response);

    QString originalTask = roleBuffers.value(role + "_task", "[Task not found]");
    DatabaseManager* logger = (role == "Vrinda" || !m_projectManager->isProjectLoaded()) ? m_globalDbManager : m_dbManager;

    if (role == "Planner") {
        // The Planner has returned a plain-text plan. Now, send it to the Manager for JSON formatting.
        chatBox->append("[System] Plain-text plan received. Sending to Manager for JSON formatting...");
        QString managerPrompt = "Please convert the following plan to the required JSON format:\n" + response;
        QTextEdit* managerInput = new QTextEdit(managerPrompt, this);
        handleAgent("", "Manager", managerInput, roleToChatBox["Manager"]);

    } else if (role == "Manager") {
        if (logger) logger->logTask(role, originalTask, response, "Plan Generated", "mistral-7B");
        m_projectWorkflow->startWorkflowFromPlan(response); // This should now always work
    } else if (role == "Corrector") {
        if (logger) logger->logTask(role, originalTask, response, "Plan Corrected", "mistral-7B");
        // The Corrector's response is a modification object.
        QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject root = doc.object();
        if (root.contains("modification")) {
            m_projectWorkflow->applyPlanModification(root["modification"].toObject());
        } else {
            chatBox->append("❌ Corrector Error: JSON response was missing a 'modification' key.");
        }
    } else if (role == "Engine") {
        if (logger) logger->logTask(role, originalTask, response, "Engine Command Sequence", "mistral-7B");
        QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
        if (doc.isNull() || !doc.isArray()) {
            ui->EngineChatArea->append("❌ Engine Error: Received an invalid JSON response from AI (expected an array).");
            m_projectWorkflow->taskFailed(taskId, "Engine agent returned malformed JSON.");
        } else {
            QJsonArray instructions = doc.array();
            m_unrealController->executeInstructionSequence(m_activeProjectPath, instructions);
            m_projectWorkflow->taskFinished(taskId);
        }
    } else if (role == "Validator") {
        if (logger) logger->logTask(role, originalTask, response, "Validation Complete", modelUsed);

        QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
        if (doc.isObject() && doc.object().contains("valid")) {
            bool valid = doc.object()["valid"].toBool();
            QString artifactId = doc.object()["artifact_id"].toString(); // The ID of the artifact being checked

            if (valid) {
                chatBox->append(QString("✅ Artifact %1 passed validation.").arg(artifactId));
                m_projectWorkflow->taskFinished(taskId);
            } else {
                // --- FIX: Wrap QJsonArray in QJsonDocument before calling toJson() ---
                QJsonArray errorsArray = doc.object()["errors"].toArray();
                QString errors = QJsonDocument(errorsArray).toJson(QJsonDocument::Compact);

                chatBox->append("❌ Artifact failed validation. Errors: " + errors);

                // CRITICAL: Fail the workflow task so the Manager/Corrector can intervene.
                m_projectWorkflow->taskFailed(taskId, "Artifact failed validation: " + errors);
            }
        } else {
            chatBox->append("❌ Validator Error: Invalid JSON response (expected 'valid' key).");
            m_projectWorkflow->taskFailed(taskId, "Validator agent returned malformed JSON.");
        }
    }

    else { // This is for Coder, Designer, Modeling, Texturing, Animation, etc.
        QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
        if (doc.isNull() || !doc.isObject()) {
            chatBox->append("❌ Agent Error: Received an invalid JSON response from " + role);
            m_projectWorkflow->taskFailed(taskId, "Agent returned malformed JSON.");
            return;
        }

        QJsonObject statusObj = doc.object();
        QString status = statusObj.value("status").toString();
        QJsonValue payloadValue = statusObj.value("payload");
        QString payloadString;

        if (payloadValue.isString()) {
            payloadString = payloadValue.toString();
        } else {
            payloadString = QJsonDocument::fromVariant(payloadValue.toVariant()).toJson(QJsonDocument::Compact);
        }

        if (status == "success") {
            if (logger) logger->logTask(role, originalTask, payloadString, "Task Succeeded", modelUsed);

            // --- DELEGATE TO SPECIALIZED CONTROLLERS FOR HEAVY TASKS ---
            if (role == "Modeling") {
                // Modeling agent gives a script or command array for the ModelingController
                m_modelingController->executeModelingCommand(taskId, payloadString);
            } else if (role == "Texturing") {
                // Texturing agent gives a description/command array for the TexturingController
                m_texturingController->executeTexturingCommand(taskId, payloadString);
            } else if (role == "Animation") {
                // Animation agent gives a description/command array for the AnimationController
                m_animationController->executeAnimationCommand(taskId, payloadString);
            } else if (role == "Validator") {
                // Validator agent provides the checklist/script; the controller executes the file/code checks
                m_validatorController->executeValidationCommand(taskId, payloadString);
            } else if (role == "Editing") { // <<< NEW DELEGATION FOR EDITING
                // Editing agent gives a structured manifest for the FfmpegController
                m_ffmpegController->executeEditingCommand(taskId, payloadString);
            }
            else {
                // For Coder, Designer (UI/Docs), Researcher, Integrator, etc., save output to file
                saveAgentOutputToFile(taskId, role, payloadString);
            }

        } else {
            if (logger) logger->logTask(role, originalTask, payloadString, "Task Failed", modelUsed);
            m_projectWorkflow->taskFailed(taskId, payloadString);
        }
    }

    roleBuffers.remove(role + "_task");
    roleBuffers.remove(role + "_taskId");
}

void MainWindow::saveAgentOutputToFile(const QString &taskId, const QString &role, const QString &payloadString)
{
    QTextEdit* chatBox = roleToChatBox.value(role, nullptr);
    if (!chatBox) {
        qDebug() << "DEBUG: ERROR - Could not find chatBox in saveAgentOutputToFile for role:" << role;
        m_projectWorkflow->taskFailed(taskId, "Internal UI error.");
        return;
    }

    // --- NEW: Parse the payload from the agent ---
    QJsonDocument payloadDoc = QJsonDocument::fromJson(payloadString.toUtf8());
    if (!payloadDoc.isObject()) {
        m_projectWorkflow->taskFailed(taskId, "Agent returned a success status, but the payload was not a valid JSON object.");
        return;
    }
    QJsonObject payloadObj = payloadDoc.object();
    QString responseText = payloadObj.value("file_content").toString();
    QJsonObject assetInfo = payloadObj.value("register_asset").toObject();

    QString responseContent = responseText;
    QString fileExtension = ".txt";
    QRegularExpression re("^\\[FILETYPE: (\\.[\\w]+)\\]\\n?");
    auto match = re.match(responseContent);

    if (match.hasMatch()) {
        fileExtension = match.captured(1);
        responseContent.remove(match.captured(0));
    }

    responseContent = responseContent.trimmed();
    if (responseContent.isEmpty()) {
        chatBox->append("❌ " + role + " Error: Agent returned a success status but the payload was empty.");
        m_projectWorkflow->taskFailed(taskId, "Agent returned empty payload.");
        return;
    }

    // NOTE: Multi-file creation does not yet support asset registration. This can be a future enhancement.
    if (fileExtension == ".json") {
        // Multi-file creation
        QJsonDocument doc = QJsonDocument::fromJson(responseContent.toUtf8());
        if (!doc.isArray()) {
            chatBox->append("❌ " + role + " Error: Received invalid multi-file JSON format (expected an array).");
            m_projectWorkflow->taskFailed(taskId, "Agent returned malformed multi-file JSON.");
            return;
        }
        QJsonArray files = doc.array();
        int successCount = 0;
        for (const QJsonValue &value : files) {
            QJsonObject fileObj = value.toObject();
            if (fileObj.contains("path") && fileObj.contains("content")) {
                QString relativePath = fileObj["path"].toString();
                QString fullFilePath = m_activeProjectPath + "/" + relativePath;
                QDir(QFileInfo(fullFilePath).path()).mkpath(".");
                QFile outFile(fullFilePath);
                if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&outFile);
                    out << fileObj["content"].toString();
                    outFile.close();
                    chatBox->append("✅ Created file via JSON: " + relativePath);
                    successCount++;
                } else {
                    chatBox->append("❌ Failed to create file via JSON: " + relativePath);
                }
            }
        }
        if (successCount > 0 && successCount == files.count()) {
            m_projectWorkflow->taskFinished(taskId);
        } else {
            m_projectWorkflow->taskFailed(taskId, "One or more files failed to save in multi-file operation.");
        }
    } else {
        // Single-file creation
        QString subFolder, specificFileName, finalFileName;
        QRegularExpression fn_re("^\\[FILENAME: ([\\w\\.]+)\\]\\n?");
        auto fn_match = fn_re.match(responseContent);
        if (fn_match.hasMatch()) {
            specificFileName = fn_match.captured(1);
            responseContent.remove(fn_match.captured(0));
        }

        if (role == "Designer") {
            if (fileExtension == ".py") subFolder = "/scripts";
            else if (fileExtension == ".qml" || fileExtension == ".ui") subFolder = "/ui";
            else subFolder = "/docs";
        } else if (role == "Coder") {
            if (fileExtension == ".html" || fileExtension == ".css" || fileExtension == ".js") subFolder = "/frontend";
            else subFolder = "/backend";
        } else if (role == "Researcher") {
            subFolder = "/r&d";
        } else if (role == "Integrator") {
            subFolder = "/";
            if (specificFileName.isEmpty()) specificFileName = "README";
        } else {
            subFolder = "/docs";
        }

        if (!specificFileName.isEmpty()) {
            QFileInfo fileInfo(specificFileName);
            finalFileName = fileInfo.baseName();
            if (fileExtension == ".txt" && !fileInfo.suffix().isEmpty()) {
                fileExtension = "." + fileInfo.suffix();
            }
        } else {
            QString taskDescription = roleBuffers.value(role + "_task", "untitled");
            QString goalSlug = taskDescription.toLower().replace(QRegularExpression(R"([^a-z0-9_])"), "_").left(30);
            finalFileName = "task_" + taskId + "_" + goalSlug;
        }

        QDir projectDir(m_activeProjectPath);
        projectDir.mkpath(subFolder);
        QString filePath = m_activeProjectPath + subFolder + "/" + finalFileName + fileExtension;
        QFile outFile(filePath);

        if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&outFile);
            out << responseContent.trimmed();
            outFile.close();
            QString relativeFilePath = QDir(m_activeProjectPath).relativeFilePath(filePath);
            chatBox->append("✅ Saved output to: " + relativeFilePath);

            // --- NEW: Register the asset after saving ---
            if (!assetInfo.isEmpty()) {
                QString assetType = assetInfo.value("type").toString();
                QString assetName = assetInfo.value("name").toString();
                QString newAssetId = m_projectStateController->registerAsset(assetType, assetName, relativeFilePath);
                chatBox->append(QString("✅ Asset registered as %1.").arg(newAssetId));
            }

            QString originalTask = roleBuffers.value(role + "_task", "[Task not found]");
            QString projectName = QFileInfo(m_activeProjectPath).baseName();
            QString memory = QString("In project '%1', the %2 agent successfully completed the task '%3' by creating the file '%4'.")
                                 .arg(projectName, role, originalTask, relativeFilePath);
            m_vectorDbManager->addMemory(memory);

            if (role == "Designer" && fileExtension == ".py") {
                m_blenderController->triggerScript(responseContent.trimmed());
            }
            m_projectWorkflow->taskFinished(taskId);
        } else {
            chatBox->append("❌ Failed to save output to: " + QDir(m_activeProjectPath).relativeFilePath(filePath));
            m_projectWorkflow->taskFailed(taskId, "Failed to write file to disk.");
        }
    }
}

void MainWindow::onAssignTask(const QString &taskId, const QString &role, const QString &task)
{
    if (role == "Integrator" && task.contains("compile", Qt::CaseInsensitive)) {
        if (!m_projectManager->isProjectLoaded()) {
            ui->IntegratorChatArea->append("❌ Cannot compile: No project loaded.");
            m_projectWorkflow->taskFinished(taskId);
            return;
        }
        roleBuffers["ActiveCompileTaskId"] = taskId;
        m_compilerController->compileProject(m_activeProjectPath);
        return;
    }

    // --- START: NEW ASSET ID RESOLUTION LOGIC ---
    QString finalTaskDescription = task;
    QString contextToInject;

    // Regex to find asset IDs like "MODEL_001", "CODE_012", etc.
    QRegularExpression assetRegex("([A-Z_]+_\\d{3})");
    auto it = assetRegex.globalMatch(task);

    while (it.hasNext()) {
        auto match = it.next();
        QString assetId = match.captured(1);
        QString assetPath = m_projectStateController->getAssetPath(assetId);

        if (!assetPath.isEmpty()) {
            qDebug() << "Resolving asset ID" << assetId << "to path" << assetPath;
            // Add the file content of the resolved asset as context for the next agent
            contextToInject += QString("[CONTEXT: %1]\n").arg(assetPath);
        } else {
            qDebug() << "Warning: Could not resolve asset ID" << assetId;
        }
    }

    // Prepend the resolved asset contexts to the task description
    if (!contextToInject.isEmpty()) {
        finalTaskDescription = contextToInject + finalTaskDescription;
    }
    // --- END: NEW ASSET ID RESOLUTION LOGIC ---

    if (roleToInputBox.contains(role)) {
        QTextEdit *targetInput = roleToInputBox[role];
        targetInput->setPlainText(finalTaskDescription);
        // --- SPECIAL CASE: VALIDATOR INJECTION ---
        if (role == "Validator") {
            // The Validator task must be handled immediately by the ValidatorController
            // which will perform the QA steps based on the task description (which contains asset IDs).
            // We do NOT send the Validator task to the LLM immediately. The LLM only generates the QA checklist.
            // For now, let's keep the LLM as the first step for simplicity, assuming its prompt is the QA spec.
        }
        handleAgent(taskId, role, targetInput, roleToChatBox[role]);
    }
}

void MainWindow::onCompilationFinished(bool success, const QString &summary)
{
    QString compileTaskId = roleBuffers.value("ActiveCompileTaskId");
    if (compileTaskId.isEmpty()) return; // Should not happen

    if (success) {
        ui->IntegratorChatArea->append("✅ Compilation task successful. Continuing workflow.");
        m_projectWorkflow->taskFinished(compileTaskId);
    } else {
        ui->IntegratorChatArea->append("❌ Compilation failed. Generating correction task for Coder.");

        // Construct a new prompt for the Coder agent
        QString correctionPrompt = QString(
                                       "CORRECTION TASK: The previous code failed to compile. "
                                       "Analyze the following compiler errors and provide a new, complete version of the file(s) that fixes the issues. "
                                       "Ensure your response contains ONLY the raw code in the correct filetype format.\n\n"
                                       "--- COMPILER ERRORS ---\n%1"
                                       ).arg(summary);

        // Dispatch the new correction task directly to the Coder
        // We use an empty Task ID because this is a dynamic, out-of-band task
        handleAgent("", "Coder", new QTextEdit(correctionPrompt, this), ui->CoderChatArea);

        // NOTE: We do NOT finish the original compile task. It remains "active"
        // and will be re-attempted after the Coder submits a fix.
    }
    roleBuffers.remove("ActiveCompileTaskId");
}

void MainWindow::onEscalateToCorrector(const QString &failedTaskId, const QString &reason, const QJsonArray &currentPlanState)
{
    QString planStateString = QJsonDocument(currentPlanState).toJson(QJsonDocument::Compact);

    QString correctionPrompt = QString(
                                   "CORRECTION TASK: Task #%1 has failed. Reason: '%2'.\n"
                                   "The current plan state is: %3\n"
                                   "Analyze the failure and the plan state, then provide a JSON object with a 'modification' key to fix the issue."
                                   ).arg(failedTaskId, reason, planStateString);

    // Call the new, specialized Corrector agent
    handleAgent("", "Corrector", new QTextEdit(correctionPrompt, this), roleToChatBox["Corrector"]);
}

void MainWindow::onManualIntervention(const QString &instructions)
{
    ui->AssistantChatArea->append("[Intervention] Sending new instructions to Manager: " + instructions);
    QString interventionTask = "The user has provided new instructions for the current project: '" + instructions + "'. Please review the project's remaining tasks and provide a new, complete plan to achieve the goal based on this new direction.";
    ui->ManagerTextEdit->setPlainText(interventionTask);
    handleAgent("", "Manager", ui->ManagerTextEdit, ui->ManagerChatArea);
}


/* -------------------------
   createDefaultEnginePrompt
   ------------------------- */
void MainWindow::createDefaultEnginePrompt() {
    QString path = basePath + "/prompts/Engine.txt";
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "You are the Unreal Engine specialist..."; // short default
        file.close();
        qDebug() << "Created default Engine prompt at:" << path;
    }
}

/* -------------------------
   routeTaskToManager
   ------------------------- */
void MainWindow::routeTaskToManager(const QString &assistantResponse, bool isIntervention)
{
    // --- GOAL EXTRACTION LOGIC ---
    int goalStart = assistantResponse.indexOf("[GOAL]");
    int goalEnd = assistantResponse.lastIndexOf("[END GOAL]");

    if (goalStart == -1 || goalEnd == -1) {
        qDebug() << "❌ Assistant Error: Could not find [GOAL] markers in the response.";
        ui->ManagerChatArea->append("❌ Assistant did not produce a valid goal. Workflow stopped.");
        return;
    }

    // Extract the full, clean goal block, including the markers themselves.
    QString cleanGoalBlock = assistantResponse.mid(goalStart, goalEnd - goalStart + QString("[END GOAL]").length());


    this->projectGoal = cleanGoalBlock; // Store the clean goal for context

    QString managerInput;
    if (isIntervention) {
        managerInput = QString(
                           "MANUAL INTERVENTION: The user has provided a new goal for the active project. "
                           "Generate a completely new [PLAN] to achieve this updated objective.\n\n"
                           "--- NEW GOAL ---\n%1"
                           ).arg(cleanGoalBlock);
        ui->AssistantChatArea->append("[System] New goal routed to Manager for plan revision.");
    } else {
        managerInput = "Process goal:\n" + cleanGoalBlock;
        ui->AssistantChatArea->append("[System] Goal routed to Manager.");
    }

    ui->ManagerTextEdit->setPlainText(managerInput);
    handleAgent("", "Manager", ui->ManagerTextEdit, ui->ManagerChatArea);
}



/* -------------------------
   Destructor: terminate processes
   ------------------------- */
MainWindow::~MainWindow() {
    // --- STOP LLAMA SERVERS BEFORE CLEANING UP ---
    m_llamaServerController->stopAllServers();
    // Defensive cleanup
    for (auto* process : findChildren<QProcess*>()) {
        if (process->state() != QProcess::NotRunning) {
            process->terminate();
            process->waitForFinished(1000);
            if (process->state() != QProcess::NotRunning) {
                process->kill();
            }
        }
        delete process;
    }

    delete ui;
}
