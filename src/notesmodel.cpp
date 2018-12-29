#include "notesmodel.h"
#include <QJsonDocument>
#include <QJsonArray>

const QHash<int, QByteArray> noteRoles = QHash<int, QByteArray>{
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
    void fromjson(const QJsonObject& jobj) {
        id = jobj.value(noteRoles[NotesModel::idRole]).toInt();
        modified = jobj.value(noteRoles[NotesModel::modifiedRole]).toInt();
        title = jobj.value(noteRoles[NotesModel::titleRole]).toString();
        category = jobj.value(noteRoles[NotesModel::categoryRole]).toString();
        content = jobj.value(noteRoles[NotesModel::contentRole]).toString();
        favorite = jobj.value(noteRoles[NotesModel::favoriteRole]).toBool();
        etag = jobj.value(noteRoles[NotesModel::etagRole]).toString();
        error = jobj.value(noteRoles[NotesModel::errorRole]).toBool(true);
        errorMessage = jobj.value(noteRoles[NotesModel::errorMessageRole]).toString();
    }
};

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
    if (!jdoc.isNull()) {
        if (jdoc.isArray()) {
            QJsonArray jarr = jdoc.array();
            while (!jarr.empty()) {
                QJsonValue jval = jarr.first();
                if (jval.isObject()) {
                    QJsonObject jobj = jval.toObject();
                    if (!jobj.isEmpty() && !jobj.value(noteRoles[errorRole]).toBool(true)) {
                        Note note;
                        note.fromjson(jobj);
                        int index = m_notes.indexOf(note);
                        if (index >= 0) {
                            m_notes.replace(index, note);
                        }
                        else {
                            // TODO
                        }
                    }
                }
                jarr.pop_front();
            }
        }
        else if (jdoc.isObject()) {
            QJsonObject jobj = jdoc.object();
            if (!jobj.isEmpty() && !jobj.value(noteRoles[errorRole]).toBool(true)) {
                Note note;
                note.fromjson(jobj);
                int index = m_notes.indexOf(note);
                if (index >= 0) {
                    m_notes.replace(index, note);
                }
                else {
                    // TODO
                }
            }
        }
        sort();
        return true;
    }
    return false;
}

bool NotesModel::removeNote(int id) {
    // TODO
    return false;
}

void NotesModel::search(QString query) const {
    // TODO
}

void NotesModel::clearSearch() const {
    // TODO
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
    QHash<int, QByteArray> roles;
    roles[idRole] = "id";
    roles[modifiedRole] = "modified";
    roles[titleRole] = "title";
    roles[categoryRole] = "category";
    roles[contentRole] = "content";
    roles[favoriteRole] = "favorite";
    roles[etagRole] = "etag";
    roles[errorRole] = "error";
    roles[errorMessageRole] = "errorMessage";
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
    return Qt::ItemIsEnabled; //| Qt::ItemIsEditable | Qt::ItemIsSelectable
}

int NotesModel::rowCount(const QModelIndex &parent) const {
    return m_notes.size();
}

QVariant NotesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    else if (role == idRole) return m_notes[index.row()].id;
    else if (role == modifiedRole) return m_notes[index.row()].modified;
    else if (role == titleRole) return m_notes[index.row()].title;
    else if (role == categoryRole) return m_notes[index.row()].category;
    else if (role == contentRole) return m_notes[index.row()].content;
    else if (role == favoriteRole) return m_notes[index.row()].favorite;
    else if (role == etagRole) return m_notes[index.row()].etag;
    else if (role == errorRole) return m_notes[index.row()].error;
    else if (role == errorMessageRole) return m_notes[index.row()].errorMessage;
    return QVariant();
}

void NotesModel::sort() {
    QList<Note> notes;
    QMap<QString, Note> map;
    QMap<QString, Note> favorites;
    switch (m_sortBy) {
    case sortByDate:
        emit layoutAboutToBeChanged();
        foreach (const Note &note, m_notes) {
            if (m_favoritesOnTop && note.favorite)
                favorites.insert(QString::number(note.modified), note);
            else
                map.insert(QString::number(note.modified), note);
        }
        notes = favorites.values();
        notes.append(map.values());
        m_notes = notes;
        emit layoutChanged();
        break;
    case sortByCategory:
        emit layoutAboutToBeChanged();
        foreach (const Note &note, m_notes) {
            if (m_favoritesOnTop && note.favorite)
                favorites.insert(note.category, note);
            else
                map.insert(note.category, note);
        }
        notes = favorites.values();
        notes.append(map.values());
        m_notes = notes;
        emit layoutChanged();
        break;
    case sortByTitle:
        emit layoutAboutToBeChanged();
        foreach (const Note &note, m_notes) {
            if (m_favoritesOnTop && note.favorite)
                favorites.insert(note.title, note);
            else
                map.insert(note.title, note);
        }
        notes = favorites.values();
        notes.append(map.values());
        m_notes = notes;
        emit layoutChanged();
        break;
    default:
        break;
    }
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

/*bool NotesModel::noteLessThan(const Note &n1, const Note &n2) const {
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
}*/

/*
bool NotesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()) return false;
    else if (role == modifiedRole) {
        m_notes[index.row()].modified = value.toDateTime();
        return true;
    }
    else if (role == categoryRole) {
        m_notes[index.row()].category = value.toString();
        return true;
    }
    else if (role == contentRole) {
        m_notes[index.row()].content = value.toString();
        return true;
    }
    else if (role == favoriteRole) {
        m_notes[index.row()].favorite = value.toBool();
        return true;
    }
    return false;
}

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
