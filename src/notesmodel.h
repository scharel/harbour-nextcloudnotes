#ifndef NOTESMODEL_H
#define NOTESMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QJsonObject>

struct Note;

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

    Q_PROPERTY(int sortBy READ sortBy WRITE setSortBy NOTIFY sortByChanged)
    int sortBy() const { return m_sortBy; }
    void setSortBy(int sortBy);

    Q_PROPERTY(bool favoritesOnTop READ favoritesOnTop WRITE setFavoritesOnTop NOTIFY favoritesOnTopChanged)
    bool favoritesOnTop() const { return m_favoritesOnTop; }
    void setFavoritesOnTop(bool favoritesOnTop);

    Q_INVOKABLE bool applyJSON(QString json, bool replaceIfArray = true);
    Q_INVOKABLE bool removeNote(int id);

    Q_INVOKABLE void search(QString query);
    Q_INVOKABLE void clearSearch();

    Q_INVOKABLE int indexOf(int id) const;

    enum NoteRoles {
        visible = Qt::UserRole,
        idRole = Qt::UserRole + 1,
        modifiedRole = Qt::UserRole + 2,
        titleRole = Qt::UserRole + 3,
        categoryRole = Qt::UserRole + 4,
        contentRole = Qt::UserRole + 5,
        favoriteRole = Qt::UserRole + 6,
        etagRole = Qt::UserRole + 7,
        errorRole = Qt::UserRole + 8,
        errorMessageRole = Qt::UserRole + 9
    };
    QHash<int, QByteArray> roleNames() const;

    enum SortingCriteria {
        sortByDate,
        sortByCategory,
        sortByTitle,
        noSorting
    };
    QHash<int, QByteArray> sortingNames() const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    //virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    //bool insertRow(int row, const QModelIndex &parent);
    //bool insertRows(int row, int count, const QModelIndex &parent);
    //bool removeRow(int row, const QModelIndex &parent);
    //bool removeRows(int row, int count, const QModelIndex &parent);

protected:

signals:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int> ());
    void sortByChanged(int sortBy);
    void favoritesOnTopChanged(bool favoritesOnTop);

private:
    QList<ModelNote<Note, bool> > m_notes;
    int m_sortBy;
    bool m_favoritesOnTop;
    QString m_searchQuery;

    void sort();
    void update();
    int insertPosition(const Note &n) const;
    bool noteLessThan(const Note &n1, const Note &n2) const;
    /*static bool noteLessThanByDate(const Note &n1, const Note &n2);
    static bool noteLessThanByCategory(const Note &n1, const Note &n2);
    static bool noteLessThanByTitle(const Note &n1, const Note &n2);*/
};

#endif // NOTESMODEL_H
