#include "notesmodel.h"
//#include <algorithm>    // std::sort
#include <QJsonDocument>
#include <QJsonObject>
#include <QtMath>
#include <QDebug>

NotesProxyModel::NotesProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {
    m_favoritesOnTop = false;
    m_sortByRole = -1;
    m_searchFilterString = "";
    //connect(this, SIGNAL(favoritesOnTopChanged(bool)), this, SLOT(resort()));
}

NotesProxyModel::~NotesProxyModel() {

}

void NotesProxyModel::setFavoritesOnTop(bool favoritesOnTop) {
    qDebug() << "Favorites on top:" << favoritesOnTop;
    if (favoritesOnTop != m_favoritesOnTop) {
        m_favoritesOnTop = favoritesOnTop;
        emit favoritesOnTopChanged(m_favoritesOnTop);
        sort(0);
    }
}

void NotesProxyModel::setSortBy(const QString sortBy) {
    qDebug() << "Sort by: " << sortBy;
    int role = roleNames().key(sortBy.toLocal8Bit(), -1);
    if (role >= 0 && role != m_sortByRole) {
        m_sortByRole = role;
        emit sortByChanged(sortBy);
        setSortRole(role);
    }
}

void NotesProxyModel::setSearchFilter(const QString searchFilter) {
    qDebug() << "Search by:" << searchFilter;
    if (searchFilter != m_searchFilterString) {
        m_searchFilterString = searchFilter;
        emit searchFilterChanged(m_searchFilterString);
        setFilterFixedString(m_searchFilterString);
    }
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

/*void NotesProxyModel::sort() {
    //invalidate();
    QSortFilterProxyModel::sort(0);
}*/

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

const QString NotesModel::m_fileSuffix = "json";

NotesModel::NotesModel(QObject *parent) : QAbstractListModel(parent) {
    mp_notesApi = nullptr;
    //m_fileDir.setCurrent(directory);
    m_fileDir.setPath("");
    m_fileDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    m_fileDir.setSorting(QDir::Name | QDir::DirsLast);
    m_fileDir.setNameFilters( { "*." + m_fileSuffix } );
}

NotesModel::~NotesModel() {
    /*QMapIterator<int, QFile> i(m_files);
    while (i.hasNext()) {
        i.next();
        m_files.take(i.key()).close();
        i.toFront();
    }*/
    setNotesApi(nullptr);
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
        //connect(mp_notesApi, SIGNAL(accountChanged(QString)), this, SIGNAL(accountChanged(QString)));
        connect(mp_notesApi, SIGNAL(noteCreated(int,QJsonObject)), this, SLOT(insert(int,QJsonObject)));
        connect(mp_notesApi, SIGNAL(noteUpdated(int,QJsonObject)), this, SLOT(update(int,QJsonObject)));
        connect(mp_notesApi, SIGNAL(noteDeleted(int)), this, SLOT(remove(int)));
    }
}

QString NotesModel::account() const {
    if (m_fileDir != QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))) {
        return m_fileDir.path();
    }
    return QString();
}

void NotesModel::setAccount(const QString& account) {
    qDebug() << "Setting account: " << account;
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    if (account != m_fileDir.path()) {
        if (m_fileDir != QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))) {
            m_fileDir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
        }
        if (!account.isEmpty()) {
            m_fileDir.setPath(account);
            if (m_fileDir.mkpath(".")) {
                m_fileDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
                m_fileDir.setSorting(QDir::Name | QDir::DirsLast);
                m_fileDir.setNameFilters( { "*." + m_fileSuffix } );
                emit accountChanged(m_fileDir.path());
            }
            else {
                qDebug() << "Failed to create or already present: " << m_fileDir.path();
                m_fileDir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
                //emit noteError(DirCannotWriteError);
            }
        }
        //qDebug() << account << m_dir.path();
    }
    endRemoveRows();
    beginInsertRows(QModelIndex(), 0, rowCount() - 1);
    qDebug() << rowCount() << "local notes in account";
    endInsertRows();
}
/*
const QList<int> NotesModel::noteIds() {
    QList<int> ids;
    if (m_fileDir.exists() && !account().isEmpty()) {
        QFileInfoList files = m_fileDir.entryInfoList();
        for (int i = 0; i < files.size(); ++i) {
            bool ok;
            int id = files[i].baseName().toInt(&ok);
            if (ok) {
                ids << id;
            }
        }
    }
    else {
        //qDebug() << errorMessage(DirNotFoundError);
        //emit noteError(DirCannotReadError);
    }
    return ids;
}

bool NotesModel::noteExists(const int id) {
    QFileInfo fileinfo(m_fileDir, QString("%1.%2").arg(id).arg(m_fileSuffix));
    return fileinfo.exists();
}

int NotesModel::noteModified(const int id) {
    return Note::modified(QJsonObject::fromVariantMap(getNoteById(id)));
}
*/

int NotesModel::newNotePosition(const int id) const {
    if (m_fileDir.exists() && !account().isEmpty() && id >= 0) {
        QStringList fileList = m_fileDir.entryList();
        qDebug() << fileList;
        fileList << QString("%1.%2").arg(id).arg(m_fileSuffix);
        fileList.sort(Qt::CaseInsensitive);
        qDebug() << fileList;
        return fileList.indexOf(QString("%1.%2").arg(id).arg(m_fileSuffix));
    }
    return -1;
}

const QVariantMap NotesModel::note(const int id) const {
    QVariantMap json;
    if (m_fileDir.exists() && !account().isEmpty() && id >= 0) {
        QFileInfo fileinfo(m_fileDir, QString("%1.%2").arg(id).arg(m_fileSuffix));
        QFile file(fileinfo.filePath());
        if (file.exists()) {
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                json = QJsonDocument::fromJson(file.readAll()).object().toVariantMap();
                file.close();
            }
        }
    }
    return json;
}

bool NotesModel::setNote(const QVariantMap &note, int id) {
    bool ok;
    if (id < 0) {
        id = note.value(m_roleNames[IdRole]).toInt(&ok);
        if (!ok) id = -1;
    }
    ok = false;
    if (m_fileDir.exists() && !account().isEmpty() && id >= 0) {
        QFileInfo fileinfo(m_fileDir, QString("%1.%2").arg(id).arg(m_fileSuffix));
        QFile file(fileinfo.filePath());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QByteArray data = QJsonDocument(QJsonObject::fromVariantMap(note)).toJson();
            if (file.write(data) == data.size()) {
                ok = true;
            }
            file.close();
        }
    }
    return ok;
}

bool NotesModel::deleteNote(const int id) {
    if (m_fileDir.exists() && !account().isEmpty() && id >= 0) {
        QFileInfo fileinfo(m_fileDir, QString("%1.%2").arg(id).arg(m_fileSuffix));
        QFile file(fileinfo.filePath());
        if (file.exists()) {
            if (file.remove()) {
                return true;
            }
        }
    }
    return false;
}

void NotesModel::insert(int id, const QJsonObject& json) {
    if (id < 0) {
        id = json.value(m_roleNames[IdRole]).toInt(-1);
    }
    if (id >= 0) {
        qDebug() << "Inserting note: " << id;
        if (indexOfNoteById(id) >= 0) {
            qDebug() << "Note already present";
            update(id, json);
        }
        else {
            qDebug() << "New position: " << newNotePosition(id);
            beginInsertRows(QModelIndex(), newNotePosition(id), newNotePosition(id));
            setNote(json.toVariantMap(), id);
            endInsertRows();
            //emit noteInserted(id, note);
            qDebug() << "Note inserted";
        }
    }
}

void NotesModel::update(int id, const QJsonObject &json) {
    if (id < 0) {
        id = json.value(m_roleNames[IdRole]).toInt(-1);
    }
    if (id >= 0) {
        qDebug() << "Updating note: " << id;
        if (indexOfNoteById(id) < 0) {
            qDebug() << "Note is new";
            insert(id, json);
        }
        else {
            setNote(json.toVariantMap(), id);
            emit dataChanged(index(indexOfNoteById(id)), index(indexOfNoteById(id)));
            qDebug() << "Note changed";
        }
    }
}

void NotesModel::remove(int id) {
    qDebug() << "Removing note: " << id;
    if (indexOfNoteById(id) >= 0) {
        beginRemoveRows(QModelIndex(), indexOfNoteById(id), indexOfNoteById(id));
        deleteNote(id);
        endRemoveRows();
        qDebug() << "Note removed";
    }
}

int NotesModel::indexOfNoteById(int id) const {
    int index = -1;
    if (m_fileDir.exists() && !account().isEmpty()) {
        index = m_fileDir.entryList().indexOf(QRegExp(QString("^%1.%2$").arg(id).arg(m_fileSuffix)));
    }
    return index;
}

int NotesModel::idOfNoteByINdex(int index) const {
    int id = -1;
    if (m_fileDir.exists() && !account().isEmpty()) {
        QFileInfo fileName = m_fileDir.entryInfoList().value(index);
        bool ok;
        id = fileName.baseName().toInt(&ok);
        if (!ok) id = -1;
    }
    return id;
}

QHash<int, QByteArray> NotesModel::roleNames() const {
    return m_roleNames;
}

int NotesModel::roleFromName(const QString &name) const {
    return roleNames().key(name.toLocal8Bit());
}

Qt::ItemFlags NotesModel::flags(const QModelIndex &index) const {
    if (index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    else {
        return Qt::NoItemFlags;
    }
}

int NotesModel::rowCount(const QModelIndex &parent) const {
    if (m_fileDir.exists() && !account().isEmpty()) {
        return static_cast<int>(m_fileDir.count());
    }
    else {
        return 0;
    }
}

QVariant NotesModel::data(const QModelIndex &index, int role) const {
    return itemData(index).value(role);
}

bool NotesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (setItemData(index, QMap<int, QVariant>{ { role, value } } )) {
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QMap<int, QVariant> NotesModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    if (index.isValid() && index.row() < rowCount()) {
        QVariantMap note = this->note(idOfNoteByINdex(index.row()));
        for (int role = IdRole; role <= ErrorMessageRole; ++role) {
            map.insert(role, note.value(m_roleNames[role]));
        }
    }
    return map;
}

bool NotesModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) {
    QVariantMap note;
    if (index.isValid() && index.row() < rowCount()) {
        note = this->note(idOfNoteByINdex(index.row()));
        QMapIterator<int, QVariant> i(roles);
        while (i.hasNext()) {
            i.next();
            note.insert(m_roleNames[i.key()], i.value());
        }
        return setNote(note, idOfNoteByINdex(index.row()));
    }
    return  false;
}
