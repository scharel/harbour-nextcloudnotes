#include "notesapi.h"
#include <QGuiApplication>
#include <QAuthenticator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

NotesApi::NotesApi(const QString statusEndpoint, const QString loginEndpoint, const QString ocsEndpoint, const QString notesEndpoint, QObject *parent)
    : NotesInterface(parent), m_statusEndpoint(statusEndpoint), m_loginEndpoint(loginEndpoint), m_ocsEndpoint(ocsEndpoint), m_notesEndpoint(notesEndpoint)
{
    // TODO verify connections (also in destructor)
    m_loginPollTimer.setInterval(POLL_INTERVALL);
    connect(&m_loginPollTimer, SIGNAL(timeout()), this, SLOT(pollLoginUrl()));
    m_statusReply = NULL;
    m_loginReply = NULL;
    m_pollReply = NULL;
    setNcStatusStatus(NextcloudStatus::NextcloudUnknown);
    setLoginStatus(LoginStatus::LoginUnknown);
    m_ncStatusStatus = NextcloudStatus::NextcloudUnknown;
    m_status_installed = false;
    m_status_maintenance = false;
    m_status_needsDbUpgrade = false;
    m_status_extendedSupport = false;
    m_loginStatus = LoginStatus::LoginUnknown;
    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(verifyUrl(QUrl)));
    connect(&m_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(requireAuthentication(QNetworkReply*,QAuthenticator*)));
    connect(&m_manager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    connect(&m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(&m_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslError(QNetworkReply*,QList<QSslError>)));
    m_request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    m_request.setHeader(QNetworkRequest::UserAgentHeader, QGuiApplication::applicationDisplayName() +  " " + QGuiApplication::applicationVersion() + " - " + QSysInfo::machineHostName());
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/x-www-form-urlencoded").toUtf8());
    m_request.setRawHeader("OCS-APIREQUEST", "true");
    m_authenticatedRequest = m_request;
    m_authenticatedRequest.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json").toUtf8());
}

NotesApi::~NotesApi() {
    disconnect(&m_loginPollTimer, SIGNAL(timeout()), this, SLOT(pollLoginUrl()));
    disconnect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(verifyUrl(QUrl)));
    disconnect(&m_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(requireAuthentication(QNetworkReply*,QAuthenticator*)));
    disconnect(&m_manager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    disconnect(&m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    disconnect(&m_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslError(QNetworkReply*,QList<QSslError>)));
}

void NotesApi::setAccount(const QString &account) {
    if (account != m_account) {
        m_account = account;
        // TODO reset the class
        emit accountChanged(m_account);
    }
}

void NotesApi::getAllNotes(Note::NoteField exclude) {
    QUrl url = apiEndpointUrl(m_notesEndpoint);
    QStringList excludeFields;
    QList<Note::NoteField> noteFields = Note::noteFields();
    QFlags<Note::NoteField> flags(exclude);

    for (int i = 0; i < noteFields.size(); ++i) {
        if (flags.testFlag(noteFields[i])) {
            excludeFields << Note::noteFieldName(noteFields[i]);
        }
    }
    if (!excludeFields.isEmpty())
        url.setQuery(QString(EXCLUDE_QUERY).append(excludeFields.join(",")));

    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "GET" << url.toDisplayString();
        m_authenticatedRequest.setUrl(url);
        m_notesReplies << m_manager.get(m_authenticatedRequest);
        emit busyChanged(true);
    }
}

void NotesApi::getNote(const int id, Note::NoteField exclude) {
    QUrl url = apiEndpointUrl(m_notesEndpoint + QString("/%1").arg(id));
    QStringList excludeFields;
    QList<Note::NoteField> noteFields = Note::noteFields();
    QFlags<Note::NoteField> flags(exclude);

    for (int i = 0; i < noteFields.size(); ++i) {
        if (flags.testFlag(noteFields[i])) {
            excludeFields << Note::noteFieldName(noteFields[i]);
        }
    }
    if (!excludeFields.isEmpty())
        url.setQuery(QString(EXCLUDE_QUERY).append(excludeFields.join(",")));

    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "GET" << url.toDisplayString();
        m_authenticatedRequest.setUrl(url);
        m_notesReplies << m_manager.get(m_authenticatedRequest);
        emit busyChanged(true);
    }
}

void NotesApi::createNote(const Note &note) {
    QUrl url = apiEndpointUrl(m_notesEndpoint);
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "POST" << url.toDisplayString();
        m_authenticatedRequest.setUrl(url);
        m_notesReplies << m_manager.post(m_authenticatedRequest, noteApiData(note));
        emit busyChanged(true);
    }
}

void NotesApi::updateNote(const Note &note) {
    QUrl url = apiEndpointUrl(m_notesEndpoint + QString("/%1").arg(note.id()));
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "PUT" << url.toDisplayString();
        m_authenticatedRequest.setUrl(url);
        m_notesReplies << m_manager.put(m_authenticatedRequest, noteApiData(note));
        emit busyChanged(true);
    }
}

void NotesApi::deleteNote(const int id) {
    QUrl url = apiEndpointUrl(m_notesEndpoint + QString("/%1").arg(id));
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "DELETE" << url.toDisplayString();
        m_authenticatedRequest.setUrl(url);
        m_notesReplies << m_manager.deleteResource(m_authenticatedRequest);
        emit busyChanged(true);
    }
}

const QByteArray NotesApi::noteApiData(const Note &note) {
    QJsonObject json = note.toJsonObject();
    json.remove(Note::noteFieldName(Note::Id));
    if (note.modified() == 0)
        json.remove(Note::noteFieldName(Note::Modified));
    if (note.title().isNull())
        json.remove(Note::noteFieldName(Note::Title));
    if (note.category().isNull())
        json.remove(Note::noteFieldName(Note::Category));
    if (note.content().isNull())
        json.remove(Note::noteFieldName(Note::Content));
    json.remove(Note::noteFieldName(Note::Etag));
    json.remove(Note::noteFieldName(Note::Error));
    json.remove(Note::noteFieldName(Note::ErrorMessage));

    return QJsonDocument(json).toJson(QJsonDocument::Compact);
}

// TODO ab hier überarbeiten

void NotesApi::setVerifySsl(bool verify) {
    if (verify != (m_request.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer)) {
        m_request.sslConfiguration().setPeerVerifyMode(verify ? QSslSocket::VerifyPeer : QSslSocket::VerifyNone);
        emit verifySslChanged(verify);
    }
    if (verify != (m_authenticatedRequest.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer)) {
        m_authenticatedRequest.sslConfiguration().setPeerVerifyMode(verify ? QSslSocket::VerifyPeer : QSslSocket::VerifyNone);
        emit verifySslChanged(verify);
    }
}

void NotesApi::setUrl(QUrl url) {
    if (url != m_url) {
        QString oldServer = server();
        setScheme(url.scheme());
        setHost(url.host());
        setPort(url.port());
        setUsername(url.userName());
        setPassword(url.password());
        setPath(url.path());
        emit urlChanged(m_url);
        if (server() != oldServer)
            emit serverChanged(server());
    }
}

QString NotesApi::server() const {
    QUrl server;
    server.setScheme(m_url.scheme());
    server.setHost(m_url.host());
    if (m_url.port() > 0)
        server.setPort(m_url.port());
    server.setPath(m_url.path());
    return server.toString();
}

void NotesApi::setServer(QString serverUrl) {
    QUrl url(serverUrl.trimmed());
    if (url != server()) {
        setScheme(url.scheme());
        setHost(url.host());
        setPort(url.port());
        setPath(url.path());
    }
}

void NotesApi::setScheme(QString scheme) {
    if (scheme != m_url.scheme() && (scheme == "http" || scheme == "https")) {
        m_url.setScheme(scheme);
        emit schemeChanged(m_url.scheme());
        emit serverChanged(server());
        emit urlChanged(m_url);
    }
}

void NotesApi::setHost(QString host) {
    if (host != m_url.host()) {
        m_url.setHost(host);
        emit hostChanged(m_url.host());
        emit serverChanged(server());
        emit urlChanged(m_url);
    }
}

void NotesApi::setPort(int port) {
    if (port != m_url.port() && port >= 1 && port <= 65535) {
        m_url.setPort(port);
        emit portChanged(m_url.port());
        emit serverChanged(server());
        emit urlChanged(m_url);
    }
}

void NotesApi::setUsername(QString username) {
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

void NotesApi::setPassword(QString password) {
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

void NotesApi::setPath(QString path) {
    if (path != m_url.path()) {
        m_url.setPath(path);
        emit pathChanged(m_url.path());
        emit serverChanged(server());
        emit urlChanged(m_url);
    }
}

bool NotesApi::getNcStatus() {
    QUrl url = apiEndpointUrl(m_statusEndpoint);
    qDebug() << "POST" << url.toDisplayString();
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        setNcStatusStatus(NextcloudStatus::NextcloudBusy);
        m_request.setUrl(url);
        m_statusReply = m_manager.post(m_request, QByteArray());
        return true;
    }
    else {
        qDebug() << "URL not valid!";
        setNcStatusStatus(NextcloudStatus::NextcloudUnknown);
    }
    return false;
}

bool NotesApi::initiateFlowV2Login() {
    if (m_loginStatus == LoginStatus::LoginFlowV2Initiating || m_loginStatus == LoginStatus::LoginFlowV2Polling) {
        abortFlowV2Login();
    }
    QUrl url = apiEndpointUrl(m_loginEndpoint);
    qDebug() << "POST" << url.toDisplayString();
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        setLoginStatus(LoginStatus::LoginFlowV2Initiating);
        m_request.setUrl(url);
        m_loginReply = m_manager.post(m_request, QByteArray());
        return true;
    }
    else {
        qDebug() << "URL not valid!";
        setLoginStatus(LoginStatus::LoginFlowV2Failed);
        abortFlowV2Login();
    }
    return false;
}

void NotesApi::abortFlowV2Login() {
    m_loginPollTimer.stop();
    m_loginUrl.clear();
    emit loginUrlChanged(m_loginUrl);
    m_pollUrl.clear();
    m_pollToken.clear();
    setLoginStatus(LoginStatus::LoginUnknown);
}

void NotesApi::pollLoginUrl() {
    qDebug() << "POST" << m_pollUrl.toDisplayString();
    if (m_pollUrl.isValid() && !m_pollUrl.scheme().isEmpty() && !m_pollUrl.host().isEmpty() && !m_pollToken.isEmpty()) {
        m_request.setUrl(m_pollUrl);
        m_pollReply = m_manager.post(m_request, QByteArray("token=").append(m_pollToken));
    }
    else {
        qDebug() << "URL not valid!";
        setLoginStatus(LoginStatus::LoginFlowV2Failed);
        abortFlowV2Login();
    }
}

void NotesApi::verifyLogin(QString username, QString password) {
    m_ocsRequest = m_authenticatedRequest;
    if (username.isEmpty())
        username = this->username();
    if (password.isEmpty())
        password = this->password();
    QUrl url = apiEndpointUrl(m_ocsEndpoint + QString("/users/%1").arg(username));
    m_ocsRequest.setRawHeader("Authorization", "Basic " + QString(username + ":" + password).toLocal8Bit().toBase64());
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "GET" << url.toDisplayString();
        m_ocsRequest.setUrl(url);
        m_ocsReply = m_manager.get(m_ocsRequest);
        emit busyChanged(true);
    }
}

const QString NotesApi::errorMessage(ErrorCodes error) const {
    QString message;
    switch (error) {
    case NoError:
        break;
    case NoConnectionError:
        message = tr("No network connection available");
        break;
    case CommunicationError:
        message = tr("Failed to communicate with the Nextcloud server");
        break;
    case LocalFileReadError:
        message = tr("An error happened while reading from the local storage");
        break;
    case LocalFileWriteError:
        message = tr("An error happened while writing to the local storage");
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

void NotesApi::verifyUrl(QUrl url) {
    emit urlValidChanged(url.isValid());
}

void NotesApi::requireAuthentication(QNetworkReply *reply, QAuthenticator *authenticator) {
    if (reply && authenticator) {
        authenticator->setUser(username());
        authenticator->setPassword(password());
    }
    else
        emit error(AuthenticationError);
}

void NotesApi::onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible) {
    emit networkAccessibleChanged(accessible == QNetworkAccessManager::Accessible);
}

void NotesApi::replyFinished(QNetworkReply *reply) {
    //qDebug() << reply->error() << reply->errorString();
    if (reply->error() == QNetworkReply::NoError) {
        emit error(NoError);

        QByteArray data = reply->readAll();

        if (reply == m_ocsReply) {
            qDebug() << "OCS reply";
            QString xml(data);
            if (xml.contains("<status>ok</status>")) {
                qDebug() << "Login Success!";
                setLoginStatus(LoginSuccess);
            }
            else {
                qDebug() << "Login Failed!";
                setLoginStatus(LoginFailed);
            }
        }
        else {
            QJsonDocument json = QJsonDocument::fromJson(data);
            if (reply == m_loginReply) {
                qDebug() << "Login reply";
                if (json.isObject())
                    updateLoginFlow(json.object());
                m_loginReply = NULL;
            }
            else if (reply == m_pollReply) {
                qDebug() << "Poll reply, finished";
                if (json.isObject())
                    updateLoginCredentials(json.object());
                m_pollReply = NULL;
                abortFlowV2Login();
            }
            else if (reply == m_statusReply) {
                qDebug() << "Status reply";
                if (json.isObject())
                    updateNcStatus(json.object());
                m_statusReply = NULL;
            }
            else if (m_notesReplies.contains(reply)) {
                qDebug() << "Notes reply";
                if (json.isArray())
                    updateApiNotes(json.array());
                else if (json.isObject())
                    updateApiNote(json.object());
                m_notesReplies.removeOne(reply);
                emit busyChanged(busy());
            }
            else {
                qDebug() << "Unknown or double reply";
            }
            //qDebug() << data;
        }
    }
    else if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
        emit error(AuthenticationError);
    }
    else if (reply->error() == QNetworkReply::ContentNotFoundError && reply == m_pollReply) {
        qDebug() << "Polling not finished yet" << reply->url().toDisplayString();
    }
    else {
        if (reply == m_loginReply) {
            m_loginReply = NULL;
            m_loginStatus = LoginStatus::LoginFailed;
            emit loginStatusChanged(m_loginStatus);
        }
        else if (reply == m_pollReply) {
            m_pollReply = NULL;
            m_loginStatus = LoginStatus::LoginFlowV2Polling;
            emit loginStatusChanged(m_loginStatus);
        }
        else if (reply == m_statusReply) {
            m_statusReply = NULL;
            updateNcStatus(QJsonObject());
            //m_statusStatus = RequestStatus::StatusError;
            //emit statusStatusChanged(m_statusStatus);
        }
        else if (m_notesReplies.contains(reply)) {
            m_notesReplies.removeOne(reply);
            emit busyChanged(busy());
        }
        if (reply != m_statusReply)
            emit error(CommunicationError);
    }
    reply->deleteLater();
}

void NotesApi::sslError(QNetworkReply *reply, const QList<QSslError> &errors) {
    qDebug() << "SSL errors accured while calling" << reply->url().toDisplayString();
    for (int i = 0; i < errors.size(); ++i) {
        qDebug() << errors[i].errorString();
    }
    emit error(SslHandshakeError);
}

QUrl NotesApi::apiEndpointUrl(const QString endpoint) const {
    QUrl url = server();
    url.setPath(url.path() + endpoint);
    return url;
}

void NotesApi::updateNcStatus(const QJsonObject &status) {
    if (m_status_installed != status.value("installed").toBool()) {
        m_status_installed = status.value("installed").toBool();
        emit statusInstalledChanged(m_status_installed);
    }
    if (m_status_maintenance != status.value("maintenance").toBool()) {
        m_status_maintenance = status.value("maintenance").toBool();
        emit statusMaintenanceChanged(m_status_maintenance);
    }
    if (m_status_needsDbUpgrade != status.value("needsDbUpgrade").toBool()) {
        m_status_needsDbUpgrade = status.value("needsDbUpgrade").toBool();
        emit statusNeedsDbUpgradeChanged(m_status_needsDbUpgrade);
    }
    if (m_status_version != status.value("version").toString()) {
        m_status_version = status.value("version").toString();
        emit statusVersionChanged(m_status_version);
    }
    if (m_status_versionstring != status.value("versionstring").toString()) {
        m_status_versionstring = status.value("versionstring").toString();
        emit statusVersionStringChanged(m_status_versionstring);
    }
    if (m_status_edition != status.value("edition").toString()) {
        m_status_edition = status.value("edition").toString();
        emit statusEditionChanged(m_status_edition);
    }
    if (m_status_productname != status.value("productname").toString()) {
        m_status_productname = status.value("productname").toString();
        emit statusProductNameChanged(m_status_productname);
    }
    if (m_status_extendedSupport != status.value("extendedSupport").toBool()) {
        m_status_extendedSupport = status.value("extendedSupport").toBool();
        emit statusExtendedSupportChanged(m_status_extendedSupport);
    }
    if (status.isEmpty())
        setNcStatusStatus(NextcloudStatus::NextcloudFailed);
    else
        setNcStatusStatus(NextcloudStatus::NextcloudSuccess);
}

void NotesApi::setNcStatusStatus(NextcloudStatus status, bool *changed) {
    if (status != m_ncStatusStatus) {
        if (changed)
            *changed = true;
        m_ncStatusStatus = status;
        emit ncStatusStatusChanged(m_ncStatusStatus);
    }
}

bool NotesApi::updateLoginFlow(const QJsonObject &login) {
    QUrl url;
    QString token;
    if (!login.isEmpty()) {
        QJsonObject poll = login.value("poll").toObject();
        url = login.value("login").toString();
        if (!poll.isEmpty() && url.isValid()) {
            if (url != m_loginUrl) {
                m_loginUrl = url;
                emit loginUrlChanged(m_loginUrl);
            }
            url = poll.value("endpoint").toString();
            token = poll.value("token").toString();
            if (url.isValid() && !token.isEmpty()) {
                m_pollUrl = url;
                qDebug() << "Poll URL: " << m_pollUrl;
                m_pollToken = token;
                qDebug() << "Poll Token: " << m_pollToken;
                setLoginStatus(LoginStatus::LoginFlowV2Polling);
                m_loginPollTimer.start();
                return true;
            }
        }
    }
    else {
        qDebug() << "Invalid Poll Data:" << login;
        setLoginStatus(LoginStatus::LoginFlowV2Failed);
        abortFlowV2Login();
    }
    return false;
}

bool NotesApi::updateLoginCredentials(const QJsonObject &credentials) {
    QString serverAddr;
    QString loginName;
    QString appPassword;
    if (!credentials.isEmpty()) {
        serverAddr = credentials.value("server").toString();
        if (!serverAddr.isEmpty() && serverAddr != server())
            setServer(serverAddr);
        loginName = credentials.value("loginName").toString();
        if (!loginName.isEmpty() && loginName != username())
            setUsername(loginName);
        appPassword = credentials.value("appPassword").toString();
        if (!appPassword.isEmpty() && appPassword != password())
            setPassword(appPassword);
    }
    if (!serverAddr.isEmpty() && !loginName.isEmpty() && !appPassword.isEmpty()) {
        qDebug() << "Login successfull for user" << loginName << "on" << serverAddr;
        setLoginStatus(LoginStatus::LoginFlowV2Success);
        return true;
    }
    qDebug() << "Login failed for user" << loginName << "on" << serverAddr;
    return false;
}

void NotesApi::setLoginStatus(LoginStatus status, bool *changed) {
    if (status != m_loginStatus) {
        if (changed)
            *changed = true;
        m_loginStatus = status;
        emit loginStatusChanged(m_loginStatus);
    }
}

void NotesApi::updateApiNotes(const QJsonArray &json) {
    for (int i = 0; i < json.size(); ++i) {
        if (json[i].isObject())
            updateApiNote(json[i].toObject());
    }
}

void NotesApi::updateApiNote(const QJsonObject &json) {
    Note note(json);
    if (!note.error())
        emit noteUpdated(note);
}
