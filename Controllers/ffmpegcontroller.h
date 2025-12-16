#ifndef FFMPEGCONTROLLER_H
#define FFMPEGCONTROLLER_H

#include <QObject>
#include <QProcess>
#include <QMap>
#include <QJsonArray>

/**
 * @class FfmpegController
 * @brief Manages the execution of the external Python-based FFmpeg engine.
 */
class FfmpegController : public QObject
{
    Q_OBJECT
public:
    explicit FfmpegController(const QString &basePath, QObject *parent = nullptr);

    /**
     * @brief Executes a video editing command by running the Python script.
     * @param taskId The ID of the workflow task.
     * @param jsonPayload A JSON string containing the FFmpeg job manifest.
     */
    void executeEditingCommand(const QString &taskId, const QString &jsonPayload);

signals:
    void processOutput(const QString &output);
    void processError(const QString &error);
    void processFinished(const QString &taskId, const QString &outputPath);

private slots:
    void readProcessOutput();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString m_basePath;
    QString m_scriptPath;
    // Maps a QProcess instance back to its original workflow task ID
    QMap<QProcess*, QString> m_activeTaskIds;
};

#endif // FFMPEGCONTROLLER_H