#ifndef NOTESSTORE_H
#define NOTESSTORE_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QStandardPaths>

#include "notesinterface.h"

class NotesStore : public QObject, public NotesInterface
{
    Q_OBJECT

public:
    explicit NotesStore(
            QString directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation),
            QObject *parent = nullptr);
    virtual ~NotesStore();

    Q_PROPERTY(QString account READ account WRITE setAccount NOTIFY accountChanged)

    QString account() const;
    void setAccount(const QString& account);

    const QList<int> noteIds();
    bool noteExists(const int id);
    int noteModified(const int id);

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
    Q_INVOKABLE const QString errorMessage(ErrorCodes error) const;

    QJsonObject readNoteFile(const int id, const QStringList& exclude = QStringList());
    bool writeNoteFile(const int id, const QJsonObject& note);
    bool removeNoteFile(const int id);

public slots:
    Q_INVOKABLE bool getAllNotes(const QStringList& exclude = QStringList());
    Q_INVOKABLE bool getNote(const int id, const QStringList& exclude = QStringList());
    Q_INVOKABLE bool createNote(const QJsonObject& note);
    Q_INVOKABLE bool updateNote(const int id, const QJsonObject& note);
    Q_INVOKABLE bool deleteNote(const int id);

signals:
    void accountChanged(const QString& account);
    void allNotesChanged(const QList<int>& ids);
    void noteCreated(const int id, const QJsonObject& note);
    void noteUpdated(const int id, const QJsonObject& note);
    void noteDeleted(const int id);
    void noteError(const ErrorCodes error);

private:
    QDir m_dir;
    const static QString m_suffix;
};

#endif // NOTESSTORE_H
