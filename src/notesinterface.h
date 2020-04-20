#ifndef NOTESINTERFACE_H
#define NOTESINTERFACE_H

//#include <QObject>

class NotesInterface
{
    //Q_CLASSINFO("author", "Scharel Clemens")
    //Q_CLASSINFO("url", "https://github.com/scharel/harbour-nextcloudnotes")

    Q_PROPERTY(QString account READ account WRITE setAccount NOTIFY accountChanged)

public:
    explicit NotesInterface() {
    }

    virtual QString account() const = 0;
    virtual void setAccount(const QString& account) = 0;

public slots:
    Q_INVOKABLE virtual bool getAllNotes(const QStringList& exclude = QStringList()) = 0;
    Q_INVOKABLE virtual bool getNote(const int id, const QStringList& exclude = QStringList()) = 0;
    Q_INVOKABLE virtual bool createNote(const QJsonObject& note) = 0;
    Q_INVOKABLE virtual bool updateNote(const int id, const QJsonObject& note) = 0;
    Q_INVOKABLE virtual bool deleteNote(const int id) = 0;

signals:
    virtual void accountChanged(const QString& account) = 0;
    virtual void allNotesChanged(const QList<int>& ids) = 0;
    virtual void noteCreated(const int id, const QJsonObject& note) = 0;
    virtual void noteUpdated(const int id, const QJsonObject& note) = 0;
    virtual void noteDeleted(const int id) = 0;

};

#endif // NOTESINTERFACE_H
