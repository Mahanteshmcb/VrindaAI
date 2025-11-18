#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDebug>

class AnimationController : public QObject
{
    Q_OBJECT
public:
    explicit AnimationController(const QString &basePath, QObject *parent = nullptr);

signals:
    void processOutput(const QString &output);
    void processFinished(const QString &taskId, const QString &assetPath);

public slots:
    void executeAnimationCommand(const QString &taskId, const QString &payloadString);

private:
    QString m_basePath;
};

#endif // ANIMATIONCONTROLLER_H
