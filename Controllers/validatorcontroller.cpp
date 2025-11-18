// validatorcontroller.cpp

#include "validatorcontroller.h"
#include <QRegularExpression>
#include <QDebug>
#include <QTimer> // Assuming QTimer is used in the executeValidationCommand stub

ValidatorController::ValidatorController(const QString &basePath, QObject *parent)
    : QObject(parent), m_basePath(basePath)
{
    qDebug() << "ValidatorController initialized.";
}

bool ValidatorController::performCheck(const QString &script)
{
    // STUB: Simulate a failure if the word "low-poly" is not in the script.

    // --- FIX APPLIED HERE: Use QRegularExpression::CaseInsensitiveOption ---
    return script.contains(QRegularExpression("low-poly", QRegularExpression::CaseInsensitiveOption));
}

void ValidatorController::executeValidationCommand(const QString &taskId, const QString &payloadString)
{
    emit validationOutput(QString("Validator task %1 received QA script. Starting checks... (STUB)").arg(taskId));

    bool success = performCheck(payloadString);
    QString summary = success ? "Asset passed all core checks (e.g., naming, basic scale)." : "Asset failed required check: Low-poly constraint not met.";

    // --- STUB LOGIC: Simulate long running task ---
    QTimer::singleShot(1500, this, [=]() {
        emit validationFinished(taskId, success, summary);
    });
}
