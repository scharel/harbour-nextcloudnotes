#ifndef NEXTCLOUDAPI_H
#define NEXTCLOUDAPI_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
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
const QString CAPABILITIES_ENDPOINT("/ocs/v2.php/cloud/capabilities");
const QString LIST_USERS_ENDPOINT("/ocs/v2.php/cloud/users");
const QString USER_METADATA_ENDPOINT("/ocs/v2.php/cloud/users/%1");
const QString USER_NOTIFICATION_ENDPOINT("/ocs/v2.php/cloud/capabilities");

// Login and authentication
const QString GET_APPPASSWORD_ENDPOINT("/ocs/v2.php/core/getapppassword");
const QString DEL_APPPASSWORD_ENDPOINT("/ocs/v2.php/core/apppassword");
const QString LOGIN_FLOWV2_ENDPOINT("/index.php/login/v2");
const int LOGIN_FLOWV2_MIN_VERSION = 16;
const int LOGIN_FLOWV2_POLL_INTERVALL = 5000;

// Diredct Download
const QString DIRECT_DOWNLOAD_ENDPOINT("/ocs/v2.php/apps/dav/api/v1/direct");

class NextcloudApi : public QObject
{
    Q_OBJECT

    // Generic API properties
    Q_PROPERTY(bool verifySsl READ verifySsl WRITE setVerifySsl NOTIFY verifySslChanged)    // to allow selfsigned certificates
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)    // complete API URL = <scheme>://<username>:<password>@<host>[:<port>]/<path>
    Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged) // url without username and password = <scheme>://<host>[:<port>]/<path>
    // the following six properties will update the url and server properties and vice versa
    Q_PROPERTY(QString scheme READ scheme WRITE setScheme NOTIFY schemeChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)

    // Networking status information
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(bool urlValid READ urlValid NOTIFY urlValidChanged)
    Q_PROPERTY(bool networkAccessible READ networkAccessible NOTIFY networkAccessibleChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

    // Nextcloud status (status.php), these properties will be automatically updated on changes of the generic properties
    Q_PROPERTY(ApiCallStatus statusStatus READ statusStatus NOTIFY statusStatusChanged)
    Q_PROPERTY(bool statusInstalled READ statusInstalled NOTIFY statusInstalledChanged)
    Q_PROPERTY(bool statusMaintenance READ statusMaintenance NOTIFY statusMaintenanceChanged)
    Q_PROPERTY(bool statusNeedsDbUpgrade READ statusNeedsDbUpgrade NOTIFY statusNeedsDbUpgradeChanged)
    Q_PROPERTY(QString statusVersion READ statusVersion NOTIFY statusVersionChanged)
    Q_PROPERTY(QString statusVersionString READ statusVersionString NOTIFY statusVersionStringChanged)
    Q_PROPERTY(QString statusEdition READ statusEdition NOTIFY statusEditionChanged)
    Q_PROPERTY(QString statusProductName READ statusProductName NOTIFY statusProductNameChanged)
    Q_PROPERTY(bool statusExtendedSupport READ statusExtendedSupport NOTIFY statusExtendedSupportChanged)

    // Login status
    Q_PROPERTY(bool loginFlowV2Possible READ loginFlowV2Possible NOTIFY loginFlowV2PossibleChanged)
    Q_PROPERTY(QUrl loginUrl READ loginUrl NOTIFY loginUrlChanged)
    Q_PROPERTY(ApiCallStatus loginStatus READ loginStatus NOTIFY loginStatusChanged)

    // User(s) status
    Q_PROPERTY(ApiCallStatus userListStatus READ userListStatus NOTIFY userListStatusChanged)
    Q_PROPERTY(QStringList userList READ userList NOTIFY userListChanged)
    Q_PROPERTY(ApiCallStatus userMetaStatus READ userMetaStatus NOTIFY userMetaStatusChanged)

    // Nextcloud capabilities
    Q_PROPERTY(ApiCallStatus capabilitiesStatus READ capabilitiesStatus NOTIFY capabilitiesStatusChanged)

public:
    explicit NextcloudApi(QObject *parent = nullptr);
    virtual ~NextcloudApi();

    // Status codes
    enum ApiCallStatus {
        ApiUnknown,                 // Initial unknown state
        ApiBusy,                    // Getting information from the nextcloud server
        ApiSuccess,                 // Got information from the nextcloud server
        ApiFailed                   // Error getting information from the nextcloud server, see ErrorCodes
    };
    Q_ENUM(ApiCallStatus)

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
    bool ready() const { return urlValid() && networkAccessible() && !busy() && statusInstalled() && !statusMaintenance() && loginStatus() == LoginSuccess; }
    bool urlValid() const { return m_url.isValid(); }
    bool networkAccessible() const { return m_manager.networkAccessible() == QNetworkAccessManager::Accessible; }
    bool busy() const { return m_replies.count() > 0; }

    // Nextcloud status (status.php)
    ApiCallStatus statusStatus() const { return m_statusStatus; }
    bool statusInstalled() const { return m_status_installed; }
    bool statusMaintenance() const { return m_status_maintenance; }
    bool statusNeedsDbUpgrade() const { return m_status_needsDbUpgrade; }
    QString statusVersion() const { return m_status_version.toString(); }
    QString statusVersionString() const { return m_status_versionstring; }
    QString statusEdition() const { return m_status_edition; }
    QString statusProductName() const { return m_status_productname; }
    bool statusExtendedSupport() const { return m_status_extendedSupport; }

    // Login status
    bool loginFlowV2Possible() const { return QVersionNumber::fromString(statusVersion()) >= QVersionNumber(LOGIN_FLOWV2_MIN_VERSION); }
    QUrl loginUrl() const { return m_loginUrl; }
    LoginStatus loginStatus() const { return m_loginStatus; }

    // User(s) status
    ApiCallStatus userListStatus() const { return m_userListStatus; }
    QStringList userList() const { return m_userList; }
    ApiCallStatus userMetaStatus() const { return m_userMetaStatus; }

    // Nextcloud capabilities
    ApiCallStatus capabilitiesStatus() const { return m_capabilitiesStatus; }

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
    // API helper functions
    Q_INVOKABLE bool get(const QString& endpoint, bool authenticated = true);
    Q_INVOKABLE bool put(const QString& endpoint, const QByteArray& data, bool authenticated = true);
    Q_INVOKABLE bool post(const QString& endpoint, const QByteArray& data, bool authenticated = true);
    Q_INVOKABLE bool del(const QString& endpoint, bool authenticated = true);

    // Callable functions
    Q_INVOKABLE bool getStatus();
    Q_INVOKABLE bool initiateFlowV2Login();
    Q_INVOKABLE void abortFlowV2Login();
    Q_INVOKABLE bool verifyLogin();
    Q_INVOKABLE bool getAppPassword();
    Q_INVOKABLE bool deleteAppPassword();
    Q_INVOKABLE bool getUserList();
    Q_INVOKABLE bool getUserMetaData(const QString& user = QString());
    Q_INVOKABLE bool getCapabilities();

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
    void readyChanged(bool ready);
    void urlValidChanged(bool valid);
    void networkAccessibleChanged(bool accessible);
    void busyChanged(bool busy);

    // Nextcloud status (status.php)
    void statusStatusChanged(ApiCallStatus status);
    void statusInstalledChanged(bool installed);
    void statusMaintenanceChanged(bool maintenance);
    void statusNeedsDbUpgradeChanged(bool needsDbUpgrade);
    void statusVersionChanged(QString version);
    void statusVersionStringChanged(QString versionString);
    void statusEditionChanged(QString edition);
    void statusProductNameChanged(QString productName);
    void statusExtendedSupportChanged(bool extendedSupport);

    // Login status
    void loginFlowV2PossibleChanged(bool loginV2possible);
    void loginUrlChanged(QUrl url);
    void loginStatusChanged(LoginStatus status);

    // User(s) status
    void userListStatusChanged(ApiCallStatus status);
    void userListChanged(QStringList users);
    void userMetaStatusChanged(ApiCallStatus status);

    // Nextcloud capabilities
    void capabilitiesStatusChanged(ApiCallStatus status);

    // API helper updates
    void getFinished(QNetworkReply* reply);
    void postFinished(QNetworkReply* reply);
    void putFinished(QNetworkReply* reply);
    void delFinished(QNetworkReply* reply);
    void apiError(ErrorCodes error);

private slots:
    void verifyUrl(QUrl url);
    void requireAuthentication(QNetworkReply * reply, QAuthenticator * authenticator);
    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    bool pollLoginUrl();
    void sslError(QNetworkReply* reply, const QList<QSslError> &errors);
    void replyFinished(QNetworkReply* reply);

private:
    QUrl m_url;
    QNetworkAccessManager m_manager;
    QVector<QNetworkReply*> m_replies;
    QNetworkRequest m_request;
    QNetworkRequest m_authenticatedRequest;

    // Nextcloud status.php
    bool updateStatus(const QJsonObject &json);
    void setStatusStatus(ApiCallStatus status, bool *changed = NULL);
    ApiCallStatus m_statusStatus;
    bool m_status_installed;
    bool m_status_maintenance;
    bool m_status_needsDbUpgrade;
    QVersionNumber m_status_version;
    QString m_status_versionstring;
    QString m_status_edition;
    QString m_status_productname;
    bool m_status_extendedSupport;

    // Nextcloud capabilities
    bool updateCapabilities(const QJsonObject &json);
    void setCababilitiesStatus(ApiCallStatus status, bool *changed = NULL);
    ApiCallStatus m_capabilitiesStatus;
    QJsonObject m_capabilities;

    // Nextcloud users list
    bool updateUserList(const QJsonObject &json);
    void setUserListStatus(ApiCallStatus status, bool *changed = NULL);
    ApiCallStatus m_userListStatus;
    QStringList m_userList;

    // Nextcloud user metadata
    bool updateUserMeta(const QJsonObject &json);
    void setUserMetaStatus(ApiCallStatus status, bool *changed = NULL);
    ApiCallStatus m_userMetaStatus;
    QHash<QString, QJsonObject> m_userMeta;

    // Nextcloud Login Flow v2
    // https://docs.nextcloud.com/server/latest/developer_manual/client_apis/LoginFlow/index.html#login-flow-v2
    bool updateLoginFlow(const QJsonObject &json);
    bool updateLoginCredentials(const QJsonObject &json);
    bool updateAppPassword(const QJsonObject &json);
    bool deleteAppPassword(const QJsonObject &json);
    void setLoginStatus(LoginStatus status, bool *changed = NULL);
    LoginStatus m_loginStatus;
    QTimer m_loginPollTimer;
    QUrl m_loginUrl;
    QUrl m_pollUrl;
    QString m_pollToken;
};

#endif // NEXTCLOUDAPI_H
