#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Core Qt
#include <QMainWindow>
#include <QMap>

// All modularized components
#include "Services/modelmanager.h"
#include "Services/databasemanager.h"
#include "Services/reportgenerator.h"
#include "Services/projectmanager.h"
#include "Controllers/blendercontroller.h"
#include "Controllers/projectworkflow.h"
#include "Controllers/unrealcontroller.h"
#include "Utils/codescanner.h"
#include "Utils/promptsanitizer.h"
#include "Controllers/compilercontroller.h"
#include "Services/vectordatabasemanager.h"
#include "Controllers/projectstatecontroller.h"
#include "Controllers/modelingcontroller.h"
#include "Controllers/texturingcontroller.h"
#include "Controllers/animationcontroller.h"
#include "Controllers/validatorcontroller.h"
#include "Controllers/llamaservercontroller.h"

class QTextEdit;
class QPushButton;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:


private slots:
    void handleAgent(const QString &taskId, const QString &role, QTextEdit *inputBox, QTextEdit *chatBox);
    void routeTaskToManager(const QString &response, bool isIntervention = false);
    void createDefaultEnginePrompt();
    void onLlamaResponse(const QString &taskId, const QString &role, const QString &response, const QString &modelUsed);
    void onLlamaError(const QString &role, const QString &errorString);
    void onAssignTask(const QString &taskId, const QString &role, const QString &task);
    void initializeServicesForProject(const QString &projectPath);
    void onManualIntervention(const QString &instructions);
    void on_setUnrealPathButton_clicked();
    void onCompilationFinished(bool success, const QString &summary);
    void onMemoryQueryResult(const QList<MemoryQueryResult> &results);
    void saveAgentOutputToFile(const QString &taskId, const QString &role, const QString &responseText);
    void onEscalateToCorrector(const QString &failedTaskId, const QString &reason, const QJsonArray &currentPlanState);

private:
    Ui::MainWindow *ui;

    ProjectManager *m_projectManager;
    DatabaseManager *m_dbManager;
    ReportGenerator *m_reportGenerator;
    BlenderController *m_blenderController;
    ModelManager *m_modelManager;
    ProjectWorkflow *m_projectWorkflow;
    UnrealController *m_unrealController;
    CodeScanner *m_codeScanner;
    PromptSanitizer *m_promptSanitizer;
    CompilerController *m_compilerController;
    VectorDatabaseManager *m_vectorDbManager;
    ProjectStateController *m_projectStateController;
    ModelingController *m_modelingController;
    TexturingController *m_texturingController;
    AnimationController *m_animationController;
    ValidatorController *m_validatorController;
    LlamaServerController *m_llamaServerController;


    DatabaseManager *m_globalDbManager;
    ReportGenerator *m_globalReportGenerator;

    // Paths for LLaMA and the model.
    QString basePath = "C:/Users/Mahantesh/DevelopmentProjects/VrindaAI/VrindaAI";
    QString llamaPath = basePath + "/llama.cpp/build/bin/Release/llama-server.exe";
    QString modelPath = basePath + "/llama.cpp/build/bin/Release/mistral.gguf";

    QString m_unrealScriptQueue; // Stores python commands for batch execution

    QString m_activeProjectPath;

    // Stores the current project goal for use by multiple agents.
    QString projectGoal;
    // Add this to temporarily store the user's goal
    QString m_pendingGoal;

    // Maps roles to their corresponding UI elements.
    QMap<QString, QTextEdit*> roleToChatBox;
    QMap<QString, QTextEdit*> roleToInputBox;

    // Buffers to hold streaming output from QProcess.
    QMap<QString, QString> outputBuffers;

    // Stores transient data for the multi-agent pipeline.
    QMap<QString, QString> roleBuffers;

};

#endif // MAINWINDOW_H
