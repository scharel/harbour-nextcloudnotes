#ifndef NOTESAPP_H
#define NOTESAPP_H

#include <QQmlEngine>
#include <QJSEngine>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QVersionNumber>
#include "../abstractnextcloudapp.h"
#include "notesmodel.h"

const int NOTES_API_VERSION(1);
const QString NOTES_APP_ENDPOINT(QString("/index.php/apps/notes/api/v%1").arg(NOTES_API_VERSION));
const QString CAtERGORY_QUERY("category");
const QString EXCLUDE_QUERY("exclude");
const QString PURGE_QUERY("purgeBefore");
const QString ETAG_HEADER("If-None-Match");

class NotesApp : public AbstractNextcloudApp {
    Q_OBJECT

    Q_PROPERTY(QVersionNumber serverVersion READ serverVersion NOTIFY capabilitiesChanged)
    Q_PROPERTY(QList<QVersionNumber> apiVersions READ apiVersions NOTIFY capabilitiesChanged)

public:
    NotesApp(QObject *parent = nullptr, NextcloudApi* api = nullptr);
    virtual ~NotesApp() {}

    // QML singleton
    static void instantiate(QObject *parent = nullptr, NextcloudApi* api = nullptr);
    static AbstractNextcloudApp & getInstance();
    static QObject * provider(QQmlEngine *, QJSEngine *);

    // Notes model
    Q_INVOKABLE NotesProxyModel* model() { return &m_notesProxy; }

    // Properties
    Q_INVOKABLE QVersionNumber serverVersion() const;
    Q_INVOKABLE QList<QVersionNumber> apiVersions() const;

public slots:
    Q_INVOKABLE bool getAllNotes(const QStringList& exclude = QStringList());
    Q_INVOKABLE bool getNote(const int id);
    Q_INVOKABLE bool createNote(const QJsonObject& note, bool local = false);
    Q_INVOKABLE bool updateNote(const int id, const QJsonObject& note, bool local = false);
    Q_INVOKABLE bool deleteNote(const int id, bool local = false);
    //Q_INVOKABLE bool getSettings();
    //Q_INVOKABLE bool changeSettings(const QJsonObject& settings);
    void updateUrl(QUrl* url);

protected slots:
    void updateCapabilities(QJsonObject*) { }
    void updateReply(QNetworkReply* reply);

signals:
    void capabilitiesChanged(QJsonObject* json);

private:
    // QML singleton
    static AbstractNextcloudApp * instance;

    NotesModel m_notesModel;
    NotesProxyModel m_notesProxy;
};

#endif // NOTESAPP_H
