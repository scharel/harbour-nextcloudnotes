#include <QtQuick>
#include <sailfishapp.h>
#include <QtQml>
#include <QObject>
#include "note.h"
#include "notesapi.h"
#include "notesstore.h"
#include "notesmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication* app = SailfishApp::application(argc, argv);
    app->setApplicationDisplayName("Nextcloud Notes");
    app->setApplicationName("harbour-nextcloudnotes");
    app->setApplicationVersion(APP_VERSION);
    app->setOrganizationDomain("https://github.com/scharel");
    app->setOrganizationName("harbour-nextcloudnotes");

    qDebug() << app->applicationDisplayName() << app->applicationVersion();

    qRegisterMetaType<Note>();
    qmlRegisterType<Note>("harbour.nextcloudnotes.note", 1, 0, "Note");
    qmlRegisterType<NotesApi>("harbour.nextcloudnotes.notesapi", 1, 0, "NotesApi");
    //qmlRegisterType<NotesStore>("harbour.nextcloudnotes.notesstore", 1, 0, "NotesStore");
    qmlRegisterType<NotesProxyModel>("harbour.nextcloudnotes.notesmodel", 1, 0, "NotesModel");

    NotesStore* notesStore = new NotesStore;

    QQuickView* view = SailfishApp::createView();
    view->rootContext()->setContextProperty("notesStore", notesStore);
    view->setSource(SailfishApp::pathTo("qml/harbour-nextcloudnotes.qml"));

#ifdef QT_DEBUG
    view->rootContext()->setContextProperty("debug", QVariant(true));
#else
    view->rootContext()->setContextProperty("debug", QVariant(false));
#endif
    view->show();

    return app->exec();
}
