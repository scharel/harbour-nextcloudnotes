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
        property var accounts: value("accounts", [])
        property int currentAccount: 0
        // For testing
        Component.onCompleted: {
            //appSettings.clear()
            for(var i=0; i<accounts.length; i++) {
                console.log("Server: " + accounts[i].server)
                console.log("Username: " + accounts[i].username)
                console.log("Password: " + accounts[i].password)
            }
            //notes.account = appSettings.accounts[appSettings.currentAccount]
        }
    }

    property var notes: NotesApi {
        name: "notes"
        account: appSettings.accounts[appSettings.currentAccount]
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
