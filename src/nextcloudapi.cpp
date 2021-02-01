#include "nextcloudapi.h"
#include <QGuiApplication>
#include <QAuthenticator>
#include <QJsonDocument>

NextcloudApi::NextcloudApi(QObject *parent) : QObject(parent)
{
    // Initial status
    setStatus(ApiCallStatus::ApiUnknown);
    setLoginStatus(LoginStatus::LoginUnknown);
    setCababilitiesStatus(ApiCallStatus::ApiUnknown);
    setUserListStatus(ApiCallStatus::ApiUnknown);
    setUserMetaStatus(ApiCallStatus::ApiUnknown);
    m_status_installed = false;
    m_status_maintenance = false;
    m_status_needsDbUpgrade = false;
    m_status_extendedSupport = false;

    // Login Flow V2 poll timer
    m_loginPollTimer.setInterval(LOGIN_FLOWV2_POLL_INTERVALL);
    connect(&m_loginPollTimer, SIGNAL(timeout()), this, SLOT(pollLoginUrl()));

    // Verify URL
    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(verifyUrl(QUrl)));

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
    m_request.setRawHeader("Accept", "application/json");
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
        QString oldServer = server();
        setScheme(url.scheme());
        setHost(url.host());
        setPort(url.port());
        setUsername(url.userName());
        setPassword(url.password());
        setPath(url.path());
        if (server() != oldServer)
            emit serverChanged(server());
        emit urlChanged(m_url);
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
        emit schemeChanged(m_url.scheme());
        emit serverChanged(server());
        emit urlChanged(m_url);
    }
}

void NextcloudApi::setHost(QString host) {
    if (host != m_url.host()) {
        m_url.setHost(host);
        emit hostChanged(m_url.host());
        emit serverChanged(server());
        emit urlChanged(m_url);
    }
}

void NextcloudApi::setPort(int port) {
    if (port != m_url.port() && port >= 1 && port <= 65535) {
        m_url.setPort(port);
        emit portChanged(m_url.port());
        emit serverChanged(server());
        emit urlChanged(m_url);
    }
}

void NextcloudApi::setUsername(QString username) {
    if (username != m_url.userName()) {
        m_url.setUserName(username);
        QString concatenated = username + ":" + password();
        QByteArray data = concatenated.toLocal8Bit().toBase64();
        QString headerData = "Basic " + data;
        m_authenticatedRequest.setRawHeader("Authorization", headerData.toLocal8Bit());
        emit usernameChanged(m_url.userName());
        emit urlChanged(m_url);
    }
}

void NextcloudApi::setPassword(QString password) {
    if (password != m_url.password()) {
        m_url.setPassword(password);
        QString concatenated = username() + ":" + password;
        QByteArray data = concatenated.toLocal8Bit().toBase64();
        QString headerData = "Basic " + data;
        m_authenticatedRequest.setRawHeader("Authorization", headerData.toLocal8Bit());
        emit passwordChanged(m_url.password());
        emit urlChanged(m_url);
    }
}

void NextcloudApi::setPath(QString path) {
    if (path != m_url.path()) {
        m_url.setPath(path);
        emit pathChanged(m_url.path());
        emit serverChanged(server());
        emit urlChanged(m_url);
    }
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

bool NextcloudApi::get(const QString& endpoint, bool authenticated) {
    QUrl url = server();
    url.setPath(url.path() + endpoint);
    qDebug() << "GET" << url.toDisplayString();
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        QNetworkRequest request = authenticated ? m_authenticatedRequest : m_request;
        request.setUrl(url);
        m_replies << m_manager.get(request);
        return true;
    }
    else {
        qDebug() << "GET URL not valid" << url.toDisplayString();
    }
    return false;
}

bool NextcloudApi::put(const QString& endpoint, const QByteArray& data, bool authenticated) {
    QUrl url = server();
    url.setPath(url.path() + endpoint);
    qDebug() << "PUT" << url.toDisplayString();
    qDebug() << data;
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        QNetworkRequest request = authenticated ? m_authenticatedRequest : m_request;
        request.setUrl(url);
        m_replies << m_manager.put(request, data);
        return true;
    }
    else {
        qDebug() << "PUT URL not valid" << url.toDisplayString();
    }
    return false;
}

bool NextcloudApi::post(const QString& endpoint, const QByteArray& data, bool authenticated) {
    QUrl url = server();
    url.setPath(url.path() + endpoint);
    qDebug() << "POST" << url.toDisplayString();
    qDebug() << data;
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        QNetworkRequest request = authenticated ? m_authenticatedRequest : m_request;
        request.setUrl(url);
        m_replies << m_manager.post(request, data);
        return true;
    }
    else {
        qDebug() << "POST URL not valid" << url.toDisplayString();
    }
    return false;
}

bool NextcloudApi::del(const QString& endpoint, bool authenticated) {
    QUrl url = server();
    url.setPath(url.path() + endpoint);
    qDebug() << "DEL" << url.toDisplayString();
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        QNetworkRequest request = authenticated ? m_authenticatedRequest : m_request;
        request.setUrl(url);
        m_replies << m_manager.deleteResource(request);
        return true;
    }
    else {
        qDebug() << "DEL URL not valid" << url.toDisplayString();
    }
    return false;
}

bool NextcloudApi::getStatus() {
    if (get(STATUS_ENDPOINT, false)) {
        setStatus(ApiCallStatus::ApiBusy);
        return true;
    }
    else {
        setStatus(ApiCallStatus::ApiFailed);
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
    return get(USER_CAPABILITIES_ENDPOINT.arg(username()), true);
}

bool NextcloudApi::getAppPassword() {
    return get(GET_APPPASSWORD_ENDPOINT, true);
}

bool NextcloudApi::deleteAppPassword() {
    return del(DEL_APPPASSWORD_ENDPOINT, true);
}

void NextcloudApi::verifyUrl(QUrl url) {
    emit urlValidChanged(
                url.isValid()&&
                !url.isRelative() &&
                !url.userName().isEmpty() &&
                !url.password().isEmpty() &&
                !url.host().isEmpty());
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
    qDebug() << data;

    switch (reply->error()) {
    case QNetworkReply::NoError:
        emit apiError(NoError);
        switch (reply->operation()) {
        case QNetworkAccessManager::GetOperation:
            if (reply->url().toString().endsWith(STATUS_ENDPOINT)) {
                updateStatus(json.object());
            }
            else if (reply->url().toString().endsWith(GET_APPPASSWORD_ENDPOINT)) {
                updateAppPassword(json.object());
            }
            else if (reply->url().toString().endsWith(LIST_USERS_ENDPOINT)) {
                updateUserList(json.object());
            }
            else if (reply->url().toString().contains(USER_METADATA_ENDPOINT)) {
                updateUserMeta(json.object());
            }
            else if (reply->url().toString().endsWith(CAPABILITIES_ENDPOINT)) {
                updateCapabilities(json.object());
            }
            else {
                emit getFinished(reply);
                break;
            }
            m_replies.removeOne(reply);
            reply->deleteLater();
            break;
        case QNetworkAccessManager::PutOperation:
            if (reply->url().toString().endsWith(DIRECT_DOWNLOAD_ENDPOINT)) {
                // TODO
            }
            else {
                emit putFinished(reply);
                break;
            }
            m_replies.removeOne(reply);
            reply->deleteLater();
            break;
        case QNetworkAccessManager::PostOperation:
            if (reply->url().toString().endsWith(LOGIN_FLOWV2_ENDPOINT)) {

            }
            else if (reply->url() == m_pollUrl) {

            }
            else {
                emit postFinished(reply);
                break;
            }
            m_replies.removeOne(reply);
            reply->deleteLater();
            break;
        case QNetworkAccessManager::DeleteOperation:
            if (reply->url().toString().endsWith(DEL_APPPASSWORD_ENDPOINT)) {

            }
            else {
                emit delFinished(reply);
                break;
            }
            m_replies.removeOne(reply);
            reply->deleteLater();
            break;
        default:
            qDebug() << "Unknown operation" << reply->operation() << reply->url();
            m_replies.removeOne(reply);
            reply->deleteLater();
            break;
        }
        break;
    case QNetworkReply::AuthenticationRequiredError:
        emit apiError(AuthenticationError);
        break;
    case QNetworkReply::ContentNotFoundError:
        if (reply->url() == m_pollUrl) {
            emit apiError(NoError);
        }
        else {
            emit apiError(CommunicationError);
        }
        break;
    default:
        emit apiError(CommunicationError);
        break;
    }
}

bool NextcloudApi::updateStatus(const QJsonObject &status) {

}

void NextcloudApi::setStatus(ApiCallStatus status, bool *changed) {

}

bool NextcloudApi::updateLoginFlow(const QJsonObject &login) {

}

bool NextcloudApi::updateLoginCredentials(const QJsonObject &credentials) {

}

bool NextcloudApi::updateAppPassword(const QJsonObject &password) {

}

void NextcloudApi::setLoginStatus(LoginStatus status, bool *changed) {

}

bool NextcloudApi::updateUserList(const QJsonObject &users) {

}

void NextcloudApi::setUserListStatus(ApiCallStatus status, bool *changed) {

}

bool NextcloudApi::updateUserMeta(const QJsonObject &userMeta) {

}

void NextcloudApi::setUserMetaStatus(ApiCallStatus status, bool *changed) {

}

bool NextcloudApi::updateCapabilities(const QJsonObject &capabilities) {

}

void NextcloudApi::setCababilitiesStatus(ApiCallStatus status, bool *changed) {

}
