#include <QtQuick>
#include <sailfishapp.h>
#include <QtQml>
#include <QObject>
#include "accounthash.h"
#include "nextcloudapi.h"
#include "apps/abstractnextcloudapp.h"
#include "apps/notes/notesapp.h"

int main(int argc, char *argv[])
{
    QGuiApplication* app = SailfishApp::application(argc, argv);
    app->setApplicationDisplayName("Nextcloud Notes");
    app->setApplicationName("harbour-nextcloudnotes");
    app->setApplicationVersion(APP_VERSION);
    app->setOrganizationDomain("https://github.com/scharel");
    app->setOrganizationName("harbour-nextcloudnotes");

    qDebug() << app->applicationDisplayName() << app->applicationVersion();

    AccountHash* accountHash = new AccountHash;
    NextcloudApi::instantiate(app);
    qmlRegisterSingletonType<NextcloudApi>("harbour.nextcloudapi", 1, 0, "Nextcloud", NextcloudApi::provider);
    NotesApp::instantiate(&NextcloudApi::getInstance(), &NextcloudApi::getInstance());
    qmlRegisterSingletonType<NotesApp>("harbour.nextcloudapi.notes", 1, 0, "Notes", NotesApp::provider);
    qmlRegisterType<NotesProxyModel>("harbour.nextcloudapi.notes", 1, 0, "NotesModel");
    //qmlRegisterType<NextcloudApi>("NextcloudApi", 1, 0, "Nextcloud");
    //qmlRegisterType<NotesApp>("NextcloudApi", 1, 0, "Notes");

    QQuickView* view = SailfishApp::createView();
#ifdef QT_DEBUG
    view->rootContext()->setContextProperty("debug", QVariant(true));
#else
    view->rootContext()->setContextProperty("debug", QVariant(false));
#endif
    view->rootContext()->setContextProperty("accountHash", accountHash);

    view->setSource(SailfishApp::pathTo("qml/harbour-nextcloudnotes.qml"));
    view->show();

    int retval = app->exec();

    return retval;
}
