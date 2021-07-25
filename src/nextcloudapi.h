#ifndef NEXTCLOUDAPI_H
#define NEXTCLOUDAPI_H

#include <QObject>
#include <QQmlEngine>
#include <QJSEngine>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVersionNumber>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QFile>
#include <QTimer>
#include <QDebug>

// Nextcloud instance information
const QString STATUS_ENDPOINT("/status.php");

// OCS APIs endpoints
// https://docs.nextcloud.com/server/latest/developer_manual/client_apis/OCS/ocs-api-overview.html
const QString USER_METADATA_ENDPOINT("/ocs/v2.php/cloud/users/%1");
const QString LIST_USERS_ENDPOINT("/ocs/v2.php/cloud/users");
const QString CAPABILITIES_ENDPOINT("/ocs/v2.php/cloud/capabilities");
const QString DIRECT_DOWNLOAD_ENDPOINT("/ocs/v2.php/apps/dav/api/v1/direct");
const QString USER_NOTIFICATION_ENDPOINT("/ocs/v2.php/apps/notifications/api/v2/notifications");

// Login Flow endpoints
// https://docs.nextcloud.com/server/latest/developer_manual/client_apis/LoginFlow/index.html
const QString GET_APPPASSWORD_ENDPOINT("/ocs/v2.php/core/getapppassword");
const QString DEL_APPPASSWORD_ENDPOINT("/ocs/v2.php/core/apppassword");
const QString LOGIN_FLOWV2_ENDPOINT("/index.php/login/v2");
const int LOGIN_FLOWV2_MIN_VERSION = 16;
const int LOGIN_FLOWV2_POLL_INTERVALL = 5000;

class NextcloudApi;
typedef void (NextcloudApi::*updateAppCapabilities)(const QJsonObject &newObject, const QJsonObject &preObject);

class NextcloudApi : public QObject
{
    Q_OBJECT

    // Generic API properties
    Q_PROPERTY(bool verifySsl READ verifySsl WRITE setVerifySsl NOTIFY verifySslChanged)    // to allow selfsigned certificates
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)    // complete nextcloud URL = <scheme>://<username>:<password>@<host>[:<port>]/<path>
    Q_PROPERTY(QString server READ server WRITE setServer NOTIFY urlChanged) // url without username and password = <scheme>://<host>[:<port>]/<path>
    // the following properties will update the url and server properties and vice versa
    Q_PROPERTY(QString scheme READ scheme WRITE setScheme NOTIFY urlChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY urlChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY urlChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY urlChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY urlChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY urlChanged)

    // Networking status information
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)   // when all needed properties are set
    Q_PROPERTY(bool urlValid READ urlValid NOTIFY urlValidChanged)  // if the property url is valid
    Q_PROPERTY(bool networkAccessible READ networkAccessible NOTIFY networkAccessibleChanged)   // when the device has connectivity
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)  // when an API call is currently running

    // Nextcloud status (status.php), these properties will be automatically updated on changes of the generic properties
    Q_PROPERTY(ApiCallStatus statusStatus READ statusStatus NOTIFY statusStatusChanged)
    Q_PROPERTY(bool statusInstalled READ statusInstalled NOTIFY statusChanged)
    Q_PROPERTY(bool statusMaintenance READ statusMaintenance NOTIFY statusChanged)
    Q_PROPERTY(bool statusNeedsDbUpgrade READ statusNeedsDbUpgrade NOTIFY statusChanged)
    Q_PROPERTY(QString statusVersion READ statusVersion NOTIFY statusChanged)
    Q_PROPERTY(QString statusVersionString READ statusVersionString NOTIFY statusChanged)
    Q_PROPERTY(QString statusEdition READ statusEdition NOTIFY statusChanged)
    Q_PROPERTY(QString statusProductName READ statusProductName NOTIFY statusChanged)
    Q_PROPERTY(bool statusExtendedSupport READ statusExtendedSupport NOTIFY statusChanged)

    // Login status
    Q_PROPERTY(LoginStatus loginStatus READ loginStatus NOTIFY loginStatusChanged)
    Q_PROPERTY(bool loginFlowV2Possible READ loginFlowV2Possible NOTIFY loginFlowV2PossibleChanged)
    Q_PROPERTY(QUrl loginUrl READ loginUrl NOTIFY loginUrlChanged)  // will be set after initiateFlowV2Login() has been called. The URL needs to be opened in a browser or webview

    // User(s) status
    Q_PROPERTY(ApiCallStatus userListStatus READ userListStatus NOTIFY userListStatusChanged)
    Q_PROPERTY(ApiCallStatus userMetaStatus READ userMetaStatus NOTIFY userMetaStatusChanged)
    Q_PROPERTY(QStringList userList READ userList NOTIFY userListChanged)
    // TODO property for user metadata

    // Nextcloud capabilities
    Q_PROPERTY(ApiCallStatus capabilitiesStatus READ capabilitiesStatus NOTIFY capabilitiesStatusChanged)
    // TODO other property for server capabilities

public:
    explicit NextcloudApi(QObject *parent = nullptr);
    virtual ~NextcloudApi();

    // QML singleton
    static void instantiate(QObject *parent = nullptr);
    static NextcloudApi & getInstance();
    static QObject * provider(QQmlEngine *, QJSEngine *);

    // API reply format
    enum ReplyFormat {
        ReplyJSON,                  // The reply should be in JSON format
        ReplyXML                    // The reply should be in XML format
    };
    Q_ENUM(ReplyFormat)

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

    Q_INVOKABLE static const QString accountHash(const QString username, const QString url);

    // Class status information
    bool ready() const { return urlValid() && networkAccessible() && !busy() && statusInstalled() && !statusMaintenance() && loginStatus() == LoginSuccess; }
    bool urlValid() const { return m_url.isValid(); }
    bool networkAccessible() const { return m_manager.networkAccessible() == QNetworkAccessManager::Accessible; }
    bool busy() const { return m_replies.count() > 0; }

    // Nextcloud status (status.php)
    ApiCallStatus statusStatus() const { return m_statusStatus; }
    bool statusInstalled() const { return m_status.value("installed").toBool(); }
    bool statusMaintenance() const { return m_status.value("maintenance").toBool(); }
    bool statusNeedsDbUpgrade() const { return m_status.value("needsDbUpgrade").toBool(); }
    QString statusVersion() const { return m_status.value("version").toString(); }
    QString statusVersionString() const { return m_status.value("versionstring").toString(); }
    QString statusEdition() const { return m_status.value("edition").toString(); }
    QString statusProductName() const { return m_status.value("productname").toString(); }
    bool statusExtendedSupport() const { return m_status.value("extendedSupport").toBool(); }

    // Login status
    LoginStatus loginStatus() const { return m_loginStatus; }
    bool loginFlowV2Possible() const { return QVersionNumber::fromString(statusVersion()) >= QVersionNumber(LOGIN_FLOWV2_MIN_VERSION); }
    QUrl loginUrl() const { return m_loginUrl; }

    // User(s) status
    ApiCallStatus userListStatus() const { return m_userListStatus; }
    ApiCallStatus userMetaStatus() const { return m_userMetaStatus; }
    QStringList userList() const { return m_userList; }

    // Nextcloud capabilities
    ApiCallStatus capabilitiesStatus() const { return m_capabilitiesStatus; }
    bool notesAppInstalled() const;
    QStringList notesAppApiVersions() const;

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
    Q_INVOKABLE QNetworkReply* get(const QString& endpoint, const QUrlQuery& query = QUrlQuery(), int format = ReplyJSON, bool authenticated = true);
    Q_INVOKABLE QNetworkReply* put(const QString& endpoint, const QByteArray& data, int format = ReplyJSON, bool authenticated = true);
    Q_INVOKABLE QNetworkReply* post(const QString& endpoint, const QByteArray& data, int format = ReplyJSON, bool authenticated = true);
    Q_INVOKABLE QNetworkReply* del(const QString& endpoint, bool authenticated = true);

    // Callable functions
    Q_INVOKABLE bool getStatus();
    Q_INVOKABLE bool initiateFlowV2Login();
    Q_INVOKABLE void abortFlowV2Login();
    Q_INVOKABLE bool verifyLogin();
    Q_INVOKABLE bool getAppPassword();
    Q_INVOKABLE bool deleteAppPassword();
    Q_INVOKABLE bool getUserMetaData(const QString& user = QString());
    Q_INVOKABLE bool getUserList();
    Q_INVOKABLE bool getCapabilities();

    // Capabilities
    Q_INVOKABLE bool appInstalled(const QString& name) const;

signals:
    // Generic API properties
    void verifySslChanged(bool verify);
    void urlChanged(QUrl* url);

    // Class status information
    void readyChanged(bool ready);
    void urlValidChanged(bool valid);
    void networkAccessibleChanged(bool accessible);
    void busyChanged(bool busy);

    // Nextcloud status (status.php)
    void statusStatusChanged(ApiCallStatus status);
    void statusChanged();

    // Login status
    void loginStatusChanged(LoginStatus status);
    void loginFlowV2PossibleChanged(bool loginV2possible);
    void loginUrlChanged(QUrl* url);

    // User(s) status
    void userMetaStatusChanged(ApiCallStatus status);
    void userMetaChanged(QString username);
    void userListStatusChanged(ApiCallStatus status);
    void userListChanged(QStringList* users);

    // Nextcloud capabilities
    void capabilitiesStatusChanged(ApiCallStatus status);
    void capabilitiesChanged(QJsonObject* json);

    // API helper updates
    void apiFinished(QNetworkReply* reply);
    void apiError(ErrorCodes error);

private slots:
    void verifyUrl(QUrl* url);
    void requireAuthentication(QNetworkReply * reply, QAuthenticator * authenticator);
    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    bool pollLoginUrl();
    void sslError(QNetworkReply* reply, const QList<QSslError> &errors);
    void replyFinished(QNetworkReply* reply);

private:
    // QML singleton
    static NextcloudApi * instance;

    QUrl m_url;
    QNetworkAccessManager m_manager;
    QVector<QNetworkReply*> m_replies;
    QNetworkRequest m_request;
    QNetworkRequest m_authenticatedRequest;
    const QNetworkRequest prepareRequest(QUrl url, int format = ReplyJSON, bool authenticated = true) const;

    // Nextcloud status.php
    bool updateStatus(const QJsonObject &json);
    void setStatusStatus(ApiCallStatus status, bool *changed = NULL);
    ApiCallStatus m_statusStatus;
    QJsonObject m_status;

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

    // Nextcloud user metadata
    bool updateUserMeta(const QJsonObject &json);
    void setUserMetaStatus(ApiCallStatus status, bool *changed = NULL);
    ApiCallStatus m_userMetaStatus;
    QHash<QString, QJsonObject> m_userMeta;

    // Nextcloud users list
    bool updateUserList(const QJsonObject &json);
    void setUserListStatus(ApiCallStatus status, bool *changed = NULL);
    ApiCallStatus m_userListStatus;
    QStringList m_userList;

    // Nextcloud capabilities
    bool updateCapabilities(const QJsonObject &json);
    void setCababilitiesStatus(ApiCallStatus status, bool *changed = NULL);
    ApiCallStatus m_capabilitiesStatus;
    QJsonObject m_capabilities;
};

#endif // NEXTCLOUDAPI_H
