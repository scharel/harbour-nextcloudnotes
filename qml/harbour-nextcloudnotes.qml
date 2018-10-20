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
        property url server: ""
        property string username: ""
        property string password: ""
        property bool unsecureConnection: false
        // For testing
        Component.onCompleted: {
            //server = ""
            //username = ""
            //password = ""
            console.log("Server: " + server)
            console.log("Username: " + username)
            console.log("Password: " + password)
        }
    }

    property var notes: NotesApi {
        name: "notes"
        saveFile: false
    }
    Connections {
        target: notes
        onLastUpdateChanged: appSettings.lastUpdate = notes.lastUpdate
    }

    initialPage: Component { NotesPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
