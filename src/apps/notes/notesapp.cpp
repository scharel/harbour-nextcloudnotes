#include "notesapp.h"

NotesApp::NotesApp(QObject *parent, NextcloudApi* api)
    : AbstractNextcloudApp(parent, "notes", api) {
    m_notesProxy.setSourceModel(&m_notesModel);
    connect(api, SIGNAL(readyChanged(bool)), this, SLOT(onApiReady(bool)));
    connect(this, SIGNAL(capabilitiesChanged(QJsonObject*)), this, SLOT(updateCapabilities(QJsonObject*)));
    connect(this, SIGNAL(replyReceived(QNetworkReply*)), this, SLOT(updateReply(QNetworkReply*)));
}

// QML singleton
AbstractNextcloudApp * NotesApp::instance = nullptr;

void NotesApp::instantiate(QObject *parent, NextcloudApi *api) {
    if (instance == nullptr) {
        instance = new NotesApp(parent, api);
    }
}

AbstractNextcloudApp & NotesApp::getInstance() {
    return *instance;
}

QObject * NotesApp::provider(QQmlEngine *, QJSEngine *) {
    return instance;
}

QVersionNumber NotesApp::serverVersion() const {
    return QVersionNumber::fromString(m_capabilities.value("version").toString());
}

QList<QVersionNumber> NotesApp::apiVersions() const {
    QJsonArray jsonVersions = m_capabilities.value("api_version").toArray();
    QList<QVersionNumber> versions;
    QJsonArray::const_iterator i;
    for (i = jsonVersions.begin(); i != jsonVersions.end(); ++i) {
        versions << QVersionNumber::fromString(i->toString());
    }
    return versions;
}

// FIXME
void NotesApp::updateUrl(QUrl* url) {
    if (url->isValid() && m_api->ready()) {
        QUrl myUrl = m_api->url();
        myUrl.setUserInfo(QString());
        m_notesModel.setAccount(m_api->accountHash(m_api->username(), myUrl.toString()));
    }
}

bool NotesApp::getAllNotes(const QStringList& exclude) {
    qDebug() << "Getting all notes";
    QUrlQuery query;
    if (!exclude.isEmpty())
        query.addQueryItem(EXCLUDE_QUERY, exclude.join(","));
    QNetworkReply* reply = m_api->get(QString(NOTES_APP_ENDPOINT).append("/notes"), query);
    if (reply->error() == QNetworkReply::NoError) {
        m_replies << reply;
    }
    else {
        reply->deleteLater();
        return false;
    }
    return true;
}

bool NotesApp::getNote(const int id) {
    qDebug() << "Getting note: " << id;
    QNetworkReply* reply = m_api->get(QString(NOTES_APP_ENDPOINT).append("/notes/%1").arg(id));
    if (reply->error() == QNetworkReply::NoError) {
        m_replies << reply;
    }
    else {
        reply->deleteLater();
        return false;
    }
    return true;
}

bool NotesApp::createNote(const QJsonObject& note, bool local) {
    qDebug() << "Creating note";
    QJsonValue value = QJsonValue(note);
    if (!local) {
        QNetworkReply* reply = m_api->post(QString(NOTES_APP_ENDPOINT).append("/notes"), QJsonDocument(note).toJson());
        if (reply->error() == QNetworkReply::NoError) {
            m_replies << reply;
        }
        else {
            reply->deleteLater();
            return false;
        }
    }
    // TODO update model
    m_notesModel.insert(-1, note);
    return true;
}

bool NotesApp::updateNote(const int id, const QJsonObject& note, bool local) {
    qDebug() << "Updating note:" << id;
    if (!local) {
        QNetworkReply* reply = m_api->put(QString(NOTES_APP_ENDPOINT).append("/notes/%1").arg(id), QJsonDocument(note).toJson());
        if (reply->error() == QNetworkReply::NoError) {
            m_replies << reply;
            // TODO update model
        }
        else {
            reply->deleteLater();
            return false;
        }
    }
    m_notesModel.update(-1, note);
    return true;
}

bool NotesApp::deleteNote(const int id, bool local) {
    qDebug() << "Deleting note: " << id;
    if (!local) {
        QNetworkReply* reply = m_api->del(QString(NOTES_APP_ENDPOINT).append("/notes/%1").arg(id));
        if (reply->error() == QNetworkReply::NoError) {
            m_replies << reply;
        }
        else {
            reply->deleteLater();
            return false;
        }
    }
    m_notesModel.remove(id);
    return false;
}

void NotesApp::updateReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError)
        qDebug() << reply->error() << reply->errorString();

    QByteArray data = reply->readAll();
    //qDebug() << data;
    QJsonDocument json = QJsonDocument::fromJson(data);
    //qDebug() << json;
    if (json.isObject()) {
        QJsonObject obj = json.object();
        updateNote(obj.value("id").toInt(), obj, true);
    }
    else if (json.isArray()) {
        QJsonArray arr = json.array();
        QJsonArray::iterator i;
        for (i = arr.begin(); i != arr.end(); ++i) {
            QJsonObject obj = i->toObject();
            updateNote(obj.value("id").toInt(), obj, true);
        }
    }
    else {
        qDebug() << "Reply not valid!";
    }
}
