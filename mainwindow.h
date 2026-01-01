#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QNetworkAccessManager>
#include <QStackedWidget>
#include <QPushButton>
#include <QTextEdit>

// --- CORE SERVICES ---
#include "Services/modelmanager.h"
#include "Services/databasemanager.h"
#include "Services/reportgenerator.h"
#include "Services/projectmanager.h" 
#include "Services/vectordatabasemanager.h"
#include "Services/headlessexecutor.h"

// --- CONTROLLERS ---
#include "Controllers/blendercontroller.h"
#include "Controllers/projectworkflow.h"
#include "Controllers/unrealcontroller.h"
#include "Controllers/compilercontroller.h"
#include "Controllers/projectstatecontroller.h"
#include "Controllers/modelingcontroller.h"
#include "Controllers/texturingcontroller.h"
#include "Controllers/animationcontroller.h"
#include "Controllers/validatorcontroller.h"
#include "Controllers/llamaservercontroller.h"
#include "Controllers/ffmpegcontroller.h"

// Utils
#include "Utils/codescanner.h"
#include "Utils/promptsanitizer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // UI Navigation Slots
    void on_btnPageDashboard_clicked();
    void on_btnPageConsole_clicked();
    void on_btnPageSettings_clicked();
    
    // Core Action Slots
    void on_btnLaunch_clicked();
    
    // Neural Link Feedback
    void onNeuralLinkFinished(const QJsonObject &result);
    void onExecutionStarted(const QString &command);
    void onExecutionError(const QString &error);

private:
    Ui::MainWindow *ui;

    // --- MANAGERS & CONTROLLERS ---
    HeadlessExecutor *m_headlessExecutor; 
    
    // --- FIX: Added Missing Declaration ---
    ProjectManager *m_projectManager; 
    
    LlamaServerController *m_llamaServerController;
    ProjectStateController *m_projectStateController;
    VectorDatabaseManager *m_vectorDbManager;
    DatabaseManager *m_globalDbManager;
    ReportGenerator *m_globalReportGenerator;
    
    // Engine Controllers
    UnrealController *m_unrealController;
    BlenderController *m_blenderController;
    FfmpegController *m_ffmpegController;
    
    // --- LEGACY MAPS ---
    QMap<QString, QTextEdit*> roleToChatBox;
    QMap<QString, QTextEdit*> roleToInputBox; 

    // Helpers
    void setupConnections();
    void setupControllers();
    void initializeNeuralLink();
    void logToConsole(const QString &sender, const QString &message);
};

#endif // MAINWINDOW_H