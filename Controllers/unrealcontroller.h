#ifndef UNREALCONTROLLER_H
#define UNREALCONTROLLER_H

#include <QObject>
#include <QJsonArray>

class UnrealController : public QObject
{
    Q_OBJECT
public:
    explicit UnrealController(QObject *parent = nullptr);

    void setUnrealEditorPath(const QString &path);

public slots:
    // This new function will replace all the old ones like importAsset, openLevel, etc.
    void executeInstructionSequence(const QString &projectUProjectPath, const QJsonArray &instructions);

signals:
    void unrealOutput(const QString &output);
    void unrealError(const QString &error);

private:
    void runUnrealProcess(const QString &projectUProjectPath, const QString &scriptPath);
    QString m_unrealEditorPath;
};

#endif // UNREALCONTROLLER_H
