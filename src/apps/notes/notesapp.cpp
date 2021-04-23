#include "notesapp.h"

NotesApp::NotesApp(QObject *parent, QString name, NextcloudApi* api)
    : AbstractNextcloudApp(parent, name, api) {
    m_notesProxy.setSourceModel(&m_notesModel);
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
    return m_api->get(NOTES_APP_ENDPOINT, query);
}

bool NotesApp::getNote(const int id) {
    qDebug() << "Getting note: " << id;
    return m_api->get(NOTES_APP_ENDPOINT + QString("/%1").arg(id));
}

bool NotesApp::createNote(const QJsonObject& note, bool local) {
    qDebug() << "Creating note";
    QJsonValue value = QJsonValue(note);
    if (!m_notes.contains(value)) {
        m_notes.append(value);
    }
    if (!local) {
        return m_api->post(NOTES_APP_ENDPOINT, QJsonDocument(note).toJson());
    }
    return true;
}

bool NotesApp::updateNote(const int id, const QJsonObject& note, bool local) {
    qDebug() << "Updating note: " << id;
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
        return m_api->put(NOTES_APP_ENDPOINT + QString("/%1").arg(id), QJsonDocument(note).toJson());
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
        return m_api->del(NOTES_APP_ENDPOINT + QString("/%1").arg(id));
    }
    return done;
}

void NotesApp::updateReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError)
        qDebug() << reply->error() << reply->errorString();

    QByteArray data = reply->readAll();
    QJsonDocument json = QJsonDocument::fromJson(data);
}
