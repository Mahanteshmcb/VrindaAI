#ifndef PROJECTSTATECONTROLLER_H
#define PROJECTSTATECONTROLLER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

class ProjectStateController : public QObject
{
    Q_OBJECT
public:
    explicit ProjectStateController(QObject *parent = nullptr);

    bool loadManifest(const QString &projectPath);
    bool saveManifest();

    QString registerAsset(const QString &assetType, const QString &assetName, const QString &assetDescription, const QString &relativePath);
    QString getAssetPath(const QString &assetId) const;
    QJsonArray findAssetsOfType(const QString &assetType) const;

private:
    QString m_manifestPath;
    QJsonObject m_manifest;
    int m_lastAssetId;
};

#endif // PROJECTSTATECONTROLLER_H
