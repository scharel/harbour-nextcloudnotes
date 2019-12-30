#include "note.h"

Note::Note(QObject *parent) : QObject(parent) {
    connect(this, SIGNAL(idChanged(double)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(modifiedChanged(double)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(titleChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(categoryChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(contentChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(favoriteChanged(bool)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(etagChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(errorChanged(bool)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(errorMessageChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(prettyDateChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(dateTimeChanged(QDateTime)), this, SIGNAL(noteChanged()));
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
}

Note::Note(const QJsonObject &note, QObject *parent) {
    setId(id(note));
    setModified(modified(note));
    setTitle(title(note));
    setCategory(category(note));
    setContent(content(note));
    setFavorite(favorite(note));
    setEtag(etag(note));
    setError(error(note));
    setErrorMessage(errorMessage(note));
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

QJsonObject Note::toJsonObject() const {
    return m_json;
}

QJsonValue Note::toJsonValue() const {
    return QJsonValue(m_json);
}

QJsonDocument Note::toJsonDocument() const {
    return QJsonDocument(m_json);
}

double Note::id() const {
    return m_json.value(ID).toDouble(-1);
}
double Note::id(const QJsonObject &jobj) {
    return jobj.value(ID).toDouble(-1);
}
void Note::setId(double id) {
    if (id != this->id()) {
        m_json.insert(ID, QJsonValue(id));
        emit idChanged(this->id());
    }
}

double Note::modified() const {
    return m_json.value(MODIFIED).toDouble();
}
double Note::modified(const QJsonObject &jobj) {
    return jobj.value(MODIFIED).toDouble();
}
void Note::setModified(double modified) {
    if (modified != this->modified()){
        m_json.insert(MODIFIED, QJsonValue(modified));
        emit modifiedChanged(this->modified());
        emit modifiedStringChanged(modifiedString());
        emit modifiedDateTimeChanged(modifiedDateTime());
    }
}

QString Note::title() const {
    return m_json.value(TITLE).toString();
}
QString Note::title(const QJsonObject &jobj) {
    return jobj.value(TITLE).toString();
}
void Note::setTitle(QString title) {
    if (title != this->title()) {
        m_json.insert(TITLE, QJsonValue(title));
        emit titleChanged(this->title());
    }
}

QString Note::category() const {
    return m_json.value(CATEGORY).toString();
}
QString Note::category(const QJsonObject &jobj) {
    return jobj.value(CATEGORY).toString();
}
void Note::setCategory(QString category) {
    if (category != this->category()) {
        m_json.insert(CATEGORY, QJsonValue(category));
        emit categoryChanged(this->category());
    }
}

QString Note::content() const {
    return m_json.value(CONTENT).toString();
}
QString Note::content(const QJsonObject &jobj) {
    return jobj.value(CONTENT).toString();
}
void Note::setContent(QString content) {
    if (content != this->content()) {
        m_json.insert(CONTENT, QJsonValue(content));
        emit contentChanged(this->content());
    }
}

bool Note::favorite() const {
    return m_json.value(FAVORITE).toBool();
}
bool Note::favorite(const QJsonObject &jobj) {
    return jobj.value(FAVORITE).toBool();
}
void Note::setFavorite(bool favorite) {
    if (favorite != this->favorite()) {
        m_json.insert(FAVORITE, QJsonValue(favorite));
        emit favoriteChanged(this->favorite());
    }
}

QString Note::etag() const {
    return m_json.value(ETAG).toString();
}
QString Note::etag(const QJsonObject &jobj) {
    return jobj.value(ETAG).toString();
}
void Note::setEtag(QString etag) {
    if (etag != this->etag()) {
        m_json.insert(ETAG, QJsonValue(etag));
        emit etagChanged(this->etag());
    }
}

bool Note::error() const {
    return m_json.value(ERROR).toBool(true);
}
bool Note::error(const QJsonObject &jobj) {
    return jobj.value(ERROR).toBool(true);
}
void Note::setError(bool error) {
    if (error != this->error()) {
        m_json.insert(ERROR, QJsonValue(error));
        emit errorChanged(this->error());
    }
}

QString Note::errorMessage() const {
    return m_json.value(ERRORMESSAGE).toString();
}
QString Note::errorMessage(const QJsonObject &jobj) {
    return jobj.value(ERRORMESSAGE).toString();
}
void Note::setErrorMessage(QString errorMessage) {
    if (errorMessage != this->errorMessage()) {
        m_json.insert(ERRORMESSAGE, QJsonValue(errorMessage));
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
    else
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
