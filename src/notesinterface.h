#ifndef NOTESINTERFACE_H
#define NOTESINTERFACE_H

#include <QObject>
#include <QMap>
#include <QJsonObject>

class NotesInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString account READ account WRITE setAccount NOTIFY accountChanged)
    Q_CLASSINFO("author", "Scharel Clemens")
    Q_CLASSINFO("url", "https://github.com/scharel/harbour-nextcloudnotes")

public:
    explicit NotesInterface(QObject *parent = nullptr) : QObject(parent) {
        m_noteFieldNames = QMap<NoteField, QString>( { {Id, "id"}, {Modified, "modified"}, {Title, "title"}, {Category, "category"}, {Content, "content"}, {Favorite, "favorite"}, {Etag, "etag"}, {Error, "error"}, {ErrorMessage, "errorMessage"} } );
    }

    virtual QString account() const = 0;
    virtual void setAccount(const QString& account) = 0;

    enum NoteField {
        None = 0x0000,
        Id = 0x0001,
        Modified = 0x0002,
        Title = 0x0004,
        Category = 0x0008,
        Content = 0x0010,
        Favorite = 0x0020,
        Etag = 0x0040,
        Error = 0x0080,
        ErrorMessage = 0x0100
    };
    Q_DECLARE_FLAGS(NoteFields, NoteField)
    Q_FLAG(NoteFields)
    Q_INVOKABLE QList<NoteField> noteFields() const {
        return m_noteFieldNames.keys();
    }
    Q_INVOKABLE QString noteFieldName(NoteField field) {
        return m_noteFieldNames[field];
    }

    Q_INVOKABLE virtual void getAllNotes(NoteField exclude = None) = 0;
    Q_INVOKABLE virtual void getNote(const int id, NoteField exclude = None) = 0;
    Q_INVOKABLE virtual void createNote(const QJsonObject& note) = 0;
    Q_INVOKABLE virtual void updateNote(const QJsonObject& note) = 0;
    Q_INVOKABLE virtual void deleteNote(const int id) = 0;

signals:
    void accountChanged(const QString account);
    void noteUpdated(const QJsonObject note);
    void noteDeleted(const int deletedNoteId);

public slots:

protected:
    QMap<NoteField, QString> m_noteFieldNames;

};

#endif // NOTESINTERFACE_H
