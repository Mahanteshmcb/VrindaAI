#ifndef MODELINGCONTROLLER_H
#define MODELINGCONTROLLER_H

#include <QObject>
#include <QString>
#include <QDebug>

class ModelingController : public QObject
{
    Q_OBJECT
public:
    explicit ModelingController(const QString &basePath, QObject *parent = nullptr);

signals:
    void processOutput(const QString &output);
    void processFinished(const QString &taskId, const QString &assetPath);

public slots:
    void executeModelingCommand(const QString &taskId, const QString &payloadString);

private:
    QString m_basePath;
};

#endif // MODELINGCONTROLLER_H
