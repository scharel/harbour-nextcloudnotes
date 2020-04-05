#ifndef NOTE_H
#define NOTE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>

#define ID "id"
#define MODIFIED "modified"
#define TITLE "title"
#define CATEGORY "category"
#define CONTENT "content"
#define FAVORITE "favorite"
#define ETAG "etag"
#define ERROR "error"
#define ERRORMESSAGE "errorMessage"

class Note : public QObject {
    Q_OBJECT

public:
    Note(QObject *parent = NULL);
    Note(const Note& note, QObject *parent = NULL);
    Note(const QJsonObject& note, QObject *parent = NULL);
    ~Note();

    Note& operator =(const Note& note);
    Note& operator =(const QJsonObject& note);
    bool operator ==(const Note& note) const;
    bool operator ==(const QJsonObject& note) const;
    bool equal(const Note& note) const;
    bool equal(const QJsonObject& note) const;
    bool operator <(const Note& note) const;
    bool operator <(const QJsonObject& note) const;
    bool operator >(const Note& note) const;
    bool operator >(const QJsonObject& note) const;

    QJsonObject toJsonObject() const;
    QJsonValue toJsonValue() const;
    QJsonDocument toJsonDocument() const;

    Q_PROPERTY(int id READ id  WRITE setId  NOTIFY idChanged)
    Q_INVOKABLE int id() const;
    void setId(int id);

    Q_PROPERTY(int modified READ modified WRITE setModified NOTIFY modifiedChanged)
    Q_INVOKABLE int modified() const;
    void setModified(int modified);

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_INVOKABLE QString title() const;
    void setTitle(QString title);

    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    Q_INVOKABLE QString category() const;
    void setCategory(QString category);

    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    Q_INVOKABLE QString content() const;
    void setContent(QString content);

    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    Q_INVOKABLE bool favorite() const;
    void setFavorite(bool favorite);

    Q_PROPERTY(QString etag READ etag WRITE setEtag NOTIFY etagChanged)
    Q_INVOKABLE QString etag() const;
    void setEtag(QString etag);

    Q_PROPERTY(bool error READ error WRITE setError NOTIFY errorChanged)
    Q_INVOKABLE bool error() const;
    void setError(bool error);

    Q_PROPERTY(QString errorMessage READ errorMessage WRITE setErrorMessage NOTIFY errorMessageChanged)
    Q_INVOKABLE QString errorMessage() const;
    void setErrorMessage(QString errorMessage);

    Q_PROPERTY(QString modifiedString READ modifiedString NOTIFY modifiedStringChanged)
    Q_INVOKABLE QString modifiedString() const;

    Q_INVOKABLE QDateTime modifiedDateTime() const;

    static int id(const QJsonObject& jobj);
    static int modified(const QJsonObject& jobj);
    static QString title(const QJsonObject& jobj);
    static QString category(const QJsonObject& jobj);
    static QString content(const QJsonObject& jobj);
    static bool favorite(const QJsonObject& jobj);
    static QString etag(const QJsonObject& jobj);
    static bool error(const QJsonObject& jobj);
    static QString errorMessage(const QJsonObject& jobj);
    static QString modifiedString(const QJsonObject& jobj);
    static QDateTime modifiedDateTime(const QJsonObject& jobj);

signals:
    void idChanged(int id);
    void modifiedChanged(int modified);
    void titleChanged(QString title);
    void categoryChanged(QString category);
    void contentChanged(QString content);
    void favoriteChanged(bool favorite);
    void etagChanged(QString etag);
    void errorChanged(bool error);
    void errorMessageChanged(QString errorMessage);
    void modifiedStringChanged(QString date);
    void modifiedDateTimeChanged(QDateTime dateTime);
    void noteChanged();

private:
    QJsonObject m_json;

    void connectSignals();
};

Q_DECLARE_METATYPE(Note)

#endif // NOTE_H
