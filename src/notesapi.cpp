#include "notesapi.h"
#include <QGuiApplication>
#include <QAuthenticator>
#include <QJsonDocument>
#include <QJsonObject>

NotesApi::NotesApi(QObject *parent) : QObject(parent)
{
    mp_model = new NotesModel(this);
    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(verifyUrl(QUrl)));
    connect(&m_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(requireAuthentication(QNetworkReply*,QAuthenticator*)));
    connect(&m_manager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    connect(&m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(&m_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslError(QNetworkReply*,QList<QSslError>)));
    m_request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    m_request.setHeader(QNetworkRequest::UserAgentHeader, QGuiApplication::applicationDisplayName() +  " / " + QGuiApplication::applicationVersion());
    m_request.setRawHeader("OCS-APIREQUEST", "true");
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json").toUtf8());
}

NotesApi::~NotesApi() {
    delete mp_model;
}

void NotesApi::setSslVerify(bool verify) {
    if (verify != (m_request.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer)) {
        m_request.sslConfiguration().setPeerVerifyMode(verify ? QSslSocket::VerifyPeer : QSslSocket::VerifyNone);
        emit sslVerifyChanged(verify);
    }
}

void NotesApi::requireAuthentication(QNetworkReply *reply, QAuthenticator *authenticator) {
    if (reply && authenticator) {
        authenticator->setUser(username());
        authenticator->setPassword(password());
    }
}

void NotesApi::setUrl(QUrl url) {
    if (url != m_url) {
        QUrl oldUrl = m_url;
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
        qDebug() << "API URL changed:" << m_url.toDisplayString();
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

bool NotesApi::busy() const {
    bool busy = false;
    for (int i = 0; i < m_replies.size(); ++i) {
        busy |=m_replies[i]->isRunning();
    }
    return busy;
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

void NotesApi::getNote(int noteId, QStringList excludeFields) {
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
    QUrl url = m_url;
    url.setPath(url.path() + "/notes");
    if (url.isValid()) {
        qDebug() << "POST" << url.toDisplayString();
        m_request.setUrl(url);
        m_replies << m_manager.post(m_request, QJsonDocument(QJsonObject::fromVariantMap(fields)).toJson());
        emit busyChanged(busy());
    }
}

void NotesApi::updateNote(int noteId, QVariantMap fields) {
    QUrl url = m_url;
    url.setPath(url.path() + QString("/notes/%1").arg(noteId));
    if (url.isValid()) {
        qDebug() << "PUT" << url.toDisplayString();
        m_request.setUrl(url);
        m_replies << m_manager.put(m_request, QJsonDocument(QJsonObject::fromVariantMap(fields)).toJson());
        emit busyChanged(busy());
    }
}

void NotesApi::deleteNote(int noteId) {
    QUrl url = m_url;
    url.setPath(url.path() + QString("/notes/%1").arg(noteId));
    if (url.isValid()) {
        qDebug() << "DELETE" << url.toDisplayString();
        m_request.setUrl(url);
        m_replies << m_manager.deleteResource(m_request);
        emit busyChanged(busy());
    }
}

void NotesApi::verifyUrl(QUrl url) {
    emit urlValidChanged(url.isValid());
}

void NotesApi::onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible) {
    qDebug() << m_manager.networkAccessible();
    emit networkAccessibleChanged(accessible == QNetworkAccessManager::Accessible);
}

void NotesApi::replyFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
        if (mp_model)
            mp_model->applyJSON(json);
        //qDebug() << json;
    }
    else {
        qDebug() << reply->error() << reply->errorString();
    }
    m_replies.removeAll(reply);
    reply->deleteLater();
    emit busyChanged(busy());
}

void NotesApi::sslError(QNetworkReply *reply, const QList<QSslError> &errors) {
    qDebug() << "SSL errors accured while calling" << reply->url().toDisplayString();
    for (int i = 0; i < errors.size(); ++i) {
        qDebug() << errors[i].errorString();
    }
}
