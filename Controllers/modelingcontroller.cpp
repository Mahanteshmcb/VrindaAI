#include "modelingcontroller.h"
#include "QTimer"

ModelingController::ModelingController(const QString &basePath, QObject *parent)
    : QObject(parent), m_basePath(basePath)
{
    qDebug() << "ModelingController initialized.";
}

void ModelingController::executeModelingCommand(const QString &taskId, const QString &payloadString)
{
    emit processOutput(QString("Modeling task %1 received payload: %2. Starting heavy modeling job... (STUB)").arg(taskId, payloadString.left(50)));

    // --- STUB LOGIC: Simulate long running task ---
    // In a real application, this would start a QProcess or make an API call.

    // Simulate success after a delay
    QTimer::singleShot(2000, this, [=]() {
        QString generatedAssetPath = m_basePath + "/assets/model_" + taskId + ".fbx";

        // IMPORTANT: In the final version, this controller is responsible for:
        // 1. Running the external tool (Blender, Python script, API).
        // 2. Saving the resulting file (e.g., .fbx).
        // 3. Calling m_projectStateController->registerAsset() (needs to be passed in or accessible).
        // 4. Emitting processFinished to tell the MainWindow that the workflow can continue.

        emit processFinished(taskId, generatedAssetPath);
    });
}
