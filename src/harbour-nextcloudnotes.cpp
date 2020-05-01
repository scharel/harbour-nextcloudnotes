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
    NotesModel* notesModel = new NotesModel;
    NotesProxyModel* notesProxyModel = new NotesProxyModel;
    notesProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    notesProxyModel->setSortLocaleAware(true);
    notesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    notesProxyModel->setFilterRole(NotesModel::ContentRole);
    notesProxyModel->setSourceModel(notesModel);

    NotesStore* notesStore = new NotesStore;
    NotesApi* notesApi = new NotesApi;
    notesModel->setNotesApi(notesApi);
    notesModel->setNotesStore(notesStore);

    QQuickView* view = SailfishApp::createView();
#ifdef QT_DEBUG
    view->rootContext()->setContextProperty("debug", QVariant(true));
#else
    view->rootContext()->setContextProperty("debug", QVariant(false));
#endif
    view->rootContext()->setContextProperty("notesModel", notesModel);
    view->rootContext()->setContextProperty("notesProxyModel", notesProxyModel);
    view->rootContext()->setContextProperty("notesStore", notesStore);
    view->rootContext()->setContextProperty("notesApi", notesApi);

    view->setSource(SailfishApp::pathTo("qml/harbour-nextcloudnotes.qml"));
    view->show();

    int retval = app->exec();

    notesApi->deleteLater();
    notesStore->deleteLater();
    notesProxyModel->deleteLater();
    notesModel->deleteLater();
    return retval;
}
