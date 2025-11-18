#include "projectstatecontroller.h"
#include <QFile>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

ProjectStateController::ProjectStateController(QObject *parent)
    : QObject(parent), m_lastAssetId(0)
{
}

bool ProjectStateController::loadManifest(const QString &projectPath)
{
    m_manifestPath = projectPath + "/asset_manifest.json";
    QFile file(m_manifestPath);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        m_manifest = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
        // Load the last used ID to continue the sequence
        m_lastAssetId = m_manifest.value("last_asset_id").toInt(0);
        qDebug() << "✅ Asset manifest loaded for project:" << QFileInfo(projectPath).baseName();
        return true;
    } else {
        // If no manifest exists, create a new one
        m_manifest = QJsonObject();
        m_manifest["assets"] = QJsonArray();
        m_manifest["last_asset_id"] = 0;
        m_lastAssetId = 0;
        qDebug() << "ℹ️ No manifest found. Creating a new one for the project.";
        return saveManifest();
    }
}

bool ProjectStateController::saveManifest()
{
    QFile file(m_manifestPath);
    if (file.open(QIODevice::WriteOnly)) {
        m_manifest["last_asset_id"] = m_lastAssetId;
        file.write(QJsonDocument(m_manifest).toJson(QJsonDocument::Indented));
        file.close();
        return true;
    }
    qDebug() << "❌ Failed to save asset manifest to:" << m_manifestPath;
    return false;
}

QString ProjectStateController::registerAsset(const QString &assetType, const QString &assetName, const QString &relativePath)
{
    m_lastAssetId++;
    QString newId = assetType.toUpper() + "_" + QString::number(m_lastAssetId).rightJustified(3, '0');

    QJsonObject newAsset;
    newAsset["id"] = newId;
    newAsset["type"] = assetType.toLower();
    newAsset["name"] = assetName;
    newAsset["path"] = relativePath;
    newAsset["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonArray assets = m_manifest["assets"].toArray();
    assets.append(newAsset);
    m_manifest["assets"] = assets;

    saveManifest();
    qDebug() << "✅ Asset Registered:" << newId << "(" << relativePath << ")";
    return newId;
}

QString ProjectStateController::getAssetPath(const QString &assetId) const
{
    QJsonArray assets = m_manifest["assets"].toArray();
    for (const QJsonValue &value : assets) {
        QJsonObject asset = value.toObject();
        if (asset["id"].toString().compare(assetId, Qt::CaseInsensitive) == 0) {
            return asset["path"].toString();
        }
    }
    return QString(); // Return empty string if not found
}

QJsonArray ProjectStateController::findAssetsOfType(const QString &assetType) const
{
    QJsonArray foundAssets;
    QJsonArray assets = m_manifest["assets"].toArray();
    for (const QJsonValue &value : assets) {
        QJsonObject asset = value.toObject();
        if (asset["type"].toString().compare(assetType, Qt::CaseInsensitive) == 0) {
            foundAssets.append(asset);
        }
    }
    return foundAssets;
}
