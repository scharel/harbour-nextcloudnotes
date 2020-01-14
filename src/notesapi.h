#ifndef NOTESAPI_H
#define NOTESAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QTimer>
#include <QDebug>
#include "notesmodel.h"

class NotesApi : public QObject
{
    Q_OBJECT
public:
    explicit NotesApi(QObject *parent = nullptr);
    virtual ~NotesApi();

    Q_PROPERTY(bool sslVerify READ sslVerify WRITE setSslVerify NOTIFY sslVerifyChanged)
    bool sslVerify() const { return m_request.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer; }
    void setSslVerify(bool verify);

    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    QUrl url() const { return m_url; }
    void setUrl(QUrl url);

    Q_PROPERTY(bool urlValid READ urlValid NOTIFY urlValidChanged)
    bool urlValid() const { return m_url.isValid(); }

    Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged)
    QString server() const;
    void setServer(QString server);

    Q_PROPERTY(QString scheme READ scheme WRITE setScheme NOTIFY schemeChanged)
    QString scheme() const { return m_url.scheme(); }
    void setScheme(QString scheme);

    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    QString host() const { return m_url.host(); }
    void setHost(QString host);

    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    int port() const { return m_url.port(); }
    void setPort(int port);

    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    QString username() const { return m_url.userName(); }
    void setUsername(QString username);

    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    QString password() const { return m_url.password(); }
    void setPassword(QString password);

    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    QString path() const { return m_url.path(); }
    void setPath(QString path);

    Q_PROPERTY(QString dataFile READ dataFile WRITE setDataFile NOTIFY dataFileChanged)
    QString dataFile() const { return m_jsonFile.fileName(); }
    void setDataFile(QString dataFile);

    Q_PROPERTY(bool networkAccessible READ networkAccessible NOTIFY networkAccessibleChanged)
    bool networkAccessible() const { return m_online; }

    Q_PROPERTY(QDateTime lastSync READ lastSync NOTIFY lastSyncChanged)
    QDateTime lastSync() const { return m_lastSync; }

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    bool busy() const;

    Q_PROPERTY(bool statusInstalled READ statusInstalled NOTIFY statusInstalledChanged)
    bool statusInstalled() const { return m_status_installed; }
    Q_PROPERTY(bool statusMaintenance READ statusMaintenance NOTIFY statusMaintenanceChanged)
    bool statusMaintenance() const { return m_status_maintenance; }
    Q_PROPERTY(bool statusNeedsDbUpgrade READ statusNeedsDbUpgrade NOTIFY statusNeedsDbUpgradeChanged)
    bool statusNeedsDbUpgrade() const { return m_status_needsDbUpgrade; }
    Q_PROPERTY(QString statusVersion READ statusVersion NOTIFY statusVersionChanged)
    QString statusVersion() const { return m_status_version; }
    Q_PROPERTY(QString statusVersionString READ statusVersionString NOTIFY statusVersionStringChanged)
    QString statusVersionString() const { return m_status_versionstring; }
    Q_PROPERTY(QString statusEdition READ statusEdition NOTIFY statusEditionChanged)
    QString statusEdition() const { return m_status_edition; }
    Q_PROPERTY(QString statusProductName READ statusProductName NOTIFY statusProductNameChanged)
    QString statusProductName() const { return m_status_productname; }
    Q_PROPERTY(bool statusExtendedSupport READ statusExtendedSupport NOTIFY statusExtendedSupportChanged)
    bool statusExtendedSupport() const { return m_status_extendedSupport; }
    Q_PROPERTY(QUrl loginUrl READ loginUrl NOTIFY loginUrlChanged)
    QUrl loginUrl() const { return m_loginUrl; }

    Q_INVOKABLE void getStatus();
    Q_INVOKABLE void initiateFlowV2Login();
    Q_INVOKABLE void getAllNotes(QStringList excludeFields = QStringList());
    Q_INVOKABLE void getNote(double noteId, QStringList excludeFields = QStringList());
    Q_INVOKABLE void createNote(QVariantMap fields = QVariantMap());
    Q_INVOKABLE void updateNote(double noteId, QVariantMap fields = QVariantMap());
    Q_INVOKABLE void deleteNote(double noteId);
    Q_INVOKABLE NotesProxyModel* model() const { return mp_modelProxy; }

    enum ErrorCodes {
        NoError,
        NoConnectionError,
        CommunicationError,
        LocalFileReadError,
        LocalFileWriteError,
        SslHandshakeError,
        AuthenticationError
    };
    Q_INVOKABLE const QString errorMessage(int error) const;

signals:
    void sslVerifyChanged(bool verify);
    void urlChanged(QUrl url);
    void urlValidChanged(bool valid);
    void serverChanged(QString server);
    void schemeChanged(QString scheme);
    void hostChanged(QString host);
    void portChanged(int port);
    void usernameChanged(QString username);
    void passwordChanged(QString password);
    void pathChanged(QString path);
    void dataFileChanged(QString dataFile);
    void networkAccessibleChanged(bool accessible);
    void lastSyncChanged(QDateTime lastSync);
    void busyChanged(bool busy);
    void statusInstalledChanged(bool installed);
    void statusMaintenanceChanged(bool maintenance);
    void statusNeedsDbUpgradeChanged(bool needsDbUpgrade);
    void statusVersionChanged(QString version);
    void statusVersionStringChanged(QString versionString);
    void statusEditionChanged(QString edition);
    void statusProductNameChanged(QString productName);
    void statusExtendedSupportChanged(bool extendedSupport);
    void loginUrlChanged(QUrl url);
    void error(int error);

public slots:

private slots:
    void verifyUrl(QUrl url);
    void requireAuthentication(QNetworkReply * reply, QAuthenticator * authenticator);
    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void replyFinished(QNetworkReply* reply);
    void pollLoginUrl();
    void sslError(QNetworkReply* reply, const QList<QSslError> &errors);
    void saveToFile(QModelIndex,QModelIndex,QVector<int>);

private:
    bool m_online;
    QDateTime m_lastSync;
    QUrl m_url;
    QNetworkAccessManager m_manager;
    QNetworkRequest m_request;
    QVector<QNetworkReply*> m_replies;
    QFile m_jsonFile;
    NotesModel* mp_model;
    NotesProxyModel* mp_modelProxy;

    void updateStatus(const QJsonObject &status);
    QVector<QNetworkReply*> m_status_replies;
    bool m_status_installed;
    bool m_status_maintenance;
    bool m_status_needsDbUpgrade;
    QString m_status_version;
    QString m_status_versionstring;
    QString m_status_edition;
    QString m_status_productname;
    bool m_status_extendedSupport;

    void updateLoginFlow(const QJsonObject &login);
    void updateLoginCredentials(const QJsonObject &credentials);
    QVector<QNetworkReply*> m_login_replies;
    QVector<QNetworkReply*> m_poll_replies;
    QTimer m_loginPollTimer;
    QUrl m_loginUrl;
    QUrl m_pollUrl;
    QString m_pollToken;
};

#endif // NOTESAPI_H
