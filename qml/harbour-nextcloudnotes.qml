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

        property int currentAccount: value("currentAccount", -1)
        property var accountIDs: value("accountIDs", [])
        //Component.onCompleted: clear()
    }
    ConfigurationGroup {
        id: accounts
        path: "/apps/harbour-nextcloudnotes/accounts"

        ConfigurationGroup {
            id: account
            path: appSettings.accountIDs[appSettings.currentAccount]

            property string name
            property url server
            property string username
            property string password
            property date update
            property bool unsecureConnection: false
            property bool unencryptedConnection: false

            onPathChanged: {
                console.log(scope.path + "/" + path + ": " + name)
                console.log(name)
            }
        }

        function uuidv4() {
          return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
            var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
            return v.toString(16);
          });
        }

        function add() {
            var uuid = uuidv4()
            return uuid
        }

        //Component.onCompleted: clear()
    }

    NotesApi {
        id: notes
    }

    initialPage: Component { NotesPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
