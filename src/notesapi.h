#ifndef NOTESAPI_H
#define NOTESAPI_H

#include <QObject>
#include <QJsonObject>
#include <QVersionNumber>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QTimer>
#include <QDebug>

const QString STATUS_ENDPOINT("/status.php");
const QString LOGIN_ENDPOINT("/index.php/login/v2");
const QString NOTES_ENDPOINT("/index.php/apps/notes/api/v0.2/notes");
const QString OCS_ENDPOINT("/ocs/v1.php/cloud");
const QString EXCLUDE_QUERY("exclude=");
const QString PURGE_QUERY("purgeBefore=");
const QString ETAG_HEADER("If-None-Match");
const int POLL_INTERVALL = 5000;

class NotesApi : public QObject
{
    Q_OBJECT

    // Generic API properties
    Q_PROPERTY(bool verifySsl READ verifySsl WRITE setVerifySsl NOTIFY verifySslChanged)    // to allow selfsigned certificates
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)    // complete API URL = <scheme>://<username>:<password>@<host>[:<port>]/<path>
    Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged) // url without username and password = <scheme>://<host>[:<port>]/<path>
    Q_PROPERTY(QString scheme READ scheme WRITE setScheme NOTIFY schemeChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)

    // Class status information
    Q_PROPERTY(bool urlValid READ urlValid NOTIFY urlValidChanged)
    Q_PROPERTY(bool networkAccessible READ networkAccessible NOTIFY networkAccessibleChanged)
    Q_PROPERTY(QDateTime lastSync READ lastSync NOTIFY lastSyncChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

    // Nextcloud capabilities
    Q_PROPERTY(CapabilitiesStatus capabilitiesStatus READ capabilitiesStatus NOTIFY capabilitiesStatusChanged)
    Q_PROPERTY(bool notesAppInstalled READ notesAppInstalled NOTIFY notesAppInstalledChanged)
    Q_PROPERTY(QStringList notesAppApiVersions READ notesAppApiVersions NOTIFY notesAppApiVersionsChanged)
    Q_PROPERTY(QString notesAppApiUsedVersion READ notesAppApiUsedVersion NOTIFY notesAppApiUsedVersionChanged)

    // Nextcloud status (status.php)
    Q_PROPERTY(NextcloudStatus ncStatusStatus READ ncStatusStatus NOTIFY ncStatusStatusChanged)
    Q_PROPERTY(bool statusInstalled READ statusInstalled NOTIFY statusInstalledChanged)
    Q_PROPERTY(bool statusMaintenance READ statusMaintenance NOTIFY statusMaintenanceChanged)
    Q_PROPERTY(bool statusNeedsDbUpgrade READ statusNeedsDbUpgrade NOTIFY statusNeedsDbUpgradeChanged)
    Q_PROPERTY(QString statusVersion READ statusVersion NOTIFY statusVersionChanged)
    Q_PROPERTY(QString statusVersionString READ statusVersionString NOTIFY statusVersionStringChanged)
    Q_PROPERTY(QString statusEdition READ statusEdition NOTIFY statusEditionChanged)
    Q_PROPERTY(QString statusProductName READ statusProductName NOTIFY statusProductNameChanged)
    Q_PROPERTY(bool statusExtendedSupport READ statusExtendedSupport NOTIFY statusExtendedSupportChanged)

    // Login status
    Q_PROPERTY(LoginStatus loginStatus READ loginStatus NOTIFY loginStatusChanged)
    Q_PROPERTY(QUrl loginUrl READ loginUrl NOTIFY loginUrlChanged)

public:
    explicit NotesApi(const QString statusEndpoint = STATUS_ENDPOINT,
                      const QString loginEndpoint = LOGIN_ENDPOINT,
                      const QString ocsEndpoint = OCS_ENDPOINT,
                      const QString notesEndpoint = NOTES_ENDPOINT,
                      QObject *parent = nullptr);
    virtual ~NotesApi();

    enum CapabilitiesStatus {
        CapabilitiesUnknown,        // Initial unknown state
        CapabilitiesBusy,           // Gettin information
        CapabilitiesSuccess,        // Capabilities successfully read
        CapabilitiesStatusFailed    // Faild to retreive capabilities
    };
    Q_ENUM(CapabilitiesStatus)

    enum NextcloudStatus {
        NextcloudUnknown,           // Initial unknown state
        NextcloudBusy,              // Getting information from the nextcloud server
        NextcloudSuccess,           // Got information about the nextcloud server
        NextcloudFailed             // Error getting information from the nextcloud server, see error()
    };
    Q_ENUM(NextcloudStatus)

    enum LoginStatus {
        LoginUnknown,               // Inital unknown state
        LoginLegacyReady,           // Ready for legacy login
        LoginFlowV2Initiating,      // Initiating login flow v2
        LoginFlowV2Polling,         // Ready for login flow v2
        LoginFlowV2Success,         // Finished login flow v2
        LoginFlowV2Failed,          // An error in login flow v2
        LoginSuccess,               // Login has been verified successfull
        LoginFailed                 // Login has failed, see error()
    };
    Q_ENUM(LoginStatus)

    bool verifySsl() const { return m_authenticatedRequest.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer; }
    void setVerifySsl(bool verify);

    QUrl url() const { return m_url; }
    void setUrl(QUrl url);

    bool urlValid() const { return m_url.isValid(); }

    QString server() const;
    void setServer(QString server);

    QString scheme() const { return m_url.scheme(); }
    void setScheme(QString scheme);

    QString host() const { return m_url.host(); }
    void setHost(QString host);

    int port() const { return m_url.port(); }
    void setPort(int port);

    QString username() const { return m_url.userName(); }
    void setUsername(QString username);

    QString password() const { return m_url.password(); }
    void setPassword(QString password);

    QString path() const { return m_url.path(); }
    void setPath(QString path);

    bool networkAccessible() const { return m_manager.networkAccessible() == QNetworkAccessManager::Accessible; }

    QDateTime lastSync() const { return m_lastSync; }

    bool busy() const;

    CapabilitiesStatus capabilitiesStatus() const { return m_capabilitiesStatus; }
    bool notesAppInstalled() const { return m_capabilities_notesInstalled; }
    QStringList notesAppApiVersions() const { return m_capabilities_notesApiVersions; }
    static QString notesAppApiUsedVersion() { return m_capabilities_implementedApiVersion.toString(); }

    NextcloudStatus ncStatusStatus() const { return m_ncStatusStatus; }
    bool statusInstalled() const { return m_status_installed; }
    bool statusMaintenance() const { return m_status_maintenance; }
    bool statusNeedsDbUpgrade() const { return m_status_needsDbUpgrade; }
    QString statusVersion() const { return m_status_version.toString(); }
    QString statusVersionString() const { return m_status_versionstring; }
    QString statusEdition() const { return m_status_edition; }
    QString statusProductName() const { return m_status_productname; }
    bool statusExtendedSupport() const { return m_status_extendedSupport; }

    LoginStatus loginStatus() const { return m_loginStatus; }
    QUrl loginUrl() const { return m_loginUrl; }

    Q_INVOKABLE bool getNcStatus();
    Q_INVOKABLE bool initiateFlowV2Login();
    Q_INVOKABLE void abortFlowV2Login();
    Q_INVOKABLE void verifyLogin(QString username = QString(), QString password = QString());

    enum ErrorCodes {
        NoError,
        NoConnectionError,
        CommunicationError,
        SslHandshakeError,
        AuthenticationError
    };
    Q_ENUM(ErrorCodes)
    Q_INVOKABLE const QString errorMessage(int error) const;

    QString account() const;
    void setAccount(const QString& account);

    const QList<int> noteIds();
    bool noteExists(const int id);
    int noteModified(const int id);

public slots:
    Q_INVOKABLE bool getAllNotes(const QStringList& exclude = QStringList());
    Q_INVOKABLE bool getNote(const int id, const QStringList& exclude = QStringList());
    Q_INVOKABLE bool createNote(const QJsonObject& note);
    Q_INVOKABLE bool updateNote(const int id, const QJsonObject& note);
    Q_INVOKABLE bool deleteNote(const int id);

signals:
    void verifySslChanged(bool verify);
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

    void capabilitiesStatusChanged(CapabilitiesStatus status);
    void notesAppInstalledChanged(bool installed);
    void notesAppApiVersionsChanged(QStringList versions);
    void notesAppApiUsedVersionChanged(QString version);

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

    void noteCreated(int id, const QJsonObject& note);
    void noteUpdated(int id, const QJsonObject& note);
    void noteDeleted(int id);
    void noteError(ErrorCodes error);

private slots:
    void verifyUrl(QUrl url);
    void requireAuthentication(QNetworkReply * reply, QAuthenticator * authenticator);
    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void replyFinished(QNetworkReply* reply);
    void pollLoginUrl();
    void sslError(QNetworkReply* reply, const QList<QSslError> &errors);

private:
    QUrl m_url;
    QMap<int, int> m_syncedNotes;
    QNetworkAccessManager m_manager;
    QNetworkRequest m_request;
    QNetworkRequest m_authenticatedRequest;
    QNetworkRequest m_ocsRequest;
    QUrl apiEndpointUrl(const QString endpoint) const;

    bool updateCapabilities(const QJsonObject & capabilities);
    CapabilitiesStatus m_capabilitiesStatus;
    void setCababilitiesStatus(CapabilitiesStatus status, bool *changed = NULL);
    bool m_capabilities_notesInstalled;
    static QVersionNumber m_capabilities_implementedApiVersion;
    QStringList m_capabilities_notesApiVersions;

    // Nextcloud status.php
    const QString m_statusEndpoint;
    QVector<QNetworkReply*> m_statusReplies;
    void updateNcStatus(const QJsonObject &status);
    NextcloudStatus m_ncStatusStatus;
    void setNcStatusStatus(NextcloudStatus status, bool *changed = NULL);
    bool m_status_installed;
    bool m_status_maintenance;
    bool m_status_needsDbUpgrade;
    QVersionNumber m_status_version;
    QString m_status_versionstring;
    QString m_status_edition;
    QString m_status_productname;
    bool m_status_extendedSupport;

    // Nextcloud Login Flow v2 - https://docs.nextcloud.com/server/18/developer_manual/client_apis/LoginFlow/index.html#login-flow-v2
    const QString m_loginEndpoint;
    QVector<QNetworkReply*> m_loginReplies;
    QVector<QNetworkReply*> m_pollReplies;
    bool updateLoginFlow(const QJsonObject &login);
    bool updateLoginCredentials(const QJsonObject &credentials);
    LoginStatus m_loginStatus;
    void setLoginStatus(LoginStatus status, bool *changed = NULL);
    QTimer m_loginPollTimer;
    QUrl m_loginUrl;
    QUrl m_pollUrl;
    QString m_pollToken;

    // Nextcloud OCS API - https://docs.nextcloud.com/server/18/developer_manual/client_apis/OCS/ocs-api-overview.html
    const QString m_ocsEndpoint;
    QVector<QNetworkReply*> m_ocsReplies;

    // Nextcloud Notes API - https://github.com/nextcloud/notes/wiki/Notes-0.2
    const QString m_notesEndpoint;
    QVersionNumber m_notesApiVersion;
    QVector<QNetworkReply*> m_getAllNotesReplies;
    QVector<QNetworkReply*> m_getNoteReplies;
    QVector<QNetworkReply*> m_createNoteReplies;
    QVector<QNetworkReply*> m_updateNoteReplies;
    QVector<QNetworkReply*> m_deleteNoteReplies;
    void updateApiNotes(const QJsonArray& json);
    void updateApiNote(const QJsonObject& json);
    void createApiNote(const QJsonObject& json);
    QDateTime m_lastSync;
};

#endif // NOTESAPI_H
