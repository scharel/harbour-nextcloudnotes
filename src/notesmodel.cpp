#include "notesmodel.h"
//#include <algorithm>    // std::sort
#include <QJsonDocument>
#include <QJsonObject>
#include <QtMath>
#include <QDebug>

NotesProxyModel::NotesProxyModel(QObject *parent) {
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
    invalidate();
    QSortFilterProxyModel::sort(0);
}

NotesModel::NotesModel(QObject *parent) {

}

NotesModel::~NotesModel() {
    m_notes.clear();
}

bool NotesModel::fromJsonDocument(const QJsonDocument &jdoc) {
    qDebug() << "Applying new JSON input"; // << json;
    if (!jdoc.isNull() && !jdoc.isEmpty()) {
        if (jdoc.isArray()) {
            qDebug() << "- It's an array...";
            QVector<double> notesIdsToRemove;
            QJsonArray jarr = jdoc.array();
            if (!jarr.empty())
                notesIdsToRemove = ids();
            while (!jarr.empty()) {
                QJsonValue jval = jarr.first();
                if (jval.isObject()) {
                    QJsonObject jobj = jval.toObject();
                    if (!jobj.isEmpty()) {
                        insertNote(jobj);
                        notesIdsToRemove.removeAll(Note::id(jobj));
                    }
                }
                else {
                    qDebug() << "-- JSON array element is not an object!";
                }
                jarr.pop_front();
            }
            while (!notesIdsToRemove.empty()) {
                removeNote(notesIdsToRemove.first());
                notesIdsToRemove.pop_front();
            }
            return true;
        }
        else if (jdoc.isObject()) {
            qDebug() << "- It's a single object...";
            insertNote(jdoc.object());
        }
        else if (jdoc.isEmpty()) {
            qDebug() << "- Empty JSON document.";
        }
        else {
            qDebug() << "- Unknown JSON document. This message should never occure!";
        }
    }
    else {
        qDebug() << "JSON document is NULL!";
    }
    return false;
}

QJsonDocument NotesModel::toJsonDocument() const {
    QJsonArray jarr;
    for (int i = 0; i < m_notes.size(); ++i) {
        jarr << m_notes[i].toJsonValue();
    }
    return QJsonDocument(jarr);
}

QVector<double> NotesModel::ids() const {
    QVector<double> ids;
    for (int i = 0; i < m_notes.size(); ++i) {
        ids.append(m_notes[i].id());
    }
    return ids;
}

int NotesModel::insertNote(const Note &note) {
    int position = m_notes.indexOf(note);
    if (position >= 0) {
        if (m_notes.at(position).equal(note)) {
            qDebug() << "-- Note already present but unchanged.";
        }
        else {
            qDebug() << "-- Note already present, updating it.";
            m_notes.replace(position, note);
            emit dataChanged(index(position), index(position));
        }
    }
    else {
        qDebug() << "-- New note, adding it";
        position = m_notes.size();
        beginInsertRows(QModelIndex(), position, position);
        m_notes.append(note);
        endInsertRows();
        emit dataChanged(index(position), index(position));
    }
    return position;
}

bool NotesModel::removeNote(const Note &note) {
    int position = m_notes.indexOf(note);
    if (position >= 0 && position < m_notes.size()) {
        beginRemoveRows(QModelIndex(), position, position);
        m_notes.removeAt(position);
        endRemoveRows();
        emit dataChanged(index(position), index(position));
        return true;
    }
    return false;
}

bool NotesModel::removeNote(double id) {
    return removeNote(Note(QJsonObject{ {"id", id} } ));
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
        {NotesModel::ModifiedStringRole, "modifiedString"},
        {NotesModel::NoneRole, "none"}
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
    else if (role == ModifiedStringRole) return m_notes[index.row()].modifiedString();
    return QVariant();
}

bool NotesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    bool retval = false;
    if (index.isValid()) {
        switch (role) {
        case IdRole: {
            double id = value.toDouble(&retval);
            if (retval) {
                m_notes[index.row()].setId(id);
                emit dataChanged(index, index, QVector<int>{ IdRole });
            }
            break;
        }
        case ModifiedRole: {
            double modified = value.toDouble(&retval);
            if (retval) {
                m_notes[index.row()].setModified(modified);
                emit dataChanged(index, index, QVector<int>{ ModifiedRole });
            }
            break;
        }
        case TitleRole: {
            QString title = value.toString();
            if (!title.isEmpty()) {
                m_notes[index.row()].setTitle(title);
                emit dataChanged(index, index, QVector<int>{ TitleRole });
                retval = true;
            }
            break;
        }
        case CategoryRole: {
            QString category = value.toString();
            if (!category.isEmpty()) {
                m_notes[index.row()].setCategory(category);
                emit dataChanged(index, index, QVector<int>{ CategoryRole });
                retval = true;
            }
            break;
        }
        case ContentRole: {
            QString content = value.toString();
            if (!content.isEmpty()) {
                m_notes[index.row()].setContent(content);
                emit dataChanged(index, index, QVector<int>{ ContentRole });
                retval = true;
            }
            break;
        }
        case FavoriteRole: {
            bool favorite = value.toBool();
            m_notes[index.row()].setFavorite(favorite);
            emit dataChanged(index, index, QVector<int>{ FavoriteRole });
            retval = true;
            break;
        }
        case EtagRole: {
            QString etag = value.toString();
            if (!etag.isEmpty()) {
                m_notes[index.row()].setEtag(etag);
                emit dataChanged(index, index, QVector<int>{ EtagRole });
                retval = true;
            }
            break;
        }
        case ErrorRole: {
            bool error = value.toBool();
            m_notes[index.row()].setError(error);
            emit dataChanged(index, index, QVector<int>{ ErrorRole });
            retval = true;
            break;
        }
        case ErrorMessageRole: {
            QString errorMessage = value.toString();
            if (!errorMessage.isEmpty()) {
                m_notes[index.row()].setErrorMessage(errorMessage);
                emit dataChanged(index, index, QVector<int>{ ErrorMessageRole });
                retval = true;
            }
            break;
        } }
    }
    return retval;
}

QMap<int, QVariant> NotesModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    if (!index.isValid()) return map;
    else {
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
