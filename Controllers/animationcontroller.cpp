 #include "animationcontroller.h"

AnimationController::AnimationController(const QString &basePath, QObject *parent)
    : QObject(parent), m_basePath(basePath)
{
    qDebug() << "AnimationController initialized.";
}

void AnimationController::executeAnimationCommand(const QString &taskId, const QString &payloadString)
{
    emit processOutput(QString("Animation task %1 received payload: %2. Starting motion synthesis... (STUB)").arg(taskId, payloadString.left(50)));

    // --- STUB LOGIC: Simulate long running task ---
    QTimer::singleShot(2500, this, [=]() {
        QString generatedAssetPath = m_basePath + "/assets/anim_" + taskId + ".fbx";

        // In a real app, this runs the external animation/mocap tool.
        // It should register the asset via ProjectStateController and then emit finished.

        emit processFinished(taskId, generatedAssetPath);
    });
}
