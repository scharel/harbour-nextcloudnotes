#include "notesapi.h"
#include <QGuiApplication>
#include <QAuthenticator>
#include <QJsonDocument>
#include <QJsonObject>

NotesApi::NotesApi(QObject *parent) : QObject(parent)
{
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
    m_request.setHeader(QNetworkRequest::UserAgentHeader, QGuiApplication::applicationDisplayName() +  " / " + QGuiApplication::applicationVersion());
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
        bool oldReady = ready();
        m_url = url;
        emit urlChanged(m_url);
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
        if (ready() != oldReady)
            emit readyChanged(ready());
        if (m_url.isValid())
            qDebug() << "API URL:" << m_url.toDisplayString();
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
    if (port >= -1 && port <= 65535) {
        QUrl url = m_url;
        url.setPort(port);
        setUrl(url);
    }
}

void NotesApi::setUsername(QString username) {
    if (!username.isEmpty()) {
        QUrl url = m_url;
        url.setUserName(username);
        QString concatenated = username + ":" + password();
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

bool NotesApi::ready() const {
    return !m_url.scheme().isEmpty() &&
            !m_url.userName().isEmpty() &&
            !m_url.host().isEmpty() &&
            !m_url.path().isEmpty() &&
            !m_url.query().isEmpty();
}

bool NotesApi::busy() const {
    bool busy = false;
    QVector<QNetworkReply*> replies;
    replies << m_replies << m_status_replies << m_login_replies;
    for (int i = 0; i < replies.size(); ++i) {
        busy |= replies[i]->isRunning();
    }
    return busy;
}

void NotesApi::getStatus() {
    QUrl url = m_url;
    url.setPath("/index.php/login/v2");
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "POST" << url.toDisplayString();
        m_request.setUrl(url);
        m_login_replies << m_manager.post(m_request, QByteArray());
        emit busyChanged(busy());
    }
}

void NotesApi::initiateFlowV2Login() {
    QUrl url = m_url;
    url.setPath("/status.php");
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        qDebug() << "POST" << url.toDisplayString();
        m_request.setUrl(url);
        m_status_replies << m_manager.post(m_request, QByteArray());
        emit busyChanged(busy());
    }
}

void NotesApi::getAllNotes(QStringList excludeFields) {
    QUrl url = m_url;
    url.setPath(url.path() + "/notes");
    if (!excludeFields.isEmpty())
        url.setQuery(QString("exclude=").append(excludeFields.join(",")));
    if (url.isValid()) {
        qDebug() << "GET" << url.toDisplayString();
        m_request.setUrl(url);
        m_replies << m_manager.get(m_request);
        emit busyChanged(busy());
    }
}

void NotesApi::getNote(double noteId, QStringList excludeFields) {
    QUrl url = m_url;
    url.setPath(url.path() + QString("/notes/%1").arg(noteId));
    if (!excludeFields.isEmpty())
        url.setQuery(QString("exclude=").append(excludeFields.join(",")));
    if (url.isValid()) {
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
    url.setPath(url.path() + "/notes");
    if (url.isValid()) {
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
    url.setPath(url.path() + QString("/notes/%1").arg(noteId));
    if (url.isValid()) {
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
    url.setPath(url.path() + QString("/notes/%1").arg(noteId));
    if (url.isValid()) {
        qDebug() << "DELETE" << url.toDisplayString();
        m_request.setUrl(url);
        m_replies << m_manager.deleteResource(m_request);
        emit busyChanged(busy());
    }
    mp_model->removeNote(noteId);
}

void NotesApi::verifyUrl(QUrl url) {
    emit urlValidChanged(url.isValid());
    if (url.isValid()) {
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
    qDebug() << reply->error() << reply->errorString();
    if (reply->error() == QNetworkReply::NoError) {
        emit error(NoError);
        QByteArray data = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(data);
        if (m_login_replies.contains(reply)) {
            if (json.isObject()) {
                updateLogin(json.object());
            }
        }
        if (m_status_replies.contains(reply)) {
            if (json.isObject()) {
                updateStatus(json.object());
            }
        }
        else {
            if (mp_model) {
                if (mp_model->fromJsonDocument(json)) {
                    m_lastSync = QDateTime::currentDateTimeUtc();
                    emit lastSyncChanged(m_lastSync);
                }
            }
        }
        //qDebug() << data;
    }
    else if (reply->error() == QNetworkReply::AuthenticationRequiredError)
        emit error(AuthenticationError);
    else
        emit error(CommunicationError);
    m_login_replies.removeAll(reply);
    m_status_replies.removeAll(reply);
    m_replies.removeAll(reply);
    reply->deleteLater();
    emit busyChanged(busy());
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
        QStringList versionStr = status.value("version").toString().split('.');
        QVector<int> version;
        for(int i = 0; i < versionStr.size(); ++i)
            version << versionStr[i].toInt();
        if (m_status_version != version) {
            m_status_version = version;
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
            qDebug() << m_status_productname;
            emit statusProductNameChanged(m_status_productname);
        }
        if (m_status_extendedSupport != status.value("extendedSupport").toBool()) {
            m_status_extendedSupport = status.value("extendedSupport").toBool();
            emit statusExtendedSupportChanged(m_status_extendedSupport);
        }
    }
}

void NotesApi::updateLogin(const QJsonObject &login) {
    QUrl url;
    QString token;
    if (!login.isEmpty()) {
        url = login.value("login").toString();
        if (m_login_loginUrl != url && url.isValid()) {
            m_login_loginUrl = url;
            emit loginLoginUrlChanged(m_login_loginUrl);
        }
        QJsonObject poll = login.value("poll").toObject();
        if (!poll.isEmpty()) {
            url = poll.value("endpoint").toString() ;
            if (m_login_pollUrl != url && urlValid()) {
                m_login_pollUrl = url;
                emit loginPollUrlChanged(m_login_pollUrl);
            }
            token = poll.value("token").toString();
            if (m_login_pollToken != token) {
                m_login_pollToken = token;
                emit loginPollTokenChanged(m_login_pollToken);
            }
        }
    }
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
