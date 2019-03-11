#include "notesmodel.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>
#include <QDebug>

NotesModel::NotesModel(QObject *parent) : QAbstractListModel(parent)
{
    m_sortBy = noSorting;
    m_favoritesOnTop = false;
}

NotesModel::~NotesModel() {
    clear();
}

void NotesModel::setSortBy(QString sortBy) {
    if (sortBy != m_sortBy && sortingNames().values().contains(sortBy.toLocal8Bit())) {
        m_sortBy = sortBy;
        sort();
        emit sortByChanged(m_sortBy);
    }
}

void NotesModel::setFavoritesOnTop(bool favoritesOnTop) {
    qDebug() << "Changed favorites on top:" << favoritesOnTop;
    if (favoritesOnTop != m_favoritesOnTop) {
        m_favoritesOnTop = favoritesOnTop;
        sort();
        emit favoritesOnTopChanged(m_favoritesOnTop);
    }
}

void NotesModel::setSearchText(QString searchText) {
    if (searchText != m_searchText) {
        search(searchText);
    }
}

bool NotesModel::applyJSON(QString json, bool replaceIfArray) {
    //qDebug() << "Applying JSON...";// << json;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toUtf8());
    int notesModified = 0;
    if (!jdoc.isNull()) {
        if (jdoc.isArray()) {
            //qDebug() << "It's an array...";
            QJsonArray jarr = jdoc.array();
            QList<int> notesToRemove;
            for (int i = 0; i < m_notes.size(); i++)
                notesToRemove << i;
            while (!jarr.empty()) {
                //qDebug() << jarr.count() << "JSON Objects to handle...";
                QJsonValue jval = jarr.first();
                if (jval.isObject()) {
                    //qDebug() << "It's an object, all fine...";
                    QJsonObject jobj = jval.toObject();
                    if (!jobj.isEmpty() && !jobj.value(roleNames()[ErrorRole]).toBool(true)) {
                        //qDebug() << "Adding it to the model...";
                        Note* note = Note::fromjson(jobj); // TODO connect signals
                        int position = indexOf(note->id());
                        if (position >= 0 && replaceIfArray) {
                            //qDebug() << "Replacing note" << note.title << "on position" << position;
                            m_notes[position].note = note;
                            emit dataChanged(index(position), index(position));
                            notesToRemove.removeAt(position);
                            delete note;
                        }
                        else {
                            //qDebug() << "New note" << note.title << "adding it to the notes to add...";
                            position = insertPosition(*note);
                            ModelNote<Note*, bool> noteToInsert;
                            noteToInsert.note = note; noteToInsert.param = true;
                            //qDebug() << "Adding note"<< note.title << "on position" << position;
                            beginInsertRows(QModelIndex(), position, position);
                            m_notes.insert(position, noteToInsert);
                            endInsertRows();
                        }
                        notesModified++;
                    }
                    else {
                        qDebug() << "Something is wrong, skipping it...";
                    }
                }
                jarr.pop_front();
            }
            // TODO the current implementation does not respect the changement of the index
            /*for (int i = 0; i < notesToRemove.size(); i++) {
                qDebug() << "Removing note" << m_notes[notesToRemove[i]].note.title;
                beginRemoveRows(QModelIndex(), notesToRemove[i], notesToRemove[i]);
                m_notes.removeAt(notesToRemove[i]);
                endRemoveRows();
            }*/
            /*for (int i = 0; i < notesToAdd.size(); i++) {
                beginInsertRows(QModelIndex(), notesToAdd[i].param, notesToAdd[i].param);
                ModelNote<Note, bool> note;
                note.note = notesToAdd[i].note;
                qDebug() << "Adding note"<< note.note.title << "on position" << notesToAdd[i].param;
                m_notes.insert(notesToAdd[i].param, note);
                endInsertRows();
            }*/
        }
        else if (jdoc.isObject()) {
            //qDebug() << "It's a single object...";
            QJsonObject jobj = jdoc.object();
            if (!jobj.isEmpty() && !jobj.value(roleNames()[ErrorRole]).toBool(true)) {
                Note* note = Note::fromjson(jobj); // TODO connect signals
                int position = indexOf(note->id());
                if (position >= 0 && replaceIfArray) {
                    m_notes[position].note = note;
                    delete note;
                }
                else {
                    position = insertPosition(*note);
                    ModelNote<Note*, bool> noteToInsert;
                    noteToInsert.note = note; noteToInsert.param = true;
                    beginInsertRows(index(position), position, position);
                    m_notes.insert(position, noteToInsert);
                    endInsertRows();
                }
                notesModified++;
            }
        }
        if (notesModified > 0) {
            sort(); // TODO react to signal connect()
            search(m_searchText);
        }
        return true;
    }
    return false;
}

bool NotesModel::removeNote(int id) {
    bool noteRemoved = false;
    int index = indexOf(id);
    while (index >= 0) {
        beginRemoveRows(QModelIndex(), index, index);
        m_notes.removeAt(index);
        endRemoveRows();
        noteRemoved = true;
        index = indexOf(id);
    }
    return noteRemoved;
}

void NotesModel::clear() {
    m_searchText.clear();
    beginRemoveRows(QModelIndex(), 0, rowCount());
    for (int i = 0; i < m_notes.size(); i++) {
        delete m_notes[i].note; // TODO disconnect signals
    }
    m_notes.clear();
    endRemoveRows();
}

void NotesModel::search(QString searchText) {
    if (m_searchText != searchText) {
        m_searchText = searchText;
        emit searchTextChanged(m_searchText);
    }
    for (int i = 0; i < m_notes.size(); i++) {
        if (m_searchText.isEmpty()) {
            m_notes[i].param = true;
        }
        else {
            m_notes[i].param = Note::searchInNote(m_searchText, m_notes[i].note);
        }
    }
}

void NotesModel::clearSearch() {
    search("");
}

int NotesModel::indexOf(int id) const {
    for (int i = 0; i < m_notes.size(); i++) {
        if (m_notes[i].note->id() == id)
            return i;
    }
    return -1;
}

Note* NotesModel::get(int index) const {
    Note* note = NULL;
    if (index >= 0 && index < m_notes.size()) {
        note = m_notes[index].note;
    }
    return note;
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
    return QHash<int, QByteArray> {
        {NotesModel::VisibleRole, "visible"},
        {NotesModel::IdRole, "id"},
        {NotesModel::ModifiedRole, "modified"},
        {NotesModel::TitleRole, "title"},
        {NotesModel::CategoryRole, "category"},
        {NotesModel::ContentRole, "content"},
        {NotesModel::FavoriteRole, "favorite"},
        {NotesModel::EtagRole, "etag"},
        {NotesModel::ErrorRole, "error"},
        {NotesModel::ErrorMessageRole, "errorMessage"},
        {NotesModel::DateRole, "date"}
    };
}

QHash<int, QByteArray> NotesModel::sortingNames() const {
    QHash<int, QByteArray> criteria;
    criteria[sortByDate] = "date";
    criteria[sortByCategory] = "category";
    criteria[sortByTitle] = "title";
    criteria[noSorting] = "none";
    return criteria;
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
    else if (role == VisibleRole) return m_notes[index.row()].param;
    else if (role == IdRole) return m_notes[index.row()].note->id();
    else if (role == ModifiedRole) return m_notes[index.row()].note->modified();
    else if (role == TitleRole) return m_notes[index.row()].note->title();
    else if (role == CategoryRole) return m_notes[index.row()].note->category();
    else if (role == ContentRole) return m_notes[index.row()].note->content();
    else if (role == FavoriteRole) return m_notes[index.row()].note->favorite();
    else if (role == EtagRole) return m_notes[index.row()].note->etag();
    else if (role == ErrorRole) return m_notes[index.row()].note->error();
    else if (role == ErrorMessageRole) return m_notes[index.row()].note->errorMessage();
    else if (role == DateRole) return m_notes[index.row()].note->date();
    return QVariant();
}

QMap<int, QVariant> NotesModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    if (!index.isValid()) return map;
    else {
        for (int role = VisibleRole; role <= ErrorMessageRole; role++) {
            map.insert(role, data(index, role));
        }
    }
    return map;
}

bool NotesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()) return false;
    else if (role == ModifiedRole && m_notes[index.row()].note->modified() != value.toUInt()) {
        m_notes[index.row()].note->setModified(value.toInt());
        emit dataChanged(this->index(index.row()), this->index(index.row()), QVector<int> { 1, role } ); // TODO remove when signals from Note are connected
        emit dataChanged(this->index(index.row()), this->index(index.row()), QVector<int> { 1, DateRole} ); // TODO remove when signals from Note are connected
        sort();
        return true;
    }
    else if (role == CategoryRole && m_notes[index.row()].note->category() != value.toString()) {
        m_notes[index.row()].note->setCategory(value.toString());
        emit dataChanged(this->index(index.row()), this->index(index.row()), QVector<int> { 1, role } ); // TODO remove when signals from Note are connected
        sort();
        return true;
    }
    else if (role == ContentRole && m_notes[index.row()].note->content() != value.toString()) {
        m_notes[index.row()].note->setContent(value.toString());
        emit dataChanged(this->index(index.row()), this->index(index.row()), QVector<int> { 1, role } ); // TODO remove when signals from Note are connected
        sort();
        return true;
    }
    else if (role == FavoriteRole && m_notes[index.row()].note->favorite() != value.toBool()) {
        m_notes[index.row()].note->setFavorite(value.toBool());
        emit dataChanged(this->index(index.row()), this->index(index.row()), QVector<int> { 1, role } ); // TODO remove when signals from Note are connected
        sort();
        return true;
    }
    return false;
}

bool NotesModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) {
    if (!index.isValid()) return false;
    else if (roles.contains(ModifiedRole) || roles.contains(CategoryRole) || roles.contains(ContentRole) || roles.contains(FavoriteRole)) {
        QMap<int, QVariant>::const_iterator i = roles.constBegin();
         while (i != roles.constEnd()) {
             setData(index, i.value(), i.key());
             i++;
         }
    }
    return false;
}

void NotesModel::sort() {
    qDebug() << "Sorting notes in the model";
    QList<ModelNote<Note*, bool> > notes;
    QMap<QString, ModelNote<Note*, bool> > map;
    QMap<QString, ModelNote<Note*, bool> > favorites;
    if (m_sortBy == sortingNames()[sortByDate]) {
        emit layoutAboutToBeChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
        for (int i = 0; i < m_notes.size(); i++) {
            if (m_favoritesOnTop && m_notes[i].note->favorite())
                favorites.insert(QString::number(m_notes[i].note->modified()), m_notes[i]);
            else
                map.insert(QString::number(m_notes[i].note->modified()), m_notes[i]);
        }
        notes = favorites.values();
        notes.append(map.values());
        m_notes = notes;
        emit layoutChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
    }
    else if (m_sortBy == sortingNames()[sortByCategory]) {
        emit layoutAboutToBeChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
        for (int i = 0; i < m_notes.size(); i++) {
            if (m_favoritesOnTop && m_notes[i].note->favorite())
                favorites.insert(m_notes[i].note->category(), m_notes[i]);
            else
                map.insert(m_notes[i].note->category(), m_notes[i]);
        }
        notes = favorites.values();
        notes.append(map.values());
        m_notes = notes;
        emit layoutChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
    }
    else if (m_sortBy == sortingNames()[sortByTitle]) {
        emit layoutAboutToBeChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
        for (int i = 0; i < m_notes.size(); i++) {
            if (m_favoritesOnTop && m_notes[i].note->favorite())
                favorites.insert(m_notes[i].note->title(), m_notes[i]);
            else
                map.insert(m_notes[i].note->title(), m_notes[i]);
        }
        notes = favorites.values();
        notes.append(map.values());
        m_notes = notes;
        emit layoutChanged(QList<QPersistentModelIndex> (), VerticalSortHint);
    }
}

bool NotesModel::noteLessThan(const Note &n1, const Note &n2) const {
    if (m_sortBy == sortingNames()[sortByDate]) {
        if (m_favoritesOnTop && n1.favorite() != n2.favorite())
            return n1.favorite();
        else
            return n1.modified() > n2.modified();
    }
    else if (m_sortBy == sortingNames()[sortByCategory]) {
        if (m_favoritesOnTop && n1.favorite() != n2.favorite())
            return n1.favorite();
        else
            return n1.category() < n2.category();
    }
    else if (m_sortBy == sortingNames()[sortByTitle]) {
        if (m_favoritesOnTop && n1.favorite() != n2.favorite())
            return n1.favorite();
        else
            return n1.title() < n2.title();
    }
    else {
        if (m_favoritesOnTop && n1.favorite() != n2.favorite())
            return n1.favorite();
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