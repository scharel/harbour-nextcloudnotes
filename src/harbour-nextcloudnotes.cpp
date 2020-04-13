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

    QObject::connect(notesApi, SIGNAL(noteUpdated(int, QJsonObject)), notesStore, SLOT(updateNote(int, QJsonObject)));
    //QObject::connect(notesStore, SIGNAL(noteUpdated(Note)), notesApi, SLOT(updateNote(Note)));
    QObject::connect(notesApi, SIGNAL(noteDeleted(int)), notesStore, SLOT(deleteNote(int)));
    //QObject::connect(notesStore, SIGNAL(noteDeleted(int)), notesApi, SLOT(deleteNote(int)));

    QObject::connect(notesStore, SIGNAL(noteUpdated(int, QJsonObject)), notesModel, SLOT(insertNote(int, QJsonObject)));
    QObject::connect(notesStore, SIGNAL(noteDeleted(int)), notesModel, SLOT(removeNote(int)));
    //QObject::connect(notesApi, SIGNAL(noteUpdated(Note)), notesModel, SLOT(insertNote(Note)));
    //QObject::connect(notesApi, SIGNAL(noteDeleted(int)), notesModel, SLOT(removeNote(int)));

    QQuickView* view = SailfishApp::createView();
#ifdef QT_DEBUG
    view->rootContext()->setContextProperty("debug", QVariant(true));
#else
    view->rootContext()->setContextProperty("debug", QVariant(false));
#endif
    view->rootContext()->setContextProperty("notesModel", notesProxyModel);
    view->rootContext()->setContextProperty("notesStore", notesStore);
    view->rootContext()->setContextProperty("notesApi", notesApi);

    view->setSource(SailfishApp::pathTo("qml/harbour-nextcloudnotes.qml"));
    view->show();

    int retval = app->exec();
    //QObject::disconnect(notesApi, SIGNAL(noteDeleted(int)), notesModel, SLOT(removeNote(int)));
    //QObject::disconnect(notesApi, SIGNAL(noteUpdated(Note)), notesModel, SLOT(insertNote(Note)));
    QObject::disconnect(notesStore, SIGNAL(noteDeleted(int)), notesModel, SLOT(removeNote(int)));
    QObject::disconnect(notesStore, SIGNAL(noteUpdated(int, QJsonObject)), notesModel, SLOT(insertNote(int, QJsonObject)));

    //QObject::disconnect(notesStore, SIGNAL(noteDeleted(int)), notesApi, SLOT(deleteNote(int)));
    QObject::disconnect(notesApi, SIGNAL(noteDeleted(int)), notesStore, SLOT(deleteNote(int)));
    //QObject::disconnect(notesStore, SIGNAL(noteUpdated(Note)), notesApi, SLOT(updateNote(Note)));
    QObject::disconnect(notesApi, SIGNAL(noteUpdated(int, QJsonObject)), notesStore, SLOT(updateNote(int, QJsonObject)));

    notesApi->deleteLater();
    notesStore->deleteLater();
    notesProxyModel->deleteLater();
    notesModel->deleteLater();
    return retval;
}
