#include "note.h"

const QMap<Note::NoteField, QString> Note::m_noteFieldNames = QMap<Note::NoteField, QString>( {
    {Note::Id, "id"},
    {Note::Modified, "modified"},
    {Note::Title, "title"},
    {Note::Category, "category"},
    {Note::Content, "content"},
    {Note::Favorite, "favorite"},
    {Note::Etag, "etag"},
    {Note::Error, "error"},
    {Note::ErrorMessage, "errorMessage"} } );

Note::Note(QObject *parent) : QObject(parent) {
    connectSignals();
}

Note::~Note() {
    //qDebug() << "Note destroyed: " << id();
}

Note::Note(const Note& note, QObject *parent) : QObject(parent) {
    setId(note.id());
    setModified(note.modified());
    setTitle(note.title());
    setCategory(note.category());
    setContent(note.content());
    setFavorite(note.favorite());
    setEtag(note.etag());
    setError(note.error());
    setErrorMessage(note.errorMessage());
    connectSignals();
}

Note::Note(const QJsonObject &note, QObject *parent) : QObject(parent) {
    setId(id(note));
    setModified(modified(note));
    setTitle(title(note));
    setCategory(category(note));
    setContent(content(note));
    setFavorite(favorite(note));
    setEtag(etag(note));
    setError(error(note));
    setErrorMessage(errorMessage(note));
    connectSignals();
}

Note& Note::operator =(const Note& note) {
    setId(note.id());
    setModified(note.modified());
    setTitle(note.title());
    setCategory(note.category());
    setContent(note.content());
    setFavorite(note.favorite());
    setEtag(note.etag());
    setError(note.error());
    setErrorMessage(note.errorMessage());
    return *this;
}
Note& Note::operator =(const QJsonObject& note) {
    setId(id(note));
    setModified(modified(note));
    setTitle(title(note));
    setCategory(category(note));
    setContent(content(note));
    setFavorite(favorite(note));
    setEtag(etag(note));
    setError(error(note));
    setErrorMessage(errorMessage(note));
    return *this;
}

bool Note::operator ==(const Note& note) const {
    return id() == note.id();
}
bool Note::operator ==(const QJsonObject& note) const {
    return id() == id(note);
}

bool Note::equal(const Note &note) const {
    return id() == note.id() &&
            modified() == note.modified() &&
            title() == note.title() &&
            category() == note.category() &&
            content() == note.content() &&
            favorite() == note.favorite() &&
            etag() == note.etag() &&
            error() == note.error() &&
            errorMessage() == note.errorMessage();
}
bool Note::equal(const QJsonObject &note) const {
    return id() == id(note) &&
            modified() == modified(note) &&
            title() == title(note) &&
            category() == category(note) &&
            content() == content(note) &&
            favorite() == favorite(note) &&
            etag() == etag(note) &&
            error() == error(note) &&
            errorMessage() == errorMessage(note);
}

bool Note::operator <(const Note& note) const {
    return modified() < note.modified();
}
bool Note::operator <(const QJsonObject& note) const {
    return modified() < modified(note);
}

bool Note::operator >(const Note& note) const {
    return modified() > note.modified();
}
bool Note::operator >(const QJsonObject& note) const {
    return modified() > modified(note);
}

const QJsonObject Note::toJsonObject() const {
    return m_json;
}

const QJsonValue Note::toJsonValue() const {
    return QJsonValue(m_json);
}

const QJsonDocument Note::toJsonDocument() const {
    return QJsonDocument(m_json);
}

int Note::id() const {
    return m_json.value(noteFieldName(Id)).toInt(-1);
}
int Note::id(const QJsonObject &jobj) {
    return jobj.value(noteFieldName(Id)).toInt(-1);
}
void Note::setId(int id) {
    if (id >= 0) {
        if (id != this->id()) {
            m_json.insert(noteFieldName(Id), QJsonValue(id));
            emit idChanged(this->id());
        }
    }
    else {
        m_json.remove(noteFieldName(Id));
        emit idChanged(this->id());
    }
}

int Note::modified() const {
    return m_json.value(noteFieldName(Modified)).toInt();
}
int Note::modified(const QJsonObject &jobj) {
    return jobj.value(noteFieldName(Modified)).toInt();
}
void Note::setModified(int modified) {
    if (modified > 0) {
        if (modified != this->modified()) {
            m_json.insert(noteFieldName(Modified), QJsonValue(modified));
            emit modifiedChanged(this->modified());
            emit modifiedStringChanged(modifiedString());
            emit modifiedDateTimeChanged(modifiedDateTime());
        }
    }
    else {
        m_json.remove(noteFieldName(Modified));
        emit modifiedChanged(this->modified());
        emit modifiedStringChanged(modifiedString());
        emit modifiedDateTimeChanged(modifiedDateTime());
    }
}

QString Note::title() const {
    return m_json.value(noteFieldName(Title)).toString();
}
QString Note::title(const QJsonObject &jobj) {
    return jobj.value(noteFieldName(Title)).toString();
}
void Note::setTitle(QString title) {
    if (!title.isNull()) {
        if (title != this->title()) {
            m_json.insert(noteFieldName(Title), QJsonValue(title));
            emit titleChanged(this->title());
        }
    }
    else {
        m_json.remove(noteFieldName(Title));
        emit titleChanged(this->title());
    }
}

QString Note::category() const {
    return m_json.value(noteFieldName(Category)).toString();
}
QString Note::category(const QJsonObject &jobj) {
    return jobj.value(noteFieldName(Category)).toString();
}
void Note::setCategory(QString category) {
    if (!category.isNull()) {
        if (category != this->category()) {
            m_json.insert(noteFieldName(Category), QJsonValue(category));
            emit categoryChanged(this->category());
        }
    }
    else {
        m_json.remove(noteFieldName(Category));
        emit categoryChanged(this->category());
    }
}

QString Note::content() const {
    return m_json.value(noteFieldName(Content)).toString();
}
QString Note::content(const QJsonObject &jobj) {
    return jobj.value(noteFieldName(Content)).toString();
}
void Note::setContent(QString content) {
    if (!content.isNull()) {
        if (content != this->content()) {
            m_json.insert(noteFieldName(Content), QJsonValue(content));
            emit contentChanged(this->content());
        }
    }
    else {
        m_json.remove(noteFieldName(Content));
        emit contentChanged(this->content());
    }
}

bool Note::favorite() const {
    return m_json.value(noteFieldName(Favorite)).toBool();
}
bool Note::favorite(const QJsonObject &jobj) {
    return jobj.value(noteFieldName(Favorite)).toBool();
}
void Note::setFavorite(bool favorite) {
    if (favorite != this->favorite()) {
        m_json.insert(noteFieldName(Favorite), QJsonValue(favorite));
        emit favoriteChanged(this->favorite());
    }
}

QString Note::etag() const {
    return m_json.value(noteFieldName(Etag)).toString();
}
QString Note::etag(const QJsonObject &jobj) {
    return jobj.value(noteFieldName(Etag)).toString();
}
void Note::setEtag(QString etag) {
    if (!etag.isNull()) {
        if (etag != this->etag()) {
            m_json.insert(noteFieldName(Etag), QJsonValue(etag));
            emit etagChanged(this->etag());
        }
    }
    else {
        m_json.remove(noteFieldName(Etag));
        emit etagChanged(this->etag());
    }
}

bool Note::error() const {
    return m_json.value(noteFieldName(Error)).toBool();
}
bool Note::error(const QJsonObject &jobj) {
    return jobj.value(noteFieldName(Error)).toBool();
}
void Note::setError(bool error) {
    if (error) {
        if (error != this->error()) {
            m_json.insert(noteFieldName(Error), QJsonValue(error));
            emit errorChanged(this->error());
        }
    }
    else {
        m_json.remove(noteFieldName(Error));
        emit errorChanged(this->error());
    }
}

QString Note::errorMessage() const {
    return m_json.value(noteFieldName(ErrorMessage)).toString();
}
QString Note::errorMessage(const QJsonObject &jobj) {
    return jobj.value(noteFieldName(ErrorMessage)).toString();
}
void Note::setErrorMessage(QString errorMessage) {
    if (!errorMessage.isNull()) {
        if (errorMessage != this->errorMessage()) {
            m_json.insert(noteFieldName(ErrorMessage), QJsonValue(errorMessage));
            emit errorMessageChanged(this->errorMessage());
        }
    }
    else {
        m_json.remove(noteFieldName(ErrorMessage));
        emit errorMessageChanged(this->errorMessage());
    }
}

QString Note::modifiedString() const {
    return modifiedString(m_json);
}
QString Note::modifiedString(const QJsonObject &jobj) {
    QDateTime date = modifiedDateTime(jobj);
    QString dateString;
    qint64 diff = date.daysTo(QDateTime::currentDateTime());
    if (diff == 0)
        dateString = tr("Today");
    else if (diff == 1)
        dateString = tr("Yesterday");
    else if (diff < 7)
        dateString = date.toLocalTime().toString("dddd");
    else if (date.toLocalTime().toString("yyyy") == QDateTime::currentDateTime().toString("yyyy"))
        dateString = date.toLocalTime().toString("MMMM");
    else if (!date.isValid())
        dateString = date.toLocalTime().toString("MMMM yyyy");
    return dateString;
}

QDateTime Note::modifiedDateTime() const {
    return modifiedDateTime(m_json);
}
QDateTime Note::modifiedDateTime(const QJsonObject &jobj) {
    QDateTime date;
    date.setTime_t(modified(jobj));
    return date;
}

void Note::connectSignals() {
    connect(this, SIGNAL(idChanged(int)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(modifiedChanged(int)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(titleChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(categoryChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(contentChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(favoriteChanged(bool)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(etagChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(errorChanged(bool)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(errorMessageChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(modifiedStringChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(modifiedDateTimeChanged(QDateTime)), this, SIGNAL(noteChanged()));
}
