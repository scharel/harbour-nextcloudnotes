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

signals:

public slots:
    Q_INVOKABLE void getAllNotes();
    Q_INVOKABLE void getNote(const int id);
    Q_INVOKABLE void getNote(const Note& note);
    Q_INVOKABLE void createNote(const Note& note);
    Q_INVOKABLE void updateNote(const Note& note);
    Q_INVOKABLE void deleteNote(const int id);
    Q_INVOKABLE void deleteNote(const Note& note);
    Q_INVOKABLE Note* noteData(const int id);
    Q_INVOKABLE Note* noteData(const Note& note);

private:
    QDir m_dir;
    Note* m_note;
};

#endif // NOTESSTORE_H
