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
TARGET = harbour-nextcloud-notes

CONFIG += sailfishapp

SOURCES += src/harbour-nextcloud-notes.cpp

DISTFILES += qml/harbour-nextcloud-notes.qml \
    qml/cover/CoverPage.qml \
    qml/pages/FirstPage.qml \
    rpm/harbour-nextcloud-notes.changes.in \
    rpm/harbour-nextcloud-notes.changes.run.in \
    rpm/harbour-nextcloud-notes.spec \
    rpm/harbour-nextcloud-notes.yaml \
    translations/*.ts \
    harbour-nextcloud-notes.desktop \
    qml/pages/NotePage.qml

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-nextcloud-notes-de.ts

HEADERS +=
