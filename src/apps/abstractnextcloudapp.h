#ifndef ABSTRACTNEXTCLOUDAPP_H
#define ABSTRACTNEXTCLOUDAPP_H

#include <QObject>
#include <QNetworkReply>
#include "../nextcloudapi.h"

class AbstractNextcloudApp : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString appName READ appName)
    Q_PROPERTY(bool installed READ installed NOTIFY installedChanged)

public:
    AbstractNextcloudApp(QObject *parent = nullptr, QString name = QString(), NextcloudApi* api = nullptr) : QObject(parent), m_appName(name), m_api(api) {
        connect(this, SIGNAL(capabilitiesChanged), this, SLOT(updateCapabilities));
        connect(m_api, SIGNAL(capabilitiesChanged), this, SLOT(updateApiCapabilities));
        connect(this, SIGNAL(replyReceived), this, SLOT(updateReply));
        connect(m_api, SIGNAL(replyReceived), this, SLOT(updateApiReply));
    }

    virtual ~AbstractNextcloudApp() {
        while (!m_replies.empty()) {
            QNetworkReply* reply = m_replies.first();
            reply->abort();
            reply->deleteLater();
            m_replies.removeFirst();
        }
    }

    const QString appName() const { return m_appName; }
    bool installed() const { return !m_capabilities.isEmpty(); }

public slots:
    void updateApiCapabilities(QJsonObject* json) {
        QJsonObject capabilities = json->value(appName()).toObject();
        if (m_capabilities != capabilities) {
            bool instChanged = m_capabilities.isEmpty() or capabilities.isEmpty();
            m_capabilities = capabilities;
            emit capabilitiesChanged(&m_capabilities);
            if (instChanged) {
                emit installedChanged(!m_capabilities.isEmpty());
            }
        }
    }

    bool updateApiReply(QNetworkReply* reply) {
        if (m_replies.contains(reply)) {
            emit replyReceived(reply);
        }
        return m_replies.removeOne(reply);
    }

signals:
    void installedChanged(bool);
    void capabilitiesChanged(QJsonObject* json);
    void replyReceived(QNetworkReply* reply);

protected:
    const QString m_appName;
    NextcloudApi* m_api;
    QJsonObject m_capabilities;
    QVector<QNetworkReply*> m_replies;
};

#endif // ABSTRACTNEXTCLOUDAPP_H
