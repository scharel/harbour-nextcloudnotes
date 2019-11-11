#ifndef NOTE_H
#define NOTE_H

#include <QObject>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

class Note : public QObject {
    Q_OBJECT

public:
    Note(QObject *parent = NULL);
    Note(const Note& note, QObject *parent = NULL);

    Note& operator=(const Note& note);
    bool operator==(const Note& note) const;
    bool same(const Note& note) const;

    Q_PROPERTY(int id READ id  WRITE setId  NOTIFY idChanged)
    int id() const { return m_id; }
    void setId(int id) { if (id != m_id) { m_id = id; emit idChanged(id); } }

    Q_PROPERTY(uint modified READ modified WRITE setModified NOTIFY modifiedChanged)
    uint modified() const { return m_modified; }
    void setModified(uint modified) { if (modified != m_modified) { m_modified = modified; emit modifiedChanged(modified); emit dateStringChanged(dateString()); } }

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    QString title() const { return m_title; }
    void setTitle(QString title) { if (title != m_title) { m_title = title; emit titleChanged(title); } }

    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    QString category() const { return m_category; }
    void setCategory(QString category) { if (category != m_category) { m_category = category; emit categoryChanged(category); } }

    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    QString content() const { return m_content; }
    void setContent(QString content) { if (content != m_content) { m_content = content; emit contentChanged(content); } }

    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    bool favorite() const { return m_favorite; }
    void setFavorite(bool favorite) { if (favorite != m_favorite) { m_favorite = favorite; emit favoriteChanged(favorite); } }

    Q_PROPERTY(QString etag READ etag WRITE setEtag NOTIFY etagChanged)
    QString etag() const { return m_etag; }
    void setEtag(QString etag) { if (etag != m_etag) { m_etag = etag; emit etagChanged(etag); } }

    Q_PROPERTY(bool error READ error WRITE setError NOTIFY errorChanged)
    bool error() const { return m_error; }
    void setError(bool error) { if (error != m_error) { m_error = error; emit errorChanged(error); } }

    Q_PROPERTY(QString errorMessage READ errorMessage WRITE setErrorMessage NOTIFY errorMessageChanged)
    QString errorMessage() const { return m_errorMessage; }
    void setErrorMessage(QString errorMessage) { if (errorMessage != m_errorMessage) { m_errorMessage = errorMessage; emit errorMessageChanged(errorMessage); } }

    Q_PROPERTY(QString dateString READ dateString NOTIFY dateStringChanged)
    QString dateString() const;
    static Note fromjson(const QJsonObject& jobj);

signals:
    void idChanged(int id);
    void modifiedChanged(uint modified);
    void titleChanged(QString title);
    void categoryChanged(QString category);
    void contentChanged(QString content);
    void favoriteChanged(bool favorite);
    void etagChanged(QString etag);
    void errorChanged(bool error);
    void errorMessageChanged(QString errorMessage);
    void dateStringChanged(QString date);
    void noteChanged();

private:
    int m_id;
    uint m_modified;
    QString m_title;
    QString m_category;
    QString m_content;
    bool m_favorite;
    QString m_etag;
    bool m_error;
    QString m_errorMessage;
};

#endif // NOTE_H
