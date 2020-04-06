#include "notesstore.h"

#include "note.h"

#include <QDebug>

NotesStore::NotesStore(QString directory, QObject *parent) : NotesInterface(parent)
{
    m_dir.setCurrent(directory);
    m_dir.setPath("");
    m_dir.setFilter(QDir::Files);
    m_dir.setNameFilters( { "*.json" } );
}

NotesStore::~NotesStore() {
}

QString NotesStore::account() const {
    if (m_dir != QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))) {
        return m_dir.path();
    }
    return QString();
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

void NotesStore::getAllNotes(NoteField exclude) {
    QFileInfoList files = m_dir.entryInfoList();
    for (int i = 0; i < files.size(); ++i) {
        bool ok;
        int id = files[i].baseName().toInt(&ok);
        if (ok) {
            getNote(id, exclude);
        }
    }
}

void NotesStore::getNote(const int id, NoteField exclude) {
    if (id >= 0) {
        QJsonObject file = readNoteFile(id, exclude);
        if (!file.empty())
            emit noteUpdated(file);
    }
}

void NotesStore::createNote(const QJsonObject& note) {
    if (Note::id(note) < 0) {
        // TODO probably crate files with an '.json.<NUMBER>.new' extension
        qDebug() << "Creating notes without the server API is not supported yet!";
    }
    else if (!noteFileExists(Note::id(note))) {
        if (writeNoteFile(note)) {
            emit noteUpdated(note);
        }
    }
}

void NotesStore::updateNote(const QJsonObject& note) {
    if (Note::id(note) >= 0) {
        QJsonObject file = readNoteFile(Note::id(note));
        if (!Note(file).equal(note)) {
            if (writeNoteFile(note)) {
                emit noteUpdated(note);
            }
        }
    }
}

void NotesStore::deleteNote(const int id) {
    if (removeNoteFile(id)) {
        emit noteDeleted(id);
    }
}

bool NotesStore::noteFileExists(const int id) const {
    QFileInfo fileinfo(m_dir, QString("%1.json").arg(id));
    return fileinfo.exists();
}

QJsonObject NotesStore::readNoteFile(const int id, NoteField exclude) const {
    QJsonObject json;
    QFileInfo fileinfo(m_dir, QString("%1.json").arg(id));
    QFile file(fileinfo.filePath());
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            json = QJsonDocument::fromJson(data).object();
            file.close();
            QFlags<NoteField> flags(exclude);
            QMapIterator<NoteField, QString> fields(m_noteFieldNames);
            while (fields.hasNext()) {
                fields.next();
                if (flags.testFlag(fields.key())) {
                    json.remove(fields.value());
                }
            }
        }
    }
    return json;
}

bool NotesStore::writeNoteFile(const QJsonObject &note) const {
    bool success = false;
    QJsonDocument json(note);
    QFileInfo fileinfo(m_dir, QString("%1.json").arg(Note::id(note)));
    QFile file(fileinfo.filePath());
    if (file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
        QByteArray data = json.toJson();
        if (file.write(data) == data.size()) {
            success = true;
        }
        file.close();
    }
    return success;
}

bool NotesStore::removeNoteFile(const int id) const {
    bool success = false;
    QFileInfo fileinfo(m_dir, QString("%1.json").arg(id));
    QFile file(fileinfo.filePath());
    if (file.exists()) {
        if (file.remove()) {
            success = true;
        }
    }
    return success;
}
