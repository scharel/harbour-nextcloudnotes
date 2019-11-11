#include "notesmodel.h"
#include <algorithm>    // std::sort
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>
#include <QDebug>

NotesProxyModel::NotesProxyModel(QObject *parent) {
    m_favoritesOnTop = true;
}

NotesProxyModel::~NotesProxyModel() {

}

void NotesProxyModel::setFavoritesOnTop(bool favoritesOnTop) {
    qDebug() << "Favorites on top:" << favoritesOnTop;
    if (favoritesOnTop != m_favoritesOnTop) {
        m_favoritesOnTop = favoritesOnTop;
        emit favoritesOnTopChanged(m_favoritesOnTop);
    }
}

QHash<int, QByteArray> NotesProxyModel::sortingNames() const {
    return QHash<int, QByteArray> {
        {ModifiedRole, roleNames()[ModifiedRole]},
        {CategoryRole, roleNames()[CategoryRole]},
        {TitleRole, roleNames()[TitleRole]},
        {noSorting, "none"}
    };
}

QList<int> NotesProxyModel::sortingRoles() const {
    return sortingNames().keys();
}

int NotesProxyModel::sortingRole(const QString &name) const {
    return sortingNames().key(name.toLocal8Bit());
}

bool NotesProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const {
    QAbstractItemModel* source = sourceModel();
    if (m_favoritesOnTop && source->data(source_left, NotesModel::FavoriteRole) != source->data(source_right, NotesModel::FavoriteRole))
        return source->data(source_left, NotesModel::FavoriteRole).toBool();
    else
        return source->data(source_left, sortRole()) < source->data(source_right, sortRole());
}

NotesModel::NotesModel(QObject *parent) {

}

NotesModel::~NotesModel() {

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

int NotesModel::indexOf(const Note &note) const {
    return indexOf(note.id());
}

int NotesModel::indexOf(int id) const {
    int retval = -1;
    for (int i = 0; i < m_notes.size(); ++i) {
        if (m_notes[i].id() == id) {
            retval = i;
        }
    }
    return retval;
}

int NotesModel::insertNote(const Note &note) {
    int position = indexOf(note.id());
    if (position >= 0) {
        if (note.etag() != m_notes[position].etag()) {
            qDebug() << "-- Existing note " << note.title() << "changed, updating the model.";
            m_notes.replace(position, note);
            emit dataChanged(index(position), index(position));
        }
        else {
            qDebug() << "-- Existing note " << note.title() << "unchanged, nothing to do.";
        }
    }
    else {
        qDebug() << "-- New note" << note.title() << ", adding it to the model.";
        position = rowCount();
        beginInsertRows(QModelIndex(), position, position);
        m_notes.append(note);
        endInsertRows();
    }
    return position;
}

bool NotesModel::removeNote(const Note &note) {
    return removeNote(note.id());
}

bool NotesModel::removeNote(int id) {
    int position = indexOf(id);
    if (position >= 0 && position < m_notes.size()) {
        beginRemoveRows(QModelIndex(), position, position);
        m_notes.removeAt(position);
        endRemoveRows();
        return true;
    }
    return false;
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
        {NotesModel::ErrorMessageRole, "errorMessage"}
    };
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
    return QVariant();
}

QMap<int, QVariant> NotesModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    if (!index.isValid()) return map;
    else {
        for (int role = IdRole; role <= ErrorMessageRole; ++role) {
            map.insert(role, data(index, role));
        }
    }
    return map;
}
