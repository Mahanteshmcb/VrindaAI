#ifndef VECTORDATABASEMANAGER_H
#define VECTORDATABASEMANAGER_H

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <qfunctionaltools_impl.h>

class QNetworkAccessManager;
class QNetworkReply;

struct MemoryQueryResult {
    QString document;
    double distance;
};
Q_DECLARE_METATYPE(MemoryQueryResult)


class VectorDatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit VectorDatabaseManager(QObject *parent = nullptr);

public slots:
    void addMemory(const QString &text);
    void queryMemory(const QString &queryText, int numResults = 3);
    void checkHealthAndProceed(std::function<void()> callback);

private slots:
    void onHealthCheckFinished(QNetworkReply *reply);

signals:
    void memoryAddedSuccessfully();
    void queryResult(const QList<MemoryQueryResult> &results);
    void memoryError(const QString &errorString);

private:
    QNetworkAccessManager *m_networkManager;
    const QUrl m_baseUrl;

    // Stores the function to call once the server is ready
    std::function<void()> m_readyCallback;
};

#endif // VECTORDATABASEMANAGER_H
