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
    connect(this, SIGNAL(dateStringChanged(QString)), this, SIGNAL(noteChanged()));
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

bool Note::equal(const Note& n) const {
    return m_id == n.id() &&
            m_modified == n.modified() &&
            m_title == n.title() &&
            m_category == n.category() &&
            m_content == n.content() &&
            m_favorite == n.favorite() &&
            m_etag == n.etag() &&
            m_error == n.error() &&
            m_errorMessage == n.errorMessage();
}
