#ifndef NOTESINTERFACE_H
#define NOTESINTERFACE_H

#include <QObject>
#include <QVector>

#include "note.h"

class NotesInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString account READ account WRITE setAccount NOTIFY accountChanged)
    Q_CLASSINFO("author", "Scharel Clemens")
    Q_CLASSINFO("url", "https://github.com/scharel/harbour-nextcloudnotes")

public:
    explicit NotesInterface(QObject *parent = nullptr) : QObject(parent) {
    }

    virtual QString account() const = 0;
    virtual void setAccount(const QString& account) = 0;

    Q_INVOKABLE virtual void getAllNotes() = 0;
    Q_INVOKABLE virtual void getNote(const int id) = 0;
    Q_INVOKABLE virtual void getNote(const Note& note) = 0;
    Q_INVOKABLE virtual void createNote(const Note& note) = 0;
    Q_INVOKABLE virtual void updateNote(const Note& note) = 0;
    Q_INVOKABLE virtual void deleteNote(const int id) = 0;
    Q_INVOKABLE virtual void deleteNote(const Note& note) = 0;
    Q_INVOKABLE virtual Note* noteData(const int id) = 0;
    Q_INVOKABLE virtual Note* noteData(const Note& note) = 0;

signals:
    void accountChanged(const QString& account);
    void noteCreated(Note* createdNote);
    void noteUpdated(Note* updatedNote);
    void noteDeleted(int deletedNoteId);

public slots:

};

#endif // NOTESINTERFACE_H
