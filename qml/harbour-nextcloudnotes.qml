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
        property url server: "https://cloud.scharel.name" + "/index.php/apps/notes/api/v0.2/notes"
        property string username: "test"
        property string password: "9J4HQ-aq7b3-69fCY-4HpPj-69MPj" // TODO provide password before testing
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
