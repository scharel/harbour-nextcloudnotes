import QtQuick 2.2
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import Nemo.Notifications 1.0
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
        property string currentAccountIndex: value("currentAccountIndex", -1, Number)
        property int autoSyncInterval: value("autoSyncInterval", 0, Number)
        property int previewLineCount: value("previewLineCount", 4, Number)
        property bool favoritesOnTop: value("favoritesOnTop", true, Boolean)
        property string sortBy: value("sortBy", "modifiedString", String)
        property bool showSeparator: value("showSeparator", false, Boolean)
        property bool useMonoFont: value("useMonoFont", false, Boolean)
        property bool useCapitalX: value("useCapitalX", false, Boolean)

        onCurrentAccountIndexChanged: {
            console.log("Current account index: " + currentAccountIndex)
            if (currentAccountIndex >= 0 && currentAccountIndex < accounts.length) {
                account = accounts[currentAccountIndex]
                console.log("Current account: " + account.username + "@" + account.url)
            }
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

        function createAccount(user, url) {
            var hash = accountHash.hash(user, url)
            console.log("Hash(" + user + "@" + url + ") = " + hash)
            return hash
        }
        function removeAccount(hash) {
            accounts[hash] = null
            currentAccount = -1        }
    }

    property var account

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
        }
    }

    Connections {
        target: notesApi

        onNetworkAccessibleChanged: {
            console.log("Device is " + (accessible ? "online" : "offline"))
            if (accessible) {
                offlineNotification.close(Notification.Closed)
                getAllNotes()
            }
            else {
                offlineNotification.publish()
            }
        }
        onNoteError: {
            apiErrorNotification.close()
            if (error)
                console.log("Notes API error (" + error + "): " + notesApi.errorMessage(error))
            if (error && notesApi.networkAccessible) {
                apiErrorNotification.body = notesApi.errorMessage(error)
                apiErrorNotification.publish()
            }
        }
        onLastSyncChanged: {
            console.log("Last API sync: " + lastSync)
            account.update = lastSync
        }
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
