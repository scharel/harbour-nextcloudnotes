#include "note.h"

Note::Note(QObject *parent) : QObject(parent) {
    m_id = -1;
    m_modified = 0;
    m_error = true;
    connect(this, SIGNAL(idChanged(int)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(modifiedChanged(uint)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(titleChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(categoryChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(contentChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(favoriteChanged(bool)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(etagChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(errorChanged(bool)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(errorMessageChanged(QString)), this, SIGNAL(noteChanged()));
    connect(this, SIGNAL(prettyDateChanged(QString)), this, SIGNAL(noteChanged()));
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

Note& Note::operator=(const Note& note) {
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

bool Note::operator==(const Note& note) const {
    return m_id == note.id() &&
            m_modified == note.modified() &&
            m_title == note.title() &&
            m_category == note.category() &&
            m_content == note.content() &&
            m_favorite == note.favorite() &&
            m_etag == note.etag() &&
            m_error == note.error() &&
            m_errorMessage == note.errorMessage();
}

bool Note::same(const Note &note) const {
    return m_id == note.id();
}

QString Note::prettyDate() const {
    QDateTime date;
    QString dateString;
    date.setTime_t(m_modified);
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

Note Note::fromjson(const QJsonObject& jobj) {
    Note note = new Note;
    note.setId(jobj.value("id").toInt());
    note.setModified(jobj.value("modified").toInt());
    note.setTitle(jobj.value("title").toString());
    note.setCategory(jobj.value("category").toString());
    note.setContent(jobj.value("content").toString());
    note.setFavorite(jobj.value("favorite").toBool());
    note.setEtag(jobj.value("etag").toString());
    note.setError(jobj.value("error").toBool(true));
    note.setErrorMessage(jobj.value("errorMessage").toString());
    return note;
}
