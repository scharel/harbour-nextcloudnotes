#include <QtQuick>
#include <sailfishapp.h>
#include <QtQml>
#include <QObject>
#include "accounthash.h"
#include "nextcloudapi.h"
#include "apps/notes/notesmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication* app = SailfishApp::application(argc, argv);
    app->setApplicationDisplayName("Nextcloud Notes");
    app->setApplicationName("harbour-nextcloudnotes");
    app->setApplicationVersion(APP_VERSION);
    app->setOrganizationDomain("https://github.com/scharel");
    app->setOrganizationName("harbour-nextcloudnotes");

    qDebug() << app->applicationDisplayName() << app->applicationVersion();

    NotesProxyModel* notesProxyModel = new NotesProxyModel;
    AccountHash* accountHash = new AccountHash;
    qmlRegisterType<NextcloudApi>("NextcloudApi", 1, 0, "Nextcloud");

    QQuickView* view = SailfishApp::createView();
#ifdef QT_DEBUG
    view->rootContext()->setContextProperty("debug", QVariant(true));
#else
    view->rootContext()->setContextProperty("debug", QVariant(false));
#endif
    view->rootContext()->setContextProperty("notesModel", notesProxyModel);
    view->rootContext()->setContextProperty("accountHash", accountHash);

    view->setSource(SailfishApp::pathTo("qml/harbour-nextcloudnotes.qml"));
    view->show();

    int retval = app->exec();

    return retval;
}
