#ifndef NOTESSTORE_H
#define NOTESSTORE_H

#include "notesinterface.h"
#include <QObject>
#include <QStandardPaths>
#include <QDir>

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

    Q_INVOKABLE void getAllNotes(NoteField exclude = None);
    Q_INVOKABLE void getNote(const int id, NoteField exclude = None);
    Q_INVOKABLE void createNote(const QJsonObject& note);
    Q_INVOKABLE void updateNote(const QJsonObject& note);
    Q_INVOKABLE void deleteNote(const int id);

signals:

public slots:

private:
    QDir m_dir;

    bool noteFileExists(const int id) const;
    QJsonObject readNoteFile(const int id, NoteField exclude = None) const;
    bool writeNoteFile(const QJsonObject& note) const;
    bool removeNoteFile(const int id) const;
};

#endif // NOTESSTORE_H
