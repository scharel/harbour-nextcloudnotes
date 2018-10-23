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
        property var accounts: [ ] // FIXME
        property int currentAccount: 0 // FIXME
        // For testing
        Component.onCompleted: {
            appSettings.clear()
            //accounts[0] = { server: "127.0.0.1", username: "fu", password: "bar", lastUpdate: new Date(0) }
            //accounts[1] = { server: "127.0.0.2", username: "fu", password: "bar", lastUpdate: new Date(0) }
            //accounts[2] = { server: "127.0.0.3", username: "fu", password: "bar", lastUpdate: new Date(0) }
            console.log("Configured accounts: " + accounts.length)
            for(var i=0; i<accounts.length; i++) {
                console.log("Account " + i + (i === currentAccount ? " (active):" : ":"))
                console.log("- Server: " + accounts[i].server)
                console.log("- Username: " + accounts[i].username)
                console.log("- Password: " + accounts[i].password)
            }
            if (typeof(accounts[currentAccount]) !== 'undefined') {
                notes.account = appSettings.accounts[appSettings.currentAccount]
            }
            else {
                currentAccount = 0
                notes.account = appSettings.accounts[0]
            }
        }
    }

    property var notes: NotesApi {
        name: "notes"
        //account: appSettings.accounts[appSettings.currentAccount]
        saveFile: false
    }
    Connections {
        target: notes
        //onLastUpdateChanged: appSettings.lastUpdate = notes.lastUpdate
    }

    initialPage: Component { NotesPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
