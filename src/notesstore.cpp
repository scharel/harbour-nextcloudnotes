#include "notesstore.h"

#include <QDebug>

NotesStore::NotesStore(QString directory, QObject *parent) : NotesInterface(parent)
{
    m_dir.setCurrent(directory);
    m_dir.setPath("");
    m_dir.setFilter(QDir::Files);
    m_dir.setNameFilters( { "*.json" } );
    m_note = nullptr;
}

NotesStore::~NotesStore() {
    if (m_note)
        m_note->deleteLater();
    m_note = nullptr;
}

QString NotesStore::account() const {
    QString dir;
    if (m_dir != QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))) {
        dir = m_dir.dirName();
    }
    return dir;
}

void NotesStore::setAccount(const QString& account) {
    //qDebug() << account << m_dir.path();
    if (account != m_dir.path()) {
        if (m_dir != QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))) {
            m_dir.cdUp();
        }
        if (!account.isEmpty()) {
            m_dir.setPath(account);
            if (!m_dir.mkpath(".")) {
                qDebug() << "Failed to create or already present: " << m_dir.path();
            }
        }
        //qDebug() << account << m_dir.path();
        emit accountChanged(m_dir.path());
    }
}

void NotesStore::getAllNotes() {
    QFileInfoList files = m_dir.entryInfoList();
    for (int i = 0; i < files.size(); ++i) {
        bool ok;
        int id = files[i].baseName().toInt(&ok);
        if (ok) {
            getNote(id);
        }
    }
}

void NotesStore::getNote(const int id) {
    QFileInfo file(m_dir, QString("%1.json").arg(id));
    if (file.exists()) {
        emit noteUpdated(noteData(id));
    }
}

void NotesStore::getNote(const Note& note) {
    getNote(note.id());
}

void NotesStore::createNote(const Note& note) {
    if (note.id() < 0) {
        // TODO probably crate files with an '.json.<NUMBER>.new' extension
        qDebug() << "Creating notes without the server API is not supported yet!";
    }
    else {
        updateNote(note);
    }
}

void NotesStore::updateNote(const Note& note) {
    if (note.id() >= 0) {
        QFileInfo fileinfo(m_dir, QString("%1.json").arg(note.id()));
        QFile file(fileinfo.filePath());
        if (file.exists()) {
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QJsonDocument fileJson = QJsonDocument::fromBinaryData(file.readAll());
                file.close();
                if (!note.equal(fileJson.object())) {
                    if (file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
                        QByteArray data = note.toJsonDocument().toJson();
                        if (file.write(data) == data.size()) {
                            emit noteUpdated(noteData(note));
                        }
                    }
                }
                file.close();
            }
        }
        else {
            if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
                QByteArray data = note.toJsonDocument().toJson();
                if (file.write(data) == data.size()) {
                    emit noteCreated(noteData(note));
                }
                file.close();
            }
        }
    }
    else {
        createNote(note);
    }
}

void NotesStore::deleteNote(const int id) {
    QFileInfo fileinfo(m_dir, QString("%1.json").arg(id));
    if (fileinfo.exists()) {
        QFile file(fileinfo.filePath());
        if (file.remove()) {
            emit noteDeleted(id);
            if (m_note)
                m_note->deleteLater();
            m_note = nullptr;
        }
    }
}

void NotesStore::deleteNote(const Note& note) {
    deleteNote(note.id());
}

Note* NotesStore::noteData(const int id) {
    if (m_note)
        m_note->deleteLater();
    m_note = nullptr;
    QFileInfo fileinfo(m_dir, QString("%1.json").arg(id));
    QFile file(fileinfo.filePath());
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            QJsonDocument json = QJsonDocument::fromJson(data);
            m_note = new Note(json.object());
            file.close();
        }
    }
    return m_note;
}

Note* NotesStore::noteData(const Note& note) {
    return noteData(note.id());
}
