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
    mp_notesApi = nullptr;
    mp_notesStore = nullptr;
}

NotesModel::~NotesModel() {
    setNotesApi(nullptr);
    setNotesStore(nullptr);
    clear();
}

void NotesModel::setNotesApi(NotesApi *notesApi) {
    if (mp_notesApi) {
        // disconnect stuff
        disconnect(mp_notesApi, SIGNAL(accountChanged(QString)), this, SIGNAL(accountChanged(QString)));
        disconnect(mp_notesApi, SIGNAL(noteCreated(int,QJsonObject)), this, SLOT(insert(int,QJsonObject)));
        disconnect(mp_notesApi, SIGNAL(noteUpdated(int,QJsonObject)), this, SLOT(update(int,QJsonObject)));
        disconnect(mp_notesApi, SIGNAL(noteDeleted(int)), this, SLOT(remove(int)));
    }
    mp_notesApi = notesApi;
    if (mp_notesApi) {
        // connect stuff
        connect(mp_notesApi, SIGNAL(accountChanged(QString)), this, SIGNAL(accountChanged(QString)));
        connect(mp_notesApi, SIGNAL(noteCreated(int,QJsonObject)), this, SLOT(insert(int,QJsonObject)));
        connect(mp_notesApi, SIGNAL(noteUpdated(int,QJsonObject)), this, SLOT(update(int,QJsonObject)));
        connect(mp_notesApi, SIGNAL(noteDeleted(int)), this, SLOT(remove(int)));
    }
}

void NotesModel::setNotesStore(NotesStore *notesStore) {
    if (mp_notesStore) {
        // disconnect stuff
        disconnect(mp_notesStore, SIGNAL(accountChanged(QString)), this, SIGNAL(accountChanged(QString)));
        disconnect(mp_notesStore, SIGNAL(noteCreated(int,QJsonObject)), this, SLOT(insert(int,QJsonObject)));
        disconnect(mp_notesStore, SIGNAL(noteUpdated(int,QJsonObject)), this, SLOT(update(int,QJsonObject)));
        disconnect(mp_notesStore, SIGNAL(noteDeleted(int)), this, SLOT(remove(int)));
    }
    mp_notesStore = notesStore;
    if (mp_notesStore) {
        // connect stuff
        connect(mp_notesStore, SIGNAL(accountChanged(QString)), this, SIGNAL(accountChanged(QString)));
        connect(mp_notesStore, SIGNAL(noteCreated(int,QJsonObject)), this, SLOT(insert(int,QJsonObject)));
        connect(mp_notesStore, SIGNAL(noteUpdated(int,QJsonObject)), this, SLOT(update(int,QJsonObject)));
        connect(mp_notesStore, SIGNAL(noteDeleted(int)), this, SLOT(remove(int)));
    }
}

QString NotesModel::account() const {
    QString account;
    if (mp_notesStore)
        account = mp_notesStore->account();
    else if (mp_notesApi)
        account = mp_notesApi->account();
    return account;
}

void NotesModel::setAccount(const QString &account) {
    if (mp_notesApi)
        mp_notesApi->setAccount(account);
    if (mp_notesStore)
        mp_notesStore->setAccount(account);
}

const QList<int> NotesModel::noteIds() {
    return mp_notesStore->noteIds();
}

bool NotesModel::noteExists(const int id) {
    return mp_notesStore->noteExists(id);
}

int NotesModel::noteModified(const int id) {
    return mp_notesStore->noteModified(id);
}

const QVariantMap NotesModel::getNoteById(const int id) const {
    return mp_notesStore->readNoteFile(id).toVariantMap();
}

bool NotesModel::getAllNotes(const QStringList &exclude) {
    bool success = true;
    if (mp_notesApi)
        success &= mp_notesApi->getAllNotes(exclude);
    if (mp_notesStore)
        success &= mp_notesStore->getAllNotes(exclude);
    return success;
}

bool NotesModel::getNote(const int id, const QStringList &exclude) {
    bool success = true;
    if (mp_notesApi)
        success &= mp_notesApi->getNote(id, exclude);
    if (mp_notesStore)
        success &= mp_notesStore->getNote(id, exclude);
    return success;
}

bool NotesModel::createNote(const QJsonObject &note) {
    bool success = true;
    if (mp_notesApi)
        success &= mp_notesApi->createNote(note);
    return success;
}

bool NotesModel::updateNote(const int id, const QJsonObject &note) {
    bool success = true;
    if (mp_notesApi)
        success &= mp_notesApi->updateNote(id, note);
    if (mp_notesStore)
        success &= mp_notesStore->updateNote(id, note);
    return success;
}

bool NotesModel::deleteNote(const int id) {
    bool success = true;
    if (mp_notesApi)
        success &= mp_notesApi->deleteNote(id);
    if (mp_notesStore)
        success &= mp_notesStore->deleteNote(id);
    return success;
}

bool NotesModel::syncNotes() {
    if (mp_notesApi && mp_notesStore) {
        // TODO
    }
    return false;
}

void NotesModel::insert(const int id, const QJsonObject& note) {
    qDebug() << "Inserting note: " << id;
    if (m_notes.contains(id)) {
        qDebug() << "Note already present";
        update(id, note);
    }
    else {
        beginInsertRows(QModelIndex(), indexOfNoteById(id), indexOfNoteById(id));
        m_notes.insert(id, note);
        endInsertRows();
        //emit noteInserted(id, note);
        qDebug() << "Note inserted";
    }
    if (mp_notesApi && mp_notesStore) {
        if (sender() == mp_notesApi) {
            if (!mp_notesStore->noteExists(id)) {
                mp_notesStore->createNote(note);
            }
        }
    }
}

void NotesModel::update(const int id, const QJsonObject &note) {
    qDebug() << "Updating note: " << id;
    if (!m_notes.contains(id)) {
        qDebug() << "Note is new";
        insert(id, note);
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
    if (mp_notesApi && mp_notesStore) {
        if (sender() == mp_notesApi) {
            if (Note::modified(note) > mp_notesStore->noteModified(id)) {
                mp_notesStore->updateNote(id, note);
            }
        }
        if (sender() == mp_notesStore) {
            if (Note::modified(note) > mp_notesApi->noteModified(id) && !mp_notesApi->lastSync().isNull()) {
                mp_notesApi->updateNote(id, note);
            }
        }
    }
}

void NotesModel::remove(const int id) {
    qDebug() << "Removing note: " << id;
    if (m_notes.contains(id)) {
        beginRemoveRows(QModelIndex(), indexOfNoteById(id), indexOfNoteById(id));
        if (m_notes.remove(id) > 0) {
            emit noteDeleted(id);
        }
        else {
            qDebug() << "Note not found";
        }
        endRemoveRows();
    }
    if (mp_notesApi && mp_notesStore) {
        if (sender() == mp_notesApi) {
            mp_notesStore->deleteNote(id);
        }
        if (sender() == mp_notesStore) {
            mp_notesApi->deleteNote(id);
        }
    }
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
    //qDebug();
    QVariant data;
    if (index.isValid() && index.row() <= m_notes.size()) {
        QMap<int, QJsonObject>::const_iterator i = m_notes.cbegin();
        i += index.row();
        data = i.value()[(m_roleNames[role])];
    }
    return data;
}

bool NotesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    //qDebug();
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
    //qDebug();
    QMap<int, QVariant> map;
    if (index.isValid() && index.row() <= m_notes.size()) {
        for (int role = IdRole; role < NoneRole; ++role) {
            map.insert(role, data(index, role));
        }
    }
    return map;
}

bool NotesModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) {
    //qDebug();
    bool retval = true;
    QMapIterator<int, QVariant> role(roles);
    while (role.hasNext()) {
        role.next();
        retval &= setData(index, role.value(), role.key());
    }
    return retval;
}
