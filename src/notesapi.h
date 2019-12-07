#ifndef NOTESAPI_H
#define NOTESAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDir>
#include <QDebug>
#include "notesmodel.h"

class NotesApi : public QObject
{
    Q_OBJECT
public:
    explicit NotesApi(QObject *parent = nullptr);
    virtual ~NotesApi();

    Q_PROPERTY(bool sslVerify READ sslVerify WRITE setSslVerify NOTIFY sslVerifyChanged)
    bool sslVerify() const { return m_request.sslConfiguration().peerVerifyMode() == QSslSocket::VerifyPeer; }
    void setSslVerify(bool verify);

    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    QUrl url() const { return m_url; }
    void setUrl(QUrl url);

    Q_PROPERTY(bool urlValid READ urlValid NOTIFY urlValidChanged)
    bool urlValid() const { return m_url.isValid(); }

    Q_PROPERTY(QString scheme READ scheme WRITE setScheme NOTIFY schemeChanged)
    QString scheme() const { return m_url.scheme(); }
    void setScheme(QString scheme);

    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    QString host() const { return m_url.host(); }
    void setHost(QString host);

    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    int port() const { return m_url.port(); }
    void setPort(int port);

    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    QString username() const { return m_url.userName(); }
    void setUsername(QString username);

    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    QString password() const { return m_url.password(); }
    void setPassword(QString password);

    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    QString path() const { return m_url.path(); }
    void setPath(QString path);

    Q_PROPERTY(QString dataDir READ dataDir WRITE setDataDir NOTIFY dataDirChanged)
    QString dataDir() const { return m_jsonDir.absolutePath(); }
    void setDataDir(QString dataDir);

    Q_PROPERTY(bool networkAccessible READ networkAccessible NOTIFY networkAccessibleChanged)
    bool networkAccessible() const { return m_manager.networkAccessible() == QNetworkAccessManager::Accessible; }

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    bool busy() const;

    Q_INVOKABLE void getAllNotes(QStringList excludeFields = QStringList());
    Q_INVOKABLE void getNote(int noteId, QStringList excludeFields = QStringList());
    Q_INVOKABLE void createNote(QVariantMap fields = QVariantMap());
    Q_INVOKABLE void updateNote(int noteId, QVariantMap fields = QVariantMap());
    Q_INVOKABLE void deleteNote(int noteId);
    Q_INVOKABLE NotesProxyModel* model() const { return mp_modelProxy; }

signals:
    void sslVerifyChanged(bool verify);
    void urlChanged(QUrl url);
    void urlValidChanged(bool valid);
    void schemeChanged(QString scheme);
    void hostChanged(QString host);
    void portChanged(int port);
    void usernameChanged(QString username);
    void passwordChanged(QString password);
    void pathChanged(QString path);
    void dataDirChanged(QString dataDir);
    void networkAccessibleChanged(bool accessible);
    void busyChanged(bool busy);

public slots:

private slots:
    void verifyUrl(QUrl url);
    void requireAuthentication(QNetworkReply * reply, QAuthenticator * authenticator);
    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void replyFinished(QNetworkReply* reply);
    void sslError(QNetworkReply* reply, const QList<QSslError> &errors);

private:
    QUrl m_url;
    QNetworkAccessManager m_manager;
    QNetworkRequest m_request;
    QVector<QNetworkReply*> m_replies;
    QDir m_jsonDir;
    NotesModel* mp_model;
    NotesProxyModel* mp_modelProxy;
};

#endif // NOTESAPI_H
