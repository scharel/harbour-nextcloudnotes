import QtQuick 2.2
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import Nemo.Notifications 1.0
import NextcloudApi 1.0
import "pages"

ApplicationWindow
{
    id: appWindow

    // General settings of the app
    ConfigurationGroup {
        id: appSettings
        path: "/apps/harbour-nextcloudnotes"

        property bool initialized: false
        property var accounts: value("accounts", [], Array)
        property string currentAccount: value("currentAccount", "", String)
        property int autoSyncInterval: value("autoSyncInterval", 0, Number)
        property int previewLineCount: value("previewLineCount", 4, Number)
        property bool favoritesOnTop: value("favoritesOnTop", true, Boolean)
        property string sortBy: value("sortBy", "modifiedString", String)
        property bool showSeparator: value("showSeparator", false, Boolean)
        property bool useMonoFont: value("useMonoFont", false, Boolean)
        property bool useCapitalX: value("useCapitalX", false, Boolean)

        onCurrentAccountChanged: {
            console.log("Current account: " + currentAccount)
        }

        onSortByChanged: {
            if (sortBy == "none")
                notesProxyModel.invalidate()
            else
                notesProxyModel.sortRole = notesModel.roleFromName(sortBy)
        }
        onFavoritesOnTopChanged: {
            notesProxyModel.favoritesOnTop = favoritesOnTop
        }

        function createAccount(username, password, url, name) {
            var hash = accountHash.hash(username, url)
            var tmpaccounts = accounts
            tmpaccounts.push(hash)
            accounts = tmpaccounts

            tmpAccount.path = appSettings.path + "/accounts/" + hash
            tmpAccount.url = url
            tmpAccount.username = username
            tmpAccount.passowrd = password
            tmpAccount.name = name

            console.log("Hash(" + username + "@" + url + ") = " + hash)
            return hash
        }
        function removeAccount(hash) {
            notesApi.deleteAppPassword(appSettings.value("accounts/" + hash + "/password"),
                                       appSettings.value("accounts/" + hash + "/username"),
                                       appSettings.value("accounts/" + hash + "/url"))
            var tmpaccounts = accounts
            tmpaccounts.pop(hash)
            accounts = tmpaccounts

            tmpAccount.path = appSettings.path + "/accounts/" + hash
            tmpAccount.clear()
            currentAccount = accounts[-1]
        }
    }

    ConfigurationGroup {
        id: account
        Connections {
            target: appSettings
            onCurrentAccountChanged: account.path = appSettings.path + "/accounts/" + appSettings.currentAccount
        }

        property url url: value("url", "", String)
        property string username: value("username", "", String)
        property string passowrd: value("password", "", String)
        property string name: value("name", "", String)
        property var update: value("update", new Date(0), Date)
    }

    ConfigurationGroup {
        id: tmpAccount
        property url url
        property string username
        property string passowrd
        property string name
        property var update
    }

    function clearApp() {
        appSettings.clear()
    }

    Notification {
        id: offlineNotification
        expireTimeout: 0
        appName: "Nextcloud " + qsTr("Notes")
        summary: qsTr("Offline")
        body: qsTr("Synced") + ": " + new Date(account.update).toLocaleString(Qt.locale())
        Component.onDestruction: close()
    }

    Notification {
        id: storeErrorNotification
        appName: offlineNotification.appName
        summary: qsTr("File error")
        Component.onDestruction: close()
    }

    Notification {
        id: apiErrorNotification
        appName: offlineNotification.appName
        summary: qsTr("API error")
        Component.onDestruction: close()
    }

    Timer {
        id: autoSyncTimer
        interval: appSettings.autoSyncInterval * 1000
        repeat: true
        running: interval > 0 && notesApi.networkAccessible && appWindow.visible
        triggeredOnStart: true
        onTriggered: {
            notesApi.getAllNotes()
        }
        onIntervalChanged: {
            if (interval > 0) {
                console.log("Auto-Sync every " + interval / 1000 + " seconds")
            }
            else {
                console.log("Auto-Sync disabled")
            }
        }
    }

    Nextcloud {
        id: nextcloud
        server: account.url
        username: account.username
        password: account.passowrd
    }

    Component.onCompleted: {
    }

    Component.onDestruction: {
        offlineNotification.close()
        storeErrorNotification.close()
        apiErrorNotification.close()
    }

    initialPage: Component { NotesPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
