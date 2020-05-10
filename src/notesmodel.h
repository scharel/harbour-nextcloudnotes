#ifndef NOTESMODEL_H
#define NOTESMODEL_H

#include <QSortFilterProxyModel>
#include <QAbstractListModel>
#include <QFile>
#include <QJsonArray>
#include <QDateTime>
#include "note.h"
#include "notesapi.h"
#include "notesstore.h"

class NotesProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(bool favoritesOnTop READ favoritesOnTop WRITE setFavoritesOnTop NOTIFY favoritesOnTopChanged)
    Q_PROPERTY(QString sortBy READ sortBy WRITE setSortBy NOTIFY sortByChanged)
    Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)

public:
    explicit NotesProxyModel(QObject *parent = nullptr);
    virtual ~NotesProxyModel();

    bool favoritesOnTop() const { return m_favoritesOnTop; }
    void setFavoritesOnTop(bool favoritesOnTop);
    QString sortBy() const { return roleNames().value(m_sortByRole); }
    void setSortBy(const QString sortBy);
    QString searchFilter() const { return m_searchFilterString; }
    void setSearchFilter(const QString searchFilter);

    //Q_INVOKABLE void sort();

protected:
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

signals:
    void favoritesOnTopChanged(bool favoritesOnTop);
    void sortByChanged(QString sortBy);
    void searchFilterChanged(QString searchFilter);

private:
    bool m_favoritesOnTop;
    int m_sortByRole;
    QString m_searchFilterString;
};

class NotesModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit NotesModel(QObject *parent = nullptr);
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
    Q_INVOKABLE int roleFromName(const QString &roleName) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    QMap<int, QVariant> itemData(const QModelIndex &index);
    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);

    void setNotesApi(NotesApi* notesApi);
    void setNotesStore(NotesStore *notesStore);

    QString account() const;
    void setAccount(const QString& account);

    Q_INVOKABLE const QVariantMap note(const int id) const;
    Q_INVOKABLE bool setNote(const QVariantMap& note, int id = -1) const;

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

    QMap<int, QFile> m_files;
    QDir m_fileDir;
    const static QString m_fileSuffix;

    NotesApi* mp_notesApi;
    NotesStore* mp_notesStore;
};

#endif // NOTESMODEL_H
