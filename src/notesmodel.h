#ifndef NOTESMODEL_H
#define NOTESMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include "note.h"

template <typename N, typename P>
struct ModelNote {
    N note;
    P param;
};

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

    Q_INVOKABLE bool applyJSON(const QString &json);
    Q_INVOKABLE int insertNote(const Note &note);
    Q_INVOKABLE bool removeNote(const Note &note);
    Q_INVOKABLE bool removeNote(int position);
    Q_INVOKABLE bool replaceNote(const Note &note);
    Q_INVOKABLE void clear();

    Q_INVOKABLE int indexOf(int id) const;
    Q_INVOKABLE Note get(int index) const;

    enum NoteRoles {
        VisibleRole = Qt::UserRole,
        IdRole = Qt::UserRole + 1,
        ModifiedRole = Qt::UserRole + 2,
        TitleRole = Qt::UserRole + 3,
        CategoryRole = Qt::UserRole + 4,
        ContentRole = Qt::UserRole + 5,
        FavoriteRole = Qt::UserRole + 6,
        EtagRole = Qt::UserRole + 7,
        ErrorRole = Qt::UserRole + 8,
        ErrorMessageRole = Qt::UserRole + 9,
        DateStringRole = Qt::UserRole + 10
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
    //virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);

    //bool insertRow(int row, const QModelIndex &parent);
    //bool insertRows(int row, int count, const QModelIndex &parent);
    //bool removeRow(int row, const QModelIndex &parent);
    //bool removeRows(int row, int count, const QModelIndex &parent);

protected:

signals:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int> ());
    void sortByChanged(QString sortBy);
    void favoritesOnTopChanged(bool favoritesOnTop);
    void searchTextChanged(QString searchText);

private:
    QList<ModelNote<Note, bool> > m_notes;
    QString m_sortBy;
    bool m_favoritesOnTop;
    QString m_searchText;

    void sort();
    //void update();
    bool applyJSONobject(const QJsonObject &jobj);
    int insertPosition(const Note &n) const;
    bool noteLessThan(const Note &n1, const Note &n2) const;
    /*static bool noteLessThanByDate(const Note &n1, const Note &n2);
    static bool noteLessThanByCategory(const Note &n1, const Note &n2);
    static bool noteLessThanByTitle(const Note &n1, const Note &n2);*/
};

#endif // NOTESMODEL_H
