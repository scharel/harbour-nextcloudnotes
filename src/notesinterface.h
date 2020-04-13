#ifndef NOTESINTERFACE_H
#define NOTESINTERFACE_H

#include <QObject>
#include <QJsonObject>

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
    Q_INVOKABLE virtual const QString errorMessage(int error) const = 0;

public slots:
    Q_INVOKABLE virtual void getAllNotes(const QStringList& exclude = QStringList()) = 0;
    //virtual void getAllNotes(Note::NoteField exclude) = 0;
    Q_INVOKABLE virtual void getNote(const int id, const QStringList& exclude = QStringList()) = 0;
    //virtual void getNote(const int id, Note::NoteField) = 0;
    Q_INVOKABLE virtual void createNote(const QJsonObject& note) = 0;
    //virtual void createNote(const Note& note) = 0;
    Q_INVOKABLE virtual void updateNote(const int id, const QJsonObject& note) = 0;
    //virtual void updateNote(const Note& note) = 0;
    Q_INVOKABLE virtual void deleteNote(const int id) = 0;

signals:
    void accountChanged(const QString& account);
    void noteError(int error);
    void noteUpdated(const int id, const QJsonObject& note);
    //void noteUpdated(const Note& note);
    void noteDeleted(const int id);

};

#endif // NOTESINTERFACE_H
