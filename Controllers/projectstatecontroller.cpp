#include "projectstatecontroller.h"
#include <QFile>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QDir>

ProjectStateController::ProjectStateController(QObject *parent)
    : QObject(parent), m_lastAssetId(0)
{
}

bool ProjectStateController::loadManifest(const QString &projectPath)
{
    // ROADMAP FIX: Use project_assets.json as the unified source of truth 
    m_manifestPath = projectPath + "/project_assets.json";
    QFile file(m_manifestPath);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        m_manifest = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
        m_lastAssetId = m_manifest.value("last_asset_id").toInt(0);
        qDebug() << "✅ Phase 1: Unified Asset Manifest loaded:" << QFileInfo(projectPath).baseName();
        return true;
    } else {
        m_manifest = QJsonObject();
        m_manifest["assets"] = QJsonArray();
        m_manifest["last_asset_id"] = 0;
        m_lastAssetId = 0;
        qDebug() << "ℹ️ Phase 1: Initializing new project_assets.json";
        return saveManifest();
    }
}

bool ProjectStateController::saveManifest()
{
    QFile file(m_manifestPath);
    if (file.open(QIODevice::WriteOnly)) {
        m_manifest["last_asset_id"] = m_lastAssetId;
        m_manifest["last_updated"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        file.write(QJsonDocument(m_manifest).toJson(QJsonDocument::Indented));
        file.close();
        return true;
    }
    return false;
}


QString ProjectStateController::registerAsset(const QString &assetType, const QString &assetName, const QString &assetDescription, const QString &relativePath)
{
    m_lastAssetId++;
    // Generates IDs like MESH_001, RENDER_002
    QString newId = assetType.toUpper() + "_" + QString::number(m_lastAssetId).rightJustified(3, '0');

    QJsonObject newAsset;
    newAsset["id"] = newId;
    newAsset["type"] = assetType.toLower();
    newAsset["name"] = assetName;
    newAsset["description"] = assetDescription; // Added for future fetching/using
    newAsset["path"] = relativePath;
    newAsset["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonArray assets = m_manifest["assets"].toArray();
    assets.append(newAsset);
    m_manifest["assets"] = assets;

    saveManifest();
    qDebug() << "✅ Asset Manifest Updated with Metadata:" << newId;
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
