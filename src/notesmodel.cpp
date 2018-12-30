#include "notesmodel.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QtMath>

const QHash<int, QByteArray> noteRoles = QHash<int, QByteArray>{
    {NotesModel::visible, "visible"},
    {NotesModel::idRole, "id"},
    {NotesModel::modifiedRole, "modified"},
    {NotesModel::titleRole, "title"},
    {NotesModel::categoryRole, "category"},
    {NotesModel::contentRole, "content"},
    {NotesModel::favoriteRole, "favorite"},
    {NotesModel::etagRole, "etag"},
    {NotesModel::errorRole, "error"},
    {NotesModel::errorMessageRole, "errorMessage"}
};

struct Note {
    int id;
    uint modified;
    QString title;
    QString category;
    QString content;
    bool favorite;
    QString etag;
    bool error;
    QString errorMessage;
    bool operator==(const Note& n) {
        return id == n.id;
    }
    enum SearchAttribute {
        NoSearchAttribute = 0x0,
        SearchInTitle = 0x1,
        SearchInCategory = 0x2,
        SearchInContent = 0x4,
        SearchAll = 0x7
    };
    Q_DECLARE_FLAGS(SearchAttributes, SearchAttribute)
    static const Note fromjson(const QJsonObject& jobj) {
        Note note;
        note.id = jobj.value(noteRoles[NotesModel::idRole]).toInt();
        note.modified = jobj.value(noteRoles[NotesModel::modifiedRole]).toInt();
        note.title = jobj.value(noteRoles[NotesModel::titleRole]).toString();
        note.category = jobj.value(noteRoles[NotesModel::categoryRole]).toString();
        note.content = jobj.value(noteRoles[NotesModel::contentRole]).toString();
        note.favorite = jobj.value(noteRoles[NotesModel::favoriteRole]).toBool();
        note.etag = jobj.value(noteRoles[NotesModel::etagRole]).toString();
        note.error = jobj.value(noteRoles[NotesModel::errorRole]).toBool(true);
        note.errorMessage = jobj.value(noteRoles[NotesModel::errorMessageRole]).toString();
        return note;
    }
    static bool searchInNote(const QString &query, const Note &note, SearchAttributes criteria = QFlag(SearchAll)) {
        bool queryFound = false;
        if (criteria.testFlag(SearchInTitle)) {
            queryFound |= note.title.contains(query, Qt::CaseInsensitive);
        }
        if (criteria.testFlag(SearchInContent)) {
            queryFound |= note.content.contains(query, Qt::CaseInsensitive);
        }
        if (criteria.testFlag(SearchInCategory)) {
            queryFound |= note.category.contains(query, Qt::CaseInsensitive);
        }
        return queryFound;
    }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Note::SearchAttributes)

NotesModel::NotesModel(QObject *parent) : QAbstractListModel(parent)
{
    m_sortBy = sortByDate;
    m_favoritesOnTop = true;
}

NotesModel::~NotesModel() {
    m_notes.clear();
}

void NotesModel::setSortBy(int sortBy) {
    if (sortBy != m_sortBy) {
        m_sortBy = sortBy;
        sort();
        emit sortByChanged(m_sortBy);
    }
}

void NotesModel::setFavoritesOnTop(bool favoritesOnTop) {
    if (favoritesOnTop != m_favoritesOnTop) {
        m_favoritesOnTop = favoritesOnTop;
        sort();
        emit favoritesOnTopChanged(m_favoritesOnTop);
    }
}

bool NotesModel::applyJSON(QString json, bool replaceIfArray) {
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toUtf8());
    int notesModified = 0;
    if (!jdoc.isNull()) {
        if (jdoc.isArray()) {
            QJsonArray jarr = jdoc.array();
            QList<int> notesToRemove;
            QList<ModelNote<Note, int> > notesToAdd;
            for (int i = 0; i < m_notes.size(); i++)
                notesToRemove << i;
            while (!jarr.empty()) {
                QJsonValue jval = jarr.first();
                if (jval.isObject()) {
                    QJsonObject jobj = jval.toObject();
                    if (!jobj.isEmpty() && !jobj.value(noteRoles[errorRole]).toBool(true)) {
                        Note note = Note::fromjson(jobj);
                        int position = indexOf(note.id);
                        if (position >= 0) {
                            m_notes[position].note = note;
                            emit dataChanged(index(position), index(position));
                            notesToRemove.removeAt(position);
                        }
                        else {
                            position = insertPosition(note);
                            //beginInsertRows(QModelIndex(), position, position);
                            ModelNote<Note, int> noteToAdd;
                            noteToAdd.note = note; noteToAdd.param = position;
                            notesToAdd << noteToAdd;
                            //m_notes[position].note = note;
                            //endInsertRows();
                        }
                        notesModified++;
                    }
                }
                jarr.pop_front();
            }
            for (int i = 0; i < notesToRemove.size(); i++) {
                beginRemoveRows(QModelIndex(), notesToRemove[i], notesToRemove[i]);
                m_notes.removeAt(notesToRemove[i]);
                endRemoveRows();
            }
            for (int i = 0; i < notesToAdd.size(); i++) {
                beginInsertRows(QModelIndex(), notesToAdd[i].param, notesToAdd[i].param);
                ModelNote<Note, bool> note;
                note.note = notesToAdd[i].note;
                m_notes.insert(notesToAdd[i].param, note);
                endInsertRows();
            }
        }
        else if (jdoc.isObject()) {
            QJsonObject jobj = jdoc.object();
            if (!jobj.isEmpty() && !jobj.value(noteRoles[errorRole]).toBool(true)) {
                Note note;
                note.fromjson(jobj);
                int position = indexOf(note.id);
                if (position >= 0) {
                    m_notes[position].note = note;
                }
                else {
                    position = insertPosition(note);
                    beginInsertRows(index(position), position, position);
                    m_notes[position].note = note;
                    endInsertRows();
                }
                notesModified++;
            }
        }
        if (notesModified > 0) {
            sort(); // TODO react to signal connect()
            search(m_searchQuery);
        }
        return true;
    }
    return false;
}

bool NotesModel::removeNote(int id) {
    // TODO
    return false;
}

void NotesModel::search(QString query) {
    m_searchQuery = query;
}

void NotesModel::clearSearch() {
    search("");
}

int NotesModel::indexOf(int id) const {
    for (int i = 0; i < m_notes.size(); i++) {
        if (m_notes[i].note.id == id)
            return i;
    }
    return -1;
}

/*
bool NotesModel::addNote(Note &note) {
    m_notes.append(note);
    return false;
}

bool NotesModel::addNotes(QList<Note> &notes) {
    for (int i = 0; i < notes.length(); i++) {
        addNote(notes[i]);
    }
    return false;
}
*/

QHash<int, QByteArray> NotesModel::roleNames() const {
    return noteRoles;
}

QHash<int, QByteArray> NotesModel::sortingNames() const {
    QHash<int, QByteArray> criteria;
    criteria[sortByDate] = "date";
    criteria[sortByCategory] = "category";
    criteria[sortByTitle] = "title";
    return criteria;
}

Qt::ItemFlags NotesModel::flags(const QModelIndex &index) const {
    return Qt::ItemIsEnabled | Qt::ItemIsEditable; // | Qt::ItemIsSelectable
}

int NotesModel::rowCount(const QModelIndex &parent) const {
    return m_notes.size();
}

QVariant NotesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    else if (role == visible) return m_notes[index.row()].param;
    else if (role == idRole) return m_notes[index.row()].note.id;
    else if (role == modifiedRole) return m_notes[index.row()].note.modified;
    else if (role == titleRole) return m_notes[index.row()].note.title;
    else if (role == categoryRole) return m_notes[index.row()].note.category;
    else if (role == contentRole) return m_notes[index.row()].note.content;
    else if (role == favoriteRole) return m_notes[index.row()].note.favorite;
    else if (role == etagRole) return m_notes[index.row()].note.etag;
    else if (role == errorRole) return m_notes[index.row()].note.error;
    else if (role == errorMessageRole) return m_notes[index.row()].note.errorMessage;
    return QVariant();
}

bool NotesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()) return false;
    else if (role == modifiedRole) {
        m_notes[index.row()].note.modified = value.toInt();
        emit dataChanged(this->index(index.row()), this->index(index.row()), QVector<int>  { 1, role } );
        sort();
        return true;
    }
    else if (role == categoryRole) {
        m_notes[index.row()].note.category = value.toString();
        emit dataChanged(this->index(index.row()), this->index(index.row()), QVector<int>  { 1, role } );
        sort();
        return true;
    }
    else if (role == contentRole) {
        m_notes[index.row()].note.content = value.toString();
        emit dataChanged(this->index(index.row()), this->index(index.row()), QVector<int>  { 1, role } );
        sort();
        return true;
    }
    else if (role == favoriteRole) {
        m_notes[index.row()].note.favorite = value.toBool();
        emit dataChanged(this->index(index.row()), this->index(index.row()), QVector<int>  { 1, role } );
        sort();
        return true;
    }
    return false;
}

void NotesModel::sort() {
    QList<ModelNote<Note, bool> > notes;
    QMap<QString, ModelNote<Note, bool> > map;
    QMap<QString, ModelNote<Note, bool> > favorites;
    switch (m_sortBy) {
    case sortByDate:
        emit layoutAboutToBeChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
        for (int i = 0; i < m_notes.size(); i++) {
            if (m_favoritesOnTop && m_notes[i].note.favorite)
                favorites.insert(QString::number(m_notes[i].note.modified), m_notes[i]);
            else
                map.insert(QString::number(m_notes[i].note.modified), m_notes[i]);
        }
        notes = favorites.values();
        notes.append(map.values());
        m_notes = notes;
        emit layoutChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
        break;
    case sortByCategory:
        emit layoutAboutToBeChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
        for (int i = 0; i < m_notes.size(); i++) {
            if (m_favoritesOnTop && m_notes[i].note.favorite)
                favorites.insert(m_notes[i].note.category, m_notes[i]);
            else
                map.insert(m_notes[i].note.category, m_notes[i]);
        }
        notes = favorites.values();
        notes.append(map.values());
        m_notes = notes;
        emit layoutChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
        break;
    case sortByTitle:
        emit layoutAboutToBeChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
        for (int i = 0; i < m_notes.size(); i++) {
            if (m_favoritesOnTop && m_notes[i].note.favorite)
                favorites.insert(m_notes[i].note.title, m_notes[i]);
            else
                map.insert(m_notes[i].note.title, m_notes[i]);
        }
        notes = favorites.values();
        notes.append(map.values());
        m_notes = notes;
        emit layoutChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
        break;
    default:
        break;
    }
}

bool NotesModel::noteLessThan(const Note &n1, const Note &n2) const {
    switch (m_sortBy) {
    case sortByDate:
        if (m_favoritesOnTop && n1.favorite != n2.favorite)
            return n1.favorite;
        else
            return n1.modified > n2.modified;
        break;
    case sortByCategory:
        if (m_favoritesOnTop && n1.favorite != n2.favorite)
            return n1.favorite;
        else
            return n1.category < n2.category;
        break;
    case sortByTitle:
        if (m_favoritesOnTop && n1.favorite != n2.favorite)
            return n1.favorite;
        else
            return n1.title < n2.title;
        break;
    default:
        break;
    }
    return true;
}

int NotesModel::insertPosition(const Note &n) const {
    int lower = 0;
    int upper = m_notes.size();
    while (lower < upper) {
        int middle = qFloor(lower + (upper-lower) / 2);
        bool result = noteLessThan(n, m_notes[middle].note);
        if (result)
            upper = middle;
        else
            lower = middle + 1;
    }
    return lower;
}

/*bool NotesModel::noteLessThanByDate(const Note &n1, const Note &n2) {
    if (m_favoritesOnTop && n1.favorite != n2.favorite)
        return n1.favorite;
    else
        return n1.modified > n2.modified;
}

bool NotesModel::noteLessThanByCategory(const Note &n1, const Note &n2) {
    if (m_favoritesOnTop && n1.favorite != n2.favorite)
        return n1.favorite;
    else
        return n1.category < n2.category;
}

bool NotesModel::noteLessThanByTitle(const Note &n1, const Note &n2) {
    if (m_favoritesOnTop && n1.favorite != n2.favorite)
        return n1.favorite;
    else
        return n1.title < n2.title;
}*/

/*
bool NotesModel::insertRow(int row, const QModelIndex &parent) {
    beginInsertRows(parent, row, row);
    m_notes.insert(row, Note());
    endInsertRows();
    return true;
}

bool NotesModel::insertRows(int row, int count, const QModelIndex &parent) {
    if (count > 0) {
        beginInsertRows(parent, row, row+count);
        for (int i = 0; i < count; i++) {
            m_notes.insert(row + i, Note());
        }
        endInsertRows();
        return true;
    }
    else {
        return false;
    }
}

bool NotesModel::removeRow(int row, const QModelIndex &parent) {
    if (row >= 0 && row < m_notes.size()) {
        beginRemoveRows(parent, row, row);
        m_notes.removeAt(row);
        endRemoveRows();
        return true;
    }
    else {
        return false;
    }
}

bool NotesModel::removeRows(int row, int count, const QModelIndex &parent) {
    if (row >= 0 && row < m_notes.size()) {
        beginRemoveRows(parent, row, count);
        for (int i = 0; i < count && row + i < m_notes.size(); i++) {
            m_notes.removeAt(row);
        }
        endRemoveRows();
        return true;
    }
    else {
        return false;
    }
}
*/
