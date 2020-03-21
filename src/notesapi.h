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

#define STATUS_ENDPOINT "/status.php"
#define LOGIN_ENDPOINT "/index.php/login/v2"
#define NOTES_ENDPOINT "/index.php/apps/notes/api/v0.2"
#define POLL_INTERVALL 5000

class NotesApi : public QObject
{
    Q_OBJECT
public:
    explicit NotesApi(const QString statusEndpoint = STATUS_ENDPOINT,
                      const QString loginEndpoint = LOGIN_ENDPOINT,
                      const QString notesEndpoint = NOTES_ENDPOINT,
                      QObject *parent = nullptr);
    virtual ~NotesApi();

    Q_PROPERTY(bool sslVerify READ sslVerify WRITE setSslVerify NOTIFY sslVerifyChanged)
    bool sslVerify() const { return m_authenticatedRequest.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer; }
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
    bool networkAccessible() const { return m_manager.networkAccessible() == QNetworkAccessManager::Accessible; }

    Q_PROPERTY(QDateTime lastSync READ lastSync NOTIFY lastSyncChanged)
    QDateTime lastSync() const { return m_lastSync; }

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    bool busy() const { return !m_notesReplies.empty();; }

    enum NextcloudStatus {
        NextcloudUnknown,       // Nothing known about the nextcloud server
        NextcloudBusy,          // Getting information from the nextcloud server
        NextcloudSuccess,       // Got information about the nextcloud server
        NextcloudFailed         // Error getting information from the nextcloud server, see error()
    };
    Q_ENUM(NextcloudStatus)
    enum LoginStatus {
        LoginUnknown,           // Inital unknown state
        LoginLegacyReady,       // Ready for legacy login
        LoginFlowV2Initiating,  // Initiating login flow v2
        LoginFlowV2Polling,     // Ready for login flow v2
        LoginFlowV2Success,     // Finished login flow v2
        LoginFlowV2Failed,      // An error in login flow v2
        LoginSuccess,           // Login has been verified successfull
        LoginFailed             // Login has failed, see error()
    };
    Q_ENUM(LoginStatus)

    Q_PROPERTY(NextcloudStatus ncStatusStatus READ ncStatusStatus NOTIFY ncStatusStatusChanged)
    NextcloudStatus ncStatusStatus() const { return m_ncStatusStatus; }
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

    Q_PROPERTY(LoginStatus loginStatus READ loginStatus NOTIFY loginStatusChanged)
    LoginStatus loginStatus() const { return m_loginStatus; }
    Q_PROPERTY(QUrl loginUrl READ loginUrl NOTIFY loginUrlChanged)
    QUrl loginUrl() const { return m_loginUrl; }

    Q_INVOKABLE bool getNcStatus();
    Q_INVOKABLE bool initiateFlowV2Login();
    Q_INVOKABLE void abortFlowV2Login();
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
    Q_INVOKABLE const QString errorMessage(ErrorCodes error) const;

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

    void ncStatusStatusChanged(NextcloudStatus status);
    void statusInstalledChanged(bool installed);
    void statusMaintenanceChanged(bool maintenance);
    void statusNeedsDbUpgradeChanged(bool needsDbUpgrade);
    void statusVersionChanged(QString version);
    void statusVersionStringChanged(QString versionString);
    void statusEditionChanged(QString edition);
    void statusProductNameChanged(QString productName);
    void statusExtendedSupportChanged(bool extendedSupport);

    void loginStatusChanged(LoginStatus status);
    void loginUrlChanged(QUrl url);
    void error(ErrorCodes error);

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
    QUrl m_url;
    QNetworkAccessManager m_manager;
    QNetworkRequest m_request;
    QNetworkRequest m_authenticatedRequest;
    QFile m_jsonFile;
    NotesModel* mp_model;
    NotesProxyModel* mp_modelProxy;
    QUrl apiEndpointUrl(const QString endpoint) const;

    // Nextcloud status.php
    const QString m_statusEndpoint;
    QNetworkReply* m_statusReply;
    void updateNcStatus(const QJsonObject &status);
    NextcloudStatus m_ncStatusStatus;
    void setNcStatusStatus(NextcloudStatus status, bool *changed = NULL);
    bool m_status_installed;
    bool m_status_maintenance;
    bool m_status_needsDbUpgrade;
    QString m_status_version;
    QString m_status_versionstring;
    QString m_status_edition;
    QString m_status_productname;
    bool m_status_extendedSupport;

    // Nextcloud Login Flow v2 - https://docs.nextcloud.com/server/18/developer_manual/client_apis/LoginFlow/index.html#login-flow-v2
    const QString m_loginEndpoint;
    QNetworkReply* m_loginReply;
    QNetworkReply* m_pollReply;
    bool updateLoginFlow(const QJsonObject &login);
    bool updateLoginCredentials(const QJsonObject &credentials);
    LoginStatus m_loginStatus;
    void setLoginStatus(LoginStatus status, bool *changed = NULL);
    QTimer m_loginPollTimer;
    QUrl m_loginUrl;
    QUrl m_pollUrl;
    QString m_pollToken;

    // Nextcloud Notes API - https://github.com/nextcloud/notes/wiki/Notes-0.2
    const QString m_notesEndpoint;
    QVector<QNetworkReply*> m_notesReplies;
    QDateTime m_lastSync;
};

#endif // NOTESAPI_H
