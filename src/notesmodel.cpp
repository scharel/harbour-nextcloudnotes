#include "notesmodel.h"
#include <algorithm>    // std::sort
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>
#include <QDebug>

NotesModel::NotesModel(QObject *parent) : QAbstractListModel(parent)
{
    m_sortBy = noSorting;
    m_favoritesOnTop = true;
}

NotesModel::~NotesModel() {
    //clear();
}

void NotesModel::setSortBy(QString sortBy) {
    qDebug() << "Sorting by:" << sortBy;
    if (sortBy != m_sortBy && sortingNames().values().contains(sortBy.toLocal8Bit())) {
        m_sortBy = sortBy;
        sort();
        emit sortByChanged(m_sortBy);
    }
}

void NotesModel::setFavoritesOnTop(bool favoritesOnTop) {
    qDebug() << "Favorites on top:" << favoritesOnTop;
    if (favoritesOnTop != m_favoritesOnTop) {
        m_favoritesOnTop = favoritesOnTop;
        sort();
        emit favoritesOnTopChanged(m_favoritesOnTop);
    }
}

void NotesModel::setSearchText(QString searchText) {
    qDebug() << "Searching by:" << searchText;
    if (searchText != m_searchText) {
        m_searchText = searchText;
        emit searchTextChanged(m_searchText);
        if (m_searchText.isEmpty()) {
            m_invisibleIds.clear();
            emit dataChanged(this->index(0), this->index(m_notes.size()));
        }
        else {
            for (int i = 0; i < m_notes.size(); i++) {
                if (Note::searchInNote(m_searchText, m_notes[i])) {
                    //qDebug() << "Note" << m_notes[i].title() << "in search";
                    m_invisibleIds.removeAll(m_notes[i].id());
                }
                else {
                    //qDebug() << "Note" << m_notes[i].title() << "not in search";
                    m_invisibleIds.append(m_notes[i].id());
                }
                emit dataChanged(this->index(i), this->index(i));
            }
        }
    }
}

void NotesModel::search(QString searchText) {
    setSearchText(searchText);
}

void NotesModel::clearSearch() {
    search();
}

bool NotesModel::applyJSON(const QJsonDocument &jdoc) {
    qDebug() << "Applying new JSON input";// << json;
    if (!jdoc.isNull()) {
        if (jdoc.isArray()) {
            qDebug() << "- It's an array...";
            QVector<Note> newNotes;
            QJsonArray jarr = jdoc.array();
            while (!jarr.empty()) {
                //qDebug() << jarr.count() << "JSON Objects to handle...";
                QJsonValue jval = jarr.first();
                if (jval.isObject()) {
                    //qDebug() << "It's an object, all fine...";
                    QJsonObject jobj = jval.toObject();
                    if (!jobj.isEmpty()) {
                        newNotes.append(Note::fromjson(jobj));
                    }
                }
                jarr.pop_front();
            }
            for (int i = 0; i < m_notes.size(); ++i) {
                bool noteToBeRemoved = true;
                for (int j = 0; j < newNotes.size(); ++j) {
                    if (m_notes[i].id() == newNotes[j].id())
                        noteToBeRemoved = false;
                }
                if (noteToBeRemoved) {
                    qDebug() << "-- Removing note " << m_notes[i].title();
                    removeNote(m_notes[i]);
                }
            }
            while (!newNotes.empty()) {
                insertNote(newNotes.first());
                newNotes.pop_front();
            }
            return true;
        }
        else if (jdoc.isObject()) {
            qDebug() << "- It's a single object...";
            insertNote(Note::fromjson(jdoc.object()));
            return true;
        }
        else {
            qDebug() << "Unknown JSON document. This message should never occure!";
        }
    }
    else
    {
        qDebug() << "JSON document is empty!";
    }
    return false;
}

bool NotesModel::applyJSON(const QString &json) {
    QJsonParseError error;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (!jdoc.isNull() && error.error == QJsonParseError::NoError) {
        return applyJSON(jdoc);
    }
    return error.error == QJsonParseError::NoError;
}

int NotesModel::insertNote(const Note &note) {
    int position = indexOf(note.id());
    if (position >= 0) {
        if (note.etag() != m_notes[position].etag()) {
            qDebug() << "-- Existing note " << note.title() << "changed, updating the model.";
            m_notes.replace(position, note);
            emit dataChanged(this->index(position), this->index(position));
        }
        else {
            qDebug() << "-- Existing note " << note.title() << "unchanged, nothing to do.";
        }
    }
    else {
        qDebug() << "-- New note" << note.title() << ", adding it to the model.";
        position = insertPosition(note);
        beginInsertRows(QModelIndex(), position, position);
        m_notes.insert(position, note);
        endInsertRows();
    }
    return position;
}

bool NotesModel::removeNote(const Note &note) {
    int position = m_notes.indexOf(note);
    if (position >= 0 && position < m_notes.size()) {
        beginRemoveRows(QModelIndex(), position, position);
        m_notes.removeAt(position);
        endRemoveRows();
        return true;
    }
    return false;
}

bool NotesModel::removeNote(int id) {
    bool retval = false;
    for (int i = 0; i < m_notes.size(); ++i) {
        if (m_notes[i].id() == id) {
            retval |= removeNote(m_notes[i]);
            if (i > 0) i--;
        }
    }
    return retval;
}

QHash<int, QByteArray> NotesModel::roleNames() const {
    return QHash<int, QByteArray> {
        {NotesModel::IdRole, "id"},
        {NotesModel::ModifiedRole, "modified"},
        {NotesModel::TitleRole, "title"},
        {NotesModel::CategoryRole, "category"},
        {NotesModel::ContentRole, "content"},
        {NotesModel::FavoriteRole, "favorite"},
        {NotesModel::EtagRole, "etag"},
        {NotesModel::ErrorRole, "error"},
        {NotesModel::ErrorMessageRole, "errorMessage"},
        {NotesModel::InSearchRole, "inSearch"}
    };
}

QHash<int, QByteArray> NotesModel::sortingNames() const {
    return QHash<int, QByteArray> {
        {NotesModel::sortByDate, "date"},
        {NotesModel::sortByCategory, "category"},
        {NotesModel::sortByTitle, "title"},
        {NotesModel::noSorting, "none"}
    };
}

QStringList NotesModel::sortingCriteria() const {
    QStringList criteria;
    QHash<int, QByteArray> names = sortingNames();
    for (int i = 0; i <= noSorting; i++)
        criteria << names[i];
    return criteria;
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
    if (parent.isValid()) {
        return 0;
    }
    else {
        return m_notes.size();
    }
}

QVariant NotesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    else if (role == IdRole) return m_notes[index.row()].id();
    else if (role == ModifiedRole) return m_notes[index.row()].modified();
    else if (role == TitleRole) return m_notes[index.row()].title();
    else if (role == CategoryRole) return m_notes[index.row()].category();
    else if (role == ContentRole) return m_notes[index.row()].content();
    else if (role == FavoriteRole) return m_notes[index.row()].favorite();
    else if (role == EtagRole) return m_notes[index.row()].etag();
    else if (role == ErrorRole) return m_notes[index.row()].error();
    else if (role == ErrorMessageRole) return m_notes[index.row()].errorMessage();
    else if (role == InSearchRole) {
        qDebug() << "Invisible:" << m_invisibleIds.contains(m_notes[index.row()].id());
        return !m_invisibleIds.contains(m_notes[index.row()].id());
    }
    return QVariant();
}

QMap<int, QVariant> NotesModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    if (!index.isValid()) return map;
    else {
        for (int role = Qt::UserRole; role < Qt::UserRole + 10; ++role) {
            map.insert(role, data(index, role));
        }
    }
    return map;
}

void NotesModel::sort() {
    qDebug() << "Sorting notes in the model";
    emit layoutAboutToBeChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
    if (m_favoritesOnTop) {
        if (m_sortBy == sortingNames()[sortByDate]) {
            std::sort(m_notes.begin(), m_notes.end(), Note::lessThanByDateFavOnTop);
        }
        else if (m_sortBy == sortingNames()[sortByCategory]) {
            std::sort(m_notes.begin(), m_notes.end(), Note::lessThanByCategoryFavOnTop);
        }
        else if (m_sortBy == sortingNames()[sortByTitle]) {
            std::sort(m_notes.begin(), m_notes.end(), Note::lessThanByTitleFavOnTop);
        }
    }
    else {
        if (m_sortBy == sortingNames()[sortByDate]) {
            std::sort(m_notes.begin(), m_notes.end(), Note::lessThanByDate);
        }
        else if (m_sortBy == sortingNames()[sortByCategory]) {
            std::sort(m_notes.begin(), m_notes.end(), Note::lessThanByCategory);
        }
        else if (m_sortBy == sortingNames()[sortByTitle]) {
            std::sort(m_notes.begin(), m_notes.end(), Note::lessThanByTitle);
        }
    }
    emit layoutChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
}

int NotesModel::indexOf(int id) const {
    for (int i = 0; i < m_notes.size(); i++) {
        if (m_notes[i].id() == id)
            return i;
    }
    return -1;
}

int NotesModel::insertPosition(const Note &n) const {
    int lower = 0;
    int upper = m_notes.size();
    while (lower < upper) {
        int middle = qFloor(lower + (upper-lower) / 2);
        bool result = noteLessThan(n, m_notes[middle]);
        if (result)
            upper = middle;
        else
            lower = middle + 1;
    }
    return lower;
}

bool NotesModel::noteLessThan(const Note &n1, const Note &n2) const {
    if (m_sortBy == sortingNames()[sortByDate]) {
        return m_favoritesOnTop ? Note::lessThanByDateFavOnTop(n1, n2) : Note::lessThanByDate(n1, n2);
    }
    else if (m_sortBy == sortingNames()[sortByCategory]) {
        return m_favoritesOnTop ? Note::lessThanByCategoryFavOnTop(n1, n2) : Note::lessThanByCategory(n1, n2);
    }
    else if (m_sortBy == sortingNames()[sortByTitle]) {
        return m_favoritesOnTop ? Note::lessThanByTitleFavOnTop(n1, n2) : Note::lessThanByTitle(n1, n2);
    }
    else {
        if (m_favoritesOnTop && n1.favorite() != n2.favorite())
            return n1.favorite();
    }
    return true;
}
