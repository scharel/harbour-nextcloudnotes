#include "note.h"

Note::Note(QObject *parent) : QObject(parent) {
    m_id = -1;
    m_modified = 0;
    m_error = true;
}

Note::Note(const Note& note, QObject *parent) : QObject(parent) {
    m_id = note.id();
    m_modified = note.modified();
    m_title = note.title();
    m_category = note.category();
    m_content = note.content();
    m_favorite = note.favorite();
    m_etag = note.etag();
    m_error = note.error();
    m_errorMessage = note.errorMessage();
}

Note& Note::operator=(const Note& note) {
    m_id = note.id();
    m_modified = note.modified();
    m_title = note.title();
    m_category = note.category();
    m_content = note.content();
    m_favorite = note.favorite();
    m_etag = note.etag();
    m_error = note.error();
    m_errorMessage = note.errorMessage();
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
