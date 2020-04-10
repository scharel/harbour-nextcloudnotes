#ifndef NOTESMODEL_H
#define NOTESMODEL_H

#include <QSortFilterProxyModel>
#include <QAbstractListModel>
#include <QJsonArray>
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

    bool fromJsonDocument(const QJsonDocument &jdoc);
    QJsonDocument toJsonDocument() const;

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

public slots:
    int insertNote(const Note &note);
    bool removeNote(const Note &note);
    bool removeNote(int id);


protected:
    //void addNote(const QJsonValue &note);
    QVector<int> ids() const;
    //int indexOf(const Note &note) const;
    //int indexOf(int id) const;
    //bool replaceNote(const Note &note);

signals:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int> ());

private:
    QVector<Note> m_notes;
    //QJsonArray m_notes;
};

#endif // NOTESMODEL_H
