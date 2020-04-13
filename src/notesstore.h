#ifndef NOTESSTORE_H
#define NOTESSTORE_H

#include <QObject>
#include <QStandardPaths>
#include <QDir>

#include "notesinterface.h"

class NotesStore : public NotesInterface
{
    Q_OBJECT

public:
    explicit NotesStore(
            QString directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation),
            QObject *parent = nullptr);
    virtual ~NotesStore();

    QString account() const;
    void setAccount(const QString& account);

    enum ErrorCodes {
        NoError,
        FileNotFoundError,
        FileCannotReadError,
        FileCannotWriteError,
        DirNotFoundError,
        DirCannotReadError,
        DirCannotWriteError
    };
    Q_ENUM(ErrorCodes)
    Q_INVOKABLE const QString errorMessage(int error) const;

public slots:
    Q_INVOKABLE void getAllNotes(const QStringList& exclude = QStringList());
    //void getAllNotes(Note::NoteField exclude);
    Q_INVOKABLE void getNote(const int id, const QStringList& exclude = QStringList());
    //void getNote(const int id, Note::NoteField exclude);
    Q_INVOKABLE void createNote(const QJsonObject& note);
    //void createNote(const Note& note);
    Q_INVOKABLE void updateNote(const int id, const QJsonObject& note);
    //void updateNote(const Note& note);
    Q_INVOKABLE void deleteNote(const int id);

private:
    QDir m_dir;
    const static QString m_suffix;

    bool noteFileExists(const int id) const;
    QJsonObject readNoteFile(const int id, const QStringList& exclude = QStringList());
    bool writeNoteFile(const int id, const QJsonObject& note);
    bool removeNoteFile(const int id);
};

#endif // NOTESSTORE_H
