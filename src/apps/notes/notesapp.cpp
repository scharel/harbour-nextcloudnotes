#include "notesapp.h"

NotesApp::NotesApp(QObject *parent, NextcloudApi* api)
    : AbstractNextcloudApp(parent, "notes", api) {
    m_notesProxy.setSourceModel(&m_notesModel);
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

bool NotesApp::getAllNotes(const QStringList& exclude) {
    qDebug() << "Getting all notes";
    QUrlQuery query;
    if (!exclude.isEmpty())
        query.addQueryItem(EXCLUDE_QUERY, exclude.join(","));
    QNetworkReply* reply = m_api->get(QString(NOTES_APP_ENDPOINT).append("/notes"), query);
    if (reply->error() == QNetworkReply::NoError) {
        m_replies << reply;
        return true;
    }
    else {
        reply->deleteLater();
        return false;
    }
}

bool NotesApp::getNote(const int id) {
    qDebug() << "Getting note: " << id;
    QNetworkReply* reply = m_api->get(QString(NOTES_APP_ENDPOINT).append("/notes/%1").arg(id));
    if (reply->error() == QNetworkReply::NoError) {
        m_replies << reply;
        return true;
    }
    else {
        reply->deleteLater();
        return false;
    }
}

bool NotesApp::createNote(const QJsonObject& note, bool local) {
    qDebug() << "Creating note";
    QJsonValue value = QJsonValue(note);
    if (!m_notes.contains(value)) {
        m_notes.append(value);
    }
    if (!local) {
        QNetworkReply* reply = m_api->post(QString(NOTES_APP_ENDPOINT).append("/notes"), QJsonDocument(note).toJson());
        if (reply->error() == QNetworkReply::NoError) {
            m_replies << reply;
            return true;
        }
        else {
            reply->deleteLater();
            return false;
        }
    }
    return true;
}

bool NotesApp::updateNote(const int id, const QJsonObject& note, bool local) {
    qDebug() << "Updating note:" << id;
    bool done = true;
    if (!m_notes.contains(QJsonValue(note))) {
        done = false;
        QJsonArray::iterator i;
        for (i = m_notes.begin(); i != m_notes.end() && !done; ++i) {
            QJsonObject localNote = i->toObject();
            int localId = localNote.value("id").toInt(-1);
            if (localId > 0) {
                if (localId == id) {
                    *i = QJsonValue(note);
                    done = true;
                }
            }
            else {
                if (localNote.value("content") == note.value("content")) {
                    *i = QJsonValue(note);
                    done = true;
                }
            }
        }
    }
    if (!local) {
        QNetworkReply* reply = m_api->put(QString(NOTES_APP_ENDPOINT).append("/notes/%1").arg(id), QJsonDocument(note).toJson());
        if (reply->error() == QNetworkReply::NoError) {
            m_replies << reply;
            return true;
        }
        else {
            reply->deleteLater();
            return false;
        }
    }
    return done;
}

bool NotesApp::deleteNote(const int id, bool local) {
    qDebug() << "Deleting note: " << id;
    bool done = false;
    QJsonArray::iterator i;
    for (i = m_notes.begin(); i != m_notes.end() && !done; ++i) {
        QJsonObject localNote = i->toObject();
        if (localNote.value("id").toInt() == id) {
            m_notes.erase(i);
            done = true;
        }
    }
    if (!local) {
        QNetworkReply* reply = m_api->del(QString(NOTES_APP_ENDPOINT).append("/notes/%1").arg(id));
        if (reply->error() == QNetworkReply::NoError) {
            m_replies << reply;
            return true;
        }
        else {
            reply->deleteLater();
            return false;
        }
    }
    return done;
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
