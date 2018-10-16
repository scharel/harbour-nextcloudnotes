import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import "pages"

ApplicationWindow
{
    id: appWindow

    ConfigurationGroup {
        id: appSettings
        path: "/apps/harbour-nextcloud-notes/settings"
        property string lastUpdate: qsTr("never")
        property url nextcloudServer: "https://cloud.scharel.name"
        property string username: "scharel"
        property string password: ""
    }

    property var notesModel: JSONListModel {
        url: appSettings.nextcloudServer + "/index.php/apps/notes/api/v0.2/notes"
        name: "notes"
        saveFile: false
    }
    Connections {
        target: notesModel
        onLastUpdateChanged: appSettings.lastUpdate = notesModel.lastUpdate
    }

    initialPage: Component { FirstPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
