#ifndef COMPILERCONTROLLER_H
#define COMPILERCONTROLLER_H

#include <QObject>
#include <QProcess>

class CompilerController : public QObject
{
    Q_OBJECT
public:
    explicit CompilerController(QObject *parent = nullptr);
    ~CompilerController();

public slots:
    void compileProject(const QString &projectPath);

signals:
    void compilationOutput(const QString &output);
    void compilationFinished(bool success, const QString &summary);

private slots:
    void onConfigureProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onBuildProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_process;
    QString m_projectPath;
    QString m_errorBuffer;
};

#endif // COMPILERCONTROLLER_H
