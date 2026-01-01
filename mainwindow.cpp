#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCoreApplication> 

#ifdef QT_NO_DEBUG
QString basePath = QCoreApplication::applicationDirPath();
#else
QString basePath = "C:/Users/Mahantesh/DevelopmentProjects/VrindaAI/VrindaAI";
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. Initialize Controllers
    setupControllers();

    // 2. Initialize Neural Link
    initializeNeuralLink();

    // 3. Setup UI Interactions
    setupConnections();

    // 4. Map Legacy Agents
    QStringList roles = {"Assistant", "Planner", "Manager", "Coder", "Integrator", 
                         "Scripter", "Modeling", "Texturing", "Animation", 
                         "Corrector", "Validator", "Compiler", 
                         "Designer", "Engine", "Editing", 
                         "Vrinda", "Researcher", "Architect"};

    for (const QString &role : roles) {
        roleToChatBox[role] = ui->consoleOutput;
        roleToInputBox[role] = (QTextEdit*)ui->consoleInput; 
    }

    logToConsole("System", "Ready. Python Neural Link Active.");
}

MainWindow::~MainWindow()
{
    if (m_llamaServerController) {
        m_llamaServerController->stopAllServers();
    }
    delete ui;
}

void MainWindow::setupControllers()
{
    m_globalDbManager = new DatabaseManager(basePath, this);
    m_projectManager = new ProjectManager(basePath, this);
    m_vectorDbManager = new VectorDatabaseManager(this);
    m_globalReportGenerator = new ReportGenerator(basePath, m_globalDbManager, this);
    m_projectStateController = new ProjectStateController(this);
    m_llamaServerController = new LlamaServerController(basePath, this);
    m_unrealController = new UnrealController(this);
    m_blenderController = new BlenderController(basePath, this);
    m_ffmpegController = new FfmpegController(basePath, this);
    m_headlessExecutor = new HeadlessExecutor(this);
}

void MainWindow::initializeNeuralLink()
{
    connect(m_headlessExecutor, &HeadlessExecutor::neuralLinkFinished,
            this, &MainWindow::onNeuralLinkFinished);
            
    connect(m_headlessExecutor, &HeadlessExecutor::executionStarted,
            this, &MainWindow::onExecutionStarted);
            
    connect(m_headlessExecutor, &HeadlessExecutor::executionError,
            this, &MainWindow::onExecutionError);
}

void MainWindow::setupConnections()
{
    // FIXED: Removed manual 'connect' calls.
    // Qt automatically connects functions named 'on_objectName_clicked' 
    // inside ui->setupUi(this). Manually connecting them causes the code to run twice.
}

void MainWindow::on_btnPageDashboard_clicked() { ui->mainStack->setCurrentIndex(0); }
void MainWindow::on_btnPageConsole_clicked() { ui->mainStack->setCurrentIndex(1); }
void MainWindow::on_btnPageSettings_clicked() { ui->mainStack->setCurrentIndex(2); }

// --- CORE LOGIC ---

void MainWindow::on_btnLaunch_clicked()
{
    QString name = ui->inputProjectName->text();
    QString prompt = ui->inputPrompt->toPlainText();
    QString type = ui->comboPipeline->currentText();

    if (name.isEmpty() || prompt.isEmpty()) {
        logToConsole("System", "❌ Error: Please provide Project Name and Prompt.");
        return;
    }

    ui->btnPageConsole->setChecked(true);
    ui->mainStack->setCurrentIndex(1);

    logToConsole("System", "🚀 Launching Project: " + name);

    // Step 1: Create Directory Structure (Once)
    QString projectPath = m_projectManager->createNewProject(name); 
    
    if (projectPath.isEmpty()) {
        logToConsole("System", "❌ Error: Failed to create project directories.");
        return;
    }
    
    logToConsole("System", "✅ Structure Created at: " + projectPath);

    // Step 2: Python Content Generation
    QJsonObject params;
    params["name"] = name;
    params["prompt"] = prompt;
    params["path"] = projectPath; 
    
    if (type.contains("Unreal")) params["type"] = "game";
    else if (type.contains("Blender")) params["type"] = "blender"; 
    else if (type.contains("FFmpeg")) params["type"] = "video";
    else if (type.contains("CAD")) params["type"] = "cad"; 

    m_headlessExecutor->executeUniversalTask("init_project_content", params);
}

void MainWindow::onExecutionStarted(const QString &command)
{
    logToConsole("NeuralLink", "Executing: " + command);
}

void MainWindow::onNeuralLinkFinished(const QJsonObject &result)
{
    QString status = result["status"].toString();
    
    if (status == "success" || status == "completed") {
        logToConsole("VrindaAI", "✅ Task Completed Successfully!");
    } else {
        logToConsole("VrindaAI", "⚠️ Task Finished with status: " + status);
        if (result.contains("message")) logToConsole("VrindaAI", "📝 " + result["message"].toString());
        if (result.contains("error")) logToConsole("VrindaAI", "❌ " + result["error"].toString());
    }
}

void MainWindow::onExecutionError(const QString &error)
{
    logToConsole("System", "❌ Execution Error: " + error);
}

void MainWindow::logToConsole(const QString &sender, const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString color = "#cccccc";
    if (sender == "System") color = "#00ccff";
    else if (sender == "NeuralLink") color = "#ff00ff";
    else if (sender == "VrindaAI") color = "#00ff00";
    else if (sender == "Error") color = "#ff0000";

    QString formattedHtml = QString("<span style='color: #666;'>[%1]</span> <span style='color: %2; font-weight:bold;'>%3:</span> <span style='color: #fff;'>%4</span>")
                                    .arg(timestamp, color, sender, message);
    ui->consoleOutput->append(formattedHtml);
}