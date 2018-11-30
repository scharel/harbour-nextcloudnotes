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
        //synchronous: true

        property int currentAccount: value("currentAccount", -1)
        property int autoSyncInterval: value("autoSyncInterval", 0)
        property int previewLineCount: value("previewLineCount", 4)
        property string sortBy: value("sortBy", "date")
        property bool showSeparator: value("showSeparator", false)
    }

    ConfigurationValue {
        id: nextcloudUUIDs
        key: "/apps/harbour-nextcloudnotes/settings/accountIDs"
        defaultValue: []
        onValueChanged: {
            nextcloudAccounts.model = value
            console.log("IDs changed: " + value)
            for (var i = 0; i < value.length; i++) {
                console.log("Account " + i + i === appSettings.currentAccount ? " (current)" : "")
                console.log(" - " + nextcloudAccounts.itemAt(i).name)
                console.log(" - " + nextcloudAccounts.itemAt(i).username + "@" + nextcloudAccounts.itemAt(i).server)
            }
        }
        Component.onCompleted: nextcloudAccounts.model = value
    }

    Repeater {
        id: nextcloudAccounts
        delegate: NotesApi {
            uuid: nextcloudUUIDs.value[index]
            saveFile: true
        }
        function add() {
            push(uuidv4())
        }
        function push(uuid) {
            var accountIDs = nextcloudUUIDs.value
            accountIDs.push(uuid)
            nextcloudUUIDs.value = accountIDs
        }
        function pop() {
            var accountIDs = nextcloudUUIDs.value
            accountIDs.pop()
            nextcloudUUIDs.value = accountIDs
        }
    }

    function uuidv4() {
        return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
            var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
            return v.toString(16);
        });
    }

    initialPage: Component { NotesPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
