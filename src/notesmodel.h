#ifndef NOTESMODEL_H
#define NOTESMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include "note.h"

class NotesModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit NotesModel(QObject *parent = 0);
    virtual ~NotesModel();

    Q_PROPERTY(QString sortBy READ sortBy WRITE setSortBy NOTIFY sortByChanged)
    QString sortBy() const { return m_sortBy; }
    void setSortBy(QString sortBy);

    Q_PROPERTY(bool favoritesOnTop READ favoritesOnTop WRITE setFavoritesOnTop NOTIFY favoritesOnTopChanged)
    bool favoritesOnTop() const { return m_favoritesOnTop; }
    void setFavoritesOnTop(bool favoritesOnTop);

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    QString searchText() const { return m_searchText; }
    void setSearchText(QString searchText);

    Q_INVOKABLE void search(QString searchText = QString());
    Q_INVOKABLE void clearSearch();

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
        ErrorMessageRole = Qt::UserRole + 8,
        InSearchRole = Qt::UserRole + 9
    };
    QHash<int, QByteArray> roleNames() const;

    enum SortingCriteria {
        sortByDate,
        sortByCategory,
        sortByTitle,
        noSorting
    };
    QHash<int, QByteArray> sortingNames() const;
    Q_INVOKABLE QStringList sortingCriteria() const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    QMap<int, QVariant> itemData(const QModelIndex &index) const;

protected:

signals:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int> ());
    void sortByChanged(QString sortBy);
    void favoritesOnTopChanged(bool favoritesOnTop);
    void searchTextChanged(QString searchText);

private:
    QVector<Note> m_notes;
    QVector<int> m_invisibleIds;
    QString m_sortBy;
    bool m_favoritesOnTop;
    QString m_searchText;

    void sort();
    //void update();
    int insertNote(const Note &note);
    bool replaceNote(const Note &note);
    bool removeNote(const Note &note);
    bool removeNote(int id);
    int indexOf(int id) const;
    int insertPosition(const Note &n) const;
    bool noteLessThan(const Note &n1, const Note &n2) const;
};

#endif // NOTESMODEL_H
