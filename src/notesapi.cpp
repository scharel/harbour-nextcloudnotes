#include "notesapi.h"
#include <QGuiApplication>
#include <QAuthenticator>
#include <QJsonDocument>
#include <QJsonObject>

NotesApi::NotesApi(QObject *parent) : QObject(parent)
{
    m_loginPollTimer.setInterval(1000);
    connect(&m_loginPollTimer, SIGNAL(timeout()), this, SLOT(pollLoginUrl()));
    m_online = m_manager.networkAccessible() == QNetworkAccessManager::Accessible;
    mp_model = new NotesModel(this);
    mp_modelProxy = new NotesProxyModel(this);
    mp_modelProxy->setSourceModel(mp_model);
    mp_modelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    mp_modelProxy->setSortLocaleAware(true);
    mp_modelProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mp_modelProxy->setFilterRole(NotesModel::ContentRole);
    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(verifyUrl(QUrl)));
    connect(&m_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(requireAuthentication(QNetworkReply*,QAuthenticator*)));
    connect(&m_manager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    connect(&m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(&m_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslError(QNetworkReply*,QList<QSslError>)));
    m_request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    m_request.setHeader(QNetworkRequest::UserAgentHeader, QGuiApplication::applicationDisplayName() +  " " + QGuiApplication::applicationVersion() + " - " + QSysInfo::machineHostName());
    m_request.setRawHeader("OCS-APIREQUEST", "true");
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json").toUtf8());
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
    if (verify != (m_request.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer)) {
        m_request.sslConfiguration().setPeerVerifyMode(verify ? QSslSocket::VerifyPeer : QSslSocket::VerifyNone);
        emit sslVerifyChanged(verify);
    }
}

void NotesApi::setUrl(QUrl url) {
    if (url != m_url) {
        QUrl oldUrl = m_url;
        QString oldServer = server();
        m_url = url;
        emit urlChanged(m_url);
        if (server() != oldServer)
            emit serverChanged(server());
        if (m_url.scheme() != oldUrl.scheme())
            emit schemeChanged(m_url.scheme());
        if (m_url.host() != oldUrl.host())
            emit hostChanged(m_url.host());
        if (m_url.port() != oldUrl.port())
            emit portChanged(m_url.port());
        if (m_url.userName() != oldUrl.userName())
            emit usernameChanged(m_url.userName());
        if (m_url.password() != oldUrl.password())
            emit passwordChanged(m_url.password());
        if (m_url.path() != oldUrl.path())
            emit pathChanged(m_url.path());
        if (m_url.isValid())
            qDebug() << "API URL:" << m_url.toDisplayString();
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
    if (scheme == "http" || scheme == "https") {
        QUrl url = m_url;
        url.setScheme(scheme);
        setUrl(url);
    }
}

void NotesApi::setHost(QString host) {
    if (!host.isEmpty()) {
        QUrl url = m_url;
        url.setHost(host);
        setUrl(url);
    }
}

void NotesApi::setPort(int port) {
    if (port >= 1 && port <= 65535) {
        QUrl url = m_url;
        url.setPort(port);
        setUrl(url);
    }
}

void NotesApi::setUsername(QString user) {
    if (!user.isEmpty()) {
        QUrl url = m_url;
        url.setUserName(user);
        QString concatenated = user + ":" + password();
        QByteArray data = concatenated.toLocal8Bit().toBase64();
        QString headerData = "Basic " + data;
        m_request.setRawHeader("Authorization", headerData.toLocal8Bit());
        setUrl(url);
    }
}

void NotesApi::setPassword(QString password) {
    if (!password.isEmpty()) {
        QUrl url = m_url;
        url.setPassword(password);
        QString concatenated = username() + ":" + password;
        QByteArray data = concatenated.toLocal8Bit().toBase64();
        QString headerData = "Basic " + data;
        m_request.setRawHeader("Authorization", headerData.toLocal8Bit());
        setUrl(url);
    }
}

void NotesApi::setPath(QString path) {
    if (!path.isEmpty()) {
        QUrl url = m_url;
        url.setPath(path);
        setUrl(url);
    }
}

void NotesApi::setDataFile(QString dataFile) {
    if (dataFile != m_jsonFile.fileName()) {
        m_jsonFile.close();
        if (!dataFile.isEmpty())
            m_jsonFile.setFileName(dataFile);
        emit dataFileChanged(m_jsonFile.fileName());
        //qDebug() << m_jsonFile.fileName();
    }
}

bool NotesApi::busy() const {
    bool busy = false;
    QVector<QNetworkReply*> replies;
    replies << m_replies << m_status_replies << m_login_replies << m_poll_replies;
    for (int i = 0; i < replies.size(); ++i) {
        busy |= replies[i]->isRunning();
    }
    return busy;
}

void NotesApi::getStatus() {
    QUrl url = server();
    QNetworkRequest request;
    url.setPath(url.path() + "/status.php");
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "POST" << url.toDisplayString();
        request.setUrl(url);
        m_status_replies << m_manager.post(request, QByteArray());
        emit busyChanged(busy());
    }
}

void NotesApi::initiateFlowV2Login() {
    QUrl url = server();
    url.setPath(url.path() + "/index.php/login/v2");
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "POST" << url.toDisplayString();
        m_request.setUrl(url);
        m_login_replies << m_manager.post(m_request, QByteArray());
        m_loginPollTimer.start();
        emit busyChanged(busy());
    }
}

void NotesApi::pollLoginUrl() {
    //QNetworkRequest request;
    //request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/x-www-form-urlencoded").toUtf8());
    if (m_pollUrl.isValid() && !m_pollUrl.scheme().isEmpty() && !m_pollUrl.host().isEmpty() && !m_pollToken.isEmpty()) {
        //qDebug() << "POST" << m_pollUrl.toDisplayString();
        m_request.setUrl(m_pollUrl);
        m_poll_replies << m_manager.post(m_request, QByteArray("token=").append(m_pollToken));
        emit busyChanged(busy());
    }
}

void NotesApi::getAllNotes(QStringList excludeFields) {
    QUrl url = server();
    url.setPath(url.path() + "/index.php/apps/notes/api/v0.2" + "/notes");
    if (!excludeFields.isEmpty())
        url.setQuery(QString("exclude=").append(excludeFields.join(",")));
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "GET" << url.toDisplayString();
        m_request.setUrl(url);
        m_replies << m_manager.get(m_request);
        emit busyChanged(busy());
    }
}

void NotesApi::getNote(double noteId, QStringList excludeFields) {
    QUrl url = m_url;
    url.setPath(url.path() + "/index.php/apps/notes/api/v0.2" + QString("/notes/%1").arg(noteId));
    if (!excludeFields.isEmpty())
        url.setQuery(QString("exclude=").append(excludeFields.join(",")));
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "GET" << url.toDisplayString();
        m_request.setUrl(url);
        m_replies << m_manager.get(m_request);
        emit busyChanged(busy());
    }
}

void NotesApi::createNote(QVariantMap fields) {
    // Update note in the model
    Note note(QJsonObject::fromVariantMap(fields));
    mp_model->insertNote(note);

    // Create note via the API
    QUrl url = m_url;
    url.setPath(url.path() + "/index.php/apps/notes/api/v0.2" + "/notes");
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "POST" << url.toDisplayString();
        m_request.setUrl(url);
        m_replies << m_manager.post(m_request, note.toJsonDocument().toJson());
        emit busyChanged(busy());
    }
}

void NotesApi::updateNote(double noteId, QVariantMap fields) {
    // Update note in the model
    Note note(QJsonObject::fromVariantMap(fields));
    mp_model->insertNote(note);

    // Update note on the server
    QUrl url = m_url;
    url.setPath(url.path() + "/index.php/apps/notes/api/v0.2" + QString("/notes/%1").arg(noteId));
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "PUT" << url.toDisplayString();
        m_request.setUrl(url);
        m_replies << m_manager.put(m_request, note.toJsonDocument().toJson());
        emit busyChanged(busy());
    }
}

void NotesApi::deleteNote(double noteId) {
    // Remove note from the model
    mp_model->removeNote(noteId);

    // Remove note from the server
    QUrl url = m_url;
    url.setPath(url.path() + "/index.php/apps/notes/api/v0.2" + QString("/notes/%1").arg(noteId));
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "DELETE" << url.toDisplayString();
        m_request.setUrl(url);
        m_replies << m_manager.deleteResource(m_request);
        emit busyChanged(busy());
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
    m_online = accessible == QNetworkAccessManager::Accessible;
    emit networkAccessibleChanged(m_online);
}

void NotesApi::replyFinished(QNetworkReply *reply) {
    //qDebug() << reply->error() << reply->errorString();
    bool deleteReply = false;
    if (reply->error() == QNetworkReply::NoError) {
        emit error(NoError);
        QByteArray data = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(data);
        if (m_login_replies.contains(reply)) {
            if (json.isObject())
                updateLoginFlow(json.object());
            m_login_replies.removeAll(reply);
        }
        else if (m_poll_replies.contains(reply)) {
            if (json.isObject())
                updateLoginCredentials(json.object());
            m_poll_replies.removeAll(reply);
            m_loginPollTimer.stop();
            m_loginUrl.clear();
        }
        else if (m_status_replies.contains(reply)) {
            if (json.isObject())
                updateStatus(json.object());
            m_status_replies.removeAll(reply);
        }
        else {
            if (mp_model) {
                if (mp_model->fromJsonDocument(json)) {
                    m_lastSync = QDateTime::currentDateTimeUtc();
                    emit lastSyncChanged(m_lastSync);
                }
            }
            m_replies.removeAll(reply);
        }
        //qDebug() << data;
        deleteReply = true;
    }
    else if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
        emit error(AuthenticationError);
        deleteReply = true;
    }
    else {
        if (!m_poll_replies.contains(reply)) {
            emit error(CommunicationError);
            deleteReply = true;
        }
        else {
            //qDebug() << "Polling not finished yet" << m_pollUrl;
        }
    }
    emit busyChanged(busy());
    //if (deleteReply) reply->deleteLater();
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

void NotesApi::updateStatus(const QJsonObject &status) {
    if (!status.isEmpty()) {
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
            qDebug() << "Login URL: " << m_loginUrl;
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
        if (serverAddr != server())
            setServer(serverAddr);
        loginName = credentials.value("loginName").toString();
        if (loginName != username())
            setUsername(loginName);
        appPassword = credentials.value("appPassword").toString();
        if (appPassword != password())
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
