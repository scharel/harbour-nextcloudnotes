# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-nextcloudnotes

CONFIG += sailfishapp

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

HEADERS += src/note.h \
    src/accounthash.h \
    src/nextcloudapi.h \
    src/notesapi.h \
    src/notesmodel.h

SOURCES += src/harbour-nextcloudnotes.cpp \
    src/nextcloudapi.cpp \
    src/note.cpp \
    src/notesapi.cpp \
    src/notesmodel.cpp

DISTFILES += qml/harbour-nextcloudnotes.qml \
    qml/cover/CoverPage.qml \
    qml/img/nextcloud-logo-dark.png \
    qml/img/nextcloud-logo-light.png \
    qml/img/icon-launcher-nextcloudnotes.svg \
    qml/pages/LoginPage.qml \
    rpm/harbour-nextcloudnotes.changes.run.in \
    rpm/harbour-nextcloudnotes.changes \
    rpm/harbour-nextcloudnotes.spec \
    rpm/harbour-nextcloudnotes.yaml \
    translations/*.ts \
    harbour-nextcloudnotes.desktop \
    qml/pages/NotePage.qml \
    qml/pages/NotesPage.qml \
    qml/pages/EditPage.qml \
    qml/pages/SettingsPage.qml \
    qml/pages/AboutPage.qml \
    qml/pages/UnencryptedDialog.qml \
    qml/pages/NotesApi.qml \
    qml/pages/MITLicense.qml \
    qml/pages/GPLLicense.qml \
    qml/pages/SyntaxPage.qml

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-nextcloudnotes-de.ts \
    translations/harbour-nextcloudnotes-sv.ts
