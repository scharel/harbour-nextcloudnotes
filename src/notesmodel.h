#ifndef NOTESMODEL_H
#define NOTESMODEL_H

#include <QSortFilterProxyModel>
#include <QAbstractListModel>
#include <QDateTime>
#include "note.h"

class NotesProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit NotesProxyModel(QObject *parent = 0);
    virtual ~NotesProxyModel();

    Q_PROPERTY(bool favoritesOnTop READ favoritesOnTop WRITE setFavoritesOnTop NOTIFY favoritesOnTopChanged)
    bool favoritesOnTop() const { return m_favoritesOnTop; }
    void setFavoritesOnTop(bool favoritesOnTop);

    enum SortingCriteria {
        ModifiedRole,
        CategoryRole,
        TitleRole,
        noSorting = Qt::UserRole + 9
    };
    QHash<int, QByteArray> sortingNames() const;
    Q_INVOKABLE QList<int> sortingRoles() const;
    Q_INVOKABLE int sortingRole(const QString &name) const;

protected:
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

private slots:
    void resort();

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

    bool applyJSON(const QJsonDocument &jdoc);
    bool applyJSON(const QString &json);

    enum NoteRoles {
        IdRole = Qt::UserRole,
        ModifiedRole = Qt::UserRole + 1,
        TitleRole = Qt::UserRole + 2,
        CategoryRole = Qt::UserRole + 3,
        ContentRole = Qt::UserRole + 4,
        FavoriteRole = Qt::UserRole + 5,
        EtagRole = Qt::UserRole + 6,
        ErrorRole = Qt::UserRole + 7,
        ErrorMessageRole = Qt::UserRole + 8
    };
    QHash<int, QByteArray> roleNames() const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    QMap<int, QVariant> itemData(const QModelIndex &index) const;

protected:
    int indexOf(const Note &note) const;
    int indexOf(int id) const;
    int insertNote(const Note &note);
    bool replaceNote(const Note &note);
    bool removeNote(const Note &note);
    bool removeNote(int id);

signals:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int> ());

private:
    QVector<Note> m_notes;
};

#endif // NOTESMODEL_H
