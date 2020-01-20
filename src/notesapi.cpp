#include "notesapi.h"
#include <QGuiApplication>
#include <QAuthenticator>
#include <QJsonDocument>
#include <QJsonObject>

NotesApi::NotesApi(const QString statusEndpoint, const QString loginEndpoint, const QString notesEndpoint, QObject *parent)
    : QObject(parent), m_statusEndpoint(statusEndpoint), m_loginEndpoint(loginEndpoint), m_notesEndpoint(notesEndpoint)
{
    // TODO verify connections (also in destructor)
    m_loginPollTimer.setInterval(5000);
    connect(&m_loginPollTimer, SIGNAL(timeout()), this, SLOT(pollLoginUrl()));
    mp_model = new NotesModel(this);
    mp_modelProxy = new NotesProxyModel(this);
    mp_modelProxy->setSourceModel(mp_model);
    mp_modelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    mp_modelProxy->setSortLocaleAware(true);
    mp_modelProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mp_modelProxy->setFilterRole(NotesModel::ContentRole);
    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(verifyUrl(QUrl)));
    connect(this, SIGNAL(statusBusyChanged(bool)), this, SIGNAL(busyChanged(bool)));
    connect(this, SIGNAL(loginBusyChanged(bool)), this, SIGNAL(busyChanged(bool)));
    connect(this, SIGNAL(notesBusyChanged(bool)), this, SIGNAL(busyChanged(bool)));
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
    connect(mp_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(saveToFile(QModelIndex,QModelIndex,QVector<int>)));
}

NotesApi::~NotesApi() {
    disconnect(mp_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(saveToFile(QModelIndex,QModelIndex,QVector<int>)));
    disconnect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(verifyUrl(QUrl)));
    disconnect(&m_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(requireAuthentication(QNetworkReply*,QAuthenticator*)));
    disconnect(&m_manager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    disconnect(&m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    disconnect(&m_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslError(QNetworkReply*,QList<QSslError>)));
    m_jsonFile.close();
    if (mp_modelProxy)
        delete mp_modelProxy;
    if (mp_model)
        delete mp_model;
}

void NotesApi::setSslVerify(bool verify) {
    if (verify != (m_authenticatedRequest.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer)) {
        m_authenticatedRequest.sslConfiguration().setPeerVerifyMode(verify ? QSslSocket::VerifyPeer : QSslSocket::VerifyNone);
        emit sslVerifyChanged(verify);
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
    QUrl url(serverUrl);
    if (serverUrl != server()) {
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
        emit urlChanged(m_url);
    }
}

void NotesApi::setHost(QString host) {
    if (host != m_url.host()) {
        m_url.setHost(host);
        emit hostChanged(m_url.host());
        emit urlChanged(m_url);
    }
}

void NotesApi::setPort(int port) {
    if (port != m_url.port() && port >= 1 && port <= 65535) {
        m_url.setPort(port);
        emit portChanged(m_url.port());
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
        emit urlChanged(m_url);
    }
}

void NotesApi::setDataFile(QString dataFile) {
    if (dataFile != m_jsonFile.fileName()) {
        m_jsonFile.close();
        m_jsonFile.setFileName(dataFile);
        emit dataFileChanged(m_jsonFile.fileName());
    }
}

void NotesApi::getStatus() {
    QUrl url = apiEndpointUrl(m_statusEndpoint);
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "POST" << url.toDisplayString();
        m_request.setUrl(url);
        m_statusReplies << m_manager.post(m_request, QByteArray());
        emit statusBusyChanged(true);
    }
}

void NotesApi::initiateFlowV2Login() {
    QUrl url = apiEndpointUrl(m_loginEndpoint);
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "POST" << url.toDisplayString();
        m_request.setUrl(url);
        m_loginReplies << m_manager.post(m_request, QByteArray());
        m_loginPollTimer.start();
        emit loginBusyChanged(true);
    }
}

void NotesApi::abortFlowV2Login() {
    // TODO crashes!
    m_loginPollTimer.stop();
    m_loginUrl.clear();
    emit loginUrlChanged(m_loginUrl);
    m_pollUrl.clear();
    m_pollToken.clear();
    for (int i = 0; i < m_loginReplies.size(); ++i) {
        m_loginReplies[i]->abort();
    }
    for (int i = 0; i < m_pollReplies.size(); ++i) {
        m_pollReplies[i]->abort();
    }
}

void NotesApi::pollLoginUrl() {
    if (m_pollUrl.isValid() && !m_pollUrl.scheme().isEmpty() && !m_pollUrl.host().isEmpty() && !m_pollToken.isEmpty()) {
        qDebug() << "POST" << m_pollUrl.toDisplayString();
        m_request.setUrl(m_pollUrl);
        m_pollReplies << m_manager.post(m_request, QByteArray("token=").append(m_pollToken));
        emit loginBusyChanged(true);
    }
}

void NotesApi::getAllNotes(QStringList excludeFields) {
    QUrl url = apiEndpointUrl(m_notesEndpoint + "/notes");
    if (!excludeFields.isEmpty())
        url.setQuery(QString("exclude=").append(excludeFields.join(",")));
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "GET" << url.toDisplayString();
        m_authenticatedRequest.setUrl(url);
        m_notesReplies << m_manager.get(m_authenticatedRequest);
        emit notesBusyChanged(true);
    }
}

void NotesApi::getNote(double noteId, QStringList excludeFields) {
    QUrl url = apiEndpointUrl(m_notesEndpoint + QString("/notes/%1").arg(noteId));
    if (!excludeFields.isEmpty())
        url.setQuery(QString("exclude=").append(excludeFields.join(",")));
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "GET" << url.toDisplayString();
        m_authenticatedRequest.setUrl(url);
        m_notesReplies << m_manager.get(m_authenticatedRequest);
        emit notesBusyChanged(true);
    }
}

void NotesApi::createNote(QVariantMap fields) {
    // Update note in the model
    Note note(QJsonObject::fromVariantMap(fields));
    mp_model->insertNote(note);

    // Create note via the API
    QUrl url = apiEndpointUrl(m_notesEndpoint + "/notes");
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "POST" << url.toDisplayString();
        m_authenticatedRequest.setUrl(url);
        m_notesReplies << m_manager.post(m_authenticatedRequest, note.toJsonDocument().toJson());
        emit notesBusyChanged(true);
    }
}

void NotesApi::updateNote(double noteId, QVariantMap fields) {
    // Update note in the model
    Note note(QJsonObject::fromVariantMap(fields));
    mp_model->insertNote(note);

    // Update note on the server
    QUrl url = apiEndpointUrl(m_notesEndpoint + QString("/notes/%1").arg(noteId));
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "PUT" << url.toDisplayString();
        m_authenticatedRequest.setUrl(url);
        m_notesReplies << m_manager.put(m_authenticatedRequest, note.toJsonDocument().toJson());
        emit notesBusyChanged(true);
    }
}

void NotesApi::deleteNote(double noteId) {
    // Remove note from the model
    mp_model->removeNote(noteId);

    // Remove note from the server
    QUrl url = apiEndpointUrl(m_notesEndpoint + QString("/notes/%1").arg(noteId));
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "DELETE" << url.toDisplayString();
        m_authenticatedRequest.setUrl(url);
        m_notesReplies << m_manager.deleteResource(m_authenticatedRequest);
        emit notesBusyChanged(true);
    }
    mp_model->removeNote(noteId);
}

void NotesApi::verifyUrl(QUrl url) {
    emit urlValidChanged(url.isValid());
    if (m_url.isValid() && !m_url.scheme().isEmpty() && !m_url.host().isEmpty()) {
        getStatus();
    }
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
        QJsonDocument json = QJsonDocument::fromJson(data);
        /*if (reply->url().toString().contains(m_loginEndpoint)) {
            qDebug() << "Login reply";
        }
        else if (reply->url() == m_pollUrl) {
            qDebug() << "Poll reply";
        }
        else if (reply->url().toString().contains(m_statusEndpoint)) {
            qDebug() << "Status reply";
        }
        else if (reply->url().toString().contains(m_notesEndpoint)) {
            qDebug() << "Notes reply";
        }*/

        if (m_loginReplies.contains(reply)) {
            qDebug() << "Login reply";
            if (json.isObject())
                updateLoginFlow(json.object());
            //m_loginReplies.removeAll(reply);
            emit loginBusyChanged(loginBusy());
        }
        else if (m_pollReplies.contains(reply)) {
            qDebug() << "Poll reply, finished";
            if (json.isObject())
                updateLoginCredentials(json.object());
            //m_pollReplies.removeAll(reply);
            abortFlowV2Login();
            emit loginBusyChanged(loginBusy());
        }
        else if (m_statusReplies.contains(reply)) {
            qDebug() << "Status reply";
            if (json.isObject())
                updateStatus(json.object());
            //m_statusReplies.removeAll(reply);
            emit statusBusyChanged(statusBusy());
        }
        else if (m_notesReplies.contains(reply)) {
            qDebug() << "Notes reply";
            if (mp_model) {
                if (mp_model->fromJsonDocument(json)) {
                    m_lastSync = QDateTime::currentDateTimeUtc();
                    emit lastSyncChanged(m_lastSync);
                }
            }
            //m_notesReplies.removeAll(reply);
            emit notesBusyChanged(notesBusy());
        }
        else {
            qDebug() << "Unknown reply";
        }
        //qDebug() << data;
    }
    else if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
        emit error(AuthenticationError);
    }
    else {
        if (m_pollReplies.contains(reply)) {
            qDebug() << "Poll reply";
            //qDebug() << "Polling not finished yet" << m_pollUrl;
        }
        else if (m_statusReplies.contains(reply)) {
            updateStatus(QJsonObject());
            qDebug() << "Could not retreive status";
        }
        else {
            emit error(CommunicationError);
        }
    }
    m_loginReplies.removeAll(reply);
    m_pollReplies.removeAll(reply);
    m_statusReplies.removeAll(reply);
    m_notesReplies.removeAll(reply);
    reply->deleteLater();
}

void NotesApi::sslError(QNetworkReply *reply, const QList<QSslError> &errors) {
    qDebug() << "SSL errors accured while calling" << reply->url().toDisplayString();
    for (int i = 0; i < errors.size(); ++i) {
        qDebug() << errors[i].errorString();
    }
    emit error(SslHandshakeError);
}

void NotesApi::saveToFile(QModelIndex, QModelIndex, QVector<int>) {
    if (m_jsonFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
        qDebug() << "Writing data to file" << m_jsonFile.fileName();
        QByteArray data = mp_model->toJsonDocument().toJson();
        if (m_jsonFile.write(data) < data.size())
            emit error(LocalFileWriteError);
        m_jsonFile.close();
    }
    else
        emit error(LocalFileWriteError);
}

QUrl NotesApi::apiEndpointUrl(const QString endpoint) const {
    QUrl url = server();
    url.setPath(url.path() + endpoint);
    return url;
}

void NotesApi::updateStatus(const QJsonObject &status) {
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
}

void NotesApi::updateLoginFlow(const QJsonObject &login) {
    QUrl url;
    QString token;
    if (!login.isEmpty()) {
        QJsonObject poll = login.value("poll").toObject();
        if (!poll.isEmpty()) {
            url = poll.value("endpoint").toString() ;
            token = poll.value("token").toString();
            if (url.isValid() && !token.isEmpty()) {
                m_pollUrl = url;
                qDebug() << "Poll URL: " << m_pollUrl;
                m_pollToken = token;
                qDebug() << "Poll Token: " << m_pollToken;
            }
            else {
                qDebug() << "Invalid Poll URL:" << url;
            }
        }
        url = login.value("login").toString();
        if (m_loginUrl != url && url.isValid()) {
            m_loginUrl = url;
            emit loginUrlChanged(m_loginUrl);
        }
    }
}

void NotesApi::updateLoginCredentials(const QJsonObject &credentials) {
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
    qDebug() << "Login successfull for user" << loginName << "on" << serverAddr;
}

const QString NotesApi::errorMessage(int error) const {
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
