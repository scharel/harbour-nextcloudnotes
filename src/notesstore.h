#ifndef NOTESSTORE_H
#define NOTESSTORE_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QStandardPaths>

class NotesStore : public QObject
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
    Q_INVOKABLE const QString errorMessage(ErrorCodes error) const;

public slots:
    Q_INVOKABLE const QJsonArray getAllNotes(const QStringList& exclude = QStringList());
    Q_INVOKABLE const QJsonObject getNote(const int id, const QStringList& exclude = QStringList());
    Q_INVOKABLE bool createNote(const QJsonObject& note);
    Q_INVOKABLE const QJsonObject updateNote(const int id, const QJsonObject& note);
    Q_INVOKABLE bool deleteNote(const int id);

signals:
    void accountChanged(const QString& account);
    void noteCreated(const int id, const QJsonObject& note);
    void noteUpdated(const int id, const QJsonObject& note);
    void noteDeleted(const int id);
    void noteError(const ErrorCodes error);

private:
    QDir m_dir;
    const static QString m_suffix;

    bool noteFileExists(const int id) const;
    QJsonObject readNoteFile(const int id, const QStringList& exclude = QStringList());
    bool writeNoteFile(const int id, const QJsonObject& note);
    bool removeNoteFile(const int id);
};

#endif // NOTESSTORE_H