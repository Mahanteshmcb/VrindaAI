#include "compilercontroller.h"
#include <QDebug>

CompilerController::CompilerController(QObject *parent)
    : QObject(parent)
{
    m_process = new QProcess(this);

    // Redirect standard error to standard output to capture all messages in order
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
        emit compilationOutput(output);
        if (output.contains("error:", Qt::CaseInsensitive) || output.contains("failed", Qt::CaseInsensitive)) {
            m_errorBuffer.append(output);
        }
    });
}

CompilerController::~CompilerController()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished();
    }
}

void CompilerController::compileProject(const QString &projectPath)
{
    m_projectPath = projectPath;
    m_errorBuffer.clear();

    emit compilationOutput("🚀 Starting compilation process...\n--- Running CMake to configure project ---");

    // We chain the commands: first configure, then build.
    // Disconnect any previous connections for the finished signal.
    disconnect(m_process, &QProcess::finished, nullptr, nullptr);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CompilerController::onConfigureProcessFinished);

    m_process->setWorkingDirectory(m_projectPath);
    // Note: This command assumes CMake and MinGW are in the system's PATH.
    m_process->start("cmake", QStringList() << "." << "-G" << "MinGW Makefiles");
}

void CompilerController::onConfigureProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        emit compilationOutput("❌ CMake configuration failed.");
        emit compilationFinished(false, "CMake configuration failed.\n" + m_errorBuffer);
        return;
    }

    emit compilationOutput("✅ CMake configuration successful.\n--- Running make to build project ---");

    // Now, run the build command (e.g., mingw32-make)
    disconnect(m_process, &QProcess::finished, nullptr, nullptr);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CompilerController::onBuildProcessFinished);

    m_process->start("mingw32-make");
}

void CompilerController::onBuildProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        emit compilationOutput("❌ Build failed.");
        emit compilationFinished(false, "Build failed with errors:\n" + m_errorBuffer);
    } else {
        emit compilationOutput("✅ Build successful!");
        emit compilationFinished(true, "Project compiled successfully.");
    }
}
