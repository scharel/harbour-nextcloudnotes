#include "notesstore.h"

#include <QDebug>

const QString NotesStore::m_suffix = "json";

NotesStore::NotesStore(QString directory, QObject *parent) : NotesInterface(parent)
{
    m_dir.setCurrent(directory);
    m_dir.setPath("");
    m_dir.setFilter(QDir::Files);
    m_dir.setNameFilters( { "*." + m_suffix } );
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
    //qDebug() << "Setting account: " << account;
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

void NotesStore::getAllNotes(Note::NoteField exclude) {
    //qDebug() << "Getting all notes";
    QFileInfoList files = m_dir.entryInfoList();
    for (int i = 0; i < files.size(); ++i) {
        bool ok;
        int id = files[i].baseName().toInt(&ok);
        if (ok) {
            getNote(id, exclude);
        }
    }
}

void NotesStore::getNote(const int id, Note::NoteField exclude) {
    //qDebug() << "Getting note: " << id;
    if (id >= 0) {
        Note note = readNoteFile(id, exclude);
        if (note.isValid())
            emit noteUpdated(note);
    }
}

void NotesStore::createNote(const Note& note) {
    //qDebug() << "Creating note: " << note.id();
    if (!note.isValid()) {
        // TODO probably crate files with an '.json.<NUMBER>.new' extension
        qDebug() << "Creating notes without the server API is not supported yet!";
    }
    else if (!noteFileExists(note.id())) {
        if (writeNoteFile(note)) {
            emit noteUpdated(note);
        }
    }
}

void NotesStore::updateNote(const Note& note) {
    //qDebug() << "Updating note: " << note.id();
    if (note.isValid()) {
        Note file = readNoteFile(note.id());
        if (!file.equal(note) && note > file) {
            if (writeNoteFile(note)) {
                emit noteUpdated(note);
            }
        }
    }
}

void NotesStore::deleteNote(const int id) {
    //qDebug() << "Deleting note: " << id;
    if (removeNoteFile(id)) {
        emit noteDeleted(id);
    }
}

bool NotesStore::noteFileExists(const int id) const {
    QFileInfo fileinfo(m_dir, QString("%1.%2").arg(id).arg(m_suffix));
    return fileinfo.exists();
}

Note NotesStore::readNoteFile(const int id, Note::NoteField exclude) const {
    QJsonObject json;
    QFileInfo fileinfo(m_dir, QString("%1.%2").arg(id).arg(m_suffix));
    QFile file(fileinfo.filePath());
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            json = QJsonDocument::fromJson(data).object();
            file.close();
            QList<Note::NoteField> noteFields = Note::noteFields();
            QFlags<Note::NoteField> flags(exclude);
            for (int i = 0; i < noteFields.size(); ++i) {
                if (flags.testFlag(noteFields[i])) {
                    json.remove(Note::noteFieldName(noteFields[i]));
                }
            }
        }
    }
    return json;
}

bool NotesStore::writeNoteFile(const Note &note) const {
    bool success = false;
    if (!account().isEmpty()) {
        QJsonDocument json = note.toJsonDocument();
        QFileInfo fileinfo(m_dir, QString("%1.%2").arg(note.id()).arg(m_suffix));
        QFile file(fileinfo.filePath());
        if (file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
            QByteArray data = json.toJson();
            if (file.write(data) == data.size()) {
                success = true;
            }
            file.close();
        }
    }
    return success;
}

bool NotesStore::removeNoteFile(const int id) const {
    bool success = false;
    if (!account().isEmpty()) {
        QFileInfo fileinfo(m_dir, QString("%1.%2").arg(id).arg(m_suffix));
        QFile file(fileinfo.filePath());
        if (file.exists()) {
            if (file.remove()) {
                success = true;
            }
        }
    }
    return success;
}
