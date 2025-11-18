#ifndef TEXTURINGCONTROLLER_H
#define TEXTURINGCONTROLLER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDebug>

class TexturingController : public QObject
{
    Q_OBJECT
public:
    explicit TexturingController(const QString &basePath, QObject *parent = nullptr);

signals:
    void processOutput(const QString &output);
    void processFinished(const QString &taskId, const QString &materialManifestPath);

public slots:
    void executeTexturingCommand(const QString &taskId, const QString &payloadString);

private:
    QString m_basePath;
};

#endif // TEXTURINGCONTROLLER_H
