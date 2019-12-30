import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import harbour.nextcloudnotes.notesapi 1.0
import "pages"

ApplicationWindow
{
    id: appWindow
    /*
    ConfigurationValue {
        id: accounts
        key: appSettings.path + "/accountIDs"
        defaultValue: [ ]
    }
    */
    ConfigurationGroup {
        id: account
        path: "/apps/harbour-nextcloudnotes/accounts/" + appSettings.currentAccount

        property string name: value("name", "", String)
        property url server: value("server", "", String)
        property string version: value("version", "v0.2", String)
        property string username: value("username", "", String)
        property string password: account.value("password", "", String)
        property bool doNotVerifySsl: account.value("doNotVerifySsl", false, Boolean)
        property bool allowUnecrypted: account.value("allowUnecrypted", false, Boolean)
        property date update: value("update", "", Date)
        onValuesChanged: console.log("A property of the current account has changed")
        onNameChanged: console.log("Account: " + name)
    }

    ConfigurationGroup {
        id: appSettings
        path: "/apps/harbour-nextcloudnotes/settings"

        property bool initialized: false
        property string currentAccount: value("currentAccount", "", String)
        property var accountIDs: value("accountIDs", [ ], Array)
        property int autoSyncInterval: value("autoSyncInterval", 0, Number)
        property int previewLineCount: value("previewLineCount", 4, Number)
        property bool favoritesOnTop: value("favoritesOnTop", true, Boolean)
        property string sortBy: value("sortBy", "prettyDate", String)
        property bool showSeparator: value("showSeparator", false, Boolean)
        property bool useMonoFont: value("useMonoFont", false, Boolean)
        property bool useCapitalX: value("useCapitalX", false, Boolean)
        onCurrentAccountChanged: {
            account.path = "/apps/harbour-nextcloudnotes/accounts/" + currentAccount
            account.sync()
            if (initialized)
                notesApi.getAllNotes();
            autoSyncTimer.restart()
        }
        Component.onCompleted: initialized = true

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
            var confGroup = ConfigurationGroup
            confGroup.path = "/apps/harbour-nextcloudnotes/accounts/" + uuid
            for (var i = accountIDs.length-1; i >= 0; i--) {
                if (accountIDs[i] !== uuid && appSettings.currentAccount === uuid) {
                    appSettings.currentAccount = accountIDs[i]
                }
                else if (accountIDs[i] === uuid) {
                    accountIDs.splice(i, 1)
                }
            }
            if (appSettings.currentAccount === uuid) {
               appSettings.currentAccount = ""
            }
            confGroup.clear()
            if (autoSyncInterval > 0 && appWindow.visible) {
                autoSyncTimer.start()
            }
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
        running: interval > 0 && notesApi.networkAccessible && appWindow.visible
        triggeredOnStart: false
        onTriggered: {
            if (!notesApi.busy) {
                notesApi.getAllNotes();
            }
            else {
                restart()
            }
        }
        onIntervalChanged: {
            if (interval > 0) {
                console.log("Auto-Sync every " + interval / 1000 + " seconds")
            }
        }
    }

    NotesApi {
        id: notesApi
        scheme: account.allowUnecrypted ? "http" : "https"
        host: account.server
        path: "/index.php/apps/notes/api/" + account.version
        username: account.username
        password: account.password
        sslVerify: !account.doNotVerifySsl
        dataFile: appSettings.currentAccount !== "" ? StandardPaths.data + "/" + appSettings.currentAccount + ".json" : ""
        Component.onCompleted: getAllNotes()
    }

    initialPage: Component { NotesPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
