#include "texturingcontroller.h"

TexturingController::TexturingController(const QString &basePath, QObject *parent)
    : QObject(parent), m_basePath(basePath)
{
    qDebug() << "TexturingController initialized.";
}

void TexturingController::executeTexturingCommand(const QString &taskId, const QString &payloadString)
{
    emit processOutput(QString("Texturing task %1 received payload: %2. Starting PBR map generation... (STUB)").arg(taskId, payloadString.left(50)));

    // --- STUB LOGIC: Simulate long running task ---
    QTimer::singleShot(3000, this, [=]() {
        // Assume three texture maps and one material descriptor are created
        QString materialPath = m_basePath + "/assets/material_" + taskId + ".json";

        // In a real app, this runs the texture diffusion model.
        // It should register the material manifest via ProjectStateController and then emit finished.

        emit processFinished(taskId, materialPath);
    });
}
