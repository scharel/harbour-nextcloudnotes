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

    Q_PROPERTY(double id READ id  WRITE setId  NOTIFY idChanged)
    double id() const;
    void setId(double id);

    Q_PROPERTY(double modified READ modified WRITE setModified NOTIFY modifiedChanged)
    double modified() const;
    void setModified(double modified);

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    QString title() const;
    void setTitle(QString title);

    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    QString category() const;
    void setCategory(QString category);

    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    QString content() const;
    void setContent(QString content);

    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    bool favorite() const;
    void setFavorite(bool favorite);

    Q_PROPERTY(QString etag READ etag WRITE setEtag NOTIFY etagChanged)
    QString etag() const;
    void setEtag(QString etag);

    Q_PROPERTY(bool error READ error WRITE setError NOTIFY errorChanged)
    bool error() const;
    void setError(bool error);

    Q_PROPERTY(QString errorMessage READ errorMessage WRITE setErrorMessage NOTIFY errorMessageChanged)
    QString errorMessage() const;
    void setErrorMessage(QString errorMessage);

    Q_PROPERTY(QString modifiedString READ modifiedString NOTIFY modifiedStringChanged)
    QString modifiedString() const;

    QDateTime modifiedDateTime() const;

    static double id(const QJsonObject& jobj);
    static double modified(const QJsonObject& jobj);
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
    void idChanged(double id);
    void modifiedChanged(double modified);
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

#endif // NOTE_H
