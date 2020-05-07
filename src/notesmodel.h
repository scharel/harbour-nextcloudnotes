#ifndef NOTESMODEL_H
#define NOTESMODEL_H

#include <QSortFilterProxyModel>
#include <QAbstractListModel>
#include <QJsonArray>
#include <QDateTime>
#include "note.h"
#include "notesapi.h"
#include "notesstore.h"

class NotesProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(bool favoritesOnTop READ favoritesOnTop WRITE setFavoritesOnTop NOTIFY favoritesOnTopChanged)

public:
    explicit NotesProxyModel(QObject *parent = 0);
    virtual ~NotesProxyModel();

    bool favoritesOnTop() const { return m_favoritesOnTop; }
    void setFavoritesOnTop(bool favoritesOnTop);

    Q_INVOKABLE void sort();
    Q_INVOKABLE int roleFromName(const QString &name) const;

protected:
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

private slots:

signals:
    void favoritesOnTopChanged(bool favoritesOnTop);

private:
    bool m_favoritesOnTop;
};

class NotesModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit NotesModel(QObject *parent = 0);
    virtual ~NotesModel();

    enum NoteRoles {
        IdRole = Qt::UserRole,
        ModifiedRole = Qt::UserRole + 1,
        TitleRole = Qt::UserRole + 2,
        CategoryRole = Qt::UserRole + 3,
        ContentRole = Qt::UserRole + 4,
        FavoriteRole = Qt::UserRole + 5,
        EtagRole = Qt::UserRole + 6,
        ErrorRole = Qt::UserRole + 7,
        ErrorMessageRole = Qt::UserRole + 8,
        ModifiedStringRole = Qt::UserRole + 9,
        NoneRole = Qt::UserRole + 10
    };
    QHash<int, QByteArray> roleNames() const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);

    void setNotesApi(NotesApi* notesApi);
    void setNotesStore(NotesStore *notesStore);

    QString account() const;
    void setAccount(const QString& account);

    const QList<int> noteIds();
    bool noteExists(const int id);
    int noteModified(const int id);

    Q_INVOKABLE const QVariantMap getNoteById(const int id) const;

public slots:
    Q_INVOKABLE bool getAllNotes(const QStringList& exclude = QStringList());
    Q_INVOKABLE bool getNote(const int id, const QStringList& exclude = QStringList());
    Q_INVOKABLE bool createNote(const QJsonObject& note);
    Q_INVOKABLE bool updateNote(const int id, const QJsonObject& note);
    Q_INVOKABLE bool deleteNote(const int id);
    Q_INVOKABLE bool syncNotes();

    void insert(const int id, const QJsonObject& note);
    void update(const int id, const QJsonObject& note);
    void remove(const int id);

    Q_INVOKABLE void clear();
    Q_INVOKABLE int indexOfNoteById(int id) const;

signals:
    void accountChanged(const QString& account);
    void allNotesChanged(const QList<int>& ids);
    void noteCreated(const int id, const QJsonObject& note);
    void noteUpdated(const int id, const QJsonObject& note);
    void noteDeleted(const int id);

    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int> ());

private:
    QMap<int, QJsonObject> m_notes;
    const static QHash<int, QByteArray> m_roleNames;

    NotesApi* mp_notesApi;
    NotesStore* mp_notesStore;
};

#endif // NOTESMODEL_H
