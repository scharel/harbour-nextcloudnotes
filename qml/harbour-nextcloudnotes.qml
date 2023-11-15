import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import "pages"

ApplicationWindow
{
    id: appWindow

    property string currentNoteTitle: qsTr("Notes")

    ConfigurationValue {
        id: accounts
        key: appSettings.path + "/accountIDs"
        defaultValue: [ ]
    }

    ConfigurationGroup {
        id: appSettings
        path: "/apps/harbour-nextcloudnotes/settings"
        //synchronous: true

        property string currentAccount: value("currentAccount", "")
        property var accountIDs: value("accountIDs", [ ])
        property int autoSyncInterval: value("autoSyncInterval", 0)
        property int previewLineCount: value("previewLineCount", 4)
        property string sortBy: value("sortBy", "date")
        property bool showSeparator: value("showSeparator", false)
        property bool titleInCover: value("titleInCover", true)
        property bool useMonoFont: value("useMonoFont", false)
        property bool useCapitalX: value("useCapitalX", false)

        onCurrentAccountChanged: api.uuid = currentAccount

        function addAccount() {
            var uuid = uuidv4()
            var tmpIDs = accounts.value
            tmpIDs.push(uuid)
            accounts.value = tmpIDs
            accounts.sync()
            return uuid
        }
        function removeAccount(uuid) {
            autoSyncTimer.stop()
            var tmpAccouunt = currentAccount
            currentAccount = uuid
            api.clear()
            currentAccount = tmpAccouunt
            var newIds = [ ]
            accountIDs.forEach(function(currentValue) {
                if (currentValue !== uuid) {
                    newIds.push(currentValue)
                }
            })
            accounts.value = newIds
            for (var i = accountIDs.length-1; i > 0; i--) {
                if (accountIDs[i] !== uuid) {
                    api.uuid = accountIDs[i]
                    break
                }
            }
            autoSyncTimer.start()
        }
        function uuidv4() {
            return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
                var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
                return v.toString(16);
            });
        }
    }

    Timer {
        id: autoSyncTimer
        interval: appSettings.autoSyncInterval * 1000
        repeat: true
        running: interval > 0 && appWindow.visible
        triggeredOnStart: true
        onTriggered: {
            if (!api.busy) {
                api.getNotes()
            }
            else {
                triggeredOnStart = false
                restart()
                triggeredOnStart = true
            }
        }
        onIntervalChanged: console.log("Auto-Sync every " + interval / 1000 + " seconds")
    }

    NotesApi {
        id: api
        uuid: appSettings.currentAccount
    }

    /*ConfigurationValue {
        id: nextcloudUUIDs
        key: "/apps/harbour-nextcloudnotes/settings/accountIDs"
        defaultValue: []
        onValueChanged: {
            nextcloudAccounts.model = value
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
        function remove(uuid) {
            for (var i = 0; i < count; i++) {
                if (itemAt(i).uuid === uuid) {
                    itemAt(i).clear()
                }
            }
            var newIds = [ ]
            nextcloudUUIDs.value.forEach(function(currentValue) {
                if (currentValue !== uuid) {
                    newIds.push(currentValue)
                }
            })
            nextcloudUUIDs.value = newIds
            if (nextcloudUUIDs.value[appSettings.currentAccount] === uuid)
                appSettings.currentAccount = nextcloudUUIDs.value.length-1
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
    }*/

    initialPage: Component { NotesPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
