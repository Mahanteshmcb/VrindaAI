#ifndef BLENDERCONTROLLER_H
#define BLENDERCONTROLLER_H

#include <QObject>
#include <QString>
#include <QTextEdit>

class QTextEdit; // Forward-declaration

class BlenderController : public QObject
{
    Q_OBJECT
public:
    explicit BlenderController(const QString &basePath, QObject *parent = nullptr);

    // --- NEW: Public setter for configuration ---
    void setBlenderPath(const QString &path);

    void setActiveProjectPath(const QString &path);

public slots:
    void triggerScript(const QString &scriptContent);

signals:
    // Emitted when the blender process produces output or errors
    void blenderOutput(const QString &output);
    void blenderError(const QString &error);
    void blenderFinished(int exitCode);

    // Emitted when a specific asset path is detected in the output
    void assetReadyForEngine(const QString &assetPath);

private:
    QString m_basePath;
    QString m_blenderPath;
    QString m_activeProjectPath;
};

#endif // BLENDERCONTROLLER_H
