#include <QtQuick>
#include <sailfishapp.h>
#include <QtQml>
#include <QObject>
#include "notesapi.h"
#include "note.h"
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
    qmlRegisterType<NotesApi>("harbour.nextcloudnotes.notesapi", 1, 0, "NotesApi");
    qmlRegisterType<Note>("harbour.nextcloudnotes.note", 1, 0, "Note");
    qmlRegisterType<NotesModel>("harbour.nextcloudnotes.notesmodel", 1, 0, "NotesModel");

    QQuickView* view = SailfishApp::createView();

    view->setSource(SailfishApp::pathTo("qml/harbour-nextcloudnotes.qml"));
#ifdef QT_DEBUG
    view->rootContext()->setContextProperty("debug", QVariant(true));
#else
    view->rootContext()->setContextProperty("debug", QVariant(false));
#endif
    view->show();

    return app->exec();
}
