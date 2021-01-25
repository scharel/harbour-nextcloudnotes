#ifndef NEXTCLOUDAPI_H
#define NEXTCLOUDAPI_H

#include <QObject>
#include <QJsonObject>
#include <QVersionNumber>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QTimer>
#include <QDebug>

// Nextcloud instance information
const QString STATUS_ENDPOINT("/status.php");

// Capabilites and users
const QString CAPABILITIES_ENDPOINT("/ocs/v1.php/cloud/capabilities");
const QString USERS_ENDPOINT("/ocs/v1.php/cloud/users");
const QString USER_CAPABILITIES_ENDPOINT("/ocs/v1.php/cloud/users/%1");

// Login and authentication
const QString GET_APPPASSWORD_ENDPOINT("/ocs/v2.php/core/getapppassword");
const QString DEL_APPPASSWORD_ENDPOINT("/ocs/v2.php/core/apppassword");
const QString LOGIN_FLOWV2_ENDPOINT("/index.php/login/v2");
const int LOGIN_FLOWV2_MIN_VERSION = 16;
const int LOGIN_FLOWV2_POLL_INTERVALL = 5000;

class NextcloudApi : public QObject
{
    Q_OBJECT

    // Generic API properties
    Q_PROPERTY(bool verifySsl READ verifySsl WRITE setVerifySsl NOTIFY verifySslChanged)    // to allow selfsigned certificates
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)    // complete API URL = <scheme>://<username>:<password>@<host>[:<port>]/<path>
    Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged) // url without username and password = <scheme>://<host>[:<port>]/<path>
    // the following six properties will update the url and server properties
    Q_PROPERTY(QString scheme READ scheme WRITE setScheme NOTIFY schemeChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)

    // Networking status information
    Q_PROPERTY(bool urlValid READ urlValid NOTIFY urlValidChanged)
    Q_PROPERTY(bool networkAccessible READ networkAccessible NOTIFY networkAccessibleChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

    // Nextcloud status (status.php), these properties will be automatically updated on changes of the generic properties
    Q_PROPERTY(NextcloudStatus ncStatusStatus READ ncStatusStatus NOTIFY ncStatusStatusChanged)
    Q_PROPERTY(bool statusInstalled READ statusInstalled NOTIFY statusInstalledChanged)
    Q_PROPERTY(bool statusMaintenance READ statusMaintenance NOTIFY statusMaintenanceChanged)
    Q_PROPERTY(bool statusNeedsDbUpgrade READ statusNeedsDbUpgrade NOTIFY statusNeedsDbUpgradeChanged)
    Q_PROPERTY(QString statusVersion READ statusVersion NOTIFY statusVersionChanged)
    Q_PROPERTY(QString statusVersionString READ statusVersionString NOTIFY statusVersionStringChanged)
    Q_PROPERTY(QString statusEdition READ statusEdition NOTIFY statusEditionChanged)
    Q_PROPERTY(QString statusProductName READ statusProductName NOTIFY statusProductNameChanged)
    Q_PROPERTY(bool statusExtendedSupport READ statusExtendedSupport NOTIFY statusExtendedSupportChanged)
    Q_PROPERTY(bool loginFlowV2Possible READ loginFlowV2Possible NOTIFY loginFlowV2PossibleChanged)

    // Nextcloud capabilities
    Q_PROPERTY(CapabilitiesStatus capabilitiesStatus READ capabilitiesStatus NOTIFY capabilitiesStatusChanged)

    // Login status
    Q_PROPERTY(LoginStatus loginStatus READ loginStatus NOTIFY loginStatusChanged)

public:
    explicit NextcloudApi(QObject *parent = nullptr);
    virtual ~NextcloudApi();

    // Status codes
    enum NextcloudStatus {
        NextcloudUnknown,           // Initial unknown state
        NextcloudBusy,              // Getting information from the nextcloud server
        NextcloudSuccess,           // Got information about the nextcloud server
        NextcloudFailed             // Error getting information from the nextcloud server, see ErrorCodes
    };
    Q_ENUM(NextcloudStatus)

    enum CapabilitiesStatus {
        CapabilitiesUnknown,        // Initial unknown state
        CapabilitiesBusy,           // Gettin information
        CapabilitiesSuccess,        // Capabilities successfully read
        CapabilitiesFailed          // Faild to retreive capabilities
    };
    Q_ENUM(CapabilitiesStatus)

    enum LoginStatus {
        LoginUnknown,               // Inital unknown state
        LoginFlowV2Initiating,      // Initiating login flow v2
        LoginFlowV2Polling,         // Ready for login flow v2
        LoginFlowV2Success,         // Finished login flow v2
        LoginFlowV2Failed,          // An error in login flow v2
        LoginSuccess,               // Login has been verified successfull
        LoginFailed                 // Login has failed, see ErrorCodes
    };
    Q_ENUM(LoginStatus)

    // Generic API properties
    bool verifySsl() const { return m_authenticatedRequest.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer; }
    void setVerifySsl(bool verify);

    QUrl url() const { return m_url; }
    void setUrl(QUrl url);

    QString server() const;
    void setServer(QString server);

    QString scheme() const { return m_url.scheme(); }
    void setScheme(QString scheme);

    QString host() const { return m_url.host(); }
    void setHost(QString host);

    int port() const { return m_url.port(); }
    void setPort(int port);

    QString path() const { return m_url.path(); }
    void setPath(QString path);

    QString username() const { return m_url.userName(); }
    void setUsername(QString username);

    QString password() const { return m_url.password(); }
    void setPassword(QString password);

    // Class status information
    bool urlValid() const { return m_url.isValid(); }
    bool networkAccessible() const { return m_manager.networkAccessible() == QNetworkAccessManager::Accessible; }
    bool busy() const;

    // Nextcloud status (status.php)
    NextcloudStatus ncStatusStatus() const { return m_statusStatus; }
    bool statusInstalled() const { return m_status_installed; }
    bool statusMaintenance() const { return m_status_maintenance; }
    bool statusNeedsDbUpgrade() const { return m_status_needsDbUpgrade; }
    QString statusVersion() const { return m_status_version.toString(); }
    QString statusVersionString() const { return m_status_versionstring; }
    QString statusEdition() const { return m_status_edition; }
    QString statusProductName() const { return m_status_productname; }
    bool statusExtendedSupport() const { return m_status_extendedSupport; }
    bool loginFlowV2Possible() const { return QVersionNumber::fromString(statusVersion()) >= QVersionNumber(LOGIN_FLOWV2_MIN_VERSION); }

    // Nextcloud capabilities
    CapabilitiesStatus capabilitiesStatus() const { return m_capabilitiesStatus; }

    // Login status
    LoginStatus loginStatus() const { return m_loginStatus; }

    enum ErrorCodes {
        NoError,
        NoConnectionError,
        CommunicationError,
        SslHandshakeError,
        AuthenticationError
    };
    Q_ENUM(ErrorCodes)
    Q_INVOKABLE const QString errorMessage(int error) const;

public slots:
    // Callable functions
    Q_INVOKABLE bool getNcStatus();
    Q_INVOKABLE bool initiateFlowV2Login();
    Q_INVOKABLE void abortFlowV2Login();
    Q_INVOKABLE void verifyLogin();
    Q_INVOKABLE void getAppPassword();
    Q_INVOKABLE void deleteAppPassword();

    // API helper functions
    Q_INVOKABLE bool get(const QString& endpoint, bool authenticated = true);
    Q_INVOKABLE bool post(const QString& endpoint, bool authenticated = true);
    Q_INVOKABLE bool put(const QString& endpoint, bool authenticated = true);
    Q_INVOKABLE bool del(const QString& endpoint, bool authenticated = true);

signals:
    // Generic API properties
    void verifySslChanged(bool verify);
    void urlChanged(QUrl url);
    void serverChanged(QString server);
    void schemeChanged(QString scheme);
    void hostChanged(QString host);
    void portChanged(int port);
    void pathChanged(QString path);
    void usernameChanged(QString username);
    void passwordChanged(QString password);

    // Class status information
    void urlValidChanged(bool valid);
    void networkAccessibleChanged(bool accessible);
    void busyChanged(bool busy);

    // Nextcloud capabilities
    void capabilitiesStatusChanged(CapabilitiesStatus status);

    // Nextcloud status (status.php)
    void ncStatusStatusChanged(NextcloudStatus status);
    void statusInstalledChanged(bool installed);
    void statusMaintenanceChanged(bool maintenance);
    void statusNeedsDbUpgradeChanged(bool needsDbUpgrade);
    void statusVersionChanged(QString version);
    void statusVersionStringChanged(QString versionString);
    void statusEditionChanged(QString edition);
    void statusProductNameChanged(QString productName);
    void statusExtendedSupportChanged(bool extendedSupport);

    // Login status
    void loginStatusChanged(LoginStatus status);

    // API helper updates
    void getFinished(QNetworkReply& reply);
    void postFinished(QNetworkReply& reply);
    void putFinished(QNetworkReply& reply);
    void delFinished(QNetworkReply& reply);
    void apiError(ErrorCodes error);

private slots:
    void verifyUrl(QUrl url);
    void requireAuthentication(QNetworkReply * reply, QAuthenticator * authenticator);
    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void replyFinished(QNetworkReply* reply);
    void pollLoginUrl();
    void sslError(QNetworkReply* reply, const QList<QSslError> &errors);

private:
    QUrl m_url;
    QNetworkAccessManager m_manager;
    QNetworkRequest m_request;
    QNetworkRequest m_authenticatedRequest;

    // Nextcloud status.php
    void updateStatus(const QJsonObject &status);
    void setNextcloudStatus(NextcloudStatus status, bool *changed = NULL);
    NextcloudStatus m_statusStatus;
    bool m_status_installed;
    bool m_status_maintenance;
    bool m_status_needsDbUpgrade;
    QVersionNumber m_status_version;
    QString m_status_versionstring;
    QString m_status_edition;
    QString m_status_productname;
    bool m_status_extendedSupport;

    // Nextcloud capabilities
    bool updateCapabilities(const QJsonObject & capabilities);
    void setCababilitiesStatus(CapabilitiesStatus status, bool *changed = NULL);
    CapabilitiesStatus m_capabilitiesStatus;
    QJsonObject m_capabilities;

    // Nextcloud Login Flow v2
    // https://docs.nextcloud.com/server/latest/developer_manual/client_apis/LoginFlow/index.html#login-flow-v2
    bool updateLoginFlow(const QJsonObject &login);
    bool updateLoginCredentials(const QJsonObject &credentials);
    void setLoginStatus(LoginStatus status, bool *changed = NULL);
    LoginStatus m_loginStatus;
    QTimer m_loginPollTimer;
    QUrl m_loginUrl;
    QUrl m_pollUrl;
    QString m_pollToken;
};

#endif // NEXTCLOUDAPI_H
