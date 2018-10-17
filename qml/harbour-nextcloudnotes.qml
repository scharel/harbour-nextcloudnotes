import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import "pages"

ApplicationWindow
{
    id: appWindow

    ConfigurationGroup {
        id: appSettings
        path: "/apps/harbour-nextcloudnotes/settings"
        property string lastUpdate: qsTr("never")
        property url server: "https://127.0.0.1" + "/index.php/apps/notes/api/v0.2/notes"
        property string username: "test"
        property string password // TODO provide password before testing. Just use revocable passwords for testing!
    }

    property var notes: NotesApi {
        name: "notes"
        saveFile: false
    }
    Connections {
        target: notes
        onLastUpdateChanged: appSettings.lastUpdate = notes.lastUpdate
    }

    initialPage: Component { FirstPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
