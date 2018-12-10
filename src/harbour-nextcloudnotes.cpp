#include <QtQuick>
#include <sailfishapp.h>

int main(int argc, char *argv[])
{
    QGuiApplication* app = SailfishApp::application(argc, argv);
    app->setApplicationDisplayName("Nextcloud Notes");
    app->setApplicationName("harbour-nextcloudnotes");
    app->setApplicationVersion(APP_VERSION);
    app->setOrganizationDomain("https://github.com/scharel");
    app->setOrganizationName("harbour-nextcloudnotes");

    qDebug() << app->applicationDisplayName() << app->applicationVersion();

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
