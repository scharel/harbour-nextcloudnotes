#include "notesstore.h"

#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>

const QString NotesStore::m_suffix = "json";

NotesStore::NotesStore(QString directory, QObject *parent)
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
    qDebug() << "Setting account: " << account;
    if (account != m_dir.path()) {
        if (m_dir != QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))) {
            m_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
        }
        if (!account.isEmpty()) {
            m_dir.setPath(account);
            if (m_dir.mkpath(".")) {
                emit accountChanged(m_dir.path());
            }
            else {
                qDebug() << "Failed to create or already present: " << m_dir.path();
                m_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
                emit noteError(DirCannotWriteError);
            }
        }
        //qDebug() << account << m_dir.path();
    }
}

const QString NotesStore::errorMessage(ErrorCodes error) const {
    QString message;
    switch (error) {
    case NoError:
        message = tr("No error");
        break;
    case FileNotFoundError:
        message = tr("File not found");
        break;
    case FileCannotReadError:
        message = tr("Cannot read from the file");
        break;
    case FileCannotWriteError:
        message = tr("Cannot write to the file");
        break;
    case DirNotFoundError:
        message = tr("Directory not found");
        break;
    case DirCannotReadError:
        message = tr("Cannot read from directory");
        break;
    case DirCannotWriteError:
        message = tr("Cannot create or write to directory");
        break;
    default:
        message = tr("Unknown error");
        break;
    }
    return message;
}

const QJsonArray NotesStore::getAllNotes(const QStringList& exclude) {
    qDebug() << "Getting all notes";
    QJsonArray notes;
    if (m_dir.exists()) {
        QFileInfoList files = m_dir.entryInfoList();
        for (int i = 0; i < files.size(); ++i) {
            bool ok;
            int id = files[i].baseName().toInt(&ok);
            if (ok) {
                notes.append(QJsonValue(getNote(id, exclude)));
            }
        }
    }
    else {
        qDebug() << errorMessage(DirNotFoundError);
        emit noteError(DirCannotReadError);
    }
    return notes;
}

const QJsonObject NotesStore::getNote(const int id, const QStringList& exclude) {
    qDebug() << "Getting note: " << id;
    QJsonObject note;
    if (id >= 0) {
        note = readNoteFile(id, exclude);
    }
    else {
        qDebug() << "Skipping, invalid ID";
    }
    return note;
}

bool NotesStore::createNote(const QJsonObject& note) {
    int id = note.value("id").toInt(-1);
    qDebug() << "Creating note: " << id;
    if (id < 0) {
        // TODO probably crate files with an '.json.<NUMBER>.new' extension
        qDebug() << "Creating notes without the server API is not supported yet!";
    }
    else if (!noteFileExists(id)) {
        if (writeNoteFile(id, note)) {
            emit noteUpdated(id, note);
            return true;
        }
    }
    else {
        qDebug() << "Note already exists";
    }
    return false;
}

const QJsonObject NotesStore::updateNote(const int id, const QJsonObject& note) {
    qDebug() << "Updating note: " << id;
    if (id >= 0) {
        QJsonObject tmpNote = readNoteFile(id);
        if (note != tmpNote) {
            if (note.value("modified").toInt() >= tmpNote.value("modified").toInt() || note.value("modified").toInt() == 0) {
                QStringList fields = note.keys();
                for (int i = 0; i < fields.size(); ++i) {
                    tmpNote[fields[i]] = note[fields[i]];
                }
                if (tmpNote.value("modified").toInt() == 0) {
                    tmpNote["modified"] = QJsonValue::fromVariant(QDateTime::currentDateTime().toTime_t());
                }
                if (writeNoteFile(id, tmpNote)) {
                    emit noteUpdated(id, tmpNote);
                }
            }
            else {
                qDebug() << "Skipping, note is older" << QDateTime::fromTime_t(note.value("modified").toInt()) << QDateTime::fromTime_t(tmpNote.value("modified").toInt());
            }
        }
        else {
            qDebug() << "Skipping, note is equal";
        }
    }
    else {
        qDebug() << "Skipping, invalid ID";
    }
}

bool NotesStore::deleteNote(const int id) {
    qDebug() << "Deleting note: " << id;
    if (removeNoteFile(id)) {
        emit noteDeleted(id);
    }
}

bool NotesStore::noteFileExists(const int id) const {
    QFileInfo fileinfo(m_dir, QString("%1.%2").arg(id).arg(m_suffix));
    return fileinfo.exists();
}

QJsonObject NotesStore::readNoteFile(const int id, const QStringList& exclude) {
    QJsonObject json;
    QFileInfo fileinfo(m_dir, QString("%1.%2").arg(id).arg(m_suffix));
    QFile file(fileinfo.filePath());
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            json = QJsonDocument::fromJson(data).object();
            file.close();
            for (int i = 0; i < exclude.size(); ++i) {
                json.remove(exclude[i]);
            }
        }
        else {
            emit noteError(FileCannotReadError);
        }
    }
    else {
        //emit noteError(FileNotFoundError);
    }
    return json;
}

bool NotesStore::writeNoteFile(const int id, const QJsonObject& note) {
    bool success = false;
    if (!account().isEmpty()) {
        QFileInfo fileinfo(m_dir, QString("%1.%2").arg(id).arg(m_suffix));
        QFile file(fileinfo.filePath());
        if (file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
            QByteArray data = QJsonDocument(note).toJson();
            if (file.write(data) == data.size()) {
                success = true;
            }
            file.close();
        }
        else {
            emit noteError(FileCannotWriteError);
        }
    }
    return success;
}

bool NotesStore::removeNoteFile(const int id) {
    bool success = false;
    if (!account().isEmpty()) {
        QFileInfo fileinfo(m_dir, QString("%1.%2").arg(id).arg(m_suffix));
        QFile file(fileinfo.filePath());
        if (file.exists()) {
            if (file.remove()) {
                success = true;
            }
            else {
                emit noteError(FileCannotWriteError);
            }
        }
        else {
            emit noteError(FileNotFoundError);
        }
    }
    return success;
}
