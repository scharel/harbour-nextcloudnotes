#include "nextcloudapi.h"
#include <QGuiApplication>
#include <QAuthenticator>

NextcloudApi::NextcloudApi(QObject *parent) : QObject(parent)
{
    // Initial status
    setStatusStatus(ApiCallStatus::ApiUnknown);
    setLoginStatus(LoginStatus::LoginUnknown);
    setCababilitiesStatus(ApiCallStatus::ApiUnknown);
    setUserListStatus(ApiCallStatus::ApiUnknown);
    setUserMetaStatus(ApiCallStatus::ApiUnknown);

    // Verify URL
    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(verifyUrl(QUrl)));

    // Login Flow V2 poll timer
    m_loginPollTimer.setInterval(LOGIN_FLOWV2_POLL_INTERVALL);
    connect(&m_loginPollTimer, SIGNAL(timeout()), this, SLOT(pollLoginUrl()));

    // Listen to signals of the QNetworkAccessManager class
    connect(&m_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(requireAuthentication(QNetworkReply*,QAuthenticator*)));
    connect(&m_manager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    connect(&m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(&m_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslError(QNetworkReply*,QList<QSslError>)));

    // Prepare the QNetworkRequest classes
    m_request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    m_request.setHeader(QNetworkRequest::UserAgentHeader, QGuiApplication::applicationDisplayName() +  " " + QGuiApplication::applicationVersion() + " - " + QSysInfo::machineHostName());
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/x-www-form-urlencoded").toUtf8());
    m_request.setRawHeader("OCS-APIRequest", "true");
    m_request.setRawHeader("Accept-Language", QLocale::system().bcp47Name().toUtf8());
    //m_request.setRawHeader("Accept", "application/json");
    m_authenticatedRequest = m_request;
    m_authenticatedRequest.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json").toUtf8());
}

NextcloudApi::~NextcloudApi() {
    while (!m_replies.empty()) {
        QNetworkReply* reply = m_replies.first();
        reply->abort();
        reply->deleteLater();
        m_replies.removeFirst();
    }
}

// QML singleton
NextcloudApi * NextcloudApi::instance = nullptr;

void NextcloudApi::instantiate(QObject *parent) {
    if (instance == nullptr) {
        instance = new NextcloudApi(parent);
    }
}

NextcloudApi & NextcloudApi::getInstance() {
    return *instance;
}

QObject * NextcloudApi::provider(QQmlEngine *, QJSEngine *) {
    return instance;
}

void NextcloudApi::setVerifySsl(bool verify) {
    if (verify != (m_request.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer) ||
        verify != (m_authenticatedRequest.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer)) {
        m_request.sslConfiguration().setPeerVerifyMode(verify ? QSslSocket::VerifyPeer : QSslSocket::VerifyNone);
        m_authenticatedRequest.sslConfiguration().setPeerVerifyMode(verify ? QSslSocket::VerifyPeer : QSslSocket::VerifyNone);
        emit verifySslChanged(verify);
    }
}

void NextcloudApi::setUrl(QUrl url) {
    if (url != m_url) {
        setScheme(url.scheme());
        setHost(url.host());
        setPort(url.port());
        setUsername(url.userName());
        setPassword(url.password());
        setPath(url.path());
        emit urlChanged(&m_url);
    }
}

QString NextcloudApi::server() const {
    QUrl server;
    server.setScheme(m_url.scheme());
    server.setHost(m_url.host());
    if (m_url.port() > 0)
        server.setPort(m_url.port());
    server.setPath(m_url.path());
    return server.toString();
}

void NextcloudApi::setServer(QString serverUrl) {
    QUrl url(serverUrl.trimmed());
    if (url != server()) {
        setScheme(url.scheme());
        setHost(url.host());
        setPort(url.port());
        setPath(url.path());
    }
}

void NextcloudApi::setScheme(QString scheme) {
    if (scheme != m_url.scheme() && (scheme == "http" || scheme == "https")) {
        m_url.setScheme(scheme);
        emit urlChanged(&m_url);
    }
}

void NextcloudApi::setHost(QString host) {
    if (host != m_url.host()) {
        m_url.setHost(host);
        emit urlChanged(&m_url);
    }
}

void NextcloudApi::setPort(int port) {
    if (port != m_url.port() && port >= 1 && port <= 65535) {
        m_url.setPort(port);
        emit urlChanged(&m_url);
    }
}

void NextcloudApi::setPath(QString path) {
    if (path != m_url.path()) {
        m_url.setPath(path);
        emit urlChanged(&m_url);
    }
}

void NextcloudApi::setUsername(QString username) {
    if (username != m_url.userName()) {
        m_url.setUserName(username);
        QString concatenated = username + ":" + password();
        QByteArray data = concatenated.toLocal8Bit().toBase64();
        QString headerData = "Basic " + data;
        m_authenticatedRequest.setRawHeader("Authorization", headerData.toLocal8Bit());
        emit urlChanged(&m_url);
    }
}

void NextcloudApi::setPassword(QString password) {
    if (password != m_url.password()) {
        m_url.setPassword(password);
        QString concatenated = username() + ":" + password;
        QByteArray data = concatenated.toLocal8Bit().toBase64();
        QString headerData = "Basic " + data;
        m_authenticatedRequest.setRawHeader("Authorization", headerData.toLocal8Bit());
        emit urlChanged(&m_url);
    }
}

bool NextcloudApi::appInstalled(const QString& name) const {
    QJsonObject app = m_capabilities.value(name).toObject();
    return !app.isEmpty();
}

QStringList NextcloudApi::notesAppApiVersions() const {
    QStringList versions;
    QJsonObject notes = m_capabilities.value("notes").toObject();
    if (!notes.isEmpty()) {
        QJsonArray apiVersion = notes.value("api_version").toArray();
        QJsonArray::const_iterator i;
        for (i = apiVersion.constBegin(); i != apiVersion.constEnd(); ++i) {
            if (i->isString())
                versions << i->toString();
        }
    }
    return versions;
}

const QString NextcloudApi::errorMessage(int error) const {
    QString message;
    switch (error) {
    case NoError:
        message = tr("No error");
        break;
    case NoConnectionError:
        message = tr("No network connection available");
        break;
    case CommunicationError:
        message = tr("Failed to communicate with the Nextcloud server");
        break;
    case SslHandshakeError:
        message = tr("An error occured while establishing an encrypted connection");
        break;
    case AuthenticationError:
        message = tr("Could not authenticate to the Nextcloud instance");
        break;
    default:
        message = tr("Unknown error");
        break;
    }
    return message;
}

const QNetworkRequest NextcloudApi::prepareRequest(QUrl url, int format, bool authenticated) const {
    QNetworkRequest request;
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        request = authenticated ? m_authenticatedRequest : m_request;
        request.setUrl(url);
        switch (format) {
        case ReplyJSON:
            request.setRawHeader("Accept", "application/json");
            break;
        case ReplyXML:
            request.setRawHeader("Accept", "application/json");
            break;
        }
    }
    return request;
}

QNetworkReply* NextcloudApi::get(const QString& endpoint, const QUrlQuery& query, int format, bool authenticated) {
    QUrl url = server();
    url.setPath(url.path() + endpoint);
    url.setQuery(query);
    qDebug() << "GET" << url.toDisplayString();
    return m_manager.get(prepareRequest(url, format, authenticated));
}

QNetworkReply* NextcloudApi::put(const QString& endpoint, const QByteArray& data, int format, bool authenticated) {
    QUrl url = server();
    url.setPath(url.path() + endpoint);
    qDebug() << "PUT" << url.toDisplayString();
    qDebug() << data;
    return m_manager.put(prepareRequest(url, format, authenticated), data);
}

QNetworkReply* NextcloudApi::post(const QString& endpoint, const QByteArray& data, int format, bool authenticated) {
    QUrl url = server();
    url.setPath(url.path() + endpoint);
    qDebug() << "POST" << url.toDisplayString();
    qDebug() << data;
    return m_manager.post(prepareRequest(url, format, authenticated), data);
}

QNetworkReply* NextcloudApi::del(const QString& endpoint, bool authenticated) {
    QUrl url = server();
    url.setPath(url.path() + endpoint);
    qDebug() << "DEL" << url.toDisplayString();
    return m_manager.deleteResource(prepareRequest(url, authenticated));
}

bool NextcloudApi::getStatus() {
    if (get(STATUS_ENDPOINT, QUrlQuery(), ReplyJSON, false)) {
        setStatusStatus(ApiCallStatus::ApiBusy);
        return true;
    }
    else {
        setStatusStatus(ApiCallStatus::ApiFailed);
    }
    return false;
}

bool NextcloudApi::initiateFlowV2Login() {
    if (m_loginStatus == LoginStatus::LoginFlowV2Initiating || m_loginStatus == LoginStatus::LoginFlowV2Polling) {
        abortFlowV2Login();
    }
    if (post(LOGIN_FLOWV2_ENDPOINT, QByteArray(), false)) {
        setLoginStatus(LoginStatus::LoginFlowV2Initiating);
        return true;
    }
    else {
        setLoginStatus(LoginStatus::LoginFlowV2Failed);
        abortFlowV2Login();
    }
    return false;
}

void NextcloudApi::abortFlowV2Login() {
    m_loginPollTimer.stop();
    m_loginUrl.clear();
    m_pollUrl.clear();
    m_pollToken.clear();
    setLoginStatus(LoginStatus::LoginUnknown);
}

bool NextcloudApi::verifyLogin() {
    return getUserMetaData(username());
}

bool NextcloudApi::getAppPassword() {
    return get(GET_APPPASSWORD_ENDPOINT);
}

bool NextcloudApi::deleteAppPassword() {
    return del(DEL_APPPASSWORD_ENDPOINT);
}

bool NextcloudApi::getUserMetaData(const QString& user) {
    if (!user.isEmpty() || !username().isEmpty())
        return get(USER_METADATA_ENDPOINT.arg(user.isEmpty() ? username() : user));
    return false;
}

bool NextcloudApi::getUserList() {
    return get(LIST_USERS_ENDPOINT);
}

bool NextcloudApi::getCapabilities() {
    return get(CAPABILITIES_ENDPOINT);
}

void NextcloudApi::verifyUrl(QUrl *url) {
    emit urlValidChanged(
                url->isValid()&&
                !url->isRelative() &&
                !url->userName().isEmpty() &&
                !url->password().isEmpty() &&
                !url->host().isEmpty());
}

void NextcloudApi::requireAuthentication(QNetworkReply *reply, QAuthenticator *authenticator) {
    if (reply && authenticator) {
        authenticator->setUser(username());
        authenticator->setPassword(password());
    }
    else
        emit apiError(AuthenticationError);
}

void NextcloudApi::onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible) {
    emit networkAccessibleChanged(accessible == QNetworkAccessManager::Accessible);
}

bool NextcloudApi::pollLoginUrl() {
    if (post(m_pollUrl.path(), QByteArray("token=").append(m_pollToken), false)) {
        setLoginStatus(LoginStatus::LoginFlowV2Polling);
        return true;
    }
    else {
        setLoginStatus(LoginStatus::LoginFlowV2Failed);
        abortFlowV2Login();
    }
    return false;
}

void NextcloudApi::sslError(QNetworkReply *reply, const QList<QSslError> &errors) {
    qDebug() << "SSL errors accured while calling" << reply->url().toDisplayString();
    for (int i = 0; i < errors.size(); ++i) {
        qDebug() << errors[i].errorString();
    }
    emit apiError(SslHandshakeError);
}

void NextcloudApi::replyFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError)
        qDebug() << reply->error() << reply->errorString();

    QByteArray data = reply->readAll();
    QJsonDocument json = QJsonDocument::fromJson(data);
    //qDebug() << data;

    switch (reply->error()) {
    case QNetworkReply::NoError:
        emit apiError(NoError);
        switch (reply->operation()) {
        case QNetworkAccessManager::GetOperation:
            if (reply->url().toString().endsWith(STATUS_ENDPOINT)) {
                qDebug() << "Nextcloud status.php";
                updateStatus(json.object());
            }
            else if (reply->url().toString().endsWith(GET_APPPASSWORD_ENDPOINT)) {
                qDebug() << "App password received";
                updateAppPassword(json.object());
            }
            else if (reply->url().toString().contains(USER_METADATA_ENDPOINT)) {
                qDebug() << "User metadata for" << reply->url().toString().split('/').last() << "received";
                updateUserMeta(json.object());
            }
            else if (reply->url().toString().endsWith(LIST_USERS_ENDPOINT)) {
                qDebug() << "User list received";
                updateUserList(json.object());
            }
            else if (reply->url().toString().endsWith(CAPABILITIES_ENDPOINT)) {
                qDebug() << "Capabilites received";
                updateCapabilities(json.object());
            }
            else if (reply->url().toString().endsWith(USER_NOTIFICATION_ENDPOINT)) {
                qDebug() << "Notifications are not yet implemented!";
                qDebug() << json.object().value("ocs").toObject().value("data").toArray();
            }
            else {
                qDebug() << "GET reply received";
                break;
            }
            m_replies.removeOne(reply);
            reply->deleteLater();
            break;
        case QNetworkAccessManager::PutOperation:
            if (reply->url().toString().endsWith(DIRECT_DOWNLOAD_ENDPOINT)) {
                qDebug() << "Direct download is not yet implemented!";
            }
            else {
                qDebug() << "PUT reply received";
                break;
            }
            m_replies.removeOne(reply);
            reply->deleteLater();
            break;
        case QNetworkAccessManager::PostOperation:
            if (reply->url().toString().endsWith(LOGIN_FLOWV2_ENDPOINT)) {
                qDebug() << "Login Flow v2 initiated.";
                updateLoginFlow(json.object());
            }
            else if (reply->url() == m_pollUrl) {
                qDebug() << "Login Flow v2 finished.";
                updateLoginCredentials(json.object());
            }
            else {
                qDebug() << "POST reply received";
                break;
            }
            m_replies.removeOne(reply);
            reply->deleteLater();
            break;
        case QNetworkAccessManager::DeleteOperation:
            if (reply->url().toString().endsWith(DEL_APPPASSWORD_ENDPOINT)) {
                deleteAppPassword(json.object());
            }
            else {
                qDebug() << "DELETE reply received";
                break;
            }
            m_replies.removeOne(reply);
            reply->deleteLater();
            break;
        default:
            qDebug() << "Unknown reply received:" << reply->operation() << reply->url();
            m_replies.removeOne(reply);
            reply->deleteLater();
            break;
        }
        emit apiFinished(reply);
        break;
    case QNetworkReply::AuthenticationRequiredError:
        qDebug() << reply->errorString();
        emit apiError(AuthenticationError);
        break;
    case QNetworkReply::ContentNotFoundError:
        if (reply->url() == m_pollUrl) {
            qDebug() << "Login Flow v2 not finished yet.";
            emit apiError(NoError);
        }
        else {
            qDebug() << reply->errorString();
            emit apiError(CommunicationError);
        }
        break;
    default:
        qDebug() << reply->errorString();
        emit apiError(CommunicationError);
        break;
    }
}

void NextcloudApi::setStatusStatus(ApiCallStatus status, bool *changed) {
    if (status != m_statusStatus) {
        m_statusStatus = status;
        if (changed) *changed = true;
        emit statusStatusChanged(m_statusStatus);
    }
}

bool NextcloudApi::updateStatus(const QJsonObject &json) {
    if (!json.isEmpty()) {
        setStatusStatus(ApiSuccess);
        if (json != m_status) {
            m_status = json;
            emit statusChanged();
        }
        return true;
    }
    else {
        setStatusStatus(ApiFailed);
    }
    return false;
}

void NextcloudApi::setLoginStatus(LoginStatus status, bool *changed) {
    if (status != m_loginStatus) {
        m_loginStatus = status;
        if (changed) *changed = true;
        emit loginStatusChanged(m_loginStatus);
    }
}

bool NextcloudApi::updateLoginFlow(const QJsonObject &json) {
    if (!json.isEmpty()) {
        QUrl loginUrl = json.value("login").toString();
        QJsonObject poll = json.value("poll").toObject();
        m_pollUrl = poll.value("endpoint").toString();
        m_pollToken = poll.value("token").toString();

        if (m_pollUrl.isValid() && !m_pollToken.isEmpty() && loginUrl.isValid()) {
            if (m_loginUrl != loginUrl) {
                m_loginUrl = loginUrl;
                emit loginUrlChanged(&m_loginUrl);
            }
            setLoginStatus(LoginStatus::LoginFlowV2Polling);
            m_loginPollTimer.start();
            return true;
        }
    }
    else {
        setLoginStatus(LoginStatus::LoginFlowV2Failed);
        abortFlowV2Login();
    }
    return false;
}

bool NextcloudApi::updateLoginCredentials(const QJsonObject &json) {
    QString serverAddr;
    QString loginName;
    QString appPassword;
    if (!json.isEmpty()) {
        serverAddr = json.value("server").toString();
        if (!serverAddr.isEmpty()) setServer(serverAddr);
        loginName = json.value("loginName").toString();
        if (!loginName.isEmpty()) setUsername(loginName);
        appPassword = json.value("appPassword").toString();
        if (!appPassword.isEmpty()) setPassword(appPassword);
    }
    if (!serverAddr.isEmpty() && !loginName.isEmpty() && !appPassword.isEmpty()) {
        abortFlowV2Login();
        qDebug() << "Login successfull for user" << loginName << "on" << serverAddr;
        setLoginStatus(LoginStatus::LoginFlowV2Success);
        return true;
    }
    qDebug() << "Login Flow v2 failed!";
    return false;
}

bool NextcloudApi::updateAppPassword(const QJsonObject &json) {
    QJsonObject ocs = json.value("ocs").toObject();
    QJsonObject data = ocs.value("data").toObject();
    QJsonValue password = data.value("apppassword");
    if (password.isString()) {
        setPassword(password.toString());
        return true;
    }
    return false;
}

bool NextcloudApi::deleteAppPassword(const QJsonObject &json) {
    setPassword(QString());
    return true;
}

void NextcloudApi::setUserMetaStatus(ApiCallStatus status, bool *changed) {
    if (status != m_userMetaStatus) {
        m_userMetaStatus = status;
        if (changed) *changed = true;
        emit userMetaStatusChanged(m_userMetaStatus);
    }
}

bool NextcloudApi::updateUserMeta(const QJsonObject &json) {
    QJsonObject ocs = json.value("ocs").toObject();
    QJsonObject meta = ocs.value("meta").toObject();
    QJsonObject data = ocs.value("data").toObject();
    QString user = data.value("id").toString();
    if (!user.isEmpty() && user == username()) {
        setLoginStatus(LoginSuccess);
    }
    if (meta.value("statuscode").toInt() == 100) {
        setUserMetaStatus(ApiSuccess);
        if (!data.isEmpty() && data != m_userMeta[user]) {
            m_userMeta[user] = data;
            emit userMetaChanged(user);
        }
        return true;
    }
    else if (meta.value("statuscode").toInt() == 404) {
        setUserMetaStatus(ApiSuccess);
        m_userMeta.remove(user);
        return true;
    }
    setUserMetaStatus(ApiFailed);
    return false;
}

void NextcloudApi::setUserListStatus(ApiCallStatus status, bool *changed) {
    if (status != m_userListStatus) {
        m_userListStatus = status;
        if (changed) *changed = true;
        emit userListStatusChanged(m_userListStatus);
    }
}

bool NextcloudApi::updateUserList(const QJsonObject &json) {
    QJsonObject ocs = json.value("ocs").toObject();
    QJsonObject data = ocs.value("data").toObject();
    QJsonValue list = data.value("users");
    if (list.isArray()) {
        QJsonArray array = list.toArray();
        QStringList userList;
        for (int i = 0; i < array.size(); ++i) {
            QString user = array.at(0).toString();
            if (!m_userList.contains(user) && !user.isEmpty()) {
                userList << user;
            }
        }
        if (userList != m_userList && !userList.isEmpty()) {
            setUserListStatus(ApiSuccess);
            m_userList = userList;
            emit userListChanged(&m_userList);
        }
        return true;
    }
    setUserListStatus(ApiFailed);
    return false;
}

void NextcloudApi::setCababilitiesStatus(ApiCallStatus status, bool *changed) {
    if (status != m_capabilitiesStatus) {
        m_capabilitiesStatus = status;
        if (changed) *changed = true;
        emit capabilitiesStatusChanged(m_capabilitiesStatus);
    }
}

bool NextcloudApi::updateCapabilities(const QJsonObject &json) {
    QJsonObject ocs = json.value("ocs").toObject();
    QJsonObject data = ocs.value("data").toObject();
    QJsonObject capabilities = data.value("capabilities").toObject();
    // TODO update version
    if (!capabilities.isEmpty()) {
        setCababilitiesStatus(ApiSuccess);
        if (capabilities != m_capabilities) {
            m_capabilities = capabilities;
            emit capabilitiesChanged(&m_capabilities);
        }
        return true;
    }
    setCababilitiesStatus(ApiFailed);
    return false;
}
