#include "notesmodel.h"
//#include <algorithm>    // std::sort
#include <QJsonDocument>
#include <QJsonObject>
#include <QtMath>
#include <QDebug>

NotesProxyModel::NotesProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {
    m_favoritesOnTop = true;
    //connect(this, SIGNAL(favoritesOnTopChanged(bool)), this, SLOT(resort()));
}

NotesProxyModel::~NotesProxyModel() {

}

void NotesProxyModel::setFavoritesOnTop(bool favoritesOnTop) {
    qDebug() << "Favorites on top:" << favoritesOnTop;
    if (favoritesOnTop != m_favoritesOnTop) {
        m_favoritesOnTop = favoritesOnTop;
        emit favoritesOnTopChanged(m_favoritesOnTop);
    }
    sort();
}

int NotesProxyModel::roleFromName(const QString &name) const {
    return roleNames().key(name.toLocal8Bit());
}

const QVariantMap NotesProxyModel::getNote(const QModelIndex &index) const {
    QMap<int, QVariant> item = sourceModel()->itemData(mapToSource(index));
    QHash<int, QByteArray> names = roleNames();
    QVariantMap note;
    QMapIterator<int, QVariant> i(item);
    while (i.hasNext()) {
        i.next();
        note[names.value(i.key())] = i.value();
    }
    return note;
}

bool NotesProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const {
    QAbstractItemModel* source = sourceModel();
    if (m_favoritesOnTop && source->data(source_left, NotesModel::FavoriteRole).toBool() != source->data(source_right, NotesModel::FavoriteRole).toBool())
        return source->data(source_left, NotesModel::FavoriteRole).toBool();
    else if (sortRole() == NotesModel::ModifiedStringRole)
        return source->data(source_left, NotesModel::ModifiedRole).toInt() >= source->data(source_right, NotesModel::ModifiedRole).toInt();
    else
        return QSortFilterProxyModel::lessThan(source_left, source_right);
}

void NotesProxyModel::sort() {
    //invalidate();
    QSortFilterProxyModel::sort(0);
}

const QHash<int, QByteArray> NotesModel::m_roleNames = QHash<int, QByteArray> ( {
    {NotesModel::IdRole, "id"},
    {NotesModel::ModifiedRole, "modified"},
    {NotesModel::TitleRole, "title"},
    {NotesModel::CategoryRole, "category"},
    {NotesModel::ContentRole, "content"},
    {NotesModel::FavoriteRole, "favorite"},
    {NotesModel::EtagRole, "etag"},
    {NotesModel::ErrorRole, "error"},
    {NotesModel::ErrorMessageRole, "errorMessage"},
    {NotesModel::ModifiedStringRole, "modifiedString"},
    {NotesModel::NoneRole, "none"} } );

NotesModel::NotesModel(QObject *parent) : QAbstractListModel(parent) {

}

NotesModel::~NotesModel() {
    clear();
}

const QJsonArray NotesModel::getAllNotes(const QStringList &exclude) {
    qDebug() << "Getting all Notes";
    QJsonArray array;
    QMapIterator<int, QJsonObject> i(m_notes);
    while (i.hasNext()) {
        i.next();
        array << QJsonValue(i.value());
    }
    return array;
}

const QJsonObject NotesModel::getNote(const int id, const QStringList &exclude) {
    qDebug() << "Getting note: " << id;
    return m_notes.value(id);
}

void NotesModel::insertNote(const int id, const QJsonObject& note) {
    qDebug() << "Inserting note: " << id;
    if (m_notes.contains(id)) {
        qDebug() << "Note already present";
        updateNote(id, note);
    }
    else {
        beginInsertRows(QModelIndex(), indexOfNoteById(id), indexOfNoteById(id));
        m_notes.insert(id, note);
        endInsertRows();
        emit noteInserted(id, note);
        qDebug() << "Note inserted";
    }
}

void NotesModel::updateNote(const int id, const QJsonObject &note) {
    qDebug() << "Updating note: " << id;
    if (!m_notes.contains(id)) {
        qDebug() << "Note is new";
        insertNote(id, note);
    }
    else {
        if (m_notes.value(id) == note) {
            qDebug() << "Note unchanged";
        }
        else {
            m_notes.insert(id, note);
            emit dataChanged(index(indexOfNoteById(id)), index(indexOfNoteById(id)));
            emit noteUpdated(id, note);
            qDebug() << "Note changed";
        }
    }
}

void NotesModel::removeNote(const int id) {
    qDebug() << "Removing note: " << id;
    if (m_notes.contains(id)) {
        beginRemoveRows(QModelIndex(), indexOfNoteById(id), indexOfNoteById(id));
        if (m_notes.remove(id) == 0) {
            qDebug() << "Note not found";
        }
        else {
            emit noteRemoved(id);
        }
        endRemoveRows();
    }
}

void NotesModel::insertNoteFromApi(const int id, const QJsonObject &note) {
    insertNote(id, note);
}

void NotesModel::updateNoteFromApi(const int id, const QJsonObject &note) {
    updateNote(id, note);
}

void NotesModel::removeNoteFromApi(const int id) {
    removeNote(id);
}

void NotesModel::insertNoteFromStore(const int id, const QJsonObject &note) {
    insertNote(id, note);
}

void NotesModel::updateNoteFromStore(const int id, const QJsonObject &note) {
    updateNote(id, note);
}

void NotesModel::removeNoteFromStore(const int id) {
    removeNote(id);
}

void NotesModel::clear() {
    qDebug() << "Clearing model";
    beginResetModel();
    m_notes.clear();
    endResetModel();
}

int NotesModel::indexOfNoteById(int id) const  {
    return std::distance(m_notes.begin(), m_notes.lowerBound(id));
}

QHash<int, QByteArray> NotesModel::roleNames() const {
    return m_roleNames;
}

Qt::ItemFlags NotesModel::flags(const QModelIndex &index) const {
    if (index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable; // | Qt::ItemIsSelectable
    }
    else {
        return Qt::NoItemFlags;
    }
}

int NotesModel::rowCount(const QModelIndex &parent) const {
    return m_notes.size();
}

QVariant NotesModel::data(const QModelIndex &index, int role) const {
    QVariant data;
    if (index.isValid() && index.row() <= m_notes.size()) {
        QMap<int, QJsonObject>::const_iterator i = m_notes.cbegin();
        i += index.row();
        data = i.value()[(m_roleNames[role])];
    }
    return data;
}

bool NotesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && index.row() <= m_notes.size()) {
        QMap<int, QJsonObject>::iterator i = m_notes.begin();
        i += index.row();
        i.value()[m_roleNames[role]] = QJsonValue::fromVariant(value);
        emit dataChanged(index, index, QVector<int>( role ));
        return true;
    }
    return false;
}

QMap<int, QVariant> NotesModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    if (index.isValid() && index.row() <= m_notes.size()) {
        for (int role = IdRole; role < NoneRole; ++role) {
            map.insert(role, data(index, role));
        }
    }
    return map;
}

bool NotesModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) {
    bool retval = true;
    QMapIterator<int, QVariant> role(roles);
    while (role.hasNext()) {
        role.next();
        retval &= setData(index, role.value(), role.key());
    }
    return retval;
}
