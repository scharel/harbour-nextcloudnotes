    #ifndef NOTE_H
#define NOTE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QMap>
#include <QDebug>

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

    bool isValid() const { return id() >= 0; }

    const QJsonObject toJsonObject() const;
    const QJsonValue toJsonValue() const;
    const QJsonDocument toJsonDocument() const;

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
    Q_INVOKABLE static QList<NoteField> noteFields() {
        return m_noteFieldNames.keys();
    }
    Q_INVOKABLE static QString noteFieldName(NoteField field) {
        return m_noteFieldNames[field];
    }
    Q_INVOKABLE static QStringList noteFieldNames() {
        return m_noteFieldNames.values();
    }
    Q_INVOKABLE static NoteField noteFieldsFromStringList(QStringList fields);
    Q_INVOKABLE static QStringList noteFieldsToStringList(Note::NoteField fields);

    Q_PROPERTY(int id READ id  WRITE setId  NOTIFY idChanged)
    int id() const;
    void setId(int id);

    Q_PROPERTY(int modified READ modified WRITE setModified NOTIFY modifiedChanged)
    int modified() const;
    void setModified(int modified);

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

    Q_PROPERTY(QDateTime modifiedDateTime READ modifiedDateTime NOTIFY modifiedDateTimeChanged)
    QDateTime modifiedDateTime() const;

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
    const static QMap<NoteField, QString> m_noteFieldNames;

    void connectSignals();
};

Q_DECLARE_METATYPE(Note)

#endif // NOTE_H
